// ============================================================
//  Конфиги Boogy_BattlePass (JSON в $profile:Boogy_BattlePass)
// ============================================================

class BBP_RewardConfig
{
	// Item | Coins | Xp | Case | Premium | Custom
	string Type = "Item";
	// Item: класс предмета
	string ClassName = "";
	// Item: количество экземпляров; Coins/Xp: сумма; Case: количество кейсов
	int Amount = 1;
	// Item: количество в стаке / патронов / наполненность (-1 = по умолчанию)
	float Quantity = -1;
	// Item: здоровье 0..1 (-1 = по умолчанию)
	float Health = -1;
	// Item: вложения/содержимое, создаются внутри предмета
	ref array<string> Attachments = new array<string>;
	// Case: Id кейса; Premium: Id сезона ("" = текущий); Custom: произвольная строка для других модов
	string Data = "";
	// Подпись в меню. Если пусто — берётся имя предмета/типа
	string DisplayName = "";
	// Вес выпадения (только для содержимого кейсов)
	int Weight = 1;

	void BBP_RewardConfig(string type = "Item", string className = "", int amount = 1)
	{
		Type = type;
		ClassName = className;
		Amount = amount;
	}
}

class BBP_LocationPoint
{
	string Name = "";
	vector Position = "0 0 0";
	float Radius = 50;

	void BBP_LocationPoint(string name = "", vector pos = "0 0 0", float radius = 50)
	{
		Name = name;
		Position = pos;
		Radius = radius;
	}
}

class BBP_QuestConfig
{
	string Id = "";
	string Name = "";
	string Description = "";

	// Kill | Action | Craft | Visit | Custom
	string Type = "Kill";

	// Kill: Any | Zombie | Animal | Player
	string KillKind = "Any";

	// Kill: классы жертв (учитывается наследование), пусто = любые
	// Action: классы действий (ActionBandageSelf, ActionBuildPart…)
	// Craft: классы созданных предметов ИЛИ классы рецептов
	// Custom: имена событий
	ref array<string> Targets = new array<string>;

	// Kill: оружие, которым нужно убить (учитывается наследование), пусто = любое
	ref array<string> Weapons = new array<string>;

	// Action: предмет в руках/используемый предмет, пусто = любой
	ref array<string> Items = new array<string>;

	// Kill: дистанция убийства в метрах (0 = без ограничений)
	float MinDistance = 0;
	float MaxDistance = 0;

	// Kill/Action/Craft: выполнять только внутри одной из этих зон (пусто = везде)
	ref array<ref BBP_LocationPoint> Zones = new array<ref BBP_LocationPoint>;

	// Visit: Any | All | Route
	string VisitMode = "Any";
	ref array<ref BBP_LocationPoint> Points = new array<ref BBP_LocationPoint>;

	// Сколько раз нужно выполнить условие (для Visit All/Route считается автоматически)
	int Count = 1;

	int Xp = 100;
	int Coins = 0;
	ref array<ref BBP_RewardConfig> Rewards = new array<ref BBP_RewardConfig>;

	// Задание доступно только с премиум-пропуском
	bool PremiumOnly = false;

	// Season | Daily | Weekly — как часто задание сбрасывается
	string Period = "Season";

	void BBP_QuestConfig(string id = "", string name = "", string type = "Kill", int count = 1, int xp = 100)
	{
		Id = id;
		Name = name;
		Type = type;
		Count = count;
		Xp = xp;
	}

	int GetRequiredCount()
	{
		if (Type == BBP_QuestType.VISIT && (VisitMode == BBP_VisitMode.ALL || VisitMode == BBP_VisitMode.ROUTE))
			return Math.Max(1, Points.Count());
		return Math.Max(1, Count);
	}
}

class BBP_LevelConfig
{
	int Level = 1;
	// Опыт, нужный именно для этого уровня (0 = XpPerLevel сезона)
	int XpRequired = 0;
	ref array<ref BBP_RewardConfig> FreeRewards = new array<ref BBP_RewardConfig>;
	ref array<ref BBP_RewardConfig> PremiumRewards = new array<ref BBP_RewardConfig>;

	void BBP_LevelConfig(int level = 1)
	{
		Level = level;
	}
}

class BBP_SeasonConfig
{
	string Id = "";
	string Name = "";
	string Description = "";
	bool Enabled = true;
	// "YYYY-MM-DD HH:MM" в часовом поясе из Settings.json
	string StartDate = "";
	string EndDate = "";
	int XpPerLevel = 1000;
	ref array<ref BBP_LevelConfig> Levels = new array<ref BBP_LevelConfig>;
	ref array<ref BBP_QuestConfig> Quests = new array<ref BBP_QuestConfig>;

	[NonSerialized()]
	int StartMinutes = -1;
	[NonSerialized()]
	int EndMinutes = -1;
	[NonSerialized()]
	ref array<int> LevelStartXp = new array<int>; // XP начала уровня i (индекс = Level-1)
	[NonSerialized()]
	ref array<int> LevelEndXp = new array<int>;

	// Сортирует уровни и считает пороги опыта. false — если сезон некорректен.
	bool Prepare(out string error)
	{
		StartMinutes = BBP_Time.Parse(StartDate);
		EndMinutes = BBP_Time.Parse(EndDate);
		if (Id == "")
		{
			error = "пустой Id";
			return false;
		}
		if (StartMinutes < 0 || EndMinutes < 0)
		{
			error = "неверный формат StartDate/EndDate (нужно YYYY-MM-DD HH:MM)";
			return false;
		}
		if (EndMinutes <= StartMinutes)
		{
			error = "EndDate раньше StartDate";
			return false;
		}
		if (XpPerLevel <= 0)
			XpPerLevel = 1000;

		// Сортировка вставками по Level
		for (int i = 1; i < Levels.Count(); i++)
		{
			int j = i;
			while (j > 0 && Levels[j - 1].Level > Levels[j].Level)
			{
				Levels.SwapItems(j - 1, j);
				j--;
			}
		}

		LevelStartXp.Clear();
		LevelEndXp.Clear();
		int total = 0;
		foreach (BBP_LevelConfig lvl : Levels)
		{
			int need = lvl.XpRequired;
			if (need <= 0)
				need = XpPerLevel;
			LevelStartXp.Insert(total);
			total = total + need;
			LevelEndXp.Insert(total);
		}

		for (int q = Quests.Count() - 1; q >= 0; q--)
		{
			if (!Quests[q] || Quests[q].Id == "")
				Quests.Remove(q);
		}
		return true;
	}

	int GetMaxLevel()
	{
		return Levels.Count();
	}

	// Достигнутый уровень (0 = ни одного)
	int GetLevelForXp(int xp)
	{
		int level = 0;
		for (int i = 0; i < LevelEndXp.Count(); i++)
		{
			if (xp >= LevelEndXp[i])
				level = i + 1;
			else
				break;
		}
		return level;
	}

	int GetMaxXp()
	{
		if (LevelEndXp.Count() == 0)
			return 0;
		return LevelEndXp[LevelEndXp.Count() - 1];
	}

	BBP_LevelConfig GetLevel(int level)
	{
		if (level < 1 || level > Levels.Count())
			return null;
		return Levels[level - 1];
	}

	BBP_QuestConfig FindQuest(string id)
	{
		foreach (BBP_QuestConfig quest : Quests)
		{
			if (quest.Id == id)
				return quest;
		}
		return null;
	}

	bool IsActiveAt(int minutes)
	{
		return Enabled && minutes >= StartMinutes && minutes < EndMinutes;
	}
}

class BBP_PremiumItemConfig
{
	string ClassName = "BBP_PremiumPass";
	// Сезон, для которого активируется пропуск ("" = текущий)
	string SeasonId = "";
	// Удалить предмет после активации
	bool Consume = true;

	void BBP_PremiumItemConfig(string className = "BBP_PremiumPass")
	{
		ClassName = className;
	}
}

class BBP_DonateConfig
{
	// Интеграция с донат-системой через HTTP. См. Docs/DonateApi.md
	bool Enabled = false;
	string BaseUrl = "https://your-shop.example.com";
	string PendingPath = "/api/battlepass/pending";
	string AckPath = "/api/battlepass/ack";
	string ApiKey = "";
	string ServerId = "server-1";
	int PollIntervalSec = 60;
}

class BBP_Settings
{
	int ConfigVersion = 1;
	bool Enabled = true;
	// Смещение от UTC в минутах для дат сезонов (Москва = 180)
	int TimezoneOffsetMinutes = 180;
	// Steam64 ID администраторов (команды /bp в чате)
	ref array<string> AdminIds = new array<string>;
	ref array<ref BBP_PremiumItemConfig> PremiumItems = new array<ref BBP_PremiumItemConfig>;
	// Уведомлять о выполнении заданий и новых уровнях
	bool NotifyQuestComplete = true;
	bool NotifyLevelUp = true;
	// Выдавать награды за уровни автоматически (без кнопки "Забрать")
	bool AutoClaimRewards = false;
	// Если инвентарь полон — класть награду на землю под игрока
	bool DropRewardsOnGroundIfFull = true;
	// Разрешить забирать награды завершившегося сезона, пока не начался новый
	bool AllowClaimAfterSeasonEnd = true;
	int VisitCheckIntervalSec = 5;
	int AutoSaveIntervalSec = 300;
	ref BBP_DonateConfig Donate = new BBP_DonateConfig;

	bool IsAdmin(string uid)
	{
		return uid != "" && AdminIds.Find(uid) >= 0;
	}
}

class BBP_CaseConfig
{
	string Id = "";
	string Name = "";
	string Description = "";
	int Price = 100;
	// Coins — монеты пропуска; Item — предметы-валюта из инвентаря (CurrencyItem)
	string Currency = "Coins";
	string CurrencyItem = "";
	// Покупка доступна только с премиум-пропуском текущего сезона
	bool PremiumOnly = false;
	// Сразу открыть кейс после покупки
	bool OpenOnPurchase = false;
	ref array<ref BBP_RewardConfig> Rewards = new array<ref BBP_RewardConfig>;

	void BBP_CaseConfig(string id = "", string name = "", int price = 100)
	{
		Id = id;
		Name = name;
		Price = price;
	}

	int GetTotalWeight()
	{
		int total = 0;
		foreach (BBP_RewardConfig reward : Rewards)
		{
			total = total + Math.Max(0, reward.Weight);
		}
		return total;
	}
}

class BBP_ShopConfig
{
	bool Enabled = true;
	ref array<ref BBP_CaseConfig> Cases = new array<ref BBP_CaseConfig>;

	BBP_CaseConfig FindCase(string id)
	{
		foreach (BBP_CaseConfig c : Cases)
		{
			if (c.Id == id)
				return c;
		}
		return null;
	}
}
