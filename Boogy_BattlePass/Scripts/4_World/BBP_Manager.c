// ============================================================
//  Boogy_BattlePass — серверный менеджер
// ============================================================
class BBP_Manager
{
	protected static ref BBP_Manager s_Instance;

	protected ref BBP_Settings m_Settings;
	protected ref BBP_ShopConfig m_Shop;
	protected ref array<ref BBP_SeasonConfig> m_Seasons;
	protected string m_ActiveSeasonId;

	protected ref map<string, ref BBP_PlayerData> m_Players;
	protected ref array<string> m_Online;
	protected ref array<string> m_Dirty;
	protected ref array<string> m_PendingSync;
	protected ref map<string, float> m_LastRequest;

	protected ref BBP_DonationHistory m_Donations;
	protected RestApi m_RestApi;
	protected ref BBP_DonateCallback m_DonateCb;
	protected ref BBP_SilentCallback m_AckCb;
	protected bool m_DonateInFlight;

	// ------------------------------------------------------------
	//  Жизненный цикл
	// ------------------------------------------------------------

	static void Init()
	{
		if (!s_Instance)
			s_Instance = new BBP_Manager();
	}

	static BBP_Manager Get()
	{
		return s_Instance;
	}

	void BBP_Manager()
	{
		m_Players = new map<string, ref BBP_PlayerData>;
		m_Online = new array<string>;
		m_Dirty = new array<string>;
		m_PendingSync = new array<string>;
		m_LastRequest = new map<string, float>;
		m_Seasons = new array<ref BBP_SeasonConfig>;

		LoadAll();
		m_ActiveSeasonId = GetActiveSeasonId();

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.CheckSeasonSwitch, 30000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.CheckVisits, Math.Max(1, m_Settings.VisitCheckIntervalSec) * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.SaveDirty, Math.Max(30, m_Settings.AutoSaveIntervalSec) * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.PollDonations, Math.Max(10, m_Settings.Donate.PollIntervalSec) * 1000, true);

		Print("[Boogy_BattlePass] Запущен. Сезонов: " + m_Seasons.Count() + ", активный: " + m_ActiveSeasonId);
	}

	void Shutdown()
	{
		SaveDirty();
		SaveDonations();
	}

	BBP_Settings GetSettings()
	{
		return m_Settings;
	}

	// ------------------------------------------------------------
	//  Загрузка конфигов
	// ------------------------------------------------------------

	void LoadAll()
	{
		if (!FileExist(BBP_Paths.DIR))
			MakeDirectory(BBP_Paths.DIR);
		if (!FileExist(BBP_Paths.SEASONS_DIR))
			MakeDirectory(BBP_Paths.SEASONS_DIR);
		if (!FileExist(BBP_Paths.PLAYERS_DIR))
			MakeDirectory(BBP_Paths.PLAYERS_DIR);

		string error;

		// Settings.json
		m_Settings = null;
		if (FileExist(BBP_Paths.SETTINGS))
		{
			if (!JsonFileLoader<BBP_Settings>.LoadFile(BBP_Paths.SETTINGS, m_Settings, error))
			{
				Print("[Boogy_BattlePass] Ошибка Settings.json: " + error + ". Используются значения по умолчанию.");
				m_Settings = null;
			}
		}
		if (!m_Settings)
		{
			m_Settings = BBP_Defaults.CreateSettings();
			if (!FileExist(BBP_Paths.SETTINGS))
				JsonFileLoader<BBP_Settings>.SaveFile(BBP_Paths.SETTINGS, m_Settings, error);
		}
		if (!m_Settings.Donate)
			m_Settings.Donate = new BBP_DonateConfig;

		BBP_Shared.PremiumItemClasses.Clear();
		foreach (BBP_PremiumItemConfig pic : m_Settings.PremiumItems)
		{
			if (pic && pic.ClassName != "")
				BBP_Shared.PremiumItemClasses.Insert(pic.ClassName);
		}

		// Shop.json
		m_Shop = null;
		if (FileExist(BBP_Paths.SHOP))
		{
			if (!JsonFileLoader<BBP_ShopConfig>.LoadFile(BBP_Paths.SHOP, m_Shop, error))
			{
				Print("[Boogy_BattlePass] Ошибка Shop.json: " + error);
				m_Shop = null;
			}
		}
		if (!m_Shop)
		{
			if (FileExist(BBP_Paths.SHOP))
			{
				m_Shop = new BBP_ShopConfig;
				m_Shop.Enabled = false;
			}
			else
			{
				m_Shop = BBP_Defaults.CreateShop();
				JsonFileLoader<BBP_ShopConfig>.SaveFile(BBP_Paths.SHOP, m_Shop, error);
			}
		}

		// Seasons\*.json
		LoadSeasons();

		// Donations.json
		m_Donations = null;
		if (FileExist(BBP_Paths.DONATIONS))
			JsonFileLoader<BBP_DonationHistory>.LoadFile(BBP_Paths.DONATIONS, m_Donations, error);
		if (!m_Donations)
			m_Donations = new BBP_DonationHistory;
	}

	protected void LoadSeasons()
	{
		m_Seasons.Clear();

		string fileName;
		FileAttr attr;
		FindFileHandle handle = FindFile(BBP_Paths.SEASONS_DIR + "\\*.json", fileName, attr, 0);
		bool found = handle && fileName != "";
		while (found)
		{
			LoadSeasonFile(BBP_Paths.SEASONS_DIR + "\\" + fileName);
			found = FindNextFile(handle, fileName, attr);
		}
		if (handle)
			CloseFindFile(handle);

		if (m_Seasons.Count() == 0 && !FileExist(BBP_Paths.SEASONS_DIR + "\\Season1.json"))
		{
			int today = BBP_Time.DayKey(BBP_Time.NowMinutes(m_Settings.TimezoneOffsetMinutes)) * 1440;
			string saveError;
			BBP_SeasonConfig first = BBP_Defaults.CreateSeason("season_1", "Сезон 1: Выживший", today, 60);
			JsonFileLoader<BBP_SeasonConfig>.SaveFile(BBP_Paths.SEASONS_DIR + "\\Season1.json", first, saveError);
			BBP_SeasonConfig second = BBP_Defaults.CreateSeason("season_2", "Сезон 2: Охотник", today + 60 * 1440, 60);
			JsonFileLoader<BBP_SeasonConfig>.SaveFile(BBP_Paths.SEASONS_DIR + "\\Season2.json", second, saveError);

			LoadSeasonFile(BBP_Paths.SEASONS_DIR + "\\Season1.json");
			LoadSeasonFile(BBP_Paths.SEASONS_DIR + "\\Season2.json");
		}

		// Сортировка по дате начала
		for (int i = 1; i < m_Seasons.Count(); i++)
		{
			int j = i;
			while (j > 0 && m_Seasons[j - 1].StartMinutes > m_Seasons[j].StartMinutes)
			{
				m_Seasons.SwapItems(j - 1, j);
				j--;
			}
		}
	}

	protected void LoadSeasonFile(string path)
	{
		BBP_SeasonConfig season;
		string error;
		if (!JsonFileLoader<BBP_SeasonConfig>.LoadFile(path, season, error) || !season)
		{
			Print("[Boogy_BattlePass] Ошибка файла сезона " + path + ": " + error);
			return;
		}

		string prepareError;
		if (!season.Prepare(prepareError))
		{
			Print("[Boogy_BattlePass] Сезон " + path + " пропущен: " + prepareError);
			return;
		}

		foreach (BBP_SeasonConfig existing : m_Seasons)
		{
			if (existing.Id == season.Id)
			{
				Print("[Boogy_BattlePass] Дубликат Id сезона '" + season.Id + "' в " + path + " — пропущен");
				return;
			}
		}

		m_Seasons.Insert(season);
	}

	void ReloadConfigs()
	{
		SaveDirty();
		LoadAll();
		CheckSeasonSwitch();
		SyncAllOnline();
		Print("[Boogy_BattlePass] Конфиги перезагружены");
	}

	// ------------------------------------------------------------
	//  Сезоны
	// ------------------------------------------------------------

	int Now()
	{
		return BBP_Time.NowMinutes(m_Settings.TimezoneOffsetMinutes);
	}

	BBP_SeasonConfig FindSeason(string id)
	{
		foreach (BBP_SeasonConfig season : m_Seasons)
		{
			if (season.Id == id)
				return season;
		}
		return null;
	}

	BBP_SeasonConfig GetActiveSeason()
	{
		int now = Now();
		foreach (BBP_SeasonConfig season : m_Seasons)
		{
			if (season.IsActiveAt(now))
				return season;
		}
		return null;
	}

	string GetActiveSeasonId()
	{
		BBP_SeasonConfig season = GetActiveSeason();
		if (season)
			return season.Id;
		return "";
	}

	// Сезон, показываемый игроку: активный или (если разрешено) последний завершившийся.
	BBP_SeasonConfig GetDisplaySeason(out bool ended)
	{
		ended = false;
		BBP_SeasonConfig active = GetActiveSeason();
		if (active)
			return active;
		if (!m_Settings.AllowClaimAfterSeasonEnd)
			return null;

		int now = Now();
		BBP_SeasonConfig last = null;
		foreach (BBP_SeasonConfig season : m_Seasons)
		{
			if (season.Enabled && season.EndMinutes <= now)
			{
				if (!last || season.EndMinutes > last.EndMinutes)
					last = season;
			}
		}
		if (last)
			ended = true;
		return last;
	}

	BBP_SeasonConfig GetNextSeason()
	{
		int now = Now();
		foreach (BBP_SeasonConfig season : m_Seasons)
		{
			if (season.Enabled && season.StartMinutes > now)
				return season;
		}
		return null;
	}

	void CheckSeasonSwitch()
	{
		string current = GetActiveSeasonId();
		if (current == m_ActiveSeasonId)
			return;

		BBP_SeasonConfig oldSeason = FindSeason(m_ActiveSeasonId);
		BBP_SeasonConfig newSeason = FindSeason(current);
		m_ActiveSeasonId = current;

		if (oldSeason)
			NotifyAll("Боевой пропуск", "Сезон \"" + oldSeason.Name + "\" завершён");
		if (newSeason)
			NotifyAll("Боевой пропуск", "Начался новый сезон: " + newSeason.Name);

		Print("[Boogy_BattlePass] Смена сезона -> '" + current + "'");
		SyncAllOnline();
	}

	// ------------------------------------------------------------
	//  Данные игроков
	// ------------------------------------------------------------

	protected string GetPlayerPath(string uid)
	{
		return BBP_Paths.PLAYERS_DIR + "\\" + uid + ".json";
	}

	BBP_PlayerData GetPlayerData(string uid, bool create = true)
	{
		if (uid == "")
			return null;

		BBP_PlayerData data;
		if (m_Players.Find(uid, data))
			return data;

		string path = GetPlayerPath(uid);
		if (FileExist(path))
		{
			string error;
			if (!JsonFileLoader<BBP_PlayerData>.LoadFile(path, data, error))
			{
				Print("[Boogy_BattlePass] Ошибка файла игрока " + uid + ": " + error);
				data = null;
			}
		}

		if (!data)
		{
			if (!create)
				return null;
			data = new BBP_PlayerData;
		}

		data.Uid = uid;
		m_Players.Set(uid, data);
		return data;
	}

	void MarkDirty(string uid)
	{
		if (m_Dirty.Find(uid) < 0)
			m_Dirty.Insert(uid);
	}

	void SaveDirty()
	{
		string error;
		foreach (string uid : m_Dirty)
		{
			BBP_PlayerData data;
			if (m_Players.Find(uid, data) && data)
				JsonFileLoader<BBP_PlayerData>.SaveFile(GetPlayerPath(uid), data, error);
		}
		m_Dirty.Clear();

		// Выгружаем из памяти игроков, которых нет на сервере
		array<string> offline = new array<string>;
		foreach (string cachedUid, BBP_PlayerData cached : m_Players)
		{
			if (m_Online.Find(cachedUid) < 0)
				offline.Insert(cachedUid);
		}
		foreach (string offlineUid : offline)
		{
			m_Players.Remove(offlineUid);
		}
	}

	protected void SaveDonations()
	{
		string error;
		while (m_Donations.ProcessedIds.Count() > 2000)
			m_Donations.ProcessedIds.RemoveOrdered(0);
		JsonFileLoader<BBP_DonationHistory>.SaveFile(BBP_Paths.DONATIONS, m_Donations, error);
	}

	PlayerBase FindOnlinePlayer(string uid)
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity() && pb.GetIdentity().GetPlainId() == uid)
				return pb;
		}
		return null;
	}

	void OnPlayerReady(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		if (m_Online.Find(uid) < 0)
			m_Online.Insert(uid);

		BBP_PlayerData data = GetPlayerData(uid);
		data.Name = player.GetIdentity().GetName();
		MarkDirty(uid);

		if (m_Settings.AutoClaimRewards)
			ClaimAll(player, false);

		SyncState(player);
	}

	void OnPlayerDisconnected(string uid)
	{
		BBP_PlayerData data;
		if (m_Players.Find(uid, data) && data)
		{
			string error;
			JsonFileLoader<BBP_PlayerData>.SaveFile(GetPlayerPath(uid), data, error);
		}
		m_Online.RemoveItem(uid);
		m_Dirty.RemoveItem(uid);
		m_PendingSync.RemoveItem(uid);
		m_Players.Remove(uid);
	}

	bool HasPremium(string uid, string seasonId = "")
	{
		if (seasonId == "")
			seasonId = m_ActiveSeasonId;
		if (seasonId == "")
			return false;
		BBP_PlayerData data = GetPlayerData(uid, false);
		if (!data)
			return false;
		BBP_SeasonProgress sp = data.GetSeason(seasonId, false);
		return sp && sp.Premium;
	}

	int GetPlayerLevel(string uid)
	{
		BBP_SeasonConfig season = GetActiveSeason();
		if (!season)
			return 0;
		BBP_PlayerData data = GetPlayerData(uid, false);
		if (!data)
			return 0;
		BBP_SeasonProgress sp = data.GetSeason(season.Id, false);
		if (!sp)
			return 0;
		return season.GetLevelForXp(sp.Xp);
	}

	// ------------------------------------------------------------
	//  Задания
	// ------------------------------------------------------------

	protected int GetPeriodKey(BBP_QuestConfig quest, int now)
	{
		if (quest.Period == BBP_Period.DAILY)
			return BBP_Time.DayKey(now);
		if (quest.Period == BBP_Period.WEEKLY)
			return BBP_Time.WeekKey(now);
		return 0;
	}

	protected BBP_QuestProgress GetQuestProgress(BBP_SeasonProgress sp, BBP_QuestConfig quest, int now)
	{
		BBP_QuestProgress qp = sp.FindQuest(quest.Id);
		int key = GetPeriodKey(quest, now);
		if (!qp)
		{
			qp = new BBP_QuestProgress;
			qp.Id = quest.Id;
			qp.PeriodKey = key;
			sp.Quests.Insert(qp);
		}
		else if (qp.PeriodKey != key)
		{
			qp.PeriodKey = key;
			qp.Value = 0;
			qp.Completed = false;
			qp.VisitedPoints.Clear();
			qp.InsidePoint = -1;
		}
		return qp;
	}

	// Общая подготовка для обработки игрового события: возвращает активный сезон и прогресс игрока.
	protected bool PrepareEvent(PlayerBase player, out BBP_SeasonConfig season, out BBP_PlayerData data, out BBP_SeasonProgress sp)
	{
		if (!m_Settings.Enabled || !player || !player.GetIdentity())
			return false;

		season = GetActiveSeason();
		if (!season || season.Quests.Count() == 0)
			return false;

		data = GetPlayerData(player.GetIdentity().GetPlainId());
		if (!data)
			return false;

		sp = data.GetSeason(season.Id, true);
		return true;
	}

	protected bool IsQuestAvailable(BBP_QuestConfig quest, BBP_SeasonProgress sp)
	{
		return !quest.PremiumOnly || sp.Premium;
	}

	protected bool IsInZones(array<ref BBP_LocationPoint> zones, vector pos)
	{
		if (!zones || zones.Count() == 0)
			return true;
		foreach (BBP_LocationPoint zone : zones)
		{
			if (IsInsidePoint(zone, pos))
				return true;
		}
		return false;
	}

	protected bool IsInsidePoint(BBP_LocationPoint point, vector pos)
	{
		float dx = pos[0] - point.Position[0];
		float dz = pos[2] - point.Position[2];
		return (dx * dx + dz * dz) <= point.Radius * point.Radius;
	}

	protected bool MatchesClassList(array<string> list, string className)
	{
		if (!list || list.Count() == 0)
			return true;
		if (className == "")
			return false;

		string lower = className;
		lower.ToLower();
		foreach (string entry : list)
		{
			string entryLower = entry;
			entryLower.ToLower();
			if (entryLower == lower || GetGame().IsKindOf(className, entry))
				return true;
		}
		return false;
	}

	protected void AddQuestProgress(PlayerBase player, BBP_SeasonConfig season, BBP_PlayerData data, BBP_SeasonProgress sp, BBP_QuestConfig quest, BBP_QuestProgress qp, int amount)
	{
		if (qp.Completed || amount <= 0)
			return;

		int required = quest.GetRequiredCount();
		qp.Value = Math.Min(required, qp.Value + amount);
		MarkDirty(data.Uid);

		if (qp.Value >= required)
			CompleteQuest(player, season, data, sp, quest, qp);

		QueueSync(data.Uid);
	}

	protected void CompleteQuest(PlayerBase player, BBP_SeasonConfig season, BBP_PlayerData data, BBP_SeasonProgress sp, BBP_QuestConfig quest, BBP_QuestProgress qp)
	{
		qp.Completed = true;
		qp.Value = quest.GetRequiredCount();

		if (m_Settings.NotifyQuestComplete && player)
		{
			string text = quest.Name + "  +" + quest.Xp + " XP";
			if (quest.Coins > 0)
				text = text + ", +" + quest.Coins + " монет";
			Notify(player, "Задание выполнено", text);
		}

		if (quest.Coins > 0)
			data.Coins = data.Coins + quest.Coins;

		foreach (BBP_RewardConfig reward : quest.Rewards)
		{
			GiveReward(player, data, reward, season.Id);
		}

		AddXp(player, season, data, sp, quest.Xp);
		BBP_API.OnQuestCompleted.Invoke(player, season.Id, quest.Id);
	}

	protected void AddXp(PlayerBase player, BBP_SeasonConfig season, BBP_PlayerData data, BBP_SeasonProgress sp, int amount)
	{
		if (amount == 0)
			return;

		int oldLevel = season.GetLevelForXp(sp.Xp);
		sp.Xp = Math.Max(0, sp.Xp + amount);
		int newLevel = season.GetLevelForXp(sp.Xp);
		MarkDirty(data.Uid);
		QueueSync(data.Uid);

		if (newLevel <= oldLevel)
			return;

		if (player)
		{
			if (m_Settings.NotifyLevelUp)
				Notify(player, "Боевой пропуск", "Новый уровень: " + newLevel + ". Заберите награды в меню!");
			BBP_API.OnLevelUp.Invoke(player, season.Id, newLevel);
			if (m_Settings.AutoClaimRewards)
				ClaimAll(player, false);
		}
	}

	// --- Убийства ---
	void OnEntityKilled(EntityAI victim, Object killer, string kind)
	{
		if (!victim)
			return;

		PlayerBase killerPlayer = ResolvePlayer(killer);
		if (!killerPlayer || killerPlayer == victim)
			return;

		BBP_SeasonConfig season;
		BBP_PlayerData data;
		BBP_SeasonProgress sp;
		if (!PrepareEvent(killerPlayer, season, data, sp))
			return;

		string weaponType = "";
		EntityAI killerEntity = EntityAI.Cast(killer);
		if (killerEntity && !PlayerBase.Cast(killerEntity))
		{
			weaponType = killerEntity.GetType();
		}
		else
		{
			EntityAI inHands = killerPlayer.GetHumanInventory().GetEntityInHands();
			if (inHands)
				weaponType = inHands.GetType();
		}

		vector killerPos = killerPlayer.GetPosition();
		float distance = vector.Distance(killerPos, victim.GetPosition());
		int now = Now();

		foreach (BBP_QuestConfig quest : season.Quests)
		{
			if (quest.Type != BBP_QuestType.KILL || !IsQuestAvailable(quest, sp))
				continue;
			if (quest.KillKind != "" && quest.KillKind != BBP_KillKind.ANY && quest.KillKind != kind)
				continue;
			if (!MatchesClassList(quest.Targets, victim.GetType()))
				continue;
			if (!MatchesClassList(quest.Weapons, weaponType))
				continue;
			if (quest.MinDistance > 0 && distance < quest.MinDistance)
				continue;
			if (quest.MaxDistance > 0 && distance > quest.MaxDistance)
				continue;
			if (!IsInZones(quest.Zones, killerPos))
				continue;

			BBP_QuestProgress qp = GetQuestProgress(sp, quest, now);
			AddQuestProgress(killerPlayer, season, data, sp, quest, qp, 1);
		}
	}

	// --- Действия ---
	void OnActionDone(PlayerBase player, ActionBase action, ItemBase item)
	{
		if (!action)
			return;

		BBP_SeasonConfig season;
		BBP_PlayerData data;
		BBP_SeasonProgress sp;
		if (!PrepareEvent(player, season, data, sp))
			return;

		string itemType = "";
		if (item)
			itemType = item.GetType();

		int now = Now();
		vector pos = player.GetPosition();

		foreach (BBP_QuestConfig quest : season.Quests)
		{
			if (quest.Type != BBP_QuestType.ACTION || !IsQuestAvailable(quest, sp))
				continue;
			if (!MatchesAction(quest.Targets, action))
				continue;
			if (!MatchesClassList(quest.Items, itemType))
				continue;
			if (!IsInZones(quest.Zones, pos))
				continue;

			BBP_QuestProgress qp = GetQuestProgress(sp, quest, now);
			AddQuestProgress(player, season, data, sp, quest, qp, 1);
		}
	}

	protected bool MatchesAction(array<string> targets, ActionBase action)
	{
		if (!targets || targets.Count() == 0)
			return true;

		string actionName = action.ClassName();
		foreach (string target : targets)
		{
			if (target == actionName)
				return true;
			typename t = target.ToType();
			if (t && action.IsInherited(t))
				return true;
		}
		return false;
	}

	// --- Крафт ---
	void OnCraft(PlayerBase player, string recipeName, array<string> results)
	{
		BBP_SeasonConfig season;
		BBP_PlayerData data;
		BBP_SeasonProgress sp;
		if (!PrepareEvent(player, season, data, sp))
			return;

		int now = Now();
		vector pos = player.GetPosition();

		foreach (BBP_QuestConfig quest : season.Quests)
		{
			if (quest.Type != BBP_QuestType.CRAFT || !IsQuestAvailable(quest, sp))
				continue;
			if (!IsInZones(quest.Zones, pos))
				continue;

			bool matched = quest.Targets.Count() == 0 || quest.Targets.Find(recipeName) >= 0;
			if (!matched)
			{
				foreach (string result : results)
				{
					if (MatchesClassList(quest.Targets, result))
					{
						matched = true;
						break;
					}
				}
			}
			if (!matched)
				continue;

			BBP_QuestProgress qp = GetQuestProgress(sp, quest, now);
			AddQuestProgress(player, season, data, sp, quest, qp, 1);
		}
	}

	// --- Пользовательские события ---
	void OnCustomEvent(PlayerBase player, string eventName, int amount)
	{
		BBP_SeasonConfig season;
		BBP_PlayerData data;
		BBP_SeasonProgress sp;
		if (!PrepareEvent(player, season, data, sp))
			return;

		int now = Now();
		vector pos = player.GetPosition();

		foreach (BBP_QuestConfig quest : season.Quests)
		{
			if (quest.Type != BBP_QuestType.CUSTOM || !IsQuestAvailable(quest, sp))
				continue;
			if (quest.Targets.Count() > 0 && quest.Targets.Find(eventName) < 0)
				continue;
			if (!IsInZones(quest.Zones, pos))
				continue;

			BBP_QuestProgress qp = GetQuestProgress(sp, quest, now);
			AddQuestProgress(player, season, data, sp, quest, qp, amount);
		}
	}

	// --- Посещение локаций (периодическая проверка) ---
	void CheckVisits()
	{
		if (!m_Settings.Enabled)
			return;

		BBP_SeasonConfig season = GetActiveSeason();
		if (!season)
			return;

		bool hasVisitQuests = false;
		foreach (BBP_QuestConfig q : season.Quests)
		{
			if (q.Type == BBP_QuestType.VISIT && q.Points.Count() > 0)
			{
				hasVisitQuests = true;
				break;
			}
		}
		if (!hasVisitQuests)
			return;

		int now = Now();
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive() || !player.GetIdentity())
				continue;

			BBP_PlayerData data = GetPlayerData(player.GetIdentity().GetPlainId());
			BBP_SeasonProgress sp = data.GetSeason(season.Id, true);
			vector pos = player.GetPosition();

			foreach (BBP_QuestConfig quest : season.Quests)
			{
				if (quest.Type != BBP_QuestType.VISIT || quest.Points.Count() == 0 || !IsQuestAvailable(quest, sp))
					continue;

				BBP_QuestProgress qp = GetQuestProgress(sp, quest, now);
				if (qp.Completed)
					continue;

				ProcessVisit(player, season, data, sp, quest, qp, pos);
			}
		}
	}

	protected void ProcessVisit(PlayerBase player, BBP_SeasonConfig season, BBP_PlayerData data, BBP_SeasonProgress sp, BBP_QuestConfig quest, BBP_QuestProgress qp, vector pos)
	{
		if (quest.VisitMode == BBP_VisitMode.ROUTE)
		{
			int nextIndex = qp.VisitedPoints.Count();
			if (nextIndex >= quest.Points.Count())
				return;
			if (!IsInsidePoint(quest.Points[nextIndex], pos))
				return;

			qp.VisitedPoints.Insert(nextIndex);
			if (nextIndex + 1 < quest.Points.Count())
				Notify(player, quest.Name, "Точка маршрута: " + quest.Points[nextIndex].Name + " (" + (nextIndex + 1) + "/" + quest.Points.Count() + ")");
			AddQuestProgress(player, season, data, sp, quest, qp, 1);
			return;
		}

		if (quest.VisitMode == BBP_VisitMode.ALL)
		{
			for (int i = 0; i < quest.Points.Count(); i++)
			{
				if (qp.VisitedPoints.Find(i) >= 0)
					continue;
				if (!IsInsidePoint(quest.Points[i], pos))
					continue;

				qp.VisitedPoints.Insert(i);
				if (qp.VisitedPoints.Count() < quest.Points.Count())
					Notify(player, quest.Name, "Посещено: " + quest.Points[i].Name + " (" + qp.VisitedPoints.Count() + "/" + quest.Points.Count() + ")");
				AddQuestProgress(player, season, data, sp, quest, qp, 1);
			}
			return;
		}

		// Any: засчитываем вход в точку; повторно — только после выхода из неё.
		int inside = -1;
		for (int p = 0; p < quest.Points.Count(); p++)
		{
			if (IsInsidePoint(quest.Points[p], pos))
			{
				inside = p;
				break;
			}
		}

		if (inside == qp.InsidePoint)
			return;

		qp.InsidePoint = inside;
		MarkDirty(data.Uid);
		if (inside >= 0)
			AddQuestProgress(player, season, data, sp, quest, qp, 1);
	}

	// ------------------------------------------------------------
	//  Награды
	// ------------------------------------------------------------

	protected string GiveReward(PlayerBase player, BBP_PlayerData data, BBP_RewardConfig reward, string seasonId)
	{
		if (!reward || !data)
			return "";

		MarkDirty(data.Uid);
		QueueSync(data.Uid);

		switch (reward.Type)
		{
			case "Item":
				if (player)
					SpawnRewardItem(player, reward);
				else
					Print("[Boogy_BattlePass] Предмет " + reward.ClassName + " не выдан: игрок " + data.Uid + " не в сети");
				break;

			case "Coins":
				data.Coins = Math.Max(0, data.Coins + reward.Amount);
				break;

			case "Xp":
			{
				BBP_SeasonConfig xpSeason = FindSeason(seasonId);
				if (!xpSeason)
					xpSeason = GetActiveSeason();
				if (xpSeason)
					AddXp(player, xpSeason, data, data.GetSeason(xpSeason.Id, true), reward.Amount);
				break;
			}

			case "Case":
			{
				string caseId = reward.Data;
				if (caseId == "")
					caseId = reward.ClassName;
				for (int c = 0; c < Math.Max(1, reward.Amount); c++)
				{
					data.Cases.Insert(caseId);
				}
				break;
			}

			case "Premium":
			{
				string premiumSeasonId = reward.Data;
				if (premiumSeasonId == "")
					premiumSeasonId = m_ActiveSeasonId;
				if (premiumSeasonId != "")
					data.GetSeason(premiumSeasonId, true).Premium = true;
				break;
			}

			case "Custom":
				BBP_API.OnCustomReward.Invoke(player, reward.Data, reward.Amount);
				break;

			default:
				Print("[Boogy_BattlePass] Неизвестный тип награды: " + reward.Type);
				break;
		}

		return BBP_Json.Write(MakeRewardView(reward, 0));
	}

	protected void SpawnRewardItem(PlayerBase player, BBP_RewardConfig reward)
	{
		if (reward.ClassName == "")
			return;

		int count = Math.Max(1, reward.Amount);
		for (int i = 0; i < count; i++)
		{
			EntityAI ent = player.GetInventory().CreateInInventory(reward.ClassName);
			if (!ent && m_Settings.DropRewardsOnGroundIfFull)
				ent = EntityAI.Cast(GetGame().CreateObjectEx(reward.ClassName, player.GetPosition(), ECE_PLACE_ON_SURFACE));
			if (!ent)
			{
				Print("[Boogy_BattlePass] Не удалось создать предмет " + reward.ClassName);
				return;
			}

			if (reward.Quantity >= 0)
			{
				Magazine mag = Magazine.Cast(ent);
				ItemBase ib = ItemBase.Cast(ent);
				if (mag)
					mag.ServerSetAmmoCount(reward.Quantity);
				else if (ib)
					ib.SetQuantity(reward.Quantity);
			}

			if (reward.Health >= 0)
				ent.SetHealth01("", "", reward.Health);

			foreach (string attachment : reward.Attachments)
			{
				if (attachment != "")
					ent.GetInventory().CreateInInventory(attachment);
			}
		}
	}

	bool ClaimReward(PlayerBase player, int level, bool premium, bool notify = true)
	{
		if (!player || !player.GetIdentity())
			return false;

		bool ended;
		BBP_SeasonConfig season = GetDisplaySeason(ended);
		if (!season)
			return false;

		BBP_PlayerData data = GetPlayerData(player.GetIdentity().GetPlainId());
		BBP_SeasonProgress sp = data.GetSeason(season.Id, true);
		BBP_LevelConfig levelCfg = season.GetLevel(level);
		if (!levelCfg)
			return false;

		if (season.GetLevelForXp(sp.Xp) < level)
		{
			if (notify)
				Notify(player, "Боевой пропуск", "Уровень " + level + " ещё не достигнут");
			return false;
		}

		if (premium)
		{
			if (!sp.Premium)
			{
				if (notify)
					Notify(player, "Боевой пропуск", "Нужен премиум-пропуск");
				return false;
			}
			if (sp.ClaimedPremium.Find(level) >= 0 || levelCfg.PremiumRewards.Count() == 0)
				return false;
			sp.ClaimedPremium.Insert(level);
			foreach (BBP_RewardConfig premiumReward : levelCfg.PremiumRewards)
			{
				GiveReward(player, data, premiumReward, season.Id);
			}
		}
		else
		{
			if (sp.ClaimedFree.Find(level) >= 0 || levelCfg.FreeRewards.Count() == 0)
				return false;
			sp.ClaimedFree.Insert(level);
			foreach (BBP_RewardConfig freeReward : levelCfg.FreeRewards)
			{
				GiveReward(player, data, freeReward, season.Id);
			}
		}

		MarkDirty(data.Uid);
		QueueSync(data.Uid);
		if (notify)
			Notify(player, "Боевой пропуск", "Награда уровня " + level + " получена");
		return true;
	}

	int ClaimAll(PlayerBase player, bool notify = true)
	{
		if (!player || !player.GetIdentity())
			return 0;

		bool ended;
		BBP_SeasonConfig season = GetDisplaySeason(ended);
		if (!season)
			return 0;

		BBP_PlayerData data = GetPlayerData(player.GetIdentity().GetPlainId());
		BBP_SeasonProgress sp = data.GetSeason(season.Id, true);
		int reached = season.GetLevelForXp(sp.Xp);
		int claimed = 0;
		for (int lvl = 1; lvl <= reached; lvl++)
		{
			if (ClaimReward(player, lvl, false, false))
				claimed++;
			if (sp.Premium && ClaimReward(player, lvl, true, false))
				claimed++;
		}

		if (claimed > 0)
			Notify(player, "Боевой пропуск", "Получено наград: " + claimed);
		else if (notify)
			Notify(player, "Боевой пропуск", "Нет доступных наград");
		return claimed;
	}

	// ------------------------------------------------------------
	//  Премиум
	// ------------------------------------------------------------

	protected BBP_PremiumItemConfig FindPremiumItemConfig(string className)
	{
		foreach (BBP_PremiumItemConfig cfg : m_Settings.PremiumItems)
		{
			if (cfg && cfg.ClassName != "" && (cfg.ClassName == className || GetGame().IsKindOf(className, cfg.ClassName)))
				return cfg;
		}
		return null;
	}

	void OnPremiumItemUsed(PlayerBase player, ItemBase item)
	{
		if (!player || !item || !player.GetIdentity())
			return;

		BBP_PremiumItemConfig cfg = FindPremiumItemConfig(item.GetType());
		if (!cfg)
			return;

		string seasonId = cfg.SeasonId;
		if (seasonId == "")
			seasonId = m_ActiveSeasonId;

		BBP_SeasonConfig season = FindSeason(seasonId);
		if (!season)
		{
			Notify(player, "Боевой пропуск", "Сейчас нет активного сезона");
			return;
		}

		BBP_PlayerData data = GetPlayerData(player.GetIdentity().GetPlainId());
		BBP_SeasonProgress sp = data.GetSeason(season.Id, true);
		if (sp.Premium)
		{
			Notify(player, "Боевой пропуск", "Премиум для сезона \"" + season.Name + "\" уже активирован");
			return;
		}

		sp.Premium = true;
		if (cfg.Consume)
			item.Delete();

		MarkDirty(data.Uid);
		Notify(player, "Боевой пропуск", "Премиум-пропуск активирован: " + season.Name);
		if (m_Settings.AutoClaimRewards)
			ClaimAll(player, false);
		SyncState(player);
	}

	// ------------------------------------------------------------
	//  Магазин кейсов
	// ------------------------------------------------------------

	void BuyCase(PlayerBase player, string caseId)
	{
		if (!player || !player.GetIdentity() || !m_Shop.Enabled)
			return;

		BBP_CaseConfig caseCfg = m_Shop.FindCase(caseId);
		if (!caseCfg)
			return;

		string uid = player.GetIdentity().GetPlainId();
		BBP_PlayerData data = GetPlayerData(uid);

		if (caseCfg.PremiumOnly && !HasPremium(uid))
		{
			Notify(player, "Магазин", "Кейс доступен только с премиум-пропуском");
			return;
		}

		if (caseCfg.Currency == BBP_Currency.ITEM)
		{
			if (caseCfg.CurrencyItem == "")
				return;
			array<ItemBase> currencyItems = new array<ItemBase>;
			int have = CountCurrencyItems(player, caseCfg.CurrencyItem, currencyItems);
			if (have < caseCfg.Price)
			{
				Notify(player, "Магазин", "Недостаточно валюты: " + have + "/" + caseCfg.Price);
				return;
			}
			TakeCurrencyItems(currencyItems, caseCfg.Price);
		}
		else
		{
			if (data.Coins < caseCfg.Price)
			{
				Notify(player, "Магазин", "Недостаточно монет: " + data.Coins + "/" + caseCfg.Price);
				return;
			}
			data.Coins = data.Coins - caseCfg.Price;
		}

		data.Cases.Insert(caseCfg.Id);
		MarkDirty(uid);

		if (caseCfg.OpenOnPurchase)
		{
			OpenCase(player, caseCfg.Id);
			return;
		}

		Notify(player, "Магазин", "Куплен кейс: " + caseCfg.Name);
		SyncState(player);
	}

	void OpenCase(PlayerBase player, string caseId)
	{
		if (!player || !player.GetIdentity())
			return;

		BBP_CaseConfig caseCfg = m_Shop.FindCase(caseId);
		if (!caseCfg)
			return;

		BBP_PlayerData data = GetPlayerData(player.GetIdentity().GetPlainId());
		int ownedIndex = data.Cases.Find(caseId);
		if (ownedIndex < 0)
			return;

		int total = caseCfg.GetTotalWeight();
		if (total <= 0)
		{
			Notify(player, "Магазин", "Кейс пуст — обратитесь к администрации");
			return;
		}

		data.Cases.Remove(ownedIndex);

		int roll = Math.RandomInt(0, total);
		BBP_RewardConfig chosen = null;
		foreach (BBP_RewardConfig reward : caseCfg.Rewards)
		{
			int weight = Math.Max(0, reward.Weight);
			if (roll < weight)
			{
				chosen = reward;
				break;
			}
			roll = roll - weight;
		}

		if (!chosen)
			chosen = caseCfg.Rewards[caseCfg.Rewards.Count() - 1];

		string rewardJson = GiveReward(player, data, chosen, m_ActiveSeasonId);
		MarkDirty(data.Uid);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(caseCfg.Name);
		rpc.Write(rewardJson);
		rpc.Send(player, BBP_RPC.CASE_RESULT, true, player.GetIdentity());

		SyncState(player);
	}

	protected int GetCurrencyUnits(ItemBase item)
	{
		if (item.ConfigGetBool("canBeSplit"))
			return Math.Max(0, Math.Round(item.GetQuantity()));
		return 1;
	}

	protected int CountCurrencyItems(PlayerBase player, string className, array<ItemBase> found)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		int total = 0;
		foreach (EntityAI ent : items)
		{
			ItemBase ib = ItemBase.Cast(ent);
			if (!ib || !ib.IsKindOf(className))
				continue;
			found.Insert(ib);
			total = total + GetCurrencyUnits(ib);
		}
		return total;
	}

	protected void TakeCurrencyItems(array<ItemBase> items, int amount)
	{
		int remaining = amount;
		foreach (ItemBase ib : items)
		{
			if (remaining <= 0)
				break;

			int units = GetCurrencyUnits(ib);
			if (units <= remaining)
			{
				remaining = remaining - units;
				ib.Delete();
			}
			else
			{
				ib.AddQuantity(-remaining);
				remaining = 0;
			}
		}
	}

	// ------------------------------------------------------------
	//  Донат-API (HTTP)
	// ------------------------------------------------------------

	void PollDonations()
	{
		BBP_DonateConfig cfg = m_Settings.Donate;
		if (!cfg || !cfg.Enabled || cfg.BaseUrl == "" || m_DonateInFlight)
			return;

		RestContext rest = GetRest(cfg.BaseUrl);
		if (!rest)
			return;

		if (!m_DonateCb)
			m_DonateCb = new BBP_DonateCallback(this);

		m_DonateInFlight = true;
		string path = cfg.PendingPath + "?server=" + cfg.ServerId + "&key=" + cfg.ApiKey;
		rest.GET(m_DonateCb, path);
	}

	protected RestContext GetRest(string baseUrl)
	{
		if (!m_RestApi)
			m_RestApi = GetRestApi();
		if (!m_RestApi)
			m_RestApi = CreateRestApi();
		if (!m_RestApi)
			return null;
		return m_RestApi.GetRestContext(baseUrl);
	}

	void OnDonateRequestFinished()
	{
		m_DonateInFlight = false;
	}

	void OnDonateResponse(string json)
	{
		m_DonateInFlight = false;

		BBP_DonateResponse response = new BBP_DonateResponse;
		JsonSerializer js = new JsonSerializer();
		string error;
		if (!js.ReadFromString(response, json, error))
		{
			Print("[Boogy_BattlePass] Донат-API: неверный ответ: " + error);
			return;
		}

		BBP_DonateAck ack = new BBP_DonateAck;
		ack.ServerId = m_Settings.Donate.ServerId;
		ack.ApiKey = m_Settings.Donate.ApiKey;

		foreach (BBP_DonateEntry entry : response.Entries)
		{
			if (!entry || entry.Id == "")
				continue;

			if (m_Donations.ProcessedIds.Find(entry.Id) < 0)
			{
				if (!ApplyDonation(entry))
					continue;
				m_Donations.ProcessedIds.Insert(entry.Id);
			}
			// Уже обработанные тоже подтверждаем — на случай, если прошлый ack не дошёл.
			ack.Ids.Insert(entry.Id);
		}

		if (ack.Ids.Count() == 0)
			return;

		SaveDirty();
		SaveDonations();

		RestContext rest = GetRest(m_Settings.Donate.BaseUrl);
		if (!rest)
			return;
		if (!m_AckCb)
			m_AckCb = new BBP_SilentCallback();
		rest.SetHeader("application/json");
		rest.POST(m_AckCb, m_Settings.Donate.AckPath, BBP_Json.Write(ack));
	}

	protected bool ApplyDonation(BBP_DonateEntry entry)
	{
		if (entry.Uid == "")
			return false;

		bool ok = false;
		string text = "";
		string entryType = entry.Type;
		entryType.ToLower();

		if (entryType == "premium")
		{
			ok = AdminGivePremium(entry.Uid, entry.Data);
			text = "Премиум-пропуск активирован. Спасибо за поддержку!";
		}
		else if (entryType == "coins")
		{
			ok = AdminAddCoins(entry.Uid, entry.Amount);
			text = "Начислено монет: " + entry.Amount;
		}
		else if (entryType == "xp")
		{
			ok = AdminAddXp(entry.Uid, entry.Amount);
			text = "Начислено опыта: " + entry.Amount;
		}
		else if (entryType == "case")
		{
			ok = AdminGiveCase(entry.Uid, entry.Data, entry.Amount);
			text = "Получен кейс: " + entry.Data;
		}
		else
		{
			Print("[Boogy_BattlePass] Донат-API: неизвестный тип '" + entry.Type + "' (id " + entry.Id + ")");
			return false;
		}

		if (!ok)
			return false;

		Print("[Boogy_BattlePass] Донат " + entry.Id + ": " + entry.Type + " -> " + entry.Uid);
		PlayerBase player = FindOnlinePlayer(entry.Uid);
		if (player)
			Notify(player, "Боевой пропуск", text);
		return true;
	}

	// ------------------------------------------------------------
	//  Административные операции (используются API, донатом и командами)
	// ------------------------------------------------------------

	bool AdminGivePremium(string uid, string seasonId)
	{
		if (seasonId == "")
			seasonId = m_ActiveSeasonId;
		if (seasonId == "")
		{
			// Нет активного сезона — активируем ближайший предстоящий
			BBP_SeasonConfig next = GetNextSeason();
			if (next)
				seasonId = next.Id;
		}
		if (seasonId == "" || !FindSeason(seasonId))
			return false;

		BBP_PlayerData data = GetPlayerData(uid);
		if (!data)
			return false;
		data.GetSeason(seasonId, true).Premium = true;
		MarkDirty(uid);
		QueueSync(uid);
		return true;
	}

	bool AdminAddXp(string uid, int amount)
	{
		BBP_SeasonConfig season = GetActiveSeason();
		BBP_PlayerData data = GetPlayerData(uid);
		if (!season || !data)
			return false;
		AddXp(FindOnlinePlayer(uid), season, data, data.GetSeason(season.Id, true), amount);
		return true;
	}

	bool AdminAddCoins(string uid, int amount)
	{
		BBP_PlayerData data = GetPlayerData(uid);
		if (!data)
			return false;
		data.Coins = Math.Max(0, data.Coins + amount);
		MarkDirty(uid);
		QueueSync(uid);
		return true;
	}

	bool AdminGiveCase(string uid, string caseId, int amount)
	{
		if (!m_Shop.FindCase(caseId))
			return false;
		BBP_PlayerData data = GetPlayerData(uid);
		if (!data)
			return false;
		for (int i = 0; i < Math.Max(1, amount); i++)
		{
			data.Cases.Insert(caseId);
		}
		MarkDirty(uid);
		QueueSync(uid);
		return true;
	}

	bool AdminResetSeason(string uid)
	{
		BBP_SeasonConfig season = GetActiveSeason();
		BBP_PlayerData data = GetPlayerData(uid);
		if (!season || !data)
			return false;
		for (int i = data.Seasons.Count() - 1; i >= 0; i--)
		{
			if (data.Seasons[i].SeasonId == season.Id)
				data.Seasons.Remove(i);
		}
		MarkDirty(uid);
		QueueSync(uid);
		return true;
	}

	// /bp reload | xp <uid|me> <n> | coins <uid|me> <n> | premium <uid|me> [seasonId] | case <uid|me> <caseId> [n] | reset <uid|me> | event <name> [n]
	void HandleAdminCommand(PlayerBase admin, string text)
	{
		array<string> parts = new array<string>;
		string trimmed = text.Trim();
		trimmed.Split(" ", parts);
		for (int i = parts.Count() - 1; i >= 0; i--)
		{
			if (parts[i] == "")
				parts.Remove(i);
		}
		if (parts.Count() < 2)
		{
			SendAdminHelp(admin);
			return;
		}

		string cmd = parts[1];
		cmd.ToLower();

		if (cmd == "reload")
		{
			ReloadConfigs();
			Notify(admin, "BattlePass Admin", "Конфиги перезагружены");
			return;
		}

		if (cmd == "event" && parts.Count() >= 3)
		{
			int eventAmount = 1;
			if (parts.Count() >= 4)
				eventAmount = parts[3].ToInt();
			OnCustomEvent(admin, parts[2], eventAmount);
			Notify(admin, "BattlePass Admin", "Событие " + parts[2] + " x" + eventAmount);
			return;
		}

		if (parts.Count() < 3)
		{
			SendAdminHelp(admin);
			return;
		}

		string uid = parts[2];
		if (uid == "me")
			uid = admin.GetIdentity().GetPlainId();

		bool ok = false;
		if (cmd == "xp" && parts.Count() >= 4)
			ok = AdminAddXp(uid, parts[3].ToInt());
		else if (cmd == "coins" && parts.Count() >= 4)
			ok = AdminAddCoins(uid, parts[3].ToInt());
		else if (cmd == "premium")
		{
			string seasonId = "";
			if (parts.Count() >= 4)
				seasonId = parts[3];
			ok = AdminGivePremium(uid, seasonId);
		}
		else if (cmd == "case" && parts.Count() >= 4)
		{
			int caseAmount = 1;
			if (parts.Count() >= 5)
				caseAmount = parts[4].ToInt();
			ok = AdminGiveCase(uid, parts[3], caseAmount);
		}
		else if (cmd == "reset")
			ok = AdminResetSeason(uid);
		else
		{
			SendAdminHelp(admin);
			return;
		}

		if (ok)
			Notify(admin, "BattlePass Admin", "Готово: " + text);
		else
			Notify(admin, "BattlePass Admin", "Ошибка выполнения: " + text);
	}

	protected void SendAdminHelp(PlayerBase admin)
	{
		Notify(admin, "BattlePass Admin", "/bp reload | xp|coins <uid|me> <n> | premium <uid|me> [season] | case <uid|me> <id> [n] | reset <uid|me> | event <name> [n]");
	}

	// ------------------------------------------------------------
	//  RPC и синхронизация
	// ------------------------------------------------------------

	void OnPlayerRPC(PlayerBase player, PlayerIdentity sender, int rpcType, ParamsReadContext ctx)
	{
		if (!player || !sender || player.GetIdentity() != sender)
			return;

		string uid = sender.GetPlainId();
		float nowTime = GetGame().GetTickTime();
		float last;
		if (m_LastRequest.Find(uid, last) && nowTime - last < 0.25 && rpcType != BBP_RPC.REQUEST_STATE)
			return;
		m_LastRequest.Set(uid, nowTime);

		switch (rpcType)
		{
			case BBP_RPC.REQUEST_STATE:
				SyncState(player);
				break;

			case BBP_RPC.CLAIM_REWARD:
			{
				int level;
				bool premium;
				if (!ctx.Read(level) || !ctx.Read(premium))
					return;
				ClaimReward(player, level, premium);
				SyncState(player);
				break;
			}

			case BBP_RPC.CLAIM_ALL:
				ClaimAll(player);
				SyncState(player);
				break;

			case BBP_RPC.SHOP_BUY:
			{
				string buyId;
				if (!ctx.Read(buyId))
					return;
				BuyCase(player, buyId);
				break;
			}

			case BBP_RPC.CASE_OPEN:
			{
				string openId;
				if (!ctx.Read(openId))
					return;
				OpenCase(player, openId);
				break;
			}

			case BBP_RPC.ADMIN_RELOAD:
				if (!m_Settings.IsAdmin(uid))
					return;
				ReloadConfigs();
				Notify(player, "BattlePass Admin", "Конфиги перезагружены");
				break;

			case BBP_RPC.ADMIN_COMMAND:
			{
				string command;
				if (!ctx.Read(command))
					return;
				if (!m_Settings.IsAdmin(uid))
				{
					Notify(player, "Боевой пропуск", "Нет прав администратора");
					return;
				}
				HandleAdminCommand(player, command);
				break;
			}
		}
	}

	void Notify(PlayerBase player, string title, string text)
	{
		if (!player || !player.GetIdentity())
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(title);
		rpc.Write(text);
		rpc.Send(player, BBP_RPC.NOTIFY, true, player.GetIdentity());
	}

	void NotifyAll(string title, string text)
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			Notify(PlayerBase.Cast(man), title, text);
		}
	}

	// Синхронизация с задержкой — чтобы серия событий (например, очередь убийств) давала один пакет.
	void QueueSync(string uid)
	{
		if (m_Online.Find(uid) < 0)
			return;
		if (m_PendingSync.Find(uid) >= 0)
			return;
		m_PendingSync.Insert(uid);
		if (m_PendingSync.Count() == 1)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.FlushSync, 1000, false);
	}

	void FlushSync()
	{
		array<string> uids = new array<string>;
		uids.Copy(m_PendingSync);
		m_PendingSync.Clear();
		foreach (string uid : uids)
		{
			PlayerBase player = FindOnlinePlayer(uid);
			if (player)
				SyncState(player);
		}
	}

	void SyncAllOnline()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
				SyncState(pb);
		}
	}

	void SyncState(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		BBP_StateView view = BuildView(player.GetIdentity().GetPlainId());
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(BBP_Json.Write(view));
		rpc.Send(player, BBP_RPC.SYNC_STATE, true, player.GetIdentity());
	}

	protected BBP_RewardView MakeRewardView(BBP_RewardConfig reward, float chance)
	{
		BBP_RewardView view = new BBP_RewardView;
		view.Type = reward.Type;
		view.ClassName = reward.ClassName;
		view.Amount = reward.Amount;
		view.DisplayName = reward.DisplayName;
		view.Chance = chance;
		if (view.DisplayName == "" && reward.Type == BBP_RewardType.CASE)
		{
			string caseId = reward.Data;
			if (caseId == "")
				caseId = reward.ClassName;
			BBP_CaseConfig caseCfg = m_Shop.FindCase(caseId);
			if (caseCfg)
				view.DisplayName = caseCfg.Name;
		}
		return view;
	}

	protected BBP_StateView BuildView(string uid)
	{
		BBP_StateView view = new BBP_StateView;
		view.Enabled = m_Settings.Enabled;
		view.IsAdmin = m_Settings.IsAdmin(uid);
		view.ShopEnabled = m_Shop.Enabled;
		view.PremiumItemClasses.InsertAll(BBP_Shared.PremiumItemClasses);

		BBP_PlayerData data = GetPlayerData(uid);
		view.Coins = data.Coins;

		int now = Now();

		BBP_SeasonConfig next = GetNextSeason();
		if (next)
		{
			view.NextSeasonName = next.Name;
			view.NextSeasonSeconds = (next.StartMinutes - now) * 60;
		}

		bool ended;
		BBP_SeasonConfig season = GetDisplaySeason(ended);
		if (season)
		{
			BBP_SeasonProgress sp = data.GetSeason(season.Id, true);
			int reached = season.GetLevelForXp(sp.Xp);

			view.HasSeason = true;
			view.SeasonEnded = ended;
			view.SeasonId = season.Id;
			view.SeasonName = season.Name;
			view.SeasonDescription = season.Description;
			view.SecondsLeft = Math.Max(0, season.EndMinutes - now) * 60;
			view.Xp = sp.Xp;
			view.Level = reached;
			view.MaxLevel = season.GetMaxLevel();
			view.Premium = sp.Premium;

			if (reached < season.GetMaxLevel())
			{
				view.LevelXpFrom = season.LevelStartXp[reached];
				view.LevelXpTo = season.LevelEndXp[reached];
			}
			else
			{
				view.LevelXpFrom = season.GetMaxXp();
				view.LevelXpTo = season.GetMaxXp();
			}

			for (int i = 0; i < season.Levels.Count(); i++)
			{
				BBP_LevelConfig lvl = season.Levels[i];
				int levelNumber = i + 1;
				BBP_LevelView lv = new BBP_LevelView;
				lv.Level = levelNumber;
				lv.XpFrom = season.LevelStartXp[i];
				lv.XpTo = season.LevelEndXp[i];
				lv.FreeClaimed = sp.ClaimedFree.Find(levelNumber) >= 0;
				lv.PremiumClaimed = sp.ClaimedPremium.Find(levelNumber) >= 0;
				foreach (BBP_RewardConfig fr : lvl.FreeRewards)
				{
					lv.Free.Insert(MakeRewardView(fr, 0));
				}
				foreach (BBP_RewardConfig pr : lvl.PremiumRewards)
				{
					lv.Premium.Insert(MakeRewardView(pr, 0));
				}
				view.Levels.Insert(lv);

				if (levelNumber <= reached)
				{
					if (!lv.FreeClaimed && lvl.FreeRewards.Count() > 0)
						view.UnclaimedRewards++;
					if (sp.Premium && !lv.PremiumClaimed && lvl.PremiumRewards.Count() > 0)
						view.UnclaimedRewards++;
				}
			}

			foreach (BBP_QuestConfig quest : season.Quests)
			{
				BBP_QuestProgress qp = GetQuestProgress(sp, quest, now);
				BBP_QuestView qv = new BBP_QuestView;
				qv.Id = quest.Id;
				qv.Name = quest.Name;
				qv.Description = quest.Description;
				qv.Type = quest.Type;
				qv.Period = quest.Period;
				qv.Value = qp.Value;
				qv.Count = quest.GetRequiredCount();
				qv.Xp = quest.Xp;
				qv.Coins = quest.Coins;
				qv.Completed = qp.Completed;
				qv.PremiumOnly = quest.PremiumOnly;
				qv.Locked = !IsQuestAvailable(quest, sp);
				foreach (BBP_RewardConfig qr : quest.Rewards)
				{
					qv.Rewards.Insert(MakeRewardView(qr, 0));
				}
				view.Quests.Insert(qv);
			}
		}

		foreach (BBP_CaseConfig caseCfg : m_Shop.Cases)
		{
			BBP_CaseView cv = new BBP_CaseView;
			cv.Id = caseCfg.Id;
			cv.Name = caseCfg.Name;
			cv.Description = caseCfg.Description;
			cv.Price = caseCfg.Price;
			cv.Currency = caseCfg.Currency;
			cv.CurrencyItem = caseCfg.CurrencyItem;
			cv.PremiumOnly = caseCfg.PremiumOnly;
			cv.Owned = data.CountCases(caseCfg.Id);
			int totalWeight = caseCfg.GetTotalWeight();
			foreach (BBP_RewardConfig cr : caseCfg.Rewards)
			{
				float chance = 0;
				if (totalWeight > 0)
					chance = Math.Max(0, cr.Weight) * 100.0 / totalWeight;
				cv.Rewards.Insert(MakeRewardView(cr, chance));
			}
			view.Cases.Insert(cv);
		}

		return view;
	}

	// ------------------------------------------------------------
	//  Утилиты
	// ------------------------------------------------------------

	static PlayerBase ResolvePlayer(Object obj)
	{
		PlayerBase pb = PlayerBase.Cast(obj);
		if (pb)
			return pb;
		EntityAI entity = EntityAI.Cast(obj);
		if (entity)
			return PlayerBase.Cast(entity.GetHierarchyRootPlayer());
		return null;
	}
}

class BBP_DonateCallback extends RestCallback
{
	protected BBP_Manager m_Manager;

	void BBP_DonateCallback(BBP_Manager manager)
	{
		m_Manager = manager;
	}

	override void OnSuccess(string data, int dataSize)
	{
		if (m_Manager)
			m_Manager.OnDonateResponse(data);
	}

	override void OnError(int errorCode)
	{
		Print("[Boogy_BattlePass] Донат-API: ошибка запроса " + errorCode);
		if (m_Manager)
			m_Manager.OnDonateRequestFinished();
	}

	override void OnTimeout()
	{
		Print("[Boogy_BattlePass] Донат-API: таймаут");
		if (m_Manager)
			m_Manager.OnDonateRequestFinished();
	}
}

class BBP_SilentCallback extends RestCallback
{
	override void OnError(int errorCode) {}
	override void OnTimeout() {}
	override void OnSuccess(string data, int dataSize) {}
}
