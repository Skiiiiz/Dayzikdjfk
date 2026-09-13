// ============================================================================
// SM_DungeonClientData.c
//
// Клиентский кэш всего, что рисует SM_DungeonMenu. Заполняется из
// modded PlayerBase::OnRPC (см. SM_DungeonPlayerBase.c) при получении
// SM_DungeonRPC.SYNC_* от сервера. Меню само по себе ничего не хранит
// между открытиями - только читает эти статические поля и выставляет
// Dirty-флаги в false после перерисовки соответствующей вкладки.
// ============================================================================

class SM_DungeonDifficultyView
{
	string Name;
}

class SM_DungeonListView
{
	string Id;
	string Name;
	string Description;
	bool Enabled;
	bool Locked;
	int MinPlayers;
	int MaxPlayers;
	bool AllowSolo;
	bool AllowInviteGroup;
	bool AllowGroupFinder;
	ref array<ref SM_DungeonDifficultyView> Difficulties = new array<ref SM_DungeonDifficultyView>;
	ref array<int> AvailableSlots = new array<int>; // абсолютные минуты, см. SM_DungeonClock
}

class SM_DungeonBookingMemberView
{
	string Uid;
	string Name;
}

class SM_DungeonBookingView
{
	string Id;
	string DungeonId;
	string DungeonName;
	int DifficultyIndex;
	string DifficultyName;
	int Mode;
	int StartMinute;
	int Status;
	string LeaderUid;
	string LeaderName;
	int PartySize;
	bool FinderOpen;
	string FinderComment;
	ref array<ref SM_DungeonBookingMemberView> Members = new array<ref SM_DungeonBookingMemberView>;
}

class SM_DungeonInviteView
{
	string Id;
	string BookingId;
	string DungeonName;
	string DifficultyName;
	string InviterName;
	int ExpiresAtMinute;
}

class SM_DungeonFinderView
{
	string BookingId;
	string DungeonId;
	string DungeonName;
	int DifficultyIndex;
	string DifficultyName;
	string LeaderName;
	int PartySize;
	int MaxPlayers;
	string Comment;
	int StartMinute;
}

class SM_DungeonOnlinePlayerView
{
	string Uid;
	string Name;
}

class SM_DungeonLootRollView
{
	string RollId;
	string ItemClassName;
	int Quantity;
	int SecondsRemaining;
}

class SM_DungeonAdminRunView
{
	string RunId;
	string BookingId;
	string DungeonName;
	int Phase;
	int SecondsRemaining;
	int ParticipantCount;
}

class SM_DungeonAdminPenaltyView
{
	string Uid;
	string Name;
	int LockUntilMinute;
	bool PermaBanned;
	int OffenseCount;
}

class SM_DungeonClientData
{
	// ---- личный кабинет ----
	static bool PermaBanned = false;
	static int LockUntilMinute = 0;
	static ref array<string> UnlockedDungeonIds = new array<string>;
	static ref array<ref SM_DungeonBookingView> MyBookings = new array<ref SM_DungeonBookingView>;
	static ref array<ref SM_DungeonInviteView> MyInvites = new array<ref SM_DungeonInviteView>;
	static bool StateDirty = false;

	// ---- список данжей ----
	static ref array<ref SM_DungeonListView> DungeonList = new array<ref SM_DungeonListView>;
	static bool DungeonListDirty = false;

	// ---- поиск спутников ----
	static ref array<ref SM_DungeonFinderView> FinderList = new array<ref SM_DungeonFinderView>;
	static bool FinderListDirty = false;

	// ---- список игроков онлайн (для приглашений) ----
	static ref array<ref SM_DungeonOnlinePlayerView> OnlinePlayers = new array<ref SM_DungeonOnlinePlayerView>;
	static bool OnlinePlayersDirty = false;

	// ---- активный ролл добычи (очередь, если несколько подряд) ----
	static ref array<ref SM_DungeonLootRollView> ActiveLootRolls = new array<ref SM_DungeonLootRollView>;
	static bool LootRollDirty = false;
	static string LastRollResultText = "";
	static bool LootRollResultDirty = false;

	// ---- HUD активного забега ----
	static int RunPhase = 0;
	static int RunSecondsRemaining = 0;
	static string RunDungeonName = "";
	static bool RunNotifyDirty = false;
	static bool EvacActive = false;
	static int EvacSecondsRemaining = 0;
	static int EvacTotalSeconds = 30;
	static bool EvacDirty = false;

	// ---- всплывающие уведомления ----
	static string LastNotify = "";
	static bool NotifyDirty = false;

	// ---- админ вкладка ----
	static ref array<ref SM_DungeonAdminRunView> AdminRuns = new array<ref SM_DungeonAdminRunView>;
	static bool AdminRunsDirty = false;
	static ref array<ref SM_DungeonAdminPenaltyView> AdminPenalties = new array<ref SM_DungeonAdminPenaltyView>;
	static bool AdminPenaltiesDirty = false;

	static void OnRPC(int rpc_type, ParamsReadContext ctx)
	{
		switch (rpc_type)
		{
			case SM_DungeonRPC.SYNC_STATE:
				ReadState(ctx);
				break;
			case SM_DungeonRPC.SYNC_DUNGEON_LIST:
				ReadDungeonList(ctx);
				break;
			case SM_DungeonRPC.SYNC_FINDER_LIST:
				ReadFinderList(ctx);
				break;
			case SM_DungeonRPC.SYNC_ONLINE_PLAYERS:
				ReadOnlinePlayers(ctx);
				break;
			case SM_DungeonRPC.SYNC_LOOT_ROLL:
				ReadLootRoll(ctx);
				break;
			case SM_DungeonRPC.SYNC_LOOT_ROLL_RESULT:
				ReadLootRollResult(ctx);
				break;
			case SM_DungeonRPC.SYNC_RUN_NOTIFY:
				ReadRunNotify(ctx);
				break;
			case SM_DungeonRPC.SYNC_EVAC_PROGRESS:
				ReadEvacProgress(ctx);
				break;
			case SM_DungeonRPC.LOCAL_NOTIFY:
				ReadLocalNotify(ctx);
				break;
			case SM_DungeonRPC.SYNC_ADMIN_RUNS:
				ReadAdminRuns(ctx);
				break;
			case SM_DungeonRPC.SYNC_ADMIN_PENALTIES:
				ReadAdminPenalties(ctx);
				break;
		}
	}

	protected static void ReadState(ParamsReadContext ctx)
	{
		int permaBannedInt, lockUntil, unlockedCount, bookingsCount, invitesCount;
		if (!ctx.Read(permaBannedInt)) return;
		if (!ctx.Read(lockUntil)) return;
		if (!ctx.Read(unlockedCount)) return;

		PermaBanned = permaBannedInt != 0;
		LockUntilMinute = lockUntil;
		UnlockedDungeonIds.Clear();
		for (int u = 0; u < unlockedCount; u++)
		{
			string unlockedId;
			if (!ctx.Read(unlockedId)) return;
			UnlockedDungeonIds.Insert(unlockedId);
		}

		if (!ctx.Read(bookingsCount)) return;
		MyBookings.Clear();
		for (int b = 0; b < bookingsCount; b++)
		{
			SM_DungeonBookingView view = new SM_DungeonBookingView();
			if (!ctx.Read(view.Id)) return;
			if (!ctx.Read(view.DungeonId)) return;
			if (!ctx.Read(view.DungeonName)) return;
			if (!ctx.Read(view.DifficultyIndex)) return;
			if (!ctx.Read(view.DifficultyName)) return;
			if (!ctx.Read(view.Mode)) return;
			if (!ctx.Read(view.StartMinute)) return;
			if (!ctx.Read(view.Status)) return;
			if (!ctx.Read(view.LeaderUid)) return;
			if (!ctx.Read(view.LeaderName)) return;
			if (!ctx.Read(view.PartySize)) return;
			int finderOpenInt;
			if (!ctx.Read(finderOpenInt)) return;
			view.FinderOpen = finderOpenInt != 0;
			if (!ctx.Read(view.FinderComment)) return;

			int memberCount;
			if (!ctx.Read(memberCount)) return;
			for (int m = 0; m < memberCount; m++)
			{
				SM_DungeonBookingMemberView member = new SM_DungeonBookingMemberView();
				if (!ctx.Read(member.Uid)) return;
				if (!ctx.Read(member.Name)) return;
				view.Members.Insert(member);
			}

			MyBookings.Insert(view);
		}

		if (!ctx.Read(invitesCount)) return;
		MyInvites.Clear();
		for (int i = 0; i < invitesCount; i++)
		{
			SM_DungeonInviteView invite = new SM_DungeonInviteView();
			if (!ctx.Read(invite.Id)) return;
			if (!ctx.Read(invite.BookingId)) return;
			if (!ctx.Read(invite.DungeonName)) return;
			if (!ctx.Read(invite.DifficultyName)) return;
			if (!ctx.Read(invite.InviterName)) return;
			if (!ctx.Read(invite.ExpiresAtMinute)) return;
			MyInvites.Insert(invite);
		}

		StateDirty = true;
	}

	protected static void ReadDungeonList(ParamsReadContext ctx)
	{
		int count;
		if (!ctx.Read(count)) return;

		DungeonList.Clear();
		for (int i = 0; i < count; i++)
		{
			SM_DungeonListView view = new SM_DungeonListView();
			if (!ctx.Read(view.Id)) return;
			if (!ctx.Read(view.Name)) return;
			if (!ctx.Read(view.Description)) return;
			int enabledInt, lockedInt, soloInt, groupInt, finderInt;
			if (!ctx.Read(enabledInt)) return;
			if (!ctx.Read(lockedInt)) return;
			if (!ctx.Read(view.MinPlayers)) return;
			if (!ctx.Read(view.MaxPlayers)) return;
			if (!ctx.Read(soloInt)) return;
			if (!ctx.Read(groupInt)) return;
			if (!ctx.Read(finderInt)) return;
			view.Enabled = enabledInt != 0;
			view.Locked = lockedInt != 0;
			view.AllowSolo = soloInt != 0;
			view.AllowInviteGroup = groupInt != 0;
			view.AllowGroupFinder = finderInt != 0;

			int diffCount;
			if (!ctx.Read(diffCount)) return;
			for (int d = 0; d < diffCount; d++)
			{
				SM_DungeonDifficultyView diff = new SM_DungeonDifficultyView();
				if (!ctx.Read(diff.Name)) return;
				view.Difficulties.Insert(diff);
			}

			int slotCount;
			if (!ctx.Read(slotCount)) return;
			for (int s = 0; s < slotCount; s++)
			{
				int slotMinute;
				if (!ctx.Read(slotMinute)) return;
				view.AvailableSlots.Insert(slotMinute);
			}

			DungeonList.Insert(view);
		}

		DungeonListDirty = true;
	}

	protected static void ReadFinderList(ParamsReadContext ctx)
	{
		int count;
		if (!ctx.Read(count)) return;

		FinderList.Clear();
		for (int i = 0; i < count; i++)
		{
			SM_DungeonFinderView view = new SM_DungeonFinderView();
			if (!ctx.Read(view.BookingId)) return;
			if (!ctx.Read(view.DungeonId)) return;
			if (!ctx.Read(view.DungeonName)) return;
			if (!ctx.Read(view.DifficultyIndex)) return;
			if (!ctx.Read(view.DifficultyName)) return;
			if (!ctx.Read(view.LeaderName)) return;
			if (!ctx.Read(view.PartySize)) return;
			if (!ctx.Read(view.MaxPlayers)) return;
			if (!ctx.Read(view.Comment)) return;
			if (!ctx.Read(view.StartMinute)) return;
			FinderList.Insert(view);
		}

		FinderListDirty = true;
	}

	protected static void ReadOnlinePlayers(ParamsReadContext ctx)
	{
		int count;
		if (!ctx.Read(count)) return;

		OnlinePlayers.Clear();
		for (int i = 0; i < count; i++)
		{
			SM_DungeonOnlinePlayerView view = new SM_DungeonOnlinePlayerView();
			if (!ctx.Read(view.Uid)) return;
			if (!ctx.Read(view.Name)) return;
			OnlinePlayers.Insert(view);
		}

		OnlinePlayersDirty = true;
	}

	protected static void ReadLootRoll(ParamsReadContext ctx)
	{
		SM_DungeonLootRollView view = new SM_DungeonLootRollView();
		if (!ctx.Read(view.RollId)) return;
		if (!ctx.Read(view.ItemClassName)) return;
		if (!ctx.Read(view.Quantity)) return;
		if (!ctx.Read(view.SecondsRemaining)) return;

		foreach (SM_DungeonLootRollView existing : ActiveLootRolls)
		{
			if (existing.RollId == view.RollId)
			{
				existing.SecondsRemaining = view.SecondsRemaining;
				LootRollDirty = true;
				return;
			}
		}

		ActiveLootRolls.Insert(view);
		LootRollDirty = true;
	}

	protected static void ReadLootRollResult(ParamsReadContext ctx)
	{
		string rollId, itemClassName, winnerName;
		int quantity, wasTieInt;
		if (!ctx.Read(rollId)) return;
		if (!ctx.Read(itemClassName)) return;
		if (!ctx.Read(quantity)) return;
		if (!ctx.Read(winnerName)) return;
		if (!ctx.Read(wasTieInt)) return;

		for (int i = ActiveLootRolls.Count() - 1; i >= 0; i--)
		{
			if (ActiveLootRolls[i].RollId == rollId)
				ActiveLootRolls.Remove(i);
		}

		string text = SM_DungeonLoc.Text("#STR_SMD_ROLL_WINNER") + ": " + winnerName + " - " + itemClassName;
		if (wasTieInt != 0)
			text = text + " (" + SM_DungeonLoc.Text("#STR_SMD_ROLL_RANDOM") + ")";

		LastRollResultText = text;
		LootRollResultDirty = true;
		LootRollDirty = true;
	}

	protected static void ReadRunNotify(ParamsReadContext ctx)
	{
		int phase, secondsRemaining;
		string dungeonName;
		if (!ctx.Read(phase)) return;
		if (!ctx.Read(secondsRemaining)) return;
		if (!ctx.Read(dungeonName)) return;

		RunPhase = phase;
		RunSecondsRemaining = secondsRemaining;
		RunDungeonName = dungeonName;
		RunNotifyDirty = true;
	}

	protected static void ReadEvacProgress(ParamsReadContext ctx)
	{
		int activeInt, secondsRemaining, totalSeconds;
		if (!ctx.Read(activeInt)) return;
		if (!ctx.Read(secondsRemaining)) return;
		if (!ctx.Read(totalSeconds)) return;

		EvacActive = activeInt != 0;
		EvacSecondsRemaining = secondsRemaining;
		EvacTotalSeconds = totalSeconds;
		EvacDirty = true;
	}

	protected static void ReadLocalNotify(ParamsReadContext ctx)
	{
		string text;
		if (!ctx.Read(text)) return;
		LastNotify = text;
		NotifyDirty = true;
	}

	protected static void ReadAdminRuns(ParamsReadContext ctx)
	{
		int count;
		if (!ctx.Read(count)) return;

		AdminRuns.Clear();
		for (int i = 0; i < count; i++)
		{
			SM_DungeonAdminRunView view = new SM_DungeonAdminRunView();
			if (!ctx.Read(view.RunId)) return;
			if (!ctx.Read(view.BookingId)) return;
			if (!ctx.Read(view.DungeonName)) return;
			if (!ctx.Read(view.Phase)) return;
			if (!ctx.Read(view.SecondsRemaining)) return;
			if (!ctx.Read(view.ParticipantCount)) return;
			AdminRuns.Insert(view);
		}

		AdminRunsDirty = true;
	}

	protected static void ReadAdminPenalties(ParamsReadContext ctx)
	{
		int count;
		if (!ctx.Read(count)) return;

		AdminPenalties.Clear();
		for (int i = 0; i < count; i++)
		{
			SM_DungeonAdminPenaltyView view = new SM_DungeonAdminPenaltyView();
			if (!ctx.Read(view.Uid)) return;
			if (!ctx.Read(view.Name)) return;
			if (!ctx.Read(view.LockUntilMinute)) return;
			int permaInt;
			if (!ctx.Read(permaInt)) return;
			view.PermaBanned = permaInt != 0;
			if (!ctx.Read(view.OffenseCount)) return;
			AdminPenalties.Insert(view);
		}

		AdminPenaltiesDirty = true;
	}
}
