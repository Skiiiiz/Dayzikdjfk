modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type < SM_PartyRPC.RANGE_START || rpc_type > SM_PartyRPC.RANGE_END)
			return;

		if (rpc_type < SM_PartyRPC.SYNC_STATE || rpc_type == SM_PartyRPC.ADMIN_RELOAD_CONFIG || rpc_type == SM_PartyRPC.REQUEST_PLAYER_TITLES || rpc_type == SM_PartyRPC.CONTRACTS || rpc_type == SM_PartyRPC.ADMIN_CHAT_COMMAND || rpc_type == SM_PartyRPC.REQUEST_ADMIN_CHAT_LOG || rpc_type == SM_PartyRPC.ADMIN_CHAT_MUTE || rpc_type == SM_PartyRPC.ADMIN_CHAT_UNMUTE || rpc_type == SM_PartyRPC.REQUEST_ONLINE_PLAYERS || rpc_type == SM_PartyRPC.MAP_MARKER_UPDATE || rpc_type == SM_PartyRPC.MARK_ITEM)
		{
			SM_ClanManager manager = SM_ClanManager.Get();
			if (manager && GetGame().IsServer())
				manager.OnPlayerRPC(this, sender, rpc_type, ctx);
			return;
		}

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
			SM_ClanClientData.OnRPC(rpc_type, ctx);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame().IsServer())
			return;

		SM_ClanManager manager = SM_ClanManager.Get();
		if (!manager)
			return;

		manager.OnPlayerDeathMarker(this);
		manager.OnStatPlayerKilled(this, killer);
		manager.OnStatMemberDied(this);
	}

	override void EEItemAttached(EntityAI item, string slot_name)
	{
		super.EEItemAttached(item, slot_name);

		if (!GetGame().IsServer())
			return;

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager)
			manager.QueueCheckPlayerRestrictedItems(this);
	}

	override void EEItemIntoHands(EntityAI item)
	{
		super.EEItemIntoHands(item);

		if (!GetGame().IsServer())
			return;

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager)
			manager.QueueCheckPlayerRestrictedItems(this);
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!GetGame().IsServer() || damageType != DamageType.FIRE_ARM)
			return;

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager)
			manager.OnStatHit(this, source, dmgZone);
	}
}
