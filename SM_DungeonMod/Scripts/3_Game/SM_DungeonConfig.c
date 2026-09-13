// ============================================================================
// SM_DungeonConfig.c
//
// Вся настройка мода хранится в одном JSON-файле:
//   $profile:SM_DungeonMod\Settings.json
// Файл создаётся автоматически при первом старте сервера со значениями
// по умолчанию (см. SM_DungeonConfig.Validate()). Именно этот файл и есть
// "админка" мода — он читается и валидируется заново командой
// !dungeonreload (см. SM_DungeonAdminCommands) без рестарта сервера.
//
// Оперативные административные действия (форс-старт/отмена брони, помилование
// игрока, вкл/выкл конкретного данжа) выполняются прямо в интерфейсе мода
// (вкладка "Админ"), доступной игрокам из General.AdminUids.
// ============================================================================

class SM_DungeonGeneralConfig
{
	bool Enabled = true;
	ref array<string> AdminUids = {"REPLACE_WITH_STEAMID64"};
	int MaxActiveBookingsPerPlayer = 1;
	int InviteTimeoutSeconds = 180;
	int GroupFinderListingTimeoutSeconds = 900;
	int ConfigReloadCooldownSeconds = 30;
}

class SM_DungeonNotificationConfig
{
	int WarnBeforeStartSeconds = 60;  // "Забег начнётся через ..." предупреждение
	int CountdownSeconds = 10;        // финальный отсчёт перед телепортацией
	int NoShowGraceMinutes = 10;      // сколько ждём игрока после времени старта, прежде чем считать неявкой
}

class SM_DungeonBookingConfig
{
	int BookingWindowDays = 7;                 // на сколько дней вперёд можно бронировать
	int DayStartHour = 8;                      // с какого часа суток доступны слоты
	int DayEndHour = 24;                       // до какого часа суток доступны слоты (не включительно)
	int SlotDurationMinutes = 60;              // длительность одного слота/забега
	int SlotIntervalMinutes = 60;              // шаг между стартами слотов
	int RestartBufferMinutes = 20;             // слоты, пересекающиеся с рестартом +/- буфер, скрываются
	ref array<string> RestartTimesUTC = {"04:00", "10:00", "16:00", "22:00"};
}

class SM_DungeonPenaltyConfig
{
	bool Enabled = true;
	int NoShowLockMinutes = 60;         // временная блокировка очереди за неявку
	int EarlyLeaveLockMinutes = 120;    // временная блокировка за досрочный выход
	int OffensesBeforePermaBan = 5;     // после скольких (незабытых) нарушений — постоянный бан очереди
	int OffenseDecayDays = 14;          // нарушения старше этого срока не учитываются в перманентный бан
}

class SM_DungeonTicketConfig
{
	string ItemClassName = "SM_DungeonRewardTicket";
}

class SM_DungeonZoneConfig
{
	vector Center = "0 0 0";
	float RadiusMeters = 60;
	float HeightMeters = 80; // вертикальная половина цилиндра защиты вокруг Center

	bool IsInside(vector position)
	{
		float dx = position[0] - Center[0];
		float dz = position[2] - Center[2];
		float distSq = dx * dx + dz * dz;
		if (distSq > RadiusMeters * RadiusMeters)
			return false;

		float dy = position[1] - Center[1];
		if (dy < 0)
			dy = -dy;
		if (dy > HeightMeters)
			return false;

		return true;
	}
}

class SM_DungeonEntryConfig
{
	ref array<vector> TeleportPositions = new array<vector>; // по одной точке на слот группы, циклически, если группа больше
	float OrientationYaw = 0;

	vector GetSlotPosition(int slotIndex)
	{
		if (!TeleportPositions || TeleportPositions.Count() == 0)
			return "0 0 0";
		return TeleportPositions[slotIndex % TeleportPositions.Count()];
	}
}

class SM_DungeonEvacuationConfig
{
	vector ExitPosition = "0 0 0";
	float RadiusMeters = 8;
	int HoldSeconds = 30;
}

class SM_DungeonLootItemConfig
{
	string ClassName;
	int MinQuantity = 1;
	int MaxQuantity = 1;
	float DropChance = 1.0;    // базовый шанс выпадения 0..1, множится на DropChanceMultiplier сложности
	bool NeedGreedRoll = true; // если выпал - разыгрывается между участниками (Мне нужно/Не откажусь/Откажусь)

	void SM_DungeonLootItemConfig(string className = "", int minQ = 1, int maxQ = 1, float chance = 1.0, bool rollEnabled = true)
	{
		ClassName = className;
		MinQuantity = minQ;
		MaxQuantity = maxQ;
		DropChance = chance;
		NeedGreedRoll = rollEnabled;
	}
}

class SM_DungeonDifficultyConfig
{
	string Name = "Normal";
	float OutgoingDamageMultiplier = 1.0; // множитель урона, наносимого игроком, внутри данжа на этой сложности
	float DropChanceMultiplier = 1.0;
	ref array<string> UnlocksDungeonIds = new array<string>; // Id данжей, которые открываются успешным прохождением
	ref array<ref SM_DungeonLootItemConfig> Loot = new array<ref SM_DungeonLootItemConfig>;

	void SM_DungeonDifficultyConfig(string name = "Normal", float dmg = 1.0, float drop = 1.0)
	{
		Name = name;
		OutgoingDamageMultiplier = dmg;
		DropChanceMultiplier = drop;
	}
}

class SM_DungeonSpawnPointConfig
{
	int Type = 0; // см. SM_DungeonSpawnType
	string ClassName;
	vector Position;
	vector Orientation;

	void SM_DungeonSpawnPointConfig(int type = 0, string className = "", vector pos = "0 0 0", vector ori = "0 0 0")
	{
		Type = type;
		ClassName = className;
		Position = pos;
		Orientation = ori;
	}
}

class SM_DungeonExtraContentConfig
{
	bool Enabled = true;
	// Путь к JSON, экспортированному из COT (Community Online Tools) или
	// VPP Admin Tools, относительно $mission: или $profile:. Формат см. в
	// SM_DungeonMapObject.c и Docs/ConfigGuide.md. Если пусто - используются
	// только ManualPoints ниже.
	string ImportFile = "";
	ref array<ref SM_DungeonSpawnPointConfig> ManualPoints = new array<ref SM_DungeonSpawnPointConfig>;
}

class SM_DungeonDefinition
{
	string Id;
	string Name;
	string Description;
	bool Enabled = true;
	int MinPlayers = 1;
	int MaxPlayers = 5;
	bool AllowSolo = true;
	bool AllowInviteGroup = true;
	bool AllowGroupFinder = true;
	bool RequiresUnlock = false; // недоступен, пока не окажется в списке разблокированных у игрока
	int RunTimeLimitMinutes = 60;
	int CooldownMinutesPerPlayer = 30; // после прохождения/провала игрок не может снова бронировать этот данж это время
	ref SM_DungeonZoneConfig Zone = new SM_DungeonZoneConfig;
	ref SM_DungeonEntryConfig Entry = new SM_DungeonEntryConfig;
	ref SM_DungeonEvacuationConfig Evacuation = new SM_DungeonEvacuationConfig;
	ref SM_DungeonExtraContentConfig ExtraContent = new SM_DungeonExtraContentConfig;
	ref array<ref SM_DungeonDifficultyConfig> Difficulties = new array<ref SM_DungeonDifficultyConfig>;

	SM_DungeonDifficultyConfig GetDifficulty(int index)
	{
		if (!Difficulties || Difficulties.Count() == 0)
			return NULL;
		if (index < 0)
			index = 0;
		if (index >= Difficulties.Count())
			index = Difficulties.Count() - 1;
		return Difficulties[index];
	}

	void EnsureSections()
	{
		if (!Zone)
			Zone = new SM_DungeonZoneConfig;
		if (!Entry)
			Entry = new SM_DungeonEntryConfig;
		if (!Entry.TeleportPositions)
			Entry.TeleportPositions = new array<vector>;
		if (!Evacuation)
			Evacuation = new SM_DungeonEvacuationConfig;
		if (!ExtraContent)
			ExtraContent = new SM_DungeonExtraContentConfig;
		if (!ExtraContent.ManualPoints)
			ExtraContent.ManualPoints = new array<ref SM_DungeonSpawnPointConfig>;
		if (!Difficulties)
			Difficulties = new array<ref SM_DungeonDifficultyConfig>;
	}
}

class SM_DungeonConfig
{
	string ConfigVersion = "1.0";

	ref SM_DungeonGeneralConfig General = new SM_DungeonGeneralConfig;
	ref SM_DungeonNotificationConfig Notifications = new SM_DungeonNotificationConfig;
	ref SM_DungeonBookingConfig Booking = new SM_DungeonBookingConfig;
	ref SM_DungeonPenaltyConfig Penalties = new SM_DungeonPenaltyConfig;
	ref SM_DungeonTicketConfig Ticket = new SM_DungeonTicketConfig;
	ref array<ref SM_DungeonDefinition> Dungeons = new array<ref SM_DungeonDefinition>;

	void EnsureSections()
	{
		if (!General)
			General = new SM_DungeonGeneralConfig;
		if (!General.AdminUids)
			General.AdminUids = new array<string>;
		if (!Notifications)
			Notifications = new SM_DungeonNotificationConfig;
		if (!Booking)
			Booking = new SM_DungeonBookingConfig;
		if (!Booking.RestartTimesUTC)
			Booking.RestartTimesUTC = new array<string>;
		if (!Penalties)
			Penalties = new SM_DungeonPenaltyConfig;
		if (!Ticket)
			Ticket = new SM_DungeonTicketConfig;
		if (!Dungeons)
			Dungeons = new array<ref SM_DungeonDefinition>;

		foreach (SM_DungeonDefinition dungeon : Dungeons)
		{
			if (dungeon)
				dungeon.EnsureSections();
		}
	}

	void Validate()
	{
		EnsureSections();
		ConfigVersion = "1.0";

		if (!General.AdminUids || General.AdminUids.Count() == 0)
		{
			General.AdminUids = new array<string>;
			General.AdminUids.Insert("REPLACE_WITH_STEAMID64");
		}
		if (General.MaxActiveBookingsPerPlayer < 1)
			General.MaxActiveBookingsPerPlayer = 1;
		if (General.InviteTimeoutSeconds < 10)
			General.InviteTimeoutSeconds = 10;
		if (General.GroupFinderListingTimeoutSeconds < 60)
			General.GroupFinderListingTimeoutSeconds = 60;
		if (General.ConfigReloadCooldownSeconds < 0)
			General.ConfigReloadCooldownSeconds = 0;

		if (Notifications.WarnBeforeStartSeconds < 5)
			Notifications.WarnBeforeStartSeconds = 5;
		if (Notifications.CountdownSeconds < 3)
			Notifications.CountdownSeconds = 3;
		if (Notifications.NoShowGraceMinutes < 1)
			Notifications.NoShowGraceMinutes = 1;

		if (Booking.BookingWindowDays < 1)
			Booking.BookingWindowDays = 1;
		if (Booking.BookingWindowDays > 31)
			Booking.BookingWindowDays = 31;
		if (Booking.DayStartHour < 0 || Booking.DayStartHour > 23)
			Booking.DayStartHour = 0;
		if (Booking.DayEndHour < 1 || Booking.DayEndHour > 24)
			Booking.DayEndHour = 24;
		if (Booking.DayEndHour <= Booking.DayStartHour)
			Booking.DayEndHour = 24;
		if (Booking.SlotDurationMinutes < 10)
			Booking.SlotDurationMinutes = 10;
		if (Booking.SlotIntervalMinutes < 10)
			Booking.SlotIntervalMinutes = Booking.SlotDurationMinutes;
		if (Booking.RestartBufferMinutes < 0)
			Booking.RestartBufferMinutes = 0;
		if (!Booking.RestartTimesUTC)
			Booking.RestartTimesUTC = new array<string>;

		if (Penalties.NoShowLockMinutes < 0)
			Penalties.NoShowLockMinutes = 0;
		if (Penalties.EarlyLeaveLockMinutes < 0)
			Penalties.EarlyLeaveLockMinutes = 0;
		if (Penalties.OffensesBeforePermaBan < 1)
			Penalties.OffensesBeforePermaBan = 1;
		if (Penalties.OffenseDecayDays < 1)
			Penalties.OffenseDecayDays = 1;

		Ticket.ItemClassName = Ticket.ItemClassName.Trim();
		if (Ticket.ItemClassName == "")
			Ticket.ItemClassName = "SM_DungeonRewardTicket";

		if (Dungeons.Count() == 0)
			Dungeons.Insert(BuildExampleDungeon());

		foreach (SM_DungeonDefinition dungeon : Dungeons)
			ValidateDungeon(dungeon);
	}

	protected void ValidateDungeon(SM_DungeonDefinition dungeon)
	{
		if (!dungeon)
			return;

		dungeon.EnsureSections();
		dungeon.Id = dungeon.Id.Trim();
		if (dungeon.Id == "")
			dungeon.Id = "dungeon_" + Dungeons.Find(dungeon);
		dungeon.Name = dungeon.Name.Trim();
		if (dungeon.Name == "")
			dungeon.Name = dungeon.Id;

		if (dungeon.MinPlayers < 1)
			dungeon.MinPlayers = 1;
		if (dungeon.MaxPlayers < dungeon.MinPlayers)
			dungeon.MaxPlayers = dungeon.MinPlayers;
		if (dungeon.RunTimeLimitMinutes < 5)
			dungeon.RunTimeLimitMinutes = 5;
		if (dungeon.CooldownMinutesPerPlayer < 0)
			dungeon.CooldownMinutesPerPlayer = 0;

		if (dungeon.Zone.RadiusMeters < 5)
			dungeon.Zone.RadiusMeters = 5;
		if (dungeon.Zone.HeightMeters < 5)
			dungeon.Zone.HeightMeters = 5;
		if (dungeon.Evacuation.RadiusMeters < 2)
			dungeon.Evacuation.RadiusMeters = 2;
		if (dungeon.Evacuation.HoldSeconds < 5)
			dungeon.Evacuation.HoldSeconds = 5;

		if (!dungeon.Difficulties || dungeon.Difficulties.Count() == 0)
		{
			dungeon.Difficulties = new array<ref SM_DungeonDifficultyConfig>;
			dungeon.Difficulties.Insert(new SM_DungeonDifficultyConfig("#STR_SMD_DIFF_EASY", 1.0, 1.0));
			dungeon.Difficulties.Insert(new SM_DungeonDifficultyConfig("#STR_SMD_DIFF_NORMAL", 0.85, 0.85));
			dungeon.Difficulties.Insert(new SM_DungeonDifficultyConfig("#STR_SMD_DIFF_HARD", 0.65, 0.65));
			dungeon.Difficulties.Insert(new SM_DungeonDifficultyConfig("#STR_SMD_DIFF_NIGHTMARE", 0.45, 0.5));
		}

		foreach (SM_DungeonDifficultyConfig difficulty : dungeon.Difficulties)
		{
			if (!difficulty)
				continue;
			if (difficulty.OutgoingDamageMultiplier < 0.05)
				difficulty.OutgoingDamageMultiplier = 0.05;
			if (difficulty.OutgoingDamageMultiplier > 3.0)
				difficulty.OutgoingDamageMultiplier = 3.0;
			if (difficulty.DropChanceMultiplier < 0)
				difficulty.DropChanceMultiplier = 0;
			if (!difficulty.UnlocksDungeonIds)
				difficulty.UnlocksDungeonIds = new array<string>;
			if (!difficulty.Loot)
				difficulty.Loot = new array<ref SM_DungeonLootItemConfig>;
			foreach (SM_DungeonLootItemConfig loot : difficulty.Loot)
			{
				if (!loot)
					continue;
				if (loot.MinQuantity < 1)
					loot.MinQuantity = 1;
				if (loot.MaxQuantity < loot.MinQuantity)
					loot.MaxQuantity = loot.MinQuantity;
				if (loot.DropChance < 0)
					loot.DropChance = 0;
				if (loot.DropChance > 1)
					loot.DropChance = 1;
			}
		}
	}

	protected SM_DungeonDefinition BuildExampleDungeon()
	{
		// Полностью заполненный пример-шаблон. Отключён по умолчанию -
		// заполните реальные координаты своей подготовленной локации и
		// включите (Enabled = true) в Settings.json.
		SM_DungeonDefinition dungeon = new SM_DungeonDefinition();
		dungeon.Id = "example_camp";
		dungeon.Name = "Пример: Заброшенный лагерь";
		dungeon.Description = "Шаблон конфигурации - замените координаты на свою локацию.";
		dungeon.Enabled = false;
		dungeon.MinPlayers = 1;
		dungeon.MaxPlayers = 5;
		dungeon.RunTimeLimitMinutes = 45;
		dungeon.CooldownMinutesPerPlayer = 60;

		dungeon.Zone.Center = Vector(7500, 300, 7500);
		dungeon.Zone.RadiusMeters = 80;
		dungeon.Zone.HeightMeters = 60;

		dungeon.Entry.TeleportPositions.Insert(Vector(7480, 300, 7480));
		dungeon.Entry.TeleportPositions.Insert(Vector(7484, 300, 7480));
		dungeon.Entry.TeleportPositions.Insert(Vector(7488, 300, 7480));
		dungeon.Entry.TeleportPositions.Insert(Vector(7492, 300, 7480));
		dungeon.Entry.TeleportPositions.Insert(Vector(7496, 300, 7480));
		dungeon.Entry.OrientationYaw = 90;

		dungeon.Evacuation.ExitPosition = Vector(7500, 300, 7520);
		dungeon.Evacuation.RadiusMeters = 10;
		dungeon.Evacuation.HoldSeconds = 30;

		dungeon.ExtraContent.Enabled = true;
		dungeon.ExtraContent.ImportFile = "$mission:SM_DungeonMod\\Locations\\example_camp.json";

		SM_DungeonDifficultyConfig easy = new SM_DungeonDifficultyConfig("#STR_SMD_DIFF_EASY", 1.0, 1.0);
		easy.Loot.Insert(new SM_DungeonLootItemConfig("TaloonBag_Green", 1, 1, 0.5, true));
		easy.Loot.Insert(new SM_DungeonLootItemConfig("Ammo_9x19", 2, 4, 0.9, false));

		SM_DungeonDifficultyConfig normal = new SM_DungeonDifficultyConfig("#STR_SMD_DIFF_NORMAL", 0.85, 0.85);
		normal.UnlocksDungeonIds.Insert("example_bunker");
		normal.Loot.Insert(new SM_DungeonLootItemConfig("CZ75P", 1, 1, 0.35, true));
		normal.Loot.Insert(new SM_DungeonLootItemConfig("Ammo_9x19", 3, 6, 0.9, false));

		SM_DungeonDifficultyConfig hard = new SM_DungeonDifficultyConfig("#STR_SMD_DIFF_HARD", 0.65, 0.65);
		hard.UnlocksDungeonIds.Insert("example_bunker");
		hard.Loot.Insert(new SM_DungeonLootItemConfig("M4A1", 1, 1, 0.2, true));

		SM_DungeonDifficultyConfig nightmare = new SM_DungeonDifficultyConfig("#STR_SMD_DIFF_NIGHTMARE", 0.45, 0.5);
		nightmare.UnlocksDungeonIds.Insert("example_bunker");
		nightmare.Loot.Insert(new SM_DungeonLootItemConfig("AK101", 1, 1, 0.15, true));

		dungeon.Difficulties.Insert(easy);
		dungeon.Difficulties.Insert(normal);
		dungeon.Difficulties.Insert(hard);
		dungeon.Difficulties.Insert(nightmare);

		return dungeon;
	}

	SM_DungeonDefinition GetDungeon(string id)
	{
		foreach (SM_DungeonDefinition dungeon : Dungeons)
		{
			if (dungeon && dungeon.Id == id)
				return dungeon;
		}
		return NULL;
	}

	bool IsAdmin(string uid)
	{
		if (!General || !General.AdminUids)
			return false;
		foreach (string adminUid : General.AdminUids)
		{
			if (adminUid == uid)
				return true;
		}
		return false;
	}
}

class SM_DungeonConfigLoader
{
	static const string DIR_PATH = "$profile:SM_DungeonMod";
	static const string CONFIG_PATH = "$profile:SM_DungeonMod\\Settings.json";

	protected static ref SM_DungeonConfig s_Config;

	static SM_DungeonConfig Get()
	{
		if (!s_Config)
			Load();
		return s_Config;
	}

	static void Load()
	{
		if (!FileExist(DIR_PATH))
			MakeDirectory(DIR_PATH);

		s_Config = new SM_DungeonConfig();

		string errorMessage;
		if (FileExist(CONFIG_PATH))
		{
			if (!JsonFileLoader<SM_DungeonConfig>.LoadFile(CONFIG_PATH, s_Config, errorMessage))
			{
				ErrorEx("[SM_DungeonMod] Failed to load Settings.json: " + errorMessage);
				s_Config = new SM_DungeonConfig();
			}
		}

		if (!s_Config)
			s_Config = new SM_DungeonConfig();

		s_Config.Validate();

		if (!JsonFileLoader<SM_DungeonConfig>.SaveFile(CONFIG_PATH, s_Config, errorMessage))
			ErrorEx("[SM_DungeonMod] Failed to save Settings.json: " + errorMessage);
	}

	static bool Reload(out string reloadMessage)
	{
		reloadMessage = "";

		if (!FileExist(DIR_PATH))
			MakeDirectory(DIR_PATH);

		SM_DungeonConfig loadedConfig = new SM_DungeonConfig();
		string errorMessage;
		if (FileExist(CONFIG_PATH))
		{
			if (!JsonFileLoader<SM_DungeonConfig>.LoadFile(CONFIG_PATH, loadedConfig, errorMessage))
			{
				reloadMessage = "#STR_SMD_ADMIN_RELOAD_FAIL" + errorMessage;
				return false;
			}
		}

		if (!loadedConfig)
			loadedConfig = new SM_DungeonConfig();

		loadedConfig.Validate();

		if (!JsonFileLoader<SM_DungeonConfig>.SaveFile(CONFIG_PATH, loadedConfig, errorMessage))
		{
			reloadMessage = "#STR_SMD_ADMIN_RELOAD_FAIL" + errorMessage;
			return false;
		}

		s_Config = loadedConfig;
		reloadMessage = "#STR_SMD_ADMIN_RELOAD_OK";
		return true;
	}
}
