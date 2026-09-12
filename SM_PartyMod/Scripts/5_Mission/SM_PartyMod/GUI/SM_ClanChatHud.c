// Постоянное окно чата мода. Всегда на экране (ignorepointer на всех виджетах,
// не блокирует управление). Длинные сообщения переносятся ВРУЧНУЮ по словам
// (как делает ванильный чат — режет по символам, а не полагается на авто-перенос
// виджета, который ненадёжен). Каждая готовая визуальная строка = отдельный
// RichTextWidget с цветной разметкой <color>. Фон коробки подгоняется под
// содержимое (нижний край закреплён у поля ввода, растёт вверх). Затухает
// в простое, проявляется при новом сообщении или открытии ввода.
class SM_ClanChatHud
{
	protected const int MAX_VISIBLE_MESSAGES = 18;
	protected const float IDLE_VISIBLE = 8.0;   // сек полной видимости после события
	protected const float FADE_DURATION = 1.5;  // сек затухания
	protected const float INPUT_BOTTOM_OFFSET = 124;

	protected Widget m_Root;
	protected Widget m_Panel;
	protected ref array<Widget> m_Rows = new array<Widget>;
	protected float m_Alpha = -1;
	protected bool m_Dirty = true;

	void Init()
	{
		m_Root = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_ClanChatHud.layout");
		if (!m_Root)
			return;

		m_Panel = m_Root.FindAnyWidget("ClanChatPanel");
		m_Root.Show(true);
		m_Dirty = true;
	}

	protected bool IsInputOpen()
	{
		if (GetGame().GetUIManager().FindMenu(SM_PARTY_CHAT_MENU_ID))
			return true;
		if (GetGame().GetUIManager().FindMenu(MENU_CHAT_INPUT))
			return true;
		return false;
	}

	void Update(float timeslice)
	{
		if (!m_Root || !m_Panel)
			return;
		if (SM_HudVisibility.IsGameHudHidden())
		{
			m_Root.Show(false);
			m_Alpha = -1;
			return;
		}
		if (!SM_ClanClientData.ChatEnabled)
		{
			m_Root.Show(false);
			m_Alpha = -1;
			return;
		}

		if (SM_ClanClientData.ChatViewDirty)
		{
			SM_ClanClientData.ChatViewDirty = false;
			m_Dirty = true;
		}

		float target = ComputeTargetAlpha(IsInputOpen());
		float opacity = SM_ClanClientData.ChatOpacityPercent / 100.0;
		if (opacity < 0)
			opacity = 0;
		if (opacity > 1)
			opacity = 1;
		float finalAlpha = target * opacity;

		if (m_Dirty || finalAlpha != m_Alpha)
		{
			m_Dirty = false;
			m_Alpha = finalAlpha;
			Refresh();
			ApplyAlpha(finalAlpha);
		}
	}

	protected void ApplyAlpha(float alpha)
	{
		int alphaByte = Math.Round(alpha * 255);
		if (alphaByte < 0)
			alphaByte = 0;
		if (alphaByte > 255)
			alphaByte = 255;

		m_Root.SetAlpha(1.0);
		m_Root.Show(alpha > 0.001);
		if (m_Panel)
			m_Panel.SetAlpha(1.0);

		for (int i = 0; i < m_Rows.Count(); i++)
		{
			if (!m_Rows[i])
				continue;

			m_Rows[i].SetAlpha(1.0);

			RichTextWidget rt = RichTextWidget.Cast(m_Rows[i]);
			if (rt)
			{
				rt.SetColor(ARGB(255, 212, 255, 199));
				rt.SetOutline(2, ARGB(alphaByte, 0, 0, 0));
				rt.SetShadow(3, ARGB(alphaByte, 0, 0, 0), 1.0, 1, 1);
			}
		}
	}

	protected float ComputeTargetAlpha(bool inputOpen)
	{
		if (SM_ClanClientData.ChatHistory.Count() == 0)
			return 0;

		if (inputOpen)
			return 1.0;

		float since = GetGame().GetTickTime() - SM_ClanClientData.ChatActivityTime;
		float idle = SM_ClanClientData.ChatFadeSeconds;
		if (since < idle)
			return 1.0;
		if (since < idle + FADE_DURATION)
			return 1.0 - (since - idle) / FADE_DURATION;
		return 0;
	}

	protected float GetScale()
	{
		float scale = SM_ClanClientData.NormalizeChatScale(SM_ClanClientData.ChatScale);
		SM_ClanClientData.ChatScale = scale;
		return scale;
	}

	// Нижний край коробки (px от низа экрана) — над строкой подсказки/ввода.
	protected float GetBottomAnchor(float scale)
	{
		float inputY = INPUT_BOTTOM_OFFSET;
		float panelH = 46 * scale;
		float gap = 8 * scale;
		float hintH = 18 * scale;
		return inputY + panelH + gap + hintH + gap;
	}

	protected Widget GetRow(int index)
	{
		while (m_Rows.Count() <= index)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_ClanChatHudRow.layout", m_Panel);
			RichTextWidget rt = RichTextWidget.Cast(w);
			if (rt)
			{
				rt.SetOutline(2, ARGB(255, 0, 0, 0));
				rt.SetShadow(3, ARGB(255, 0, 0, 0), 1.0, 1, 1);
			}
			w.SetAlpha(1.0);
			m_Rows.Insert(w);
		}
		return m_Rows[index];
	}

	// ARGB int -> строка "r, g, b, a" для разметки RichText.
	protected string RichColor(int color)
	{
		int a = (color >> 24) & 0xFF;
		int r = (color >> 16) & 0xFF;
		int g = (color >> 8) & 0xFF;
		int b = color & 0xFF;
		if (m_Alpha >= 0)
			a = Math.Round(a * m_Alpha);
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		return r.ToString() + ", " + g.ToString() + ", " + b.ToString() + ", " + a.ToString();
	}

	protected string ColorTag(string text, int color)
	{
		return "<color rgba=\"" + RichColor(color) + "\">" + text + "</color>";
	}

	// Убираем угловые скобки (защита от инъекции разметки игроками).
	protected string Sanitize(string s)
	{
		string copy = s;
		copy.Replace("<", "");
		copy.Replace(">", "");
		return copy;
	}

	// Разбивает строку на слова по пробелам (UTF-8 безопасно).
	protected void SplitWords(string s, array<string> words)
	{
		int n = s.LengthUtf8();
		string cur = "";
		for (int i = 0; i < n; i++)
		{
			string ch = s.SubstringUtf8(i, 1);
			if (ch == " ")
			{
				if (cur != "")
				{
					words.Insert(cur);
					cur = "";
				}
			}
			else
			{
				cur = cur + ch;
			}
		}
		if (cur != "")
			words.Insert(cur);
	}

	// Переносит сообщение на несколько строк по словам (с запасом по ширине,
	// чтобы гарантированно не обрезалось). Возвращает готовые markup-строки.
	protected void BuildWrappedLines(SM_ChatHistoryEntry entry, int maxChars, array<string> outLines)
	{
		int channelColor = entry.ChannelColor;
		if (channelColor == 0)
			channelColor = entry.Color;
		if (channelColor == 0)
			channelColor = ARGB(255, 195, 235, 183);

		int nameColor = entry.NameColor;
		if (nameColor == 0)
			nameColor = ARGB(255, 195, 235, 183);

		int textColor = entry.TextColor;
		if (textColor == 0)
			textColor = ARGB(255, 195, 235, 183);

		int prefixColor = entry.PrefixColor;
		if (prefixColor == 0)
			prefixColor = nameColor;

		// Токены: текст + цвет (имя/мета не разрезаем; текст бьём на слова).
		array<string> tokTexts = new array<string>;
		array<int> tokColors = new array<int>;

		if (SM_ClanClientData.ChatTimestamps && entry.Stamp != "")
			tokTexts.Insert("[" + entry.Stamp + "] [" + entry.Label + "]");
		else
			tokTexts.Insert("[" + entry.Label + "]");
		tokColors.Insert(channelColor);
		if (entry.Prefix != "")
		{
			tokTexts.Insert(Sanitize(entry.Prefix));
			tokColors.Insert(prefixColor);
		}
		if (entry.AuthorName != "")
		{
			tokTexts.Insert(Sanitize(entry.AuthorName) + ":");
			tokColors.Insert(nameColor);
		}

		array<string> words = new array<string>;
		SplitWords(Sanitize(entry.Text), words);
		for (int wi = 0; wi < words.Count(); wi++)
		{
			tokTexts.Insert(words[wi]);
			tokColors.Insert(textColor);
		}

		// Перенос токенов по символам.
		string curMarkup = "";
		int curLen = 0;

		for (int k = 0; k < tokTexts.Count(); k++)
		{
			string t = tokTexts[k];
			int col = tokColors[k];

			// Сверхдлинный токен — жёстко режем по maxChars.
			while (t.LengthUtf8() > maxChars)
			{
				if (curLen > 0)
				{
					outLines.Insert(curMarkup);
					curMarkup = "";
					curLen = 0;
				}
				string chunk = t.SubstringUtf8(0, maxChars);
				outLines.Insert(ColorTag(chunk, col));
				t = t.SubstringUtf8(maxChars, t.LengthUtf8() - maxChars);
			}

			int add = t.LengthUtf8();
			int sep = 0;
			if (curLen > 0)
				sep = 1;

			if (curLen + sep + add > maxChars && curLen > 0)
			{
				outLines.Insert(curMarkup);
				curMarkup = "";
				curLen = 0;
				sep = 0;
			}

			if (curLen > 0)
				curMarkup = curMarkup + " ";
			curMarkup = curMarkup + ColorTag(t, col);
			curLen = curLen + sep + add;
		}

		if (curLen > 0)
			outLines.Insert(curMarkup);
	}

	void Refresh()
	{
		if (!m_Panel)
			return;

		float scale = GetScale();
		float panelW = 560 * scale;
		float pad = 14 * scale;
		float lineH = 21 * scale;
		float innerW = panelW - 2 * pad;
		int maxMessages = SM_ClanClientData.ChatVisibleMessages;
		if (maxMessages < 1)
			maxMessages = 1;
		if (maxMessages > MAX_VISIBLE_MESSAGES)
			maxMessages = MAX_VISIBLE_MESSAGES;

		float minContentH = 2 * pad + lineH;
		int textSize = Math.Round(14 * scale);
		if (textSize < 11)
			textSize = 11;
		if (textSize > 20)
			textSize = 20;

		// Перенос с запасом: ширина символа берётся щедро, чтобы строка была
		// заметно уже коробки и не обрезалась даже на широких буквах.
		int maxChars = Math.Round(innerW / (10.0 * scale));
		if (maxChars < 12)
			maxChars = 12;

		int total = SM_ClanClientData.ChatHistory.Count();
		if (!SM_ClanClientData.ChatHistoryEnabled)
			SM_ClanClientData.ChatScrollOffset = 0;
		int endIndex = total - SM_ClanClientData.ChatScrollOffset;
		if (endIndex > total)
			endIndex = total;
		if (endIndex < 0)
			endIndex = 0;

		int startMsg = endIndex - maxMessages;
		if (startMsg < 0)
			startMsg = 0;

		// Все строки выбранных сообщений в хронологическом порядке (старые → новые).
		array<string> allLines = new array<string>;
		for (int i = startMsg; i < endIndex; i++)
		{
			SM_ChatHistoryEntry entry = SM_ClanClientData.ChatHistory[i];
			if (entry)
				BuildWrappedLines(entry, maxChars, allLines);
		}

		// Ограничение по высоте: коробка не должна вылезать за верх экрана.
		// Режем по числу ВИЗУАЛЬНЫХ строк (а не сообщений) — перенос длинных
		// сообщений раздувает панель, и при нижнем якоре она уезжает вверх.
		// Лишние (самые старые) строки убираем сверху, свежие остаются снизу.
		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);
		float availH = screenH - GetBottomAnchor(scale) - 40 * scale;
		int maxLines = Math.Floor((availH - 2 * pad) / lineH);
		if (maxLines < 1)
			maxLines = 1;
		while (allLines.Count() > maxLines)
			allLines.Remove(0);

		int lineCount = allLines.Count();
		float contentH = 2 * pad + lineCount * lineH;
		if (lineCount == 0)
			contentH = 2 * pad;
		if (contentH < minContentH)
			contentH = minContentH;

		// Панель ставим ЯВНО по верхней грани: нижний край ложится на якорь
		// у поля ввода, верх считаем сами. Раньше панель была с нижним якорем
		// (valign bottom) и движок «дорастал» её вверх сам — при высокой
		// панели это ломало порядок строк (новое улетало наверх, низ застывал).
		// Теперь координата верха детерминирована. Размер задаём ДО позиции,
		// чтобы дочерние строки не отставали на кадр.
		float panelTop = screenH - GetBottomAnchor(scale) - contentH;
		if (panelTop < 0)
			panelTop = 0;
		m_Panel.SetSize(panelW, contentH);
		m_Panel.SetPos(20 + SM_ClanClientData.ChatOffsetX, panelTop + SM_ClanClientData.ChatOffsetY);

		float y = pad;
		int rowIdx = 0;

		for (int li = 0; li < allLines.Count(); li++)
		{
			Widget rowWidget = GetRow(rowIdx);
			RichTextWidget rt = RichTextWidget.Cast(rowWidget);
			if (rt)
			{
				rt.SetText(SM_PartyLoc.Text(allLines[li]));
				rt.SetTextExactSize(textSize);
				rt.SetPos(pad, y);
				rt.SetSize(innerW, lineH);
				rt.Show(true);
			}
			y = y + lineH;
			rowIdx++;
		}

		// Прячем неиспользованные строки пула.
		for (int j = rowIdx; j < m_Rows.Count(); j++)
		{
			if (m_Rows[j])
				m_Rows[j].Show(false);
		}
	}
}
