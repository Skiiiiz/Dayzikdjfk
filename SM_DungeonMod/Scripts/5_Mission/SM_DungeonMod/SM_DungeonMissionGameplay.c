modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		if (id == SM_DUNGEON_MENU_ID)
			return new SM_DungeonMenu();
		if (id == SM_DUNGEON_LOOTROLL_MENU_ID)
			return new SM_DungeonLootRollMenu();

		return super.CreateScriptedMenu(id);
	}
}

modded class MissionGameplay
{
	protected ref SM_DungeonHud m_SMDHud;
	protected bool m_SMDHudInitDone;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (!m_SMDHudInitDone)
		{
			m_SMDHudInitDone = true;
			m_SMDHud = new SM_DungeonHud();
			m_SMDHud.Init();
		}
		if (m_SMDHud)
			m_SMDHud.Update(timeslice);

		UAInput dungeonMenuInput = GetUApi().GetInputByName("UASMDDungeonMenu");
		if (dungeonMenuInput && dungeonMenuInput.LocalPress())
		{
			SM_DungeonMenu openMenu = SM_DungeonMenu.Cast(GetGame().GetUIManager().FindMenu(SM_DUNGEON_MENU_ID));
			if (openMenu)
			{
				openMenu.Close();
			}
			else
			{
				Man preUpdatePlayer = GetGame().GetPlayer();
				if (preUpdatePlayer && !GetGame().GetUIManager().GetMenu())
				{
					PlayerBase menuPb = PlayerBase.Cast(preUpdatePlayer);
					if (menuPb && menuPb.IsAlive())
					{
						dungeonMenuInput.Supress();
						GetGame().GetUIManager().EnterScriptedMenu(SM_DUNGEON_MENU_ID, NULL);
					}
				}
			}
		}

		bool rollMenuOpen = GetGame().GetUIManager().FindMenu(SM_DUNGEON_LOOTROLL_MENU_ID) != NULL;
		if (!rollMenuOpen && SM_DungeonLootRollMenu.HasPendingRoll() && !GetGame().GetUIManager().GetMenu())
			GetGame().GetUIManager().EnterScriptedMenu(SM_DUNGEON_LOOTROLL_MENU_ID, NULL);
	}
}
