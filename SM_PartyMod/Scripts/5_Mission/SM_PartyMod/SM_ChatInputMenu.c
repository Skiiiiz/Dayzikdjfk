modded class ChatInputMenu
{
	protected void SM_SendAdminReloadConfigRpc()
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(player, SM_PartyRPC.ADMIN_RELOAD_CONFIG, true, NULL);
	}

	protected void SM_SendAdminChatCommandRpc(string text)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(text);
		rpc.Send(player, SM_PartyRPC.ADMIN_CHAT_COMMAND, true, NULL);
	}

	protected void SM_CloseChatInput()
	{
		Close();
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (!finished)
			return super.OnChange(w, x, y, finished);

		EditBoxWidget editBox = EditBoxWidget.Cast(w);
		if (!editBox)
			return super.OnChange(w, x, y, finished);

		string text = editBox.GetText();
		if (SM_PartyAdminCommands.IsReloadConfigCommand(text))
		{
			SM_SendAdminReloadConfigRpc();
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.SM_CloseChatInput, 100, false);
			GetUApi().GetInputByID(UAPersonView).Supress();
			return true;
		}

		if (SM_PartyAdminCommands.IsChatAdminCommand(text))
		{
			SM_SendAdminChatCommandRpc(text);
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.SM_CloseChatInput, 100, false);
			GetUApi().GetInputByID(UAPersonView).Supress();
			return true;
		}

		if (SM_ClanClientData.IsChatMuted())
		{
			int mutedColor = SM_ClanClientData.ChatColorAlert;
			if (mutedColor == 0)
				mutedColor = ARGB(255, 255, 110, 95);
			SM_ClanClientData.AddSystemChat(SM_ChatChannel.SERVER, "#STR_SMP_01063", "#STR_SMP_00917", SM_ClanClientData.GetChatMuteMessage(), mutedColor, mutedColor);
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.SM_CloseChatInput, 100, false);
			GetUApi().GetInputByID(UAPersonView).Supress();
			return true;
		}

		return super.OnChange(w, x, y, finished);
	}
}
