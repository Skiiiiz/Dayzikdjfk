class SM_HudVisibility
{
	static bool IsGameHudHidden()
	{
		if (!g_Game)
			return false;

		Mission mission = g_Game.GetMission();
		if (!mission)
			return false;

		IngameHud hud = IngameHud.Cast(mission.GetHud());
		if (!hud)
			return false;

		IngameHudVisibility visibility = hud.GetHudVisibility();
		if (!visibility)
			return false;

		if (visibility.IsContextFlagActive(EHudContextFlags.HUD_DISABLE))
			return true;
		if (visibility.IsContextFlagActive(EHudContextFlags.HUD_HIDE))
			return true;
		if (visibility.IsContextFlagActive(EHudContextFlags.UNCONSCIOUS))
			return true;

		return false;
	}
}
