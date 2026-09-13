// ============================================================================
// SM_DungeonMenu.c
//
// Личный кабинет игрока: список данжей с выбором сложности/режима/времени
// и записью, мои брони (отмена/выход/приглашения/выгнать), приглашения,
// поиск спутников, и вкладка администратора (перезагрузка конфига,
// включение/выключение данжей, активные забеги, штрафы игроков).
// ============================================================================

class SM_DungeonMenu extends UIScriptedMenu
{
	protected ButtonWidget m_CloseBtn;
	protected ButtonWidget m_TabDungeonsBtn;
	protected ButtonWidget m_TabBookingsBtn;
	protected ButtonWidget m_TabInvitesBtn;
	protected ButtonWidget m_TabFinderBtn;
	protected ButtonWidget m_TabAdminBtn;
	protected TextWidget m_StatusText;

	protected Widget m_PanelDungeons;
	protected TextListboxWidget m_DungeonsList;
	protected TextWidget m_DifficultyValueText;
	protected ButtonWidget m_DifficultyPrevBtn;
	protected ButtonWidget m_DifficultyNextBtn;
	protected TextWidget m_ModeValueText;
	protected ButtonWidget m_ModePrevBtn;
	protected ButtonWidget m_ModeNextBtn;
	protected TextWidget m_SlotValueText;
	protected ButtonWidget m_SlotPrevBtn;
	protected ButtonWidget m_SlotNextBtn;
	protected MultilineEditBoxWidget m_FinderCommentEdit;
	protected ButtonWidget m_RegisterBtn;
	protected ButtonWidget m_AdminToggleBtn;
	protected TextWidget m_DescriptionText;

	protected Widget m_PanelBookings;
	protected TextListboxWidget m_BookingsList;
	protected TextWidget m_BookingDetailsText;
	protected ButtonWidget m_CancelBtn;
	protected ButtonWidget m_LeaveBtn;
	protected ButtonWidget m_InviteBtn;
	protected ButtonWidget m_KickBtn;
	protected TextListboxWidget m_OnlinePlayersList;
	protected ButtonWidget m_SendInviteBtn;

	protected Widget m_PanelInvites;
	protected TextListboxWidget m_InvitesList;
	protected ButtonWidget m_AcceptBtn;
	protected ButtonWidget m_DeclineBtn;

	protected Widget m_PanelFinder;
	protected TextListboxWidget m_FinderList;
	protected ButtonWidget m_JoinBtn;
	protected ButtonWidget m_UnlistBtn;
	protected ButtonWidget m_RefreshFinderBtn;

	protected Widget m_PanelAdmin;
	protected ButtonWidget m_AdminSubRunsBtn;
	protected ButtonWidget m_AdminSubPenaltiesBtn;
	protected ButtonWidget m_AdminReloadBtn;
	protected TextListboxWidget m_AdminList;
	protected ButtonWidget m_AdminActionBtn;
	protected ButtonWidget m_AdminAction2Btn;

	protected int m_CurrentTab;
	protected int m_SelectedDungeonRow = -1;
	protected int m_SelectedDifficultyIndex;
	protected int m_SelectedModeIndex;
	protected int m_SelectedSlotIndex;
	protected int m_SelectedBookingRow = -1;
	protected int m_SelectedInviteRow = -1;
	protected int m_SelectedFinderRow = -1;
	protected int m_SelectedOnlinePlayerRow = -1;
	protected int m_SelectedAdminRow = -1;
	protected int m_AdminSubView;
	protected int m_PlayerPickerMode; // 0 = приглашение, 1 = выгнать
	protected bool m_PlayerPickerOpen;
	protected float m_StatusTextTimer;
	protected string m_LastSeenNotify;

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("SM_DungeonMod/GUI/layouts/SM_DungeonMenu.layout");

		m_CloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDCloseBtn"));
		m_TabDungeonsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDTabDungeonsBtn"));
		m_TabBookingsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDTabBookingsBtn"));
		m_TabInvitesBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDTabInvitesBtn"));
		m_TabFinderBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDTabFinderBtn"));
		m_TabAdminBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDTabAdminBtn"));
		m_StatusText = TextWidget.Cast(layoutRoot.FindAnyWidget("SMDStatusText"));

		m_PanelDungeons = layoutRoot.FindAnyWidget("SMDPanelDungeons");
		m_DungeonsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("SMDDungeonsList"));
		m_DifficultyValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("SMDDifficultyValueText"));
		m_DifficultyPrevBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDDifficultyPrevBtn"));
		m_DifficultyNextBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDDifficultyNextBtn"));
		m_ModeValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("SMDModeValueText"));
		m_ModePrevBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDModePrevBtn"));
		m_ModeNextBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDModeNextBtn"));
		m_SlotValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("SMDSlotValueText"));
		m_SlotPrevBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDSlotPrevBtn"));
		m_SlotNextBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDSlotNextBtn"));
		m_FinderCommentEdit = MultilineEditBoxWidget.Cast(layoutRoot.FindAnyWidget("SMDFinderCommentEdit"));
		m_RegisterBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDRegisterBtn"));
		m_AdminToggleBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDAdminToggleBtn"));
		m_DescriptionText = TextWidget.Cast(layoutRoot.FindAnyWidget("SMDDescriptionText"));

		m_PanelBookings = layoutRoot.FindAnyWidget("SMDPanelBookings");
		m_BookingsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("SMDBookingsList"));
		m_BookingDetailsText = TextWidget.Cast(layoutRoot.FindAnyWidget("SMDBookingDetailsText"));
		m_CancelBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDCancelBtn"));
		m_LeaveBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDLeaveBtn"));
		m_InviteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDInviteBtn"));
		m_KickBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDKickBtn"));
		m_OnlinePlayersList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("SMDOnlinePlayersList"));
		m_SendInviteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDSendInviteBtn"));

		m_PanelInvites = layoutRoot.FindAnyWidget("SMDPanelInvites");
		m_InvitesList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("SMDInvitesList"));
		m_AcceptBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDAcceptBtn"));
		m_DeclineBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDDeclineBtn"));

		m_PanelFinder = layoutRoot.FindAnyWidget("SMDPanelFinder");
		m_FinderList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("SMDFinderList"));
		m_JoinBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDJoinBtn"));
		m_UnlistBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDUnlistBtn"));
		m_RefreshFinderBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDRefreshFinderBtn"));

		m_PanelAdmin = layoutRoot.FindAnyWidget("SMDPanelAdmin");
		m_AdminSubRunsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDAdminSubRunsBtn"));
		m_AdminSubPenaltiesBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDAdminSubPenaltiesBtn"));
		m_AdminReloadBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDAdminReloadBtn"));
		m_AdminList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("SMDAdminList"));
		m_AdminActionBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDAdminActionBtn"));
		m_AdminAction2Btn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SMDAdminAction2Btn"));

		m_OnlinePlayersList.Show(false);
		m_SendInviteBtn.Show(false);

		RequestAll();
		SwitchTab(0);

		return layoutRoot;
	}

	override int GetID()
	{
		return SM_DUNGEON_MENU_ID;
	}

	override bool UseMouse()
	{
		return true;
	}

	override bool UseKeyboard()
	{
		return true;
	}

	override void OnShow()
	{
		super.OnShow();
		RequestAll();
	}

	protected void RequestAll()
	{
		SendSimple(SM_DungeonRPC.REQUEST_STATE);
		SendSimple(SM_DungeonRPC.REQUEST_DUNGEON_LIST);
		SendSimple(SM_DungeonRPC.REQUEST_FINDER_LIST);
	}

	protected void RequestAdminData()
	{
		if (m_AdminSubView == 0)
			SendSimple(SM_DungeonRPC.REQUEST_ADMIN_RUNS);
		else
			SendSimple(SM_DungeonRPC.REQUEST_ADMIN_PENALTIES);
	}

	protected void SwitchTab(int tab)
	{
		m_CurrentTab = tab;
		m_PanelDungeons.Show(tab == 0);
		m_PanelBookings.Show(tab == 1);
		m_PanelInvites.Show(tab == 2);
		m_PanelFinder.Show(tab == 3);
		m_PanelAdmin.Show(tab == 4);

		if (tab == 3)
			SendSimple(SM_DungeonRPC.REQUEST_FINDER_LIST);
		if (tab == 4)
			RequestAdminData();
	}

	protected void SwitchAdminSubView(int view)
	{
		m_AdminSubView = view;
		m_SelectedAdminRow = -1;

		if (view == 0)
		{
			m_AdminActionBtn.SetText(SM_DungeonLoc.Text("#STR_SMD_BTN_FORCE_CANCEL"));
			m_AdminAction2Btn.Show(true);
			m_AdminAction2Btn.SetText(SM_DungeonLoc.Text("#STR_SMD_BTN_FORCE_EVAC"));
		}
		else
		{
			m_AdminActionBtn.SetText(SM_DungeonLoc.Text("#STR_SMD_BTN_PARDON"));
			m_AdminAction2Btn.Show(false);
		}

		RequestAdminData();
		RefreshAdminList();
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (SM_DungeonClientData.StateDirty)
		{
			SM_DungeonClientData.StateDirty = false;
			RefreshBookingsList();
			RefreshInvitesList();
		}
		if (SM_DungeonClientData.DungeonListDirty)
		{
			SM_DungeonClientData.DungeonListDirty = false;
			RefreshDungeonsList();
		}
		if (SM_DungeonClientData.FinderListDirty)
		{
			SM_DungeonClientData.FinderListDirty = false;
			RefreshFinderList();
		}
		if (SM_DungeonClientData.OnlinePlayersDirty)
		{
			SM_DungeonClientData.OnlinePlayersDirty = false;
			if (m_PlayerPickerOpen && m_PlayerPickerMode == 0)
				RefreshPlayerPickerList();
		}
		if (SM_DungeonClientData.AdminRunsDirty && m_AdminSubView == 0)
		{
			SM_DungeonClientData.AdminRunsDirty = false;
			RefreshAdminList();
		}
		if (SM_DungeonClientData.AdminPenaltiesDirty && m_AdminSubView == 1)
		{
			SM_DungeonClientData.AdminPenaltiesDirty = false;
			RefreshAdminList();
		}
		if (SM_DungeonClientData.LastNotify != m_LastSeenNotify)
		{
			m_LastSeenNotify = SM_DungeonClientData.LastNotify;
			m_StatusText.SetText(m_LastSeenNotify);
			m_StatusTextTimer = 5.0;
		}

		if (m_StatusTextTimer > 0)
		{
			m_StatusTextTimer -= timeslice;
			if (m_StatusTextTimer <= 0)
				m_StatusText.SetText("");
		}
	}

	// ------------------------------------------------------------------
	// Обновление списков
	// ------------------------------------------------------------------

	protected SM_DungeonListView GetSelectedDungeonView()
	{
		if (m_SelectedDungeonRow < 0 || m_SelectedDungeonRow >= SM_DungeonClientData.DungeonList.Count())
			return NULL;
		return SM_DungeonClientData.DungeonList[m_SelectedDungeonRow];
	}

	protected void GetAllowedModesForView(SM_DungeonListView view, out array<int> modes)
	{
		modes = new array<int>;
		if (!view)
			return;
		if (view.AllowSolo)
			modes.Insert(SM_DungeonMode.SOLO);
		if (view.AllowInviteGroup)
			modes.Insert(SM_DungeonMode.INVITE_GROUP);
		if (view.AllowGroupFinder)
			modes.Insert(SM_DungeonMode.GROUP_FINDER);
	}

	protected void RefreshDungeonsList()
	{
		m_DungeonsList.ClearItems();
		for (int i = 0; i < SM_DungeonClientData.DungeonList.Count(); i++)
		{
			SM_DungeonListView view = SM_DungeonClientData.DungeonList[i];
			int row = m_DungeonsList.AddItem(view.Name, NULL, 0);
			m_DungeonsList.SetItem(row, view.MinPlayers.ToString() + "-" + view.MaxPlayers.ToString(), NULL, 1);

			string statusText = SM_DungeonLoc.Text("#STR_SMD_DUNGEON_STATUS_OPEN");
			if (!view.Enabled)
				statusText = SM_DungeonLoc.Text("#STR_SMD_DUNGEON_STATUS_OFF");
			else if (view.Locked)
				statusText = SM_DungeonLoc.Text("#STR_SMD_DUNGEON_STATUS_LOCKED");
			m_DungeonsList.SetItem(row, statusText, NULL, 2);
		}

		m_SelectedDungeonRow = -1;
		RefreshDungeonDetailPanel();
	}

	protected void RefreshDungeonDetailPanel()
	{
		SM_DungeonListView view = GetSelectedDungeonView();
		if (!view)
		{
			m_DifficultyValueText.SetText("");
			m_ModeValueText.SetText("");
			m_SlotValueText.SetText("");
			m_DescriptionText.SetText("");
			m_FinderCommentEdit.Show(false);
			return;
		}

		if (view.Difficulties.Count() > 0)
		{
			if (m_SelectedDifficultyIndex >= view.Difficulties.Count())
				m_SelectedDifficultyIndex = 0;
			m_DifficultyValueText.SetText(view.Difficulties[m_SelectedDifficultyIndex].Name);
		}
		else
		{
			m_DifficultyValueText.SetText("");
		}

		array<int> modes;
		GetAllowedModesForView(view, modes);
		if (modes.Count() > 0)
		{
			if (m_SelectedModeIndex >= modes.Count())
				m_SelectedModeIndex = 0;
			int mode = modes[m_SelectedModeIndex];
			m_ModeValueText.SetText(SM_DungeonMode.ToLabel(mode));
			m_FinderCommentEdit.Show(mode == SM_DungeonMode.GROUP_FINDER);
		}
		else
		{
			m_ModeValueText.SetText("");
			m_FinderCommentEdit.Show(false);
		}

		if (view.AvailableSlots.Count() > 0)
		{
			if (m_SelectedSlotIndex >= view.AvailableSlots.Count())
				m_SelectedSlotIndex = 0;
			m_SlotValueText.SetText(SM_DungeonClock.FormatAbsoluteMinute(view.AvailableSlots[m_SelectedSlotIndex]));
		}
		else
		{
			m_SlotValueText.SetText("-");
		}

		m_DescriptionText.SetText(SM_DungeonLoc.Text(view.Description));

		if (view.Enabled)
			m_AdminToggleBtn.SetText(SM_DungeonLoc.Text("#STR_SMD_BTN_DISABLE"));
		else
			m_AdminToggleBtn.SetText(SM_DungeonLoc.Text("#STR_SMD_BTN_ENABLE"));
	}

	protected void ChangeDifficulty(int delta)
	{
		SM_DungeonListView view = GetSelectedDungeonView();
		if (!view || view.Difficulties.Count() == 0)
			return;
		m_SelectedDifficultyIndex = (m_SelectedDifficultyIndex + delta + view.Difficulties.Count()) % view.Difficulties.Count();
		RefreshDungeonDetailPanel();
	}

	protected void ChangeMode(int delta)
	{
		SM_DungeonListView view = GetSelectedDungeonView();
		array<int> modes;
		GetAllowedModesForView(view, modes);
		if (modes.Count() == 0)
			return;
		m_SelectedModeIndex = (m_SelectedModeIndex + delta + modes.Count()) % modes.Count();
		RefreshDungeonDetailPanel();
	}

	protected void ChangeSlot(int delta)
	{
		SM_DungeonListView view = GetSelectedDungeonView();
		if (!view || view.AvailableSlots.Count() == 0)
			return;
		m_SelectedSlotIndex = (m_SelectedSlotIndex + delta + view.AvailableSlots.Count()) % view.AvailableSlots.Count();
		RefreshDungeonDetailPanel();
	}

	protected void SendRegister()
	{
		SM_DungeonListView view = GetSelectedDungeonView();
		if (!view)
			return;

		array<int> modes;
		GetAllowedModesForView(view, modes);
		if (modes.Count() == 0)
			return;
		int mode = modes[m_SelectedModeIndex % modes.Count()];

		if (view.AvailableSlots.Count() == 0)
			return;
		int startMinute = view.AvailableSlots[m_SelectedSlotIndex % view.AvailableSlots.Count()];

		if (mode == SM_DungeonMode.SOLO)
			SendRegisterSolo(view.Id, m_SelectedDifficultyIndex, startMinute);
		else if (mode == SM_DungeonMode.INVITE_GROUP)
			SendRegisterGroupCreate(view.Id, m_SelectedDifficultyIndex, startMinute);
		else
			SendFinderPublish(view.Id, m_SelectedDifficultyIndex, startMinute, m_FinderCommentEdit.GetText());
	}

	protected void RefreshBookingsList()
	{
		m_BookingsList.ClearItems();
		foreach (SM_DungeonBookingView booking : SM_DungeonClientData.MyBookings)
		{
			int row = m_BookingsList.AddItem(booking.DungeonName, NULL, 0);
			m_BookingsList.SetItem(row, booking.DifficultyName, NULL, 1);
			m_BookingsList.SetItem(row, SM_DungeonMode.ToLabel(booking.Mode), NULL, 2);
			m_BookingsList.SetItem(row, SM_DungeonClock.FormatAbsoluteMinute(booking.StartMinute), NULL, 3);
			m_BookingsList.SetItem(row, SM_DungeonBookingStatus.ToLabel(booking.Status), NULL, 4);
		}

		m_SelectedBookingRow = -1;
		RefreshBookingDetails();
	}

	protected void RefreshBookingDetails()
	{
		if (m_SelectedBookingRow < 0 || m_SelectedBookingRow >= SM_DungeonClientData.MyBookings.Count())
		{
			m_BookingDetailsText.SetText("");
			return;
		}

		SM_DungeonBookingView booking = SM_DungeonClientData.MyBookings[m_SelectedBookingRow];
		string text = SM_DungeonLoc.Text("#STR_SMD_COL_LEADER") + ": " + booking.LeaderName;
		foreach (SM_DungeonBookingMemberView member : booking.Members)
			text = text + "\n" + member.Name;

		m_BookingDetailsText.SetText(text);
	}

	protected void RefreshInvitesList()
	{
		m_InvitesList.ClearItems();
		foreach (SM_DungeonInviteView invite : SM_DungeonClientData.MyInvites)
		{
			int row = m_InvitesList.AddItem(invite.DungeonName, NULL, 0);
			m_InvitesList.SetItem(row, invite.DifficultyName, NULL, 1);
			m_InvitesList.SetItem(row, invite.InviterName, NULL, 2);
			m_InvitesList.SetItem(row, SM_DungeonClock.FormatAbsoluteMinute(invite.ExpiresAtMinute), NULL, 3);
		}

		m_SelectedInviteRow = -1;
	}

	protected void RefreshFinderList()
	{
		m_FinderList.ClearItems();
		foreach (SM_DungeonFinderView listing : SM_DungeonClientData.FinderList)
		{
			int row = m_FinderList.AddItem(listing.DungeonName, NULL, 0);
			m_FinderList.SetItem(row, listing.DifficultyName, NULL, 1);
			m_FinderList.SetItem(row, listing.LeaderName, NULL, 2);
			m_FinderList.SetItem(row, listing.PartySize.ToString() + "/" + listing.MaxPlayers.ToString(), NULL, 3);
			m_FinderList.SetItem(row, SM_DungeonClock.FormatAbsoluteMinute(listing.StartMinute), NULL, 4);
			m_FinderList.SetItem(row, listing.Comment, NULL, 5);
		}

		m_SelectedFinderRow = -1;
	}

	protected void RefreshPlayerPickerList()
	{
		m_OnlinePlayersList.Show(m_PlayerPickerOpen);
		m_SendInviteBtn.Show(m_PlayerPickerOpen);
		m_OnlinePlayersList.ClearItems();
		m_SelectedOnlinePlayerRow = -1;

		if (!m_PlayerPickerOpen)
			return;

		if (m_PlayerPickerMode == 1)
		{
			m_SendInviteBtn.SetText(SM_DungeonLoc.Text("#STR_SMD_BTN_KICK"));
			if (m_SelectedBookingRow >= 0 && m_SelectedBookingRow < SM_DungeonClientData.MyBookings.Count())
			{
				foreach (SM_DungeonBookingMemberView member : SM_DungeonClientData.MyBookings[m_SelectedBookingRow].Members)
					m_OnlinePlayersList.AddItem(member.Name, NULL, 0);
			}
		}
		else
		{
			m_SendInviteBtn.SetText(SM_DungeonLoc.Text("#STR_SMD_BTN_INVITE"));
			foreach (SM_DungeonOnlinePlayerView onlinePlayer : SM_DungeonClientData.OnlinePlayers)
				m_OnlinePlayersList.AddItem(onlinePlayer.Name, NULL, 0);
		}
	}

	protected void RefreshAdminList()
	{
		m_AdminList.ClearItems();

		if (m_AdminSubView == 0)
		{
			foreach (SM_DungeonAdminRunView run : SM_DungeonClientData.AdminRuns)
			{
				int runRow = m_AdminList.AddItem(run.DungeonName, NULL, 0);
				m_AdminList.SetItem(runRow, SM_DungeonRunPhase.ToLabel(run.Phase), NULL, 1);
				m_AdminList.SetItem(runRow, run.SecondsRemaining.ToString(), NULL, 2);
				m_AdminList.SetItem(runRow, run.ParticipantCount.ToString(), NULL, 3);
			}
		}
		else
		{
			foreach (SM_DungeonAdminPenaltyView penalty : SM_DungeonClientData.AdminPenalties)
			{
				int penaltyRow = m_AdminList.AddItem(penalty.Name, NULL, 0);

				string lockText = "-";
				if (penalty.PermaBanned)
					lockText = SM_DungeonLoc.Text("#STR_SMD_NOTIFY_PERMA_BAN");
				else if (penalty.LockUntilMinute > 0)
					lockText = SM_DungeonClock.FormatAbsoluteMinute(penalty.LockUntilMinute);

				m_AdminList.SetItem(penaltyRow, lockText, NULL, 1);
				m_AdminList.SetItem(penaltyRow, penalty.OffenseCount.ToString(), NULL, 2);
				m_AdminList.SetItem(penaltyRow, "", NULL, 3);
			}
		}

		m_SelectedAdminRow = -1;
	}

	// ------------------------------------------------------------------
	// Клики
	// ------------------------------------------------------------------

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_CloseBtn)
		{
			Close();
			return true;
		}
		if (w == m_TabDungeonsBtn) { SwitchTab(0); return true; }
		if (w == m_TabBookingsBtn) { SwitchTab(1); return true; }
		if (w == m_TabInvitesBtn) { SwitchTab(2); return true; }
		if (w == m_TabFinderBtn) { SwitchTab(3); return true; }
		if (w == m_TabAdminBtn) { SwitchTab(4); return true; }

		if (w == m_DifficultyPrevBtn) { ChangeDifficulty(-1); return true; }
		if (w == m_DifficultyNextBtn) { ChangeDifficulty(1); return true; }
		if (w == m_ModePrevBtn) { ChangeMode(-1); return true; }
		if (w == m_ModeNextBtn) { ChangeMode(1); return true; }
		if (w == m_SlotPrevBtn) { ChangeSlot(-1); return true; }
		if (w == m_SlotNextBtn) { ChangeSlot(1); return true; }
		if (w == m_RegisterBtn) { SendRegister(); return true; }
		if (w == m_AdminToggleBtn)
		{
			SM_DungeonListView view = GetSelectedDungeonView();
			if (view)
			{
				int newEnabled = 0;
				if (!view.Enabled)
					newEnabled = 1;
				SendStringInt(SM_DungeonRPC.ADMIN_TOGGLE_DUNGEON, view.Id, newEnabled);
			}
			return true;
		}

		if (w == m_CancelBtn)
		{
			if (m_SelectedBookingRow >= 0 && m_SelectedBookingRow < SM_DungeonClientData.MyBookings.Count())
				SendString(SM_DungeonRPC.CANCEL_BOOKING, SM_DungeonClientData.MyBookings[m_SelectedBookingRow].Id);
			return true;
		}
		if (w == m_LeaveBtn)
		{
			if (m_SelectedBookingRow >= 0 && m_SelectedBookingRow < SM_DungeonClientData.MyBookings.Count())
				SendString(SM_DungeonRPC.LEAVE_GROUP, SM_DungeonClientData.MyBookings[m_SelectedBookingRow].Id);
			return true;
		}
		if (w == m_InviteBtn)
		{
			bool wasOpenAsInvite = m_PlayerPickerOpen && m_PlayerPickerMode == 0;
			m_PlayerPickerMode = 0;
			m_PlayerPickerOpen = !wasOpenAsInvite;
			RefreshPlayerPickerList();
			if (m_PlayerPickerOpen)
				SendSimple(SM_DungeonRPC.REQUEST_ONLINE_PLAYERS);
			return true;
		}
		if (w == m_KickBtn)
		{
			bool wasOpenAsKick = m_PlayerPickerOpen && m_PlayerPickerMode == 1;
			m_PlayerPickerMode = 1;
			m_PlayerPickerOpen = !wasOpenAsKick;
			RefreshPlayerPickerList();
			return true;
		}
		if (w == m_SendInviteBtn)
		{
			if (m_SelectedBookingRow < 0 || m_SelectedBookingRow >= SM_DungeonClientData.MyBookings.Count() || m_SelectedOnlinePlayerRow < 0)
				return true;

			string bookingId = SM_DungeonClientData.MyBookings[m_SelectedBookingRow].Id;
			if (m_PlayerPickerMode == 1)
			{
				SM_DungeonBookingView booking = SM_DungeonClientData.MyBookings[m_SelectedBookingRow];
				if (m_SelectedOnlinePlayerRow < booking.Members.Count())
					SendStringString(SM_DungeonRPC.KICK_MEMBER, bookingId, booking.Members[m_SelectedOnlinePlayerRow].Uid);
			}
			else if (m_SelectedOnlinePlayerRow < SM_DungeonClientData.OnlinePlayers.Count())
			{
				SendStringString(SM_DungeonRPC.INVITE_PLAYER, bookingId, SM_DungeonClientData.OnlinePlayers[m_SelectedOnlinePlayerRow].Uid);
			}
			return true;
		}

		if (w == m_AcceptBtn)
		{
			if (m_SelectedInviteRow >= 0 && m_SelectedInviteRow < SM_DungeonClientData.MyInvites.Count())
				SendString(SM_DungeonRPC.ACCEPT_INVITE, SM_DungeonClientData.MyInvites[m_SelectedInviteRow].Id);
			return true;
		}
		if (w == m_DeclineBtn)
		{
			if (m_SelectedInviteRow >= 0 && m_SelectedInviteRow < SM_DungeonClientData.MyInvites.Count())
				SendString(SM_DungeonRPC.DECLINE_INVITE, SM_DungeonClientData.MyInvites[m_SelectedInviteRow].Id);
			return true;
		}

		if (w == m_JoinBtn)
		{
			if (m_SelectedFinderRow >= 0 && m_SelectedFinderRow < SM_DungeonClientData.FinderList.Count())
				SendString(SM_DungeonRPC.FINDER_JOIN, SM_DungeonClientData.FinderList[m_SelectedFinderRow].BookingId);
			return true;
		}
		if (w == m_UnlistBtn)
		{
			foreach (SM_DungeonBookingView booking : SM_DungeonClientData.MyBookings)
			{
				if (booking.Mode == SM_DungeonMode.GROUP_FINDER && booking.FinderOpen)
				{
					SendString(SM_DungeonRPC.FINDER_UNLIST, booking.Id);
					break;
				}
			}
			return true;
		}
		if (w == m_RefreshFinderBtn)
		{
			SendSimple(SM_DungeonRPC.REQUEST_FINDER_LIST);
			return true;
		}

		if (w == m_AdminSubRunsBtn) { SwitchAdminSubView(0); return true; }
		if (w == m_AdminSubPenaltiesBtn) { SwitchAdminSubView(1); return true; }
		if (w == m_AdminReloadBtn) { SendSimple(SM_DungeonRPC.ADMIN_RELOAD_CONFIG); return true; }
		if (w == m_AdminActionBtn)
		{
			if (m_AdminSubView == 0)
			{
				if (m_SelectedAdminRow >= 0 && m_SelectedAdminRow < SM_DungeonClientData.AdminRuns.Count())
					SendString(SM_DungeonRPC.ADMIN_FORCE_CANCEL, SM_DungeonClientData.AdminRuns[m_SelectedAdminRow].BookingId);
			}
			else if (m_SelectedAdminRow >= 0 && m_SelectedAdminRow < SM_DungeonClientData.AdminPenalties.Count())
			{
				SendString(SM_DungeonRPC.ADMIN_PARDON_PLAYER, SM_DungeonClientData.AdminPenalties[m_SelectedAdminRow].Uid);
			}
			return true;
		}
		if (w == m_AdminAction2Btn)
		{
			if (m_AdminSubView == 0 && m_SelectedAdminRow >= 0 && m_SelectedAdminRow < SM_DungeonClientData.AdminRuns.Count())
				SendString(SM_DungeonRPC.ADMIN_FORCE_EVAC, SM_DungeonClientData.AdminRuns[m_SelectedAdminRow].RunId);
			return true;
		}

		return super.OnClick(w, x, y, button);
	}

	override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		if (w == m_DungeonsList)
		{
			m_SelectedDungeonRow = row;
			m_SelectedDifficultyIndex = 0;
			m_SelectedModeIndex = 0;
			m_SelectedSlotIndex = 0;
			RefreshDungeonDetailPanel();
			return true;
		}
		if (w == m_BookingsList)
		{
			m_SelectedBookingRow = row;
			RefreshBookingDetails();
			return true;
		}
		if (w == m_InvitesList) { m_SelectedInviteRow = row; return true; }
		if (w == m_FinderList) { m_SelectedFinderRow = row; return true; }
		if (w == m_OnlinePlayersList) { m_SelectedOnlinePlayerRow = row; return true; }
		if (w == m_AdminList) { m_SelectedAdminRow = row; return true; }

		return super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
	}

	// ------------------------------------------------------------------
	// RPC хелперы
	// ------------------------------------------------------------------

	protected void SendSimple(int rpcType)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(player, rpcType, true, NULL);
	}

	protected void SendString(int rpcType, string value)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(value);
		rpc.Send(player, rpcType, true, NULL);
	}

	protected void SendStringInt(int rpcType, string a, int b)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(a);
		rpc.Write(b);
		rpc.Send(player, rpcType, true, NULL);
	}

	protected void SendStringString(int rpcType, string a, string b)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(a);
		rpc.Write(b);
		rpc.Send(player, rpcType, true, NULL);
	}

	protected void SendRegisterSolo(string dungeonId, int difficultyIndex, int startMinute)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(dungeonId);
		rpc.Write(difficultyIndex);
		rpc.Write(startMinute);
		rpc.Send(player, SM_DungeonRPC.REGISTER_SOLO, true, NULL);
	}

	protected void SendRegisterGroupCreate(string dungeonId, int difficultyIndex, int startMinute)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(dungeonId);
		rpc.Write(difficultyIndex);
		rpc.Write(startMinute);
		rpc.Send(player, SM_DungeonRPC.REGISTER_GROUP_CREATE, true, NULL);
	}

	protected void SendFinderPublish(string dungeonId, int difficultyIndex, int startMinute, string comment)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(dungeonId);
		rpc.Write(difficultyIndex);
		rpc.Write(startMinute);
		rpc.Write(comment);
		rpc.Send(player, SM_DungeonRPC.FINDER_PUBLISH, true, NULL);
	}
}
