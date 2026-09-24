modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		BBP_Manager.Init();
	}

	override void OnClientReadyEvent(PlayerIdentity identity, PlayerBase player)
	{
		super.OnClientReadyEvent(identity, player);

		BBP_Manager manager = BBP_Manager.Get();
		if (manager && player)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(manager.OnPlayerReady, 2000, false, player);
	}

	override void PlayerDisconnected(PlayerBase player, PlayerIdentity identity, string uid)
	{
		string plainUid = uid;
		if (identity)
			plainUid = identity.GetPlainId();

		super.PlayerDisconnected(player, identity, uid);

		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.OnPlayerDisconnected(plainUid);
	}

	override void OnMissionFinish()
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.Shutdown();

		super.OnMissionFinish();
	}
}
