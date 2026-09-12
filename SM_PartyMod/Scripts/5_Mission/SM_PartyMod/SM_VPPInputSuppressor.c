class SM_VPPInputSuppressor
{
	protected static bool s_VPPInputsChecked;
	protected static bool s_VPPInputsAvailable;

	static bool IsSMMenuOpen()
	{
		if (!GetGame())
			return false;

		if (!GetGame().GetUIManager())
			return false;

		UIScriptedMenu menu = GetGame().GetUIManager().GetMenu();
		if (!menu)
			return false;

		if (SM_ClanChatMenu.Cast(menu))
			return true;

		if (SM_ClanMenu.Cast(menu))
			return true;

		return false;
	}

	static void SuppressIfSMMenuOpen()
	{
		if (!IsSMMenuOpen())
			return;

		SuppressVPPAdminInputs();
	}

	static void SuppressVPPAdminInputs()
	{
		if (!HasVPPInputs())
			return;

		SuppressInputByName("UAToggleAdminTools");
		SuppressInputByName("UAOpenAdminTools");
		SuppressInputByName("UATogglePlayerControls");
		SuppressInputByName("UAToggleGodMode");
		SuppressInputByName("UAToggleInvis");
		SuppressInputByName("UATeleportToCrosshair");
		SuppressInputByName("UADeleteObjCrosshair");
		SuppressInputByName("UACollapseESPDropDwn");
		SuppressInputByName("UAToggleESP");
		SuppressInputByName("UAToggleMeshEsp");
		SuppressInputByName("UATogglePlayerDetailEsp");
		SuppressInputByName("UACopyPositionClipboard");
		SuppressInputByName("UARepairVehicleAtCrosshairs");
		SuppressInputByName("UAToggleFreeCam");
		SuppressInputByName("UAHealTargets");
		SuppressInputByName("UAToggleCmdConsole");
		SuppressInputByName("UASelectObject");
		SuppressInputByName("UADeSelectObject");
		SuppressInputByName("UARotateLeft");
		SuppressInputByName("UARotateRight");
		SuppressInputByName("UASupriseBind");
		SuppressInputByName("UACamForward");
		SuppressInputByName("UACamBackward");
		SuppressInputByName("UACamRight");
		SuppressInputByName("UACamLeft");
		SuppressInputByName("UACamUp");
		SuppressInputByName("UACamDown");
		SuppressInputByName("UACamTurbo");
		SuppressInputByName("UACamShiftLeft");
		SuppressInputByName("UACamShiftRight");
		SuppressInputByName("UACamShiftUp");
		SuppressInputByName("UACamShiftDown");
		SuppressInputByName("UACamSpeedAdd");
		SuppressInputByName("UACamSpeedDeduct");
		SuppressInputByName("UACamFOV");
		SuppressInputByName("UACamRelease");
		SuppressInputByName("UAFocusOnGame");
		SuppressInputByName("UAExitSpectate");
		SuppressInputByName("UAExecuteCommand");
		SuppressInputByName("UAUPCommand");
		SuppressInputByName("UADOWNCommand");
	}

	protected static bool HasVPPInputs()
	{
		if (s_VPPInputsChecked)
			return s_VPPInputsAvailable;

		if (!GetGame())
			return false;

		if (!GetGame().ConfigIsExisting("CfgPatches DZM_VPPAdminToolsScripts"))
		{
			s_VPPInputsChecked = true;
			s_VPPInputsAvailable = false;
			return false;
		}

		UAInputAPI api = GetUApi();
		if (!api)
			return false;

		s_VPPInputsChecked = true;
		s_VPPInputsAvailable = false;

		UAInput input = api.GetInputByName("UAToggleAdminTools");
		if (input)
			s_VPPInputsAvailable = true;

		return s_VPPInputsAvailable;
	}

	protected static void SuppressInputByName(string inputName)
	{
		UAInputAPI api = GetUApi();
		if (!api)
			return;

		UAInput input = api.GetInputByName(inputName);
		if (input)
			input.Supress();
	}
}
