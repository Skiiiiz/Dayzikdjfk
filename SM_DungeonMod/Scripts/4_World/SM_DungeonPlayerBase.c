modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type < SM_DungeonRPC.RANGE_START || rpc_type > SM_DungeonRPC.RANGE_END)
			return;

		if (rpc_type < SM_DungeonRPC.SYNC_STATE)
		{
			SM_DungeonManager manager = SM_DungeonManager.Get();
			if (manager && GetGame().IsServer())
				manager.OnPlayerRPC(this, sender, rpc_type, ctx);
			return;
		}

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
			SM_DungeonClientData.OnRPC(rpc_type, ctx);
	}
}
