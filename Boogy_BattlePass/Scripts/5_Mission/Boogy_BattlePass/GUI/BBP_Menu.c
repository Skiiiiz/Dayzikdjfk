class BBP_Menu extends UIScriptedMenu
{
	protected const int TAB_REWARDS = 0;
	protected const int TAB_QUESTS  = 1;
	protected const int TAB_SHOP    = 2;

	protected int COLOR_TEXT;
	protected int COLOR_MUTED;
	protected int COLOR_AMBER;
	protected int COLOR_COPPER;
	protected int COLOR_GREEN;
	protected int COLOR_RED;
	protected int COLOR_BTN;
	protected int COLOR_BTN_ACTIVE;
	protected int COLOR_BTN_DISABLED;
	protected int COLOR_ROW;
	protected int COLOR_ROW_DONE;
	protected int COLOR_ROW_LOCKED;

	protected static int s_LastTab = 0;

	protected int m_Tab;
	protected Widget m_Content;
	protected ScrollWidget m_Scroll;
	protected TextWidget m_SeasonName;
	protected TextWidget m_SeasonTimer;
	protected TextWidget m_LevelText;
	protected Widget m_XpBarFill;
	protected TextWidget m_XpText;
	protected TextWidget m_PremiumBadge;
	protected TextWidget m_CoinsText;
	protected TextWidget m_EmptyText;
	protected TextWidget m_FooterText;
	protected ButtonWidget m_CloseButton;
	protected ButtonWidget m_TabRewards;
	protected ButtonWidget m_TabQuests;
	protected ButtonWidget m_TabShop;
	protected ButtonWidget m_ClaimAll;
	protected ButtonWidget m_AdminReload;
	protected float m_TimerAccum;

	protected ref array<Widget> m_Rows;
	// Кнопки строк -> данные
	protected ref map<Widget, int> m_ClaimFreeButtons;
	protected ref map<Widget, int> m_ClaimPremiumButtons;
	protected ref map<Widget, string> m_BuyButtons;
	protected ref map<Widget, string> m_OpenButtons;

	void BBP_Menu()
	{
		m_Rows = new array<Widget>;
		m_ClaimFreeButtons = new map<Widget, int>;
		m_ClaimPremiumButtons = new map<Widget, int>;
		m_BuyButtons = new map<Widget, string>;
		m_OpenButtons = new map<Widget, string>;

		COLOR_TEXT = ARGB(255, 238, 230, 217);
		COLOR_MUTED = ARGB(255, 179, 166, 146);
		COLOR_AMBER = ARGB(255, 232, 181, 69);
		COLOR_COPPER = ARGB(255, 184, 115, 51);
		COLOR_GREEN = ARGB(255, 127, 176, 105);
		COLOR_RED = ARGB(255, 200, 85, 61);
		COLOR_BTN = ARGB(255, 75, 64, 53);
		COLOR_BTN_ACTIVE = ARGB(255, 184, 115, 51);
		COLOR_BTN_DISABLED = ARGB(255, 42, 36, 30);
		COLOR_ROW = ARGB(217, 45, 38, 31);
		COLOR_ROW_DONE = ARGB(217, 35, 42, 28);
		COLOR_ROW_LOCKED = ARGB(217, 26, 23, 20);
	}

	void ~BBP_Menu()
	{
		BBP_ClientData.OnStateChanged.Remove(this.Refresh);
	}

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("Boogy_BattlePass/GUI/layouts/BBP_Menu.layout");

		m_Scroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("ContentScroll"));
		m_Content = layoutRoot.FindAnyWidget("Content");
		m_SeasonName = TextWidget.Cast(layoutRoot.FindAnyWidget("SeasonName"));
		m_SeasonTimer = TextWidget.Cast(layoutRoot.FindAnyWidget("SeasonTimer"));
		m_LevelText = TextWidget.Cast(layoutRoot.FindAnyWidget("LevelText"));
		m_XpBarFill = layoutRoot.FindAnyWidget("XpBarFill");
		m_XpText = TextWidget.Cast(layoutRoot.FindAnyWidget("XpText"));
		m_PremiumBadge = TextWidget.Cast(layoutRoot.FindAnyWidget("PremiumBadge"));
		m_CoinsText = TextWidget.Cast(layoutRoot.FindAnyWidget("CoinsText"));
		m_EmptyText = TextWidget.Cast(layoutRoot.FindAnyWidget("EmptyText"));
		m_FooterText = TextWidget.Cast(layoutRoot.FindAnyWidget("FooterText"));
		m_CloseButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseButton"));
		m_TabRewards = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabRewards"));
		m_TabQuests = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabQuests"));
		m_TabShop = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabShop"));
		m_ClaimAll = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ClaimAll"));
		m_AdminReload = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AdminReload"));

		m_Tab = s_LastTab;
		return layoutRoot;
	}

	override void OnShow()
	{
		super.OnShow();

		GetGame().GetInput().ChangeGameFocus(1);
		GetGame().GetUIManager().ShowUICursor(true);
		Mission mission = GetGame().GetMission();
		if (mission)
			mission.PlayerControlDisable(INPUT_EXCLUDE_ALL);

		BBP_ClientData.OnStateChanged.Insert(this.Refresh);
		BBP_ClientData.Request(BBP_RPC.REQUEST_STATE);
		Refresh();
	}

	override void OnHide()
	{
		super.OnHide();

		BBP_ClientData.OnStateChanged.Remove(this.Refresh);

		GetGame().GetInput().ResetGameFocus();
		GetGame().GetUIManager().ShowUICursor(false);
		Mission mission = GetGame().GetMission();
		if (mission)
			mission.PlayerControlEnable(false);
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (GetUApi().GetInputByID(UAUIBack).LocalPress())
		{
			Close();
			return;
		}

		m_TimerAccum += timeslice;
		if (m_TimerAccum >= 1.0)
		{
			m_TimerAccum = 0;
			UpdateTimer();
		}
	}

	void CloseFromHotkey()
	{
		Close();
	}

	// ------------------------------------------------------------
	//  Отрисовка
	// ------------------------------------------------------------

	void Refresh()
	{
		if (!layoutRoot)
			return;

		UpdateHeader();
		UpdateTabs();
		ClearRows();

		BBP_StateView state = BBP_ClientData.State;
		if (!state)
		{
			ShowEmpty("Загрузка данных…");
			return;
		}
		if (!state.Enabled)
		{
			ShowEmpty("Боевой пропуск отключён администрацией");
			return;
		}

		switch (m_Tab)
		{
			case TAB_REWARDS:
				BuildRewards(state);
				break;
			case TAB_QUESTS:
				BuildQuests(state);
				break;
			case TAB_SHOP:
				BuildShop(state);
				break;
		}
	}

	protected void UpdateHeader()
	{
		BBP_StateView state = BBP_ClientData.State;
		if (!state || !state.HasSeason)
		{
			m_SeasonName.SetText("Нет активного сезона");
			m_LevelText.SetText("");
			m_XpText.SetText("");
			m_XpBarFill.SetSize(0, 14);
			m_PremiumBadge.SetText("");
			if (state)
				m_CoinsText.SetText("Монеты: " + state.Coins);
			else
				m_CoinsText.SetText("");
			m_ClaimAll.Show(false);
			ShowButtonBg(m_ClaimAll, false);
			UpdateAdmin(state);
			UpdateTimer();
			return;
		}

		string seasonTitle = state.SeasonName;
		if (state.SeasonEnded)
			seasonTitle = seasonTitle + "  (завершён)";
		m_SeasonName.SetText(seasonTitle);

		m_LevelText.SetText("УРОВЕНЬ " + state.Level + " / " + state.MaxLevel);

		float progress = 1.0;
		string xpLine;
		if (state.Level < state.MaxLevel && state.LevelXpTo > state.LevelXpFrom)
		{
			int inLevel = state.Xp - state.LevelXpFrom;
			int levelSize = state.LevelXpTo - state.LevelXpFrom;
			float inLevelF = inLevel;
			progress = Math.Clamp(inLevelF / levelSize, 0, 1);
			xpLine = "Опыт: " + inLevel + " / " + levelSize + "   (всего " + state.Xp + ")";
		}
		else
		{
			xpLine = "Максимальный уровень достигнут  (всего " + state.Xp + " XP)";
		}
		m_XpBarFill.SetSize(360 * progress, 14);
		m_XpText.SetText(xpLine);

		if (state.Premium)
		{
			m_PremiumBadge.SetText("★ ПРЕМИУМ");
			m_PremiumBadge.SetColor(COLOR_AMBER);
		}
		else
		{
			m_PremiumBadge.SetText("БЕСПЛАТНЫЙ");
			m_PremiumBadge.SetColor(COLOR_MUTED);
		}
		m_CoinsText.SetText("Монеты: " + state.Coins);

		bool canClaim = state.UnclaimedRewards > 0;
		m_ClaimAll.Show(true);
		ShowButtonBg(m_ClaimAll, true);
		if (canClaim)
		{
			m_ClaimAll.SetText("ЗАБРАТЬ ВСЁ (" + state.UnclaimedRewards + ")");
			SetButtonBgColor(m_ClaimAll, COLOR_BTN_ACTIVE);
		}
		else
		{
			m_ClaimAll.SetText("ЗАБРАТЬ ВСЁ");
			SetButtonBgColor(m_ClaimAll, COLOR_BTN_DISABLED);
		}

		UpdateAdmin(state);
		UpdateTimer();
	}

	protected void UpdateAdmin(BBP_StateView state)
	{
		bool isAdmin = state && state.IsAdmin;
		m_AdminReload.Show(isAdmin);
		ShowButtonBg(m_AdminReload, isAdmin);
	}

	protected void UpdateTimer()
	{
		BBP_StateView state = BBP_ClientData.State;
		if (!state)
		{
			m_SeasonTimer.SetText("");
			m_FooterText.SetText("");
			return;
		}

		string timer = "";
		if (state.HasSeason && !state.SeasonEnded)
			timer = "До конца сезона: " + BBP_Time.FormatDuration(BBP_ClientData.GetSecondsLeft());
		else if (state.HasSeason && state.SeasonEnded)
			timer = "Сезон завершён — успейте забрать награды";

		if (state.NextSeasonName != "")
		{
			if (timer != "")
				timer = timer + "   •   ";
			timer = timer + "Следующий: " + state.NextSeasonName + " через " + BBP_Time.FormatDuration(BBP_ClientData.GetNextSeasonSeconds());
		}
		m_SeasonTimer.SetText(timer);

		string footer = "";
		if (state.HasSeason && !state.Premium)
			footer = "Премиум-пропуск открывает премиум-награды и задания. Активируйте его специальным предметом.";
		else if (state.HasSeason)
			footer = state.SeasonDescription;
		m_FooterText.SetText(footer);
	}

	protected void UpdateTabs()
	{
		SetButtonBgColor(m_TabRewards, COLOR_BTN);
		SetButtonBgColor(m_TabQuests, COLOR_BTN);
		SetButtonBgColor(m_TabShop, COLOR_BTN);

		BBP_StateView state = BBP_ClientData.State;
		bool shopVisible = state && state.ShopEnabled;
		m_TabShop.Show(shopVisible);
		ShowButtonBg(m_TabShop, shopVisible);
		if (!shopVisible && m_Tab == TAB_SHOP)
			m_Tab = TAB_REWARDS;

		switch (m_Tab)
		{
			case TAB_REWARDS:
				SetButtonBgColor(m_TabRewards, COLOR_BTN_ACTIVE);
				break;
			case TAB_QUESTS:
				SetButtonBgColor(m_TabQuests, COLOR_BTN_ACTIVE);
				break;
			case TAB_SHOP:
				SetButtonBgColor(m_TabShop, COLOR_BTN_ACTIVE);
				break;
		}
	}

	protected void ShowEmpty(string message)
	{
		m_EmptyText.SetText(message);
		m_EmptyText.Show(true);
	}

	protected void ClearRows()
	{
		foreach (Widget row : m_Rows)
		{
			if (row)
				row.Unlink();
		}
		m_Rows.Clear();
		m_ClaimFreeButtons.Clear();
		m_ClaimPremiumButtons.Clear();
		m_BuyButtons.Clear();
		m_OpenButtons.Clear();
		m_EmptyText.Show(false);
	}

	protected Widget AddRow(string layout, float y)
	{
		Widget row = GetGame().GetWorkspace().CreateWidgets(layout, m_Content);
		row.SetPos(0, y);
		m_Rows.Insert(row);
		return row;
	}

	protected void FinishRows(float totalHeight)
	{
		m_Content.SetSize(1036, Math.Max(486, totalHeight));
		if (m_Scroll)
			m_Scroll.Update();
	}

	protected void BuildRewards(BBP_StateView state)
	{
		if (!state.HasSeason || state.Levels.Count() == 0)
		{
			ShowEmpty("Сейчас нет активного сезона");
			return;
		}

		float y = 0;
		float scrollTo = -1;
		foreach (BBP_LevelView lv : state.Levels)
		{
			Widget row = AddRow("Boogy_BattlePass/GUI/layouts/BBP_LevelRow.layout", y);
			bool reached = lv.Level <= state.Level;

			TextWidget.Cast(row.FindAnyWidget("LevelNum")).SetText(lv.Level.ToString());
			TextWidget.Cast(row.FindAnyWidget("LevelXp")).SetText(lv.XpTo.ToString() + " XP");
			TextWidget.Cast(row.FindAnyWidget("FreeRewards")).SetText(DescribeRewards(lv.Free));
			TextWidget.Cast(row.FindAnyWidget("PremiumRewards")).SetText(DescribeRewards(lv.Premium));

			Widget accent = row.FindAnyWidget("LevelAccent");
			Widget rowBg = row.FindAnyWidget("RowBg");
			if (reached)
			{
				accent.SetColor(COLOR_AMBER);
				rowBg.SetColor(COLOR_ROW);
			}
			else
			{
				accent.SetColor(COLOR_BTN);
				rowBg.SetColor(COLOR_ROW_LOCKED);
			}

			ButtonWidget freeBtn = ButtonWidget.Cast(row.FindAnyWidget("FreeClaim"));
			ButtonWidget premiumBtn = ButtonWidget.Cast(row.FindAnyWidget("PremiumClaim"));
			SetupClaimButton(row, freeBtn, lv.Free.Count() > 0, reached, true, lv.FreeClaimed);
			SetupClaimButton(row, premiumBtn, lv.Premium.Count() > 0, reached, state.Premium, lv.PremiumClaimed);

			if (lv.Free.Count() > 0 && reached && !lv.FreeClaimed)
				m_ClaimFreeButtons.Set(freeBtn, lv.Level);
			if (lv.Premium.Count() > 0 && reached && state.Premium && !lv.PremiumClaimed)
				m_ClaimPremiumButtons.Set(premiumBtn, lv.Level);

			if (scrollTo < 0 && lv.Level == state.Level + 1)
				scrollTo = Math.Max(0, y - 152);

			y = y + 76;
		}

		FinishRows(y);
		if (m_Scroll && scrollTo > 0)
			m_Scroll.VScrollToPos(scrollTo);
	}

	protected void SetupClaimButton(Widget row, ButtonWidget btn, bool hasRewards, bool reached, bool allowed, bool claimed)
	{
		if (!btn)
			return;

		if (!hasRewards)
		{
			btn.Show(false);
			ShowButtonBg(btn, false);
			return;
		}

		if (claimed)
		{
			btn.SetText("ПОЛУЧЕНО");
			SetButtonBgColor(btn, COLOR_ROW_DONE);
		}
		else if (!allowed)
		{
			btn.SetText("ПРЕМИУМ");
			SetButtonBgColor(btn, COLOR_BTN_DISABLED);
		}
		else if (!reached)
		{
			btn.SetText("ЗАКРЫТО");
			SetButtonBgColor(btn, COLOR_BTN_DISABLED);
		}
		else
		{
			btn.SetText("ЗАБРАТЬ");
			SetButtonBgColor(btn, COLOR_BTN_ACTIVE);
		}
	}

	protected void BuildQuests(BBP_StateView state)
	{
		if (!state.HasSeason || state.Quests.Count() == 0)
		{
			ShowEmpty("Нет доступных заданий");
			return;
		}

		float y = 0;
		// Сначала активные, затем выполненные, в конце — заблокированные
		for (int pass = 0; pass < 3; pass++)
		{
			foreach (BBP_QuestView qv : state.Quests)
			{
				int group = 0;
				if (qv.Locked)
					group = 2;
				else if (qv.Completed)
					group = 1;
				if (group != pass)
					continue;

				Widget row = AddRow("Boogy_BattlePass/GUI/layouts/BBP_QuestRow.layout", y);
				FillQuestRow(row, qv);
				y = y + 74;
			}
		}

		FinishRows(y);
	}

	protected void FillQuestRow(Widget row, BBP_QuestView qv)
	{
		string tag = GetQuestTypeName(qv.Type);
		if (qv.Period == BBP_Period.DAILY)
			tag = tag + "  •  ЕЖЕДНЕВНОЕ";
		else if (qv.Period == BBP_Period.WEEKLY)
			tag = tag + "  •  ЕЖЕНЕДЕЛЬНОЕ";
		if (qv.PremiumOnly)
			tag = tag + "  •  ★ ПРЕМИУМ";

		TextWidget.Cast(row.FindAnyWidget("QuestTag")).SetText(tag);
		TextWidget.Cast(row.FindAnyWidget("QuestName")).SetText(qv.Name);
		TextWidget.Cast(row.FindAnyWidget("QuestDesc")).SetText(qv.Description);

		float progress = 0;
		if (qv.Count > 0)
		{
			float questValue = qv.Value;
			progress = Math.Clamp(questValue / qv.Count, 0, 1);
		}
		row.FindAnyWidget("ProgressFill").SetSize(250 * progress, 12);
		TextWidget.Cast(row.FindAnyWidget("ProgressText")).SetText(qv.Value.ToString() + " / " + qv.Count.ToString());

		string reward = "+" + qv.Xp + " XP";
		if (qv.Coins > 0)
			reward = reward + "  +" + qv.Coins + " мон.";
		if (qv.Rewards.Count() > 0)
			reward = reward + "  +" + qv.Rewards.Count() + " нагр.";
		TextWidget.Cast(row.FindAnyWidget("RewardText")).SetText(reward);

		TextWidget status = TextWidget.Cast(row.FindAnyWidget("StatusText"));
		Widget rowBg = row.FindAnyWidget("RowBg");
		Widget accent = row.FindAnyWidget("RowAccent");
		if (qv.Locked)
		{
			status.SetText("Нужен премиум");
			status.SetColor(COLOR_RED);
			rowBg.SetColor(COLOR_ROW_LOCKED);
			accent.SetColor(COLOR_BTN);
		}
		else if (qv.Completed)
		{
			status.SetText("✔ Выполнено");
			status.SetColor(COLOR_GREEN);
			rowBg.SetColor(COLOR_ROW_DONE);
			accent.SetColor(COLOR_GREEN);
			row.FindAnyWidget("ProgressFill").SetColor(COLOR_GREEN);
		}
		else
		{
			status.SetText("В процессе");
			status.SetColor(COLOR_MUTED);
		}
	}

	protected string GetQuestTypeName(string type)
	{
		switch (type)
		{
			case "Kill":   return "УБИЙСТВА";
			case "Action": return "ДЕЙСТВИЕ";
			case "Craft":  return "КРАФТ";
			case "Visit":  return "ЛОКАЦИИ";
			case "Custom": return "СОБЫТИЕ";
		}
		return type;
	}

	protected void BuildShop(BBP_StateView state)
	{
		if (state.Cases.Count() == 0)
		{
			ShowEmpty("Магазин пуст");
			return;
		}

		float y = 0;
		foreach (BBP_CaseView cv : state.Cases)
		{
			Widget row = AddRow("Boogy_BattlePass/GUI/layouts/BBP_CaseRow.layout", y);

			string title = cv.Name;
			if (cv.PremiumOnly)
				title = title + "  ★";
			TextWidget.Cast(row.FindAnyWidget("CaseName")).SetText(title);
			TextWidget.Cast(row.FindAnyWidget("CaseDesc")).SetText(cv.Description);

			string contents = "Содержимое: ";
			for (int i = 0; i < cv.Rewards.Count(); i++)
			{
				if (i > 0)
					contents = contents + ",  ";
				contents = contents + BBP_ClientData.DescribeReward(cv.Rewards[i]) + " (" + FormatChance(cv.Rewards[i].Chance) + ")";
			}
			TextWidget.Cast(row.FindAnyWidget("CaseContents")).SetText(contents);

			string price;
			if (cv.Currency == BBP_Currency.ITEM)
				price = "Цена: " + cv.Price + " x " + BBP_ClientData.GetItemDisplayName(cv.CurrencyItem);
			else
				price = "Цена: " + cv.Price + " монет";
			TextWidget.Cast(row.FindAnyWidget("CasePrice")).SetText(price);
			TextWidget.Cast(row.FindAnyWidget("CaseOwned")).SetText("У вас: " + cv.Owned);

			ButtonWidget buyBtn = ButtonWidget.Cast(row.FindAnyWidget("BuyButton"));
			ButtonWidget openBtn = ButtonWidget.Cast(row.FindAnyWidget("OpenButton"));

			bool canBuy = !cv.PremiumOnly || state.Premium;
			if (cv.Currency != BBP_Currency.ITEM && state.Coins < cv.Price)
				canBuy = false;
			if (canBuy)
				SetButtonBgColor(buyBtn, COLOR_BTN_ACTIVE);
			else
				SetButtonBgColor(buyBtn, COLOR_BTN_DISABLED);
			if (cv.PremiumOnly && !state.Premium)
				buyBtn.SetText("ПРЕМИУМ");
			m_BuyButtons.Set(buyBtn, cv.Id);

			if (cv.Owned > 0)
			{
				SetButtonBgColor(openBtn, COLOR_AMBER);
				openBtn.SetText("ОТКРЫТЬ (" + cv.Owned + ")");
				m_OpenButtons.Set(openBtn, cv.Id);
			}
			else
			{
				SetButtonBgColor(openBtn, COLOR_BTN_DISABLED);
			}

			y = y + 124;
		}

		FinishRows(y);
	}

	protected string FormatChance(float chance)
	{
		if (chance >= 10)
		{
			int whole = Math.Round(chance);
			return whole.ToString() + "%";
		}
		float rounded = Math.Round(chance * 10) / 10;
		return rounded.ToString() + "%";
	}

	protected string DescribeRewards(array<ref BBP_RewardView> rewards)
	{
		if (!rewards || rewards.Count() == 0)
			return "—";

		string result = "";
		foreach (BBP_RewardView reward : rewards)
		{
			if (result != "")
				result = result + ", ";
			result = result + BBP_ClientData.DescribeReward(reward);
		}
		return result;
	}

	// ------------------------------------------------------------
	//  Кнопки
	// ------------------------------------------------------------

	protected Widget GetButtonBg(Widget btn)
	{
		if (!btn || !btn.GetParent())
			return null;
		return btn.GetParent().FindAnyWidget(btn.GetName() + "Bg");
	}

	protected void SetButtonBgColor(Widget btn, int color)
	{
		Widget bg = GetButtonBg(btn);
		if (bg)
			bg.SetColor(color);
	}

	protected void ShowButtonBg(Widget btn, bool show)
	{
		Widget bg = GetButtonBg(btn);
		if (bg)
			bg.Show(show);
	}

	protected void SwitchTab(int tab)
	{
		m_Tab = tab;
		s_LastTab = tab;
		Refresh();
		if (m_Scroll)
			m_Scroll.VScrollToPos(0);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != MouseState.LEFT)
			return false;

		if (w == m_CloseButton)
		{
			Close();
			return true;
		}
		if (w == m_TabRewards)
		{
			SwitchTab(TAB_REWARDS);
			return true;
		}
		if (w == m_TabQuests)
		{
			SwitchTab(TAB_QUESTS);
			return true;
		}
		if (w == m_TabShop)
		{
			SwitchTab(TAB_SHOP);
			return true;
		}
		if (w == m_ClaimAll)
		{
			BBP_ClientData.Request(BBP_RPC.CLAIM_ALL);
			return true;
		}
		if (w == m_AdminReload)
		{
			BBP_ClientData.Request(BBP_RPC.ADMIN_RELOAD);
			return true;
		}

		int level;
		if (m_ClaimFreeButtons.Find(w, level))
		{
			BBP_ClientData.SendClaim(level, false);
			return true;
		}
		if (m_ClaimPremiumButtons.Find(w, level))
		{
			BBP_ClientData.SendClaim(level, true);
			return true;
		}

		string caseId;
		if (m_BuyButtons.Find(w, caseId))
		{
			BBP_ClientData.SendString(BBP_RPC.SHOP_BUY, caseId);
			return true;
		}
		if (m_OpenButtons.Find(w, caseId))
		{
			BBP_ClientData.SendString(BBP_RPC.CASE_OPEN, caseId);
			return true;
		}

		return super.OnClick(w, x, y, button);
	}
}
