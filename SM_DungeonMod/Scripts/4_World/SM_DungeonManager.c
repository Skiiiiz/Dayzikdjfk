// ============================================================================
// SM_DungeonManager.c
//
// Серверный синглтон, который целиком владеет логикой мода: брони и
// расписание, приглашения и поиск спутников, жизненный цикл забега
// (обратный отсчёт -> телепорт -> активная фаза -> эвакуация -> завершение),
// защита зоны от посторонних, распределение добычи (Нужно/Не откажусь/
// Откажусь) и штрафы за неявку/досрочный выход.
//
// Хранится в трёх JSON-файлах в $profile:SM_DungeonMod\ - брони,
// приглашения, персональные записи игроков (разблокировки/штрафы/КД).
// Активные забеги (SM_DungeonRunState) существуют только в памяти -
// при рестарте сервера не восстанавливаются, а "зависшие" ACTIVE-брони
// сбрасываются в CANCELLED при следующем старте менеджера.
// ============================================================================

class SM_DungeonManager
{
	protected static ref SM_DungeonManager s_Instance;

	static const string DB_DIR = "$profile:SM_DungeonMod";
	static const string BOOKINGS_PATH = "$profile:SM_DungeonMod\\Bookings.json";
	static const string INVITES_PATH = "$profile:SM_DungeonMod\\Invites.json";
	static const string PLAYERS_PATH = "$profile:SM_DungeonMod\\Players.json";
	protected static const int LOOT_ROLL_SECONDS = 30;

	protected ref SM_DungeonConfig m_Config;
	protected ref SM_DungeonBookingsDB m_BookingsDB;
	protected ref SM_DungeonInvitesDB m_InvitesDB;
	protected ref SM_DungeonPlayersDB m_PlayersDB;
	protected ref array<ref SM_DungeonRunState> m_ActiveRuns;
	protected int m_NextIdCounter;

	static SM_DungeonManager Get()
	{
		return s_Instance;
	}

	static void Init()
	{
		if (!s_Instance)
			s_Instance = new SM_DungeonManager();
	}

	void SM_DungeonManager()
	{
		m_Config = SM_DungeonConfigLoader.Get();
		m_ActiveRuns = new array<ref SM_DungeonRunState>;
		m_NextIdCounter = 1;

		LoadBookingsDB();
		LoadInvitesDB();
		LoadPlayersDB();

		bool hadStaleRuns = false;
		foreach (SM_DungeonBooking booking : m_BookingsDB.Bookings)
		{
			if (booking && booking.Status == SM_DungeonBookingStatus.ACTIVE)
			{
				booking.Status = SM_DungeonBookingStatus.CANCELLED;
				hadStaleRuns = true;
			}
		}
		if (hadStaleRuns)
			SaveBookingsDB();

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.SchedulerTick, 30000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.RunTick, 1000, true);

		Print("[SM_DungeonMod] Manager initialized, " + m_BookingsDB.Bookings.Count() + " booking(s) loaded.");
	}

	// ------------------------------------------------------------------
	// Персистентность
	// ------------------------------------------------------------------

	protected void LoadBookingsDB()
	{
		if (!FileExist(DB_DIR))
			MakeDirectory(DB_DIR);

		m_BookingsDB = new SM_DungeonBookingsDB();
		string errorMessage;
		if (FileExist(BOOKINGS_PATH))
		{
			if (!JsonFileLoader<SM_DungeonBookingsDB>.LoadFile(BOOKINGS_PATH, m_BookingsDB, errorMessage))
			{
				ErrorEx("[SM_DungeonMod] Failed to load Bookings.json: " + errorMessage);
				m_BookingsDB = new SM_DungeonBookingsDB();
			}
		}
		if (!m_BookingsDB.Bookings)
			m_BookingsDB.Bookings = new array<ref SM_DungeonBooking>;
	}

	protected void SaveBookingsDB()
	{
		string errorMessage;
		if (!JsonFileLoader<SM_DungeonBookingsDB>.SaveFile(BOOKINGS_PATH, m_BookingsDB, errorMessage))
			ErrorEx("[SM_DungeonMod] Failed to save Bookings.json: " + errorMessage);
	}

	protected void LoadInvitesDB()
	{
		if (!FileExist(DB_DIR))
			MakeDirectory(DB_DIR);

		m_InvitesDB = new SM_DungeonInvitesDB();
		string errorMessage;
		if (FileExist(INVITES_PATH))
		{
			if (!JsonFileLoader<SM_DungeonInvitesDB>.LoadFile(INVITES_PATH, m_InvitesDB, errorMessage))
			{
				ErrorEx("[SM_DungeonMod] Failed to load Invites.json: " + errorMessage);
				m_InvitesDB = new SM_DungeonInvitesDB();
			}
		}
		if (!m_InvitesDB.Invites)
			m_InvitesDB.Invites = new array<ref SM_DungeonInvite>;
	}

	protected void SaveInvitesDB()
	{
		string errorMessage;
		if (!JsonFileLoader<SM_DungeonInvitesDB>.SaveFile(INVITES_PATH, m_InvitesDB, errorMessage))
			ErrorEx("[SM_DungeonMod] Failed to save Invites.json: " + errorMessage);
	}

	protected void LoadPlayersDB()
	{
		if (!FileExist(DB_DIR))
			MakeDirectory(DB_DIR);

		m_PlayersDB = new SM_DungeonPlayersDB();
		string errorMessage;
		if (FileExist(PLAYERS_PATH))
		{
			if (!JsonFileLoader<SM_DungeonPlayersDB>.LoadFile(PLAYERS_PATH, m_PlayersDB, errorMessage))
			{
				ErrorEx("[SM_DungeonMod] Failed to load Players.json: " + errorMessage);
				m_PlayersDB = new SM_DungeonPlayersDB();
			}
		}
		if (!m_PlayersDB.Players)
			m_PlayersDB.Players = new array<ref SM_DungeonPlayerRecord>;
	}

	protected void SavePlayersDB()
	{
		string errorMessage;
		if (!JsonFileLoader<SM_DungeonPlayersDB>.SaveFile(PLAYERS_PATH, m_PlayersDB, errorMessage))
			ErrorEx("[SM_DungeonMod] Failed to save Players.json: " + errorMessage);
	}

	protected void SaveConfig()
	{
		string errorMessage;
		if (!JsonFileLoader<SM_DungeonConfig>.SaveFile(SM_DungeonConfigLoader.CONFIG_PATH, m_Config, errorMessage))
			ErrorEx("[SM_DungeonMod] Failed to save Settings.json: " + errorMessage);
	}

	// ------------------------------------------------------------------
	// Мелкие хелперы
	// ------------------------------------------------------------------

	protected int BoolToInt(bool value)
	{
		if (value)
			return 1;
		return 0;
	}

	protected string GenerateId()
	{
		m_NextIdCounter++;
		return "d" + SM_DungeonClock.NowMinutes() + "_" + m_NextIdCounter + "_" + Math.RandomIntInclusive(1000, 9999);
	}

	protected PlayerBase GetOnlinePlayerByUid(string uid)
	{
		if (uid == "")
			return NULL;

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity() && pb.GetIdentity().GetPlainId() == uid)
				return pb;
		}
		return NULL;
	}

	// Множитель урона, наносимого игроком, если он сейчас участвует в
	// активном забеге - используется хуком EEHitBy на ZombieBase/AnimalBase
	// (см. SM_DungeonCreatureHooks.c) для реализации "сложности" данжа.
	float GetOutgoingDamageMultiplier(EntityAI source)
	{
		PlayerBase player = PlayerBase.Cast(source);
		if (!player || !player.GetIdentity())
			return 1.0;

		string uid = player.GetIdentity().GetPlainId();
		foreach (SM_DungeonRunState run : m_ActiveRuns)
		{
			if (run.Phase != SM_DungeonRunPhase.ACTIVE && run.Phase != SM_DungeonRunPhase.EVACUATING)
				continue;

			SM_DungeonRunParticipant participant = run.GetParticipant(uid);
			if (!participant || participant.Disconnected || participant.LeftEarly)
				continue;

			SM_DungeonDefinition dungeon = m_Config.GetDungeon(run.DungeonId);
			if (!dungeon)
				continue;
			SM_DungeonDifficultyConfig difficulty = dungeon.GetDifficulty(run.DifficultyIndex);
			if (!difficulty)
				continue;

			return difficulty.OutgoingDamageMultiplier;
		}

		return 1.0;
	}

	protected SM_DungeonPlayerRecord GetOrCreatePlayerRecord(string uid, string knownName = "")
	{
		foreach (SM_DungeonPlayerRecord record : m_PlayersDB.Players)
		{
			if (record && record.Uid == uid)
			{
				if (knownName != "")
					record.LastKnownName = knownName;
				return record;
			}
		}

		SM_DungeonPlayerRecord created = new SM_DungeonPlayerRecord();
		created.Uid = uid;
		created.LastKnownName = knownName;
		m_PlayersDB.Players.Insert(created);
		return created;
	}

	protected SM_DungeonBooking FindBooking(string id)
	{
		foreach (SM_DungeonBooking booking : m_BookingsDB.Bookings)
		{
			if (booking && booking.Id == id)
				return booking;
		}
		return NULL;
	}

	protected SM_DungeonInvite FindInvite(string id)
	{
		foreach (SM_DungeonInvite invite : m_InvitesDB.Invites)
		{
			if (invite && invite.Id == id)
				return invite;
		}
		return NULL;
	}

	protected void RemoveInvite(string id)
	{
		for (int i = m_InvitesDB.Invites.Count() - 1; i >= 0; i--)
		{
			if (m_InvitesDB.Invites[i] && m_InvitesDB.Invites[i].Id == id)
				m_InvitesDB.Invites.Remove(i);
		}
		SaveInvitesDB();
	}

	protected SM_DungeonRunState FindRunByBooking(string bookingId)
	{
		foreach (SM_DungeonRunState run : m_ActiveRuns)
		{
			if (run && run.BookingId == bookingId)
				return run;
		}
		return NULL;
	}

	protected int GetDungeonMaxPlayers(string dungeonId)
	{
		SM_DungeonDefinition dungeon = m_Config.GetDungeon(dungeonId);
		if (!dungeon)
			return 1;
		return dungeon.MaxPlayers;
	}

	protected bool BookingOccupiesCalendar(SM_DungeonBooking booking)
	{
		return booking.Status == SM_DungeonBookingStatus.PENDING
			|| booking.Status == SM_DungeonBookingStatus.CONFIRMED
			|| booking.Status == SM_DungeonBookingStatus.ACTIVE;
	}

	protected int CountActiveBookingsFor(string uid)
	{
		int count = 0;
		foreach (SM_DungeonBooking booking : m_BookingsDB.Bookings)
		{
			if (booking && BookingOccupiesCalendar(booking) && booking.HasMember(uid))
				count++;
		}
		return count;
	}

	// ------------------------------------------------------------------
	// Уведомления
	// ------------------------------------------------------------------

	void Notify(PlayerBase player, string textKey)
	{
		if (!player || !player.GetIdentity())
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(SM_DungeonLoc.Text(textKey));
		rpc.Send(player, SM_DungeonRPC.LOCAL_NOTIFY, true, player.GetIdentity());
	}

	protected void NotifyBookingMembers(SM_DungeonBooking booking, string textKey)
	{
		if (!booking)
			return;

		PlayerBase leader = GetOnlinePlayerByUid(booking.LeaderUid);
		if (leader)
			Notify(leader, textKey);

		foreach (string memberUid : booking.MemberUids)
		{
			PlayerBase pb = GetOnlinePlayerByUid(memberUid);
			if (pb)
				Notify(pb, textKey);
		}
	}

	// ------------------------------------------------------------------
	// Расписание: доступные слоты, пересечения, рестарты
	// ------------------------------------------------------------------

	protected bool IsRestartConflict(int startMinute, int endMinute)
	{
		if (!m_Config.Booking.RestartTimesUTC)
			return false;

		int buffer = m_Config.Booking.RestartBufferMinutes;
		int startDay = startMinute / 1440;

		foreach (string restartText : m_Config.Booking.RestartTimesUTC)
		{
			int restartMinuteOfDay = SM_DungeonClock.ParseHHMM(restartText);
			if (restartMinuteOfDay < 0)
				continue;

			for (int dayOffset = -1; dayOffset <= 1; dayOffset++)
			{
				int restartAbs = (startDay + dayOffset) * 1440 + restartMinuteOfDay;
				int bufferStart = restartAbs - buffer;
				int bufferEnd = restartAbs + buffer;
				if (startMinute < bufferEnd && bufferStart < endMinute)
					return true;
			}
		}

		return false;
	}

	protected bool IsSlotFree(string dungeonId, int startMinute, int endMinute, string excludeBookingId)
	{
		if (IsRestartConflict(startMinute, endMinute))
			return false;

		foreach (SM_DungeonBooking booking : m_BookingsDB.Bookings)
		{
			if (!booking || booking.DungeonId != dungeonId)
				continue;
			if (excludeBookingId != "" && booking.Id == excludeBookingId)
				continue;
			if (!BookingOccupiesCalendar(booking))
				continue;
			if (startMinute < booking.EndMinute && booking.StartMinute < endMinute)
				return false;
		}

		return true;
	}

	protected void ComputeAvailableSlots(string dungeonId, out array<int> result, int maxCount = 20)
	{
		result = new array<int>;

		int interval = m_Config.Booking.SlotIntervalMinutes;
		int duration = m_Config.Booking.SlotDurationMinutes;
		int now = SM_DungeonClock.NowMinutes();
		int windowEnd = now + m_Config.Booking.BookingWindowDays * 1440;

		int cursor = now - (now % interval) + interval;

		while (cursor < windowEnd && result.Count() < maxCount)
		{
			int minuteOfDay = SM_DungeonClock.MinuteOfDay(cursor);
			if (minuteOfDay >= m_Config.Booking.DayStartHour * 60 && minuteOfDay < m_Config.Booking.DayEndHour * 60)
			{
				int end = cursor + duration;
				if (IsSlotFree(dungeonId, cursor, end, ""))
					result.Insert(cursor);
			}
			cursor += interval;
		}
	}

	// ------------------------------------------------------------------
	// Бронирование
	// ------------------------------------------------------------------

	protected bool CanRegister(PlayerBase player, string uid, string dungeonId)
	{
		SM_DungeonDefinition dungeon = m_Config.GetDungeon(dungeonId);
		if (!dungeon || !dungeon.Enabled)
		{
			Notify(player, "#STR_SMD_ERR_GENERIC");
			return false;
		}

		SM_DungeonPlayerRecord record = GetOrCreatePlayerRecord(uid);
		if (record.PermaBanned)
		{
			Notify(player, "#STR_SMD_ERR_PERMA_BANNED");
			return false;
		}
		if (record.TempLockUntilMinute > SM_DungeonClock.NowMinutes())
		{
			Notify(player, "#STR_SMD_ERR_TEMP_LOCKED");
			return false;
		}
		if (dungeon.RequiresUnlock && !record.IsUnlocked(dungeonId))
		{
			Notify(player, "#STR_SMD_ERR_DUNGEON_LOCKED");
			return false;
		}
		if (record.GetCooldownUntil(dungeonId) > SM_DungeonClock.NowMinutes())
		{
			Notify(player, "#STR_SMD_ERR_GENERIC");
			return false;
		}
		if (CountActiveBookingsFor(uid) >= m_Config.General.MaxActiveBookingsPerPlayer)
		{
			Notify(player, "#STR_SMD_ERR_ALREADY_BOOKED");
			return false;
		}

		return true;
	}

	protected SM_DungeonBooking CreateBooking(string dungeonId, int difficultyIndex, int mode, int startMinute, string leaderUid, string leaderName)
	{
		SM_DungeonDefinition dungeon = m_Config.GetDungeon(dungeonId);
		if (!dungeon)
			return NULL;

		int duration = m_Config.Booking.SlotDurationMinutes;
		int endMinute = startMinute + duration;
		if (!IsSlotFree(dungeonId, startMinute, endMinute, ""))
			return NULL;

		SM_DungeonBooking booking = new SM_DungeonBooking();
		booking.Id = GenerateId();
		booking.DungeonId = dungeonId;
		booking.DifficultyIndex = difficultyIndex;
		booking.Mode = mode;
		booking.StartMinute = startMinute;
		booking.EndMinute = endMinute;
		booking.Status = SM_DungeonBookingStatus.CONFIRMED;
		booking.LeaderUid = leaderUid;
		booking.LeaderName = leaderName;
		booking.CreatedAtMinute = SM_DungeonClock.NowMinutes();
		booking.FinderOpen = mode == SM_DungeonMode.GROUP_FINDER;

		m_BookingsDB.Bookings.Insert(booking);
		SaveBookingsDB();
		return booking;
	}

	// ------------------------------------------------------------------
	// RPC: диспетчер (вызывается из modded PlayerBase::OnRPC на сервере)
	// ------------------------------------------------------------------

	void OnPlayerRPC(PlayerBase player, PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		if (!player || !sender)
			return;

		string uid = sender.GetPlainId();
		string name = sender.GetName();

		switch (rpc_type)
		{
			case SM_DungeonRPC.REQUEST_STATE:
				SendState(player);
				break;
			case SM_DungeonRPC.REQUEST_DUNGEON_LIST:
				SendDungeonList(player);
				break;
			case SM_DungeonRPC.REQUEST_FINDER_LIST:
				SendFinderList(player);
				break;
			case SM_DungeonRPC.REQUEST_ONLINE_PLAYERS:
				SendOnlinePlayers(player);
				break;
			case SM_DungeonRPC.REGISTER_SOLO:
				HandleRegisterSolo(player, uid, name, ctx);
				break;
			case SM_DungeonRPC.REGISTER_GROUP_CREATE:
				HandleRegisterGroup(player, uid, name, ctx, SM_DungeonMode.INVITE_GROUP);
				break;
			case SM_DungeonRPC.FINDER_PUBLISH:
				HandleRegisterGroup(player, uid, name, ctx, SM_DungeonMode.GROUP_FINDER);
				break;
			case SM_DungeonRPC.INVITE_PLAYER:
				HandleInvitePlayer(player, uid, ctx);
				break;
			case SM_DungeonRPC.ACCEPT_INVITE:
				HandleAcceptInvite(player, uid, name, ctx);
				break;
			case SM_DungeonRPC.DECLINE_INVITE:
				HandleDeclineInvite(player, uid, ctx);
				break;
			case SM_DungeonRPC.CANCEL_BOOKING:
				HandleCancelBooking(player, uid, ctx);
				break;
			case SM_DungeonRPC.LEAVE_GROUP:
				HandleLeaveGroup(player, uid, ctx);
				break;
			case SM_DungeonRPC.KICK_MEMBER:
				HandleKickMember(player, uid, ctx);
				break;
			case SM_DungeonRPC.FINDER_JOIN:
				HandleFinderJoin(player, uid, name, ctx);
				break;
			case SM_DungeonRPC.FINDER_UNLIST:
				HandleFinderUnlist(player, uid, ctx);
				break;
			case SM_DungeonRPC.LOOT_ROLL_CHOICE:
				HandleLootRollChoice(uid, ctx);
				break;
			case SM_DungeonRPC.ADMIN_RELOAD_CONFIG:
				HandleAdminReload(player, uid);
				break;
			case SM_DungeonRPC.ADMIN_TOGGLE_DUNGEON:
				HandleAdminToggleDungeon(player, uid, ctx);
				break;
			case SM_DungeonRPC.ADMIN_FORCE_CANCEL:
				HandleAdminForceCancel(player, uid, ctx);
				break;
			case SM_DungeonRPC.ADMIN_PARDON_PLAYER:
				HandleAdminPardon(player, uid, ctx);
				break;
			case SM_DungeonRPC.ADMIN_FORCE_EVAC:
				HandleAdminForceEvac(player, uid, ctx);
				break;
			case SM_DungeonRPC.REQUEST_ADMIN_RUNS:
				if (m_Config.IsAdmin(uid))
					SendAdminRuns(player);
				break;
			case SM_DungeonRPC.REQUEST_ADMIN_PENALTIES:
				if (m_Config.IsAdmin(uid))
					SendAdminPenalties(player);
				break;
		}
	}

	protected void HandleRegisterSolo(PlayerBase player, string uid, string name, ParamsReadContext ctx)
	{
		string dungeonId;
		int difficultyIndex, startMinute;
		if (!ctx.Read(dungeonId)) return;
		if (!ctx.Read(difficultyIndex)) return;
		if (!ctx.Read(startMinute)) return;

		if (!CanRegister(player, uid, dungeonId))
			return;

		SM_DungeonBooking booking = CreateBooking(dungeonId, difficultyIndex, SM_DungeonMode.SOLO, startMinute, uid, name);
		if (!booking)
		{
			Notify(player, "#STR_SMD_ERR_SLOT_TAKEN");
			return;
		}

		Notify(player, "#STR_SMD_NOTIFY_BOOKING_CREATED");
		SendState(player);
	}

	protected void HandleRegisterGroup(PlayerBase player, string uid, string name, ParamsReadContext ctx, int mode)
	{
		string dungeonId;
		int difficultyIndex, startMinute;
		string comment = "";
		if (!ctx.Read(dungeonId)) return;
		if (!ctx.Read(difficultyIndex)) return;
		if (!ctx.Read(startMinute)) return;
		if (mode == SM_DungeonMode.GROUP_FINDER)
		{
			if (!ctx.Read(comment)) return;
		}

		if (!CanRegister(player, uid, dungeonId))
			return;

		SM_DungeonBooking booking = CreateBooking(dungeonId, difficultyIndex, mode, startMinute, uid, name);
		if (!booking)
		{
			Notify(player, "#STR_SMD_ERR_SLOT_TAKEN");
			return;
		}
		booking.FinderComment = comment;
		SaveBookingsDB();

		Notify(player, "#STR_SMD_NOTIFY_BOOKING_CREATED");
		SendState(player);
	}

	protected void HandleInvitePlayer(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string bookingId, targetUid;
		if (!ctx.Read(bookingId)) return;
		if (!ctx.Read(targetUid)) return;

		SM_DungeonBooking booking = FindBooking(bookingId);
		if (!booking || booking.LeaderUid != uid)
		{
			Notify(player, "#STR_SMD_ERR_NOT_LEADER");
			return;
		}
		if (booking.PartySize() >= GetDungeonMaxPlayers(booking.DungeonId))
		{
			Notify(player, "#STR_SMD_ERR_GROUP_FULL");
			return;
		}
		if (booking.HasMember(targetUid))
			return;

		PlayerBase target = GetOnlinePlayerByUid(targetUid);
		if (!target || !target.GetIdentity() || !player.GetIdentity())
			return;

		SM_DungeonInvite invite = new SM_DungeonInvite();
		invite.Id = GenerateId();
		invite.BookingId = bookingId;
		invite.InviterUid = uid;
		invite.InviterName = player.GetIdentity().GetName();
		invite.InviteeUid = targetUid;
		invite.InviteeName = target.GetIdentity().GetName();
		invite.ExpiresAtMinute = SM_DungeonClock.NowMinutes() + m_Config.General.InviteTimeoutSeconds / 60 + 1;
		invite.Status = SM_DungeonInviteStatus.PENDING;
		m_InvitesDB.Invites.Insert(invite);
		SaveInvitesDB();

		Notify(target, "#STR_SMD_NOTIFY_INVITE_RECEIVED" + invite.InviterName);
		SendState(target);
	}

	protected void HandleAcceptInvite(PlayerBase player, string uid, string name, ParamsReadContext ctx)
	{
		string inviteId;
		if (!ctx.Read(inviteId)) return;

		SM_DungeonInvite invite = FindInvite(inviteId);
		if (!invite || invite.InviteeUid != uid)
			return;

		SM_DungeonBooking booking = FindBooking(invite.BookingId);
		RemoveInvite(inviteId);

		if (!booking || !BookingOccupiesCalendar(booking))
		{
			SendState(player);
			return;
		}
		if (booking.PartySize() >= GetDungeonMaxPlayers(booking.DungeonId))
		{
			Notify(player, "#STR_SMD_ERR_GROUP_FULL");
			SendState(player);
			return;
		}
		if (!CanRegister(player, uid, booking.DungeonId))
		{
			SendState(player);
			return;
		}

		booking.MemberUids.Insert(uid);
		booking.MemberNames.Insert(name);
		SaveBookingsDB();
		SendState(player);
	}

	protected void HandleDeclineInvite(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string inviteId;
		if (!ctx.Read(inviteId)) return;

		SM_DungeonInvite invite = FindInvite(inviteId);
		if (!invite || invite.InviteeUid != uid)
			return;

		RemoveInvite(inviteId);
		SendState(player);
	}

	protected void HandleCancelBooking(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string bookingId;
		if (!ctx.Read(bookingId)) return;

		SM_DungeonBooking booking = FindBooking(bookingId);
		if (!booking || booking.LeaderUid != uid)
		{
			Notify(player, "#STR_SMD_ERR_NOT_LEADER");
			return;
		}
		if (booking.Status == SM_DungeonBookingStatus.ACTIVE)
			return;

		booking.Status = SM_DungeonBookingStatus.CANCELLED;
		SaveBookingsDB();
		NotifyBookingMembers(booking, "#STR_SMD_NOTIFY_BOOKING_CANCELLED");
		SendState(player);
	}

	protected void HandleLeaveGroup(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string bookingId;
		if (!ctx.Read(bookingId)) return;

		SM_DungeonBooking booking = FindBooking(bookingId);
		if (!booking)
			return;

		if (booking.Status == SM_DungeonBookingStatus.ACTIVE)
		{
			SM_DungeonRunState run = FindRunByBooking(bookingId);
			if (run)
				HandleEarlyLeave(run, uid);
			return;
		}

		if (booking.LeaderUid == uid)
		{
			if (booking.MemberUids.Count() > 0)
			{
				string newLeaderUid = booking.MemberUids[0];
				string newLeaderName = booking.MemberNames[0];
				booking.RemoveMember(newLeaderUid);
				booking.LeaderUid = newLeaderUid;
				booking.LeaderName = newLeaderName;
			}
			else
			{
				booking.Status = SM_DungeonBookingStatus.CANCELLED;
			}
		}
		else
		{
			booking.RemoveMember(uid);
		}

		SaveBookingsDB();
		SendState(player);
	}

	protected void HandleEarlyLeave(SM_DungeonRunState run, string uid)
	{
		SM_DungeonRunParticipant participant = run.GetParticipant(uid);
		if (!participant || participant.LeftEarly)
			return;

		participant.LeftEarly = true;
		ApplyPenalty(uid, SM_DungeonPenaltyType.EARLY_LEAVE);

		PlayerBase pb = GetOnlinePlayerByUid(uid);
		if (pb)
		{
			pb.SetPosition(participant.OriginalPosition);
			Notify(pb, "#STR_SMD_NOTIFY_EARLY_LEAVE_PENALTY");
			SendState(pb);
		}
	}

	protected void HandleKickMember(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string bookingId, targetUid;
		if (!ctx.Read(bookingId)) return;
		if (!ctx.Read(targetUid)) return;

		SM_DungeonBooking booking = FindBooking(bookingId);
		if (!booking || booking.LeaderUid != uid)
		{
			Notify(player, "#STR_SMD_ERR_NOT_LEADER");
			return;
		}

		booking.RemoveMember(targetUid);
		SaveBookingsDB();

		PlayerBase target = GetOnlinePlayerByUid(targetUid);
		if (target)
		{
			Notify(target, "#STR_SMD_NOTIFY_BOOKING_CANCELLED");
			SendState(target);
		}
		SendState(player);
	}

	protected void HandleFinderJoin(PlayerBase player, string uid, string name, ParamsReadContext ctx)
	{
		string bookingId;
		if (!ctx.Read(bookingId)) return;

		SM_DungeonBooking booking = FindBooking(bookingId);
		if (!booking || !booking.FinderOpen || !BookingOccupiesCalendar(booking))
		{
			Notify(player, "#STR_SMD_ERR_GENERIC");
			return;
		}
		if (booking.HasMember(uid))
			return;
		if (booking.PartySize() >= GetDungeonMaxPlayers(booking.DungeonId))
		{
			Notify(player, "#STR_SMD_ERR_GROUP_FULL");
			return;
		}
		if (!CanRegister(player, uid, booking.DungeonId))
			return;

		booking.MemberUids.Insert(uid);
		booking.MemberNames.Insert(name);
		if (booking.PartySize() >= GetDungeonMaxPlayers(booking.DungeonId))
			booking.FinderOpen = false;

		SaveBookingsDB();
		Notify(player, "#STR_SMD_NOTIFY_BOOKING_CREATED");
		SendState(player);
	}

	protected void HandleFinderUnlist(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string bookingId;
		if (!ctx.Read(bookingId)) return;

		SM_DungeonBooking booking = FindBooking(bookingId);
		if (!booking || booking.LeaderUid != uid)
		{
			Notify(player, "#STR_SMD_ERR_NOT_LEADER");
			return;
		}

		booking.FinderOpen = false;
		SaveBookingsDB();
		SendState(player);
	}

	protected void HandleLootRollChoice(string uid, ParamsReadContext ctx)
	{
		string rollId;
		int choice;
		if (!ctx.Read(rollId)) return;
		if (!ctx.Read(choice)) return;

		foreach (SM_DungeonRunState run : m_ActiveRuns)
		{
			foreach (SM_DungeonLootRoll roll : run.ActiveLootRolls)
			{
				if (roll.Id == rollId && !roll.Resolved && roll.EligibleUids.Find(uid) >= 0)
				{
					roll.Choices.Set(uid, choice);
					return;
				}
			}
		}
	}

	// ------------------------------------------------------------------
	// Админ-действия (гейт по SM_DungeonConfig.IsAdmin)
	// ------------------------------------------------------------------

	protected void HandleAdminReload(PlayerBase player, string uid)
	{
		if (!m_Config.IsAdmin(uid))
			return;

		string message;
		SM_DungeonConfigLoader.Reload(message);
		m_Config = SM_DungeonConfigLoader.Get();
		Notify(player, message);
	}

	protected void HandleAdminToggleDungeon(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string dungeonId;
		int enabledInt;
		if (!ctx.Read(dungeonId)) return;
		if (!ctx.Read(enabledInt)) return;
		if (!m_Config.IsAdmin(uid))
			return;

		SM_DungeonDefinition dungeon = m_Config.GetDungeon(dungeonId);
		if (!dungeon)
			return;

		dungeon.Enabled = enabledInt != 0;
		SaveConfig();
		SendDungeonList(player);
	}

	protected void HandleAdminForceCancel(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string bookingId;
		if (!ctx.Read(bookingId)) return;
		if (!m_Config.IsAdmin(uid))
			return;

		SM_DungeonBooking booking = FindBooking(bookingId);
		if (!booking)
			return;

		if (booking.Status == SM_DungeonBookingStatus.ACTIVE)
		{
			SM_DungeonRunState run = FindRunByBooking(bookingId);
			if (run)
				FailRun(run);
		}
		else
		{
			booking.Status = SM_DungeonBookingStatus.CANCELLED;
			SaveBookingsDB();
		}

		NotifyBookingMembers(booking, "#STR_SMD_NOTIFY_BOOKING_CANCELLED");
		SendAdminRuns(player);
	}

	protected void HandleAdminPardon(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string targetUid;
		if (!ctx.Read(targetUid)) return;
		if (!m_Config.IsAdmin(uid))
			return;

		SM_DungeonPlayerRecord record = GetOrCreatePlayerRecord(targetUid);
		record.TempLockUntilMinute = 0;
		record.PermaBanned = false;
		record.OffenseMinutes.Clear();
		SavePlayersDB();
		SendAdminPenalties(player);
	}

	protected void HandleAdminForceEvac(PlayerBase player, string uid, ParamsReadContext ctx)
	{
		string runId;
		if (!ctx.Read(runId)) return;
		if (!m_Config.IsAdmin(uid))
			return;

		foreach (SM_DungeonRunState run : m_ActiveRuns)
		{
			if (run.RunId == runId && (run.Phase == SM_DungeonRunPhase.ACTIVE || run.Phase == SM_DungeonRunPhase.EVACUATING))
			{
				run.Phase = SM_DungeonRunPhase.EVACUATING;
				run.EvacHoldSecondsRemaining = 0;
				return;
			}
		}
	}

	// ------------------------------------------------------------------
	// Синхронизация клиента
	// ------------------------------------------------------------------

	void SendState(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_DungeonPlayerRecord record = GetOrCreatePlayerRecord(uid, player.GetIdentity().GetName());

		array<ref SM_DungeonBooking> myBookings = new array<ref SM_DungeonBooking>;
		foreach (SM_DungeonBooking booking : m_BookingsDB.Bookings)
		{
			if (booking && BookingOccupiesCalendar(booking) && booking.HasMember(uid))
				myBookings.Insert(booking);
		}

		array<ref SM_DungeonInvite> myInvites = new array<ref SM_DungeonInvite>;
		foreach (SM_DungeonInvite invite : m_InvitesDB.Invites)
		{
			if (invite && invite.InviteeUid == uid && invite.Status == SM_DungeonInviteStatus.PENDING)
				myInvites.Insert(invite);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(BoolToInt(record.PermaBanned));
		rpc.Write(record.TempLockUntilMinute);
		rpc.Write(record.UnlockedDungeonIds.Count());
		foreach (string unlockedId : record.UnlockedDungeonIds)
			rpc.Write(unlockedId);

		rpc.Write(myBookings.Count());
		foreach (SM_DungeonBooking myBooking : myBookings)
		{
			SM_DungeonDefinition dungeon = m_Config.GetDungeon(myBooking.DungeonId);
			string dungeonName = myBooking.DungeonId;
			string difficultyName = "";
			if (dungeon)
			{
				dungeonName = dungeon.Name;
				SM_DungeonDifficultyConfig diff = dungeon.GetDifficulty(myBooking.DifficultyIndex);
				if (diff)
					difficultyName = SM_DungeonLoc.Text(diff.Name);
			}

			rpc.Write(myBooking.Id);
			rpc.Write(myBooking.DungeonId);
			rpc.Write(dungeonName);
			rpc.Write(myBooking.DifficultyIndex);
			rpc.Write(difficultyName);
			rpc.Write(myBooking.Mode);
			rpc.Write(myBooking.StartMinute);
			rpc.Write(myBooking.Status);
			rpc.Write(myBooking.LeaderUid);
			rpc.Write(myBooking.LeaderName);
			rpc.Write(myBooking.PartySize());
			rpc.Write(BoolToInt(myBooking.FinderOpen));
			rpc.Write(myBooking.FinderComment);

			rpc.Write(myBooking.MemberUids.Count());
			for (int m = 0; m < myBooking.MemberUids.Count(); m++)
			{
				rpc.Write(myBooking.MemberUids[m]);
				rpc.Write(myBooking.MemberNames[m]);
			}
		}

		rpc.Write(myInvites.Count());
		foreach (SM_DungeonInvite myInvite : myInvites)
		{
			SM_DungeonBooking inviteBooking = FindBooking(myInvite.BookingId);
			string inviteDungeonName = "";
			string inviteDifficultyName = "";
			if (inviteBooking)
			{
				SM_DungeonDefinition dungeon = m_Config.GetDungeon(inviteBooking.DungeonId);
				if (dungeon)
				{
					inviteDungeonName = dungeon.Name;
					SM_DungeonDifficultyConfig diff = dungeon.GetDifficulty(inviteBooking.DifficultyIndex);
					if (diff)
						inviteDifficultyName = SM_DungeonLoc.Text(diff.Name);
				}
			}

			rpc.Write(myInvite.Id);
			rpc.Write(myInvite.BookingId);
			rpc.Write(inviteDungeonName);
			rpc.Write(inviteDifficultyName);
			rpc.Write(myInvite.InviterName);
			rpc.Write(myInvite.ExpiresAtMinute);
		}

		rpc.Send(player, SM_DungeonRPC.SYNC_STATE, true, player.GetIdentity());
	}

	void SendDungeonList(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_DungeonPlayerRecord record = GetOrCreatePlayerRecord(uid);
		bool isAdmin = m_Config.IsAdmin(uid);

		array<ref SM_DungeonDefinition> visible = new array<ref SM_DungeonDefinition>;
		foreach (SM_DungeonDefinition dungeon : m_Config.Dungeons)
		{
			// Отключённые данжи видят только админы (чтобы иметь возможность
			// снова включить их через вкладку "Данжи") - обычным игрокам
			// незачем видеть выключенный контент в списке.
			if (dungeon && (dungeon.Enabled || isAdmin))
				visible.Insert(dungeon);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(visible.Count());
		foreach (SM_DungeonDefinition visibleDungeon : visible)
		{
			bool locked = visibleDungeon.RequiresUnlock && !record.IsUnlocked(visibleDungeon.Id);

			rpc.Write(visibleDungeon.Id);
			rpc.Write(visibleDungeon.Name);
			rpc.Write(visibleDungeon.Description);
			rpc.Write(BoolToInt(visibleDungeon.Enabled));
			rpc.Write(BoolToInt(locked));
			rpc.Write(visibleDungeon.MinPlayers);
			rpc.Write(visibleDungeon.MaxPlayers);
			rpc.Write(BoolToInt(visibleDungeon.AllowSolo));
			rpc.Write(BoolToInt(visibleDungeon.AllowInviteGroup));
			rpc.Write(BoolToInt(visibleDungeon.AllowGroupFinder));

			rpc.Write(visibleDungeon.Difficulties.Count());
			foreach (SM_DungeonDifficultyConfig diff : visibleDungeon.Difficulties)
				rpc.Write(SM_DungeonLoc.Text(diff.Name));

			array<int> slots;
			ComputeAvailableSlots(visibleDungeon.Id, slots);
			rpc.Write(slots.Count());
			foreach (int slot : slots)
				rpc.Write(slot);
		}

		rpc.Send(player, SM_DungeonRPC.SYNC_DUNGEON_LIST, true, player.GetIdentity());
	}

	void SendFinderList(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		array<ref SM_DungeonBooking> listings = new array<ref SM_DungeonBooking>;
		foreach (SM_DungeonBooking booking : m_BookingsDB.Bookings)
		{
			if (booking && booking.FinderOpen && BookingOccupiesCalendar(booking))
				listings.Insert(booking);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(listings.Count());
		foreach (SM_DungeonBooking listing : listings)
		{
			SM_DungeonDefinition dungeon = m_Config.GetDungeon(listing.DungeonId);
			string dungeonName = listing.DungeonId;
			string difficultyName = "";
			int maxPlayers = 0;
			if (dungeon)
			{
				dungeonName = dungeon.Name;
				maxPlayers = dungeon.MaxPlayers;
				SM_DungeonDifficultyConfig diff = dungeon.GetDifficulty(listing.DifficultyIndex);
				if (diff)
					difficultyName = SM_DungeonLoc.Text(diff.Name);
			}

			rpc.Write(listing.Id);
			rpc.Write(listing.DungeonId);
			rpc.Write(dungeonName);
			rpc.Write(listing.DifficultyIndex);
			rpc.Write(difficultyName);
			rpc.Write(listing.LeaderName);
			rpc.Write(listing.PartySize());
			rpc.Write(maxPlayers);
			rpc.Write(listing.FinderComment);
			rpc.Write(listing.StartMinute);
		}

		rpc.Send(player, SM_DungeonRPC.SYNC_FINDER_LIST, true, player.GetIdentity());
	}

	void SendOnlinePlayers(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string selfUid = player.GetIdentity().GetPlainId();
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		array<string> uids = new array<string>;
		array<string> names = new array<string>;
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;
			string otherUid = pb.GetIdentity().GetPlainId();
			if (otherUid == selfUid)
				continue;
			uids.Insert(otherUid);
			names.Insert(pb.GetIdentity().GetName());
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(uids.Count());
		for (int i = 0; i < uids.Count(); i++)
		{
			rpc.Write(uids[i]);
			rpc.Write(names[i]);
		}
		rpc.Send(player, SM_DungeonRPC.SYNC_ONLINE_PLAYERS, true, player.GetIdentity());
	}

	void SendAdminRuns(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_ActiveRuns.Count());
		foreach (SM_DungeonRunState run : m_ActiveRuns)
		{
			SM_DungeonDefinition dungeon = m_Config.GetDungeon(run.DungeonId);
			string dungeonName = run.DungeonId;
			if (dungeon)
				dungeonName = dungeon.Name;

			rpc.Write(run.RunId);
			rpc.Write(run.BookingId);
			rpc.Write(dungeonName);
			rpc.Write(run.Phase);
			rpc.Write(run.RunTimeSecondsRemaining);
			rpc.Write(run.CountActiveParticipants());
		}
		rpc.Send(player, SM_DungeonRPC.SYNC_ADMIN_RUNS, true, player.GetIdentity());
	}

	void SendAdminPenalties(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		int now = SM_DungeonClock.NowMinutes();
		array<ref SM_DungeonPlayerRecord> flagged = new array<ref SM_DungeonPlayerRecord>;
		foreach (SM_DungeonPlayerRecord record : m_PlayersDB.Players)
		{
			if (record && (record.PermaBanned || record.TempLockUntilMinute > now || record.OffenseMinutes.Count() > 0))
				flagged.Insert(record);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(flagged.Count());
		foreach (SM_DungeonPlayerRecord flaggedRecord : flagged)
		{
			rpc.Write(flaggedRecord.Uid);
			rpc.Write(flaggedRecord.LastKnownName);
			rpc.Write(flaggedRecord.TempLockUntilMinute);
			rpc.Write(BoolToInt(flaggedRecord.PermaBanned));
			rpc.Write(flaggedRecord.OffenseMinutes.Count());
		}
		rpc.Send(player, SM_DungeonRPC.SYNC_ADMIN_PENALTIES, true, player.GetIdentity());
	}

	// ------------------------------------------------------------------
	// Штрафы
	// ------------------------------------------------------------------

	protected void ApplyPenalty(string uid, int penaltyType)
	{
		if (!m_Config.Penalties.Enabled)
			return;

		SM_DungeonPlayerRecord record = GetOrCreatePlayerRecord(uid);
		int now = SM_DungeonClock.NowMinutes();

		int lockMinutes = m_Config.Penalties.NoShowLockMinutes;
		if (penaltyType == SM_DungeonPenaltyType.EARLY_LEAVE)
			lockMinutes = m_Config.Penalties.EarlyLeaveLockMinutes;

		int lockUntil = now + lockMinutes;
		if (lockUntil > record.TempLockUntilMinute)
			record.TempLockUntilMinute = lockUntil;

		record.OffenseMinutes.Insert(now);

		int decayThreshold = now - m_Config.Penalties.OffenseDecayDays * 1440;
		int activeOffenses = 0;
		for (int i = record.OffenseMinutes.Count() - 1; i >= 0; i--)
		{
			if (record.OffenseMinutes[i] < decayThreshold)
				record.OffenseMinutes.Remove(i);
			else
				activeOffenses++;
		}

		if (activeOffenses >= m_Config.Penalties.OffensesBeforePermaBan)
			record.PermaBanned = true;

		SavePlayersDB();

		PlayerBase pb = GetOnlinePlayerByUid(uid);
		if (pb)
		{
			if (record.PermaBanned)
				Notify(pb, "#STR_SMD_NOTIFY_PERMA_BAN");
			else if (penaltyType == SM_DungeonPenaltyType.NO_SHOW)
				Notify(pb, "#STR_SMD_NOTIFY_NO_SHOW_PENALTY");
		}
	}

	// ------------------------------------------------------------------
	// Планировщик: старт забегов по расписанию, очистка старых записей
	// ------------------------------------------------------------------

	protected void SchedulerTick()
	{
		int now = SM_DungeonClock.NowMinutes();
		int warnMinutesBefore = (m_Config.Notifications.WarnBeforeStartSeconds + 59) / 60;
		if (warnMinutesBefore < 1)
			warnMinutesBefore = 1;

		for (int i = m_BookingsDB.Bookings.Count() - 1; i >= 0; i--)
		{
			SM_DungeonBooking booking = m_BookingsDB.Bookings[i];
			if (!booking || booking.Status != SM_DungeonBookingStatus.CONFIRMED)
				continue;

			if (!booking.Warned && now >= booking.StartMinute - warnMinutesBefore)
			{
				booking.Warned = true;
				NotifyBookingMembers(booking, "#STR_SMD_NOTIFY_RUN_STARTING_SOON");
				SaveBookingsDB();
			}

			// Как только время брони настало, ждём ещё до NoShowGraceMinutes,
			// пока не соберутся все записавшиеся - но если все уже онлайн,
			// не тянем время и стартуем сразу.
			if (now >= booking.StartMinute)
			{
				if (AllMembersOnline(booking) || now >= booking.StartMinute + m_Config.Notifications.NoShowGraceMinutes)
					TryStartRun(booking);
			}
		}

		CleanupOldBookings(now);
		CleanupExpiredInvites(now);
	}

	protected bool AllMembersOnline(SM_DungeonBooking booking)
	{
		if (!GetOnlinePlayerByUid(booking.LeaderUid))
			return false;

		foreach (string memberUid : booking.MemberUids)
		{
			if (!GetOnlinePlayerByUid(memberUid))
				return false;
		}

		return true;
	}

	protected void CleanupOldBookings(int now)
	{
		bool changed = false;
		for (int i = m_BookingsDB.Bookings.Count() - 1; i >= 0; i--)
		{
			SM_DungeonBooking booking = m_BookingsDB.Bookings[i];
			if (!booking)
			{
				m_BookingsDB.Bookings.Remove(i);
				changed = true;
				continue;
			}

			bool finished = booking.Status == SM_DungeonBookingStatus.CANCELLED
				|| booking.Status == SM_DungeonBookingStatus.COMPLETED
				|| booking.Status == SM_DungeonBookingStatus.NO_SHOW
				|| booking.Status == SM_DungeonBookingStatus.EXPIRED;

			if (finished && now - booking.EndMinute > 2 * 1440)
			{
				m_BookingsDB.Bookings.Remove(i);
				changed = true;
			}
		}

		if (changed)
			SaveBookingsDB();
	}

	protected void CleanupExpiredInvites(int now)
	{
		bool changed = false;
		for (int i = m_InvitesDB.Invites.Count() - 1; i >= 0; i--)
		{
			SM_DungeonInvite invite = m_InvitesDB.Invites[i];
			if (!invite || invite.ExpiresAtMinute < now)
			{
				m_InvitesDB.Invites.Remove(i);
				changed = true;
			}
		}

		if (changed)
			SaveInvitesDB();
	}

	protected void TryStartRun(SM_DungeonBooking booking)
	{
		SM_DungeonDefinition dungeon = m_Config.GetDungeon(booking.DungeonId);
		if (!dungeon)
		{
			booking.Status = SM_DungeonBookingStatus.CANCELLED;
			SaveBookingsDB();
			return;
		}

		array<string> allUids = new array<string>;
		allUids.Insert(booking.LeaderUid);
		foreach (string memberUid : booking.MemberUids)
			allUids.Insert(memberUid);

		array<PlayerBase> present = new array<PlayerBase>;
		array<string> absent = new array<string>;
		foreach (string participantUid : allUids)
		{
			PlayerBase pb = GetOnlinePlayerByUid(participantUid);
			if (pb)
				present.Insert(pb);
			else
				absent.Insert(participantUid);
		}

		foreach (string absentUid : absent)
			ApplyPenalty(absentUid, SM_DungeonPenaltyType.NO_SHOW);

		if (present.Count() < dungeon.MinPlayers)
		{
			booking.Status = SM_DungeonBookingStatus.NO_SHOW;
			SaveBookingsDB();
			foreach (PlayerBase notEnoughPb : present)
			{
				Notify(notEnoughPb, "#STR_SMD_ERR_NOT_ENOUGH_PLAYERS");
				SendState(notEnoughPb);
			}
			return;
		}

		StartRun(booking, dungeon, present);
	}

	protected void StartRun(SM_DungeonBooking booking, SM_DungeonDefinition dungeon, array<PlayerBase> present)
	{
		booking.Status = SM_DungeonBookingStatus.ACTIVE;
		SaveBookingsDB();

		SM_DungeonRunState run = new SM_DungeonRunState();
		run.RunId = GenerateId();
		run.BookingId = booking.Id;
		run.DungeonId = booking.DungeonId;
		run.DifficultyIndex = booking.DifficultyIndex;
		run.Phase = SM_DungeonRunPhase.COUNTDOWN;
		run.PhaseSecondsRemaining = m_Config.Notifications.CountdownSeconds;
		run.RunTimeSecondsRemaining = dungeon.RunTimeLimitMinutes * 60;

		foreach (PlayerBase pb : present)
		{
			if (!pb.GetIdentity())
				continue;

			SM_DungeonRunParticipant participant = new SM_DungeonRunParticipant();
			participant.Uid = pb.GetIdentity().GetPlainId();
			participant.Name = pb.GetIdentity().GetName();
			run.Participants.Insert(participant);
			SendState(pb);
		}

		m_ActiveRuns.Insert(run);
	}

	// ------------------------------------------------------------------
	// Жизненный цикл активного забега (тик раз в секунду)
	// ------------------------------------------------------------------

	protected void RunTick()
	{
		for (int i = m_ActiveRuns.Count() - 1; i >= 0; i--)
		{
			SM_DungeonRunState run = m_ActiveRuns[i];
			if (!run)
			{
				m_ActiveRuns.Remove(i);
				continue;
			}

			TickRun(run);

			if (RunFullyFinished(run))
			{
				CleanupRun(run);
				m_ActiveRuns.Remove(i);
			}
		}
	}

	protected void TickRun(SM_DungeonRunState run)
	{
		SM_DungeonDefinition dungeon = m_Config.GetDungeon(run.DungeonId);
		if (!dungeon)
		{
			// Данж пропал из конфига (переименован/удалён через !dungeonreload)
			// пока забег шёл - используем тот же путь очистки, что и обычный
			// провал: участники телепортируются обратно, бронь закрывается.
			FailRun(run);
			return;
		}

		switch (run.Phase)
		{
			case SM_DungeonRunPhase.COUNTDOWN:
				CountdownTick(run, dungeon);
				break;
			case SM_DungeonRunPhase.ACTIVE:
			case SM_DungeonRunPhase.EVACUATING:
				ActivePhaseTick(run, dungeon);
				break;
			case SM_DungeonRunPhase.COMPLETED:
				LootRollTick(run);
				break;
		}
	}

	protected void CountdownTick(SM_DungeonRunState run, SM_DungeonDefinition dungeon)
	{
		BroadcastRunNotify(run, SM_DungeonRunPhase.COUNTDOWN, run.PhaseSecondsRemaining, dungeon.Name);
		run.PhaseSecondsRemaining--;
		if (run.PhaseSecondsRemaining > 0)
			return;

		TeleportParticipantsIn(run, dungeon);
		SpawnExtraContent(run, dungeon);
		run.Phase = SM_DungeonRunPhase.ACTIVE;
		BroadcastRunNotify(run, SM_DungeonRunPhase.ACTIVE, 0, dungeon.Name);

		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			PlayerBase pb = GetOnlinePlayerByUid(participant.Uid);
			if (pb)
				Notify(pb, "#STR_SMD_NOTIFY_RUN_STARTED");
		}
	}

	protected void TeleportParticipantsIn(SM_DungeonRunState run, SM_DungeonDefinition dungeon)
	{
		int slot = 0;
		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			PlayerBase pb = GetOnlinePlayerByUid(participant.Uid);
			if (!pb)
			{
				participant.Disconnected = true;
				continue;
			}

			participant.OriginalPosition = pb.GetPosition();
			vector currentOrientation = pb.GetOrientation();
			participant.OriginalYaw = currentOrientation[0];

			vector destination = dungeon.Entry.GetSlotPosition(slot);
			pb.SetPosition(destination);
			pb.SetOrientation(Vector(dungeon.Entry.OrientationYaw, 0, 0));
			Notify(pb, "#STR_SMD_NOTIFY_TELEPORTING");
			slot++;
		}
	}

	protected void SpawnExtraContent(SM_DungeonRunState run, SM_DungeonDefinition dungeon)
	{
		if (!dungeon.ExtraContent || !dungeon.ExtraContent.Enabled)
			return;

		array<ref SM_DungeonSpawnPointConfig> points;
		SM_DungeonLocationImporter.BuildSpawnPoints(dungeon.ExtraContent, points);

		foreach (SM_DungeonSpawnPointConfig point : points)
		{
			if (!point || point.ClassName == "")
				continue;

			Object obj = GetGame().CreateObjectEx(point.ClassName, point.Position, ECE_NONE);
			EntityAI entity = EntityAI.Cast(obj);
			if (!entity)
				continue;

			entity.SetPosition(point.Position);
			entity.SetOrientation(point.Orientation);
			run.SpawnedEntities.Insert(entity);
		}
	}

	protected void ActivePhaseTick(SM_DungeonRunState run, SM_DungeonDefinition dungeon)
	{
		run.RunTimeSecondsRemaining--;
		if (run.RunTimeSecondsRemaining <= 0)
		{
			FailRun(run);
			return;
		}

		ZoneProtectionTick(run, dungeon);
		EvacCheckTick(run, dungeon);
		LootRollTick(run);
	}

	protected void ZoneProtectionTick(SM_DungeonRunState run, SM_DungeonDefinition dungeon)
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;

			string uid = pb.GetIdentity().GetPlainId();
			if (run.GetParticipant(uid))
				continue;
			if (!dungeon.Zone.IsInside(pb.GetPosition()))
				continue;

			PushPlayerOutOfZone(pb, dungeon.Zone);
			Notify(pb, "#STR_SMD_NOTIFY_ZONE_KICK");
		}
	}

	protected void PushPlayerOutOfZone(PlayerBase player, SM_DungeonZoneConfig zone)
	{
		vector pos = player.GetPosition();
		vector dir = pos - zone.Center;
		dir[1] = 0;

		float len = dir.Length();
		if (len < 0.01)
			dir = Vector(1, 0, 0);
		else
			dir = dir / len;

		vector target = zone.Center + dir * (zone.RadiusMeters + 5);
		target[1] = pos[1];
		player.SetPosition(target);
	}

	protected void EvacCheckTick(SM_DungeonRunState run, SM_DungeonDefinition dungeon)
	{
		bool allReady = true;
		int requiredCount = 0;

		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			if (!participant || participant.Disconnected || participant.LeftEarly)
				continue;

			PlayerBase pb = GetOnlinePlayerByUid(participant.Uid);
			if (!pb || !pb.IsAlive())
				continue;

			requiredCount++;
			float dist = vector.Distance(pb.GetPosition(), dungeon.Evacuation.ExitPosition);
			if (dist > dungeon.Evacuation.RadiusMeters)
				allReady = false;
		}

		if (requiredCount == 0)
		{
			FailRun(run);
			return;
		}

		if (allReady)
		{
			if (run.Phase != SM_DungeonRunPhase.EVACUATING)
			{
				run.Phase = SM_DungeonRunPhase.EVACUATING;
				run.EvacHoldSecondsRemaining = dungeon.Evacuation.HoldSeconds;
				BroadcastLocalNotify(run, "#STR_SMD_NOTIFY_EVAC_STARTED");
			}

			run.EvacHoldSecondsRemaining--;
			BroadcastEvacProgress(run, true, run.EvacHoldSecondsRemaining, dungeon.Evacuation.HoldSeconds);

			if (run.EvacHoldSecondsRemaining <= 0)
				CompleteRun(run, dungeon);
		}
		else
		{
			if (run.Phase == SM_DungeonRunPhase.EVACUATING)
			{
				run.Phase = SM_DungeonRunPhase.ACTIVE;
				BroadcastLocalNotify(run, "#STR_SMD_NOTIFY_EVAC_INTERRUPTED");
			}
			BroadcastEvacProgress(run, false, 0, dungeon.Evacuation.HoldSeconds);
		}
	}

	protected void CompleteRun(SM_DungeonRunState run, SM_DungeonDefinition dungeon)
	{
		run.Phase = SM_DungeonRunPhase.COMPLETED;
		BroadcastRunNotify(run, SM_DungeonRunPhase.COMPLETED, 0, dungeon.Name);

		SM_DungeonDifficultyConfig difficulty = dungeon.GetDifficulty(run.DifficultyIndex);

		array<string> survivorUids = new array<string>;
		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			if (participant && !participant.Disconnected && !participant.LeftEarly)
				survivorUids.Insert(participant.Uid);
		}

		foreach (string survivorUid : survivorUids)
		{
			SM_DungeonPlayerRecord record = GetOrCreatePlayerRecord(survivorUid);
			if (difficulty && difficulty.UnlocksDungeonIds)
			{
				foreach (string unlockId : difficulty.UnlocksDungeonIds)
				{
					bool wasNew = !record.IsUnlocked(unlockId);
					record.Unlock(unlockId);
					if (wasNew)
					{
						PlayerBase pb = GetOnlinePlayerByUid(survivorUid);
						if (pb)
							Notify(pb, "#STR_SMD_NOTIFY_UNLOCK_GRANTED");
					}
				}
			}
			record.SetCooldownUntil(dungeon.Id, SM_DungeonClock.NowMinutes() + dungeon.CooldownMinutesPerPlayer);
		}
		SavePlayersDB();

		if (difficulty && difficulty.Loot)
		{
			foreach (SM_DungeonLootItemConfig loot : difficulty.Loot)
			{
				if (!loot || loot.ClassName == "" || survivorUids.Count() == 0)
					continue;

				float chance = loot.DropChance * difficulty.DropChanceMultiplier;
				if (Math.RandomFloat01() > chance)
					continue;

				int qty = loot.MinQuantity;
				if (loot.MaxQuantity > loot.MinQuantity)
					qty = Math.RandomIntInclusive(loot.MinQuantity, loot.MaxQuantity);

				if (loot.NeedGreedRoll && survivorUids.Count() > 1)
				{
					StartLootRoll(run, loot.ClassName, qty, survivorUids);
				}
				else
				{
					string luckyUid = survivorUids[Math.RandomIntInclusive(0, survivorUids.Count() - 1)];
					PlayerBase luckyPb = GetOnlinePlayerByUid(luckyUid);
					if (luckyPb)
						GrantTicket(luckyPb, loot.ClassName, qty);
				}
			}
		}

		TeleportParticipantsBack(run);
	}

	protected void TeleportParticipantsBack(SM_DungeonRunState run)
	{
		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			if (!participant || participant.Disconnected || participant.LeftEarly)
				continue;

			PlayerBase pb = GetOnlinePlayerByUid(participant.Uid);
			if (!pb)
				continue;

			pb.SetPosition(participant.OriginalPosition);
			pb.SetOrientation(Vector(participant.OriginalYaw, 0, 0));
			Notify(pb, "#STR_SMD_NOTIFY_RUN_COMPLETE");
		}

		SM_DungeonBooking booking = FindBooking(run.BookingId);
		if (booking)
			booking.Status = SM_DungeonBookingStatus.COMPLETED;
		SaveBookingsDB();
	}

	protected void FailRun(SM_DungeonRunState run)
	{
		run.Phase = SM_DungeonRunPhase.FAILED;

		SM_DungeonDefinition dungeon = m_Config.GetDungeon(run.DungeonId);
		string dungeonName = run.DungeonId;
		if (dungeon)
			dungeonName = dungeon.Name;

		BroadcastRunNotify(run, SM_DungeonRunPhase.FAILED, 0, dungeonName);

		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			if (!participant || participant.Disconnected || participant.LeftEarly)
				continue;

			PlayerBase pb = GetOnlinePlayerByUid(participant.Uid);
			if (pb)
				pb.SetPosition(participant.OriginalPosition);
		}

		SM_DungeonBooking booking = FindBooking(run.BookingId);
		if (booking)
			booking.Status = SM_DungeonBookingStatus.CANCELLED;
		SaveBookingsDB();
	}

	protected bool RunFullyFinished(SM_DungeonRunState run)
	{
		if (run.Phase == SM_DungeonRunPhase.FAILED)
			return true;
		if (run.Phase == SM_DungeonRunPhase.COMPLETED && run.ActiveLootRolls.Count() == 0)
			return true;
		return false;
	}

	protected void CleanupRun(SM_DungeonRunState run)
	{
		foreach (EntityAI entity : run.SpawnedEntities)
		{
			if (entity)
				entity.Delete();
		}
		run.SpawnedEntities.Clear();
	}

	// ------------------------------------------------------------------
	// Добыча: Нужно / Не откажусь / Откажусь
	// ------------------------------------------------------------------

	protected void StartLootRoll(SM_DungeonRunState run, string className, int qty, array<string> eligibleUids)
	{
		SM_DungeonLootRoll roll = new SM_DungeonLootRoll();
		roll.Id = run.RunId + "_" + run.ActiveLootRolls.Count() + "_" + m_NextIdCounter;
		m_NextIdCounter++;
		roll.ItemClassName = className;
		roll.Quantity = qty;
		roll.EligibleUids = new array<string>;
		foreach (string eligibleUid : eligibleUids)
			roll.EligibleUids.Insert(eligibleUid);
		roll.SecondsRemaining = LOOT_ROLL_SECONDS;

		run.ActiveLootRolls.Insert(roll);
		BroadcastLootRollTick(run, roll);
	}

	protected void LootRollTick(SM_DungeonRunState run)
	{
		for (int i = run.ActiveLootRolls.Count() - 1; i >= 0; i--)
		{
			SM_DungeonLootRoll roll = run.ActiveLootRolls[i];
			roll.SecondsRemaining--;

			bool allChose = true;
			foreach (string eligibleUid : roll.EligibleUids)
			{
				if (!roll.Choices.Contains(eligibleUid))
				{
					allChose = false;
					break;
				}
			}

			if (roll.SecondsRemaining <= 0 || allChose)
			{
				ResolveLootRoll(run, roll);
				run.ActiveLootRolls.Remove(i);
			}
			else
			{
				BroadcastLootRollTick(run, roll);
			}
		}
	}

	protected void ResolveLootRoll(SM_DungeonRunState run, SM_DungeonLootRoll roll)
	{
		array<string> needers = new array<string>;
		array<string> greeders = new array<string>;
		foreach (string uid : roll.EligibleUids)
		{
			int choice = SM_DungeonLootChoice.PASS;
			if (roll.Choices.Contains(uid))
				choice = roll.Choices.Get(uid);

			if (choice == SM_DungeonLootChoice.NEED)
				needers.Insert(uid);
			else if (choice == SM_DungeonLootChoice.GREED)
				greeders.Insert(uid);
		}

		array<string> pool = needers;
		if (pool.Count() == 0)
			pool = greeders;

		bool wasTie = pool.Count() > 1;
		string winnerName = "-";
		bool granted = false;

		while (pool.Count() > 0 && !granted)
		{
			int pickIndex = Math.RandomIntInclusive(0, pool.Count() - 1);
			string candidateUid = pool[pickIndex];
			PlayerBase pb = GetOnlinePlayerByUid(candidateUid);
			if (pb)
			{
				GrantTicket(pb, roll.ItemClassName, roll.Quantity);
				if (pb.GetIdentity())
					winnerName = pb.GetIdentity().GetName();
				roll.WinnerUid = candidateUid;
				granted = true;
			}
			else
			{
				pool.Remove(pickIndex);
			}
		}

		roll.WasTie = wasTie;
		roll.Resolved = true;
		BroadcastLootRollResult(roll, winnerName, wasTie);
	}

	protected void GrantTicket(PlayerBase player, string className, int qty)
	{
		if (!player)
			return;

		array<ref SM_DungeonTicketLootEntry> payload = new array<ref SM_DungeonTicketLootEntry>;
		payload.Insert(new SM_DungeonTicketLootEntry(className, qty));

		string ticketClass = m_Config.Ticket.ItemClassName;
		EntityAI spawned = player.GetInventory().CreateInInventory(ticketClass);
		if (!spawned)
			spawned = EntityAI.Cast(GetGame().CreateObjectEx(ticketClass, player.GetPosition(), ECE_PLACE_ON_SURFACE));

		SM_DungeonRewardTicket ticket = SM_DungeonRewardTicket.Cast(spawned);
		if (ticket)
			ticket.SetLootPayload(payload);

		Notify(player, "#STR_SMD_NOTIFY_TICKET_RECEIVED");
	}

	// ------------------------------------------------------------------
	// Широковещательные сообщения об активном забеге
	// ------------------------------------------------------------------

	protected void BroadcastRunNotify(SM_DungeonRunState run, int phase, int secondsRemaining, string dungeonName)
	{
		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			if (!participant || participant.Disconnected || participant.LeftEarly)
				continue;

			PlayerBase pb = GetOnlinePlayerByUid(participant.Uid);
			if (!pb || !pb.GetIdentity())
				continue;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(phase);
			rpc.Write(secondsRemaining);
			rpc.Write(dungeonName);
			rpc.Send(pb, SM_DungeonRPC.SYNC_RUN_NOTIFY, true, pb.GetIdentity());
		}
	}

	protected void BroadcastEvacProgress(SM_DungeonRunState run, bool active, int secondsRemaining, int totalSeconds)
	{
		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			if (!participant || participant.Disconnected || participant.LeftEarly)
				continue;

			PlayerBase pb = GetOnlinePlayerByUid(participant.Uid);
			if (!pb || !pb.GetIdentity())
				continue;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(BoolToInt(active));
			rpc.Write(secondsRemaining);
			rpc.Write(totalSeconds);
			rpc.Send(pb, SM_DungeonRPC.SYNC_EVAC_PROGRESS, true, pb.GetIdentity());
		}
	}

	protected void BroadcastLocalNotify(SM_DungeonRunState run, string textKey)
	{
		foreach (SM_DungeonRunParticipant participant : run.Participants)
		{
			if (!participant || participant.Disconnected || participant.LeftEarly)
				continue;

			PlayerBase pb = GetOnlinePlayerByUid(participant.Uid);
			if (pb)
				Notify(pb, textKey);
		}
	}

	protected void BroadcastLootRollTick(SM_DungeonRunState run, SM_DungeonLootRoll roll)
	{
		foreach (string uid : roll.EligibleUids)
		{
			PlayerBase pb = GetOnlinePlayerByUid(uid);
			if (!pb || !pb.GetIdentity())
				continue;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(roll.Id);
			rpc.Write(roll.ItemClassName);
			rpc.Write(roll.Quantity);
			rpc.Write(roll.SecondsRemaining);
			rpc.Send(pb, SM_DungeonRPC.SYNC_LOOT_ROLL, true, pb.GetIdentity());
		}
	}

	protected void BroadcastLootRollResult(SM_DungeonLootRoll roll, string winnerName, bool wasTie)
	{
		foreach (string uid : roll.EligibleUids)
		{
			PlayerBase pb = GetOnlinePlayerByUid(uid);
			if (!pb || !pb.GetIdentity())
				continue;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(roll.Id);
			rpc.Write(roll.ItemClassName);
			rpc.Write(roll.Quantity);
			rpc.Write(winnerName);
			rpc.Write(BoolToInt(wasTie));
			rpc.Send(pb, SM_DungeonRPC.SYNC_LOOT_ROLL_RESULT, true, pb.GetIdentity());
		}
	}

	// ------------------------------------------------------------------
	// Хуки жизненного цикла игрока (см. modded MissionServer)
	// ------------------------------------------------------------------

	// Позволяет админам перезагрузить Settings.json командой в обычном
	// игровом чате (!dungeonreload / !smdreload), без открытия меню -
	// удобно, если конфиг правится прямо во время наблюдения за сервером.
	void OnServerChatMessage(string senderName, string text)
	{
		if (!SM_DungeonAdminCommands.IsReloadConfigCommand(text))
			return;

		PlayerBase player = FindUniqueOnlinePlayerByName(senderName);
		if (!player || !player.GetIdentity())
			return;

		HandleAdminReload(player, player.GetIdentity().GetPlainId());
	}

	protected PlayerBase FindUniqueOnlinePlayerByName(string name)
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		PlayerBase found = NULL;
		int matches = 0;
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;
			if (pb.GetIdentity().GetName() != name)
				continue;

			found = pb;
			matches++;
		}

		if (matches == 1)
			return found;
		return NULL;
	}

	void OnPlayerDisconnected(string uid)
	{
		if (uid == "")
			return;

		foreach (SM_DungeonRunState run : m_ActiveRuns)
		{
			SM_DungeonRunParticipant participant = run.GetParticipant(uid);
			if (!participant || participant.Disconnected || participant.LeftEarly)
				continue;

			participant.Disconnected = true;
			if (run.Phase == SM_DungeonRunPhase.ACTIVE || run.Phase == SM_DungeonRunPhase.EVACUATING)
				ApplyPenalty(uid, SM_DungeonPenaltyType.EARLY_LEAVE);
		}
	}
}
