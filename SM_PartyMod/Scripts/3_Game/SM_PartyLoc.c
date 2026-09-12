class SM_PartyLoc
{
	static const string KEY_PREFIX = "#STR_SMP_";
	static const int KEY_LENGTH = 14;

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

			if (pos + KEY_LENGTH > text.Length())
			{
				result = result + text.Substring(pos, text.Length() - pos);
				return result;
			}

			string key = text.Substring(pos, KEY_LENGTH);
			string translated = Widget.TranslateString(key);
			if (translated != "" && translated != key)
				result = result + translated;
			else
				result = result + key;

			cursor = pos + KEY_LENGTH;
			if (cursor >= text.Length())
				return result;

			string rest = text.Substring(cursor, text.Length() - cursor);
			int nextPos = rest.IndexOf(KEY_PREFIX);
			if (nextPos < 0)
				return result + rest;

			pos = cursor + nextPos;
		}

		return result;
	}

	static void AddNotification(float showTime, string title, string text, string icon = "")
	{
		NotificationSystem.AddNotificationExtended(showTime, Text(title), Text(text), icon);
	}
}
