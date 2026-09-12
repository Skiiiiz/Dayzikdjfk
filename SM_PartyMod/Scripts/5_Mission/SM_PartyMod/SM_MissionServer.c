modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		SM_ClanManager.Init();
	}

	override void OnClientReadyEvent(PlayerIdentity identity, PlayerBase player)
	{
		super.OnClientReadyEvent(identity, player);

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager && player)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(manager.OnPlayerReady, 3000, false, player);
		}
	}

	override void OnEvent(EventType eventTypeId, Param params)
	{
		if (eventTypeId == ChatMessageEventTypeID)
		{
			ChatMessageEventParams chatParams = ChatMessageEventParams.Cast(params);
			if (chatParams)
			{
				SM_ClanManager chatManager = SM_ClanManager.Get();
				if (chatManager)
					chatManager.OnServerChatMessage(chatParams.param2, chatParams.param3);
			}
		}

		super.OnEvent(eventTypeId, params);
	}

	override void PlayerDisconnected(PlayerBase player, PlayerIdentity identity, string uid)
	{
		string plainUid = uid;
		if (identity)
			plainUid = identity.GetPlainId();

		super.PlayerDisconnected(player, identity, uid);

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager)
			manager.OnPlayerDisconnected(plainUid);
	}
}
