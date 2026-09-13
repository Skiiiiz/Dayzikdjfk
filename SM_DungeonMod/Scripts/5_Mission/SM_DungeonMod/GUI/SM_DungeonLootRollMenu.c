// ============================================================================
// SM_DungeonLootRollMenu.c
//
// Небольшое модальное окно "Нужно / Не откажусь / Откажусь". Открывается и
// закрывается автоматически из modded MissionGameplay, пока у клиента есть
// хотя бы один активный ролл добычи, на который он ещё не ответил.
// ============================================================================

class SM_DungeonLootRollMenu extends UIScriptedMenu
{
	protected TextWidget m_ItemText;
	protected TextWidget m_TimerText;
	protected ButtonWidget m_NeedBtn;
	protected ButtonWidget m_GreedBtn;
	protected ButtonWidget m_PassBtn;

	protected string m_CurrentRollId;
	protected ref array<string> m_ChosenRollIds = new array<string>;

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("SM_DungeonMod/GUI/layouts/SM_DungeonLootRoll.layout");

		m_ItemText = TextWidget.Cast(layoutRoot.FindAnyWidget("SMDRollItemText"));
		m_TimerText = TextWidget.Cast(layoutRoot.FindAnyWidget("SMDRollTimerText"));
		m_NeedBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDRollNeedBtn"));
		m_GreedBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDRollGreedBtn"));
		m_PassBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDRollPassBtn"));

		return layoutRoot;
	}

	override int GetID()
	{
		return SM_DUNGEON_LOOTROLL_MENU_ID;
	}

	override bool UseMouse()
	{
		return true;
	}

	override bool UseKeyboard()
	{
		return false;
	}

	// Есть ли вообще активный ролл - используется modded MissionGameplay,
	// чтобы решить, нужно ли открыть меню (сам факт "уже ответил на этот
	// конкретный ролл" меню отслеживает внутри себя, пока открыто).
	static bool HasPendingRoll()
	{
		return SM_DungeonClientData.ActiveLootRolls.Count() > 0;
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		for (int i = m_ChosenRollIds.Count() - 1; i >= 0; i--)
		{
			bool stillActive = false;
			foreach (SM_DungeonLootRollView activeRoll : SM_DungeonClientData.ActiveLootRolls)
			{
				if (activeRoll.RollId == m_ChosenRollIds[i])
				{
					stillActive = true;
					break;
				}
			}
			if (!stillActive)
				m_ChosenRollIds.Remove(i);
		}

		SM_DungeonLootRollView pick = NULL;
		foreach (SM_DungeonLootRollView roll : SM_DungeonClientData.ActiveLootRolls)
		{
			if (m_ChosenRollIds.Find(roll.RollId) < 0)
			{
				pick = roll;
				break;
			}
		}

		if (!pick)
		{
			m_CurrentRollId = "";
			Close();
			return;
		}

		m_CurrentRollId = pick.RollId;
		m_ItemText.SetText(SM_DungeonLoc.Text("#STR_SMD_ROLL_ITEM") + ": " + pick.ItemClassName + " x" + pick.Quantity.ToString());
		m_TimerText.SetText(pick.SecondsRemaining.ToString());
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_CurrentRollId == "")
			return super.OnClick(w, x, y, button);

		int choice = -1;
		if (w == m_NeedBtn)
			choice = SM_DungeonLootChoice.NEED;
		else if (w == m_GreedBtn)
			choice = SM_DungeonLootChoice.GREED;
		else if (w == m_PassBtn)
			choice = SM_DungeonLootChoice.PASS;

		if (choice >= 0)
		{
			SendChoice(m_CurrentRollId, choice);
			m_ChosenRollIds.Insert(m_CurrentRollId);
			return true;
		}

		return super.OnClick(w, x, y, button);
	}

	protected void SendChoice(string rollId, int choice)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(rollId);
		rpc.Write(choice);
		rpc.Send(player, SM_DungeonRPC.LOOT_ROLL_CHOICE, true, NULL);
	}
}
