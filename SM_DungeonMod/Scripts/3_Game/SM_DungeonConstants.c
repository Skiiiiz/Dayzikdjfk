const int SM_DUNGEON_MENU_ID = 78901;
const int SM_DUNGEON_LOOTROLL_MENU_ID = 78902;

class SM_DungeonRPC
{
	static const int RANGE_START = 887900;
	static const int RANGE_END   = 887999;

	// ---- клиент -> сервер ----
	static const int REQUEST_STATE            = 887901;
	static const int REQUEST_DUNGEON_LIST     = 887902;
	static const int REGISTER_SOLO            = 887903;
	static const int REGISTER_GROUP_CREATE    = 887904;
	static const int INVITE_PLAYER            = 887905;
	static const int ACCEPT_INVITE            = 887906;
	static const int DECLINE_INVITE           = 887907;
	static const int CANCEL_BOOKING           = 887908;
	static const int LEAVE_GROUP              = 887909;
	static const int KICK_MEMBER              = 887910;
	static const int FINDER_PUBLISH           = 887911;
	static const int FINDER_JOIN              = 887912;
	static const int FINDER_UNLIST            = 887913;
	static const int REQUEST_FINDER_LIST      = 887914;
	static const int LOOT_ROLL_CHOICE         = 887915;
	static const int ADMIN_RELOAD_CONFIG      = 887916;
	static const int ADMIN_TOGGLE_DUNGEON     = 887917;
	static const int ADMIN_FORCE_CANCEL       = 887918;
	static const int ADMIN_PARDON_PLAYER      = 887919;
	static const int REQUEST_ADMIN_RUNS       = 887920;
	static const int REQUEST_ADMIN_PENALTIES  = 887921;
	static const int ADMIN_FORCE_EVAC         = 887923;
	static const int REQUEST_ONLINE_PLAYERS   = 887924;

	// ---- сервер -> клиент ----
	static const int SYNC_STATE               = 887950;
	static const int SYNC_DUNGEON_LIST        = 887951;
	static const int SYNC_FINDER_LIST         = 887952;
	static const int SYNC_LOOT_ROLL           = 887953;
	static const int SYNC_LOOT_ROLL_RESULT    = 887954;
	static const int SYNC_RUN_NOTIFY          = 887955;
	static const int SYNC_ADMIN_RUNS          = 887956;
	static const int SYNC_ADMIN_PENALTIES     = 887957;
	static const int LOCAL_NOTIFY             = 887958;
	static const int SYNC_EVAC_PROGRESS       = 887959;
	static const int SYNC_ONLINE_PLAYERS      = 887960;
}

class SM_DungeonMode
{
	static const int SOLO         = 0;
	static const int INVITE_GROUP = 1;
	static const int GROUP_FINDER = 2;
	static const int COUNT        = 3;

	static string ToLabel(int mode)
	{
		if (mode == INVITE_GROUP)
			return SM_DungeonLoc.Text("#STR_SMD_MODE_GROUP");
		if (mode == GROUP_FINDER)
			return SM_DungeonLoc.Text("#STR_SMD_MODE_FINDER");
		return SM_DungeonLoc.Text("#STR_SMD_MODE_SOLO");
	}
}

class SM_DungeonBookingStatus
{
	static const int PENDING   = 0; // группа набирается / ждём подтверждения
	static const int CONFIRMED = 1; // забронировано, ждём времени старта
	static const int ACTIVE    = 2; // забег идёт
	static const int COMPLETED = 3;
	static const int CANCELLED = 4;
	static const int NO_SHOW   = 5;
	static const int EXPIRED   = 6;

	static string ToLabel(int status)
	{
		if (status == CONFIRMED)
			return SM_DungeonLoc.Text("#STR_SMD_STATUS_CONFIRMED");
		if (status == ACTIVE)
			return SM_DungeonLoc.Text("#STR_SMD_STATUS_ACTIVE");
		if (status == COMPLETED)
			return SM_DungeonLoc.Text("#STR_SMD_STATUS_COMPLETED");
		if (status == CANCELLED)
			return SM_DungeonLoc.Text("#STR_SMD_STATUS_CANCELLED");
		if (status == NO_SHOW)
			return SM_DungeonLoc.Text("#STR_SMD_STATUS_NO_SHOW");
		if (status == EXPIRED)
			return SM_DungeonLoc.Text("#STR_SMD_STATUS_EXPIRED");
		return SM_DungeonLoc.Text("#STR_SMD_STATUS_PENDING");
	}
}

class SM_DungeonRunPhase
{
	static const int NONE        = 0;
	static const int COUNTDOWN   = 1;
	static const int TELEPORTING = 2;
	static const int ACTIVE      = 3;
	static const int EVACUATING  = 4;
	static const int COMPLETED   = 5;
	static const int FAILED      = 6;

	static string ToLabel(int phase)
	{
		if (phase == COUNTDOWN || phase == TELEPORTING)
			return SM_DungeonLoc.Text("#STR_SMD_PHASE_COUNTDOWN");
		if (phase == ACTIVE)
			return SM_DungeonLoc.Text("#STR_SMD_PHASE_ACTIVE");
		if (phase == EVACUATING)
			return SM_DungeonLoc.Text("#STR_SMD_PHASE_EVACUATING");
		if (phase == COMPLETED)
			return SM_DungeonLoc.Text("#STR_SMD_PHASE_COMPLETED");
		if (phase == FAILED)
			return SM_DungeonLoc.Text("#STR_SMD_PHASE_FAILED");
		return "";
	}
}

class SM_DungeonLootChoice
{
	static const int NONE  = 0;
	static const int NEED  = 1;
	static const int GREED = 2;
	static const int PASS  = 3;
}

class SM_DungeonSpawnType
{
	static const int STATIC_OBJECT = 0;
	static const int INFECTED      = 1;
	static const int ANIMAL        = 2;
}

class SM_DungeonInviteStatus
{
	static const int PENDING  = 0;
	static const int ACCEPTED = 1;
	static const int DECLINED = 2;
	static const int EXPIRED  = 3;
}

class SM_DungeonPenaltyType
{
	static const int NONE        = 0;
	static const int NO_SHOW     = 1;
	static const int EARLY_LEAVE = 2;
}

class SM_DungeonAdminCommands
{
	protected static bool MatchesCommand(string text, string command)
	{
		string check = text;
		check = check.Trim();
		check.ToLower();

		if (check == command)
			return true;

		string prefix = command + " ";
		if (check.IndexOf(prefix) == 0)
			return true;

		return false;
	}

	static bool IsReloadConfigCommand(string text)
	{
		return MatchesCommand(text, "!dungeonreload") || MatchesCommand(text, "!smdreload");
	}
}
