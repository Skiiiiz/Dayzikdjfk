// ============================================================
//  Хуки игровых событий -> прогресс заданий
// ============================================================

modded class ZombieBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame().IsServer())
			return;

		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.OnEntityKilled(this, killer, BBP_KillKind.ZOMBIE);
	}
}

modded class AnimalBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame().IsServer())
			return;

		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.OnEntityKilled(this, killer, BBP_KillKind.ANIMAL);
	}
}

modded class PlayerBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame().IsServer())
			return;

		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.OnEntityKilled(this, killer, BBP_KillKind.PLAYER);
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type < BBP_RPC.RANGE_START || rpc_type > BBP_RPC.RANGE_END)
			return;

		if (rpc_type <= BBP_RPC.LAST_CLIENT_RPC)
		{
			if (!GetGame().IsServer())
				return;
			BBP_Manager manager = BBP_Manager.Get();
			if (manager)
				manager.OnPlayerRPC(this, sender, rpc_type, ctx);
			return;
		}

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
			BBP_ClientData.OnRPC(rpc_type, ctx);
	}
}

// Одиночные действия и взаимодействия: засчитываем, если действие действительно выполнилось.
modded class ActionBase
{
	override void OnEndServer(ActionData action_data)
	{
		super.OnEndServer(action_data);

		if (!action_data || !action_data.m_WasExecuted)
			return;
		// Продолжительные действия считаются в ActionContinuousBase.OnFinishProgress
		if (ActionContinuousBase.Cast(this))
			return;

		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.OnActionDone(action_data.m_Player, this, action_data.m_MainItem);
	}
}

// Продолжительные действия (перевязка, розжиг, строительство, еда…): каждый завершённый цикл = +1.
modded class ActionContinuousBase
{
	override void OnFinishProgress(ActionData action_data)
	{
		super.OnFinishProgress(action_data);

		if (!GetGame().IsServer() || !action_data)
			return;

		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.OnActionDone(action_data.m_Player, this, action_data.m_MainItem);
	}
}

// Крафт по рецептам
modded class RecipeBase
{
	override void PerformRecipe(ItemBase item1, ItemBase item2, PlayerBase player)
	{
		super.PerformRecipe(item1, item2, player);

		if (!GetGame().IsServer() || !player)
			return;

		BBP_Manager manager = BBP_Manager.Get();
		if (!manager)
			return;

		array<string> results = new array<string>;
		for (int i = 0; i < m_NumberOfResults; i++)
		{
			if (m_ItemsToCreate[i] != "")
				results.Insert(m_ItemsToCreate[i]);
		}
		manager.OnCraft(player, ClassName(), results);
	}
}

// Действие активации премиум-пропуска (для любых классов из Settings.json -> PremiumItems)
class ActionBBPActivatePremium : ActionSingleUseBase
{
	void ActionBBPActivatePremium()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_ITEM_ON;
		m_CommandUIDProne = DayZPlayerConstants.CMD_ACTIONFB_ITEM_ON;
		m_Text = "Активировать боевой пропуск";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNone;
	}

	override bool HasTarget()
	{
		return false;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!item)
			return false;
		return BBP_Shared.IsPremiumItemClass(item.GetType());
	}

	override void OnExecuteServer(ActionData action_data)
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.OnPremiumItemUsed(action_data.m_Player, action_data.m_MainItem);
	}
}

modded class ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionBBPActivatePremium);
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(ActionBBPActivatePremium);
	}
}
