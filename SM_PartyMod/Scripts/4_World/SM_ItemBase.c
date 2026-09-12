modded class ItemBase
{
	override void OnInventoryEnter(Man player)
	{
		super.OnInventoryEnter(player);

		if (!GetGame().IsServer())
			return;

		PlayerBase pb = PlayerBase.Cast(player);
		if (!pb)
			return;

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager)
			manager.QueueCheckPlayerRestrictedItems(pb);
	}
}
