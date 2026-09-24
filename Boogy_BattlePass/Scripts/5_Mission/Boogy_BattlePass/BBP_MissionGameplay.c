modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		if (id == BBP_MENU_ID)
			return new BBP_Menu();

		return super.CreateScriptedMenu(id);
	}
}

modded class MissionGameplay
{
	protected bool m_BBPStateRequested;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		if (!m_BBPStateRequested)
		{
			m_BBPStateRequested = true;
			BBP_ClientData.Request(BBP_RPC.REQUEST_STATE);
		}

		UAInput menuInput = GetUApi().GetInputByName("UABBPMenu");
		if (!menuInput || !menuInput.LocalPress())
			return;

		BBP_Menu openMenu = BBP_Menu.Cast(GetGame().GetUIManager().FindMenu(BBP_MENU_ID));
		if (openMenu)
		{
			openMenu.CloseFromHotkey();
			return;
		}

		if (GetGame().GetUIManager().GetMenu())
			return;

		PlayerBase pb = PlayerBase.Cast(player);
		if (pb && pb.IsAlive())
			GetGame().GetUIManager().EnterScriptedMenu(BBP_MENU_ID, null);
	}
}

// Команды администратора в чате: /bp ...
modded class ChatInputMenu
{
	protected void BBP_CloseChatInput()
	{
		Close();
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (finished)
		{
			EditBoxWidget editBox = EditBoxWidget.Cast(w);
			if (editBox)
			{
				string text = editBox.GetText();
				text = text.Trim();
				string lower = text;
				lower.ToLower();
				if (lower == "/bp" || lower.IndexOf("/bp ") == 0)
				{
					BBP_ClientData.SendString(BBP_RPC.ADMIN_COMMAND, text);
					editBox.SetText("");
					GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.BBP_CloseChatInput, 100, false);
					return true;
				}
			}
		}

		return super.OnChange(w, x, y, finished);
	}
}
