// Небольшой помощник локализации: ищет в строке подстроки вида
// "#STR_SMD_XXX" (ключ из stringtable.csv) и заменяет их переводом,
// оставляя остальной текст (например, динамически подставленные имена
// игроков или числа) как есть. Ключ считается всем, что идёт после
// префикса и состоит из букв, цифр и подчёркивания.
class SM_DungeonLoc
{
	static const string KEY_PREFIX = "#STR_SMD_";

	static string Text(string text)
	{
		if (text == "")
			return text;

		int pos = text.IndexOf(KEY_PREFIX);
		if (pos < 0)
			return text;

		string result = "";
		int cursor = 0;

		while (pos >= 0)
		{
			if (pos > cursor)
				result = result + text.Substring(cursor, pos - cursor);

			int keyEnd = pos;
			int len = text.Length();
			while (keyEnd < len && IsKeyChar(text.Substring(keyEnd, 1)))
				keyEnd++;

			string key = text.Substring(pos, keyEnd - pos);
			string translated = Widget.TranslateString(key);
			if (translated != "" && translated != key)
				result = result + translated;
			else
				result = result + key;

			cursor = keyEnd;
			if (cursor >= len)
				return result;

			string rest = text.Substring(cursor, len - cursor);
			int nextPos = rest.IndexOf(KEY_PREFIX);
			if (nextPos < 0)
			{
				result = result + rest;
				return result;
			}
			pos = cursor + nextPos;
		}

		if (cursor < text.Length())
			result = result + text.Substring(cursor, text.Length() - cursor);

		return result;
	}

	protected static bool IsKeyChar(string ch)
	{
		if (ch == "_")
			return true;
		if (ch >= "A" && ch <= "Z")
			return true;
		if (ch >= "a" && ch <= "z")
			return true;
		if (ch >= "0" && ch <= "9")
			return true;
		return false;
	}
}
