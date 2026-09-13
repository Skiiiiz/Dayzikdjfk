class ActionOpenDungeonTicket: ActionInteractBase
{
	void ActionOpenDungeonTicket()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
		m_Text = "#STR_SMD_TICKET_ACTION";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNone;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		SM_DungeonRewardTicket ticket = SM_DungeonRewardTicket.Cast(item);
		if (!ticket)
			return false;

		return ticket.HasRewardPayload();
	}

	override void OnEndServer(ActionData action_data)
	{
		super.OnEndServer(action_data);
		if (!action_data)
			return;

		SM_DungeonRewardTicket ticket = SM_DungeonRewardTicket.Cast(action_data.m_MainItem);
		if (ticket && action_data.m_Player)
			ticket.OpenTicket(action_data.m_Player);
	}
}
