import re, sys

def clamp(x):
    return max(0.0, min(1.0, x))

def transform_rgb(r, g, b):
    mx = max(r, g, b)
    mn = min(r, g, b)
    spread = mx - mn
    V = (r + g + b) / 3.0
    sat = (spread / mx) if mx > 1e-6 else 0.0

    # 1. Red / danger — leave untouched (unconscious, bleeding, disband, errors)
    if r == mx and (r - max(g, b)) > 0.30 and abs(g - b) < 0.12:
        return (r, g, b)

    def warm_dark(L):
        return (L * 1.17, L * 1.00, L * 0.83)

    def warm_mid(L):
        return (L * 1.10, L * 1.01, L * 0.88)

    def warm_pale(L):
        return (L * 1.04, L * 1.00, L * 0.95)

    def vivid_copper(L):
        # calibrated so the mod's original saturated orange accent lands on #b87333
        return (L * 1.229, L * 0.768, L * 0.341)

    def vivid_amber(L):
        # calibrated so the mod's original teal/mint accent lands on #e8b545
        return (L * 1.376, L * 1.074, L * 0.409)

    def blend(a, b_, k):
        return tuple(a[i] * (1 - k) + b_[i] * k for i in range(3))

    def warm_neutral(L):
        if L >= 0.80:
            return warm_pale(L)
        if L >= 0.35:
            return warm_mid(L)
        return warm_dark(L)

    # 2. Green family — g is the max channel (mint/cyan AND pristine
    # saturated greens alike) -> secondary amber accent. No hue is left
    # green anywhere in the new palette.
    if g == mx and (g - r) > 0.20:
        k = min(1.0, sat * 1.7)
        out = blend(warm_neutral(V), vivid_amber(V), k)
        return tuple(clamp(c) for c in out)

    # 3. Warm orange (r dominant, r>g>b) -> primary copper accent
    if r == mx and r > g and g > b and (g - b) > 0.15:
        k = min(1.0, sat * 1.7)
        out = blend(warm_neutral(V), vivid_copper(V), k)
        return tuple(clamp(c) for c in out)

    # 4. Blue-ish (b dominant) -> treat as secondary amber accent too,
    # keeping exactly two accent hues (copper + amber) as in the mockup.
    if b == mx and (b - r) > 0.15:
        k = min(1.0, sat * 1.7)
        out = blend(warm_neutral(V), vivid_amber(V), k)
        return tuple(clamp(c) for c in out)

    # 5. Neutral / gray family (structural chrome) -> warm charcoal ramp
    out = warm_neutral(V)
    return tuple(clamp(c) for c in out)


def fmt_float(x):
    s = f"{x:.4f}".rstrip('0').rstrip('.')
    if s in ('', '-0'):
        s = '0'
    return s


def process_layout(text):
    pattern = re.compile(
        r'(?m)^(?P<indent>[ \t]*color[ \t]+)'
        r'(?P<r>[0-9]*\.?[0-9]+)[ \t]+(?P<g>[0-9]*\.?[0-9]+)[ \t]+'
        r'(?P<b>[0-9]*\.?[0-9]+)[ \t]+(?P<a>[0-9]*\.?[0-9]+)'
    )

    def repl(m):
        r, g, b = float(m.group('r')), float(m.group('g')), float(m.group('b'))
        a = m.group('a')
        nr, ng, nb = transform_rgb(r, g, b)
        return f"{m.group('indent')}{fmt_float(nr)} {fmt_float(ng)} {fmt_float(nb)} {a}"

    return pattern.sub(repl, text)


def process_code(text):
    pattern = re.compile(r'ARGB\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)')

    def repl(m):
        a, r, g, b = (int(x) for x in m.groups())
        nr, ng, nb = transform_rgb(r / 255.0, g / 255.0, b / 255.0)
        R, G, B = round(nr * 255), round(ng * 255), round(nb * 255)
        return f"ARGB({a}, {R}, {G}, {B})"

    return pattern.sub(repl, text)


if __name__ == '__main__':
    mode, path = sys.argv[1], sys.argv[2]
    with open(path, 'rb') as f:
        raw = f.read()
    text = raw.decode('utf-8')
    if mode == 'layout':
        out = process_layout(text)
    else:
        out = process_code(text)
    with open(path, 'wb') as f:
        f.write(out.encode('utf-8'))
    print("done", path)
