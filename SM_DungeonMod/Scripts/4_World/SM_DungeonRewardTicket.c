// Наградной билет: выдаётся победителю ролла добычи (или напрямую, если
// для конкретного предмета ролл выключен в конфиге). Содержимое хранится
// прямо на предмете (OnStoreSave/OnStoreLoad), никакого отдельного реестра
// на сервере не требуется - билет остаётся честным описанием самого себя
// даже если менеджер данжей перезапустится.
class SM_DungeonRewardTicket extends ItemBase
{
	protected ref array<ref SM_DungeonTicketLootEntry> m_Loot;

	void SM_DungeonRewardTicket()
	{
		m_Loot = new array<ref SM_DungeonTicketLootEntry>;
	}

	void SetLootPayload(array<ref SM_DungeonTicketLootEntry> loot)
	{
		m_Loot.Clear();
		if (!loot)
			return;
		foreach (SM_DungeonTicketLootEntry entry : loot)
		{
			if (entry)
				m_Loot.Insert(entry);
		}
	}

	bool HasRewardPayload()
	{
		return m_Loot && m_Loot.Count() > 0;
	}

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);

		ctx.Write(m_Loot.Count());
		foreach (SM_DungeonTicketLootEntry entry : m_Loot)
		{
			ctx.Write(entry.ClassName);
			ctx.Write(entry.Quantity);
		}
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		int count;
		if (!ctx.Read(count))
			return true; // старое сохранение без нашего блока данных - не критично

		m_Loot.Clear();
		for (int i = 0; i < count; i++)
		{
			string className;
			int quantity;
			if (!ctx.Read(className))
				return false;
			if (!ctx.Read(quantity))
				return false;
			m_Loot.Insert(new SM_DungeonTicketLootEntry(className, quantity));
		}

		return true;
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionOpenDungeonTicket);
	}

	// Раскрывает билет: пытается выдать каждый предмет в инвентарь игрока,
	// при нехватке места - кладёт под ноги. Билет уничтожается в любом случае.
	void OpenTicket(PlayerBase player)
	{
		if (!GetGame().IsServer())
			return;
		if (!player || !HasRewardPayload())
			return;

		foreach (SM_DungeonTicketLootEntry entry : m_Loot)
		{
			if (!entry || entry.ClassName == "")
				continue;

			int quantity = entry.Quantity;
			if (quantity < 1)
				quantity = 1;

			for (int i = 0; i < quantity; i++)
			{
				EntityAI spawned = player.GetInventory().CreateInInventory(entry.ClassName);
				if (!spawned)
					GetGame().CreateObject(entry.ClassName, player.GetPosition());
			}
		}

		m_Loot.Clear();
		Delete();
	}
}
