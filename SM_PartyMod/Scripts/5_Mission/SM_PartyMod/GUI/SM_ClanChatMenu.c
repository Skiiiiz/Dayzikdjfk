// Окно ввода чата (клавиша чата). Сообщения показывает постоянное окно
// SM_ClanChatHud — здесь только строка ввода и выбор канала.
class SM_ClanChatMenu extends UIScriptedMenu
{
	protected const float INPUT_BOTTOM_OFFSET = 124;

	protected EditBoxWidget m_EditBox;
	protected TextWidget m_ChannelText;
	protected TextWidget m_HintText;
	protected Widget m_ChannelPill;
	protected Widget m_ChannelFill;
	protected Widget m_ChannelAccent;
	protected Widget m_Panel;
	protected Widget m_InputBg;
	protected Widget m_InputCover;
	protected Widget m_InputUnderline;
	protected TextWidget m_InputText;
	protected ref Timer m_CloseTimer;

	protected ref array<int> m_Channels = new array<int>;
	protected int m_ChannelIdx;
	protected bool m_TabWasDown;
	protected bool m_ScaleUpWasDown;
	protected bool m_ScaleDownWasDown;
	protected bool m_HistoryUpWasDown;
	protected bool m_HistoryDownWasDown;

	void SM_ClanChatMenu()
	{
		m_CloseTimer = new Timer();
	}

	override int GetID()
	{
		return SM_PARTY_CHAT_MENU_ID;
	}

	override bool UseMouse()
	{
		return false;
	}

	override bool UseKeyboard()
	{
		return true;
	}

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_ClanChatInput.layout");
		m_EditBox = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("ClanChatEditBox"));
		m_ChannelText = TextWidget.Cast(layoutRoot.FindAnyWidget("ClanChatChannelText"));
		m_HintText = TextWidget.Cast(layoutRoot.FindAnyWidget("ClanChatHintText"));
		m_ChannelPill = layoutRoot.FindAnyWidget("ClanChatChannelPill");
		m_ChannelFill = layoutRoot.FindAnyWidget("ClanChatChannelFill");
		m_ChannelAccent = layoutRoot.FindAnyWidget("ClanChatChannelAccent");
		m_Panel = layoutRoot.FindAnyWidget("ClanChatPanel");
		m_InputBg = layoutRoot.FindAnyWidget("ClanChatInputBg");
		m_InputCover = layoutRoot.FindAnyWidget("ClanChatInputCover");
		m_InputUnderline = layoutRoot.FindAnyWidget("ClanChatInputUnderline");
		m_InputText = TextWidget.Cast(layoutRoot.FindAnyWidget("ClanChatVisibleText"));

		ApplyInputStyle();
		ApplyTextReadability();
		BuildChannels();
		ApplyChannel();
		ApplyScale();
		RefreshInputText();

		return layoutRoot;
	}

	protected void ApplyInputStyle()
	{
		if (m_InputBg)
			m_InputBg.SetColor(SM_ChatChannelPalette.GetBorderColor(SM_ChatChannel.GLOBAL));
		if (m_InputCover)
			m_InputCover.SetColor(SM_ChatChannelPalette.GetBackgroundColor(SM_ChatChannel.GLOBAL));
		if (m_InputUnderline)
			m_InputUnderline.SetColor(ARGB(115, 184, 115, 51));
		if (m_InputText)
			m_InputText.SetColor(ARGB(255, 244, 235, 223));
	}

	protected void ApplyTextReadability()
	{
		if (m_ChannelText)
		{
			m_ChannelText.SetOutline(1, ARGB(255, 0, 0, 0));
			m_ChannelText.SetShadow(2, ARGB(255, 0, 0, 0), 0.95, 1, 1);
		}
		if (m_HintText)
		{
			m_HintText.SetOutline(2, ARGB(255, 0, 0, 0));
			m_HintText.SetShadow(3, ARGB(255, 0, 0, 0), 1.0, 1, 1);
		}
		if (m_InputText)
		{
			m_InputText.SetShadow(1, ARGB(255, 0, 0, 0), 0.55, 1, 1);
		}
	}

	protected void RefreshInputText()
	{
		if (!m_EditBox || !m_InputText)
			return;

		string visible = m_EditBox.GetText() + "|";
		m_InputText.SetText(visible);

		int textW;
		int textH;
		m_InputText.GetTextSize(textW, textH);

		float maxW;
		float maxH;
		m_InputText.GetSize(maxW, maxH);
		maxW = maxW - 6;

		while (visible.LengthUtf8() > 1 && textW > maxW)
		{
			visible = visible.SubstringUtf8(1, visible.LengthUtf8() - 1);
			m_InputText.SetText(visible);
			m_InputText.GetTextSize(textW, textH);
		}
	}

	protected void BuildChannels()
	{
		m_Channels.Clear();
		if (!SM_ClanClientData.ChatEnabled)
		{
			m_ChannelIdx = 0;
			return;
		}

		if (SM_ClanClientData.HasClan && SM_ClanClientData.ChatEnabled)
			m_Channels.Insert(SM_ChatChannel.CLAN);
		if (SM_ClanClientData.LocalChatEnabled)
			m_Channels.Insert(SM_ChatChannel.LOCAL);
		if (SM_ClanClientData.GlobalChatEnabled)
			m_Channels.Insert(SM_ChatChannel.GLOBAL);
		if (SM_ClanClientData.IsAdmin)
			m_Channels.Insert(SM_ChatChannel.SERVER);
		if (m_Channels.Count() == 0)
		{
			m_ChannelIdx = 0;
			return;
		}

		m_ChannelIdx = 0;
		for (int i = 0; i < m_Channels.Count(); i++)
		{
			if (m_Channels[i] == SM_ChatChannel.GLOBAL)
			{
				m_ChannelIdx = i;
				break;
			}
		}
	}

	protected bool HasChannels()
	{
		if (m_Channels && m_Channels.Count() > 0)
			return true;
		return false;
	}

	protected int CurrentChannel()
	{
		if (!HasChannels())
			return SM_ChatChannel.GLOBAL;
		return m_Channels[m_ChannelIdx];
	}

	protected void CycleChannel()
	{
		if (m_Channels.Count() <= 1)
			return;
		m_ChannelIdx = (m_ChannelIdx + 1) % m_Channels.Count();
		ApplyChannel();
		SetFocus(m_EditBox);
	}

	protected int GetChannelTextColor(int channel)
	{
		if (channel == SM_ChatChannel.LOCAL && SM_ClanClientData.ChatColorDirect != 0)
			return SM_ClanClientData.ChatColorDirect;
		if (channel == SM_ChatChannel.GLOBAL && SM_ClanClientData.ChatColorGlobal != 0)
			return SM_ClanClientData.ChatColorGlobal;
		if (channel == SM_ChatChannel.SERVER && SM_ClanClientData.ChatColorServer != 0)
			return SM_ClanClientData.ChatColorServer;
		return SM_ChatChannelPalette.GetTextColor(channel);
	}

	protected int GetChannelAccentColor(int channel)
	{
		if (channel == SM_ChatChannel.CLAN)
			return SM_ClanClientData.GetClanColor();
		if (channel == SM_ChatChannel.LOCAL)
		{
			if (SM_ClanClientData.ChatColorDirectPlayer != 0)
				return SM_ClanClientData.ChatColorDirectPlayer;
			if (SM_ClanClientData.ChatColorDirect != 0)
				return SM_ClanClientData.ChatColorDirect;
		}
		if (channel == SM_ChatChannel.GLOBAL)
		{
			if (SM_ClanClientData.ChatColorGlobalPlayer != 0)
				return SM_ClanClientData.ChatColorGlobalPlayer;
			if (SM_ClanClientData.ChatColorGlobal != 0)
				return SM_ClanClientData.ChatColorGlobal;
		}
		if (channel == SM_ChatChannel.SERVER && SM_ClanClientData.ChatColorServer != 0)
			return SM_ClanClientData.ChatColorServer;
		return SM_ChatChannelPalette.GetAccentColor(channel);
	}

	protected void ApplyChannel()
	{
		if (!HasChannels())
		{
			if (m_ChannelText)
			{
				m_ChannelText.SetText(SM_PartyLoc.Text("#STR_SMP_01064"));
				m_ChannelText.SetColor(ARGB(255, 198, 182, 158));
			}
			if (m_ChannelPill)
				m_ChannelPill.SetColor(ARGB(120, 94, 80, 66));
			if (m_ChannelFill)
				m_ChannelFill.SetColor(ARGB(90, 23, 20, 17));
			if (m_ChannelAccent)
				m_ChannelAccent.SetColor(ARGB(0, 0, 0, 0));
			RefreshHint("#STR_SMP_01063");
			return;
		}

		int ch = CurrentChannel();
		string label = "#STR_SMP_01063";
		switch (ch)
		{
			case SM_ChatChannel.CLAN:
				label = "#STR_SMP_00505";
				break;
			case SM_ChatChannel.LOCAL:
				label = "#STR_SMP_00894";
				break;
			case SM_ChatChannel.GLOBAL:
				label = "#STR_SMP_00383";
				break;
			case SM_ChatChannel.SERVER:
				label = "#STR_SMP_00909";
				break;
		}

		if (m_ChannelText)
		{
			m_ChannelText.SetText(SM_PartyLoc.Text(label));
			m_ChannelText.SetColor(GetChannelTextColor(ch));
		}
		if (m_ChannelPill)
		{
			m_ChannelPill.SetColor(SM_ChatChannelPalette.GetBorderColor(ch));
		}
		if (m_ChannelFill)
		{
			m_ChannelFill.SetColor(SM_ChatChannelPalette.GetBackgroundColor(ch));
		}
		if (m_ChannelAccent)
		{
			m_ChannelAccent.SetColor(GetChannelAccentColor(ch));
		}
		RefreshHint(label);
	}

	protected void RefreshHint(string label)
	{
		if (!m_HintText)
			return;

		int pct = SM_ClanClientData.GetChatScalePercent();
		string hint = "#STR_SMP_00226" + pct.ToString() + "% · Enter / Esc";
		if (SM_ClanClientData.ChatHistoryEnabled)
			hint = "#STR_SMP_00229" + hint;
		if (m_Channels.Count() > 1)
			hint = "#STR_SMP_00232" + hint;
		m_HintText.SetText(SM_PartyLoc.Text(hint));
	}

	protected void ApplyScale()
	{
		float scale = SM_ClanClientData.NormalizeChatScale(SM_ClanClientData.ChatScale);
		SM_ClanClientData.ChatScale = scale;

		float x = 20;
		float inputY = INPUT_BOTTOM_OFFSET;
		float panelW = 760 * scale;
		float panelH = 46 * scale;
		float gap = 8 * scale;
		float hintH = 18 * scale;

		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);
		if (screenW > 0 && panelW > screenW - x * 2)
			panelW = screenW - x * 2;

		float fieldH = 30 * scale;
		float fieldY = (panelH - fieldH) / 2;
		float pillW = 92 * scale;
		float pillX = 8 * scale;
		float pillBorder = 1 * scale;
		if (pillBorder < 1)
			pillBorder = 1;
		float accentW = 4 * scale;
		if (accentW < 3)
			accentW = 3;
		float inputX = pillX + pillW + 8 * scale;
		float inputW = panelW - inputX - 8 * scale;

		if (m_Panel)
		{
			m_Panel.SetPos(x, inputY);
			m_Panel.SetSize(panelW, panelH);
		}
		if (m_ChannelPill)
		{
			m_ChannelPill.SetPos(pillX, fieldY);
			m_ChannelPill.SetSize(pillW, fieldH);
		}
		if (m_ChannelFill)
		{
			m_ChannelFill.SetPos(pillBorder, pillBorder);
			m_ChannelFill.SetSize(pillW - pillBorder * 2, fieldH - pillBorder * 2);
		}
		if (m_ChannelAccent)
		{
			m_ChannelAccent.SetPos(0, 0);
			m_ChannelAccent.SetSize(accentW, fieldH - pillBorder * 2);
		}
		if (m_ChannelText)
		{
			m_ChannelText.SetPos(accentW, 0);
			m_ChannelText.SetSize(pillW - pillBorder * 2 - accentW, fieldH - pillBorder * 2);
		}
		if (m_InputBg)
		{
			m_InputBg.SetPos(inputX, fieldY);
			m_InputBg.SetSize(inputW, fieldH);
		}
		if (m_EditBox)
		{
			m_EditBox.SetPos(12 * scale, 0);
			m_EditBox.SetSize(inputW - 24 * scale, fieldH);
		}
		if (m_InputCover)
		{
			float inputBorder = 1 * scale;
			if (inputBorder < 1)
				inputBorder = 1;
			m_InputCover.SetPos(inputBorder, inputBorder);
			m_InputCover.SetSize(inputW - inputBorder * 2, fieldH - inputBorder * 2);
		}
		if (m_InputText)
		{
			m_InputText.SetPos(12 * scale, 0);
			m_InputText.SetSize(inputW - 24 * scale, fieldH);
		}
		if (m_InputUnderline)
		{
			float underlineH = 2 * scale;
			if (underlineH < 1)
				underlineH = 1;
			m_InputUnderline.SetPos(0, fieldH - underlineH);
			m_InputUnderline.SetSize(inputW, underlineH);
		}
		if (m_HintText)
		{
			float hintX = x + 16 * scale;
			float hintW = panelW + 220 * scale;
			if (screenW > 0)
			{
				hintW = screenW - hintX - 20 * scale;
				if (hintW < panelW)
					hintW = panelW;
			}

			m_HintText.SetPos(hintX, inputY + panelH + gap);
			m_HintText.SetSize(hintW, hintH);
		}

		ApplyChannel();
		RefreshInputText();
	}

	protected void ChangeScale(int deltaPercent)
	{
		SM_ClanClientData.ChangeChatScale(deltaPercent);
		ApplyScale();
		SetFocus(m_EditBox);
	}

	protected bool IsScaleUpKey(int key)
	{
		if (key == KeyCode.KC_ADD)
			return true;
		if (key == KeyCode.KC_EQUALS)
			return true;
		return false;
	}

	protected bool IsCtrlDown()
	{
		if (KeyState(KeyCode.KC_LCONTROL) != 0)
			return true;
		if (KeyState(KeyCode.KC_RCONTROL) != 0)
			return true;
		return false;
	}

	protected bool IsScaleUpPressed()
	{
		if (KeyState(KeyCode.KC_ADD) != 0)
			return true;
		if (KeyState(KeyCode.KC_EQUALS) != 0)
			return true;
		return false;
	}

	protected bool IsScaleDownPressed()
	{
		if (KeyState(KeyCode.KC_SUBTRACT) != 0)
			return true;
		if (KeyState(KeyCode.KC_MINUS) != 0)
			return true;
		return false;
	}

	protected bool IsScaleDownKey(int key)
	{
		if (key == KeyCode.KC_SUBTRACT)
			return true;
		if (key == KeyCode.KC_MINUS)
			return true;
		return false;
	}

	protected bool ShouldUseVanillaChatOnly(string text)
	{
		string checkText = text;
		checkText = checkText.Trim();
		if (checkText == "")
			return false;
		if (checkText.IndexOf("#") == 0)
			return true;
		if (checkText.IndexOf("!") == 0)
			return true;
		if (checkText.IndexOf("/") == 0)
			return true;
		if (checkText.IndexOf("\\") == 0)
			return true;
		return false;
	}

	protected void SendVanillaChat(string text)
	{
		string sendText = text;
		sendText = sendText.Trim();
		if (sendText == "")
			return;

		GetGame().ChatPlayer(sendText);
		if (!GetGame().IsMultiplayer())
		{
			string name;
			GetGame().GetPlayerName(name);
			int directColor = SM_ClanClientData.ChatColorDirect;
			if (directColor == 0)
				directColor = ARGB(255, 239, 230, 218);
			SM_ClanClientData.AddSystemChat(SM_ChatChannel.SERVER, "#STR_SMP_00877", name, sendText, directColor, directColor);
		}
	}

	protected void SendCFToolsChatMirror(int channel, string text)
	{
		string mirrorText = SM_CFToolsChat.BuildMirrorMessage(channel, text);
		if (mirrorText == "")
			return;

		GetGame().ChatPlayer(mirrorText);
	}

	protected void SendAdminChatCommand(string text)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(text);
		rpc.Send(player, SM_PartyRPC.ADMIN_CHAT_COMMAND, true, NULL);
	}

	override void OnShow()
	{
		super.OnShow();
		SM_VPPInputSuppressor.SuppressVPPAdminInputs();

		if (!SM_ClanClientData.ChatEnabled || !HasChannels())
		{
			m_CloseTimer.Run(0.01, this, "Close");
			return;
		}

		Mission mission = GetGame().GetMission();
		if (mission)
			mission.PlayerControlDisable(INPUT_EXCLUDE_ALL);

		// Показываем самые свежие строки и будим постоянное окно.
		SM_ClanClientData.ChatScrollOffset = 0;
		SM_ClanClientData.ChatActivityTime = GetGame().GetTickTime();
		SM_ClanClientData.ChatViewDirty = true;

		SetFocus(m_EditBox);
	}

	override void OnHide()
	{
		super.OnHide();

		// Возвращаемся к свежим строкам, окно снова начнёт затухать.
		SM_ClanClientData.ChatScrollOffset = 0;
		SM_ClanClientData.ChatActivityTime = GetGame().GetTickTime();
		SM_ClanClientData.ChatViewDirty = true;

		Mission mission = GetGame().GetMission();
		if (mission)
			mission.PlayerControlEnable(false);
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		super.OnChange(w, x, y, finished);

		if (!finished)
		{
			RefreshInputText();
			return false;
		}

		RefreshInputText();

		if (!SM_ClanClientData.ChatEnabled || !HasChannels())
		{
			m_CloseTimer.Run(0.1, this, "Close");
			GetUApi().GetInputByID(UAPersonView).Supress();
			return true;
		}

		string text = m_EditBox.GetText();
		if (text != "")
		{
			int channel = CurrentChannel();
			if (SM_PartyAdminCommands.IsReloadConfigCommand(text))
			{
				Man reloadPlayer = GetGame().GetPlayer();
				if (reloadPlayer)
				{
					ScriptRPC reloadRpc = new ScriptRPC();
					reloadRpc.Send(reloadPlayer, SM_PartyRPC.ADMIN_RELOAD_CONFIG, true, NULL);
				}
			}
			else if (SM_PartyAdminCommands.IsChatAdminCommand(text))
			{
				SendAdminChatCommand(text);
			}
			else if (channel == SM_ChatChannel.SERVER)
			{
				Man serverChannelPlayer = GetGame().GetPlayer();
				if (serverChannelPlayer)
				{
					ScriptRPC serverChannelRpc = new ScriptRPC();
					serverChannelRpc.Write(channel);
					serverChannelRpc.Write(text);
					serverChannelRpc.Send(serverChannelPlayer, SM_PartyRPC.CHAT_SEND, true, NULL);
				}
			}
			else if (SM_ClanClientData.IsChatMuted())
			{
				int mutedColor = SM_ClanClientData.ChatColorAlert;
				if (mutedColor == 0)
					mutedColor = ARGB(255, 255, 110, 95);
				SM_ClanClientData.AddSystemChat(SM_ChatChannel.SERVER, "#STR_SMP_01063", "#STR_SMP_00917", SM_ClanClientData.GetChatMuteMessage(), mutedColor, mutedColor);
			}
			else if (ShouldUseVanillaChatOnly(text))
			{
				SendVanillaChat(text);
			}
			else
			{
				Man player = GetGame().GetPlayer();
				if (player)
				{
					ScriptRPC rpc = new ScriptRPC();
					rpc.Write(channel);
					rpc.Write(text);
					rpc.Send(player, SM_PartyRPC.CHAT_SEND, true, NULL);
					SendCFToolsChatMirror(channel, text);
				}
			}
		}

		m_CloseTimer.Run(0.1, this, "Close");
		GetUApi().GetInputByID(UAPersonView).Supress();
		return true;
	}

	override bool OnKeyPress(Widget w, int x, int y, int key)
	{
		SM_VPPInputSuppressor.SuppressVPPAdminInputs();

		if (key == KeyCode.KC_ESCAPE)
		{
			Close();
			return true;
		}

		if (IsCtrlDown() && IsScaleUpKey(key))
		{
			m_ScaleUpWasDown = true;
			ChangeScale(SM_ClanClientData.ChatScaleStepPercent);
			return true;
		}

		if (IsCtrlDown() && IsScaleDownKey(key))
		{
			m_ScaleDownWasDown = true;
			ChangeScale(-SM_ClanClientData.ChatScaleStepPercent);
			return true;
		}

		if (key == KeyCode.KC_PRIOR)
		{
			m_HistoryUpWasDown = true;
			SM_ClanClientData.ScrollChatHistory(1);
			return true;
		}

		if (key == KeyCode.KC_NEXT)
		{
			m_HistoryDownWasDown = true;
			SM_ClanClientData.ScrollChatHistory(-1);
			return true;
		}

		return super.OnKeyPress(w, x, y, key);
	}

	override void Update(float timeslice)
	{
		SM_VPPInputSuppressor.SuppressVPPAdminInputs();
		super.Update(timeslice);

		UAInputAPI inputApi = GetUApi();
		if (inputApi)
		{
			UAInput backInput = inputApi.GetInputByID(UAUIBack);
			if (backInput && backInput.LocalPress())
			{
				Close();
				return;
			}
		}

		bool tabDown = (KeyState(KeyCode.KC_TAB) != 0);
		if (tabDown && !m_TabWasDown)
			CycleChannel();
		m_TabWasDown = tabDown;

		bool scaleUpDown = IsCtrlDown() && IsScaleUpPressed();
		if (scaleUpDown && !m_ScaleUpWasDown)
			ChangeScale(SM_ClanClientData.ChatScaleStepPercent);
		m_ScaleUpWasDown = scaleUpDown;

		bool scaleDownDown = IsCtrlDown() && IsScaleDownPressed();
		if (scaleDownDown && !m_ScaleDownWasDown)
			ChangeScale(-SM_ClanClientData.ChatScaleStepPercent);
		m_ScaleDownWasDown = scaleDownDown;

		bool historyUpDown = KeyState(KeyCode.KC_PRIOR) != 0;
		if (historyUpDown && !m_HistoryUpWasDown)
			SM_ClanClientData.ScrollChatHistory(1);
		m_HistoryUpWasDown = historyUpDown;

		bool historyDownDown = KeyState(KeyCode.KC_NEXT) != 0;
		if (historyDownDown && !m_HistoryDownWasDown)
			SM_ClanClientData.ScrollChatHistory(-1);
		m_HistoryDownWasDown = historyDownDown;

		RefreshInputText();
	}
}
