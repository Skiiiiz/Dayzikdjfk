modded class TerritoryFlag
{
	bool SM_IsRaisedForClanBase()
	{
		if (IsDamageDestroyed())
			return false;
		if (!FindAttachmentBySlotName("Material_FPole_Flag"))
			return false;
		if (GetAnimationPhase("flag_mast") <= 0.05)
			return true;
		return false;
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionSetClanBase);
		AddAction(ActionUnsetClanBase);
	}
}
