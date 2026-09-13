modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		SM_DungeonManager.Init();
	}

	override void OnEvent(EventType eventTypeId, Param params)
	{
		if (eventTypeId == ChatMessageEventTypeID)
		{
			ChatMessageEventParams chatParams = ChatMessageEventParams.Cast(params);
			if (chatParams)
			{
				SM_DungeonManager manager = SM_DungeonManager.Get();
				if (manager)
					manager.OnServerChatMessage(chatParams.param2, chatParams.param3);
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

		SM_DungeonManager manager = SM_DungeonManager.Get();
		if (manager)
			manager.OnPlayerDisconnected(plainUid);
	}
}
