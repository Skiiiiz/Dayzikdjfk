class ActionSetClanBase: ActionInteractBase
{
	void ActionSetClanBase()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_Text = "#STR_SMP_00906";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!target)
			return false;

		TerritoryFlag flag = TerritoryFlag.Cast(target.GetObject());
		if (!flag)
			return false;
		if (flag.IsDamageDestroyed())
			return false;
		if (!flag.SM_IsRaisedForClanBase())
			return false;

		if (GetGame().IsClient())
		{
			if (!SM_ClanClientData.StorageEnabled)
				return false;
			if (!SM_ClanClientData.HasClan)
				return false;
			if (SM_ClanClientData.HasBase && vector.Distance(flag.GetPosition(), SM_ClanClientData.BasePos) <= 2.0)
				return false;
			if (SM_ClanClientData.MyRank < SM_ClanClientData.GetActionRank(SM_ClanAction.SET_BASE))
				return false;
		}

		return true;
	}

	override void OnEndServer(ActionData action_data)
	{
		super.OnEndServer(action_data);
		if (!action_data || !action_data.m_Target)
			return;

		TerritoryFlag flag = TerritoryFlag.Cast(action_data.m_Target.GetObject());
		if (flag && action_data.m_Player)
			SM_ClanManager.Get().SetClanBaseFromFlag(action_data.m_Player, flag);
	}
}
