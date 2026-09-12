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

    def olive(L):
        return (L * 0.62, min(1.0, L * 0.86), L * 0.40)

    def vivid_green(L):
        return (L * 0.30, min(1.0, L * 1.45), L * 0.22)

    def vivid_amber(L):
        return (min(1.0, L * 1.50), L * 1.05, L * 0.30)

    def vivid_cyan(L):
        return (L * 0.20, min(1.0, L * 1.25), L * 0.85)

    def blend(a, b_, k):
        return tuple(a[i] * (1 - k) + b_[i] * k for i in range(3))

    # 1b. Already-green (g dominant, b and r both clearly below g) — leave as-is,
    # it already reads as phosphor green (e.g. success/positive states).
    if g == mx and (g - b) > 0.28 and (g - r) > 0.28:
        return (r, g, b)

    # 2. Mint/teal (g dominant, g>b>r, b close to g i.e. cyan-ish) -> amber accent
    if g == mx and g > b and g > r and (g - r) > 0.25:
        k = min(1.0, sat * 1.7)
        out = blend(olive(V), vivid_amber(V), k)
        return tuple(clamp(c) for c in out)

    # 3. Warm orange (r dominant, r>g>b) -> primary green accent
    if r == mx and r > g and g > b and (g - b) > 0.15:
        k = min(1.0, sat * 1.7)
        out = blend(olive(V), vivid_green(V), k)
        return tuple(clamp(c) for c in out)

    # 4. Blue-ish (b dominant, b>g>=r) -> cyan-green accent
    if b == mx and b > g and g >= r and (b - r) > 0.15:
        k = min(1.0, sat * 1.7)
        out = blend(olive(V), vivid_cyan(V), k)
        return tuple(clamp(c) for c in out)

    # 5. Neutral / gray family (structural chrome) -> olive-black PDA ramp
    if V >= 0.80:
        out = (V * 0.83, min(1.0, V * 1.00), V * 0.78)
    elif V >= 0.55:
        out = (V * 0.70, min(1.0, V * 0.95), V * 0.58)
    else:
        out = olive(V)
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
    # Read/write raw bytes with manual utf-8 decode so existing line endings
    # (this project's files mix bare \n and \r\n) are never touched — only
    # the numeric color tokens inside matched lines are replaced.
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
