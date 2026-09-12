class SM_ClanManager
{
	protected static ref SM_ClanManager s_Instance;

	static const string CLAN_DB_DIR = "$profile:SM_PartyMod\\Clans";
	static const string CLAN_INDEX_PATH = "$profile:SM_PartyMod\\Clans\\Index.json";
	static const string CHAT_MUTES_PATH = "$profile:SM_PartyMod\\ChatMutes.json";

	protected static const int STATS_TICK_SECONDS = 10;
	protected static const int MAX_WITHDRAW_ITEMS = 100; // защита от завала инвентаря при снятии
	protected static const float BASE_FLAG_MATCH_RADIUS = 2.0;
	protected static const float NOTIFY_SHOW_SECONDS = 4.0;
	protected static const float NOTIFY_REPEAT_COOLDOWN_SECONDS = 8.0;
	protected static const float NOTIFY_CLEANUP_INTERVAL_SECONDS = 60.0;
	protected static const float TOP_LONG_KILL_HIT_WINDOW_SECONDS = 60.0;
	protected static const int BASE_AUDIT_MEMBER_LIST_LIMIT = 1800;

	protected ref SM_ClanDatabase m_DB;
	protected ref SM_PartyConfig m_Config;
	protected ref map<string, ref array<ref SM_Invite>> m_Invites;
	protected ref map<string, float> m_LastPings;
	protected ref map<string, int> m_PingCounters;
	protected ref SM_MarketDB m_MarketDB;
	protected ref SM_ServerMarketState m_ServerMarketState;
	protected ref SM_StorageDB m_StorageDB;
	protected ref SM_AchievementsDB m_AchievementsDB;
	protected ref SM_PlayerExperienceDB m_PlayerExperienceDB;
	protected ref SM_AuctionState m_AuctionState;
	protected ref SM_ContractsDB m_ContractsDB;
	protected ref SM_ChatMutesDB m_ChatMutesDB;
	protected ref array<ref SM_AdminChatLogEntry> m_AdminChatLog;
	protected ref map<string, vector> m_LastPositions;
	protected ref map<string, vector> m_LastAchievementPositions;
	protected ref map<string, int> m_PlayerExperienceOnlineSeconds;
	protected ref map<string, string> m_AdminClanViews;
	protected ref map<string, int> m_HudActivityStatuses;
	protected ref map<string, float> m_LastNotifications;
	protected ref map<string, float> m_LastAdminConfigReloads;
	protected ref map<string, string> m_LastPlayerHitShooterUid;
	protected ref map<string, float> m_LastPlayerHitDistance;
	protected ref map<string, float> m_LastPlayerHitTime;
	protected ref map<string, bool> m_RestrictedItemCheckQueued;
	protected ref array<ref SM_ExternalServerMapMarker> m_ExternalServerMapMarkers;
	protected ref map<string, ref SM_DeathMapMarkerView> m_PendingPersonalDeathMapMarkers;
	protected string m_ServerSessionKey;
	protected float m_LastNotificationCleanupTime;
	protected bool m_StatsDirty;
	protected bool m_AchievementsDirty;
	protected bool m_PlayerExperienceDirty;

	static SM_ClanManager Get()
	{
		return s_Instance;
	}

	static void Init()
	{
		if (!s_Instance)
			s_Instance = new SM_ClanManager();
	}

	void SM_ClanManager()
	{
		m_Config = SM_PartyConfigLoader.Get();
		m_ServerSessionKey = BuildServerSessionKey();
		m_Invites = new map<string, ref array<ref SM_Invite>>;
		m_LastPings = new map<string, float>;
		m_PingCounters = new map<string, int>;
		m_LastPositions = new map<string, vector>;
		m_LastAchievementPositions = new map<string, vector>;
		m_PlayerExperienceOnlineSeconds = new map<string, int>;
		m_AdminClanViews = new map<string, string>;
		m_HudActivityStatuses = new map<string, int>;
		m_LastNotifications = new map<string, float>;
		m_LastAdminConfigReloads = new map<string, float>;
		m_LastPlayerHitShooterUid = new map<string, string>;
		m_LastPlayerHitDistance = new map<string, float>;
		m_LastPlayerHitTime = new map<string, float>;
		m_RestrictedItemCheckQueued = new map<string, bool>;
		m_ExternalServerMapMarkers = new array<ref SM_ExternalServerMapMarker>;
		m_PendingPersonalDeathMapMarkers = new map<string, ref SM_DeathMapMarkerView>;
		m_AdminChatLog = new array<ref SM_AdminChatLogEntry>;
		LoadDB();
		LoadMarketDB();
		LoadServerMarketState();
		LoadStorageDB();
		LoadAchievementsDB();
		LoadPlayerExperienceDB();
		LoadAuctionState();
		LoadContractsDB();
		LoadChatMutesDB();
		ReconcileOrphanedItems();
		EnsureTopRewardSeasonStarted(true);

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.HudTick, m_Config.MemberHud.UpdateIntervalSeconds * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.InviteTick, 10000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.StatsTick, STATS_TICK_SECONDS * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.StatsSaveTick, 120000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.RewardTick, 300000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ServerMarketTick, 60000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.AuctionTick, 60000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.TradePendingTick, 60000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ContractsTick, m_Config.Contracts.TickSeconds * 1000, true);

		Print(SM_PartyLoc.Text("#STR_SMP_00176" + m_DB.Clans.Count()));
	}

	// ---------- база ----------

	protected void LoadDB()
	{
		if (!FileExist(SM_PartyConfigLoader.DIR_PATH))
			MakeDirectory(SM_PartyConfigLoader.DIR_PATH);
		if (!FileExist(CLAN_DB_DIR))
			MakeDirectory(CLAN_DB_DIR);

		m_DB = new SM_ClanDatabase();

		if (FileExist(CLAN_INDEX_PATH))
			LoadClanFileDB();

		if (!m_DB)
			m_DB = new SM_ClanDatabase();
		if (!m_DB.Clans)
			m_DB.Clans = new array<ref SM_Clan>;

		RemoveNullClans();
		EnsureClanIds();

		int leaderRank = m_Config.GetLeaderRank();
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (!clan.Members)
				clan.Members = new array<ref SM_ClanMember>;
			if (!clan.Log)
				clan.Log = new array<string>;
			if (!clan.Applications)
				clan.Applications = new array<ref SM_ClanApplication>;
			if (!clan.MapMarkers)
				clan.MapMarkers = new array<ref SM_ClanMapMarker>;
			if (clan.NextMapMarkerId < 1)
				clan.NextMapMarkerId = 1;
			foreach (SM_ClanMapMarker marker : clan.MapMarkers)
			{
				if (marker.Id >= clan.NextMapMarkerId)
					clan.NextMapMarkerId = marker.Id + 1;
			}
			if (!clan.Stats)
				clan.Stats = new SM_ClanStats();
			if (!clan.ClaimedAchievements)
				clan.ClaimedAchievements = new array<string>;
			if (!clan.NotifiedAchievements)
				clan.NotifiedAchievements = new array<string>;
			if (clan.Treasury < 0)
				clan.Treasury = 0;
			if (clan.Level < 1)
				clan.Level = 1;
			if (clan.Level > m_Config.GetMaxLevel())
				clan.Level = m_Config.GetMaxLevel();
			if (clan.DiscordWebhookUrl == "")
			{
				clan.DiscordWebhookEnabled = false;
				clan.DiscordAuditEnabled = false;
			}
			if (!clan.DiscordWebhookEnabled)
				clan.DiscordAuditEnabled = false;
			foreach (SM_ClanMember member : clan.Members)
			{
				if (member.Rank > leaderRank)
					member.Rank = leaderRank;
				if (member.Rank < 0)
					member.Rank = 0;
			}

			if (!clan.Perms)
				clan.Perms = new array<int>;
			if (clan.Perms.Count() < SM_ClanAction.COUNT)
			{
				array<int> defaultPerms;
				m_Config.GetDefaultPerms(defaultPerms);
				while (clan.Perms.Count() < SM_ClanAction.COUNT)
					clan.Perms.Insert(defaultPerms[clan.Perms.Count()]);
			}
			for (int p = 0; p < clan.Perms.Count(); p++)
			{
				if (clan.Perms[p] < 0)
					clan.Perms.Set(p, 0);
				if (clan.Perms[p] > leaderRank)
					clan.Perms.Set(p, leaderRank);
			}
		}
	}

	protected bool LoadClanFileDB()
	{
		SM_ClanIndex index = new SM_ClanIndex();
		string errorMessage;
		if (!JsonFileLoader<SM_ClanIndex>.LoadFile(CLAN_INDEX_PATH, index, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00201" + errorMessage));
			return false;
		}

		if (!index)
			return false;
		if (!index.Clans)
			index.Clans = new array<ref SM_ClanIndexEntry>;

		m_DB.LastRewardYear = index.LastRewardYear;
		m_DB.LastRewardMonth = index.LastRewardMonth;
		m_DB.LastRewardStampMinutes = index.LastRewardStampMinutes;
		m_DB.LastRewardStampSeconds = index.LastRewardStampSeconds;

		foreach (SM_ClanIndexEntry entry : index.Clans)
		{
			if (!entry)
				continue;
			if (!IsSafeClanFileName(entry.FileName))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00213" + entry.FileName));
				continue;
			}

			SM_Clan clan = new SM_Clan();
			string clanPath = CLAN_DB_DIR + "\\" + entry.FileName;
			if (!JsonFileLoader<SM_Clan>.LoadFile(clanPath, clan, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00210" + clanPath + ": " + errorMessage));
				continue;
			}
			if (!clan)
				continue;
			if (clan.Id == "")
				clan.Id = entry.Id;
			if (clan.Name == "" && entry.Name != "")
				clan.Name = entry.Name;
			m_DB.Clans.Insert(clan);
		}

		return true;
	}

	protected void RemoveNullClans()
	{
		for (int i = m_DB.Clans.Count() - 1; i >= 0; i--)
		{
			if (!m_DB.Clans[i])
				m_DB.Clans.Remove(i);
		}
	}

	protected bool IsSafeClanFileName(string fileName)
	{
		if (fileName == "")
			return false;
		if (fileName.Contains("\\") || fileName.Contains("/") || fileName.Contains(":") || fileName.Contains(".."))
			return false;
		if (fileName.Length() < 6)
			return false;
		if (fileName.Substring(fileName.Length() - 5, 5) != ".json")
			return false;
		return true;
	}

	protected string MakeClanId(string clanName)
	{
		string lower = clanName;
		lower.ToLower();
		int hash = lower.Hash();
		if (hash < 0)
			hash = Math.AbsInt(hash);
		if (hash < 0)
			hash = 0;
		return "clan_" + hash.ToString();
	}

	protected string NormalizeClanId(string clanId)
	{
		string result = "";
		for (int i = 0; i < clanId.Length(); i++)
		{
			string c = clanId.Get(i);
			if (SM_PartyUtil.LATIN_ALNUM.Contains(c) || c == "_" || c == "-")
				result = result + c;
			else
				result = result + "_";
		}
		return result;
	}

	protected bool IsClanIdUsed(string clanId, SM_Clan owner)
	{
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (clan == owner)
				continue;
			if (clan.Id == clanId)
				return true;
		}
		return false;
	}

	protected void EnsureClanIds()
	{
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (!clan)
				continue;

			string clanId = NormalizeClanId(clan.Id);
			if (clanId == "")
				clanId = MakeClanId(clan.Name);

			clan.Id = clanId;
			string baseId = clan.Id;
			int suffix = 2;
			while (IsClanIdUsed(clan.Id, clan))
			{
				clan.Id = baseId + "_" + suffix.ToString();
				suffix++;
			}
		}
	}

	protected string GetClanFileName(SM_Clan clan)
	{
		return clan.Id + ".json";
	}

	protected bool HasCurrentClanFile(string fileName)
	{
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (GetClanFileName(clan) == fileName)
				return true;
		}
		return false;
	}

	protected SM_ClanIndex LoadExistingClanIndex()
	{
		if (!FileExist(CLAN_INDEX_PATH))
			return null;

		SM_ClanIndex index = new SM_ClanIndex();
		string errorMessage;
		if (!JsonFileLoader<SM_ClanIndex>.LoadFile(CLAN_INDEX_PATH, index, errorMessage))
			return null;
		if (!index || !index.Clans)
			return null;
		return index;
	}

	protected void CleanupDeletedClanFiles(SM_ClanIndex oldIndex)
	{
		if (!oldIndex || !oldIndex.Clans)
			return;

		foreach (SM_ClanIndexEntry entry : oldIndex.Clans)
		{
			if (!entry)
				continue;
			if (!IsSafeClanFileName(entry.FileName))
				continue;
			if (HasCurrentClanFile(entry.FileName))
				continue;
			DeleteFile(CLAN_DB_DIR + "\\" + entry.FileName);
		}
	}

	protected void AddLog(SM_Clan clan, string text)
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);

		string stamp = FormatTwo(day) + "." + FormatTwo(month) + " " + FormatTwo(hour) + ":" + FormatTwo(minute);
		string line = stamp + "  " + text;
		clan.Log.InsertAt(line, 0); // свежие сверху

		while (clan.Log.Count() > m_Config.Logs.MaxEntries)
			clan.Log.Remove(clan.Log.Count() - 1);

		if (clan.DiscordAuditEnabled && clan.DiscordWebhookEnabled && clan.DiscordWebhookUrl != "")
			RelayToDiscordUrl(clan.DiscordWebhookUrl, "#STR_SMP_00255" + clan.Name, "SM_PartyMod", line);
	}

	protected string FormatTwo(int value)
	{
		if (value < 10)
			return "0" + value.ToString();
		return value.ToString();
	}

	protected string GetChatStamp()
	{
		int hour;
		int minute;
		int second;
		GetHourMinuteSecond(hour, minute, second);
		return FormatTwo(hour) + ":" + FormatTwo(minute);
	}

	protected void SendChatHistory(PlayerBase target, int channel, string label, string prefix, string author, string text, int channelColor, int nameColor, int textColor, int prefixColor)
	{
		if (!target || !target.GetIdentity())
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(channel);
		rpc.Write(label);
		rpc.Write(prefix);
		rpc.Write(author);
		rpc.Write(text);
		rpc.Write(GetChatStamp());
		rpc.Write(channelColor);
		rpc.Write(nameColor);
		rpc.Write(textColor);
		rpc.Write(prefixColor);
		rpc.Send(target, SM_PartyRPC.CHAT_RECEIVE, true, target.GetIdentity());
	}

	void SaveDB()
	{
		string errorMessage;
		if (!FileExist(CLAN_DB_DIR))
			MakeDirectory(CLAN_DB_DIR);

		EnsureClanIds();

		SM_ClanIndex oldIndex = LoadExistingClanIndex();
		SM_ClanIndex index = new SM_ClanIndex();
		index.LastRewardYear = m_DB.LastRewardYear;
		index.LastRewardMonth = m_DB.LastRewardMonth;
		index.LastRewardStampMinutes = m_DB.LastRewardStampMinutes;
		index.LastRewardStampSeconds = m_DB.LastRewardStampSeconds;

		bool saveFailed = false;
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (!clan)
				continue;

			string fileName = GetClanFileName(clan);
			index.Clans.Insert(new SM_ClanIndexEntry(clan.Id, clan.Name, fileName));
			if (!JsonFileLoader<SM_Clan>.SaveFile(CLAN_DB_DIR + "\\" + fileName, clan, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00194" + clan.Name + ": " + errorMessage));
				saveFailed = true;
			}
		}

		if (saveFailed)
			return;

		if (!JsonFileLoader<SM_ClanIndex>.SaveFile(CLAN_INDEX_PATH, index, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00185" + errorMessage));
			return;
		}

		CleanupDeletedClanFiles(oldIndex);
	}

	protected void LoadChatMutesDB()
	{
		if (!FileExist(SM_PartyConfigLoader.DIR_PATH))
			MakeDirectory(SM_PartyConfigLoader.DIR_PATH);

		m_ChatMutesDB = new SM_ChatMutesDB();
		if (FileExist(CHAT_MUTES_PATH))
		{
			string errorMessage;
			if (!JsonFileLoader<SM_ChatMutesDB>.LoadFile(CHAT_MUTES_PATH, m_ChatMutesDB, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00200" + errorMessage));
				m_ChatMutesDB = new SM_ChatMutesDB();
			}
		}

		if (!m_ChatMutesDB)
			m_ChatMutesDB = new SM_ChatMutesDB();
		if (!m_ChatMutesDB.Mutes)
			m_ChatMutesDB.Mutes = new array<ref SM_ChatMuteRecord>;

		CleanupExpiredChatMutes(true);
	}

	protected void SaveChatMutesDB()
	{
		if (!FileExist(SM_PartyConfigLoader.DIR_PATH))
			MakeDirectory(SM_PartyConfigLoader.DIR_PATH);

		if (!m_ChatMutesDB)
			m_ChatMutesDB = new SM_ChatMutesDB();
		if (!m_ChatMutesDB.Mutes)
			m_ChatMutesDB.Mutes = new array<ref SM_ChatMuteRecord>;

		string errorMessage;
		if (!JsonFileLoader<SM_ChatMutesDB>.SaveFile(CHAT_MUTES_PATH, m_ChatMutesDB, errorMessage))
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00184" + errorMessage));
	}

	protected void CleanupExpiredChatMutes(bool saveIfChanged = false)
	{
		if (!m_ChatMutesDB || !m_ChatMutesDB.Mutes)
			return;

		int now = GetNowAbsSeconds();
		bool changed = false;
		for (int i = m_ChatMutesDB.Mutes.Count() - 1; i >= 0; i--)
		{
			SM_ChatMuteRecord mute = m_ChatMutesDB.Mutes[i];
			if (!mute || mute.Uid == "" || mute.Until <= now)
			{
				m_ChatMutesDB.Mutes.Remove(i);
				changed = true;
			}
		}

		if (changed && saveIfChanged)
			SaveChatMutesDB();
	}


	SM_Clan FindClan(string name)
	{
		string searched = name;
		searched.ToLower();
		foreach (SM_Clan clan : m_DB.Clans)
		{
			string current = clan.Name;
			current.ToLower();
			if (current == searched)
				return clan;
		}
		return null;
	}

	SM_Clan FindClanByMember(string uid)
	{
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (clan.FindMember(uid))
				return clan;
		}
		return null;
	}

	protected bool IsClanAdmin(string uid)
	{
		if (!m_Config || !m_Config.Ranks.AdminUids)
			return false;
		if (m_Config.Ranks.AdminUids.Find(uid) >= 0)
			return true;
		return false;
	}

	protected SM_Clan GetAdminViewedClan(string uid)
	{
		if (!IsClanAdmin(uid))
			return null;

		string clanName;
		if (!m_AdminClanViews.Find(uid, clanName))
			return null;

		SM_Clan clan = FindClan(clanName);
		if (!clan)
		{
			m_AdminClanViews.Remove(uid);
			return null;
		}
		return clan;
	}

	protected bool IsAdminModeForClan(string uid, SM_Clan clan)
	{
		if (!clan)
			return false;

		SM_Clan adminClan = GetAdminViewedClan(uid);
		if (adminClan && adminClan == clan)
			return true;
		return false;
	}

	protected SM_Clan GetActorClan(string uid)
	{
		SM_Clan adminClan = GetAdminViewedClan(uid);
		if (adminClan)
			return adminClan;
		return FindClanByMember(uid);
	}

	protected int GetActorRank(string uid, SM_Clan clan)
	{
		if (!clan)
			return -1;
		if (IsAdminModeForClan(uid, clan))
			return m_Config.GetLeaderRank();

		SM_ClanMember member = clan.FindMember(uid);
		if (!member)
			return -1;
		return member.Rank;
	}

	protected string GetActorName(PlayerBase player, SM_Clan clan)
	{
		string uid = player.GetIdentity().GetPlainId();
		if (IsAdminModeForClan(uid, clan))
			return "#STR_SMP_00241" + player.GetIdentity().GetName();

		SM_ClanMember member = clan.FindMember(uid);
		if (member)
			return member.Name;
		return player.GetIdentity().GetName();
	}

	protected bool HasActionAccess(PlayerBase player, SM_Clan clan, int action, string title, string deniedText)
	{
		if (!player || !player.GetIdentity() || !clan)
			return false;

		string uid = player.GetIdentity().GetPlainId();
		int actorRank = GetActorRank(uid, clan);
		if (actorRank >= clan.GetMinRank(action))
			return true;

		Notify(player, title, deniedText);
		return false;
	}

	protected bool IsSameBaseFlag(vector a, vector b)
	{
		if (vector.Distance(a, b) <= BASE_FLAG_MATCH_RADIUS)
			return true;
		return false;
	}

	protected SM_Clan FindClanByBaseFlag(TerritoryFlag flag)
	{
		if (!flag)
			return null;

		vector flagPos = flag.GetPosition();
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (clan.HasBase && IsSameBaseFlag(clan.BasePos, flagPos))
				return clan;
		}
		return null;
	}

	protected TerritoryFlag FindBaseFlagAtPosition(vector basePos)
	{
		array<Object> objects = new array<Object>;
		array<CargoBase> proxyCargos = new array<CargoBase>;
		GetGame().GetObjectsAtPosition3D(basePos, BASE_FLAG_MATCH_RADIUS, objects, proxyCargos);

		foreach (Object obj : objects)
		{
			TerritoryFlag flag = TerritoryFlag.Cast(obj);
			if (!flag)
				continue;
			if (flag.IsDamageDestroyed())
				continue;
			if (IsSameBaseFlag(flag.GetPosition(), basePos))
				return flag;
		}

		return null;
	}

	protected bool ClanBaseFlagExists(SM_Clan clan)
	{
		if (!clan)
			return false;
		if (!clan.HasBase)
			return false;
		if (FindBaseFlagAtPosition(clan.BasePos))
			return true;
		return false;
	}

	protected bool ShouldSendBaseServerMarker(SM_Clan clan)
	{
		if (!clan)
			return false;
		if (!clan.HasBase)
			return false;
		return true;
	}

	protected bool ShouldSendClanMapMarkerToUid(SM_ClanMapMarker marker, string uid)
	{
		if (!marker)
			return false;
		if (marker.Icon == SM_MapMarkerIconSet.DeathIcon() && marker.AuthorUid == uid)
			return false;
		return true;
	}

	protected int CountClanMapMarkersForUid(SM_Clan clan, string uid)
	{
		if (!clan || !clan.MapMarkers)
			return 0;

		int count = 0;
		foreach (SM_ClanMapMarker marker : clan.MapMarkers)
		{
			if (ShouldSendClanMapMarkerToUid(marker, uid))
				count++;
		}
		return count;
	}

	protected void WriteServerMapMarker(ScriptRPC rpc, string name, vector position, int color, string iconPaa)
	{
		iconPaa = SM_MapMarkerIconPath.ForMapWidget(iconPaa);
		rpc.Write(name);
		rpc.Write(position);
		rpc.Write(color);
		rpc.Write(iconPaa);
	}

	protected void EnsureExternalServerMapMarkers()
	{
		if (!m_ExternalServerMapMarkers)
			m_ExternalServerMapMarkers = new array<ref SM_ExternalServerMapMarker>;
	}

	protected int FindExternalServerMapMarkerIndex(string source, string id)
	{
		EnsureExternalServerMapMarkers();
		for (int i = 0; i < m_ExternalServerMapMarkers.Count(); i++)
		{
			SM_ExternalServerMapMarker marker = m_ExternalServerMapMarkers[i];
			if (!marker)
				continue;
			if (marker.Source == source && marker.Id == id)
				return i;
		}
		return -1;
	}

	protected int CountExternalServerMapMarkers()
	{
		EnsureExternalServerMapMarkers();

		int count = 0;
		foreach (SM_ExternalServerMapMarker marker : m_ExternalServerMapMarkers)
		{
			if (marker)
				count++;
		}
		return count;
	}

	protected void WriteExternalServerMapMarkers(ScriptRPC rpc)
	{
		EnsureExternalServerMapMarkers();

		foreach (SM_ExternalServerMapMarker marker : m_ExternalServerMapMarkers)
		{
			if (!marker)
				continue;

			WriteServerMapMarker(rpc, marker.Name, marker.Position, marker.Color, marker.GetMapIconPath());
		}
	}

	protected void SyncStateToOnlinePlayers()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;

			SyncStateTo(pb);
		}
	}

	void UpsertExternalServerMapMarker(string source, string id, string name, vector position, int color, string iconPaa)
	{
		if (source == "" || id == "")
			return;

		EnsureExternalServerMapMarkers();

		if (name == "")
			name = source;
		if (color == 0)
			color = SM_ClanColors.GetColor(0);
		iconPaa = SM_MapMarkerIconPath.Normalize(iconPaa);
		if (iconPaa == "")
			iconPaa = SM_ClanMarkerIcon.GetPath();

		int index = FindExternalServerMapMarkerIndex(source, id);
		if (index >= 0)
		{
			SM_ExternalServerMapMarker existing = m_ExternalServerMapMarkers[index];
			if (existing)
			{
				existing.Name = name;
				existing.Position = position;
				existing.Color = color;
				existing.IconPaa = iconPaa;
			}
		}
		else
		{
			m_ExternalServerMapMarkers.Insert(new SM_ExternalServerMapMarker(source, id, name, position, color, iconPaa));
		}

		SyncStateToOnlinePlayers();
	}

	void RemoveExternalServerMapMarker(string source, string id)
	{
		if (source == "" || id == "")
			return;

		int index = FindExternalServerMapMarkerIndex(source, id);
		if (index < 0)
			return;

		m_ExternalServerMapMarkers.Remove(index);
		SyncStateToOnlinePlayers();
	}

	void ClearExternalServerMapMarkers(string source)
	{
		if (source == "")
			return;

		EnsureExternalServerMapMarkers();

		bool changed = false;
		for (int i = m_ExternalServerMapMarkers.Count() - 1; i >= 0; i--)
		{
			SM_ExternalServerMapMarker marker = m_ExternalServerMapMarkers[i];
			if (!marker || marker.Source == source)
			{
				m_ExternalServerMapMarkers.Remove(i);
				changed = true;
			}
		}

		if (changed)
			SyncStateToOnlinePlayers();
	}

	protected int CountAdminBaseMapMarkers()
	{
		if (!m_DB || !m_DB.Clans)
			return 0;

		int count = 0;
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (clan && clan.HasBase)
				count++;
		}
		return count;
	}

	protected string GetAdminBaseClanName(SM_Clan clan)
	{
		string clanName = "#STR_SMP_00274";
		if (clan && clan.Name != "")
			clanName = clan.Name;

		return clanName;
	}

	protected string GetAdminBaseOwnerName(SM_Clan clan)
	{
		string ownerName = "#STR_SMP_00678";
		SM_ClanMember owner = GetClanLeaderMember(clan);
		if (owner && owner.Name != "")
			ownerName = owner.Name;

		return ownerName;
	}

	protected string GetAdminBaseOwnerUid(SM_Clan clan)
	{
		SM_ClanMember owner = GetClanLeaderMember(clan);
		if (owner && owner.Uid != "")
			return owner.Uid;
		return "";
	}

	protected void WriteAdminBaseMapMarker(ScriptRPC rpc, SM_Clan clan)
	{
		string clanName = GetAdminBaseClanName(clan);
		string ownerName = GetAdminBaseOwnerName(clan);
		string ownerUid = GetAdminBaseOwnerUid(clan);
		string label = "#STR_SMP_00266" + clanName + "#STR_SMP_00024" + ownerName;

		rpc.Write(label);
		rpc.Write(clanName);
		rpc.Write(ownerName);
		rpc.Write(ownerUid);
		rpc.Write(clan.BasePos);
		rpc.Write(clan.Color);
		rpc.Write("\\dz\\gear\\navigation\\data\\map_camp_ca.paa");
	}

	protected void WriteAdminBaseMapMarkers(ScriptRPC rpc, bool isAdmin)
	{
		if (!isAdmin)
		{
			rpc.Write(0);
			return;
		}

		rpc.Write(CountAdminBaseMapMarkers());
		if (!m_DB || !m_DB.Clans)
			return;

		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (!clan || !clan.HasBase)
				continue;

			WriteAdminBaseMapMarker(rpc, clan);
		}
	}

	protected void BuildOnlineMap(out map<string, PlayerBase> onlineMap)
	{
		onlineMap = new map<string, PlayerBase>;
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
				onlineMap.Set(pb.GetIdentity().GetPlainId(), pb);
		}
	}

	protected PlayerBase FindOnlinePlayer(string uid)
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

	protected void CleanupNotificationThrottle(float now)
	{
		if (!m_LastNotifications)
			return;
		if (now - m_LastNotificationCleanupTime < NOTIFY_CLEANUP_INTERVAL_SECONDS)
			return;

		m_LastNotificationCleanupTime = now;
		float maxAge = NOTIFY_REPEAT_COOLDOWN_SECONDS * 3.0;
		for (int i = m_LastNotifications.Count() - 1; i >= 0; i--)
		{
			float lastTime = m_LastNotifications.GetElement(i);
			if (now - lastTime > maxAge)
				m_LastNotifications.Remove(m_LastNotifications.GetKey(i));
		}
	}

	protected void Notify(PlayerBase player, string title, string text)
	{
		if (!player || !player.GetIdentity())
			return;

		if (!m_LastNotifications)
			m_LastNotifications = new map<string, float>;

		float now = GetGame().GetTickTime();
		CleanupNotificationThrottle(now);

		string key = player.GetIdentity().GetPlainId() + "|" + title + "|" + text;
		float lastNotifyTime;
		if (m_LastNotifications.Find(key, lastNotifyTime))
		{
			if (now - lastNotifyTime < NOTIFY_REPEAT_COOLDOWN_SECONDS)
				return;
		}

		m_LastNotifications.Set(key, now);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(NOTIFY_SHOW_SECONDS);
		rpc.Write(title);
		rpc.Write(text);
		rpc.Write("");
		rpc.Send(player, SM_PartyRPC.LOCAL_NOTIFY, true, player.GetIdentity());
	}

	protected void NotifyClan(SM_Clan clan, string title, string text, string exceptUid = "")
	{
		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_ClanMember member : clan.Members)
		{
			if (member.Uid == exceptUid)
				continue;
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
				Notify(pb, title, text);
		}
	}

	void OnPlayerRPC(PlayerBase player, PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		if (!player || !sender)
			return;

		PlayerIdentity own = player.GetIdentity();
		if (!own || own.GetPlainId() != sender.GetPlainId())
			return;

		string s1;
		string clanName;
		string clanTag;
		string clanDescription;
		int colorIndex;
		vector markPos;
	string markerName;
	int markerId;
	int chatChannel;
	int hudActivityStatus;

		switch (rpc_type)
		{
			case SM_PartyRPC.CREATE_CLAN:
				if (ctx.Read(clanName) && ctx.Read(clanTag))
				{
					if (!ctx.Read(clanDescription))
						clanDescription = "";
					HandleCreateClan(player, clanName, clanTag, clanDescription);
				}
				break;

			case SM_PartyRPC.INVITE:
				if (ctx.Read(s1))
					HandleInvite(player, s1);
				break;

			case SM_PartyRPC.ACCEPT_INVITE:
				if (ctx.Read(s1))
					HandleAcceptInvite(player, s1);
				break;

			case SM_PartyRPC.DECLINE_INVITE:
				if (ctx.Read(s1))
					HandleDeclineInvite(player, s1);
				break;

			case SM_PartyRPC.KICK:
				if (ctx.Read(s1))
					HandleKick(player, s1);
				break;

			case SM_PartyRPC.PROMOTE:
				if (ctx.Read(s1))
					HandleRankChange(player, s1, 1);
				break;

			case SM_PartyRPC.DEMOTE:
				if (ctx.Read(s1))
					HandleRankChange(player, s1, -1);
				break;

			case SM_PartyRPC.LEAVE:
				HandleLeave(player);
				break;

			case SM_PartyRPC.DISBAND:
				HandleDisband(player);
				break;

			case SM_PartyRPC.REQUEST_STATE:
				SyncStateTo(player);
				break;

			case SM_PartyRPC.REQUEST_CLAN_LIST:
				SendClanList(player);
				break;

			case SM_PartyRPC.REQUEST_PLAYERS:
				SendInvitablePlayers(player);
				break;

			case SM_PartyRPC.REQUEST_ONLINE_PLAYERS:
				SendOnlinePlayers(player);
				break;

			case SM_PartyRPC.SET_COLOR:
				if (ctx.Read(colorIndex))
					HandleSetColor(player, colorIndex);
				break;

			case SM_PartyRPC.CHAT_SEND:
				if (ctx.Read(chatChannel) && ctx.Read(s1))
					HandleChat(player, chatChannel, s1);
				break;

			case SM_PartyRPC.MARK_PLACE:
				if (ctx.Read(markPos))
					HandleMarkPlace(player, markPos);
				break;

			case SM_PartyRPC.MARK_ITEM:
				if (ctx.Read(markPos) && ctx.Read(s1))
				{
					int pingType = SM_ClanPingTargetType.ITEM;
					ctx.Read(pingType);
					HandleMarkItem(player, markPos, s1, pingType);
				}
				break;

			case SM_PartyRPC.MARK_CLEAR:
				HandleMarkClear(player);
				break;

			case SM_PartyRPC.HUD_ACTIVITY_STATUS:
				if (ctx.Read(hudActivityStatus))
					HandleHudActivityStatus(player, hudActivityStatus);
				break;

			case SM_PartyRPC.SET_DESCRIPTION:
				if (ctx.Read(s1))
					HandleSetDescription(player, s1);
				break;

			case SM_PartyRPC.REQUEST_CLAN_INFO:
				if (ctx.Read(s1))
					SendClanInfo(player, s1);
				break;

			case SM_PartyRPC.APPLY_TO_CLAN:
				if (ctx.Read(s1))
					HandleApply(player, s1);
				break;

			case SM_PartyRPC.APPLICATION_ACCEPT:
				if (ctx.Read(s1))
					HandleApplicationDecision(player, s1, true);
				break;

			case SM_PartyRPC.APPLICATION_DECLINE:
				if (ctx.Read(s1))
					HandleApplicationDecision(player, s1, false);
				break;

			case SM_PartyRPC.REQUEST_TOPS:
				SendTops(player);
				break;

			case SM_PartyRPC.TREASURY_DEPOSIT:
				int depositAmount;
				if (ctx.Read(depositAmount))
					HandleTreasuryDeposit(player, depositAmount);
				break;

			case SM_PartyRPC.TREASURY_WITHDRAW:
				int withdrawAmount;
				if (ctx.Read(withdrawAmount))
					HandleTreasuryWithdraw(player, withdrawAmount);
				break;

			case SM_PartyRPC.CLAN_UPGRADE:
				HandleClanUpgrade(player);
				break;

			case SM_PartyRPC.STORAGE_DEPOSIT:
				HandleStorageDeposit(player);
				break;

			case SM_PartyRPC.STORAGE_TAKE:
				int takeItemId;
				if (ctx.Read(takeItemId))
					HandleStorageTake(player, takeItemId);
				break;

			case SM_PartyRPC.REQUEST_STORAGE:
				SendStorage(player);
				break;

			case SM_PartyRPC.SET_PERMISSION:
				int permAction;
				int permRank;
				if (ctx.Read(permAction) && ctx.Read(permRank))
					HandleSetPermission(player, permAction, permRank);
				break;

			case SM_PartyRPC.SET_CLAN_WEBHOOK:
				if (ctx.Read(s1))
					HandleSetClanWebhook(player, s1);
				break;

			case SM_PartyRPC.CLEAR_CLAN_WEBHOOK:
				HandleClearClanWebhook(player);
				break;

			case SM_PartyRPC.TEST_CLAN_WEBHOOK:
				HandleTestClanWebhook(player);
				break;

			case SM_PartyRPC.TOGGLE_CLAN_AUDIT_WEBHOOK:
				HandleToggleClanAuditWebhook(player);
				break;

			case SM_PartyRPC.ADMIN_ENTER_CLAN:
				if (ctx.Read(s1))
					HandleAdminEnterClan(player, s1);
				break;

			case SM_PartyRPC.ADMIN_LEAVE_CLAN:
				HandleAdminLeaveClan(player);
				break;

			case SM_PartyRPC.ADMIN_RELOAD_CONFIG:
				HandleAdminReloadConfig(player);
				break;

			case SM_PartyRPC.ADMIN_CHAT_COMMAND:
				if (ctx.Read(s1))
					HandleAdminChatCommand(player, s1);
				break;

			case SM_PartyRPC.REQUEST_ADMIN_CHAT_LOG:
				SendAdminChatLog(player);
				break;

			case SM_PartyRPC.ADMIN_CHAT_MUTE:
				int muteMinutes;
				string muteReason;
				if (ctx.Read(s1) && ctx.Read(muteMinutes) && ctx.Read(muteReason))
					HandleAdminChatMute(player, s1, muteMinutes, muteReason);
				break;

			case SM_PartyRPC.ADMIN_CHAT_UNMUTE:
				if (ctx.Read(s1))
					HandleAdminChatUnmute(player, s1);
				break;

			case SM_PartyRPC.REQUEST_PLAYER_TITLES:
				SendPlayerTitles(player);
				break;

			case SM_PartyRPC.MARKET_LIST:
				SendMarket(player);
				break;

			case SM_PartyRPC.MARKET_SELL:
				int sellPrice;
				if (ctx.Read(sellPrice))
					HandleMarketSell(player, sellPrice);
				break;

			case SM_PartyRPC.MARKET_BUY:
				int buyLotId;
				if (ctx.Read(buyLotId))
					HandleMarketBuy(player, buyLotId);
				break;

			case SM_PartyRPC.MARKET_CANCEL:
				int cancelLotId;
				if (ctx.Read(cancelLotId))
					HandleMarketCancel(player, cancelLotId);
				break;

			case SM_PartyRPC.SERVER_MARKET_LIST:
				SendServerMarket(player);
				break;

			case SM_PartyRPC.SERVER_MARKET_BUY:
				int serverMarketItemId;
				if (ctx.Read(serverMarketItemId))
					HandleServerMarketBuy(player, serverMarketItemId);
				break;

			case SM_PartyRPC.AUCTION_LIST:
				SendAuction(player);
				break;

			case SM_PartyRPC.AUCTION_SELL:
				int auctionStartPrice;
				int auctionBidStep;
				int auctionDurationMinutes;
				if (ctx.Read(auctionStartPrice) && ctx.Read(auctionBidStep) && ctx.Read(auctionDurationMinutes))
					HandleAuctionSell(player, auctionStartPrice, auctionBidStep, auctionDurationMinutes);
				break;

			case SM_PartyRPC.AUCTION_BID:
				int auctionBidLotId;
				int auctionBidAmount;
				if (ctx.Read(auctionBidLotId) && ctx.Read(auctionBidAmount))
					HandleAuctionBid(player, auctionBidLotId, auctionBidAmount);
				break;

			case SM_PartyRPC.AUCTION_CANCEL:
				int auctionCancelLotId;
				if (ctx.Read(auctionCancelLotId))
					HandleAuctionCancel(player, auctionCancelLotId);
				break;

			case SM_PartyRPC.ACHIEVEMENTS:
				int achievementAction;
				int achievementScope;
				if (ctx.Read(achievementAction))
				{
					if (achievementAction == SM_AchievementRPCAction.REQUEST)
					{
						SendAchievements(player);
					}
					else if (achievementAction == SM_AchievementRPCAction.CLAIM)
					{
						string achievementId;
						if (ctx.Read(achievementScope) && ctx.Read(achievementId))
							HandleAchievementClaim(player, achievementScope, achievementId);
					}
				}
				break;

			case SM_PartyRPC.CONTRACTS:
				int contractAction;
				if (ctx.Read(contractAction))
				{
					if (contractAction == SM_ContractRPCAction.REQUEST)
					{
						SendContracts(player);
					}
					else if (contractAction == SM_ContractRPCAction.CREATE_KILL)
					{
						string contractTargetUid;
						int contractPrice;
						int contractDuration;
						if (ctx.Read(contractTargetUid) && ctx.Read(contractPrice) && ctx.Read(contractDuration))
							HandleContractCreateKill(player, contractTargetUid, contractPrice, contractDuration);
					}
					else if (contractAction == SM_ContractRPCAction.CREATE_ITEM)
					{
						string contractItemClass;
						int contractItemQuantity;
						int contractItemPrice;
						int contractItemDuration;
						if (ctx.Read(contractItemClass) && ctx.Read(contractItemQuantity) && ctx.Read(contractItemPrice) && ctx.Read(contractItemDuration))
							HandleContractCreateItem(player, contractItemClass, contractItemQuantity, contractItemPrice, contractItemDuration);
					}
					else if (contractAction == SM_ContractRPCAction.ACCEPT)
					{
						int acceptContractId;
						if (ctx.Read(acceptContractId))
							HandleContractAccept(player, acceptContractId);
					}
					else if (contractAction == SM_ContractRPCAction.ABANDON)
					{
						int abandonContractId;
						if (ctx.Read(abandonContractId))
							HandleContractAbandon(player, abandonContractId);
					}
					else if (contractAction == SM_ContractRPCAction.TURN_IN_ITEM)
					{
						int turnInContractId;
						if (ctx.Read(turnInContractId))
							HandleContractTurnInItem(player, turnInContractId);
					}
					else if (contractAction == SM_ContractRPCAction.REQUEST_TARGET_PREVIEW)
					{
						string previewTargetUid;
						if (ctx.Read(previewTargetUid))
							SendContractTargetPreview(player, previewTargetUid);
					}
				}
				break;

			case SM_PartyRPC.MAP_MARKER_ADD:
				int markerColor;
				int markerIcon;
				if (ctx.Read(markPos) && ctx.Read(markerName) && ctx.Read(markerColor) && ctx.Read(markerIcon))
					HandleMapMarkerAdd(player, markPos, markerName, markerColor, markerIcon);
				break;

			case SM_PartyRPC.MAP_MARKER_REMOVE:
				if (ctx.Read(markerId))
					HandleMapMarkerRemove(player, markerId);
				break;

			case SM_PartyRPC.MAP_MARKER_UPDATE:
				int updateMarkerColor;
				int updateMarkerIcon;
				if (ctx.Read(markerId) && ctx.Read(markPos) && ctx.Read(markerName) && ctx.Read(updateMarkerColor) && ctx.Read(updateMarkerIcon))
					HandleMapMarkerUpdate(player, markerId, markPos, markerName, updateMarkerColor, updateMarkerIcon);
				break;
		}
	}


	protected void HandleCreateClan(PlayerBase player, string name, string tag, string description)
	{
		string uid = player.GetIdentity().GetPlainId();

		if (FindClanByMember(uid))
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00352");
			return;
		}

		name = name.Trim();
		tag = tag.Trim();
		description = description.Trim();

		int nameLength = name.LengthUtf8();
		if (nameLength < m_Config.General.MinClanNameLength || nameLength > m_Config.General.MaxClanNameLength)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00669" + m_Config.General.MinClanNameLength + "#STR_SMP_00039" + m_Config.General.MaxClanNameLength + "#STR_SMP_00082");
			return;
		}

		if (tag.Length() > m_Config.General.MaxClanTagLength)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00986" + m_Config.General.MaxClanTagLength + "#STR_SMP_00082");
			return;
		}

		if (tag != "" && !SM_PartyUtil.IsLatinAlnum(tag))
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00987");
			return;
		}

		if (description.Length() > m_Config.General.MaxClanDescriptionLength)
			description = description.Substring(0, m_Config.General.MaxClanDescriptionLength);

		if (FindClan(name))
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00540");
			return;
		}

		if (m_Config.General.MaxClans > 0 && m_DB.Clans.Count() >= m_Config.General.MaxClans)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00419");
			return;
		}

		SM_Clan clan = new SM_Clan();
		clan.Id = MakeClanId(name);
		clan.Name = name;
		clan.Tag = tag;
		clan.Description = description;
		clan.Color = 0;
		m_Config.GetDefaultPerms(clan.Perms);
		clan.Members.Insert(new SM_ClanMember(uid, player.GetIdentity().GetName(), m_Config.GetLeaderRank(), MakeDateStamp()));
		AddLog(clan, "#STR_SMP_00549" + player.GetIdentity().GetName());
		m_DB.Clans.Insert(clan);
		SaveDB();
		AddPersonalAchievementValue(uid, player.GetIdentity().GetName(), "CreatedClan", 1, player);
		AddPersonalAchievementValue(uid, player.GetIdentity().GetName(), "JoinedClan", 1, player);
		CheckClanAchievementNotifications(clan);

		Notify(player, "#STR_SMP_00505", "#STR_SMP_00507" + name + "#STR_SMP_00137");
		SyncStateTo(player);

		Print(SM_PartyLoc.Text("#STR_SMP_00218" + name + "#STR_SMP_00132" + uid));
	}

	protected void HandleInvite(PlayerBase player, string targetUid)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.INVITE, "#STR_SMP_00505", "#STR_SMP_00684"))
			return;

		if (clan.Members.Count() >= m_Config.General.MaxClanMembers)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00520" + m_Config.General.MaxClanMembers + ")");
			return;
		}

		if (FindClanByMember(targetUid))
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00468");
			return;
		}

		PlayerBase target = FindOnlinePlayer(targetUid);
		if (!target)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00465");
			return;
		}

		array<ref SM_Invite> invites;
		if (!m_Invites.Find(targetUid, invites))
		{
			invites = new array<ref SM_Invite>;
			m_Invites.Set(targetUid, invites);
		}

		foreach (SM_Invite existing : invites)
		{
			if (existing.ClanName == clan.Name)
			{
				existing.SecondsLeft = m_Config.General.InviteTimeoutSeconds;
				Notify(player, "#STR_SMP_00505", "#STR_SMP_00840");
				return;
			}
		}

		string actorName = GetActorName(player, clan);
		invites.Insert(new SM_Invite(clan.Name, actorName, m_Config.General.InviteTimeoutSeconds));

		Notify(player, "#STR_SMP_00505", "#STR_SMP_00839" + target.GetIdentity().GetName());
		Notify(target, "#STR_SMP_00838", actorName + "#STR_SMP_00077" + clan.Name + "#STR_SMP_00138");
		SyncStateTo(target);
	}

	protected void HandleAcceptInvite(PlayerBase player, string clanName)
	{
		string uid = player.GetIdentity().GetPlainId();

		array<ref SM_Invite> invites;
		if (!m_Invites.Find(uid, invites))
			return;

		SM_Invite found = null;
		foreach (SM_Invite invite : invites)
		{
			if (invite.ClanName == clanName)
			{
				found = invite;
				break;
			}
		}
		if (!found)
			return;

		if (FindClanByMember(uid))
		{
			m_Invites.Remove(uid);
			SyncStateTo(player);
			return;
		}

		SM_Clan clan = FindClan(clanName);
		if (!clan)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00508");
			invites.RemoveItem(found);
			SyncStateTo(player);
			return;
		}

		if (clan.Members.Count() >= m_Config.General.MaxClanMembers)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00560");
			invites.RemoveItem(found);
			SyncStateTo(player);
			return;
		}

		clan.Members.Insert(new SM_ClanMember(uid, player.GetIdentity().GetName(), 0, MakeDateStamp()));
		m_Invites.Remove(uid); // вступил - остальные приглашения сгорают
		RemoveApplicationsEverywhere(uid);
		AddLog(clan, player.GetIdentity().GetName() + "#STR_SMP_00033");
		SendBaseMemberChangeAudit(clan, "#STR_SMP_00476", "#STR_SMP_00464" + found.InviterName, player, uid, player.GetIdentity().GetName());
		SaveDB();
		AddPersonalAchievementValue(uid, player.GetIdentity().GetName(), "JoinedClan", 1, player);
		CheckClanAchievementNotifications(clan);

		Notify(player, "#STR_SMP_00505", "#STR_SMP_00345" + clan.Name + "\"");
		NotifyClan(clan, "#STR_SMP_00505", player.GetIdentity().GetName() + "#STR_SMP_00033", uid);
		SyncClanState(clan);
	}

	protected void HandleDeclineInvite(PlayerBase player, string clanName)
	{
		string uid = player.GetIdentity().GetPlainId();

		array<ref SM_Invite> invites;
		if (!m_Invites.Find(uid, invites))
			return;

		for (int i = invites.Count() - 1; i >= 0; i--)
		{
			if (invites[i].ClanName == clanName)
				invites.Remove(i);
		}

		SyncStateTo(player);
	}

	protected void HandleKick(PlayerBase player, string targetUid)
	{
		string uid = player.GetIdentity().GetPlainId();
		if (uid == targetUid)
			return;

		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		SM_ClanMember target = clan.FindMember(targetUid);
		if (!target)
			return;

		int actorRank = GetActorRank(uid, clan);
		bool adminMode = IsAdminModeForClan(uid, clan);
		if (actorRank < clan.GetMinRank(SM_ClanAction.KICK) || (!adminMode && actorRank <= target.Rank))
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00687");
			return;
		}

		string actorName = GetActorName(player, clan);
		string targetName = target.Name;
		clan.Members.RemoveItem(target);
		AddLog(clan, targetName + "#STR_SMP_00049" + actorName + ")");
		SendBaseMemberChangeAudit(clan, "#STR_SMP_00476", targetName + "#STR_SMP_00051" + actorName, player, targetUid, targetName);
		SaveDB();

		Notify(player, "#STR_SMP_00505", targetName + "#STR_SMP_00050");
		NotifyClan(clan, "#STR_SMP_00505", targetName + "#STR_SMP_00050", uid);

		PlayerBase kicked = FindOnlinePlayer(targetUid);
		if (kicked)
		{
			Notify(kicked, "#STR_SMP_00505", "#STR_SMP_00300" + clan.Name + "\"");
			SyncStateTo(kicked);
		}
		SyncClanState(clan);
	}

	protected void HandleRankChange(PlayerBase player, string targetUid, int delta)
	{
		string uid = player.GetIdentity().GetPlainId();
		if (uid == targetUid)
			return;

		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		int leaderRank = m_Config.GetLeaderRank();
		bool adminMode = IsAdminModeForClan(uid, clan);

		if (GetActorRank(uid, clan) != leaderRank)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00998");
			return;
		}

		SM_ClanMember target = clan.FindMember(targetUid);
		if (!target)
			return;

		int newRank = target.Rank + delta;
		int maxRank = leaderRank - 1;
		if (adminMode)
			maxRank = leaderRank;

		if (newRank < 0 || newRank > maxRank)
			return;

		if (adminMode && newRank == leaderRank)
		{
			int demotedRank = leaderRank - 1;
			if (demotedRank < 0)
				demotedRank = 0;

			foreach (SM_ClanMember member : clan.Members)
			{
				if (member != target && member.Rank == leaderRank)
					member.Rank = demotedRank;
			}
		}

		string oldRankName = m_Config.GetRankName(target.Rank);
		target.Rank = newRank;
		string rankName = m_Config.GetRankName(newRank);
		AddLog(clan, target.Name + "#STR_SMP_00009" + rankName + " (" + GetActorName(player, clan) + ")");
		SendBaseRankChangeAudit(clan, "#STR_SMP_00475", "#STR_SMP_00883" + oldRankName + " -> " + rankName, player, targetUid, target.Name);
		SaveDB();

		Notify(player, "#STR_SMP_00505", target.Name + "#STR_SMP_00087" + rankName);

		PlayerBase targetPlayer = FindOnlinePlayer(targetUid);
		if (targetPlayer)
			Notify(targetPlayer, "#STR_SMP_00505", "#STR_SMP_00306" + rankName);

		SyncClanState(clan);
	}

	protected void HandleLeave(PlayerBase player)
	{
		string uid = player.GetIdentity().GetPlainId();
		if (GetAdminViewedClan(uid))
		{
			HandleAdminLeaveClan(player);
			return;
		}

		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
			return;

		SM_ClanMember member = clan.FindMember(uid);
		bool wasLeader = (member.Rank == m_Config.GetLeaderRank());
		string memberName = member.Name;
		string leftClanName = clan.Name;

		clan.Members.RemoveItem(member);

		if (clan.Members.Count() == 0)
		{
			ReturnClanLots(leftClanName);
			ReturnClanStorage(leftClanName);
			SendBaseMemberChangeAudit(clan, "#STR_SMP_00536", memberName + "#STR_SMP_00072", player, uid, memberName);

			m_DB.Clans.RemoveItem(clan);
			SaveDB();
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00351" + leftClanName + "#STR_SMP_00135");
			SyncStateTo(player);
			return;
		}

		if (wasLeader)
		{
			SM_ClanMember best = clan.Members[0];
			foreach (SM_ClanMember candidate : clan.Members)
			{
				if (candidate.Rank > best.Rank)
					best = candidate;
			}
			best.Rank = m_Config.GetLeaderRank();
			AddLog(clan, "#STR_SMP_00600" + best.Name);
			SendBaseRankChangeAudit(clan, "#STR_SMP_00937", "#STR_SMP_00600" + best.Name, player, best.Uid, best.Name);
			NotifyClan(clan, "#STR_SMP_00505", "#STR_SMP_00712" + best.Name);
		}

		AddLog(clan, memberName + "#STR_SMP_00071");
		SendBaseMemberChangeAudit(clan, "#STR_SMP_00476", memberName + "#STR_SMP_00071", player, uid, memberName);
		SaveDB();
		Notify(player, "#STR_SMP_00505", "#STR_SMP_00350" + clan.Name + "\"");
		NotifyClan(clan, "#STR_SMP_00505", memberName + "#STR_SMP_00071");
		SyncStateTo(player);
		SyncClanState(clan);
	}

	protected void HandleDisband(PlayerBase player)
	{
		string uid = player.GetIdentity().GetPlainId();
		if (GetAdminViewedClan(uid))
		{
			Notify(player, "#STR_SMP_00240", "#STR_SMP_00947");
			return;
		}

		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
			return;

		SM_ClanMember actor = clan.FindMember(uid);
		if (actor.Rank != m_Config.GetLeaderRank())
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00999");
			return;
		}

		NotifyClan(clan, "#STR_SMP_00505", "#STR_SMP_00507" + clan.Name + "#STR_SMP_00136");
		SendBaseMemberChangeAudit(clan, "#STR_SMP_00536", "#STR_SMP_00539", player, uid, player.GetIdentity().GetName());

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		array<string> memberUids = new array<string>;
		foreach (SM_ClanMember member : clan.Members)
			memberUids.Insert(member.Uid);

		ReturnClanLots(clan.Name);
		ReturnClanStorage(clan.Name);

		m_DB.Clans.RemoveItem(clan);
		SaveDB();

		foreach (string memberUid : memberUids)
		{
			PlayerBase pb;
			if (onlineMap.Find(memberUid, pb))
				SyncStateTo(pb);
		}

		Print(SM_PartyLoc.Text("#STR_SMP_00174" + uid));
	}

	protected void HandleSetColor(PlayerBase player, int colorIndex)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.COLOR, "#STR_SMP_00505", "#STR_SMP_00689"))
			return;

		if (colorIndex < 0 || colorIndex >= SM_ClanColors.Count())
			colorIndex = 0;

		if (clan.Color == colorIndex)
			return;

		clan.Color = colorIndex;
		AddLog(clan, GetActorName(player, clan) + "#STR_SMP_00084");
		SaveDB();
		SyncClanState(clan);
	}

	protected RestApi m_RestApi;
	protected ref SM_RestCallback m_DiscordCb;

	protected void HandleAdminEnterClan(PlayerBase player, string clanName)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		if (!IsClanAdmin(uid))
		{
			Notify(player, "#STR_SMP_00240", "#STR_SMP_00303");
			return;
		}

		SM_Clan clan = FindClan(clanName);
		if (!clan)
		{
			Notify(player, "#STR_SMP_00240", "#STR_SMP_00521");
			return;
		}

		m_AdminClanViews.Set(uid, clan.Name);
		Notify(player, "#STR_SMP_00240", "#STR_SMP_00754" + clan.Name);
		SyncStateTo(player);
	}

	protected void HandleAdminLeaveClan(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		m_AdminClanViews.Remove(uid);
		Notify(player, "#STR_SMP_00240", "#STR_SMP_00248");
		SyncStateTo(player);
	}

	protected void RestartHudTick()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.HudTick);

		int interval = m_Config.MemberHud.UpdateIntervalSeconds;
		if (interval < 1)
			interval = 1;

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.HudTick, interval * 1000, true);
	}

	protected void CleanupAdminViewsAfterConfigReload()
	{
		for (int i = m_AdminClanViews.Count() - 1; i >= 0; i--)
		{
			string adminUid = m_AdminClanViews.GetKey(i);
			string clanName = m_AdminClanViews.GetElement(i);
			if (!IsClanAdmin(adminUid))
			{
				m_AdminClanViews.Remove(adminUid);
				continue;
			}
			if (!FindClan(clanName))
				m_AdminClanViews.Remove(adminUid);
		}
	}

	protected void PrepareClansAfterConfigReload()
	{
		if (!m_DB || !m_DB.Clans)
			return;

		bool changed = false;
		int leaderRank = m_Config.GetLeaderRank();
		int maxLevel = m_Config.GetMaxLevel();
		array<int> defaultPerms;
		m_Config.GetDefaultPerms(defaultPerms);

		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (!clan)
				continue;

			if (!clan.Members)
			{
				clan.Members = new array<ref SM_ClanMember>;
				changed = true;
			}
			if (!clan.Log)
			{
				clan.Log = new array<string>;
				changed = true;
			}
			if (!clan.Applications)
			{
				clan.Applications = new array<ref SM_ClanApplication>;
				changed = true;
			}
			if (!clan.MapMarkers)
			{
				clan.MapMarkers = new array<ref SM_ClanMapMarker>;
				changed = true;
			}
			if (!clan.Stats)
			{
				clan.Stats = new SM_ClanStats();
				changed = true;
			}
			if (!clan.ClaimedAchievements)
			{
				clan.ClaimedAchievements = new array<string>;
				changed = true;
			}
			if (!clan.NotifiedAchievements)
			{
				clan.NotifiedAchievements = new array<string>;
				changed = true;
			}
			if (clan.Treasury < 0)
			{
				clan.Treasury = 0;
				changed = true;
			}
			if (clan.Level < 1)
			{
				clan.Level = 1;
				changed = true;
			}
			if (clan.Level > maxLevel)
			{
				clan.Level = maxLevel;
				changed = true;
			}
			if (clan.DiscordWebhookUrl == "")
			{
				if (clan.DiscordWebhookEnabled || clan.DiscordAuditEnabled)
					changed = true;
				clan.DiscordWebhookEnabled = false;
				clan.DiscordAuditEnabled = false;
			}
			if (!clan.DiscordWebhookEnabled && clan.DiscordAuditEnabled)
			{
				clan.DiscordAuditEnabled = false;
				changed = true;
			}

			foreach (SM_ClanMember member : clan.Members)
			{
				if (!member)
					continue;
				if (member.Rank > leaderRank)
				{
					member.Rank = leaderRank;
					changed = true;
				}
				if (member.Rank < 0)
				{
					member.Rank = 0;
					changed = true;
				}
			}

			if (!clan.Perms)
			{
				clan.Perms = new array<int>;
				changed = true;
			}
			while (clan.Perms.Count() < SM_ClanAction.COUNT)
			{
				clan.Perms.Insert(defaultPerms[clan.Perms.Count()]);
				changed = true;
			}
			for (int p = 0; p < clan.Perms.Count(); p++)
			{
				if (clan.Perms[p] < 0)
				{
					clan.Perms.Set(p, 0);
					changed = true;
				}
				if (clan.Perms[p] > leaderRank)
				{
					clan.Perms.Set(p, leaderRank);
					changed = true;
				}
			}
		}

		if (changed)
			SaveDB();
	}

	protected void SyncOnlineAfterConfigReload()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;

			SyncStateTo(pb);
			SendClanList(pb);
			SendTops(pb);
			SendMarket(pb);
			SendServerMarket(pb);
			SendAuction(pb);
			SendAchievements(pb);
			SendStorage(pb);
		}
	}

	protected PlayerBase FindUniqueOnlinePlayerByName(string name)
	{
		PlayerBase found = null;
		int matches = 0;

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
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
		return null;
	}

	void OnServerChatMessage(string senderName, string text)
	{
		if (!SM_PartyAdminCommands.IsReloadConfigCommand(text))
			return;

		PlayerBase player = FindUniqueOnlinePlayerByName(senderName);
		if (!player)
		{
			Print(SM_PartyLoc.Text("#STR_SMP_00175" + senderName + ")"));
			return;
		}

		HandleAdminReloadConfig(player);
	}

	protected void HandleAdminReloadConfig(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		if (!IsClanAdmin(uid))
		{
			Notify(player, "#STR_SMP_00240", "#STR_SMP_00303");
			return;
		}

		float now = GetGame().GetTickTime();
		float lastReloadTime;
		if (m_LastAdminConfigReloads.Find(uid, lastReloadTime))
		{
			if (now - lastReloadTime < 1.0)
				return;
		}
		m_LastAdminConfigReloads.Set(uid, now);

		string reloadMessage;
		if (!SM_PartyConfigLoader.Reload(reloadMessage))
		{
			Notify(player, "#STR_SMP_00584", "#STR_SMP_00768");
			ErrorEx("[SM_PartyMod] " + reloadMessage);
			return;
		}

		m_Config = SM_PartyConfigLoader.Get();
		PrepareClansAfterConfigReload();
		CleanupAdminViewsAfterConfigReload();
		RestartHudTick();
		PrepareAchievementsDB();
		PrepareServerMarketState();
		PrepareContractsDB();
		RestartContractsTick();
		EnsureServerMarketReady(true);
		EnsureTopRewardSeasonStarted(true);
		CheckOnlineRestrictedItems();
		SyncOnlineAfterConfigReload();
		BroadcastContracts();

		Notify(player, "#STR_SMP_00584", "#STR_SMP_00231");
		Print(SM_PartyLoc.Text("#STR_SMP_00172" + player.GetIdentity().GetName() + " (" + uid + ")"));
	}

	protected bool IsValidDiscordWebhookUrl(string url)
	{
		if (url == "")
			return false;
		if (url.Length() > 300)
			return false;
		if (!url.Contains("/api/webhooks/"))
			return false;
		if (url.IndexOf("https://discord.com/") != 0 && url.IndexOf("https://discordapp.com/") != 0)
			return false;
		return true;
	}

	protected void HandleSetClanWebhook(PlayerBase player, string url)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.MANAGE_WEBHOOK, "Discord", "#STR_SMP_00683"))
			return;

		url = url.Trim();
		if (!IsValidDiscordWebhookUrl(url))
		{
			Notify(player, "Discord", "#STR_SMP_00713");
			return;
		}

		string actorName = GetActorName(player, clan);
		clan.DiscordWebhookUrl = url;
		clan.DiscordWebhookEnabled = true;
		AddLog(clan, actorName + "#STR_SMP_00062");
		SaveDB();

		Notify(player, "Discord", "#STR_SMP_00236");
		SyncClanState(clan);
	}

	protected void HandleClearClanWebhook(PlayerBase player)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.MANAGE_WEBHOOK, "Discord", "#STR_SMP_00683"))
			return;

		string actorName = GetActorName(player, clan);
		AddLog(clan, actorName + "#STR_SMP_00067");
		clan.DiscordWebhookUrl = "";
		clan.DiscordWebhookEnabled = false;
		clan.DiscordAuditEnabled = false;
		SaveDB();

		Notify(player, "Discord", "#STR_SMP_00234");
		SyncClanState(clan);
	}

	protected void HandleTestClanWebhook(PlayerBase player)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.MANAGE_WEBHOOK, "Discord", "#STR_SMP_00683"))
			return;

		if (!clan.DiscordWebhookEnabled || clan.DiscordWebhookUrl == "")
		{
			Notify(player, "Discord", "#STR_SMP_00233");
			return;
		}

		RelayToDiscordUrl(clan.DiscordWebhookUrl, "#STR_SMP_00991" + clan.Name, GetActorName(player, clan), "#STR_SMP_00235");
		Notify(player, "Discord", "#STR_SMP_00993");
	}

	protected void HandleToggleClanAuditWebhook(PlayerBase player)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.MANAGE_WEBHOOK, "Discord", "#STR_SMP_00683"))
			return;

		string actorName = GetActorName(player, clan);
		if (clan.DiscordAuditEnabled)
		{
			AddLog(clan, actorName + "#STR_SMP_00034");
			clan.DiscordAuditEnabled = false;
			Notify(player, "Discord", "#STR_SMP_00257");
		}
		else
		{
			if (!clan.DiscordWebhookEnabled || clan.DiscordWebhookUrl == "")
			{
				Notify(player, "Discord", "#STR_SMP_00949");
				return;
			}
			clan.DiscordAuditEnabled = true;
			AddLog(clan, actorName + "#STR_SMP_00030");
			Notify(player, "Discord", "#STR_SMP_00256");
		}

		SaveDB();
		SyncClanState(clan);
	}

	protected void HandleServerAdminChat(PlayerBase player, string uid, string text)
	{
		if (!IsClanAdmin(uid))
		{
			Notify(player, "#STR_SMP_00909", "#STR_SMP_00703");
			return;
		}

		string senderName = player.GetIdentity().GetName();
		AddAdminChatLog(uid, senderName, "#STR_SMP_00909", text);

		int channelColor = m_Config.ChatSystem.GetServerColor();
		int textColor = ARGB(255, 230, 230, 230);

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
			{
				SendChatHistory(pb, SM_ChatChannel.SERVER, "#STR_SMP_00909", "", "", text, channelColor, channelColor, textColor, channelColor);
			}
		}
	}

	protected void HandleChat(PlayerBase player, int channel, string text)
	{
		if (!player || !player.GetIdentity())
			return;
		if (!m_Config || !m_Config.ChatSystem || !m_Config.ChatSystem.Enabled)
			return;

		text = text.Trim();
		if (text == "")
			return;
		if (text.LengthUtf8() > m_Config.ChatSystem.MessageMaxLength)
			text = text.SubstringUtf8(0, m_Config.ChatSystem.MessageMaxLength);

		string uid = player.GetIdentity().GetPlainId();
		if (channel == SM_ChatChannel.SERVER)
		{
			HandleServerAdminChat(player, uid, text);
			return;
		}

		SM_ChatMuteRecord mute;
		if (IsPlayerChatMuted(uid, mute))
		{
			Notify(player, "#STR_SMP_01063", "#STR_SMP_01066" + FormatDurationSeconds(GetChatMuteRemainingSeconds(mute)));
			SyncChatMuteStatus(player);
			return;
		}

		string senderName = player.GetIdentity().GetName();
		AddAdminChatLog(uid, senderName, GetAdminChatChannelName(channel), text);

		switch (channel)
		{
			case SM_ChatChannel.LOCAL:
				HandleLocalChat(player, uid, senderName, text);
				break;
			case SM_ChatChannel.GLOBAL:
				HandleGlobalChat(player, uid, senderName, text);
				break;
			default:
				HandleClanChat(player, uid, text);
				break;
		}
	}

	protected string GetAdminChatChannelName(int channel)
	{
		if (channel == SM_ChatChannel.LOCAL)
			return "#STR_SMP_00894";
		if (channel == SM_ChatChannel.GLOBAL)
			return "#STR_SMP_00383";
		if (channel == SM_ChatChannel.SERVER)
			return "#STR_SMP_00909";
		return "#STR_SMP_00505";
	}

	protected void AddAdminChatLog(string uid, string name, string channel, string text)
	{
		if (!m_AdminChatLog)
			m_AdminChatLog = new array<ref SM_AdminChatLogEntry>;

		m_AdminChatLog.InsertAt(new SM_AdminChatLogEntry(uid, name, channel, text, GetChatStamp()), 0);

		while (m_AdminChatLog.Count() > 200)
		{
			m_AdminChatLog.Remove(m_AdminChatLog.Count() - 1);
		}

		SyncAdminChatLogToAdmins();
	}

	protected string FindAdminChatLogName(string uid)
	{
		if (!m_AdminChatLog)
			return "";

		for (int i = 0; i < m_AdminChatLog.Count(); i++)
		{
			SM_AdminChatLogEntry entry = m_AdminChatLog[i];
			if (entry && entry.Uid == uid)
				return entry.Name;
		}

		return "";
	}

	protected void SendAdminChatLog(PlayerBase admin)
	{
		if (!admin || !admin.GetIdentity())
			return;

		string adminUid = admin.GetIdentity().GetPlainId();
		if (!IsClanAdmin(adminUid))
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00703");
			return;
		}

		if (!m_AdminChatLog)
			m_AdminChatLog = new array<ref SM_AdminChatLogEntry>;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_AdminChatLog.Count());
		for (int i = 0; i < m_AdminChatLog.Count(); i++)
		{
			SM_AdminChatLogEntry entry = m_AdminChatLog[i];
			if (!entry)
				entry = new SM_AdminChatLogEntry();
			rpc.Write(entry.Uid);
			rpc.Write(entry.Name);
			rpc.Write(entry.Channel);
			rpc.Write(entry.Text);
			rpc.Write(entry.Stamp);
		}
		rpc.Send(admin, SM_PartyRPC.SYNC_ADMIN_CHAT_LOG, true, admin.GetIdentity());
	}

	protected void SyncAdminChatLogToAdmins()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase admin = PlayerBase.Cast(man);
			if (!admin || !admin.GetIdentity())
				continue;
			if (!IsClanAdmin(admin.GetIdentity().GetPlainId()))
				continue;

			SendAdminChatLog(admin);
		}
	}

	protected void HandleAdminChatMute(PlayerBase admin, string targetUid, int minutes, string reason)
	{
		if (!admin || !admin.GetIdentity())
			return;

		string adminUid = admin.GetIdentity().GetPlainId();
		if (!IsClanAdmin(adminUid))
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00705");
			return;
		}

		targetUid = targetUid.Trim();
		if (targetUid == "")
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00466");
			return;
		}

		if (minutes <= 0)
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00340");
			return;
		}
		if (minutes > 43200)
			minutes = 43200;

		if (IsClanAdmin(targetUid))
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00247");
			return;
		}

		PlayerBase targetPlayer = FindOnlinePlayer(targetUid);
		string targetName = FindKnownPlayerName(targetUid);
		if (targetName == "")
			targetName = FindAdminChatLogName(targetUid);
		if (targetPlayer && targetPlayer.GetIdentity())
			targetName = targetPlayer.GetIdentity().GetName();
		if (targetName == "")
			targetName = targetUid;

		reason = reason.Trim();
		if (reason == "")
			reason = "#STR_SMP_00243";

		int now = GetNowAbsSeconds();
		int until = now + minutes * 60;

		if (!m_ChatMutesDB)
			m_ChatMutesDB = new SM_ChatMutesDB();
		if (!m_ChatMutesDB.Mutes)
			m_ChatMutesDB.Mutes = new array<ref SM_ChatMuteRecord>;

		SM_ChatMuteRecord mute = FindChatMute(targetUid);
		if (!mute)
		{
			mute = new SM_ChatMuteRecord();
			m_ChatMutesDB.Mutes.Insert(mute);
		}

		mute.Uid = targetUid;
		mute.Name = targetName;
		mute.AdminUid = adminUid;
		mute.AdminName = admin.GetIdentity().GetName();
		mute.Reason = reason;
		mute.MutedAt = now;
		mute.Until = until;

		SaveChatMutesDB();
		Notify(admin, "#STR_SMP_01063", "#STR_SMP_00650" + targetName + "#STR_SMP_00061" + minutes.ToString() + "#STR_SMP_00058");

		if (targetPlayer)
		{
			SyncChatMuteStatus(targetPlayer);
			Notify(targetPlayer, "#STR_SMP_01063", "#STR_SMP_01065" + minutes.ToString() + "#STR_SMP_00060" + reason);
		}
	}

	protected void HandleAdminChatUnmute(PlayerBase admin, string targetUid)
	{
		if (!admin || !admin.GetIdentity())
			return;

		string adminUid = admin.GetIdentity().GetPlainId();
		if (!IsClanAdmin(adminUid))
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00706");
			return;
		}

		targetUid = targetUid.Trim();
		if (targetUid == "")
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00466");
			return;
		}

		PlayerBase targetPlayer = FindOnlinePlayer(targetUid);
		string targetName = FindKnownPlayerName(targetUid);
		if (targetName == "")
			targetName = FindAdminChatLogName(targetUid);
		if (targetPlayer && targetPlayer.GetIdentity())
			targetName = targetPlayer.GetIdentity().GetName();
		if (targetName == "")
			targetName = targetUid;

		SM_ChatMuteRecord mute = FindChatMute(targetUid);
		if (!mute)
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00251" + targetName);
			return;
		}

		RemoveChatMute(targetUid);
		SaveChatMutesDB();
		Notify(admin, "#STR_SMP_01063", "#STR_SMP_00651" + targetName);

		if (targetPlayer)
		{
			SyncChatMuteStatus(targetPlayer);
			Notify(targetPlayer, "#STR_SMP_01063", "#STR_SMP_00653");
		}
	}

	protected void HandleAdminChatCommand(PlayerBase admin, string commandText)
	{
		if (!admin || !admin.GetIdentity())
			return;

		string adminUid = admin.GetIdentity().GetPlainId();
		if (!IsClanAdmin(adminUid))
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00704");
			return;
		}

		array<string> tokens = TokenizeAdminChatCommand(commandText);
		if (!tokens || tokens.Count() == 0)
			return;

		string action = GetAdminChatCommandAction(tokens[0]);
		if (action == "list")
		{
			SendChatMuteList(admin);
			return;
		}

		if (action == "mute")
		{
			HandleAdminMuteCommand(admin, adminUid, tokens);
			return;
		}

		if (action == "unmute")
		{
			HandleAdminUnmuteCommand(admin, tokens);
			return;
		}

		SendChatMuteUsage(admin);
	}

	protected array<string> TokenizeAdminChatCommand(string commandText)
	{
		array<string> raw = new array<string>;
		array<string> tokens = new array<string>;

		string text = commandText;
		text = text.Trim();
		text.Split(" ", raw);
		for (int i = 0; i < raw.Count(); i++)
		{
			string token = raw[i];
			token = token.Trim();
			if (token != "")
				tokens.Insert(token);
		}

		return tokens;
	}

	protected string GetAdminChatCommandAction(string command)
	{
		string cmd = command;
		cmd = cmd.Trim();
		cmd.ToLower();

		if (cmd == "!mute" || cmd == "/mute" || cmd == "#mute" || cmd == "!chatmute" || cmd == "/chatmute" || cmd == "#chatmute")
			return "mute";
		if (cmd == "!unmute" || cmd == "/unmute" || cmd == "#unmute" || cmd == "!chatunmute" || cmd == "/chatunmute" || cmd == "#chatunmute")
			return "unmute";
		if (cmd == "!mutelist" || cmd == "/mutelist" || cmd == "#mutelist" || cmd == "!mutes" || cmd == "/mutes" || cmd == "#mutes")
			return "list";
		return "";
	}

	protected void SendChatMuteUsage(PlayerBase admin)
	{
		Notify(admin, "#STR_SMP_01063", "#STR_SMP_00130");
	}

	protected void HandleAdminMuteCommand(PlayerBase admin, string adminUid, array<string> tokens)
	{
		if (tokens.Count() < 3)
		{
			SendChatMuteUsage(admin);
			return;
		}

		string targetToken = tokens[1];
		int minutes = tokens[2].ToInt();
		if (minutes <= 0)
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00340");
			return;
		}
		if (minutes > 43200)
			minutes = 43200;

		string targetUid;
		string targetName;
		PlayerBase targetPlayer;
		string resolveError;
		if (!ResolveChatMuteTarget(targetToken, targetUid, targetName, targetPlayer, resolveError))
		{
			Notify(admin, "#STR_SMP_01063", resolveError);
			return;
		}

		if (IsClanAdmin(targetUid))
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00247");
			return;
		}

		string reason = BuildChatMuteReason(tokens, 3);
		int now = GetNowAbsSeconds();
		int until = now + minutes * 60;

		if (!m_ChatMutesDB)
			m_ChatMutesDB = new SM_ChatMutesDB();
		if (!m_ChatMutesDB.Mutes)
			m_ChatMutesDB.Mutes = new array<ref SM_ChatMuteRecord>;

		SM_ChatMuteRecord mute = FindChatMute(targetUid);
		if (!mute)
		{
			mute = new SM_ChatMuteRecord();
			m_ChatMutesDB.Mutes.Insert(mute);
		}

		mute.Uid = targetUid;
		mute.Name = targetName;
		mute.AdminUid = adminUid;
		mute.AdminName = admin.GetIdentity().GetName();
		mute.Reason = reason;
		mute.MutedAt = now;
		mute.Until = until;

		SaveChatMutesDB();
		Notify(admin, "#STR_SMP_01063", "#STR_SMP_00650" + targetName + "#STR_SMP_00061" + minutes.ToString() + "#STR_SMP_00058");

		if (targetPlayer)
		{
			SyncChatMuteStatus(targetPlayer);
			Notify(targetPlayer, "#STR_SMP_01063", "#STR_SMP_01065" + minutes.ToString() + "#STR_SMP_00060" + reason);
		}
	}

	protected void HandleAdminUnmuteCommand(PlayerBase admin, array<string> tokens)
	{
		if (tokens.Count() < 2)
		{
			SendChatMuteUsage(admin);
			return;
		}

		string targetUid;
		string targetName;
		PlayerBase targetPlayer;
		string resolveError;
		if (!ResolveChatMuteTarget(tokens[1], targetUid, targetName, targetPlayer, resolveError))
		{
			Notify(admin, "#STR_SMP_01063", resolveError);
			return;
		}

		SM_ChatMuteRecord mute = FindChatMute(targetUid);
		if (!mute)
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00251" + targetName);
			return;
		}

		RemoveChatMute(targetUid);
		SaveChatMutesDB();
		Notify(admin, "#STR_SMP_01063", "#STR_SMP_00651" + targetName);

		if (targetPlayer)
		{
			SyncChatMuteStatus(targetPlayer);
			Notify(targetPlayer, "#STR_SMP_01063", "#STR_SMP_00653");
		}
	}

	protected string BuildChatMuteReason(array<string> tokens, int startIndex)
	{
		string reason = "";
		for (int i = startIndex; i < tokens.Count(); i++)
		{
			if (reason != "")
				reason = reason + " ";
			reason = reason + tokens[i];
		}
		reason = reason.Trim();
		if (reason == "")
			reason = "#STR_SMP_00275";
		return reason;
	}

	protected void SendChatMuteList(PlayerBase admin)
	{
		CleanupExpiredChatMutes(true);
		if (!m_ChatMutesDB || !m_ChatMutesDB.Mutes || m_ChatMutesDB.Mutes.Count() == 0)
		{
			Notify(admin, "#STR_SMP_01063", "#STR_SMP_00252");
			return;
		}

		int now = GetNowAbsSeconds();
		string text = "";
		int shown = 0;
		for (int i = 0; i < m_ChatMutesDB.Mutes.Count(); i++)
		{
			SM_ChatMuteRecord mute = m_ChatMutesDB.Mutes[i];
			if (!mute)
				continue;
			if (shown >= 6)
				break;
			int left = mute.Until - now;
			if (left < 0)
				left = 0;
			if (text != "")
				text = text + "\n";
			text = text + mute.Name + " - " + FormatDurationSeconds(left);
			shown++;
		}

		int hidden = m_ChatMutesDB.Mutes.Count() - shown;
		if (hidden > 0)
			text = text + "#STR_SMP_00101" + hidden.ToString();

		Notify(admin, "#STR_SMP_00654", text);
	}

	protected bool IsPlayerChatMuted(string uid, out SM_ChatMuteRecord mute)
	{
		mute = FindChatMute(uid);
		if (!mute)
			return false;
		if (mute.Until <= GetNowAbsSeconds())
		{
			RemoveChatMute(uid);
			SaveChatMutesDB();
			return false;
		}
		return true;
	}

	protected SM_ChatMuteRecord FindChatMute(string uid)
	{
		if (!m_ChatMutesDB || !m_ChatMutesDB.Mutes)
			return null;
		for (int i = 0; i < m_ChatMutesDB.Mutes.Count(); i++)
		{
			SM_ChatMuteRecord mute = m_ChatMutesDB.Mutes[i];
			if (mute && mute.Uid == uid)
				return mute;
		}
		return null;
	}

	protected void RemoveChatMute(string uid)
	{
		if (!m_ChatMutesDB || !m_ChatMutesDB.Mutes)
			return;
		for (int i = m_ChatMutesDB.Mutes.Count() - 1; i >= 0; i--)
		{
			SM_ChatMuteRecord mute = m_ChatMutesDB.Mutes[i];
			if (mute && mute.Uid == uid)
				m_ChatMutesDB.Mutes.Remove(i);
		}
	}

	protected int GetChatMuteRemainingSeconds(SM_ChatMuteRecord mute)
	{
		if (!mute)
			return 0;
		int left = mute.Until - GetNowAbsSeconds();
		if (left < 0)
			left = 0;
		return left;
	}

	protected string FormatDurationSeconds(int seconds)
	{
		if (seconds <= 0)
			return "#STR_SMP_00223";

		int days = seconds / 86400;
		int hours = (seconds % 86400) / 3600;
		int minutes = (seconds % 3600) / 60;
		int secs = seconds % 60;

		string result = "";
		if (days > 0)
			result = days.ToString() + "#STR_SMP_00037";
		if (hours > 0)
		{
			if (result != "")
				result = result + " ";
			result = result + hours.ToString() + "#STR_SMP_00092";
		}
		if (minutes > 0)
		{
			if (result != "")
				result = result + " ";
			result = result + minutes.ToString() + "#STR_SMP_00058";
		}
		if (result == "")
			result = secs.ToString() + "#STR_SMP_00081";

		return result;
	}

	protected void SyncChatMuteStatus(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		SM_ChatMuteRecord mute;
		bool muted = IsPlayerChatMuted(player.GetIdentity().GetPlainId(), mute);
		int remainingSeconds = 0;
		string reason = "";
		if (muted && mute)
		{
			remainingSeconds = GetChatMuteRemainingSeconds(mute);
			reason = mute.Reason;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(muted);
		rpc.Write(remainingSeconds);
		rpc.Write(reason);
		rpc.Send(player, SM_PartyRPC.SYNC_CHAT_MUTE, true, player.GetIdentity());
	}

	protected bool ResolveChatMuteTarget(string token, out string uid, out string name, out PlayerBase onlinePlayer, out string error)
	{
		uid = "";
		name = "";
		onlinePlayer = null;
		error = "";

		string search = token;
		search = search.Trim();
		if (search == "")
		{
			error = "#STR_SMP_01038";
			return false;
		}

		CleanupExpiredChatMutes(true);

		SM_ChatMuteRecord mutedRecord = FindChatMuteByToken(search);
		if (mutedRecord)
		{
			uid = mutedRecord.Uid;
			name = mutedRecord.Name;
			onlinePlayer = FindOnlinePlayer(uid);
			if (onlinePlayer && onlinePlayer.GetIdentity())
				name = onlinePlayer.GetIdentity().GetName();
			return true;
		}

		if (IsDigitsOnly(search))
		{
			uid = search;
			onlinePlayer = FindOnlinePlayer(uid);
			name = FindKnownPlayerName(uid);
			if (onlinePlayer && onlinePlayer.GetIdentity())
				name = onlinePlayer.GetIdentity().GetName();
			return true;
		}

		if (FindOnlinePlayerByNameToken(search, uid, name, onlinePlayer, error))
			return true;
		if (error != "")
			return false;

		if (FindKnownClanMemberByNameToken(search, uid, name, error))
		{
			onlinePlayer = FindOnlinePlayer(uid);
			if (onlinePlayer && onlinePlayer.GetIdentity())
				name = onlinePlayer.GetIdentity().GetName();
			return true;
		}
		if (error != "")
			return false;

		error = "#STR_SMP_00467";
		return false;
	}

	protected SM_ChatMuteRecord FindChatMuteByToken(string token)
	{
		if (!m_ChatMutesDB || !m_ChatMutesDB.Mutes)
			return null;

		string search = token;
		search = search.Trim();
		string searchLower = search;
		searchLower.ToLower();

		SM_ChatMuteRecord found = null;
		int matches = 0;
		for (int i = 0; i < m_ChatMutesDB.Mutes.Count(); i++)
		{
			SM_ChatMuteRecord mute = m_ChatMutesDB.Mutes[i];
			if (!mute)
				continue;
			string muteName = mute.Name;
			muteName.ToLower();
			if (mute.Uid == search || muteName == searchLower)
			{
				found = mute;
				matches = 1;
				break;
			}
			if (searchLower.Length() >= 3 && muteName.Contains(searchLower))
			{
				found = mute;
				matches++;
			}
		}

		if (matches == 1)
			return found;
		return null;
	}

	protected bool FindOnlinePlayerByNameToken(string token, out string uid, out string name, out PlayerBase onlinePlayer, out string error)
	{
		uid = "";
		name = "";
		onlinePlayer = null;
		error = "";

		string search = token;
		search = search.Trim();
		string searchLower = search;
		searchLower.ToLower();

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		PlayerBase found = null;
		int matches = 0;
		for (int i = 0; i < players.Count(); i++)
		{
			PlayerBase pb = PlayerBase.Cast(players[i]);
			if (!pb || !pb.GetIdentity())
				continue;
			string playerName = pb.GetIdentity().GetName();
			string lowerName = playerName;
			lowerName.ToLower();
			if (lowerName == searchLower)
			{
				found = pb;
				matches = 1;
				break;
			}
			if (searchLower.Length() >= 3 && lowerName.Contains(searchLower))
			{
				found = pb;
				matches++;
			}
		}

		if (matches > 1)
		{
			error = "#STR_SMP_00671";
			return false;
		}
		if (matches == 1 && found && found.GetIdentity())
		{
			onlinePlayer = found;
			uid = found.GetIdentity().GetPlainId();
			name = found.GetIdentity().GetName();
			return true;
		}
		return false;
	}

	protected bool FindKnownClanMemberByNameToken(string token, out string uid, out string name, out string error)
	{
		uid = "";
		name = "";
		error = "";

		string search = token;
		search = search.Trim();
		string searchLower = search;
		searchLower.ToLower();

		int matches = 0;
		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (!clan || !clan.Members)
				continue;
			foreach (SM_ClanMember member : clan.Members)
			{
				if (!member)
					continue;
				string memberName = member.Name;
				memberName.ToLower();
				if (memberName == searchLower)
				{
					uid = member.Uid;
					name = member.Name;
					matches = 1;
					return true;
				}
				if (searchLower.Length() >= 3 && memberName.Contains(searchLower))
				{
					uid = member.Uid;
					name = member.Name;
					matches++;
				}
			}
		}

		if (matches > 1)
		{
			error = "#STR_SMP_00670";
			return false;
		}
		if (matches == 1)
			return true;
		return false;
	}

	protected string FindKnownPlayerName(string uid)
	{
		PlayerBase onlinePlayer = FindOnlinePlayer(uid);
		if (onlinePlayer && onlinePlayer.GetIdentity())
			return onlinePlayer.GetIdentity().GetName();

		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (!clan || !clan.Members)
				continue;
			foreach (SM_ClanMember member : clan.Members)
			{
				if (member && member.Uid == uid && member.Name != "")
					return member.Name;
			}
		}

		SM_ChatMuteRecord mute = FindChatMute(uid);
		if (mute && mute.Name != "")
			return mute.Name;

		return uid;
	}

	protected bool IsDigitsOnly(string value)
	{
		if (value == "")
			return false;
		string digits = "0123456789";
		for (int i = 0; i < value.Length(); i++)
		{
			if (!digits.Contains(value.Get(i)))
				return false;
		}
		return true;
	}

	protected void GetChatStyle(int channel, string uid, out string prefix, out int nameColor, out int textColor, out int prefixColor)
	{
		prefix = "";
		int fallbackColor = SM_ChatChannelPalette.GetTextColor(channel);
		array<ref SM_ChatPlayerStyle> styles = m_Config.ChatSystem.Global.PlayerStyles;

		if (channel == SM_ChatChannel.CLAN)
		{
			nameColor = m_Config.ChatSystem.GetClanPlayerColor();
			textColor = m_Config.ChatSystem.GetClanColor();
			styles = m_Config.ChatSystem.Clan.PlayerStyles;
		}
		else if (channel == SM_ChatChannel.LOCAL)
		{
			nameColor = m_Config.ChatSystem.GetDirectPlayerColor();
			textColor = m_Config.ChatSystem.GetDirectColor();
			styles = m_Config.ChatSystem.Local.PlayerStyles;
		}
		else
		{
			nameColor = m_Config.ChatSystem.GetGlobalPlayerColor();
			textColor = m_Config.ChatSystem.GetGlobalColor();
		}

		prefixColor = nameColor;   // по умолчанию тег цветом ника

		if (!styles)
			return;

		foreach (SM_ChatPlayerStyle style : styles)
		{
			if (!style)
				continue;
			if (style.SteamId != uid)
				continue;

			prefix = style.Prefix;
			nameColor = style.GetNameColor(nameColor);
			textColor = style.GetTextColor(textColor);
			prefixColor = nameColor;   // следует за (возможно переопределённым) цветом ника
			prefixColor = style.GetPrefixColor(prefixColor);
			return;
		}
	}

	protected string BuildChatDisplayName(string prefix, string senderName)
	{
		if (prefix != "")
			return prefix + " " + senderName;
		return senderName;
	}

	protected void HandleClanChat(PlayerBase player, string uid, string text)
	{
		if (!m_Config.ChatSystem.Enabled)
			return;

		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		string senderName = GetActorName(player, clan);
		string prefix;
		int nameColor;
		int textColor;
		int prefixColor;
		GetChatStyle(SM_ChatChannel.CLAN, uid, prefix, nameColor, textColor, prefixColor);

		string displayName = BuildChatDisplayName(prefix, senderName);
		int channelColor = SM_ClanColors.ResolveColor(clan.Color);

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_ClanMember member : clan.Members)
		{
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
			{
				SendChatHistory(pb, SM_ChatChannel.CLAN, "#STR_SMP_00505", prefix, senderName, text, channelColor, nameColor, textColor, prefixColor);
			}
		}

		if (clan.DiscordWebhookEnabled && clan.DiscordWebhookUrl != "")
		{
			RelayToDiscordUrl(clan.DiscordWebhookUrl, "#STR_SMP_00506" + clan.Name, displayName, text);
		}
		else if (m_Config.Discord.RelayClan)
		{
			RelayToDiscordUrl(GetDiscordWebhookForChannel(SM_ChatChannel.CLAN), "#STR_SMP_00506" + clan.Name, displayName, text);
		}
	}

	protected void HandleLocalChat(PlayerBase player, string uid, string senderName, string text)
	{
		if (!m_Config.ChatSystem.LocalEnabled)
			return;

		string prefix;
		int nameColor;
		int textColor;
		int prefixColor;
		GetChatStyle(SM_ChatChannel.LOCAL, uid, prefix, nameColor, textColor, prefixColor);

		string displayName = BuildChatDisplayName(prefix, senderName);
		int channelColor = m_Config.ChatSystem.GetDirectColor();
		vector myPos = player.GetPosition();
		float range = m_Config.ChatSystem.LocalRangeMeters;

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;
			if (vector.Distance(pb.GetPosition(), myPos) <= range)
			{
				SendChatHistory(pb, SM_ChatChannel.LOCAL, "#STR_SMP_00894", prefix, senderName, text, channelColor, nameColor, textColor, prefixColor);
			}
		}

		if (m_Config.Discord.RelayLocal)
			RelayToDiscordUrl(GetDiscordWebhookForChannel(SM_ChatChannel.LOCAL), "#STR_SMP_00894", displayName, text);
	}

	protected void HandleGlobalChat(PlayerBase player, string uid, string senderName, string text)
	{
		if (!m_Config.ChatSystem.GlobalEnabled)
			return;

		string prefix;
		int nameColor;
		int textColor;
		int prefixColor;
		GetChatStyle(SM_ChatChannel.GLOBAL, uid, prefix, nameColor, textColor, prefixColor);

		string displayName = BuildChatDisplayName(prefix, senderName);
		int channelColor = m_Config.ChatSystem.GetGlobalColor();

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
			{
				SendChatHistory(pb, SM_ChatChannel.GLOBAL, "#STR_SMP_00383", prefix, senderName, text, channelColor, nameColor, textColor, prefixColor);
			}
		}

		if (m_Config.Discord.RelayGlobal)
			RelayToDiscordUrl(GetDiscordWebhookForChannel(SM_ChatChannel.GLOBAL), "#STR_SMP_00383", displayName, text);
	}

	protected string GetDiscordWebhookForChannel(int channel)
	{
		string url = "";
		if (channel == SM_ChatChannel.CLAN)
			url = m_Config.Discord.ClanWebhookUrl;
		else if (channel == SM_ChatChannel.LOCAL)
			url = m_Config.Discord.LocalWebhookUrl;
		else if (channel == SM_ChatChannel.GLOBAL)
			url = m_Config.Discord.GlobalWebhookUrl;
		if (url == "")
			url = m_Config.Discord.WebhookUrl;
		return url;
	}

	protected void RelayToDiscordUrl(string url, string channelLabel, string senderName, string text)
	{
		if (url == "")
			return;

		string localChannelLabel = SM_PartyLoc.Text(channelLabel);
		string title = localChannelLabel;
		if (m_Config.Discord.ServerLabel != "")
			title = m_Config.Discord.ServerLabel + " | " + localChannelLabel;

		string description = "**" + senderName + "**\n" + SM_PartyLoc.Text(text);
		string color = GetDiscordEmbedColor(channelLabel).ToString();
		string json = "{\"username\":\"SobrMods Clan System\",\"allowed_mentions\":{\"parse\":[]},\"embeds\":[{\"title\":\"" + JsonEscape(title) + "\",\"description\":\"" + JsonEscape(description) + "\",\"color\":" + color + ",\"footer\":{\"text\":\"SobrMods Clan System\"}}]}";

		PostDiscordJson(url, json);
	}

	protected void RelayEmbedToDiscordUrl(string url, string username, string title, string description, int color, string footer)
	{
		if (url == "")
			return;

		title = SM_PartyLoc.Text(title);
		description = SM_PartyLoc.Text(description);
		string json = "{\"username\":\"" + JsonEscape(username) + "\",\"allowed_mentions\":{\"parse\":[]},\"embeds\":[{\"title\":\"" + JsonEscape(title) + "\",\"description\":\"" + JsonEscape(description) + "\",\"color\":" + color.ToString() + ",\"footer\":{\"text\":\"" + JsonEscape(footer) + "\"}}]}";
		PostDiscordJson(url, json);
	}

	protected void PostDiscordJson(string url, string json)
	{
		int apiIdx = url.IndexOf("/api/");
		if (apiIdx <= 0)
			return;
		string baseUrl = url.Substring(0, apiIdx);
		string path = url.Substring(apiIdx, url.Length() - apiIdx);

		if (!m_RestApi)
			m_RestApi = GetRestApi();
		if (!m_RestApi)
			return;
		if (!m_DiscordCb)
			m_DiscordCb = new SM_RestCallback();

		RestContext rest = m_RestApi.GetRestContext(baseUrl);
		if (!rest)
			return;
		rest.SetHeader("application/json");
		rest.POST(m_DiscordCb, path, json);
	}

	protected int GetDiscordEmbedColor(string channelLabel)
	{
		if (channelLabel.Contains("#STR_SMP_00254"))
			return 15105570;
		if (channelLabel.Contains("#STR_SMP_00990"))
			return 5763719;
		if (channelLabel.Contains("#STR_SMP_00383"))
			return 3447003;
		if (channelLabel.Contains("#STR_SMP_00894"))
			return 10181046;
		if (channelLabel.Contains("#STR_SMP_00505"))
			return 15844367;
		return 15844367;
	}

	protected string JsonEscape(string s)
	{
		string result = "";
		for (int i = 0; i < s.Length(); i++)
		{
			string c = s.Get(i);
			if (c == "\"")
				result = result + "\\" + "\"";
			else if (c == "\\")
				result = result + "\\" + "\\";
			else if (c == "\n")
				result = result + "\\n";
			else if (c == "\r" || c == "\t")
				result = result + " ";
			else
				result = result + c;
		}
		return result;
	}

	protected string GetFullDateStamp()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return FormatTwo(day) + "." + FormatTwo(month) + "." + year.ToString() + " " + FormatTwo(hour) + ":" + FormatTwo(minute) + ":" + FormatTwo(second);
	}

	protected string BuildServerSessionKey()
	{
		return GetFullDateStamp() + " #" + Math.RandomInt(100000, 999999).ToString();
	}

	protected string FormatBaseCoordinates(vector position)
	{
		return "X " + Math.Round(position[0]).ToString() + " | Y " + Math.Round(position[1]).ToString() + " | Z " + Math.Round(position[2]).ToString();
	}

	protected string FormatMapCoordinatesShort(vector position)
	{
		return "X " + Math.Round(position[0]).ToString() + " / Z " + Math.Round(position[2]).ToString();
	}

	protected string CleanDiscordLinkText(string text)
	{
		text = text.Trim();
		text.Replace("[", "(");
		text.Replace("]", ")");
		text.Replace("\n", " ");
		text.Replace("\r", " ");
		if (text == "")
			text = "Unknown";
		return text;
	}

	protected string FormatSteamProfileLink(string name, string uid)
	{
		string cleanName = CleanDiscordLinkText(name);
		if (uid == "")
			return cleanName;
		return "[" + cleanName + "](https://steamcommunity.com/profiles/" + uid + ")";
	}

	protected SM_ClanMember GetClanLeaderMember(SM_Clan clan)
	{
		if (!clan || !clan.Members || clan.Members.Count() == 0)
			return null;

		int leaderRank = m_Config.GetLeaderRank();
		SM_ClanMember best = clan.Members[0];
		foreach (SM_ClanMember member : clan.Members)
		{
			if (!member)
				continue;
			if (member.Rank == leaderRank)
				return member;
			if (member.Rank > best.Rank)
				best = member;
		}
		return best;
	}

	protected int CountOnlineClanMembers(SM_Clan clan, map<string, PlayerBase> onlineMap)
	{
		if (!clan || !clan.Members || !onlineMap)
			return 0;

		int count = 0;
		foreach (SM_ClanMember member : clan.Members)
		{
			if (!member)
				continue;
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
				count++;
		}
		return count;
	}

	protected string BuildClanResidentsList(SM_Clan clan, map<string, PlayerBase> onlineMap)
	{
		if (!clan || !clan.Members || clan.Members.Count() == 0)
			return "-";

		string result = "";
		int hidden = 0;
		foreach (SM_ClanMember member : clan.Members)
		{
			if (!member)
				continue;

			string line = "• " + FormatSteamProfileLink(member.Name, member.Uid) + " — " + m_Config.GetRankName(member.Rank);
			PlayerBase pb;
			if (onlineMap && onlineMap.Find(member.Uid, pb))
				line = line + "#STR_SMP_00019";

			if (result.Length() + line.Length() + 1 > BASE_AUDIT_MEMBER_LIST_LIMIT)
			{
				hidden++;
				continue;
			}

			if (result != "")
				result = result + "\n";
			result = result + line;
		}

		if (hidden > 0)
			result = result + "#STR_SMP_00094" + hidden.ToString() + "#STR_SMP_00090";
		if (result == "")
			result = "-";
		return result;
	}

	protected string GetBaseAuditWebhookUrl()
	{
		if (!m_Config || !m_Config.BaseRegistrationAudit)
			return "";
		if (!m_Config.BaseRegistrationAudit.Enabled)
			return "";

		string url = m_Config.BaseRegistrationAudit.WebhookUrl;
		if (url != "")
			return url;

		if (m_Config.BaseRegistrationAudit.UseMainWebhookFallback && m_Config.Discord)
			return m_Config.Discord.WebhookUrl;
		return "";
	}

	protected string GetBaseAuditTitle(string actionTitle)
	{
		if (m_Config && m_Config.Discord && m_Config.Discord.ServerLabel != "")
			return m_Config.Discord.ServerLabel + " | " + actionTitle;
		return actionTitle;
	}

	protected string BuildBaseAuditDescription(SM_Clan clan, string eventText, PlayerBase actor, string targetUid, string targetName)
	{
		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);

		string desc = "#STR_SMP_00149" + eventText + "\n";
		desc = desc + "#STR_SMP_00145" + clan.Name + "\n";
		if (clan.Tag != "")
			desc = desc + "#STR_SMP_00150" + clan.Tag + "\n";
		desc = desc + "#STR_SMP_00151" + clan.Level.ToString() + "\n";
		desc = desc + "#STR_SMP_00143" + clan.Treasury.ToString() + "\n";
		desc = desc + "#STR_SMP_00152" + clan.Members.Count().ToString() + "#STR_SMP_00025" + CountOnlineClanMembers(clan, onlineMap).ToString() + "\n";

		SM_ClanMember leader = GetClanLeaderMember(clan);
		if (leader)
			desc = desc + "#STR_SMP_00153" + FormatSteamProfileLink(leader.Name, leader.Uid) + "\n";

		if (actor && actor.GetIdentity())
		{
			string actorUid = actor.GetIdentity().GetPlainId();
			desc = desc + "#STR_SMP_00148" + FormatSteamProfileLink(GetActorName(actor, clan), actorUid) + "\n";
			desc = desc + "**SteamID:** `" + actorUid + "`\n";
		}

		if (targetUid != "" || targetName != "")
			desc = desc + "#STR_SMP_00146" + FormatSteamProfileLink(targetName, targetUid) + "\n";

		if (clan.HasBase)
		{
			desc = desc + "#STR_SMP_00147" + FormatBaseCoordinates(clan.BasePos) + "`\n";
			desc = desc + "#STR_SMP_00144" + FormatMapCoordinatesShort(clan.BasePos) + "`\n";
		}

		desc = desc + "#STR_SMP_00142" + GetFullDateStamp();

		if (m_Config.BaseRegistrationAudit && m_Config.BaseRegistrationAudit.IncludeMemberList)
			desc = desc + "#STR_SMP_00093" + BuildClanResidentsList(clan, onlineMap);

		return desc;
	}

	protected void SendBaseAuditEmbed(SM_Clan clan, string title, string eventText, PlayerBase actor, string targetUid, string targetName, int color)
	{
		if (!clan)
			return;
		if (!clan.HasBase)
			return;

		string url = GetBaseAuditWebhookUrl();
		if (url == "")
			return;

		string desc = BuildBaseAuditDescription(clan, eventText, actor, targetUid, targetName);
		RelayEmbedToDiscordUrl(url, "SobrMods Base Audit", GetBaseAuditTitle(title), desc, color, "SobrMods Base Audit");
	}

	protected void SendBaseRegisterAudit(SM_Clan clan, PlayerBase actor, bool rebind)
	{
		if (!m_Config || !m_Config.BaseRegistrationAudit || !m_Config.BaseRegistrationAudit.SendRegister)
			return;

		string eventText = "#STR_SMP_00262";
		string title = "#STR_SMP_00889";
		if (rebind)
		{
			eventText = "#STR_SMP_00268";
			title = "#STR_SMP_00784";
		}
		SendBaseAuditEmbed(clan, title, eventText, actor, "", "", 5763719);
	}

	protected void SendBaseUnregisterAudit(SM_Clan clan, PlayerBase actor)
	{
		if (!m_Config || !m_Config.BaseRegistrationAudit || !m_Config.BaseRegistrationAudit.SendUnregister)
			return;
		SendBaseAuditEmbed(clan, "#STR_SMP_00749", "#STR_SMP_00267", actor, "", "", 15158332);
	}

	protected void SendBaseMemberChangeAudit(SM_Clan clan, string title, string eventText, PlayerBase actor, string targetUid, string targetName)
	{
		if (!m_Config || !m_Config.BaseRegistrationAudit || !m_Config.BaseRegistrationAudit.SendMemberChanges)
			return;
		SendBaseAuditEmbed(clan, title, eventText, actor, targetUid, targetName, 15844367);
	}

	protected void SendBaseRankChangeAudit(SM_Clan clan, string title, string eventText, PlayerBase actor, string targetUid, string targetName)
	{
		if (!m_Config || !m_Config.BaseRegistrationAudit || !m_Config.BaseRegistrationAudit.SendRankChanges)
			return;
		SendBaseAuditEmbed(clan, title, eventText, actor, targetUid, targetName, 10181046);
	}

	protected bool PrepareClanPing(PlayerBase player, out SM_Clan clan, out string uid, out SM_ClanMember sender, out float duration, out int pingId)
	{
		clan = null;
		uid = "";
		sender = null;
		duration = 0;
		pingId = 0;

		if (!m_Config.Pings.Enabled)
			return false;
		if (!player || !player.GetIdentity())
			return false;

		uid = player.GetIdentity().GetPlainId();
		clan = FindClanByMember(uid);
		if (!clan)
			return false;

		float now = GetGame().GetTickTime();
		float last;
		if (m_LastPings.Find(uid, last))
		{
			if (now - last < m_Config.Pings.CooldownSeconds)
				return false;
		}

		sender = clan.FindMember(uid);
		if (!sender)
			return false;

		m_LastPings.Set(uid, now);

		duration = m_Config.Pings.DurationSeconds;
		pingId = 1;
		int lastPingId;
		if (m_PingCounters.Find(uid, lastPingId))
			pingId = lastPingId + 1;
		m_PingCounters.Set(uid, pingId);
		return true;
	}

	protected void SendClanPing(SM_Clan clan, string authorName, vector position, float duration, string authorUid, int pingId, string label = "", string iconPath = "", int color = 0, bool suppressDefaultChat = false)
	{
		if (!clan || !clan.Members)
			return;

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_ClanMember member : clan.Members)
		{
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
			{
				ScriptRPC rpc = new ScriptRPC();
				rpc.Write(authorName);
				rpc.Write(position);
				rpc.Write(duration);
				rpc.Write(authorUid);
				rpc.Write(pingId);
				rpc.Write(label);
				rpc.Write(iconPath);
				rpc.Write(color);
				rpc.Write(suppressDefaultChat);
				rpc.Send(pb, SM_PartyRPC.MARK_SYNC, true, pb.GetIdentity());
			}
		}
	}

	protected void HandleMarkPlace(PlayerBase player, vector position)
	{
		SM_Clan clan;
		string uid;
		SM_ClanMember sender;
		float duration;
		int pingId;
		if (!PrepareClanPing(player, clan, uid, sender, duration, pingId))
			return;

		SendClanPing(clan, sender.Name, position, duration, uid, pingId);
	}

	protected string NormalizeItemPingName(string itemName)
	{
		itemName = itemName.Trim();
		itemName.Replace("\r", " ");
		itemName.Replace("\n", " ");
		itemName.Replace("\t", " ");

		while (itemName.Contains("  "))
			itemName.Replace("  ", " ");

		int maxLen = m_Config.ClanMap.MarkerMaxNameLength;
		if (maxLen < 16)
			maxLen = 16;
		if (maxLen > 64)
			maxLen = 64;

		if (itemName.LengthUtf8() > maxLen)
			itemName = itemName.SubstringUtf8(0, maxLen);

		return itemName;
	}

	protected string GetClanPingChatTextPrefix(int pingType)
	{
		pingType = SM_ClanPingTargetType.Normalize(pingType);
		if (pingType == SM_ClanPingTargetType.VEHICLE)
			return "#STR_SMP_01105";
		if (pingType == SM_ClanPingTargetType.CORPSE)
			return "#STR_SMP_01107";
		return "#STR_SMP_01103";
	}

	protected string GetClanPingLabelPrefix(int pingType)
	{
		pingType = SM_ClanPingTargetType.Normalize(pingType);
		if (pingType == SM_ClanPingTargetType.VEHICLE)
			return "#STR_SMP_01104";
		if (pingType == SM_ClanPingTargetType.CORPSE)
			return "#STR_SMP_01106";
		return "#STR_SMP_01102";
	}

	protected string GetClanPingIconPath(int pingType)
	{
		pingType = SM_ClanPingTargetType.Normalize(pingType);
		if (pingType == SM_ClanPingTargetType.VEHICLE)
			return SM_MapMarkerIconSet.GetPath(6);
		if (pingType == SM_ClanPingTargetType.CORPSE)
			return "SM_PartyMod\\GUI\\pings\\corpse.paa";
		return SM_MapMarkerIconSet.GetPath(2);
	}

	protected int GetClanPingColor(int pingType)
	{
		pingType = SM_ClanPingTargetType.Normalize(pingType);
		if (pingType == SM_ClanPingTargetType.VEHICLE)
			return SM_ClanColors.GetColor(6);
		if (pingType == SM_ClanPingTargetType.CORPSE)
			return SM_ClanColors.GetColor(1);
		return SM_ClanColors.GetColor(2);
	}

	protected void SendClanItemPingChat(SM_Clan clan, PlayerBase player, string uid, string itemName, int pingType)
	{
		if (!m_Config.ChatSystem.Enabled)
			return;
		if (!clan || !player)
			return;

		string senderName = GetActorName(player, clan);
		string prefix;
		int nameColor;
		int textColor;
		int prefixColor;
		GetChatStyle(SM_ChatChannel.CLAN, uid, prefix, nameColor, textColor, prefixColor);

		int channelColor = SM_ClanColors.ResolveColor(clan.Color);
		string text = GetClanPingChatTextPrefix(pingType) + itemName;

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_ClanMember member : clan.Members)
		{
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
				SendChatHistory(pb, SM_ChatChannel.CLAN, "#STR_SMP_00505", prefix, senderName, text, channelColor, nameColor, textColor, prefixColor);
		}
	}

	protected void HandleMarkItem(PlayerBase player, vector position, string itemName, int pingType = 0)
	{
		pingType = SM_ClanPingTargetType.Normalize(pingType);
		itemName = NormalizeItemPingName(itemName);
		if (itemName == "")
			return;

		SM_Clan clan;
		string uid;
		SM_ClanMember sender;
		float duration;
		int pingId;
		if (!PrepareClanPing(player, clan, uid, sender, duration, pingId))
			return;

		string label = GetClanPingLabelPrefix(pingType) + itemName;
		SendClanPing(clan, sender.Name, position, duration, uid, pingId, label, GetClanPingIconPath(pingType), GetClanPingColor(pingType), true);
		SendClanItemPingChat(clan, player, uid, itemName, pingType);
	}

	protected void HandleMarkClear(PlayerBase player)
	{
		if (!m_Config.Pings.Enabled)
			return;
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
			return;

		m_LastPings.Remove(uid);

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_ClanMember member : clan.Members)
		{
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
			{
				ScriptRPC rpc = new ScriptRPC();
				rpc.Write(uid);
				rpc.Send(pb, SM_PartyRPC.MARK_CLEAR_SYNC, true, pb.GetIdentity());
			}
		}
	}

	protected void HandleMapMarkerAdd(PlayerBase player, vector position, string name, int color, int icon)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.MAP_MARKERS, "#STR_SMP_00498", "#STR_SMP_00682"))
			return;

		if (!clan.MapMarkers)
			clan.MapMarkers = new array<ref SM_ClanMapMarker>;

		if (clan.MapMarkers.Count() >= m_Config.ClanMap.MaxMarkersPerClan)
		{
			Notify(player, "#STR_SMP_00498", "#STR_SMP_00605" + m_Config.ClanMap.MaxMarkersPerClan);
			return;
		}

		name = name.Trim();
		if (name == "")
		{
			Notify(player, "#STR_SMP_00498", "#STR_SMP_00315");
			return;
		}
		if (name.Length() > m_Config.ClanMap.MarkerMaxNameLength)
			name = name.Substring(0, m_Config.ClanMap.MarkerMaxNameLength);

		position[1] = GetGame().SurfaceY(position[0], position[2]);
		color = SM_ClanColors.NormalizeMarkerColor(color);
		icon = SM_MapMarkerIconSet.Normalize(icon);

		if (clan.NextMapMarkerId < 1)
			clan.NextMapMarkerId = 1;

		string actorName = GetActorName(player, clan);
		SM_ClanMapMarker marker = new SM_ClanMapMarker(clan.NextMapMarkerId, name, uid, actorName, position, color, icon);
		clan.NextMapMarkerId++;
		clan.MapMarkers.Insert(marker);

		AddLog(clan, actorName + "#STR_SMP_00040" + name);
		SaveDB();
		SyncClanState(clan);
	}

	protected void HandleMapMarkerRemove(PlayerBase player, int markerId)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan || !clan.MapMarkers)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.MAP_MARKERS, "#STR_SMP_00498", "#STR_SMP_00682"))
			return;

		for (int i = 0; i < clan.MapMarkers.Count(); i++)
		{
			SM_ClanMapMarker marker = clan.MapMarkers[i];
			if (marker.Id != markerId)
				continue;

			string actorName = GetActorName(player, clan);
			string markerName = marker.Name;
			clan.MapMarkers.Remove(i);
			AddLog(clan, actorName + "#STR_SMP_00088" + markerName);
			SaveDB();
			SyncClanState(clan);
			return;
		}

		Notify(player, "#STR_SMP_00498", "#STR_SMP_00630");
	}

	protected void HandleMapMarkerUpdate(PlayerBase player, int markerId, vector position, string name, int color, int icon)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan || !clan.MapMarkers)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.MAP_MARKERS, "#STR_SMP_00498", "#STR_SMP_00682"))
			return;

		name = name.Trim();
		if (name == "")
		{
			Notify(player, "#STR_SMP_00498", "#STR_SMP_00315");
			return;
		}
		if (name.Length() > m_Config.ClanMap.MarkerMaxNameLength)
			name = name.Substring(0, m_Config.ClanMap.MarkerMaxNameLength);

		position[1] = GetGame().SurfaceY(position[0], position[2]);
		color = SM_ClanColors.NormalizeMarkerColor(color);
		icon = SM_MapMarkerIconSet.Normalize(icon);

		foreach (SM_ClanMapMarker marker : clan.MapMarkers)
		{
			if (!marker || marker.Id != markerId)
				continue;

			string oldName = marker.Name;
			marker.Name = name;
			marker.Position = position;
			marker.Color = color;
			marker.Icon = icon;

			string actorName = GetActorName(player, clan);
			if (oldName == name)
				AddLog(clan, actorName + "#STR_SMP_00064" + name);
			else
				AddLog(clan, actorName + "#STR_SMP_00068" + oldName + " -> " + name);

			SaveDB();
			SyncClanState(clan);
			return;
		}

		Notify(player, "#STR_SMP_00498", "#STR_SMP_00630");
	}

	protected void HandleSetDescription(PlayerBase player, string text)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.DESCRIPTION, "#STR_SMP_00505", "#STR_SMP_00688"))
			return;

		text = text.Trim();
		if (text.Length() > m_Config.General.MaxClanDescriptionLength)
			text = text.Substring(0, m_Config.General.MaxClanDescriptionLength);

		if (clan.Description == text)
			return;

		clan.Description = text;
		AddLog(clan, GetActorName(player, clan) + "#STR_SMP_00047");
		SaveDB();

		Notify(player, "#STR_SMP_00505", "#STR_SMP_00734");
		SyncClanState(clan);
	}

	void SendClanInfo(PlayerBase player, string clanName)
	{
		if (!player || !player.GetIdentity())
			return;

		SM_Clan clan = FindClan(clanName);
		if (!clan)
			return;

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(clan.Name);
		rpc.Write(clan.Tag);
		rpc.Write(clan.Color);
		rpc.Write(clan.Description);
		rpc.Write(clan.Members.Count());
		foreach (SM_ClanMember member : clan.Members)
		{
			PlayerBase pb;
			bool online = onlineMap.Find(member.Uid, pb);
			rpc.Write(member.Name);
			rpc.Write(member.Rank);
			rpc.Write(online);
			rpc.Write(member.LastSeen);
			string playerTitle = "";
			int playerXP = 0;
			int playerTitleColor = 0;
			if (m_Config.PlayerExperience && m_Config.PlayerExperience.Enabled)
			{
				playerXP = GetPlayerExperienceXP(member.Uid);
				playerTitle = GetPlayerTitleByXP(playerXP);
				playerTitleColor = GetPlayerTitleColorByXP(playerXP);
			}
			rpc.Write(playerTitle);
			rpc.Write(playerXP);
			rpc.Write(playerTitleColor);
		}
		rpc.Send(player, SM_PartyRPC.CLAN_INFO, true, player.GetIdentity());
	}

	protected void HandleApply(PlayerBase player, string clanName)
	{
		string uid = player.GetIdentity().GetPlainId();

		if (FindClanByMember(uid))
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00352");
			return;
		}

		SM_Clan clan = FindClan(clanName);
		if (!clan)
			return;

		if (clan.Members.Count() >= m_Config.General.MaxClanMembers)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00519");
			return;
		}

		if (clan.FindApplication(uid))
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00452");
			return;
		}

		if (clan.Applications.Count() >= m_Config.General.MaxApplicationsPerClan)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_01017");
			return;
		}

		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		string stamp = FormatTwo(day) + "." + FormatTwo(month) + " " + FormatTwo(hour) + ":" + FormatTwo(minute);

		clan.Applications.Insert(new SM_ClanApplication(uid, player.GetIdentity().GetName(), stamp));
		SaveDB();

		Notify(player, "#STR_SMP_00505", "#STR_SMP_00450" + clan.Name + "#STR_SMP_00134");

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_ClanMember member : clan.Members)
		{
			if (member.Rank < clan.GetMinRank(SM_ClanAction.APPLICATIONS))
				continue;
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
			{
				Notify(pb, "#STR_SMP_00449", player.GetIdentity().GetName() + "#STR_SMP_00091");
				SyncStateTo(pb);
			}
		}
	}

	protected void HandleApplicationDecision(PlayerBase player, string applicantUid, bool accept)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.APPLICATIONS, "#STR_SMP_00505", "#STR_SMP_00685"))
			return;

		SM_ClanApplication app = clan.FindApplication(applicantUid);
		if (!app)
			return;

		string applicantName = app.Name;
		clan.Applications.RemoveItem(app);

		PlayerBase applicant = FindOnlinePlayer(applicantUid);

		if (!accept)
		{
			SaveDB();
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00451" + applicantName + "#STR_SMP_00066");
			if (applicant)
				Notify(applicant, "#STR_SMP_00505", "#STR_SMP_00309" + clan.Name + "#STR_SMP_00133");
			SyncClanState(clan);
			return;
		}

		if (FindClanByMember(applicantUid))
		{
			SaveDB();
			Notify(player, "#STR_SMP_00505", applicantName + "#STR_SMP_00089");
			SyncClanState(clan);
			return;
		}

		if (clan.Members.Count() >= m_Config.General.MaxClanMembers)
		{
			SaveDB();
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00520" + m_Config.General.MaxClanMembers + ")");
			SyncClanState(clan);
			return;
		}

		string applicantLastSeen = "";
		if (applicant)
			applicantLastSeen = MakeDateStamp();
		clan.Members.Insert(new SM_ClanMember(applicantUid, applicantName, 0, applicantLastSeen));
		RemoveApplicationsEverywhere(applicantUid);
		m_Invites.Remove(applicantUid);
		AddLog(clan, applicantName + "#STR_SMP_00078" + GetActorName(player, clan) + ")");
		SendBaseMemberChangeAudit(clan, "#STR_SMP_00476", applicantName + "#STR_SMP_00079" + GetActorName(player, clan), player, applicantUid, applicantName);
		SaveDB();
		if (applicant)
			AddPersonalAchievementValue(applicantUid, applicantName, "JoinedClan", 1, applicant);
		else
			AddPersonalAchievementValue(applicantUid, applicantName, "JoinedClan", 1, null);
		CheckClanAchievementNotifications(clan);

		NotifyClan(clan, "#STR_SMP_00505", applicantName + "#STR_SMP_00033");
		if (applicant)
		{
			Notify(applicant, "#STR_SMP_00505", "#STR_SMP_00313" + clan.Name + "\"");
			SyncStateTo(applicant);
		}
		SyncClanState(clan);
	}

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

	protected SM_Clan FindClanByPlayer(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return null;
		return FindClanByMember(player.GetIdentity().GetPlainId());
	}

	protected void EnsurePendingPersonalDeathMapMarkers()
	{
		if (!m_PendingPersonalDeathMapMarkers)
			m_PendingPersonalDeathMapMarkers = new map<string, ref SM_DeathMapMarkerView>;
	}

	protected string BuildDeathClanMapMarkerName(string victimName)
	{
		string prefix = "#STR_SMP_01101";
		victimName = victimName.Trim();
		if (victimName == "")
			return "#STR_SMP_01100";

		int maxLen = m_Config.ClanMap.MarkerMaxNameLength;
		if (maxLen <= prefix.Length())
			return "#STR_SMP_01100";

		int nameMaxLen = maxLen - prefix.Length();
		if (victimName.LengthUtf8() > nameMaxLen)
			victimName = victimName.SubstringUtf8(0, nameMaxLen);

		return prefix + victimName;
	}

	protected bool RemoveOldestClanDeathMapMarker(SM_Clan clan)
	{
		if (!clan || !clan.MapMarkers)
			return false;

		for (int i = 0; i < clan.MapMarkers.Count(); i++)
		{
			SM_ClanMapMarker marker = clan.MapMarkers[i];
			if (marker && marker.Icon == SM_MapMarkerIconSet.DeathIcon())
			{
				clan.MapMarkers.Remove(i);
				return true;
			}
		}
		return false;
	}

	protected bool RemoveClanDeathMapMarkerForUid(SM_Clan clan, string victimUid)
	{
		if (!clan || !clan.MapMarkers || victimUid == "")
			return false;

		bool removed = false;
		for (int i = clan.MapMarkers.Count() - 1; i >= 0; i--)
		{
			SM_ClanMapMarker marker = clan.MapMarkers[i];
			if (!marker)
				continue;
			if (marker.Icon != SM_MapMarkerIconSet.DeathIcon())
				continue;
			if (marker.AuthorUid != victimUid)
				continue;

			clan.MapMarkers.Remove(i);
			removed = true;
		}

		return removed;
	}

	protected void SendPersonalDeathMapMarker(PlayerBase player, SM_DeathMapMarkerView marker)
	{
		if (!player || !player.GetIdentity() || !marker)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(marker.Name);
		rpc.Write(marker.Position);
		rpc.Write(marker.Color);
		rpc.Write(marker.Icon);
		rpc.Send(player, SM_PartyRPC.PERSONAL_DEATH_MARKER, true, player.GetIdentity());
	}

	protected void SendPendingPersonalDeathMapMarker(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		EnsurePendingPersonalDeathMapMarkers();

		string uid = player.GetIdentity().GetPlainId();
		SM_DeathMapMarkerView marker;
		if (!m_PendingPersonalDeathMapMarkers.Find(uid, marker))
			return;
		if (!marker)
		{
			m_PendingPersonalDeathMapMarkers.Remove(uid);
			return;
		}

		SendPersonalDeathMapMarker(player, marker);
		m_PendingPersonalDeathMapMarkers.Remove(uid);
	}

	protected void AddClanDeathMapMarker(SM_Clan clan, string victimUid, string victimName, vector position)
	{
		if (!clan)
			return;
		if (!clan.MapMarkers)
			clan.MapMarkers = new array<ref SM_ClanMapMarker>;
		if (m_Config.ClanMap.MaxMarkersPerClan <= 0)
			return;

		RemoveClanDeathMapMarkerForUid(clan, victimUid);

		if (clan.MapMarkers.Count() >= m_Config.ClanMap.MaxMarkersPerClan)
		{
			if (!RemoveOldestClanDeathMapMarker(clan))
				return;
		}

		if (clan.NextMapMarkerId < 1)
			clan.NextMapMarkerId = 1;

		string markerName = BuildDeathClanMapMarkerName(victimName);
		SM_ClanMapMarker marker = new SM_ClanMapMarker(clan.NextMapMarkerId, markerName, victimUid, victimName, position, 1, SM_MapMarkerIconSet.DeathIcon());
		clan.NextMapMarkerId++;
		clan.MapMarkers.Insert(marker);

		SaveDB();
		SyncClanState(clan);
	}

	void OnPlayerDeathMarker(PlayerBase victim)
	{
		if (!victim || !victim.GetIdentity())
			return;

		string uid = victim.GetIdentity().GetPlainId();
		string victimName = victim.GetIdentity().GetName();
		vector position = victim.GetPosition();
		position[1] = GetGame().SurfaceY(position[0], position[2]);

		int icon = SM_MapMarkerIconSet.DeathIcon();
		int color = 1;
		SM_DeathMapMarkerView personalMarker = new SM_DeathMapMarkerView("#STR_SMP_01100", position, color, icon);

		EnsurePendingPersonalDeathMapMarkers();
		m_PendingPersonalDeathMapMarkers.Set(uid, personalMarker);
		SendPersonalDeathMapMarker(victim, personalMarker);

		SM_Clan clan = FindClanByMember(uid);
		if (clan)
			AddClanDeathMapMarker(clan, uid, victimName, position);
	}

	void OnStatPlayerKilled(PlayerBase victim, Object killer)
	{
		bool progressSystemsEnabled = false;
		if (m_Config.Tops.Enabled || m_Config.AchievementSettings.Enabled || IsPlayerExperienceEnabled())
			progressSystemsEnabled = true;
		if (!progressSystemsEnabled && !IsContractsEnabled())
			return;

		PlayerBase killerPlayer = ResolvePlayer(killer);
		PlayerBase hitShooter = ResolveLastPlayerHitShooter(victim);
		if (!killerPlayer && hitShooter)
			killerPlayer = hitShooter;
		if (!killerPlayer || killerPlayer == victim)
		{
			ClearLongKillTargetData(victim);
			return;
		}
		if (!killerPlayer.GetIdentity())
		{
			ClearLongKillTargetData(victim);
			return;
		}

		CompleteContractKill(killerPlayer, victim);
		if (!progressSystemsEnabled)
		{
			ClearLongKillTargetData(victim);
			return;
		}

		UpdatePersonalLongestKill(killerPlayer, victim);

		SM_Clan clan = FindClanByPlayer(killerPlayer);
		bool victimIsClanMate = false;
		if (clan && victim && victim.GetIdentity() && clan.FindMember(victim.GetIdentity().GetPlainId()))
			victimIsClanMate = true;

		if (victim && victim.GetIdentity() && !victimIsClanMate)
		{
			AddPersonalAchievementValue(killerPlayer.GetIdentity().GetPlainId(), killerPlayer.GetIdentity().GetName(), "PlayerKills", 1, killerPlayer);
			if (IsPlayerExperienceEnabled())
				AwardPlayerExperience(killerPlayer, m_Config.PlayerExperience.PlayerKillXP, "#STR_SMP_01035", 0, 1, 0);
		}

		if (!clan)
		{
			ClearLongKillTargetData(victim);
			return;
		}

		UpdateLongestPlayerKill(clan, killerPlayer, victim);
		ClearLongKillTargetData(victim);

		if (victimIsClanMate)
			return;

		clan.Stats.PlayerKills++;
		m_StatsDirty = true;
		CheckClanAchievementNotifications(clan);
	}

	protected PlayerBase ResolveLastPlayerHitShooter(EntityAI victim)
	{
		if (!victim)
			return null;
		if (!m_LastPlayerHitShooterUid || !m_LastPlayerHitTime)
			return null;

		string victimUid = GetLongKillTargetKey(victim);
		if (victimUid == "")
			return null;

		string shooterUid;
		if (!m_LastPlayerHitShooterUid.Find(victimUid, shooterUid))
			return null;

		float hitTime;
		if (!m_LastPlayerHitTime.Find(victimUid, hitTime))
			return null;
		if (GetGame().GetTickTime() - hitTime > TOP_LONG_KILL_HIT_WINDOW_SECONDS)
			return null;

		return FindOnlinePlayer(shooterUid);
	}

	protected void UpdateLongestPlayerKill(SM_Clan clan, PlayerBase killerPlayer, PlayerBase victim)
	{
		if (!clan || !killerPlayer || !victim || !killerPlayer.GetIdentity())
			return;
		if (!m_LastPlayerHitShooterUid || !m_LastPlayerHitDistance || !m_LastPlayerHitTime)
			return;

		string victimUid = GetLongKillTargetKey(victim);
		if (victimUid == "")
			return;

		string shooterUid;
		if (!m_LastPlayerHitShooterUid.Find(victimUid, shooterUid))
			return;
		if (shooterUid != killerPlayer.GetIdentity().GetPlainId())
			return;

		float hitTime;
		if (!m_LastPlayerHitTime.Find(victimUid, hitTime))
			return;
		if (GetGame().GetTickTime() - hitTime > TOP_LONG_KILL_HIT_WINDOW_SECONDS)
			return;

		float distance;
		if (!m_LastPlayerHitDistance.Find(victimUid, distance))
			return;

		int roundedDistance = Math.Round(distance);
		if (roundedDistance <= clan.Stats.LongestPlayerKillDistance)
			return;

		clan.Stats.LongestPlayerKillDistance = roundedDistance;
		clan.Stats.LongestPlayerKillName = GetLongKillActorName(killerPlayer) + " -> " + GetLongKillTargetName(victim);
		m_StatsDirty = true;
		CheckClanAchievementNotifications(clan);
	}

	protected void UpdatePersonalLongestKill(PlayerBase killerPlayer, PlayerBase victim)
	{
		if (!killerPlayer || !victim || !killerPlayer.GetIdentity())
			return;
		if (!m_LastPlayerHitShooterUid || !m_LastPlayerHitDistance || !m_LastPlayerHitTime)
			return;

		string victimUid = GetLongKillTargetKey(victim);
		if (victimUid == "")
			return;

		string shooterUid;
		if (!m_LastPlayerHitShooterUid.Find(victimUid, shooterUid))
			return;
		if (shooterUid != killerPlayer.GetIdentity().GetPlainId())
			return;

		float hitTime;
		if (!m_LastPlayerHitTime.Find(victimUid, hitTime))
			return;
		if (GetGame().GetTickTime() - hitTime > TOP_LONG_KILL_HIT_WINDOW_SECONDS)
			return;

		float distance;
		if (!m_LastPlayerHitDistance.Find(victimUid, distance))
			return;

		SetPersonalAchievementBest(killerPlayer.GetIdentity().GetPlainId(), killerPlayer.GetIdentity().GetName(), "LongestKill", Math.Round(distance), killerPlayer);
	}

	protected string GetLongKillTargetKey(EntityAI target)
	{
		if (!target)
			return "";

		PlayerBase playerTarget = PlayerBase.Cast(target);
		if (playerTarget && playerTarget.GetIdentity())
			return playerTarget.GetIdentity().GetPlainId();

		return "obj:" + target.ToString();
	}

	protected void ClearLongKillTargetData(EntityAI target)
	{
		string targetKey = GetLongKillTargetKey(target);
		if (targetKey == "")
			return;

		if (m_LastPlayerHitShooterUid)
			m_LastPlayerHitShooterUid.Remove(targetKey);
		if (m_LastPlayerHitDistance)
			m_LastPlayerHitDistance.Remove(targetKey);
		if (m_LastPlayerHitTime)
			m_LastPlayerHitTime.Remove(targetKey);
	}

	protected string GetLongKillActorName(PlayerBase player)
	{
		if (!player)
			return "#STR_SMP_00463";
		if (player.GetIdentity())
			return player.GetIdentity().GetName();

		string name = player.GetDisplayName();
		if (name != "")
			return name;
		return player.GetType();
	}

	protected string GetLongKillTargetName(PlayerBase victim)
	{
		if (!victim)
			return "#STR_SMP_01053";
		if (victim.GetIdentity())
			return victim.GetIdentity().GetName();

		string name = victim.GetDisplayName();
		if (name != "")
			return name;
		return victim.GetType();
	}

	void OnStatZombieKilled(Object zombie, Object killer)
	{
		if (!m_Config.Tops.Enabled && !m_Config.AchievementSettings.Enabled && !IsPlayerExperienceEnabled())
			return;
		if (!IsZombieStatTarget(zombie))
			return;

		PlayerBase killerPlayer = ResolvePlayer(killer);
		if (killerPlayer && killerPlayer.GetIdentity())
		{
			AddPersonalAchievementValue(killerPlayer.GetIdentity().GetPlainId(), killerPlayer.GetIdentity().GetName(), "ZombieKills", 1, killerPlayer);
			if (IsPlayerExperienceEnabled())
				AwardPlayerExperience(killerPlayer, GetZombieExperience(zombie), "#STR_SMP_00445", 1, 0, 0);
		}

		SM_Clan clan = FindClanByPlayer(killerPlayer);
		if (!clan)
			return;

		clan.Stats.ZombieKills++;
		m_StatsDirty = true;
		CheckClanAchievementNotifications(clan);
	}

	void OnStatMemberDied(PlayerBase victim)
	{
		if ((!m_Config.Tops.Enabled && !m_Config.AchievementSettings.Enabled) || !victim || !victim.GetIdentity())
			return;

		string uid = victim.GetIdentity().GetPlainId();
		AddPersonalAchievementValue(uid, victim.GetIdentity().GetName(), "PlayerDeaths", 1, victim);
		int lifeSeconds = Math.Round(victim.StatGet(AnalyticsManagerServer.STAT_PLAYTIME));
		SetPersonalAchievementBest(uid, victim.GetIdentity().GetName(), "BestLifeSeconds", lifeSeconds, victim);
		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
			return;

		m_LastPositions.Remove(uid);
		m_LastAchievementPositions.Remove(uid);
		if (m_LastPlayerHitShooterUid)
			m_LastPlayerHitShooterUid.Remove(uid);
		if (m_LastPlayerHitDistance)
			m_LastPlayerHitDistance.Remove(uid);
		if (m_LastPlayerHitTime)
			m_LastPlayerHitTime.Remove(uid);

		clan.Stats.PlayerDeaths++;
		UpdateBestLife(clan, victim.GetIdentity().GetName(), lifeSeconds);
		CheckClanAchievementNotifications(clan);
	}

	protected void UpdateBestLife(SM_Clan clan, string name, int lifeSeconds)
	{
		if (lifeSeconds <= clan.Stats.BestLifeSeconds)
			return;
		clan.Stats.BestLifeSeconds = lifeSeconds;
		clan.Stats.BestLifeName = name;
		m_StatsDirty = true;
	}

	void OnStatShotFired(Object shooter)
	{
		if (!m_Config.Tops.Enabled && !m_Config.AchievementSettings.Enabled)
			return;

		PlayerBase shooterPlayer = ResolvePlayer(shooter);
		if (shooterPlayer && shooterPlayer.GetIdentity())
			AddPersonalAchievementValue(shooterPlayer.GetIdentity().GetPlainId(), shooterPlayer.GetIdentity().GetName(), "ShotsFired", 1, shooterPlayer);

		SM_Clan clan = FindClanByPlayer(shooterPlayer);
		if (!clan)
			return;

		clan.Stats.ShotsFired++;
		m_StatsDirty = true;
		CheckClanAchievementNotifications(clan);
	}

	void OnStatHit(EntityAI target, EntityAI source, string dmgZone)
	{
		if ((!m_Config.Tops.Enabled && !m_Config.AchievementSettings.Enabled) || !source)
			return;

		PlayerBase shooter = ResolvePlayer(source);
		if (!shooter || shooter == target)
			return;

		if (shooter.GetIdentity())
		{
			AddPersonalAchievementValue(shooter.GetIdentity().GetPlainId(), shooter.GetIdentity().GetName(), "HitsLanded", 1, shooter);
			if (dmgZone.Contains("Head") || dmgZone.Contains("Brain"))
				AddPersonalAchievementValue(shooter.GetIdentity().GetPlainId(), shooter.GetIdentity().GetName(), "Headshots", 1, shooter);
		}

		SM_Clan clan = FindClanByPlayer(shooter);
		if (clan)
		{
			clan.Stats.HitsLanded++;
			if (dmgZone.Contains("Head") || dmgZone.Contains("Brain"))
				clan.Stats.Headshots++;
			m_StatsDirty = true;
			CheckClanAchievementNotifications(clan);
		}

		PlayerBase victimPlayer = PlayerBase.Cast(target);
		if (victimPlayer && shooter.GetIdentity())
		{
			string victimUid = GetLongKillTargetKey(victimPlayer);
			vector shooterPos = shooter.GetPosition();
			vector victimPos = victimPlayer.GetPosition();
			float dx = shooterPos[0] - victimPos[0];
			float dz = shooterPos[2] - victimPos[2];
			float distance = Math.Sqrt(dx * dx + dz * dz);
			if (victimUid != "")
			{
				m_LastPlayerHitShooterUid.Set(victimUid, shooter.GetIdentity().GetPlainId());
				m_LastPlayerHitDistance.Set(victimUid, distance);
				m_LastPlayerHitTime.Set(victimUid, GetGame().GetTickTime());
			}
		}
	}

	protected void StatsTick()
	{
		if (!m_Config.Tops.Enabled && !m_Config.AchievementSettings.Enabled && !IsPlayerExperienceEnabled())
			return;

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);

		AwardOnlineExperience(onlineMap);

		if (m_Config.AchievementSettings.Enabled)
		{
			for (int p = 0; p < onlineMap.Count(); p++)
			{
				string playerUid = onlineMap.GetKey(p);
				PlayerBase onlinePlayer = onlineMap.GetElement(p);
				if (!onlinePlayer || !onlinePlayer.GetIdentity() || !onlinePlayer.IsAlive())
					continue;

				AddPersonalAchievementValue(playerUid, onlinePlayer.GetIdentity().GetName(), "OnlineSeconds", STATS_TICK_SECONDS, onlinePlayer);

				vector playerPos = onlinePlayer.GetPosition();
				vector lastPlayerPos;
				if (m_LastAchievementPositions.Find(playerUid, lastPlayerPos))
				{
					if (!onlinePlayer.IsInVehicle() && !onlinePlayer.IsUnconscious())
					{
						float adx = playerPos[0] - lastPlayerPos[0];
						float adz = playerPos[2] - lastPlayerPos[2];
						float adist = Math.Sqrt(adx * adx + adz * adz);
						if (adist > 0.5 && adist < 120)
							AddPersonalAchievementValue(playerUid, onlinePlayer.GetIdentity().GetName(), "DistanceWalked", Math.Round(adist), onlinePlayer);
					}
				}
				m_LastAchievementPositions.Set(playerUid, playerPos);

				int playerLifeSeconds = Math.Round(onlinePlayer.StatGet(AnalyticsManagerServer.STAT_PLAYTIME));
				SetPersonalAchievementBest(playerUid, onlinePlayer.GetIdentity().GetName(), "BestLifeSeconds", playerLifeSeconds, onlinePlayer);
			}
		}

		if (!m_Config.Tops.Enabled && !m_Config.AchievementSettings.Enabled)
			return;

		foreach (SM_Clan clan : m_DB.Clans)
		{
			bool clanChanged = false;
			foreach (SM_ClanMember member : clan.Members)
			{
				PlayerBase pb;
				if (!onlineMap.Find(member.Uid, pb))
					continue;
				if (!pb.IsAlive())
					continue;

				clan.Stats.OnlineSeconds += STATS_TICK_SECONDS;

				vector pos = pb.GetPosition();
				vector last;
				if (m_LastPositions.Find(member.Uid, last))
				{
					if (!pb.IsInVehicle() && !pb.IsUnconscious())
					{
						float dx = pos[0] - last[0];
						float dz = pos[2] - last[2];
						float dist = Math.Sqrt(dx * dx + dz * dz);
						if (dist > 0.5 && dist < 120)
						{
							clan.Stats.DistanceWalked += Math.Round(dist);
							clanChanged = true;
						}
					}
				}
				m_LastPositions.Set(member.Uid, pos);

				int lifeSeconds = Math.Round(pb.StatGet(AnalyticsManagerServer.STAT_PLAYTIME));
				if (lifeSeconds > clan.Stats.BestLifeSeconds)
				{
					clan.Stats.BestLifeSeconds = lifeSeconds;
					clan.Stats.BestLifeName = member.Name;
					clanChanged = true;
				}

				m_StatsDirty = true;
				clanChanged = true;
			}
			if (clanChanged)
				CheckClanAchievementNotifications(clan);
		}
	}

	protected void StatsSaveTick()
	{
		if (!m_StatsDirty && !m_AchievementsDirty && !m_PlayerExperienceDirty)
			return;
		if (m_StatsDirty)
		{
			m_StatsDirty = false;
			SaveDB();
		}
		if (m_AchievementsDirty)
		{
			m_AchievementsDirty = false;
			SaveAchievementsDB();
		}
		if (m_PlayerExperienceDirty)
		{
			m_PlayerExperienceDirty = false;
			SavePlayerExperienceDB();
		}
	}

	protected int GetTopSeasonSecondsLeft()
	{
		if (!m_Config.Tops.Enabled)
			return 0;

		int nowSeconds = GetNowAbsSeconds();
		int targetSeconds;
		if (m_Config.Tops.RewardSchedule == "monthly")
		{
			int year;
			int month;
			int day;
			GetYearMonthDay(year, month, day);

			int nextYear = year;
			int nextMonth = month + 1;
			if (nextMonth > 12)
			{
				nextMonth = 1;
				nextYear++;
			}

			targetSeconds = AbsSeconds(nextYear, nextMonth, 1, 0, 0, 0);
		}
		else
		{
			EnsureTopRewardSeasonStarted(true);
			if (m_DB.LastRewardStampSeconds <= 0)
				return m_Config.Tops.RewardIntervalHours * 3600;
			targetSeconds = m_DB.LastRewardStampSeconds + m_Config.Tops.RewardIntervalHours * 3600;
		}

		int left = targetSeconds - nowSeconds;
		if (left < 0)
			left = 0;
		return left;
	}

	protected void EnsureTopRewardSeasonStarted(bool save)
	{
		if (!m_Config.Tops.Enabled)
			return;
		if (m_Config.Tops.RewardSchedule != "hours")
			return;

		bool changed = false;
		if (m_DB.LastRewardStampSeconds <= 0 && m_DB.LastRewardStampMinutes > 0)
		{
			m_DB.LastRewardStampSeconds = m_DB.LastRewardStampMinutes * 60;
			changed = true;
		}

		if (m_DB.LastRewardStampSeconds <= 0)
		{
			m_DB.LastRewardStampSeconds = GetNowAbsSeconds();
			m_DB.LastRewardStampMinutes = m_DB.LastRewardStampSeconds / 60;
			changed = true;
		}
		else
		{
			int rewardMinutes = m_DB.LastRewardStampSeconds / 60;
			if (m_DB.LastRewardStampMinutes != rewardMinutes)
			{
				m_DB.LastRewardStampMinutes = rewardMinutes;
				changed = true;
			}
		}

		if (changed && save)
			SaveDB();
	}

	void SendTops(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Config.Tops.Enabled);
		rpc.Write(m_Config.Tops.MinShotsForAccuracyTop);
		rpc.Write(GetTopSeasonSecondsLeft());
		rpc.Write(SM_TopCategory.COUNT);
		for (int category = 0; category < SM_TopCategory.COUNT; category++)
			rpc.Write(m_Config.IsTopCategoryEnabled(category));

		if (!m_Config.Tops.Enabled)
		{
			rpc.Write(0);
			rpc.Send(player, SM_PartyRPC.SYNC_TOPS, true, player.GetIdentity());
			return;
		}

		rpc.Write(m_DB.Clans.Count());
		foreach (SM_Clan clan : m_DB.Clans)
		{
			rpc.Write(clan.Name);
			rpc.Write(clan.Tag);
			rpc.Write(clan.Color);
			rpc.Write(clan.Stats.PlayerKills);
			rpc.Write(clan.Stats.PlayerDeaths);
			rpc.Write(clan.Stats.ZombieKills);
			rpc.Write(clan.Stats.OnlineSeconds);
			rpc.Write(clan.Stats.BestLifeSeconds);
			rpc.Write(clan.Stats.BestLifeName);
			rpc.Write(clan.Stats.DistanceWalked);
			rpc.Write(clan.Stats.ShotsFired);
			rpc.Write(clan.Stats.HitsLanded);
			rpc.Write(clan.Stats.Headshots);
			rpc.Write(clan.Stats.LongestPlayerKillDistance);
			rpc.Write(clan.Stats.LongestPlayerKillName);
		}
		rpc.Send(player, SM_PartyRPC.SYNC_TOPS, true, player.GetIdentity());
	}

	protected int GetCurrencyValue(string itemType, array<ref SM_CurrencyItem> currency)
	{
		itemType.ToLower();
		foreach (SM_CurrencyItem entry : currency)
		{
			string classname = entry.Classname;
			classname.ToLower();
			if (classname == itemType)
				return entry.Value;
		}
		return 0;
	}

	protected void HandleTreasuryDeposit(PlayerBase player, int amount)
	{
		if (!m_Config.Treasury.Enabled)
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
			return;

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (currency.Count() == 0)
		{
			Notify(player, "#STR_SMP_00482", "#STR_SMP_00295");
			return;
		}

		int points;
		if (amount <= 0)
			points = ConsumeAllCurrency(player, currency);
		else
			points = ConsumeCurrency(player, amount, currency);

		if (points <= 0)
		{
			if (amount > 0)
				Notify(player, "#STR_SMP_00482", "#STR_SMP_00675" + amount + "#STR_SMP_00168");
			else
				Notify(player, "#STR_SMP_00482", "#STR_SMP_00286");
			return;
		}

		SM_ClanMember actor = clan.FindMember(uid);
		clan.Treasury += points;
		clan.Stats.TreasuryDeposited += points;
		AddLog(clan, actor.Name + "#STR_SMP_00031" + points + "#STR_SMP_00014" + clan.Treasury + ")");
		SaveDB();
		AddPersonalAchievementValue(uid, player.GetIdentity().GetName(), "TreasuryDeposited", points, player);
		CheckClanAchievementNotifications(clan);

		if (amount > 0 && points < amount)
			Notify(player, "#STR_SMP_00482", "#STR_SMP_00325" + points + "#STR_SMP_00045" + amount + "#STR_SMP_00015" + clan.Treasury);
		else
			Notify(player, "#STR_SMP_00482", "#STR_SMP_00325" + points + "#STR_SMP_00164" + clan.Treasury);
		NotifyClan(clan, "#STR_SMP_00482", actor.Name + "#STR_SMP_00031" + points, uid);
		SyncClanState(clan);
	}

	protected int ConsumeAllCurrency(PlayerBase player, array<ref SM_CurrencyItem> currency)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		int points = 0;
		array<EntityAI> toDelete = new array<EntityAI>;
		foreach (EntityAI item : items)
		{
			if (!item)
				continue;
			int value = GetCurrencyValue(item.GetType(), currency);
			if (value <= 0)
				continue;

			int count = 1;
			ItemBase ib = ItemBase.Cast(item);
			if (ib && ib.CanBeSplit())
			{
				count = Math.Floor(ib.GetQuantity());
				if (count < 1)
					count = 1;
			}

			points += value * count;
			toDelete.Insert(item);
		}

		foreach (EntityAI deleteItem : toDelete)
			GetGame().ObjectDelete(deleteItem);

		return points;
	}

	protected int ConsumeCurrency(PlayerBase player, int amount, array<ref SM_CurrencyItem> currency)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		array<EntityAI> wholeItems = new array<EntityAI>;
		array<int> wholeValues = new array<int>;
		array<ItemBase> splitItems = new array<ItemBase>;
		array<int> splitValues = new array<int>;

		foreach (EntityAI item : items)
		{
			if (!item)
				continue;
			int value = GetCurrencyValue(item.GetType(), currency);
			if (value <= 0)
				continue;

			ItemBase ib = ItemBase.Cast(item);
			if (ib && ib.CanBeSplit())
			{
				splitItems.Insert(ib);
				splitValues.Insert(value);
			}
			else
			{
				wholeItems.Insert(item);
				wholeValues.Insert(value);
			}
		}

		int remaining = amount;
		int consumed = 0;

		array<int> wholeOrder = new array<int>;
		for (int wi = 0; wi < wholeItems.Count(); wi++)
		{
			int insertAt = wholeOrder.Count();
			for (int w = 0; w < wholeOrder.Count(); w++)
			{
				if (wholeValues[wi] > wholeValues[wholeOrder[w]])
				{
					insertAt = w;
					break;
				}
			}
			wholeOrder.InsertAt(wi, insertAt);
		}
		foreach (int idx : wholeOrder)
		{
			if (remaining <= 0)
				break;
			int v = wholeValues[idx];
			if (v > remaining)
				continue;
			GetGame().ObjectDelete(wholeItems[idx]);
			remaining -= v;
			consumed += v;
		}

		array<int> splitOrder = new array<int>;
		for (int si = 0; si < splitItems.Count(); si++)
		{
			int sInsertAt = splitOrder.Count();
			for (int s = 0; s < splitOrder.Count(); s++)
			{
				if (splitValues[si] < splitValues[splitOrder[s]])
				{
					sInsertAt = s;
					break;
				}
			}
			splitOrder.InsertAt(si, sInsertAt);
		}
		foreach (int sidx : splitOrder)
		{
			if (remaining <= 0)
				break;
			int sv = splitValues[sidx];
			ItemBase stack = splitItems[sidx];
			int qty = Math.Floor(stack.GetQuantity());
			if (qty < 1)
				continue;
			int units = remaining / sv;
			if (units > qty)
				units = qty;
			if (units <= 0)
				continue;
			int leftQty = qty - units;
			if (leftQty <= 0)
				GetGame().ObjectDelete(stack);
			else
				stack.SetQuantity(leftQty);
			remaining -= units * sv;
			consumed += units * sv;
		}

		return consumed;
	}

	protected void HandleClanUpgrade(PlayerBase player)
	{
		if (!m_Config.Levels.Enabled)
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.UPGRADE, "#STR_SMP_00505", "#STR_SMP_00692"))
			return;

		int cost = m_Config.GetUpgradeCost(clan.Level);
		if (cost < 0)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00420" + clan.Level + ")");
			return;
		}

		if (clan.Treasury < cost)
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00679" + cost + "#STR_SMP_00155" + clan.Treasury);
			return;
		}

		clan.Treasury -= cost;
		clan.Level++;
		SM_ClanLevel data = m_Config.GetLevelData(clan.Level);
		AddLog(clan, GetActorName(player, clan) + "#STR_SMP_00080" + clan.Level + " (-" + cost + "#STR_SMP_00046");
		SaveDB();
		CheckClanAchievementNotifications(clan);

		Notify(player, "#STR_SMP_00505", "#STR_SMP_00532" + clan.Level + "#STR_SMP_00129" + data.StorageSlots + "#STR_SMP_00158" + data.MarketLots + "#STR_SMP_00161" + data.ServerMarketItems);
		NotifyClan(clan, "#STR_SMP_00505", "#STR_SMP_00532" + clan.Level, uid);
		SyncClanState(clan);
	}

	protected void HandleSetPermission(PlayerBase player, int action, int minRank)
	{
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (GetActorRank(uid, clan) != m_Config.GetLeaderRank())
		{
			Notify(player, "#STR_SMP_00505", "#STR_SMP_00673");
			return;
		}

		if (action < 0 || action >= SM_ClanAction.COUNT)
			return;

		int leaderRank = m_Config.GetLeaderRank();
		if (minRank < 0)
			minRank = 0;
		if (minRank > leaderRank)
			minRank = leaderRank;

		if (!clan.Perms || clan.Perms.Count() < SM_ClanAction.COUNT)
		{
			array<int> defaultPerms;
			m_Config.GetDefaultPerms(defaultPerms);
			if (!clan.Perms)
				clan.Perms = new array<int>;
			while (clan.Perms.Count() < SM_ClanAction.COUNT)
				clan.Perms.Insert(defaultPerms[clan.Perms.Count()]);
		}

		clan.Perms.Set(action, minRank);
		AddLog(clan, GetActorName(player, clan) + "#STR_SMP_00048" + SM_ClanAction.GetName(action) + "#STR_SMP_00221" + m_Config.GetRankName(minRank) + "#STR_SMP_00222");
		SaveDB();
		SyncClanState(clan);
	}

	protected void HandleTreasuryWithdraw(PlayerBase player, int amount)
	{
		if (!m_Config.Treasury.Enabled)
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
			return;

		if (!HasActionAccess(player, clan, SM_ClanAction.TREASURY_WITHDRAW, "#STR_SMP_00482", "#STR_SMP_00693"))
			return;

		if (amount <= 0)
			return;
		if (amount > clan.Treasury)
			amount = clan.Treasury;
		if (amount <= 0)
		{
			Notify(player, "#STR_SMP_00482", "#STR_SMP_00491");
			return;
		}

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (currency.Count() == 0)
		{
			Notify(player, "#STR_SMP_00482", "#STR_SMP_00295");
			return;
		}

		array<ref SM_CurrencyItem> sorted = new array<ref SM_CurrencyItem>;
		foreach (SM_CurrencyItem entry : currency)
		{
			int insertAt = sorted.Count();
			for (int s = 0; s < sorted.Count(); s++)
			{
				if (entry.Value > sorted[s].Value)
				{
					insertAt = s;
					break;
				}
			}
			sorted.InsertAt(entry, insertAt);
		}

		int remaining = amount;
		int issued = 0;
		int spawnedItems = 0;

		foreach (SM_CurrencyItem denom : sorted)
		{
			int needItems = remaining / denom.Value;
			while (needItems > 0 && spawnedItems < MAX_WITHDRAW_ITEMS)
			{
				ItemBase spawned = SpawnCurrencyItem(player, denom.Classname);
				if (!spawned)
					break;
				spawnedItems++;

				int put = 1;
				// Стак валюты: проверяем IsSplitable() (конфиг canBeSplit), а НЕ
				// CanBeSplit() — последний дополнительно требует текущее кол-во > 1,
				// поэтому на только что заспавненном предмете (кол-во 1) всегда
				// возвращает false, и валюта выдавалась бы по 1 штуке без стака.
				if (spawned.IsSplitable())
				{
					int maxQuantity = spawned.GetQuantityMax();
					if (maxQuantity < 1)
						maxQuantity = 1;
					put = Math.Min(needItems, maxQuantity);
					spawned.SetQuantity(put);
				}

				needItems -= put;
				remaining -= put * denom.Value;
				issued += put * denom.Value;
			}
		}

		if (issued <= 0)
		{
			Notify(player, "#STR_SMP_00482", "#STR_SMP_00676");
			return;
		}

		clan.Treasury -= issued;
		AddLog(clan, GetActorName(player, clan) + "#STR_SMP_00085" + issued + "#STR_SMP_00014" + clan.Treasury + ")");
		SaveDB();

		if (issued < amount)
			Notify(player, "#STR_SMP_00482", "#STR_SMP_00370" + issued + "#STR_SMP_00044" + amount + "#STR_SMP_00020");
		else
			Notify(player, "#STR_SMP_00482", "#STR_SMP_00370" + issued + "#STR_SMP_00164" + clan.Treasury);
		SyncClanState(clan);
	}

	protected ItemBase SpawnCurrencyItem(PlayerBase player, string classname)
	{
		ItemBase item = ItemBase.Cast(player.GetInventory().CreateInInventory(classname));
		if (!item)
			item = ItemBase.Cast(GetGame().CreateObjectEx(classname, player.GetPosition(), ECE_PLACE_ON_SURFACE));
		return item;
	}

	protected void RewardTick()
	{
		if (!m_Config.Tops.RewardsEnabled)
			return;

		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);

		if (m_Config.Tops.RewardSchedule == "monthly")
		{
			if (m_DB.LastRewardYear == 0)
			{
				m_DB.LastRewardYear = year;
				m_DB.LastRewardMonth = month;
				SaveDB();
				return;
			}

			if (year != m_DB.LastRewardYear || month != m_DB.LastRewardMonth)
			{
				PayoutTopRewards("#STR_SMP_00787");
				m_DB.LastRewardYear = year;
				m_DB.LastRewardMonth = month;
				SaveDB();
			}
			return;
		}

		EnsureTopRewardSeasonStarted(true);
		int stampSeconds = GetNowAbsSeconds();
		if (stampSeconds - m_DB.LastRewardStampSeconds >= m_Config.Tops.RewardIntervalHours * 3600)
		{
			PayoutTopRewards("#STR_SMP_00788");
			m_DB.LastRewardStampSeconds = stampSeconds;
			m_DB.LastRewardStampMinutes = stampSeconds / 60;
			SaveDB();
		}
	}

	protected int AbsMinutes(int year, int month, int day, int hour, int minute)
	{
		int a = (14 - month) / 12;
		int y = year + 4800 - a;
		int m = month + 12 * a - 3;
		int jdn = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
		return (jdn - 2451545) * 1440 + hour * 60 + minute;
	}

	protected int AbsSeconds(int year, int month, int day, int hour, int minute, int second)
	{
		return AbsMinutes(year, month, day, hour, minute) * 60 + second;
	}

	protected float GetTopRewardValue(SM_ClanStats stats, int category)
	{
		if (!stats)
			return 0;

		if (category == SM_TopCategory.ACCURACY || category == SM_TopCategory.HEADSHOTS)
		{
			if (stats.ShotsFired < m_Config.Tops.MinShotsForAccuracyTop)
				return 0;
			if (category == SM_TopCategory.ACCURACY)
				return stats.HitsLanded * 10000.0 / stats.ShotsFired;
			return stats.Headshots * 10000.0 / stats.ShotsFired;
		}

		if (category == SM_TopCategory.KILL_DEATH)
		{
			if (stats.PlayerKills <= 0)
				return 0;
			if (stats.PlayerDeaths <= 0)
				return stats.PlayerKills * 10000.0;
			return stats.PlayerKills * 10000.0 / stats.PlayerDeaths;
		}

		return stats.GetValue(category);
	}

	protected SM_TopRewardContainerConfig PickTopRewardContainer()
	{
		if (!m_Config.Tops.RewardLootContainers)
			return null;

		array<ref SM_TopRewardContainerConfig> candidates = new array<ref SM_TopRewardContainerConfig>;
		foreach (SM_TopRewardContainerConfig container : m_Config.Tops.RewardLootContainers)
		{
			if (!container || container.ClassName == "")
				continue;
			if (container.SpawnChance <= 0.0)
				continue;
			if (Math.RandomFloat01() <= container.SpawnChance)
				candidates.Insert(container);
		}

		if (candidates.Count() == 0)
			return null;

		int idx = Math.RandomInt(0, candidates.Count());
		return candidates[idx];
	}

	protected SM_MarketItem CreateTopRewardLootItem(string className, int quantity)
	{
		SM_MarketItem item = new SM_MarketItem();
		item.ClassName = className;
		item.Health = 1;
		item.Quantity = quantity;
		item.LiquidType = -1;
		item.AmmoCount = -1;
		return item;
	}

	protected SM_MarketItem BuildTopRewardContainer(SM_TopRewardContainerConfig container)
	{
		if (!container)
			return null;

		SM_MarketItem reward = CreateTopRewardLootItem(container.ClassName, -1);
		if (!container.Loot)
			return reward;

		foreach (SM_TopRewardLootItemConfig lootItem : container.Loot)
		{
			if (!lootItem || lootItem.ClassName == "")
				continue;
			if (lootItem.SpawnChance <= 0.0)
				continue;
			if (Math.RandomFloat01() > lootItem.SpawnChance)
				continue;

			int quantity = Math.RandomIntInclusive(lootItem.MinQuantity, lootItem.MaxQuantity);
			reward.Cargo.Insert(CreateTopRewardLootItem(lootItem.ClassName, quantity));
		}

		return reward;
	}

	protected bool AddTopRewardContainerToStorage(SM_Clan clan)
	{
		if (!clan || !m_Config.Tops.RewardLootEnabled)
			return false;
		if (!m_Config.Storage.Enabled)
			return false;

		SM_TopRewardContainerConfig container = PickTopRewardContainer();
		if (!container)
			return false;

		PrepareStorageDB();
		int containerCount = Math.RandomIntInclusive(container.MinQuantity, container.MaxQuantity);
		bool added = false;

		for (int i = 0; i < containerCount; i++)
		{
			SM_MarketItem rewardItem = BuildTopRewardContainer(container);
			if (!rewardItem)
				continue;

			SM_StorageItem stored = new SM_StorageItem();
			stored.Item = rewardItem;
			stored.Id = m_StorageDB.NextItemId;
			m_StorageDB.NextItemId++;
			stored.ClanName = clan.Name;
			stored.DepositorUid = "SERVER_TOP_REWARD";
			stored.DepositorName = "#STR_SMP_01005";
			stored.Date = MakeDateStamp();

			m_StorageDB.Items.Insert(stored);
			added = true;
		}

		if (!added)
			return false;
		SaveStorageDBForClan(clan.Name);
		BroadcastStorage(clan);
		return true;
	}

	protected void PayoutTopRewards(string periodLabel)
	{
		int points = m_Config.Tops.RewardTreasuryPoints;
		bool rewardLoot = m_Config.Tops.RewardLootEnabled;

		for (int category = 0; category < SM_TopCategory.COUNT; category++)
		{
			if (!m_Config.IsTopCategoryEnabled(category))
				continue;

			SM_Clan best = null;
			float bestValue = 0;

			foreach (SM_Clan clan : m_DB.Clans)
			{
				if (!clan.Stats)
					continue;

				float value = GetTopRewardValue(clan.Stats, category);

				if (value > bestValue)
				{
					bestValue = value;
					best = clan;
				}
			}

			if (!best)
				continue;
			if (points <= 0 && !rewardLoot)
				continue;

			string categoryName = SM_TopCategory.GetName(category);
			string rewardText = "";
			if (points > 0)
			{
				best.Treasury += points;
				rewardText = "+" + points + "#STR_SMP_00027";
			}

			bool containerAdded = AddTopRewardContainerToStorage(best);
			if (containerAdded)
			{
				if (rewardText != "")
					rewardText = rewardText + "#STR_SMP_00156";
				else
					rewardText = "#STR_SMP_00574";
			}

			if (rewardText == "")
				rewardText = "#STR_SMP_00273";

			AddLog(best, "#STR_SMP_00657" + periodLabel + "#STR_SMP_00170" + categoryName + "» " + rewardText);
			NotifyClan(best, "#STR_SMP_01001", "#STR_SMP_00304" + categoryName + "»: " + rewardText);
			Print(SM_PartyLoc.Text("#STR_SMP_00177" + periodLabel + "#STR_SMP_00169" + best.Name + "#STR_SMP_00131" + categoryName + "»"));
		}

		if (m_Config.Tops.ResetStatsAfterReward)
		{
			foreach (SM_Clan resetClan : m_DB.Clans)
			{
				resetClan.Stats = new SM_ClanStats();
				AddLog(resetClan, "#STR_SMP_00908");
			}
		}

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_Clan syncClan : m_DB.Clans)
		{
			foreach (SM_ClanMember member : syncClan.Members)
			{
				PlayerBase pb;
				if (onlineMap.Find(member.Uid, pb))
					SyncStateTo(pb);
			}
		}
	}

	protected void RemoveApplicationsEverywhere(string uid)
	{
		foreach (SM_Clan clan : m_DB.Clans)
		{
			for (int i = clan.Applications.Count() - 1; i >= 0; i--)
			{
				if (clan.Applications[i].Uid == uid)
					clan.Applications.Remove(i);
			}
		}
	}

	void SyncStateTo(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan realClan = FindClanByMember(uid);
		SM_Clan adminClan = GetAdminViewedClan(uid);
		SM_Clan clan = realClan;
		bool isAdmin = IsClanAdmin(uid);
		bool isAdminMode = false;
		if (adminClan)
		{
			clan = adminClan;
			isAdminMode = true;
		}

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);

		ScriptRPC rpc = new ScriptRPC();

		bool hasClan = (clan != null);
		rpc.Write(hasClan);
		rpc.Write(uid);

		int myRank = 0;
		string clanName = "";
		string clanTag = "";
		int clanColor = 0;
		string clanDesc = "";
		if (clan)
		{
			clanName = clan.Name;
			clanTag = clan.Tag;
			clanColor = clan.Color;
			clanDesc = clan.Description;
			myRank = GetActorRank(uid, clan);
			if (myRank < 0)
				myRank = 0;
		}
		rpc.Write(clanName);
		rpc.Write(clanTag);
		rpc.Write(clanColor);
		rpc.Write(clanDesc);
		rpc.Write(myRank);

		if (clan)
		{
			rpc.Write(clan.Members.Count());
			foreach (SM_ClanMember member : clan.Members)
			{
				PlayerBase pb;
				bool online = onlineMap.Find(member.Uid, pb);
				float health = 0;
				if (online)
					health = pb.GetHealth01("GlobalHealth", "Health");
				rpc.Write(member.Uid);
				rpc.Write(member.Name);
				rpc.Write(member.Rank);
				rpc.Write(online);
				rpc.Write(health);
				rpc.Write(member.LastSeen);
				string playerTitle = "";
				int playerXP = 0;
				int playerTitleColor = 0;
				if (m_Config.PlayerExperience && m_Config.PlayerExperience.Enabled && m_Config.PlayerExperience.ShowInClanList)
				{
					playerXP = GetPlayerExperienceXP(member.Uid);
					playerTitle = GetPlayerTitleByXP(playerXP);
					playerTitleColor = GetPlayerTitleColorByXP(playerXP);
				}
				rpc.Write(playerTitle);
				rpc.Write(playerXP);
				rpc.Write(playerTitleColor);
			}
		}
		else
		{
			rpc.Write(0);
		}

		array<ref SM_Invite> invites;
		if (m_Invites.Find(uid, invites) && invites.Count() > 0)
		{
			rpc.Write(invites.Count());
			foreach (SM_Invite invite : invites)
			{
				rpc.Write(invite.ClanName);
				rpc.Write(invite.InviterName);
			}
		}
		else
		{
			rpc.Write(0);
		}

		if (clan)
		{
			rpc.Write(clan.Log.Count());
			foreach (string logLine : clan.Log)
				rpc.Write(logLine);
		}
		else
		{
			rpc.Write(0);
		}

		if (clan)
		{
			rpc.Write(clan.Applications.Count());
			foreach (SM_ClanApplication app : clan.Applications)
			{
				rpc.Write(app.Uid);
				rpc.Write(app.Name);
				rpc.Write(app.Date);
			}
		}
		else
		{
			rpc.Write(0);
		}

		rpc.Write(m_Config.Ranks.RankNames.Count());
		foreach (string rankName : m_Config.Ranks.RankNames)
			rpc.Write(rankName);

		rpc.Write(m_Config.Ranks.MinRankToInvite);
		rpc.Write(m_Config.Ranks.MinRankToKick);
		rpc.Write(m_Config.MemberHud.Enabled);
		rpc.Write(m_Config.General.MinClanNameLength);
		rpc.Write(m_Config.General.MaxClanNameLength);
		rpc.Write(m_Config.General.MaxClanTagLength);
		rpc.Write(m_Config.General.MaxClanDescriptionLength);
		rpc.Write(m_Config.General.CreateClanLogoPaa);
		rpc.Write(m_Config.ChatSystem.Enabled);
		rpc.Write(m_Config.ChatSystem.GetDirectColor());
		rpc.Write(m_Config.ChatSystem.GetDirectPlayerColor());
		rpc.Write(m_Config.ChatSystem.GetGlobalColor());
		rpc.Write(m_Config.ChatSystem.GetGlobalPlayerColor());
		rpc.Write(m_Config.ChatSystem.GetServerColor());
		rpc.Write(m_Config.ChatSystem.GetAlertColor());
		rpc.Write(m_Config.Pings.Enabled);

		rpc.Write(m_Config.Tops.Enabled);
		rpc.Write(m_Config.Treasury.Enabled);
		int treasury = 0;
		if (clan)
			treasury = clan.Treasury;
		rpc.Write(treasury);

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		rpc.Write(currency.Count());
		foreach (SM_CurrencyItem currencyEntry : currency)
		{
			rpc.Write(currencyEntry.Classname);
			rpc.Write(currencyEntry.Value);
		}

		rpc.Write(m_Config.Market.Enabled);
		rpc.Write(m_Config.Market.FeePercent);
		rpc.Write(m_Config.Market.ListingFeePercent);
		rpc.Write(m_Config.Market.MaxLotsPerClan);
		rpc.Write(m_Config.Market.MinRankToSell);
		rpc.Write(m_Config.ServerMarket.Enabled);
		rpc.Write(m_Config.Auction.Enabled);
		rpc.Write(m_Config.Auction.ListingFee);
		rpc.Write(m_Config.Auction.ListingFeePercent);
		rpc.Write(m_Config.Auction.SaleTaxPercent);
		rpc.Write(m_Config.Auction.MinDurationMinutes);
		rpc.Write(m_Config.Auction.MaxDurationMinutes);
		rpc.Write(m_Config.Auction.DefaultDurationMinutes);
		rpc.Write(m_Config.Auction.MinBidStep);
		rpc.Write(m_Config.Auction.MaxStartPrice);
		rpc.Write(m_Config.AchievementSettings.Enabled);
		rpc.Write(m_Config.AchievementSettings.ClaimRewardManually);
		bool playerExperiencePublicTab = false;
		bool playerExperienceClanList = false;
		if (m_Config.PlayerExperience && m_Config.PlayerExperience.Enabled)
		{
			playerExperiencePublicTab = m_Config.PlayerExperience.ShowPublicTab;
			playerExperienceClanList = m_Config.PlayerExperience.ShowInClanList;
		}
		rpc.Write(playerExperiencePublicTab);
		rpc.Write(playerExperienceClanList);
		rpc.Write(m_Config.ClanMap.Enabled);
		rpc.Write(m_Config.ClanMap.MaxMarkersPerClan);
		rpc.Write(m_Config.ClanMap.MarkerMaxNameLength);

		int serverMarkerCount = 0;
		if (m_Config.ClanMap.ServerMarkers)
		{
			foreach (SM_ServerMapMarker serverMarkerCountEntry : m_Config.ClanMap.ServerMarkers)
			{
				if (serverMarkerCountEntry && serverMarkerCountEntry.Enabled)
					serverMarkerCount++;
			}
		}
		bool sendBaseServerMarker = ShouldSendBaseServerMarker(clan);
		if (sendBaseServerMarker)
			serverMarkerCount++;
		serverMarkerCount = serverMarkerCount + CountExternalServerMapMarkers();

		rpc.Write(serverMarkerCount);
		if (m_Config.ClanMap.ServerMarkers)
		{
			foreach (SM_ServerMapMarker serverMarker : m_Config.ClanMap.ServerMarkers)
			{
				if (!serverMarker || !serverMarker.Enabled)
					continue;

				WriteServerMapMarker(rpc, serverMarker.Name, serverMarker.Position, serverMarker.GetColor(SM_ClanColors.GetColor(0)), serverMarker.GetMapIconPath());
			}
		}
		WriteExternalServerMapMarkers(rpc);
		if (sendBaseServerMarker)
		{
			string baseMarkerName = "#STR_SMP_00263";
			if (!ClanBaseFlagExists(clan))
				baseMarkerName = "#STR_SMP_00264";
			WriteServerMapMarker(rpc, baseMarkerName, clan.BasePos, clan.Color, "\\dz\\gear\\navigation\\data\\map_camp_ca.paa");
		}

		WriteAdminBaseMapMarkers(rpc, isAdmin);

		if (clan && clan.MapMarkers)
		{
			rpc.Write(CountClanMapMarkersForUid(clan, uid));
			foreach (SM_ClanMapMarker marker : clan.MapMarkers)
			{
				if (!ShouldSendClanMapMarkerToUid(marker, uid))
					continue;

				rpc.Write(marker.Id);
				rpc.Write(marker.Name);
				rpc.Write(marker.AuthorUid);
				rpc.Write(marker.AuthorName);
				rpc.Write(marker.Position);
				rpc.Write(marker.Color);
				rpc.Write(marker.Icon);
			}
		}
		else
		{
			rpc.Write(0);
		}

		int clanLevel = 1;
		int storageCap = 0;
		int lotCap = m_Config.Market.MaxLotsPerClan;
		int storageUsed = 0;
		if (clan)
		{
			clanLevel = clan.Level;
			storageCap = GetClanStorageCap(clan);
			lotCap = GetClanLotCap(clan);
			storageUsed = CountClanStorage(clan.Name);
		}
		rpc.Write(m_Config.Levels.Enabled);
		rpc.Write(m_Config.Storage.Enabled);
		rpc.Write(clanLevel);
		rpc.Write(storageCap);
		rpc.Write(lotCap);
		rpc.Write(storageUsed);

		bool hasBase = false;
		vector basePos = "0 0 0";
		if (clan)
		{
			hasBase = clan.HasBase;
			basePos = clan.BasePos;
		}
		rpc.Write(hasBase);
		rpc.Write(basePos);
		rpc.Write(m_Config.Storage.RadiusMeters);
		rpc.Write(m_Config.ClanMap.BaseRadiusEnabled);
		rpc.Write(m_Config.ClanMap.BaseRadiusMeters);

		int levelCount = m_Config.GetMaxLevel();
		rpc.Write(levelCount);
		for (int li = 1; li <= levelCount; li++)
		{
			SM_ClanLevel levelData = m_Config.GetLevelData(li);
			rpc.Write(levelData.UpgradeCost);
			rpc.Write(levelData.StorageSlots);
			rpc.Write(levelData.MarketLots);
			rpc.Write(levelData.ServerMarketItems);
		}

		rpc.Write(SM_ClanAction.COUNT);
		for (int pa = 0; pa < SM_ClanAction.COUNT; pa++)
		{
			int permRank = 0;
			if (clan && clan.Perms && pa < clan.Perms.Count())
				permRank = clan.Perms[pa];
			rpc.Write(permRank);
		}

		bool localChatEnabled = false;
		bool globalChatEnabled = false;
		if (m_Config.ChatSystem.Enabled)
		{
			localChatEnabled = m_Config.ChatSystem.LocalEnabled;
			globalChatEnabled = m_Config.ChatSystem.GlobalEnabled;
		}
		rpc.Write(localChatEnabled);
		rpc.Write(globalChatEnabled);

		rpc.Write(isAdmin);
		rpc.Write(isAdminMode);
		bool webhookEnabled = false;
		bool webhookConfigured = false;
		if (clan)
		{
			webhookEnabled = clan.DiscordWebhookEnabled;
			if (clan.DiscordWebhookUrl != "")
				webhookConfigured = true;
		}
		rpc.Write(webhookEnabled);
		rpc.Write(webhookConfigured);
		bool webhookAuditEnabled = false;
		if (clan && clan.DiscordAuditEnabled)
			webhookAuditEnabled = true;
		rpc.Write(webhookAuditEnabled);
		rpc.Write(m_ServerSessionKey);

		rpc.Send(player, SM_PartyRPC.SYNC_STATE, true, player.GetIdentity());
		SyncChatMuteStatus(player);
	}

	void SyncClanState(SM_Clan clan)
	{
		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_ClanMember member : clan.Members)
		{
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
				SyncStateTo(pb);
		}
		for (int i = 0; i < m_AdminClanViews.Count(); i++)
		{
			string adminUid = m_AdminClanViews.GetKey(i);
			string viewedClanName = m_AdminClanViews.GetElement(i);
			if (viewedClanName != clan.Name)
				continue;

			PlayerBase adminPlayer;
			if (onlineMap.Find(adminUid, adminPlayer))
				SyncStateTo(adminPlayer);
		}
	}

	void SyncAdminStates()
	{
		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		for (int i = 0; i < onlineMap.Count(); i++)
		{
			string uid = onlineMap.GetKey(i);
			if (!IsClanAdmin(uid))
				continue;

			PlayerBase adminPlayer = onlineMap.GetElement(i);
			if (adminPlayer)
				SyncStateTo(adminPlayer);
		}
	}

	void SendClanList(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		if (!m_Config.General.ShowClanListToEveryone && !FindClanByMember(uid) && !IsClanAdmin(uid))
		{
			ScriptRPC empty = new ScriptRPC();
			empty.Write(0);
			empty.Send(player, SM_PartyRPC.SYNC_CLAN_LIST, true, player.GetIdentity());
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_DB.Clans.Count());
		foreach (SM_Clan clan : m_DB.Clans)
		{
			string leaderName = "";
			string leaderTitle = "";
			int leaderTitleColor = 0;
			SM_ClanMember leader = clan.FindMemberByRank(m_Config.GetLeaderRank());
			if (leader)
			{
				leaderName = leader.Name;
				if (m_Config.PlayerExperience && m_Config.PlayerExperience.Enabled)
				{
					int leaderXP = GetPlayerExperienceXP(leader.Uid);
					leaderTitle = GetPlayerTitleByXP(leaderXP);
					leaderTitleColor = GetPlayerTitleColorByXP(leaderXP);
				}
			}
			rpc.Write(clan.Name);
			rpc.Write(clan.Tag);
			rpc.Write(clan.Members.Count());
			rpc.Write(leaderName);
			rpc.Write(leaderTitle);
			rpc.Write(leaderTitleColor);
		}
		rpc.Send(player, SM_PartyRPC.SYNC_CLAN_LIST, true, player.GetIdentity());
	}

	void SendInvitablePlayers(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();

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
			if (otherUid == uid)
				continue;
			if (FindClanByMember(otherUid))
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
		rpc.Send(player, SM_PartyRPC.SYNC_PLAYERS, true, player.GetIdentity());
	}

	void SendOnlinePlayers(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		array<ref SM_OnlinePlayerView> views = new array<ref SM_OnlinePlayerView>;
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;

			string uid = pb.GetIdentity().GetPlainId();
			string name = pb.GetIdentity().GetName();
			string clanName = "";
			string title = "";
			int titleColor = 0;

			SM_Clan clan = FindClanByMember(uid);
			if (clan)
				clanName = clan.Name;

			if (m_Config && m_Config.PlayerExperience && m_Config.PlayerExperience.Enabled)
			{
				int xp = GetPlayerExperienceXP(uid);
				title = GetPlayerTitleByXP(xp);
				titleColor = GetPlayerTitleColorByXP(xp);
			}

			views.Insert(new SM_OnlinePlayerView(uid, name, clanName, title, titleColor));
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(views.Count());
		foreach (SM_OnlinePlayerView view : views)
		{
			if (!view)
				view = new SM_OnlinePlayerView();
			rpc.Write(view.Uid);
			rpc.Write(view.Name);
			rpc.Write(view.ClanName);
			rpc.Write(view.Title);
			rpc.Write(view.TitleColor);
		}
		rpc.Send(player, SM_PartyRPC.SYNC_ONLINE_PLAYERS, true, player.GetIdentity());
	}

	protected int GetHudStatus(string uid, PlayerBase player)
	{
		int status = 0;
		if (player)
		{
			if (player.IsUnconscious())
				status |= SM_MemberStatus.UNCONSCIOUS;
			if (player.GetBleedingSourceCount() > 0)
				status |= SM_MemberStatus.BLEEDING;
			if (player.IsInVehicle())
				status |= SM_MemberStatus.IN_VEHICLE;
		}

		int activityStatus;
		if (m_HudActivityStatuses.Find(uid, activityStatus))
			status |= activityStatus;

		return status;
	}

	protected void WriteHudMember(ScriptRPC rpc, string uid, string name, float health, int status, vector position)
	{
		rpc.Write(uid);
		rpc.Write(name);
		rpc.Write(health);
		rpc.Write(status);
		rpc.Write(position);
	}

	protected void SendHudForClan(SM_Clan clan, map<string, PlayerBase> onlineMap)
	{
		if (!clan)
			return;

		array<string> uids = new array<string>;
		array<string> names = new array<string>;
		array<float> healths = new array<float>;
		array<int> statuses = new array<int>;
		array<vector> positions = new array<vector>;
		array<PlayerBase> receivers = new array<PlayerBase>;

		foreach (SM_ClanMember member : clan.Members)
		{
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
			{
				uids.Insert(member.Uid);
				names.Insert(member.Name);
				healths.Insert(pb.GetHealth01("GlobalHealth", "Health"));
				statuses.Insert(GetHudStatus(member.Uid, pb));
				positions.Insert(pb.GetPosition());
				receivers.Insert(pb);
			}
		}

		if (receivers.Count() < 1)
			return;

		for (int r = 0; r < receivers.Count(); r++)
		{
			PlayerBase receiver = receivers[r];
			if (!receiver || !receiver.GetIdentity())
				continue;

			int visibleCount = receivers.Count();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(visibleCount);

			if (visibleCount > 0)
			{
				for (int i = 0; i < uids.Count(); i++)
					WriteHudMember(rpc, uids[i], names[i], healths[i], statuses[i], positions[i]);
			}

			rpc.Send(receiver, SM_PartyRPC.HUD_UPDATE, false, receiver.GetIdentity());
		}
	}

	protected void HandleHudActivityStatus(PlayerBase player, int status)
	{
		if (!player || !player.GetIdentity())
			return;

		status = status & (SM_MemberStatus.TYPING_CHAT | SM_MemberStatus.VIEWING_MAP);

		string uid = player.GetIdentity().GetPlainId();
		if (status == 0)
			m_HudActivityStatuses.Remove(uid);
		else
			m_HudActivityStatuses.Set(uid, status);

		if (!m_Config.MemberHud.Enabled)
			return;

		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
			return;

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		SendHudForClan(clan, onlineMap);
	}

	protected void HudTick()
	{
		if (!m_Config.MemberHud.Enabled)
			return;

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);

		foreach (SM_Clan clan : m_DB.Clans)
			SendHudForClan(clan, onlineMap);
	}

	protected void InviteTick()
	{
		array<string> emptyKeys = new array<string>;

		for (int k = 0; k < m_Invites.Count(); k++)
		{
			string key = m_Invites.GetKey(k);
			array<ref SM_Invite> invites = m_Invites.GetElement(k);

			for (int i = invites.Count() - 1; i >= 0; i--)
			{
				invites[i].SecondsLeft -= 10;
				if (invites[i].SecondsLeft <= 0)
					invites.Remove(i);
			}

			if (invites.Count() == 0)
				emptyKeys.Insert(key);
		}

		foreach (string emptyKey : emptyKeys)
			m_Invites.Remove(emptyKey);
	}

	void OnPlayerReady(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		GetOrCreateAchievementRecord(uid, player.GetIdentity().GetName());
		GetOrCreatePlayerExperienceRecord(uid, player.GetIdentity().GetName());
		GivePendingReturns(player, uid);
		GiveStoragePendingReturns(player, uid);
		GiveAuctionPending(player, uid);
		GiveContractsPending(player, uid);
		SM_Clan clan = FindClanByMember(uid);
		if (clan)
		{
			SM_ClanMember member = clan.FindMember(uid);
			if (!member)
				return;

			bool changed = false;
			if (member.Name != player.GetIdentity().GetName())
			{
				member.Name = player.GetIdentity().GetName();
				changed = true;
			}
			string lastSeen = MakeDateStamp();
			if (member.LastSeen != lastSeen)
			{
				member.LastSeen = lastSeen;
				changed = true;
			}
			if (changed)
				SaveDB();
			SyncClanState(clan);
		}
		else
		{
			SyncStateTo(player);
		}
		SendPendingPersonalDeathMapMarker(player);
		QueueCheckPlayerRestrictedItems(player);
		BroadcastPlayerTitles();
		BroadcastContracts();
	}

	void QueueCheckPlayerRestrictedItems(PlayerBase player)
	{
		if (!GetGame().IsServer())
			return;
		if (!player || !player.GetIdentity())
			return;

		if (!m_RestrictedItemCheckQueued)
			m_RestrictedItemCheckQueued = new map<string, bool>;

		string uid = player.GetIdentity().GetPlainId();
		if (m_RestrictedItemCheckQueued.Contains(uid))
			return;

		m_RestrictedItemCheckQueued.Set(uid, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.CheckPlayerRestrictedItems, 250, false, player);
	}

	void CheckPlayerRestrictedItems(PlayerBase player)
	{
		if (!GetGame().IsServer())
			return;
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		if (m_RestrictedItemCheckQueued)
			m_RestrictedItemCheckQueued.Remove(uid);

		if (!IsPlayerExperienceEnabled())
			return;
		if (!m_Config.PlayerExperience.RestrictedItemsEnabled)
			return;
		if (!m_Config.PlayerExperience.RestrictedItems || m_Config.PlayerExperience.RestrictedItems.Count() == 0)
			return;

		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		for (int i = 0; i < items.Count(); i++)
		{
			EntityAI item = items[i];
			if (!item)
				continue;
			if (item == player)
				continue;
			if (item.GetHierarchyRootPlayer() != player)
				continue;

			string requiredTitle;
			if (IsItemForbiddenByPlayerTitle(player, item, requiredTitle))
				DropRestrictedItem(player, item, requiredTitle);
		}
	}

	protected void CheckOnlineRestrictedItems()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (player)
				QueueCheckPlayerRestrictedItems(player);
		}
	}

	protected bool IsRestrictedItemRuleMatch(SM_PlayerExperienceRestrictedItemConfig rule, EntityAI item)
	{
		if (!rule || !item)
			return false;
		if (rule.ClassName == "")
			return false;
		if (rule.ClassName == "REPLACE_WITH_ITEM_CLASSNAME")
			return false;
		if (item.GetType() == rule.ClassName)
			return true;
		if (rule.MatchInherited && item.IsKindOf(rule.ClassName))
			return true;
		return false;
	}

	protected SM_PlayerExperienceRestrictedItemConfig FindRestrictedItemRule(EntityAI item)
	{
		if (!m_Config || !m_Config.PlayerExperience || !m_Config.PlayerExperience.RestrictedItems)
			return null;

		foreach (SM_PlayerExperienceRestrictedItemConfig rule : m_Config.PlayerExperience.RestrictedItems)
		{
			if (IsRestrictedItemRuleMatch(rule, item))
				return rule;
		}
		return null;
	}

	protected int GetPlayerTitleRankIndexByName(string title)
	{
		if (!m_Config || !m_Config.PlayerExperience || !m_Config.PlayerExperience.Ranks)
			return -1;

		string searched = title.Trim();
		searched.ToLower();
		if (searched == "")
			return -1;

		for (int i = 0; i < m_Config.PlayerExperience.Ranks.Count(); i++)
		{
			SM_PlayerTitleRankConfig rank = m_Config.PlayerExperience.Ranks[i];
			if (!rank)
				continue;

			string current = rank.Name;
			current.ToLower();
			if (current == searched)
				return i;
		}
		return -1;
	}

	protected bool IsItemForbiddenByPlayerTitle(PlayerBase player, EntityAI item, out string requiredTitle)
	{
		requiredTitle = "";
		if (!player || !player.GetIdentity() || !item)
			return false;

		SM_PlayerExperienceRestrictedItemConfig rule = FindRestrictedItemRule(item);
		if (!rule)
			return false;

		int requiredIndex = GetPlayerTitleRankIndexByName(rule.RequiredTitle);
		if (requiredIndex < 0)
			return false;

		int currentXP = GetPlayerExperienceXP(player.GetIdentity().GetPlainId());
		int currentIndex = GetPlayerTitleRankIndexByXP(currentXP);
		if (currentIndex >= requiredIndex)
			return false;

		SM_PlayerTitleRankConfig requiredRank = m_Config.PlayerExperience.Ranks[requiredIndex];
		if (requiredRank && requiredRank.Name != "")
			requiredTitle = requiredRank.Name;
		else
			requiredTitle = rule.RequiredTitle;
		return true;
	}

	protected void DropRestrictedItem(PlayerBase player, EntityAI item, string requiredTitle)
	{
		if (!player || !player.GetIdentity() || !item)
			return;
		if (item.GetHierarchyRootPlayer() != player)
			return;

		string itemType = item.GetType();
		bool dropped = player.ServerDropEntity(item);
		if (!dropped)
			dropped = player.GetInventory().DropEntity(InventoryMode.SERVER, player, item);
		if (!dropped)
			return;

		Notify(player, "#STR_SMP_00457", itemType + "#STR_SMP_00041" + requiredTitle);
	}

	void OnPlayerDisconnected(string uid)
	{
		m_AdminClanViews.Remove(uid);
		m_HudActivityStatuses.Remove(uid);
		m_LastAchievementPositions.Remove(uid);
		if (m_RestrictedItemCheckQueued)
			m_RestrictedItemCheckQueued.Remove(uid);
		if (m_PlayerExperienceOnlineSeconds)
			m_PlayerExperienceOnlineSeconds.Remove(uid);
		SM_Clan clan = FindClanByMember(uid);
		if (clan)
			SyncClanState(clan);
		BroadcastPlayerTitles();
		BroadcastContracts();
	}

	static const string PLAYER_EXPERIENCE_DIR = "$profile:SM_PartyMod\\PlayerExperience";
	static const string PLAYER_EXPERIENCE_PATH = "$profile:SM_PartyMod\\PlayerExperience\\Players.json";

	protected bool IsPlayerExperienceEnabled()
	{
		if (!m_Config)
			return false;
		if (!m_Config.PlayerExperience)
			return false;
		return m_Config.PlayerExperience.Enabled;
	}

	protected void LoadPlayerExperienceDB()
	{
		if (!FileExist(PLAYER_EXPERIENCE_DIR))
			MakeDirectory(PLAYER_EXPERIENCE_DIR);

		m_PlayerExperienceDB = new SM_PlayerExperienceDB();
		if (FileExist(PLAYER_EXPERIENCE_PATH))
		{
			string errorMessage;
			if (!JsonFileLoader<SM_PlayerExperienceDB>.LoadFile(PLAYER_EXPERIENCE_PATH, m_PlayerExperienceDB, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00205" + errorMessage));
				m_PlayerExperienceDB = new SM_PlayerExperienceDB();
			}
		}
		PreparePlayerExperienceDB();
	}

	protected void PreparePlayerExperienceDB()
	{
		if (!m_PlayerExperienceDB)
			m_PlayerExperienceDB = new SM_PlayerExperienceDB();
		if (!m_PlayerExperienceDB.Players)
			m_PlayerExperienceDB.Players = new array<ref SM_PlayerExperienceRecord>;

		for (int i = m_PlayerExperienceDB.Players.Count() - 1; i >= 0; i--)
		{
			SM_PlayerExperienceRecord record = m_PlayerExperienceDB.Players[i];
			if (!record || record.Uid == "")
			{
				m_PlayerExperienceDB.Players.Remove(i);
				continue;
			}
			record.Name = record.Name.Trim();
			if (record.XP < 0)
				record.XP = 0;
			if (record.ZombieKills < 0)
				record.ZombieKills = 0;
			if (record.PlayerKills < 0)
				record.PlayerKills = 0;
			if (record.OnlineSeconds < 0)
				record.OnlineSeconds = 0;
		}
	}

	protected void SavePlayerExperienceDB()
	{
		if (!FileExist(PLAYER_EXPERIENCE_DIR))
			MakeDirectory(PLAYER_EXPERIENCE_DIR);

		string errorMessage;
		if (!JsonFileLoader<SM_PlayerExperienceDB>.SaveFile(PLAYER_EXPERIENCE_PATH, m_PlayerExperienceDB, errorMessage))
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00189" + errorMessage));
	}

	protected SM_PlayerExperienceRecord FindPlayerExperienceRecord(string uid)
	{
		if (!m_PlayerExperienceDB || !m_PlayerExperienceDB.Players)
			return null;

		foreach (SM_PlayerExperienceRecord record : m_PlayerExperienceDB.Players)
		{
			if (record && record.Uid == uid)
				return record;
		}
		return null;
	}

	protected SM_PlayerExperienceRecord GetOrCreatePlayerExperienceRecord(string uid, string name)
	{
		if (uid == "")
			return null;
		if (!m_PlayerExperienceDB)
			m_PlayerExperienceDB = new SM_PlayerExperienceDB();
		if (!m_PlayerExperienceDB.Players)
			m_PlayerExperienceDB.Players = new array<ref SM_PlayerExperienceRecord>;

		SM_PlayerExperienceRecord record = FindPlayerExperienceRecord(uid);
		if (!record)
		{
			record = new SM_PlayerExperienceRecord(uid, name);
			m_PlayerExperienceDB.Players.Insert(record);
			m_PlayerExperienceDirty = true;
		}
		if (name != "" && record.Name != name)
		{
			record.Name = name;
			m_PlayerExperienceDirty = true;
		}
		return record;
	}

	protected int GetPlayerTitleRankIndexByXP(int xp)
	{
		if (!m_Config || !m_Config.PlayerExperience || !m_Config.PlayerExperience.Ranks || m_Config.PlayerExperience.Ranks.Count() == 0)
			return 0;

		int rankIndex = 0;
		for (int i = 0; i < m_Config.PlayerExperience.Ranks.Count(); i++)
		{
			SM_PlayerTitleRankConfig rank = m_Config.PlayerExperience.Ranks[i];
			if (!rank)
				continue;
			if (xp >= rank.RequiredXP)
				rankIndex = i;
		}
		return rankIndex;
	}

	protected SM_PlayerTitleRankConfig GetPlayerTitleRankByXP(int xp)
	{
		if (!m_Config || !m_Config.PlayerExperience || !m_Config.PlayerExperience.Ranks || m_Config.PlayerExperience.Ranks.Count() == 0)
			return null;

		int rankIndex = GetPlayerTitleRankIndexByXP(xp);
		if (rankIndex < 0 || rankIndex >= m_Config.PlayerExperience.Ranks.Count())
			return null;
		return m_Config.PlayerExperience.Ranks[rankIndex];
	}

	protected string GetPlayerTitleByXP(int xp)
	{
		SM_PlayerTitleRankConfig rank = GetPlayerTitleRankByXP(xp);
		if (rank && rank.Name != "")
			return rank.Name;
		return "#STR_SMP_00709";
	}

	protected int GetFallbackPlayerTitleColor(int rankIndex)
	{
		switch (rankIndex)
		{
			case 0: return ARGB(255, 190, 196, 205);
			case 1: return ARGB(255, 156, 204, 172);
			case 2: return ARGB(255, 122, 206, 194);
			case 3: return ARGB(255, 110, 170, 235);
			case 4: return ARGB(255, 166, 146, 238);
			case 5: return ARGB(255, 225, 188, 92);
			case 6: return ARGB(255, 236, 142, 82);
			case 7: return ARGB(255, 236, 103, 103);
			case 8: return ARGB(255, 242, 118, 180);
			case 9: return ARGB(255, 248, 222, 125);
		}
		return ARGB(255, 248, 242, 220);
	}

	protected int GetPlayerTitleColorByXP(int xp)
	{
		int rankIndex = GetPlayerTitleRankIndexByXP(xp);
		SM_PlayerTitleRankConfig rank = GetPlayerTitleRankByXP(xp);
		int fallback = GetFallbackPlayerTitleColor(rankIndex);
		if (rank)
			return rank.GetColor(fallback);
		return fallback;
	}

	protected int GetPlayerNextTitleXP(int xp)
	{
		if (!m_Config || !m_Config.PlayerExperience || !m_Config.PlayerExperience.Ranks)
			return 0;

		foreach (SM_PlayerTitleRankConfig rank : m_Config.PlayerExperience.Ranks)
		{
			if (rank && rank.RequiredXP > xp)
				return rank.RequiredXP;
		}
		return 0;
	}

	protected int GetPlayerExperienceXP(string uid)
	{
		SM_PlayerExperienceRecord record = FindPlayerExperienceRecord(uid);
		if (!record)
			return 0;
		return record.XP;
	}

	protected string GetPlayerExperienceTitle(string uid)
	{
		return GetPlayerTitleByXP(GetPlayerExperienceXP(uid));
	}

	protected int GetPlayerExperienceTitleColor(string uid)
	{
		return GetPlayerTitleColorByXP(GetPlayerExperienceXP(uid));
	}

	protected string GetPlayerExperienceClanName(string uid)
	{
		if (!m_DB || !m_DB.Clans)
			return "";

		foreach (SM_Clan clan : m_DB.Clans)
		{
			if (!clan || !clan.Members)
				continue;
			if (clan.FindMember(uid))
				return clan.Name;
		}
		return "";
	}

	protected int GetZombieExperience(Object zombie)
	{
		if (!m_Config || !m_Config.PlayerExperience)
			return 0;

		int xp = m_Config.PlayerExperience.ZombieKillXP;
		if (!zombie || !m_Config.PlayerExperience.ZombieClassRewards)
			return xp;

		foreach (SM_PlayerExperienceZombieRewardConfig reward : m_Config.PlayerExperience.ZombieClassRewards)
		{
			if (!reward || reward.ClassName == "")
				continue;
			if (IsZombieRewardMatch(zombie, reward))
				xp = reward.XP;
		}
		return xp;
	}

	protected bool IsSameClassName(string left, string right)
	{
		left = left.Trim();
		right = right.Trim();
		if (left == "" || right == "")
			return false;
		if (left == right)
			return true;

		string leftLower = left;
		string rightLower = right;
		leftLower.ToLower();
		rightLower.ToLower();
		if (leftLower == rightLower)
			return true;
		return false;
	}

	protected bool IsCfgVehicleKindOf(string className, string baseClassName)
	{
		className = className.Trim();
		baseClassName = baseClassName.Trim();
		if (className == "" || baseClassName == "")
			return false;
		if (IsSameClassName(className, baseClassName))
			return true;
		if (!GetGame())
			return false;

		string currentClass = className;
		string path = "CfgVehicles " + currentClass;
		for (int i = 0; i < 64; i++)
		{
			string parentClass;
			if (!GetGame().ConfigGetBaseName(path, parentClass))
				return false;

			parentClass = parentClass.Trim();
			if (parentClass == "")
				return false;
			if (IsSameClassName(parentClass, baseClassName))
				return true;
			if (IsSameClassName(parentClass, currentClass))
				return false;

			currentClass = parentClass;
			path = "CfgVehicles " + currentClass;
		}

		return false;
	}

	protected bool IsZombieRewardMatch(Object zombie, SM_PlayerExperienceZombieRewardConfig reward)
	{
		if (!zombie || !reward)
			return false;

		string rewardClass = reward.ClassName;
		rewardClass = rewardClass.Trim();
		if (rewardClass == "")
			return false;

		string zombieClass = zombie.GetType();
		if (IsSameClassName(zombieClass, rewardClass))
			return true;
		if (!reward.MatchInherited)
			return false;
		if (zombie.IsKindOf(rewardClass))
			return true;
		if (IsCfgVehicleKindOf(zombieClass, rewardClass))
			return true;
		return false;
	}

	protected bool IsConfiguredZombieRewardTarget(Object zombie)
	{
		if (!zombie || !m_Config || !m_Config.PlayerExperience || !m_Config.PlayerExperience.ZombieClassRewards)
			return false;

		foreach (SM_PlayerExperienceZombieRewardConfig reward : m_Config.PlayerExperience.ZombieClassRewards)
		{
			if (IsZombieRewardMatch(zombie, reward))
				return true;
		}
		return false;
	}

	bool IsZombieStatTarget(Object zombie)
	{
		if (!zombie)
			return false;
		if (zombie.IsKindOf("ZombieBase"))
			return true;
		if (IsCfgVehicleKindOf(zombie.GetType(), "ZombieBase"))
			return true;
		if (IsConfiguredZombieRewardTarget(zombie))
			return true;
		return false;
	}

	protected void AwardPlayerExperience(PlayerBase player, int amount, string reason, int zombieKills = 0, int playerKills = 0, int onlineSeconds = 0)
	{
		if (!IsPlayerExperienceEnabled())
			return;
		if (!player || !player.GetIdentity())
			return;
		if (amount <= 0 && zombieKills <= 0 && playerKills <= 0 && onlineSeconds <= 0)
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_PlayerExperienceRecord record = GetOrCreatePlayerExperienceRecord(uid, player.GetIdentity().GetName());
		if (!record)
			return;

		int oldRank = GetPlayerTitleRankIndexByXP(record.XP);
		record.XP += amount;
		record.ZombieKills += zombieKills;
		record.PlayerKills += playerKills;
		record.OnlineSeconds += onlineSeconds;
		if (record.XP < 0)
			record.XP = 0;
		m_PlayerExperienceDirty = true;

		int newRank = GetPlayerTitleRankIndexByXP(record.XP);
		if (m_Config.PlayerExperience.ShowNotifications && amount > 0)
		{
			if (newRank > oldRank)
				Notify(player, "#STR_SMP_00457", "#STR_SMP_00711" + GetPlayerTitleByXP(record.XP));
			else
				Notify(player, "#STR_SMP_00738", "+" + amount.ToString() + " XP: " + reason);
		}

		SendPlayerTitles(player);
		SM_Clan clan = FindClanByMember(uid);
		if (clan)
			SyncClanState(clan);
		if (newRank > oldRank)
			BroadcastPlayerTitles();
	}

	protected void AwardOnlineExperience(map<string, PlayerBase> onlineMap)
	{
		if (!IsPlayerExperienceEnabled())
			return;
		if (!m_Config.PlayerExperience)
			return;
		if (m_Config.PlayerExperience.OnlineRewardXP <= 0)
			return;

		int interval = m_Config.PlayerExperience.OnlineRewardIntervalSeconds;
		if (interval < STATS_TICK_SECONDS)
			interval = STATS_TICK_SECONDS;

		for (int i = 0; i < onlineMap.Count(); i++)
		{
			string uid = onlineMap.GetKey(i);
			PlayerBase player = onlineMap.GetElement(i);
			if (!player || !player.GetIdentity() || !player.IsAlive())
				continue;

			int seconds = 0;
			m_PlayerExperienceOnlineSeconds.Find(uid, seconds);
			seconds += STATS_TICK_SECONDS;
			if (seconds >= interval)
			{
				seconds = seconds - interval;
				AwardPlayerExperience(player, m_Config.PlayerExperience.OnlineRewardXP, "#STR_SMP_00726", 0, 0, interval);
			}
			m_PlayerExperienceOnlineSeconds.Set(uid, seconds);
		}
	}

	protected void BuildPlayerTitleViews(out array<ref SM_PlayerTitleView> views)
	{
		views = new array<ref SM_PlayerTitleView>;
		if (!m_PlayerExperienceDB || !m_PlayerExperienceDB.Players)
			return;

		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);

		foreach (SM_PlayerExperienceRecord record : m_PlayerExperienceDB.Players)
		{
			if (!record || record.Uid == "")
				continue;

			bool online = false;
			PlayerBase pb;
			if (onlineMap.Find(record.Uid, pb))
				online = true;

			string title = GetPlayerTitleByXP(record.XP);
			int rankIndex = GetPlayerTitleRankIndexByXP(record.XP);
			int nextXP = GetPlayerNextTitleXP(record.XP);
			int color = GetPlayerTitleColorByXP(record.XP);
			string clanName = GetPlayerExperienceClanName(record.Uid);
			views.Insert(new SM_PlayerTitleView(record.Uid, record.Name, title, record.XP, rankIndex, nextXP, color, clanName, online));
		}

		for (int a = 0; a < views.Count(); a++)
		{
			for (int b = a + 1; b < views.Count(); b++)
			{
				SM_PlayerTitleView left = views[a];
				SM_PlayerTitleView right = views[b];
				bool swap = false;
				if (left && right)
				{
					if (right.RankIndex > left.RankIndex)
						swap = true;
					else if (right.RankIndex == left.RankIndex && right.XP > left.XP)
						swap = true;
				}
				if (swap)
				{
					views.Set(a, right);
					views.Set(b, left);
				}
			}
		}

		int maxPlayers = 200;
		if (m_Config && m_Config.PlayerExperience)
			maxPlayers = m_Config.PlayerExperience.MaxPublicPlayers;
		while (views.Count() > maxPlayers)
			views.Remove(views.Count() - 1);
	}

	void SendPlayerTitles(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		bool enabled = false;
		if (m_Config && m_Config.PlayerExperience && m_Config.PlayerExperience.Enabled && m_Config.PlayerExperience.ShowPublicTab)
			enabled = true;

		string uid = player.GetIdentity().GetPlainId();
		SM_PlayerExperienceRecord myRecord = GetOrCreatePlayerExperienceRecord(uid, player.GetIdentity().GetName());
		int myXP = 0;
		if (myRecord)
			myXP = myRecord.XP;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(enabled);
		rpc.Write(GetPlayerTitleByXP(myXP));
		rpc.Write(myXP);
		rpc.Write(GetPlayerTitleRankIndexByXP(myXP));
		rpc.Write(GetPlayerNextTitleXP(myXP));
		rpc.Write(GetPlayerTitleColorByXP(myXP));

		if (!enabled)
		{
			rpc.Write(0);
			rpc.Send(player, SM_PartyRPC.SYNC_PLAYER_TITLES, true, player.GetIdentity());
			return;
		}

		array<ref SM_PlayerTitleView> views;
		BuildPlayerTitleViews(views);
		rpc.Write(views.Count());
		foreach (SM_PlayerTitleView view : views)
		{
			rpc.Write(view.Uid);
			rpc.Write(view.Name);
			rpc.Write(view.Title);
			rpc.Write(view.XP);
			rpc.Write(view.RankIndex);
			rpc.Write(view.NextXP);
			rpc.Write(view.Color);
			rpc.Write(view.ClanName);
			rpc.Write(view.Online);
		}
		rpc.Send(player, SM_PartyRPC.SYNC_PLAYER_TITLES, true, player.GetIdentity());
	}

	protected void BroadcastPlayerTitles()
	{
		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		for (int i = 0; i < onlineMap.Count(); i++)
		{
			PlayerBase player = onlineMap.GetElement(i);
			if (player)
				SendPlayerTitles(player);
		}
	}

	static const string ACHIEVEMENTS_DIR = "$profile:SM_PartyMod\\Achievements";
	static const string ACHIEVEMENTS_PATH = "$profile:SM_PartyMod\\Achievements\\Players.json";

	protected void LoadAchievementsDB()
	{
		if (!FileExist(ACHIEVEMENTS_DIR))
			MakeDirectory(ACHIEVEMENTS_DIR);

		m_AchievementsDB = new SM_AchievementsDB();
		if (FileExist(ACHIEVEMENTS_PATH))
		{
			string errorMessage;
			if (!JsonFileLoader<SM_AchievementsDB>.LoadFile(ACHIEVEMENTS_PATH, m_AchievementsDB, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00198" + errorMessage));
				m_AchievementsDB = new SM_AchievementsDB();
			}
		}
		PrepareAchievementsDB();
	}

	protected void PrepareAchievementsDB()
	{
		if (!m_AchievementsDB)
			m_AchievementsDB = new SM_AchievementsDB();
		if (!m_AchievementsDB.Players)
			m_AchievementsDB.Players = new array<ref SM_PlayerAchievementRecord>;

		for (int i = m_AchievementsDB.Players.Count() - 1; i >= 0; i--)
		{
			SM_PlayerAchievementRecord record = m_AchievementsDB.Players[i];
			if (!record || record.Uid == "")
			{
				m_AchievementsDB.Players.Remove(i);
				continue;
			}
			PrepareAchievementRecord(record);
		}
	}

	protected void PrepareAchievementRecord(SM_PlayerAchievementRecord record)
	{
		if (!record)
			return;
		if (!record.Stats)
			record.Stats = new SM_AchievementStats();
		if (!record.ClaimedAchievements)
			record.ClaimedAchievements = new array<string>;
		if (!record.NotifiedAchievements)
			record.NotifiedAchievements = new array<string>;
	}

	protected void SaveAchievementsDB()
	{
		if (!m_Config.AchievementSettings.Enabled)
			return;
		if (!FileExist(ACHIEVEMENTS_DIR))
			MakeDirectory(ACHIEVEMENTS_DIR);

		string errorMessage;
		if (!JsonFileLoader<SM_AchievementsDB>.SaveFile(ACHIEVEMENTS_PATH, m_AchievementsDB, errorMessage))
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00182" + errorMessage));
	}

	protected SM_PlayerAchievementRecord FindAchievementRecord(string uid)
	{
		if (!m_AchievementsDB || !m_AchievementsDB.Players)
			return null;

		foreach (SM_PlayerAchievementRecord record : m_AchievementsDB.Players)
		{
			if (record && record.Uid == uid)
				return record;
		}
		return null;
	}

	protected SM_PlayerAchievementRecord GetOrCreateAchievementRecord(string uid, string name)
	{
		if (uid == "")
			return null;
		if (!m_AchievementsDB)
			m_AchievementsDB = new SM_AchievementsDB();
		if (!m_AchievementsDB.Players)
			m_AchievementsDB.Players = new array<ref SM_PlayerAchievementRecord>;

		SM_PlayerAchievementRecord record = FindAchievementRecord(uid);
		if (!record)
		{
			record = new SM_PlayerAchievementRecord(uid, name);
			m_AchievementsDB.Players.Insert(record);
			m_AchievementsDirty = true;
		}
		PrepareAchievementRecord(record);
		if (name != "" && record.Name != name)
		{
			record.Name = name;
			m_AchievementsDirty = true;
		}
		return record;
	}

	protected bool AchievementIdInList(array<string> list, string id)
	{
		if (!list)
			return false;
		if (list.Find(id) >= 0)
			return true;
		return false;
	}

	protected SM_AchievementConfig FindAchievementConfig(int scope, string id)
	{
		array<ref SM_AchievementConfig> list;
		if (scope == SM_AchievementScope.CLAN)
			list = m_Config.AchievementSettings.Clan;
		else
			list = m_Config.AchievementSettings.Personal;

		if (!list)
			return null;

		foreach (SM_AchievementConfig achievement : list)
		{
			if (achievement && achievement.Enabled && achievement.Id == id)
				return achievement;
		}
		return null;
	}

	protected int GetPersonalAchievementProgress(SM_AchievementStats stats, string type)
	{
		if (!stats)
			return 0;

		if (type == "PlayerKills") return stats.PlayerKills;
		if (type == "PlayerDeaths") return stats.PlayerDeaths;
		if (type == "ZombieKills") return stats.ZombieKills;
		if (type == "OnlineSeconds") return stats.OnlineSeconds;
		if (type == "BestLifeSeconds") return stats.BestLifeSeconds;
		if (type == "DistanceWalked") return stats.DistanceWalked;
		if (type == "ShotsFired") return stats.ShotsFired;
		if (type == "HitsLanded") return stats.HitsLanded;
		if (type == "Headshots") return stats.Headshots;
		if (type == "LongestKill") return stats.LongestKillDistance;
		if (type == "TreasuryDeposited") return stats.TreasuryDeposited;
		if (type == "MarketBuys") return stats.MarketBuys;
		if (type == "MarketSells") return stats.MarketSells;
		if (type == "StorageDeposits") return stats.StorageDeposits;
		if (type == "AuctionWins") return stats.AuctionWins;
		if (type == "AuctionSales") return stats.AuctionSales;
		if (type == "CreatedClan") return stats.CreatedClan;
		if (type == "JoinedClan") return stats.JoinedClan;
		return 0;
	}

	protected int GetClanAchievementProgress(SM_Clan clan, string type)
	{
		if (!clan || !clan.Stats)
			return 0;

		if (type == "PlayerKills") return clan.Stats.PlayerKills;
		if (type == "PlayerDeaths") return clan.Stats.PlayerDeaths;
		if (type == "ZombieKills") return clan.Stats.ZombieKills;
		if (type == "OnlineSeconds") return clan.Stats.OnlineSeconds;
		if (type == "BestLifeSeconds") return clan.Stats.BestLifeSeconds;
		if (type == "DistanceWalked") return clan.Stats.DistanceWalked;
		if (type == "ShotsFired") return clan.Stats.ShotsFired;
		if (type == "HitsLanded") return clan.Stats.HitsLanded;
		if (type == "Headshots") return clan.Stats.Headshots;
		if (type == "LongestKill") return clan.Stats.LongestPlayerKillDistance;
		if (type == "TreasuryDeposited") return clan.Stats.TreasuryDeposited;
		if (type == "MarketSells") return clan.Stats.MarketSells;
		if (type == "StorageDeposits") return clan.Stats.StorageDeposits;
		if (type == "AuctionSales") return clan.Stats.AuctionSales;
		if (type == "AuctionWins") return clan.Stats.AuctionWins;
		if (type == "Treasury") return clan.Treasury;
		if (type == "Level") return clan.Level;
		if (type == "Members") return clan.Members.Count();
		return 0;
	}

	protected void AddPersonalAchievementValue(string uid, string name, string type, int amount, PlayerBase onlinePlayer = null)
	{
		if (!m_Config.AchievementSettings.Enabled || uid == "" || amount == 0)
			return;

		SM_PlayerAchievementRecord record = GetOrCreateAchievementRecord(uid, name);
		if (!record)
			return;

		if (type == "PlayerKills") record.Stats.PlayerKills += amount;
		else if (type == "PlayerDeaths") record.Stats.PlayerDeaths += amount;
		else if (type == "ZombieKills") record.Stats.ZombieKills += amount;
		else if (type == "OnlineSeconds") record.Stats.OnlineSeconds += amount;
		else if (type == "DistanceWalked") record.Stats.DistanceWalked += amount;
		else if (type == "ShotsFired") record.Stats.ShotsFired += amount;
		else if (type == "HitsLanded") record.Stats.HitsLanded += amount;
		else if (type == "Headshots") record.Stats.Headshots += amount;
		else if (type == "TreasuryDeposited") record.Stats.TreasuryDeposited += amount;
		else if (type == "MarketBuys") record.Stats.MarketBuys += amount;
		else if (type == "MarketSells") record.Stats.MarketSells += amount;
		else if (type == "StorageDeposits") record.Stats.StorageDeposits += amount;
		else if (type == "AuctionWins") record.Stats.AuctionWins += amount;
		else if (type == "AuctionSales") record.Stats.AuctionSales += amount;
		else if (type == "CreatedClan") record.Stats.CreatedClan += amount;
		else if (type == "JoinedClan") record.Stats.JoinedClan += amount;
		else return;

		m_AchievementsDirty = true;
		if (onlinePlayer)
			CheckPlayerAchievementNotifications(onlinePlayer, record);
	}

	protected void SetPersonalAchievementBest(string uid, string name, string type, int value, PlayerBase onlinePlayer = null)
	{
		if (!m_Config.AchievementSettings.Enabled || uid == "")
			return;

		SM_PlayerAchievementRecord record = GetOrCreateAchievementRecord(uid, name);
		if (!record)
			return;

		bool changed = false;
		if (type == "BestLifeSeconds")
		{
			if (value > record.Stats.BestLifeSeconds)
			{
				record.Stats.BestLifeSeconds = value;
				changed = true;
			}
		}
		else if (type == "LongestKill")
		{
			if (value > record.Stats.LongestKillDistance)
			{
				record.Stats.LongestKillDistance = value;
				changed = true;
			}
		}

		if (!changed)
			return;

		m_AchievementsDirty = true;
		if (onlinePlayer)
			CheckPlayerAchievementNotifications(onlinePlayer, record);
	}

	protected void CheckPlayerAchievementNotifications(PlayerBase player, SM_PlayerAchievementRecord record)
	{
		if (!m_Config.AchievementSettings.NotifyOnComplete || !player || !record)
			return;
		if (!m_Config.AchievementSettings.Personal)
			return;

		foreach (SM_AchievementConfig achievement : m_Config.AchievementSettings.Personal)
		{
			if (!achievement || !achievement.Enabled)
				continue;
			if (AchievementIdInList(record.NotifiedAchievements, achievement.Id))
				continue;
			if (GetPersonalAchievementProgress(record.Stats, achievement.Type) < achievement.Target)
				continue;

			record.NotifiedAchievements.Insert(achievement.Id);
			m_AchievementsDirty = true;
			Notify(player, "#STR_SMP_00421", "#STR_SMP_00378" + achievement.Name);
		}
	}

	protected void CheckClanAchievementNotifications(SM_Clan clan)
	{
		if (!m_Config.AchievementSettings.Enabled || !m_Config.AchievementSettings.NotifyOnComplete || !clan)
			return;
		if (!m_Config.AchievementSettings.Clan)
			return;

		foreach (SM_AchievementConfig achievement : m_Config.AchievementSettings.Clan)
		{
			if (!achievement || !achievement.Enabled)
				continue;
			if (AchievementIdInList(clan.NotifiedAchievements, achievement.Id))
				continue;
			if (GetClanAchievementProgress(clan, achievement.Type) < achievement.Target)
				continue;

			clan.NotifiedAchievements.Insert(achievement.Id);
			m_StatsDirty = true;
			NotifyClan(clan, "#STR_SMP_00424", "#STR_SMP_00378" + achievement.Name);
		}
	}

	protected void ApplyAchievementReward(PlayerBase player, SM_AchievementConfig achievement, SM_Clan clan)
	{
		if (!player || !achievement)
			return;

		if (achievement.RewardMoney > 0)
			IssueCurrency(player, achievement.RewardMoney);

		if (achievement.RewardClanTreasury > 0 && clan)
		{
			clan.Treasury += achievement.RewardClanTreasury;
			AddLog(clan, "#STR_SMP_00422" + achievement.Name + "\": +" + achievement.RewardClanTreasury + "#STR_SMP_00027");
			m_StatsDirty = true;
			SyncClanState(clan);
		}

		if (achievement.RewardContainer && achievement.RewardContainer.ClassName != "")
		{
			SM_MarketItem rewardItem = BuildTopRewardContainer(achievement.RewardContainer);
			if (rewardItem)
				RestoreItemTree(rewardItem, player, true);
		}
	}

	protected void HandleAchievementClaim(PlayerBase player, int scope, string achievementId)
	{
		if (!m_Config.AchievementSettings.Enabled || !player || !player.GetIdentity())
			return;

		SM_AchievementConfig achievement = FindAchievementConfig(scope, achievementId);
		if (!achievement)
		{
			Notify(player, "#STR_SMP_00426", "#STR_SMP_00425");
			SendAchievements(player);
			return;
		}

		string uid = player.GetIdentity().GetPlainId();
		if (scope == SM_AchievementScope.CLAN)
		{
			SM_Clan clan = FindClanByMember(uid);
			if (!clan)
			{
				Notify(player, "#STR_SMP_00426", "#STR_SMP_00348");
				return;
			}
			if (AchievementIdInList(clan.ClaimedAchievements, achievement.Id))
			{
				Notify(player, "#STR_SMP_00426", "#STR_SMP_00660");
				SendAchievements(player);
				return;
			}
			if (GetClanAchievementProgress(clan, achievement.Type) < achievement.Target)
			{
				Notify(player, "#STR_SMP_00426", "#STR_SMP_00423");
				SendAchievements(player);
				return;
			}

			clan.ClaimedAchievements.Insert(achievement.Id);
			ApplyAchievementReward(player, achievement, clan);
			AddLog(clan, player.GetIdentity().GetName() + "#STR_SMP_00074" + achievement.Name + "\"");
			SaveDB();
			Notify(player, "#STR_SMP_00426", "#STR_SMP_00659" + achievement.Name);
			SendAchievements(player);
			return;
		}

		SM_PlayerAchievementRecord record = GetOrCreateAchievementRecord(uid, player.GetIdentity().GetName());
		if (!record)
			return;
		if (AchievementIdInList(record.ClaimedAchievements, achievement.Id))
		{
			Notify(player, "#STR_SMP_00426", "#STR_SMP_00660");
			SendAchievements(player);
			return;
		}
		if (GetPersonalAchievementProgress(record.Stats, achievement.Type) < achievement.Target)
		{
			Notify(player, "#STR_SMP_00426", "#STR_SMP_00423");
			SendAchievements(player);
			return;
		}

		record.ClaimedAchievements.Insert(achievement.Id);
		m_AchievementsDirty = true;
		ApplyAchievementReward(player, achievement, FindClanByMember(uid));
		SaveAchievementsDB();
		Notify(player, "#STR_SMP_00426", "#STR_SMP_00659" + achievement.Name);
		SendAchievements(player);
	}

	protected void WriteAchievementView(ScriptRPC rpc, int scope, SM_AchievementConfig achievement, int progress, bool claimed)
	{
		bool completed = false;
		if (progress >= achievement.Target)
			completed = true;

		string name = achievement.Name;
		string description = achievement.Description;
		if (achievement.Hidden && !completed)
		{
			name = "#STR_SMP_00929";
			description = "#STR_SMP_01043";
		}

		string containerClass = "";
		if (achievement.RewardContainer)
			containerClass = achievement.RewardContainer.ClassName;

		rpc.Write(scope);
		rpc.Write(achievement.Id);
		rpc.Write(name);
		rpc.Write(description);
		rpc.Write(achievement.Type);
		rpc.Write(achievement.Target);
		rpc.Write(progress);
		rpc.Write(completed);
		rpc.Write(claimed);
		rpc.Write(achievement.Hidden);
		rpc.Write(achievement.RewardMoney);
		rpc.Write(achievement.RewardClanTreasury);
		rpc.Write(containerClass);
	}

	void SendAchievements(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_PlayerAchievementRecord record = GetOrCreateAchievementRecord(uid, player.GetIdentity().GetName());
		SM_Clan clan = FindClanByMember(uid);

		int count = 0;
		if (m_Config.AchievementSettings.Personal)
		{
			foreach (SM_AchievementConfig personalCount : m_Config.AchievementSettings.Personal)
			{
				if (personalCount && personalCount.Enabled)
					count++;
			}
		}
		if (clan && m_Config.AchievementSettings.Clan)
		{
			foreach (SM_AchievementConfig clanCount : m_Config.AchievementSettings.Clan)
			{
				if (clanCount && clanCount.Enabled)
					count++;
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Config.AchievementSettings.Enabled);
		rpc.Write(m_Config.AchievementSettings.ClaimRewardManually);
		rpc.Write(count);

		if (m_Config.AchievementSettings.Personal)
		{
			foreach (SM_AchievementConfig personal : m_Config.AchievementSettings.Personal)
			{
				if (!personal || !personal.Enabled)
					continue;
				int progress = GetPersonalAchievementProgress(record.Stats, personal.Type);
				bool claimed = AchievementIdInList(record.ClaimedAchievements, personal.Id);
				WriteAchievementView(rpc, SM_AchievementScope.PERSONAL, personal, progress, claimed);
			}
		}

		if (clan && m_Config.AchievementSettings.Clan)
		{
			foreach (SM_AchievementConfig clanAchievement : m_Config.AchievementSettings.Clan)
			{
				if (!clanAchievement || !clanAchievement.Enabled)
					continue;
				int clanProgress = GetClanAchievementProgress(clan, clanAchievement.Type);
				bool clanClaimed = AchievementIdInList(clan.ClaimedAchievements, clanAchievement.Id);
				WriteAchievementView(rpc, SM_AchievementScope.CLAN, clanAchievement, clanProgress, clanClaimed);
			}
		}

		rpc.Send(player, SM_PartyRPC.SYNC_ACHIEVEMENTS, true, player.GetIdentity());
	}

	static const string CONTRACTS_DIR = "$profile:SM_PartyMod\\Contracts";
	static const string CONTRACTS_STATE_PATH = "$profile:SM_PartyMod\\Contracts\\State.json";

	protected bool IsContractsEnabled()
	{
		if (!m_Config)
			return false;
		if (!m_Config.Contracts)
			return false;
		return m_Config.Contracts.Enabled;
	}

	protected void LoadContractsDB()
	{
		if (!FileExist(CONTRACTS_DIR))
			MakeDirectory(CONTRACTS_DIR);

		m_ContractsDB = new SM_ContractsDB();
		if (FileExist(CONTRACTS_STATE_PATH))
		{
			string errorMessage;
			if (!JsonFileLoader<SM_ContractsDB>.LoadFile(CONTRACTS_STATE_PATH, m_ContractsDB, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00202" + errorMessage));
				m_ContractsDB = new SM_ContractsDB();
			}
		}
		PrepareContractsDB();
	}

	protected void PrepareContractsDB()
	{
		if (!m_ContractsDB)
			m_ContractsDB = new SM_ContractsDB();
		if (!m_ContractsDB.Contracts)
			m_ContractsDB.Contracts = new array<ref SM_Contract>;
		if (!m_ContractsDB.PendingMoney)
			m_ContractsDB.PendingMoney = new array<ref SM_ContractPendingMoney>;
		if (!m_ContractsDB.PendingItems)
			m_ContractsDB.PendingItems = new array<ref SM_ContractPendingItem>;

		for (int i = m_ContractsDB.Contracts.Count() - 1; i >= 0; i--)
		{
			SM_Contract contract = m_ContractsDB.Contracts[i];
			if (!contract || contract.Id <= 0 || contract.CreatorUid == "")
			{
				m_ContractsDB.Contracts.Remove(i);
				continue;
			}
			if (contract.Id >= m_ContractsDB.NextContractId)
				m_ContractsDB.NextContractId = contract.Id + 1;
			if (contract.ItemQuantity < 1)
				contract.ItemQuantity = 1;
			if (contract.Reward < 0)
				contract.Reward = 0;
			if (contract.Status < SM_ContractStatus.OPEN || contract.Status > SM_ContractStatus.CANCELLED)
				contract.Status = SM_ContractStatus.OPEN;
		}

		for (int p = m_ContractsDB.PendingMoney.Count() - 1; p >= 0; p--)
		{
			SM_ContractPendingMoney pending = m_ContractsDB.PendingMoney[p];
			if (!pending || pending.OwnerUid == "" || pending.Amount <= 0)
				m_ContractsDB.PendingMoney.Remove(p);
		}

		for (int ip = m_ContractsDB.PendingItems.Count() - 1; ip >= 0; ip--)
		{
			SM_ContractPendingItem pendingItem = m_ContractsDB.PendingItems[ip];
			if (!pendingItem || pendingItem.OwnerUid == "" || !pendingItem.Item || pendingItem.Item.ClassName == "")
				m_ContractsDB.PendingItems.Remove(ip);
		}
	}

	protected void SaveContractsDB()
	{
		if (!FileExist(CONTRACTS_DIR))
			MakeDirectory(CONTRACTS_DIR);

		string errorMessage;
		if (!JsonFileLoader<SM_ContractsDB>.SaveFile(CONTRACTS_STATE_PATH, m_ContractsDB, errorMessage))
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00186" + errorMessage));
	}

	protected void RestartContractsTick()
	{
		if (!GetGame())
			return;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.ContractsTick);
		int tickSeconds = 30;
		if (m_Config && m_Config.Contracts)
			tickSeconds = m_Config.Contracts.TickSeconds;
		if (tickSeconds < 5)
			tickSeconds = 5;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ContractsTick, tickSeconds * 1000, true);
	}

	protected SM_Contract FindContract(int id)
	{
		if (!m_ContractsDB || !m_ContractsDB.Contracts)
			return null;
		foreach (SM_Contract contract : m_ContractsDB.Contracts)
		{
			if (contract && contract.Id == id)
				return contract;
		}
		return null;
	}

	protected bool CanCreateContractByTitle(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return false;
		if (!m_Config.Contracts)
			return false;

		int requiredIndex = GetPlayerTitleRankIndexByName(m_Config.Contracts.RequiredCreatorTitle);
		if (requiredIndex < 0)
			requiredIndex = 0;

		int currentXP = GetPlayerExperienceXP(player.GetIdentity().GetPlainId());
		int currentIndex = GetPlayerTitleRankIndexByXP(currentXP);
		if (currentIndex >= requiredIndex)
			return true;
		return false;
	}

	protected bool ConsumePlayerMoney(PlayerBase player, int amount, string title)
	{
		if (amount <= 0)
			return true;

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (currency.Count() == 0)
		{
			Notify(player, title, "#STR_SMP_00294");
			return false;
		}

		int total = GetPlayerCurrencyTotal(player, currency);
		if (total < amount)
		{
			Notify(player, title, "#STR_SMP_00681" + amount.ToString() + "#STR_SMP_00163" + total.ToString());
			return false;
		}

		int consumed = ConsumeAllCurrency(player, currency);

		int change = consumed - amount;
		if (change > 0)
			IssueCurrency(player, change);
		return true;
	}

	protected int NormalizeContractPrice(int price)
	{
		if (price < m_Config.Contracts.MinPrice)
			return m_Config.Contracts.MinPrice;
		if (price > m_Config.Contracts.MaxPrice)
			return m_Config.Contracts.MaxPrice;
		return price;
	}

	protected int NormalizeContractDuration(int minutes)
	{
		if (minutes < m_Config.Contracts.MinDurationMinutes)
			return m_Config.Contracts.MinDurationMinutes;
		if (minutes > m_Config.Contracts.MaxDurationMinutes)
			return m_Config.Contracts.MaxDurationMinutes;
		return minutes;
	}

	protected int CountCreatedOpenContracts(string uid)
	{
		int count = 0;
		if (!m_ContractsDB || !m_ContractsDB.Contracts)
			return count;
		foreach (SM_Contract contract : m_ContractsDB.Contracts)
		{
			if (!contract)
				continue;
			if (contract.CreatorUid != uid)
				continue;
			if (contract.Status == SM_ContractStatus.OPEN || contract.Status == SM_ContractStatus.ACCEPTED)
				count++;
		}
		return count;
	}

	protected int CountAcceptedContracts(string uid)
	{
		int count = 0;
		if (!m_ContractsDB || !m_ContractsDB.Contracts)
			return count;
		foreach (SM_Contract contract : m_ContractsDB.Contracts)
		{
			if (!contract)
				continue;
			if (contract.ExecutorUid == uid && contract.Status == SM_ContractStatus.ACCEPTED)
				count++;
		}
		return count;
	}

	protected bool IsContractTargetHiddenForViewer(SM_Contract contract, string viewerUid, string viewerClanName)
	{
		if (!contract)
			return true;
		if (contract.Type != SM_ContractType.KILL)
			return false;
		if (contract.TargetUid != "" && contract.TargetUid == viewerUid)
			return true;
		if (contract.TargetClanName != "" && contract.TargetClanName == viewerClanName)
			return true;
		return false;
	}

	protected bool IsContractVisibleInActiveList(SM_Contract contract, string viewerUid, string viewerClanName)
	{
		if (!contract)
			return false;
		if (contract.CreatorUid == viewerUid)
		{
			if (contract.Status == SM_ContractStatus.OPEN)
				return true;
			if (contract.Status == SM_ContractStatus.ACCEPTED)
				return true;
		}
		if (IsContractTargetHiddenForViewer(contract, viewerUid, viewerClanName))
			return false;
		if (contract.Status == SM_ContractStatus.OPEN)
			return true;
		if (contract.Status == SM_ContractStatus.ACCEPTED)
		{
			if (contract.CreatorUid == viewerUid)
				return true;
			if (contract.ExecutorUid == viewerUid)
				return true;
			if (contract.ExecutorClanName != "" && contract.ExecutorClanName == viewerClanName)
				return true;
		}
		return false;
	}

	protected bool IsContractVisibleInHistory(SM_Contract contract, string viewerUid, string viewerClanName)
	{
		if (!contract)
			return false;
		if (contract.Status == SM_ContractStatus.OPEN)
			return false;
		if (contract.CreatorUid == viewerUid)
			return true;
		if (contract.ExecutorUid == viewerUid)
			return true;
		if (contract.ExecutorClanName != "" && contract.ExecutorClanName == viewerClanName)
			return true;
		return false;
	}

	protected bool IsContractMarkerVisible(SM_Contract contract, string viewerUid, string viewerClanName)
	{
		if (!contract)
			return false;
		if (contract.Type != SM_ContractType.KILL)
			return false;
		if (contract.Status != SM_ContractStatus.ACCEPTED)
			return false;
		if (!contract.TargetOnline)
			return false;
		if (contract.LastMarkerAt <= 0)
			return false;
		if (contract.TargetUid != "" && contract.TargetUid == viewerUid)
			return false;
		if (contract.TargetClanName != "" && contract.TargetClanName == viewerClanName)
			return false;
		if (contract.ExecutorUid == viewerUid)
			return true;
		if (contract.ExecutorClanName != "" && contract.ExecutorClanName == viewerClanName)
			return true;
		return false;
	}

	protected string GetContractDate(SM_Contract contract)
	{
		if (!contract)
			return "";
		if (contract.CompletedDate != "")
			return contract.CompletedDate;
		if (contract.AcceptedDate != "")
			return contract.AcceptedDate;
		return contract.Date;
	}

	protected int GetContractSecondsLeft(SM_Contract contract)
	{
		if (!contract)
			return 0;
		if (contract.EndsAt <= 0)
			return 0;
		int seconds = contract.EndsAt - GetNowAbsSeconds();
		if (seconds < 0)
			seconds = 0;
		return seconds;
	}

	protected void WriteContractView(ScriptRPC rpc, SM_Contract contract, string viewerUid)
	{
		rpc.Write(contract.Id);
		rpc.Write(contract.Type);
		rpc.Write(contract.Status);
		rpc.Write(contract.CreatorName);
		rpc.Write(contract.CreatorClanName);
		rpc.Write(contract.TargetName);
		rpc.Write(contract.TargetClanName);
		rpc.Write(contract.ItemClassName);
		rpc.Write(contract.ItemDisplayName);
		rpc.Write(contract.ItemQuantity);
		rpc.Write(contract.Price);
		rpc.Write(contract.Reward);
		rpc.Write(contract.Fee);
		rpc.Write(contract.Penalty);
		rpc.Write(contract.ExecutorName);
		rpc.Write(contract.ExecutorClanName);
		rpc.Write(GetContractSecondsLeft(contract));
		rpc.Write(GetContractDate(contract));
		rpc.Write(contract.CreatorUid == viewerUid);
		rpc.Write(contract.ExecutorUid == viewerUid);
	}

	protected string GetContractViewerClanName(string uid)
	{
		SM_Clan clan = FindClanByMember(uid);
		if (clan)
			return clan.Name;
		return "";
	}

	protected SM_ContractItemConfig FindContractItemConfig(string className)
	{
		if (!m_Config || !m_Config.Contracts || !m_Config.Contracts.SearchItems)
			return null;
		foreach (SM_ContractItemConfig item : m_Config.Contracts.SearchItems)
		{
			if (!item)
				continue;
			if (item.ClassName == className)
				return item;
		}
		return null;
	}

	protected bool IsContractItemMatch(SM_ContractItemConfig rule, EntityAI item)
	{
		if (!rule || !item)
			return false;
		if (item.GetType() == rule.ClassName)
			return true;
		if (rule.MatchInherited && item.IsKindOf(rule.ClassName))
			return true;
		return false;
	}

	protected bool IsTargetAllowedForCreator(PlayerBase creator, PlayerBase target)
	{
		if (!creator || !target || !creator.GetIdentity() || !target.GetIdentity())
			return false;
		string creatorUid = creator.GetIdentity().GetPlainId();
		string targetUid = target.GetIdentity().GetPlainId();
		if (creatorUid == targetUid)
			return true;

		SM_Clan creatorClan = FindClanByMember(creatorUid);
		if (creatorClan && creatorClan.FindMember(targetUid))
			return false;
		return true;
	}

	protected void FillContractCommon(SM_Contract contract, PlayerBase creator, int type, int price, int durationMinutes)
	{
		string uid = creator.GetIdentity().GetPlainId();
		contract.Id = m_ContractsDB.NextContractId;
		m_ContractsDB.NextContractId++;
		contract.Type = type;
		contract.Status = SM_ContractStatus.OPEN;
		contract.CreatorUid = uid;
		contract.CreatorName = creator.GetIdentity().GetName();
		SM_Clan creatorClan = FindClanByMember(uid);
		if (creatorClan)
			contract.CreatorClanName = creatorClan.Name;
		contract.Price = price;
		contract.Fee = price * m_Config.Contracts.FeePercent / 100;
		contract.Reward = price - contract.Fee;
		if (contract.Reward < 0)
			contract.Reward = 0;
		contract.Penalty = price * m_Config.Contracts.AbandonPenaltyPercent / 100;
		contract.CreatedAt = GetNowAbsSeconds();
		contract.EndsAt = contract.CreatedAt + durationMinutes * 60;
		contract.Date = MakeDateStamp();
	}

	protected bool CheckContractCreateRequirements(PlayerBase player, int price)
	{
		if (!IsContractsEnabled())
			return false;
		if (!player || !player.GetIdentity())
			return false;
		if (!CanCreateContractByTitle(player))
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00961" + m_Config.Contracts.RequiredCreatorTitle);
			return false;
		}
		if (CountCreatedOpenContracts(player.GetIdentity().GetPlainId()) >= m_Config.Contracts.MaxActiveCreatedPerPlayer)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00607" + m_Config.Contracts.MaxActiveCreatedPerPlayer.ToString());
			return false;
		}
		if (price < m_Config.Contracts.MinPrice)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00644" + m_Config.Contracts.MinPrice.ToString());
			return false;
		}
		if (price > m_Config.Contracts.MaxPrice)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00626" + m_Config.Contracts.MaxPrice.ToString());
			return false;
		}
		return true;
	}

	protected void HandleContractCreateKill(PlayerBase player, string targetUid, int price, int durationMinutes)
	{
		durationMinutes = NormalizeContractDuration(durationMinutes);
		if (!CheckContractCreateRequirements(player, price))
			return;
		price = NormalizeContractPrice(price);

		PlayerBase target = FindOnlinePlayer(targetUid);
		if (!target || !target.GetIdentity())
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_01055");
			return;
		}
		if (!IsTargetAllowedForCreator(player, target))
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00700");
			return;
		}
		if (!ConsumePlayerMoney(player, price, "#STR_SMP_00583"))
			return;

		SM_Contract contract = new SM_Contract();
		FillContractCommon(contract, player, SM_ContractType.KILL, price, durationMinutes);
		contract.TargetUid = target.GetIdentity().GetPlainId();
		contract.TargetName = target.GetIdentity().GetName();
		SM_Clan targetClan = FindClanByMember(contract.TargetUid);
		if (targetClan)
			contract.TargetClanName = targetClan.Name;
		contract.TargetOnline = true;

		m_ContractsDB.Contracts.Insert(contract);
		SaveContractsDB();
		Notify(player, "#STR_SMP_00583", "#STR_SMP_00577" + contract.Fee.ToString());
		BroadcastContracts();
	}

	protected void HandleContractCreateItem(PlayerBase player, string className, int quantity, int price, int durationMinutes)
	{
		durationMinutes = NormalizeContractDuration(durationMinutes);
		if (quantity < 1)
			quantity = 1;
		if (!CheckContractCreateRequirements(player, price))
			return;
		price = NormalizeContractPrice(price);

		SM_ContractItemConfig itemConfig = FindContractItemConfig(className);
		if (!itemConfig)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_01076");
			return;
		}
		if (!ConsumePlayerMoney(player, price, "#STR_SMP_00583"))
			return;

		SM_Contract contract = new SM_Contract();
		FillContractCommon(contract, player, SM_ContractType.ITEM, price, durationMinutes);
		contract.ItemClassName = itemConfig.ClassName;
		contract.ItemDisplayName = itemConfig.DisplayName;
		contract.ItemQuantity = quantity;

		m_ContractsDB.Contracts.Insert(contract);
		SaveContractsDB();
		Notify(player, "#STR_SMP_00583", "#STR_SMP_00578" + contract.Fee.ToString());
		BroadcastContracts();
	}

	protected bool CanViewerAcceptContract(SM_Contract contract, PlayerBase player)
	{
		if (!contract || !player || !player.GetIdentity())
			return false;
		string uid = player.GetIdentity().GetPlainId();
		string viewerClanName = GetContractViewerClanName(uid);
		if (contract.Status != SM_ContractStatus.OPEN)
			return false;
		if (contract.CreatorUid == uid)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00699");
			return false;
		}
		if (IsContractTargetHiddenForViewer(contract, uid, viewerClanName))
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_01075");
			return false;
		}
		if (CountAcceptedContracts(uid) >= m_Config.Contracts.MaxActiveAcceptedPerPlayer)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00606" + m_Config.Contracts.MaxActiveAcceptedPerPlayer.ToString());
			return false;
		}
		return true;
	}

	protected void HandleContractAccept(PlayerBase player, int contractId)
	{
		if (!IsContractsEnabled())
			return;
		SM_Contract contract = FindContract(contractId);
		if (!CanViewerAcceptContract(contract, player))
			return;

		string uid = player.GetIdentity().GetPlainId();
		contract.Status = SM_ContractStatus.ACCEPTED;
		contract.ExecutorUid = uid;
		contract.ExecutorName = player.GetIdentity().GetName();
		SM_Clan executorClan = FindClanByMember(uid);
		if (executorClan)
			contract.ExecutorClanName = executorClan.Name;
		contract.AcceptedAt = GetNowAbsSeconds();
		contract.AcceptedDate = MakeDateStamp();
		if (contract.Type == SM_ContractType.KILL)
		{
			PlayerBase target = FindOnlinePlayer(contract.TargetUid);
			if (target && target.GetIdentity())
			{
				contract.TargetOnline = true;
				contract.LastTargetPos = target.GetPosition();
				contract.LastMarkerAt = contract.AcceptedAt;
			}
		}

		SaveContractsDB();
		Notify(player, "#STR_SMP_00583", "#STR_SMP_00581");
		BroadcastContracts();
	}

	protected void RefundContractRewardToCreator(SM_Contract contract, string reason)
	{
		if (!contract)
			return;
		if (contract.Reward <= 0)
			return;
		GiveContractMoney(contract.CreatorUid, contract.CreatorName, contract.Reward, reason);
	}

	protected void HandleContractAbandon(PlayerBase player, int contractId)
	{
		if (!IsContractsEnabled())
			return;
		SM_Contract contract = FindContract(contractId);
		if (!contract || !player || !player.GetIdentity())
			return;
		string uid = player.GetIdentity().GetPlainId();
		if (contract.Status != SM_ContractStatus.ACCEPTED || contract.ExecutorUid != uid)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00751");
			return;
		}
		if (!ConsumePlayerMoney(player, contract.Penalty, "#STR_SMP_00583"))
			return;

		contract.Status = SM_ContractStatus.ABANDONED;
		contract.CompletedAt = GetNowAbsSeconds();
		contract.CompletedDate = MakeDateStamp();
		RefundContractRewardToCreator(contract, "#STR_SMP_00332");

		SaveContractsDB();
		Notify(player, "#STR_SMP_00583", "#STR_SMP_00579" + contract.Penalty.ToString());
		BroadcastContracts();
	}

	protected int CountContractItems(PlayerBase player, SM_Contract contract, out array<EntityAI> matchingItems)
	{
		matchingItems = new array<EntityAI>;
		if (!player || !contract)
			return 0;
		SM_ContractItemConfig itemConfig = FindContractItemConfig(contract.ItemClassName);
		if (!itemConfig)
			return 0;

		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		for (int i = 0; i < items.Count(); i++)
		{
			EntityAI item = items[i];
			if (!item)
				continue;
			if (item == player)
				continue;
			if (item.GetHierarchyRootPlayer() != player)
				continue;
			if (IsContractItemMatch(itemConfig, item))
				matchingItems.Insert(item);
		}
		return matchingItems.Count();
	}

	protected bool IsContractItemNestedInSelection(EntityAI item, array<EntityAI> selectedItems)
	{
		if (!item || !selectedItems)
			return false;

		EntityAI parent = item.GetHierarchyParent();
		while (parent)
		{
			for (int i = 0; i < selectedItems.Count(); i++)
			{
				if (parent == selectedItems[i])
					return true;
			}
			parent = parent.GetHierarchyParent();
		}
		return false;
	}

	protected void QueueContractItem(string uid, string name, SM_MarketItem item, string reason)
	{
		if (uid == "" || !item)
			return;
		if (!m_ContractsDB)
			return;
		if (!m_ContractsDB.PendingItems)
			m_ContractsDB.PendingItems = new array<ref SM_ContractPendingItem>;

		SM_ContractPendingItem pending = new SM_ContractPendingItem();
		pending.OwnerUid = uid;
		pending.OwnerName = name;
		pending.Reason = reason;
		pending.Item = item;
		m_ContractsDB.PendingItems.Insert(pending);
	}

	protected void DeliverContractItemsToCreator(SM_Contract contract, array<ref SM_MarketItem> items)
	{
		if (!contract || !items || items.Count() == 0)
			return;

		string itemName = contract.ItemDisplayName;
		itemName = itemName.Trim();
		if (itemName == "")
			itemName = SM_PartyUtil.GetItemDisplayName(contract.ItemClassName);

		string reason = "#STR_SMP_00828" + itemName;
		PlayerBase creator = FindOnlinePlayer(contract.CreatorUid);
		int delivered = 0;

		for (int i = 0; i < items.Count(); i++)
		{
			SM_MarketItem item = items[i];
			if (!item)
				continue;

			if (creator)
			{
				EntityAI restored = RestoreItemTree(item, creator, true);
				if (restored)
				{
					delivered++;
				}
				else
				{
					QueueContractItem(contract.CreatorUid, contract.CreatorName, item, reason);
				}
			}
			else
			{
				QueueContractItem(contract.CreatorUid, contract.CreatorName, item, reason);
				delivered++;
			}
		}

		if (creator && delivered > 0)
			Notify(creator, "#STR_SMP_00583", "#STR_SMP_00808" + itemName + " x" + delivered.ToString());
	}

	protected void CompleteContractItem(SM_Contract contract, PlayerBase executor, array<ref SM_MarketItem> deliveredItems)
	{
		if (!contract || !executor || !executor.GetIdentity())
			return;

		contract.Status = SM_ContractStatus.COMPLETED;
		contract.CompletedAt = GetNowAbsSeconds();
		contract.CompletedDate = MakeDateStamp();
		contract.TargetOnline = false;

		DeliverContractItemsToCreator(contract, deliveredItems);
		GiveContractMoney(contract.ExecutorUid, contract.ExecutorName, contract.Reward, "#STR_SMP_00658");
		Notify(executor, "#STR_SMP_00583", "#STR_SMP_00580" + contract.Reward.ToString());
		SaveContractsDB();
	}

	protected void HandleContractTurnInItem(PlayerBase player, int contractId)
	{
		if (!IsContractsEnabled())
			return;
		SM_Contract contract = FindContract(contractId);
		if (!contract || !player || !player.GetIdentity())
			return;
		string uid = player.GetIdentity().GetPlainId();
		if (contract.Status != SM_ContractStatus.ACCEPTED || contract.ExecutorUid != uid)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00347");
			return;
		}
		if (contract.Type != SM_ContractType.ITEM)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_01074");
			return;
		}

		array<EntityAI> matchingItems;
		int count = CountContractItems(player, contract, matchingItems);
		if (count < contract.ItemQuantity)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00694" + count.ToString() + "/" + contract.ItemQuantity.ToString());
			return;
		}

		array<EntityAI> selectedItems = new array<EntityAI>;
		for (int i = 0; i < matchingItems.Count(); i++)
		{
			if (selectedItems.Count() >= contract.ItemQuantity)
				break;

			EntityAI selectedItem = matchingItems[i];
			if (!selectedItem)
				continue;
			if (IsContractItemNestedInSelection(selectedItem, selectedItems))
				continue;

			selectedItems.Insert(selectedItem);
		}

		if (selectedItems.Count() < contract.ItemQuantity)
		{
			Notify(player, "#STR_SMP_00583", "#STR_SMP_00694" + selectedItems.Count().ToString() + "/" + contract.ItemQuantity.ToString());
			return;
		}

		array<ref SM_MarketItem> deliveredItems = new array<ref SM_MarketItem>;
		for (int c = 0; c < selectedItems.Count(); c++)
		{
			int nested = 0;
			deliveredItems.Insert(CaptureItemTree(selectedItems[c], nested));
		}

		for (int d = 0; d < selectedItems.Count(); d++)
			GetGame().ObjectDelete(selectedItems[d]);

		CompleteContractItem(contract, player, deliveredItems);
		BroadcastContracts();
	}

	protected void CompleteContractKill(PlayerBase killerPlayer, PlayerBase victim)
	{
		if (!IsContractsEnabled())
			return;
		if (!killerPlayer || !victim || !killerPlayer.GetIdentity() || !victim.GetIdentity())
			return;
		if (!m_ContractsDB || !m_ContractsDB.Contracts)
			return;

		string killerUid = killerPlayer.GetIdentity().GetPlainId();
		string victimUid = victim.GetIdentity().GetPlainId();
		bool changed = false;
		for (int i = 0; i < m_ContractsDB.Contracts.Count(); i++)
		{
			SM_Contract contract = m_ContractsDB.Contracts[i];
			if (!contract)
				continue;
			if (contract.Type != SM_ContractType.KILL)
				continue;
			if (contract.Status != SM_ContractStatus.ACCEPTED)
				continue;
			if (contract.ExecutorUid != killerUid)
				continue;
			if (contract.TargetUid != victimUid)
				continue;

			CompleteContract(contract, killerPlayer, "#STR_SMP_00576");
			changed = true;
		}

		if (changed)
			BroadcastContracts();
	}

	protected void CompleteContract(SM_Contract contract, PlayerBase executor, string notifyText)
	{
		if (!contract || !executor || !executor.GetIdentity())
			return;
		contract.Status = SM_ContractStatus.COMPLETED;
		contract.CompletedAt = GetNowAbsSeconds();
		contract.CompletedDate = MakeDateStamp();
		contract.TargetOnline = false;

		GiveContractMoney(contract.ExecutorUid, contract.ExecutorName, contract.Reward, "#STR_SMP_00658");
		Notify(executor, "#STR_SMP_00583", notifyText + "#STR_SMP_00165" + contract.Reward.ToString());
		SaveContractsDB();
	}

	protected void GiveContractMoney(string uid, string name, int amount, string reason)
	{
		if (uid == "" || amount <= 0)
			return;

		PlayerBase player = FindOnlinePlayer(uid);
		if (player && player.GetIdentity())
		{
			IssueCurrency(player, amount);
			Notify(player, "#STR_SMP_00583", reason + ": " + amount.ToString());
			return;
		}

		SM_ContractPendingMoney pending = new SM_ContractPendingMoney(uid, name, amount, reason);
		m_ContractsDB.PendingMoney.Insert(pending);
		SaveContractsDB();
	}

	protected void GiveContractsPending(PlayerBase player, string uid)
	{
		if (!player || uid == "" || !m_ContractsDB)
			return;
		if (!m_ContractsDB.PendingMoney)
			m_ContractsDB.PendingMoney = new array<ref SM_ContractPendingMoney>;
		if (!m_ContractsDB.PendingItems)
			m_ContractsDB.PendingItems = new array<ref SM_ContractPendingItem>;

		bool changed = false;
		for (int itemIndex = m_ContractsDB.PendingItems.Count() - 1; itemIndex >= 0; itemIndex--)
		{
			SM_ContractPendingItem pendingItem = m_ContractsDB.PendingItems[itemIndex];
			if (!pendingItem || pendingItem.OwnerUid != uid)
				continue;
			if (!pendingItem.Item)
			{
				m_ContractsDB.PendingItems.Remove(itemIndex);
				changed = true;
				continue;
			}

			m_ContractsDB.PendingItems.Remove(itemIndex);
			RestoreItemTree(pendingItem.Item, player, true);
			Notify(player, "#STR_SMP_00583", pendingItem.Reason);
			changed = true;
		}

		for (int i = m_ContractsDB.PendingMoney.Count() - 1; i >= 0; i--)
		{
			SM_ContractPendingMoney pending = m_ContractsDB.PendingMoney[i];
			if (!pending || pending.OwnerUid != uid)
				continue;
			m_ContractsDB.PendingMoney.Remove(i);
			IssueCurrency(player, pending.Amount);
			Notify(player, "#STR_SMP_00583", pending.Reason + ": " + pending.Amount.ToString());
			changed = true;
		}

		if (changed)
			SaveContractsDB();
	}

	protected void ContractsTick()
	{
		if (!m_ContractsDB || !m_ContractsDB.Contracts)
			return;

		bool changed = false;
		int now = GetNowAbsSeconds();
		int markerInterval = 120;
		if (m_Config && m_Config.Contracts)
			markerInterval = m_Config.Contracts.TargetMarkerIntervalSeconds;

		for (int i = 0; i < m_ContractsDB.Contracts.Count(); i++)
		{
			SM_Contract contract = m_ContractsDB.Contracts[i];
			if (!contract)
				continue;

			if ((contract.Status == SM_ContractStatus.OPEN || contract.Status == SM_ContractStatus.ACCEPTED) && contract.EndsAt > 0 && now >= contract.EndsAt)
			{
				contract.Status = SM_ContractStatus.EXPIRED;
				contract.CompletedAt = now;
				contract.CompletedDate = MakeDateStamp();
				contract.TargetOnline = false;
				RefundContractRewardToCreator(contract, "#STR_SMP_00333");
				changed = true;
				continue;
			}

			if (contract.Type == SM_ContractType.KILL && contract.Status == SM_ContractStatus.ACCEPTED)
			{
				PlayerBase target = FindOnlinePlayer(contract.TargetUid);
				if (target && target.GetIdentity())
				{
					contract.TargetOnline = true;
					if (contract.LastMarkerAt <= 0 || now - contract.LastMarkerAt >= markerInterval)
					{
						contract.LastTargetPos = target.GetPosition();
						contract.LastMarkerAt = now;
						changed = true;
					}
				}
				else
				{
					if (contract.TargetOnline)
					{
						contract.TargetOnline = false;
						changed = true;
					}
				}
			}
		}

		if (changed)
		{
			SaveContractsDB();
			BroadcastContracts();
		}
	}

	protected void SendContractTargetPreview(PlayerBase viewer, string targetUid)
	{
		if (!viewer || !viewer.GetIdentity())
			return;

		PlayerBase target = FindOnlinePlayer(targetUid);
		if (!target || !target.GetIdentity())
		{
			Notify(viewer, "#STR_SMP_00583", "#STR_SMP_01055");
			return;
		}

		if (!IsTargetAllowedForCreator(viewer, target))
		{
			Notify(viewer, "#STR_SMP_00583", "#STR_SMP_00825");
			return;
		}

		array<string> attachments = new array<string>;
		GameInventory inv = target.GetInventory();
		if (inv)
		{
			int count = inv.AttachmentCount();
			for (int i = 0; i < count; i++)
			{
				EntityAI att = inv.GetAttachmentFromIndex(i);
				if (!att)
					continue;
				string attClass = att.GetType();
				if (attClass != "")
					attachments.Insert(attClass);
			}
		}

		string handClass = "";
		HumanInventory humanInventory = target.GetHumanInventory();
		if (humanInventory)
		{
			EntityAI handItem = humanInventory.GetEntityInHands();
			if (handItem)
				handClass = handItem.GetType();
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(target.GetIdentity().GetPlainId());
		rpc.Write(target.GetIdentity().GetName());
		rpc.Write(target.GetType());
		rpc.Write(handClass);
		rpc.Write(attachments.Count());
		foreach (string className : attachments)
		{
			rpc.Write(className);
		}
		rpc.Send(viewer, SM_PartyRPC.SYNC_CONTRACT_TARGET_PREVIEW, true, viewer.GetIdentity());
	}

	protected void WriteContractTargets(ScriptRPC rpc, PlayerBase player)
	{
		array<ref SM_ContractTargetView> targets = new array<ref SM_ContractTargetView>;
		if (player && player.GetIdentity())
		{
			array<Man> players = new array<Man>;
			GetGame().GetPlayers(players);
			foreach (Man man : players)
			{
				PlayerBase target = PlayerBase.Cast(man);
				if (!target || !target.GetIdentity())
					continue;
				if (!IsTargetAllowedForCreator(player, target))
					continue;
				string targetUid = target.GetIdentity().GetPlainId();
				string clanName = GetContractViewerClanName(targetUid);
				string title = GetPlayerExperienceTitle(targetUid);
				targets.Insert(new SM_ContractTargetView(targetUid, target.GetIdentity().GetName(), clanName, title, true));
			}
		}

		rpc.Write(targets.Count());
		foreach (SM_ContractTargetView targetView : targets)
		{
			rpc.Write(targetView.Uid);
			rpc.Write(targetView.Name);
			rpc.Write(targetView.ClanName);
			rpc.Write(targetView.Title);
			rpc.Write(targetView.Online);
		}
	}

	protected void WriteContractItems(ScriptRPC rpc)
	{
		int count = 0;
		if (m_Config.Contracts && m_Config.Contracts.SearchItems)
			count = m_Config.Contracts.SearchItems.Count();
		rpc.Write(count);
		if (count <= 0)
			return;

		foreach (SM_ContractItemConfig item : m_Config.Contracts.SearchItems)
		{
			rpc.Write(item.ClassName);
			rpc.Write(item.DisplayName);
		}
	}

	protected void SendContracts(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		string viewerClanName = GetContractViewerClanName(uid);
		bool enabled = IsContractsEnabled();
		bool canCreate = false;
		if (enabled && CanCreateContractByTitle(player))
			canCreate = true;

		array<ref SM_Contract> active = new array<ref SM_Contract>;
		array<ref SM_Contract> history = new array<ref SM_Contract>;
		array<ref SM_Contract> markers = new array<ref SM_Contract>;
		if (enabled && m_ContractsDB && m_ContractsDB.Contracts)
		{
			foreach (SM_Contract contract : m_ContractsDB.Contracts)
			{
				if (!contract)
					continue;
				if (IsContractVisibleInActiveList(contract, uid, viewerClanName))
					active.Insert(contract);
				if (IsContractVisibleInHistory(contract, uid, viewerClanName))
					history.Insert(contract);
				if (IsContractMarkerVisible(contract, uid, viewerClanName))
					markers.Insert(contract);
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(enabled);
		rpc.Write(canCreate);
		rpc.Write(m_Config.Contracts.MinPrice);
		rpc.Write(m_Config.Contracts.MaxPrice);
		rpc.Write(m_Config.Contracts.FeePercent);
		rpc.Write(m_Config.Contracts.AbandonPenaltyPercent);
		rpc.Write(m_Config.Contracts.MinDurationMinutes);
		rpc.Write(m_Config.Contracts.MaxDurationMinutes);

		rpc.Write(active.Count());
		foreach (SM_Contract activeContract : active)
			WriteContractView(rpc, activeContract, uid);

		rpc.Write(history.Count());
		foreach (SM_Contract historyContract : history)
			WriteContractView(rpc, historyContract, uid);

		WriteContractTargets(rpc, player);
		WriteContractItems(rpc);

		rpc.Write(markers.Count());
		foreach (SM_Contract markerContract : markers)
		{
			string label = markerContract.TargetName;
			if (markerContract.TargetClanName != "")
				label = label + " [" + markerContract.TargetClanName + "]";
			rpc.Write(markerContract.Id);
			rpc.Write(markerContract.Type);
			rpc.Write(label);
			rpc.Write(markerContract.LastTargetPos);
			rpc.Write(GetContractSecondsLeft(markerContract));
		}

		rpc.Send(player, SM_PartyRPC.SYNC_CONTRACTS, true, player.GetIdentity());
	}

	protected void BroadcastContracts()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
				SendContracts(pb);
		}
	}

	protected bool IsPlayerInTradeZone(PlayerBase player, SM_PartyTradeZoneRestrictionConfig tradeZones)
	{
		if (!player)
			return false;
		if (!tradeZones || !tradeZones.Enabled)
			return true;
		if (!tradeZones.Zones)
			return false;

		vector playerPos = player.GetPosition();
		foreach (SM_PartyTradeZoneConfig zone : tradeZones.Zones)
		{
			if (!zone)
				continue;
			if (!zone.Enabled)
				continue;
			if (zone.RadiusMeters < 1.0)
				continue;
			if (vector.Distance(playerPos, zone.Position) <= zone.RadiusMeters)
				return true;
		}
		return false;
	}

	protected bool CheckTradeZone(PlayerBase player, SM_PartyTradeZoneRestrictionConfig tradeZones, bool requireZone, string title)
	{
		if (!requireZone)
			return true;
		if (!tradeZones || !tradeZones.Enabled)
			return true;
		if (IsPlayerInTradeZone(player, tradeZones))
			return true;

		Notify(player, title, "#STR_SMP_01130");
		return false;
	}

	protected bool CheckMarketListingZone(PlayerBase player)
	{
		bool requireZone = false;
		if (m_Config.Market.TradeZones && m_Config.Market.TradeZones.RequireForListing)
			requireZone = true;
		return CheckTradeZone(player, m_Config.Market.TradeZones, requireZone, "#STR_SMP_00891");
	}

	protected bool CheckMarketTakingZone(PlayerBase player)
	{
		bool requireZone = false;
		if (m_Config.Market.TradeZones && m_Config.Market.TradeZones.RequireForTaking)
			requireZone = true;
		return CheckTradeZone(player, m_Config.Market.TradeZones, requireZone, "#STR_SMP_00891");
	}

	protected bool CheckAuctionListingZone(PlayerBase player)
	{
		bool requireZone = false;
		if (m_Config.Auction.TradeZones && m_Config.Auction.TradeZones.RequireForListing)
			requireZone = true;
		return CheckTradeZone(player, m_Config.Auction.TradeZones, requireZone, "#STR_SMP_00258");
	}

	protected bool CheckAuctionTakingZone(PlayerBase player)
	{
		bool requireZone = false;
		if (m_Config.Auction.TradeZones && m_Config.Auction.TradeZones.RequireForTaking)
			requireZone = true;
		return CheckTradeZone(player, m_Config.Auction.TradeZones, requireZone, "#STR_SMP_00258");
	}

	protected int GetPercentFee(int value, int percent)
	{
		if (value <= 0 || percent <= 0)
			return 0;

		int fee = value * percent / 100;
		if (fee < 1)
			fee = 1;
		return fee;
	}

	protected bool ChargePlayerCurrency(PlayerBase player, int amount, string title, string notEnoughText)
	{
		if (amount <= 0)
			return true;

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (currency.Count() == 0)
		{
			Notify(player, title, "#STR_SMP_00294");
			return false;
		}

		int total = GetPlayerCurrencyTotal(player, currency);
		if (total < amount)
		{
			Notify(player, title, notEnoughText + amount + "#STR_SMP_00163" + total);
			return false;
		}

		int consumed = ConsumeAllCurrency(player, currency);
		int change = consumed - amount;
		if (change > 0)
			IssueCurrency(player, change);
		return true;
	}

	protected int GetCreatedAgeSeconds(int createdAtSeconds)
	{
		if (createdAtSeconds <= 0)
			return 2147483647;

		int age = GetNowAbsSeconds() - createdAtSeconds;
		if (age < 0)
			age = 0;
		return age;
	}

	protected int GetPendingReturnSecondsLeft(int availableAtSeconds)
	{
		if (availableAtSeconds <= 0)
			return 0;

		int left = availableAtSeconds - GetNowAbsSeconds();
		if (left < 0)
			left = 0;
		return left;
	}

	protected bool ShouldQueueMarketReturnForPickup(int availableAtSeconds)
	{
		if (GetPendingReturnSecondsLeft(availableAtSeconds) > 0)
			return true;
		if (m_Config.Market.TradeZones && m_Config.Market.TradeZones.Enabled && m_Config.Market.TradeZones.RequireForTaking)
			return true;
		return false;
	}

	protected bool CanAutoGiveMarketPending(PlayerBase player, SM_MarketLot lot)
	{
		if (!player || !lot)
			return false;
		if (GetPendingReturnSecondsLeft(lot.ReturnAvailableAtSeconds) > 0)
			return false;
		if (m_Config.Market.TradeZones && m_Config.Market.TradeZones.Enabled && m_Config.Market.TradeZones.RequireForTaking)
		{
			if (!IsPlayerInTradeZone(player, m_Config.Market.TradeZones))
				return false;
		}
		return true;
	}

	protected bool CanAutoGiveAuctionPending(PlayerBase player, SM_AuctionPendingItem pendingItem)
	{
		if (!player || !pendingItem)
			return false;
		if (GetPendingReturnSecondsLeft(pendingItem.AvailableAtSeconds) > 0)
			return false;
		if (m_Config.Auction.TradeZones && m_Config.Auction.TradeZones.Enabled && m_Config.Auction.TradeZones.RequireForTaking)
		{
			if (!IsPlayerInTradeZone(player, m_Config.Auction.TradeZones))
				return false;
		}
		return true;
	}

	protected void GiveReadyMarketPending(PlayerBase player, string uid)
	{
		if (!player || uid == "" || !m_MarketDB || !m_MarketDB.PendingReturns)
			return;

		bool changed = false;
		for (int i = m_MarketDB.PendingReturns.Count() - 1; i >= 0; i--)
		{
			SM_MarketLot lot = m_MarketDB.PendingReturns[i];
			if (!lot || lot.SellerUid != uid)
				continue;
			if (!CanAutoGiveMarketPending(player, lot))
				continue;

			m_MarketDB.PendingReturns.Remove(i);
			changed = true;
			RestoreItemTree(lot.Item, player, false);
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00510" + lot.Item.ClassName + "#STR_SMP_00032");
		}

		if (changed)
			SaveMarketDB();
	}

	protected void GiveReadyAuctionItemPending(PlayerBase player, string uid)
	{
		if (!player || uid == "" || !m_AuctionState || !m_AuctionState.PendingItems)
			return;

		bool changed = false;
		for (int i = m_AuctionState.PendingItems.Count() - 1; i >= 0; i--)
		{
			SM_AuctionPendingItem pendingItem = m_AuctionState.PendingItems[i];
			if (!pendingItem || pendingItem.OwnerUid != uid)
				continue;
			if (!CanAutoGiveAuctionPending(player, pendingItem))
				continue;

			m_AuctionState.PendingItems.Remove(i);
			changed = true;
			RestoreItemTree(pendingItem.Item, player, true);
			Notify(player, "#STR_SMP_00258", pendingItem.Reason);
		}

		if (changed)
			SaveAuctionState();
	}

	protected void TradePendingTick()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.GetIdentity())
				continue;

			string uid = player.GetIdentity().GetPlainId();
			GiveReadyMarketPending(player, uid);
			GiveReadyAuctionItemPending(player, uid);
		}
	}

	static const string AUCTION_DIR = "$profile:SM_PartyMod\\Auction";
	static const string AUCTION_STATE_PATH = "$profile:SM_PartyMod\\Auction\\State.json";

	protected void LoadAuctionState()
	{
		if (!FileExist(AUCTION_DIR))
			MakeDirectory(AUCTION_DIR);

		m_AuctionState = new SM_AuctionState();
		if (m_Config.Auction.SaveState && FileExist(AUCTION_STATE_PATH))
		{
			string errorMessage;
			if (!JsonFileLoader<SM_AuctionState>.LoadFile(AUCTION_STATE_PATH, m_AuctionState, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00199" + errorMessage));
				m_AuctionState = new SM_AuctionState();
			}
		}
		PrepareAuctionState();
	}

	protected void PrepareAuctionState()
	{
		if (!m_AuctionState)
			m_AuctionState = new SM_AuctionState();
		if (!m_AuctionState.Lots)
			m_AuctionState.Lots = new array<ref SM_AuctionLot>;
		if (!m_AuctionState.PendingItems)
			m_AuctionState.PendingItems = new array<ref SM_AuctionPendingItem>;
		if (!m_AuctionState.PendingMoney)
			m_AuctionState.PendingMoney = new array<ref SM_AuctionPendingMoney>;

		for (int i = m_AuctionState.Lots.Count() - 1; i >= 0; i--)
		{
			SM_AuctionLot lot = m_AuctionState.Lots[i];
			if (!lot || !lot.Item || lot.SellerUid == "")
			{
				m_AuctionState.Lots.Remove(i);
				continue;
			}
			if (lot.Id >= m_AuctionState.NextLotId)
				m_AuctionState.NextLotId = lot.Id + 1;
			if (lot.BidStep < 1)
				lot.BidStep = m_Config.Auction.MinBidStep;
		}

		for (int p = m_AuctionState.PendingItems.Count() - 1; p >= 0; p--)
		{
			SM_AuctionPendingItem pendingItem = m_AuctionState.PendingItems[p];
			if (!pendingItem || pendingItem.OwnerUid == "" || !pendingItem.Item)
				m_AuctionState.PendingItems.Remove(p);
		}

		for (int m = m_AuctionState.PendingMoney.Count() - 1; m >= 0; m--)
		{
			SM_AuctionPendingMoney pendingMoney = m_AuctionState.PendingMoney[m];
			if (!pendingMoney || pendingMoney.OwnerUid == "" || pendingMoney.Amount <= 0)
				m_AuctionState.PendingMoney.Remove(m);
		}

		if (m_AuctionState.NextLotId < 1)
			m_AuctionState.NextLotId = 1;
	}

	protected void SaveAuctionState()
	{
		if (!m_Config.Auction.SaveState)
			return;
		if (!FileExist(AUCTION_DIR))
			MakeDirectory(AUCTION_DIR);

		string errorMessage;
		if (!JsonFileLoader<SM_AuctionState>.SaveFile(AUCTION_STATE_PATH, m_AuctionState, errorMessage))
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00183" + errorMessage));
	}

	protected SM_AuctionLot FindAuctionLot(int lotId)
	{
		if (!m_AuctionState || !m_AuctionState.Lots)
			return null;
		foreach (SM_AuctionLot lot : m_AuctionState.Lots)
		{
			if (lot && lot.Id == lotId)
				return lot;
		}
		return null;
	}

	protected int CountAuctionLotsBySeller(string uid)
	{
		int count = 0;
		foreach (SM_AuctionLot lot : m_AuctionState.Lots)
		{
			if (lot && lot.SellerUid == uid)
				count++;
		}
		return count;
	}

	protected int GetAuctionSecondsLeft(SM_AuctionLot lot)
	{
		if (!lot)
			return 0;
		int left = lot.EndStampSeconds - GetNowAbsSeconds();
		if (left < 0)
			left = 0;
		return left;
	}

	protected int GetAuctionNextBid(SM_AuctionLot lot)
	{
		if (!lot)
			return 0;
		if (lot.CurrentBid <= 0)
			return lot.StartPrice;
		return lot.CurrentBid + lot.BidStep;
	}

	protected void QueueAuctionMoney(string uid, string name, int amount, string reason)
	{
		if (uid == "" || amount <= 0)
			return;
		SM_AuctionPendingMoney pending = new SM_AuctionPendingMoney(uid, name, amount, reason);
		m_AuctionState.PendingMoney.Insert(pending);
	}

	protected void GiveAuctionMoney(string uid, string name, int amount, string reason)
	{
		if (amount <= 0 || uid == "")
			return;

		PlayerBase player = FindOnlinePlayer(uid);
		if (player)
		{
			IssueCurrency(player, amount);
			Notify(player, "#STR_SMP_00258", reason + ": +" + amount);
		}
		else
		{
			QueueAuctionMoney(uid, name, amount, reason);
		}
	}

	protected void QueueAuctionItem(string uid, string name, SM_MarketItem item, string reason, int availableAtSeconds = 0)
	{
		if (uid == "" || !item)
			return;

		SM_AuctionPendingItem pending = new SM_AuctionPendingItem();
		pending.OwnerUid = uid;
		pending.OwnerName = name;
		pending.Reason = reason;
		pending.AvailableAtSeconds = availableAtSeconds;
		pending.Item = item;
		m_AuctionState.PendingItems.Insert(pending);
	}

	protected bool ShouldQueueAuctionItemForPickup(int availableAtSeconds)
	{
		if (GetPendingReturnSecondsLeft(availableAtSeconds) > 0)
			return true;
		if (m_Config.Auction.TradeZones && m_Config.Auction.TradeZones.Enabled && m_Config.Auction.TradeZones.RequireForTaking)
			return true;
		return false;
	}

	protected void GiveAuctionItem(string uid, string name, SM_MarketItem item, string reason, int availableAtSeconds = 0)
	{
		if (uid == "" || !item)
			return;

		PlayerBase player = FindOnlinePlayer(uid);
		if (ShouldQueueAuctionItemForPickup(availableAtSeconds))
		{
			QueueAuctionItem(uid, name, item, reason, availableAtSeconds);
			if (player)
			{
				int left = GetPendingReturnSecondsLeft(availableAtSeconds);
				if (left > 0)
					Notify(player, "#STR_SMP_00258", "#STR_SMP_01135" + SM_PartyUtil.FormatDuration(left));
				else
					Notify(player, "#STR_SMP_00258", "#STR_SMP_01139");
			}
			return;
		}

		if (player)
		{
			RestoreItemTree(item, player, true);
			Notify(player, "#STR_SMP_00258", reason);
		}
		else
		{
			QueueAuctionItem(uid, name, item, reason, availableAtSeconds);
		}
	}

	protected void GiveAuctionPending(PlayerBase player, string uid)
	{
		if (!player || uid == "" || !m_AuctionState)
			return;

		bool changed = false;
		for (int i = m_AuctionState.PendingItems.Count() - 1; i >= 0; i--)
		{
			SM_AuctionPendingItem pendingItem = m_AuctionState.PendingItems[i];
			if (!pendingItem || pendingItem.OwnerUid != uid)
				continue;
			int itemLeft = GetPendingReturnSecondsLeft(pendingItem.AvailableAtSeconds);
			if (itemLeft > 0)
			{
				Notify(player, "#STR_SMP_00258", "#STR_SMP_01131" + SM_PartyUtil.FormatDuration(itemLeft));
				continue;
			}
			if (!CheckAuctionTakingZone(player))
				continue;

			m_AuctionState.PendingItems.Remove(i);
			changed = true;
			RestoreItemTree(pendingItem.Item, player, true);
			Notify(player, "#STR_SMP_00258", pendingItem.Reason);
		}

		for (int m = m_AuctionState.PendingMoney.Count() - 1; m >= 0; m--)
		{
			SM_AuctionPendingMoney pendingMoney = m_AuctionState.PendingMoney[m];
			if (!pendingMoney || pendingMoney.OwnerUid != uid)
				continue;

			m_AuctionState.PendingMoney.Remove(m);
			changed = true;
			IssueCurrency(player, pendingMoney.Amount);
			Notify(player, "#STR_SMP_00258", pendingMoney.Reason + ": +" + pendingMoney.Amount);
		}

		if (changed)
			SaveAuctionState();
	}

	protected void HandleAuctionSell(PlayerBase player, int startPrice, int bidStep, int durationMinutes)
	{
		if (!m_Config.Auction.Enabled || !player || !player.GetIdentity())
			return;

		if (!CheckAuctionListingZone(player))
			return;

		string uid = player.GetIdentity().GetPlainId();
		if (CountAuctionLotsBySeller(uid) >= m_Config.Auction.MaxActiveLotsPerPlayer)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00602" + m_Config.Auction.MaxActiveLotsPerPlayer);
			return;
		}
		if (startPrice < 1 || startPrice > m_Config.Auction.MaxStartPrice)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00975" + m_Config.Auction.MaxStartPrice);
			return;
		}
		if (bidStep < m_Config.Auction.MinBidStep)
			bidStep = m_Config.Auction.MinBidStep;
		if (durationMinutes < m_Config.Auction.MinDurationMinutes)
			durationMinutes = m_Config.Auction.MinDurationMinutes;
		if (durationMinutes > m_Config.Auction.MaxDurationMinutes)
			durationMinutes = m_Config.Auction.MaxDurationMinutes;

		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		if (!inHands)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00336");
			return;
		}

		string itemType = inHands.GetType();
		foreach (string banned : m_Config.Market.Blacklist)
		{
			if (banned == itemType)
			{
				Notify(player, "#STR_SMP_00258", "#STR_SMP_01077");
				return;
			}
		}

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (GetCurrencyValue(itemType, currency) > 0)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00297");
			return;
		}

		int listingFee = m_Config.Auction.ListingFee + GetPercentFee(startPrice, m_Config.Auction.ListingFeePercent);
		if (listingFee > 0)
		{
			if (!ChargePlayerCurrency(player, listingFee, "#STR_SMP_00258", "#STR_SMP_00680"))
				return;
		}

		int nested = 0;
		int now = GetNowAbsSeconds();
		SM_AuctionLot lot = new SM_AuctionLot();
		lot.Item = CaptureItemTree(inHands, nested);
		lot.Id = m_AuctionState.NextLotId;
		m_AuctionState.NextLotId++;
		lot.SellerUid = uid;
		lot.SellerName = player.GetIdentity().GetName();
		SM_Clan sellerClan = FindClanByMember(uid);
		if (sellerClan)
			lot.SellerClanName = sellerClan.Name;
		lot.StartPrice = startPrice;
		lot.BidStep = bidStep;
		lot.CurrentBid = 0;
		lot.CreatedAtSeconds = now;
		lot.EndStampSeconds = now + durationMinutes * 60;
		lot.Date = MakeDateStamp();

		GetGame().ObjectDelete(inHands);
		m_AuctionState.Lots.Insert(lot);
		SaveAuctionState();

		Notify(player, "#STR_SMP_00258", "#STR_SMP_00615" + lot.Item.ClassName);
		BroadcastAuction();
	}

	protected void HandleAuctionBid(PlayerBase player, int lotId, int bidAmount)
	{
		if (!m_Config.Auction.Enabled || !player || !player.GetIdentity())
			return;

		SM_AuctionLot lot = FindAuctionLot(lotId);
		if (!lot)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00617");
			SendAuction(player);
			return;
		}
		if (GetAuctionSecondsLeft(lot) <= 0)
		{
			AuctionTick();
			SendAuction(player);
			return;
		}

		string uid = player.GetIdentity().GetPlainId();
		if (lot.SellerUid == uid)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00701");
			return;
		}
		if (lot.CurrentBidderUid == uid)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00311");
			return;
		}

		int minBid = GetAuctionNextBid(lot);
		if (bidAmount < minBid)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00643" + minBid);
			return;
		}

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (currency.Count() == 0)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00294");
			return;
		}

		int total = GetPlayerCurrencyTotal(player, currency);
		if (total < bidAmount)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00681" + bidAmount + "#STR_SMP_00163" + total);
			return;
		}

		int consumed = ConsumeAllCurrency(player, currency);
		int change = consumed - bidAmount;
		if (change > 0)
			IssueCurrency(player, change);

		if (lot.CurrentBid > 0 && lot.CurrentBidderUid != "")
			GiveAuctionMoney(lot.CurrentBidderUid, lot.CurrentBidderName, lot.CurrentBid, "#STR_SMP_00334");

		lot.CurrentBid = bidAmount;
		lot.CurrentBidderUid = uid;
		lot.CurrentBidderName = player.GetIdentity().GetName();

		int secondsLeft = GetAuctionSecondsLeft(lot);
		if (m_Config.Auction.AntiSnipeSeconds > 0 && secondsLeft <= m_Config.Auction.AntiSnipeSeconds)
			lot.EndStampSeconds += m_Config.Auction.AntiSnipeExtendSeconds;

		SaveAuctionState();
		Notify(player, "#STR_SMP_00258", "#STR_SMP_00970" + bidAmount);
		BroadcastAuction();
	}

	protected void HandleAuctionCancel(PlayerBase player, int lotId)
	{
		if (!m_Config.Auction.Enabled || !player || !player.GetIdentity())
			return;

		if (!CheckAuctionTakingZone(player))
			return;

		SM_AuctionLot lot = FindAuctionLot(lotId);
		if (!lot)
			return;
		string uid = player.GetIdentity().GetPlainId();
		if (lot.SellerUid != uid)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00956");
			return;
		}
		if (lot.CurrentBid > 0 || lot.CurrentBidderUid != "")
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00620");
			return;
		}
		if (!m_Config.Auction.AllowCancelWithoutBids)
		{
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00758");
			return;
		}

		int minAgeSeconds = m_Config.Auction.MinCancelAgeMinutes * 60;
		if (minAgeSeconds > 0 && GetCreatedAgeSeconds(lot.CreatedAtSeconds) < minAgeSeconds)
		{
			int ageLeft = minAgeSeconds - GetCreatedAgeSeconds(lot.CreatedAtSeconds);
			Notify(player, "#STR_SMP_00258", "#STR_SMP_01132" + SM_PartyUtil.FormatDuration(ageLeft));
			return;
		}

		int penalty = GetPercentFee(lot.StartPrice, m_Config.Auction.CancelPenaltyPercent);
		if (!ChargePlayerCurrency(player, penalty, "#STR_SMP_00258", "#STR_SMP_01137"))
			return;

		m_AuctionState.Lots.RemoveItem(lot);

		int availableAt = 0;
		if (m_Config.Auction.ReturnDelaySeconds > 0)
			availableAt = GetNowAbsSeconds() + m_Config.Auction.ReturnDelaySeconds;

		if (GetPendingReturnSecondsLeft(availableAt) > 0)
		{
			QueueAuctionItem(lot.SellerUid, lot.SellerName, lot.Item, "#STR_SMP_00618", availableAt);
			Notify(player, "#STR_SMP_00258", "#STR_SMP_01135" + SM_PartyUtil.FormatDuration(m_Config.Auction.ReturnDelaySeconds));
		}
		else
		{
			RestoreItemTree(lot.Item, player, true);
			Notify(player, "#STR_SMP_00258", "#STR_SMP_00618");
		}
		SaveAuctionState();
		BroadcastAuction();
	}

	protected void FinishAuctionLot(SM_AuctionLot lot)
	{
		if (!lot)
			return;

		m_AuctionState.Lots.RemoveItem(lot);
		if (lot.CurrentBid <= 0 || lot.CurrentBidderUid == "")
		{
			if (m_Config.Auction.ReturnItemIfNoBids)
			{
				int availableAt = 0;
				if (m_Config.Auction.ReturnDelaySeconds > 0)
					availableAt = GetNowAbsSeconds() + m_Config.Auction.ReturnDelaySeconds;
				GiveAuctionItem(lot.SellerUid, lot.SellerName, lot.Item, "#STR_SMP_00616", availableAt);
			}
			return;
		}

		GiveAuctionItem(lot.CurrentBidderUid, lot.CurrentBidderName, lot.Item, "#STR_SMP_00346" + lot.Item.ClassName);

		int tax = lot.CurrentBid * m_Config.Auction.SaleTaxPercent / 100;
		int sellerIncome = lot.CurrentBid - tax;
		if (sellerIncome < 0)
			sellerIncome = 0;
		GiveAuctionMoney(lot.SellerUid, lot.SellerName, sellerIncome, "#STR_SMP_00859" + lot.Item.ClassName);

		PlayerBase winner = FindOnlinePlayer(lot.CurrentBidderUid);
		PlayerBase seller = FindOnlinePlayer(lot.SellerUid);
		AddPersonalAchievementValue(lot.CurrentBidderUid, lot.CurrentBidderName, "AuctionWins", 1, winner);
		AddPersonalAchievementValue(lot.SellerUid, lot.SellerName, "AuctionSales", 1, seller);

		SM_Clan winnerClan = FindClanByMember(lot.CurrentBidderUid);
		if (winnerClan)
		{
			winnerClan.Stats.AuctionWins++;
			m_StatsDirty = true;
			CheckClanAchievementNotifications(winnerClan);
		}

		SM_Clan sellerClan = FindClanByMember(lot.SellerUid);
		if (sellerClan)
		{
			sellerClan.Stats.AuctionSales++;
			m_StatsDirty = true;
			CheckClanAchievementNotifications(sellerClan);
		}
	}

	protected void AuctionTick()
	{
		if (!m_Config.Auction.Enabled || !m_AuctionState)
			return;

		bool changed = false;
		for (int i = m_AuctionState.Lots.Count() - 1; i >= 0; i--)
		{
			SM_AuctionLot lot = m_AuctionState.Lots[i];
			if (!lot)
				continue;
			if (GetAuctionSecondsLeft(lot) > 0)
				continue;
			FinishAuctionLot(lot);
			changed = true;
		}

		if (changed)
		{
			SaveAuctionState();
			BroadcastAuction();
		}
	}

	void SendAuction(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		GiveAuctionPending(player, player.GetIdentity().GetPlainId());

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_Config.Auction.Enabled);
		rpc.Write(m_Config.Auction.ListingFee);
		rpc.Write(m_Config.Auction.ListingFeePercent);
		rpc.Write(m_Config.Auction.SaleTaxPercent);
		rpc.Write(m_Config.Auction.MinDurationMinutes);
		rpc.Write(m_Config.Auction.MaxDurationMinutes);
		rpc.Write(m_Config.Auction.DefaultDurationMinutes);
		rpc.Write(m_Config.Auction.MinBidStep);
		rpc.Write(m_Config.Auction.MaxStartPrice);
		rpc.Write(m_AuctionState.Lots.Count());

		foreach (SM_AuctionLot lot : m_AuctionState.Lots)
		{
			rpc.Write(lot.Id);
			rpc.Write(lot.Item.ClassName);
			rpc.Write(lot.Item.Quantity);
			rpc.Write(lot.Item.CountNested());
			rpc.Write(lot.Item.Health);
			rpc.Write(lot.SellerUid);
			rpc.Write(lot.SellerName);
			rpc.Write(lot.SellerClanName);
			rpc.Write(lot.StartPrice);
			rpc.Write(lot.BidStep);
			rpc.Write(lot.CurrentBid);
			rpc.Write(lot.CurrentBidderName);
			rpc.Write(GetAuctionSecondsLeft(lot));
			rpc.Write(lot.Date);
			rpc.Write(lot.Item.Attachments.Count());
			foreach (SM_MarketItem attData : lot.Item.Attachments)
				rpc.Write(attData.ClassName);
		}
		rpc.Send(player, SM_PartyRPC.SYNC_AUCTION, true, player.GetIdentity());
	}

	protected void BroadcastAuction()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
				SendAuction(pb);
		}
	}

	static const string MARKET_DB_DIR = "$profile:SM_PartyMod\\Market";
	static const string MARKET_LOTS_DIR = "$profile:SM_PartyMod\\Market\\Lots";
	static const string MARKET_INDEX_PATH = "$profile:SM_PartyMod\\Market\\Index.json";
	static const string MARKET_PENDING_PATH = "$profile:SM_PartyMod\\Market\\PendingReturns.json";
	static const int MAX_MARKET_NESTED = 60;

	protected void LoadMarketDB()
	{
		if (!FileExist(MARKET_DB_DIR))
			MakeDirectory(MARKET_DB_DIR);
		if (!FileExist(MARKET_LOTS_DIR))
			MakeDirectory(MARKET_LOTS_DIR);

		m_MarketDB = new SM_MarketDB();

		if (FileExist(MARKET_INDEX_PATH))
			LoadMarketFileDB();

		PrepareMarketDB();
	}

	protected bool LoadMarketFileDB()
	{
		SM_MarketIndex index = new SM_MarketIndex();
		string errorMessage;
		if (!JsonFileLoader<SM_MarketIndex>.LoadFile(MARKET_INDEX_PATH, index, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00203" + errorMessage));
			return false;
		}
		if (!index)
			return false;
		if (!index.Lots)
			index.Lots = new array<int>;

		m_MarketDB.NextLotId = index.NextLotId;
		foreach (int lotId : index.Lots)
		{
			if (lotId < 1)
				continue;

			SM_MarketLot lot = new SM_MarketLot();
			string lotPath = MARKET_LOTS_DIR + "\\" + GetMarketLotFileName(lotId);
			if (!JsonFileLoader<SM_MarketLot>.LoadFile(lotPath, lot, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00211" + lotPath + ": " + errorMessage));
				continue;
			}
			if (!lot)
				continue;
			if (lot.Id < 1)
				lot.Id = lotId;
			m_MarketDB.Lots.Insert(lot);
		}

		LoadMarketPendingReturns();
		return true;
	}

	protected void LoadMarketPendingReturns()
	{
		if (!FileExist(MARKET_PENDING_PATH))
			return;

		SM_MarketPendingReturns pending = new SM_MarketPendingReturns();
		string errorMessage;
		if (!JsonFileLoader<SM_MarketPendingReturns>.LoadFile(MARKET_PENDING_PATH, pending, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00204" + errorMessage));
			return;
		}
		if (pending && pending.PendingReturns)
			m_MarketDB.PendingReturns = pending.PendingReturns;
	}

	protected void PrepareMarketDB()
	{
		if (!m_MarketDB)
			m_MarketDB = new SM_MarketDB();
		if (!m_MarketDB.Lots)
			m_MarketDB.Lots = new array<ref SM_MarketLot>;
		if (!m_MarketDB.PendingReturns)
			m_MarketDB.PendingReturns = new array<ref SM_MarketLot>;

		for (int i = m_MarketDB.Lots.Count() - 1; i >= 0; i--)
		{
			if (!m_MarketDB.Lots[i] || !m_MarketDB.Lots[i].Item)
				m_MarketDB.Lots.Remove(i);
		}
		for (int p = m_MarketDB.PendingReturns.Count() - 1; p >= 0; p--)
		{
			if (!m_MarketDB.PendingReturns[p] || !m_MarketDB.PendingReturns[p].Item)
				m_MarketDB.PendingReturns.Remove(p);
		}

		if (m_MarketDB.NextLotId < 1)
			m_MarketDB.NextLotId = 1;
		foreach (SM_MarketLot lot : m_MarketDB.Lots)
		{
			if (lot.Id >= m_MarketDB.NextLotId)
				m_MarketDB.NextLotId = lot.Id + 1;
		}
		foreach (SM_MarketLot pendingLot : m_MarketDB.PendingReturns)
		{
			if (pendingLot.Id >= m_MarketDB.NextLotId)
				m_MarketDB.NextLotId = pendingLot.Id + 1;
		}
	}

	protected string GetMarketLotFileName(int lotId)
	{
		return "lot_" + lotId.ToString() + ".json";
	}

	protected SM_MarketIndex LoadExistingMarketIndex()
	{
		if (!FileExist(MARKET_INDEX_PATH))
			return null;

		SM_MarketIndex index = new SM_MarketIndex();
		string errorMessage;
		if (!JsonFileLoader<SM_MarketIndex>.LoadFile(MARKET_INDEX_PATH, index, errorMessage))
			return null;
		if (!index || !index.Lots)
			return null;
		return index;
	}

	protected bool HasCurrentMarketLotFile(int lotId)
	{
		foreach (SM_MarketLot lot : m_MarketDB.Lots)
		{
			if (lot && lot.Id == lotId)
				return true;
		}
		return false;
	}

	protected void CleanupDeletedMarketLotFiles(SM_MarketIndex oldIndex)
	{
		if (!oldIndex || !oldIndex.Lots)
			return;

		foreach (int lotId : oldIndex.Lots)
		{
			if (lotId < 1)
				continue;
			if (HasCurrentMarketLotFile(lotId))
				continue;
			DeleteFile(MARKET_LOTS_DIR + "\\" + GetMarketLotFileName(lotId));
		}
	}

	protected void SaveMarketDB()
	{
		string errorMessage;
		if (!FileExist(MARKET_DB_DIR))
			MakeDirectory(MARKET_DB_DIR);
		if (!FileExist(MARKET_LOTS_DIR))
			MakeDirectory(MARKET_LOTS_DIR);

		PrepareMarketDB();

		SM_MarketIndex oldIndex = LoadExistingMarketIndex();
		SM_MarketIndex index = new SM_MarketIndex();
		index.NextLotId = m_MarketDB.NextLotId;

		bool saveFailed = false;
		foreach (SM_MarketLot lot : m_MarketDB.Lots)
		{
			if (!lot)
				continue;
			index.Lots.Insert(lot.Id);
			string lotPath = MARKET_LOTS_DIR + "\\" + GetMarketLotFileName(lot.Id);
			if (oldIndex && FileExist(lotPath))
				continue;
			if (!JsonFileLoader<SM_MarketLot>.SaveFile(lotPath, lot, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00195" + lot.Id + ": " + errorMessage));
				saveFailed = true;
			}
		}

		if (saveFailed)
			return;

		SM_MarketPendingReturns pending = new SM_MarketPendingReturns();
		pending.PendingReturns = m_MarketDB.PendingReturns;
		if (!JsonFileLoader<SM_MarketPendingReturns>.SaveFile(MARKET_PENDING_PATH, pending, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00188" + errorMessage));
			return;
		}

		if (!JsonFileLoader<SM_MarketIndex>.SaveFile(MARKET_INDEX_PATH, index, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00187" + errorMessage));
			return;
		}

		CleanupDeletedMarketLotFiles(oldIndex);
	}

	protected SM_MarketLot FindLot(int lotId)
	{
		foreach (SM_MarketLot lot : m_MarketDB.Lots)
		{
			if (lot.Id == lotId)
				return lot;
		}
		return null;
	}

	protected int CountClanLots(string clanName)
	{
		int total = 0;
		foreach (SM_MarketLot lot : m_MarketDB.Lots)
		{
			if (lot.ClanName == clanName)
				total++;
		}
		return total;
	}

	protected SM_MarketItem CaptureItemTree(EntityAI entity, out int nested)
	{
		SM_MarketItem data = new SM_MarketItem();
		data.ClassName = entity.GetType();
		data.Health = entity.GetHealth01("", "");

		Magazine mag = Magazine.Cast(entity);
		ItemBase ib = ItemBase.Cast(entity);
		if (mag)
		{
			data.AmmoCount = mag.GetAmmoCount();
		}
		else if (ib && ib.HasQuantity())
		{
			data.Quantity = ib.GetQuantity();
			if (ib.GetLiquidType() > 0)
				data.LiquidType = ib.GetLiquidType();
		}

		GameInventory inv = entity.GetInventory();
		if (inv && nested < MAX_MARKET_NESTED)
		{
			for (int a = 0; a < inv.AttachmentCount(); a++)
			{
				EntityAI att = inv.GetAttachmentFromIndex(a);
				if (att)
				{
					nested++;
					data.Attachments.Insert(CaptureItemTree(att, nested));
				}
			}

			CargoBase cargo = inv.GetCargo();
			if (cargo)
			{
				for (int c = 0; c < cargo.GetItemCount(); c++)
				{
					EntityAI cargoItem = cargo.GetItem(c);
					if (cargoItem)
					{
						nested++;
						data.Cargo.Insert(CaptureItemTree(cargoItem, nested));
					}
				}
			}
		}

		return data;
	}

	protected void ApplyItemProps(EntityAI entity, SM_MarketItem data)
	{
		entity.SetHealth01("", "", data.Health);

		Magazine mag = Magazine.Cast(entity);
		ItemBase ib = ItemBase.Cast(entity);
		if (mag && data.AmmoCount >= 0)
		{
			mag.ServerSetAmmoCount(data.AmmoCount);
		}
		else if (ib && data.Quantity >= 0)
		{
			if (data.LiquidType > 0)
				ib.SetLiquidType(data.LiquidType);
			ib.SetQuantity(data.Quantity);
		}
	}

	protected void RestoreChildren(EntityAI parent, SM_MarketItem data, vector groundPos)
	{
		foreach (SM_MarketItem attData : data.Attachments)
		{
			EntityAI att = parent.GetInventory().CreateAttachment(attData.ClassName);
			if (!att)
				att = parent.GetInventory().CreateEntityInCargo(attData.ClassName);
			if (!att)
				att = EntityAI.Cast(GetGame().CreateObjectEx(attData.ClassName, groundPos, ECE_PLACE_ON_SURFACE));
			if (att)
			{
				ApplyItemProps(att, attData);
				RestoreChildren(att, attData, groundPos);
			}
		}

		foreach (SM_MarketItem cargoData : data.Cargo)
		{
			EntityAI cargoItem = parent.GetInventory().CreateEntityInCargo(cargoData.ClassName);
			if (!cargoItem)
				cargoItem = parent.GetInventory().CreateAttachment(cargoData.ClassName);
			if (!cargoItem)
				cargoItem = EntityAI.Cast(GetGame().CreateObjectEx(cargoData.ClassName, groundPos, ECE_PLACE_ON_SURFACE));
			if (cargoItem)
			{
				ApplyItemProps(cargoItem, cargoData);
				RestoreChildren(cargoItem, cargoData, groundPos);
			}
		}
	}

	protected EntityAI RestoreItemTree(SM_MarketItem data, PlayerBase receiver, bool toInventory)
	{
		vector groundPos = receiver.GetPosition();

		EntityAI root;
		if (toInventory)
			root = receiver.GetInventory().CreateInInventory(data.ClassName);
		if (!root)
			root = EntityAI.Cast(GetGame().CreateObjectEx(data.ClassName, groundPos, ECE_PLACE_ON_SURFACE));
		if (!root)
			return null;

		ApplyItemProps(root, data);
		RestoreChildren(root, data, groundPos);
		return root;
	}

	void SendMarket(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		GivePendingReturns(player, player.GetIdentity().GetPlainId());

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_MarketDB.Lots.Count());
		foreach (SM_MarketLot lot : m_MarketDB.Lots)
		{
			rpc.Write(lot.Id);
			rpc.Write(lot.Item.ClassName);
			rpc.Write(lot.Item.Quantity);
			rpc.Write(lot.Item.CountNested());
			rpc.Write(lot.Item.Health);
			rpc.Write(lot.Price);
			rpc.Write(lot.ClanName);
			rpc.Write(lot.SellerUid);
			rpc.Write(lot.SellerName);
			rpc.Write(lot.Date);
			rpc.Write(lot.Item.Attachments.Count());
			foreach (SM_MarketItem attData : lot.Item.Attachments)
				rpc.Write(attData.ClassName);
		}
		rpc.Send(player, SM_PartyRPC.SYNC_MARKET, true, player.GetIdentity());
	}

	protected void BroadcastMarket()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
				SendMarket(pb);
		}
	}

	protected void HandleMarketSell(PlayerBase player, int price)
	{
		if (!m_Config.Market.Enabled || !player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00853");
			return;
		}

		SM_ClanMember actor = clan.FindMember(uid);
		if (actor.Rank < clan.GetMinRank(SM_ClanAction.MARKET_SELL))
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00695");
			return;
		}

		if (!CheckMarketListingZone(player))
			return;

		if (price <= 0 || price > m_Config.Market.MaxPrice)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_01060" + m_Config.Market.MaxPrice);
			return;
		}

		int lotCap = GetClanLotCap(clan);
		if (CountClanLots(clan.Name) >= lotCap)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00604" + lotCap);
			return;
		}

		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		if (!inHands)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00336");
			return;
		}

		string itemType = inHands.GetType();
		foreach (string banned : m_Config.Market.Blacklist)
		{
			if (banned == itemType)
			{
				Notify(player, "#STR_SMP_00891", "#STR_SMP_01079");
				return;
			}
		}

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (GetCurrencyValue(itemType, currency) > 0)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00299");
			return;
		}

		int listingFee = GetPercentFee(price, m_Config.Market.ListingFeePercent);
		if (!ChargePlayerCurrency(player, listingFee, "#STR_SMP_00891", "#STR_SMP_00680"))
			return;

		int nested = 0;
		SM_MarketLot lot = new SM_MarketLot();
		lot.Item = CaptureItemTree(inHands, nested);
		lot.Id = m_MarketDB.NextLotId;
		m_MarketDB.NextLotId++;
		lot.ClanName = clan.Name;
		lot.SellerUid = uid;
		lot.SellerName = actor.Name;
		lot.Price = price;
		lot.Date = MakeDateStamp();
		lot.CreatedAtSeconds = GetNowAbsSeconds();
		lot.ReturnAvailableAtSeconds = 0;

		GetGame().ObjectDelete(inHands);

		m_MarketDB.Lots.Insert(lot);
		SaveMarketDB();

		AddLog(clan, actor.Name + "#STR_SMP_00035" + lot.Item.ClassName + "#STR_SMP_00042" + price);
		SaveDB();
		AddPersonalAchievementValue(uid, actor.Name, "MarketSells", 1, player);
		clan.Stats.MarketSells++;
		m_StatsDirty = true;
		CheckClanAchievementNotifications(clan);

		Notify(player, "#STR_SMP_00891", "#STR_SMP_00615" + lot.Item.ClassName + "#STR_SMP_00042" + price);
		BroadcastMarket();
		SyncClanState(clan);
	}

	protected void HandleMarketBuy(PlayerBase player, int lotId)
	{
		if (!m_Config.Market.Enabled || !player || !player.GetIdentity())
			return;

		if (!CheckMarketTakingZone(player))
			return;

		SM_MarketLot lot = FindLot(lotId);
		if (!lot)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00621");
			SendMarket(player);
			return;
		}

		SM_Clan sellerClan = FindClan(lot.ClanName);

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (currency.Count() == 0)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00294");
			return;
		}

		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		int total = 0;
		array<EntityAI> moneyItems = new array<EntityAI>;
		foreach (EntityAI invItem : items)
		{
			if (!invItem)
				continue;
			int value = GetCurrencyValue(invItem.GetType(), currency);
			if (value <= 0)
				continue;

			int count = 1;
			ItemBase moneyIb = ItemBase.Cast(invItem);
			if (moneyIb && moneyIb.CanBeSplit())
			{
				count = Math.Floor(moneyIb.GetQuantity());
				if (count < 1)
					count = 1;
			}

			total += value * count;
			moneyItems.Insert(invItem);
		}

		if (total < lot.Price)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00681" + lot.Price + "#STR_SMP_00163" + total);
			return;
		}

		foreach (EntityAI moneyItem : moneyItems)
			GetGame().ObjectDelete(moneyItem);

		int change = total - lot.Price;
		if (change > 0)
			IssueCurrency(player, change);

		EntityAI restored = RestoreItemTree(lot.Item, player, true);
		if (!restored)
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00769" + lot.Id + ")");

		int fee = lot.Price * m_Config.Market.FeePercent / 100;
		int income = lot.Price - fee;

		string buyerName = player.GetIdentity().GetName();
		if (sellerClan)
		{
			sellerClan.Treasury += income;
			AddLog(sellerClan, "#STR_SMP_00893" + lot.Item.ClassName + "#STR_SMP_00042" + lot.Price + " (+" + income + "#STR_SMP_00028");
			SaveDB();
			NotifyClan(sellerClan, "#STR_SMP_00891", "#STR_SMP_00860" + lot.Item.ClassName + "#STR_SMP_00042" + lot.Price + " (+" + income + "#STR_SMP_00028");
			SyncClanState(sellerClan);
		}

		m_MarketDB.Lots.RemoveItem(lot);
		SaveMarketDB();

		Notify(player, "#STR_SMP_00891", "#STR_SMP_00595" + lot.Item.ClassName + "#STR_SMP_00042" + lot.Price);
		AddPersonalAchievementValue(player.GetIdentity().GetPlainId(), player.GetIdentity().GetName(), "MarketBuys", 1, player);
		BroadcastMarket();

		Print(SM_PartyLoc.Text("#STR_SMP_00215" + buyerName + "#STR_SMP_00054" + lot.Id));
	}

	protected void HandleMarketCancel(PlayerBase player, int lotId)
	{
		if (!m_Config.Market.Enabled || !player || !player.GetIdentity())
			return;

		if (!CheckMarketTakingZone(player))
			return;

		SM_MarketLot lot = FindLot(lotId);
		if (!lot)
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = FindClanByMember(uid);
		if (!clan || clan.Name != lot.ClanName)
			return;

		if (uid != lot.SellerUid)
		{
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00957");
			return;
		}

		SM_ClanMember actor = clan.FindMember(uid);

		int minAgeSeconds = m_Config.Market.MinCancelAgeMinutes * 60;
		if (minAgeSeconds > 0 && GetCreatedAgeSeconds(lot.CreatedAtSeconds) < minAgeSeconds)
		{
			int ageLeft = minAgeSeconds - GetCreatedAgeSeconds(lot.CreatedAtSeconds);
			Notify(player, "#STR_SMP_00891", "#STR_SMP_01132" + SM_PartyUtil.FormatDuration(ageLeft));
			return;
		}

		int penalty = GetPercentFee(lot.Price, m_Config.Market.CancelPenaltyPercent);
		if (!ChargePlayerCurrency(player, penalty, "#STR_SMP_00891", "#STR_SMP_01137"))
			return;

		m_MarketDB.Lots.RemoveItem(lot);
		lot.ReturnAvailableAtSeconds = 0;
		if (m_Config.Market.ReturnDelaySeconds > 0)
			lot.ReturnAvailableAtSeconds = GetNowAbsSeconds() + m_Config.Market.ReturnDelaySeconds;

		if (GetPendingReturnSecondsLeft(lot.ReturnAvailableAtSeconds) > 0)
		{
			m_MarketDB.PendingReturns.Insert(lot);
			Notify(player, "#STR_SMP_00891", "#STR_SMP_01135" + SM_PartyUtil.FormatDuration(m_Config.Market.ReturnDelaySeconds));
		}
		else
		{
			RestoreItemTree(lot.Item, player, true);
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00619" + lot.Item.ClassName);
		}
		SaveMarketDB();

		AddLog(clan, actor.Name + "#STR_SMP_00086" + lot.Item.ClassName);
		SaveDB();

		BroadcastMarket();
		SyncClanState(clan);
	}

	protected void ReturnClanLots(string clanName)
	{
		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);

		bool changed = false;
		for (int i = m_MarketDB.Lots.Count() - 1; i >= 0; i--)
		{
			SM_MarketLot lot = m_MarketDB.Lots[i];
			if (lot.ClanName != clanName)
				continue;

			m_MarketDB.Lots.Remove(i);
			changed = true;

			lot.ReturnAvailableAtSeconds = 0;
			if (m_Config.Market.ReturnDelaySeconds > 0)
				lot.ReturnAvailableAtSeconds = GetNowAbsSeconds() + m_Config.Market.ReturnDelaySeconds;

			PlayerBase seller;
			bool sellerOnline = onlineMap.Find(lot.SellerUid, seller);
			if (sellerOnline && !ShouldQueueMarketReturnForPickup(lot.ReturnAvailableAtSeconds))
			{
				RestoreItemTree(lot.Item, seller, false);
				Notify(seller, "#STR_SMP_00891", "#STR_SMP_00537" + lot.Item.ClassName + "#STR_SMP_00032");
			}
			else
			{
				m_MarketDB.PendingReturns.Insert(lot);
				if (sellerOnline)
				{
					int left = GetPendingReturnSecondsLeft(lot.ReturnAvailableAtSeconds);
					if (left > 0)
						Notify(seller, "#STR_SMP_00891", "#STR_SMP_01135" + SM_PartyUtil.FormatDuration(left));
					else
						Notify(seller, "#STR_SMP_00891", "#STR_SMP_01139");
				}
			}
		}

		if (changed)
		{
			SaveMarketDB();
			BroadcastMarket();
		}
	}

	protected void GivePendingReturns(PlayerBase player, string uid)
	{
		bool changed = false;
		for (int i = m_MarketDB.PendingReturns.Count() - 1; i >= 0; i--)
		{
			SM_MarketLot lot = m_MarketDB.PendingReturns[i];
			if (lot.SellerUid != uid)
				continue;
			int left = GetPendingReturnSecondsLeft(lot.ReturnAvailableAtSeconds);
			if (left > 0)
			{
				Notify(player, "#STR_SMP_00891", "#STR_SMP_01131" + SM_PartyUtil.FormatDuration(left));
				continue;
			}
			if (!CheckMarketTakingZone(player))
				continue;

			m_MarketDB.PendingReturns.Remove(i);
			changed = true;

			RestoreItemTree(lot.Item, player, false);
			Notify(player, "#STR_SMP_00891", "#STR_SMP_00510" + lot.Item.ClassName + "#STR_SMP_00032");
		}

		if (changed)
			SaveMarketDB();
	}

	protected void IssueCurrency(PlayerBase player, int amount)
	{
		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);

		array<ref SM_CurrencyItem> sorted = new array<ref SM_CurrencyItem>;
		foreach (SM_CurrencyItem entry : currency)
		{
			int insertAt = sorted.Count();
			for (int sIdx = 0; sIdx < sorted.Count(); sIdx++)
			{
				if (entry.Value > sorted[sIdx].Value)
				{
					insertAt = sIdx;
					break;
				}
			}
			sorted.InsertAt(entry, insertAt);
		}

		int remaining = amount;
		int spawnedItems = 0;
		foreach (SM_CurrencyItem denom : sorted)
		{
			int needItems = remaining / denom.Value;
			while (needItems > 0 && spawnedItems < MAX_WITHDRAW_ITEMS)
			{
				ItemBase spawned = SpawnCurrencyItem(player, denom.Classname);
				if (!spawned)
					break;
				spawnedItems++;

				int put = 1;
				// Стак валюты: проверяем IsSplitable() (конфиг canBeSplit), а НЕ
				// CanBeSplit() — последний дополнительно требует текущее кол-во > 1,
				// поэтому на только что заспавненном предмете (кол-во 1) всегда
				// возвращает false, и валюта выдавалась бы по 1 штуке без стака.
				if (spawned.IsSplitable())
				{
					int maxQuantity = spawned.GetQuantityMax();
					if (maxQuantity < 1)
						maxQuantity = 1;
					put = Math.Min(needItems, maxQuantity);
					spawned.SetQuantity(put);
				}

				needItems -= put;
				remaining -= put * denom.Value;
			}
		}
	}

	protected string MakeDateStamp()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return FormatTwo(day) + "." + FormatTwo(month) + " " + FormatTwo(hour) + ":" + FormatTwo(minute);
	}

	static const string SERVER_MARKET_DIR = "$profile:SM_PartyMod\\ServerMarket";
	static const string SERVER_MARKET_STATE_PATH = "$profile:SM_PartyMod\\ServerMarket\\State.json";

	protected int GetNowAbsSeconds()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return AbsSeconds(year, month, day, hour, minute, second);
	}

	protected int GetServerMarketIntervalSeconds()
	{
		int seconds = m_Config.ServerMarket.RefreshIntervalSeconds;
		if (seconds < 60)
			seconds = 60;
		return seconds;
	}

	protected void SetServerMarketNextRefreshStamp()
	{
		int nextSeconds = GetNowAbsSeconds() + GetServerMarketIntervalSeconds();
		m_ServerMarketState.NextRefreshStampSeconds = nextSeconds;
		m_ServerMarketState.NextRefreshStampMinutes = Math.Ceil(nextSeconds / 60.0);
	}

	protected void LoadServerMarketState()
	{
		if (!FileExist(SERVER_MARKET_DIR))
			MakeDirectory(SERVER_MARKET_DIR);

		m_ServerMarketState = new SM_ServerMarketState();

		if (m_Config.ServerMarket.SaveState && FileExist(SERVER_MARKET_STATE_PATH))
		{
			string errorMessage;
			if (!JsonFileLoader<SM_ServerMarketState>.LoadFile(SERVER_MARKET_STATE_PATH, m_ServerMarketState, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00206" + errorMessage));
				m_ServerMarketState = new SM_ServerMarketState();
			}
		}

		PrepareServerMarketState();

		if (m_Config.ServerMarket.Enabled)
		{
			if (m_Config.ServerMarket.RefreshOnServerStart)
			{
				GenerateServerMarketRotation(false);
			}
			else
			{
				EnsureServerMarketReady(false);
			}
		}
	}

	protected void PrepareServerMarketState()
	{
		if (!m_ServerMarketState)
			m_ServerMarketState = new SM_ServerMarketState();
		if (!m_ServerMarketState.Items)
			m_ServerMarketState.Items = new array<ref SM_ServerMarketEntry>;
		if (m_ServerMarketState.RotationId < 1)
			m_ServerMarketState.RotationId = 1;
		if (m_ServerMarketState.NextItemId < 1)
			m_ServerMarketState.NextItemId = 1;
		if (m_ServerMarketState.NextRefreshStampSeconds <= 0 && m_ServerMarketState.NextRefreshStampMinutes > 0)
			m_ServerMarketState.NextRefreshStampSeconds = m_ServerMarketState.NextRefreshStampMinutes * 60;
		if (m_ServerMarketState.NextRefreshStampSeconds > 0)
			m_ServerMarketState.NextRefreshStampMinutes = Math.Ceil(m_ServerMarketState.NextRefreshStampSeconds / 60.0);

		for (int i = m_ServerMarketState.Items.Count() - 1; i >= 0; i--)
		{
			SM_ServerMarketEntry entry = m_ServerMarketState.Items[i];
			if (!entry || entry.ClassName == "")
			{
				m_ServerMarketState.Items.Remove(i);
				continue;
			}

			entry.ReservedUid = "";
			entry.ReservedUntil = 0;
			if (!entry.Purchases)
				entry.Purchases = new array<ref SM_ServerMarketPurchase>;
			if (entry.Id >= m_ServerMarketState.NextItemId)
				m_ServerMarketState.NextItemId = entry.Id + 1;
			if (entry.InitialQuantity < 1)
				entry.InitialQuantity = 1;
			if (entry.Quantity < 0)
				entry.Quantity = 0;
			if (entry.Quantity > entry.InitialQuantity)
				entry.Quantity = entry.InitialQuantity;
			if (entry.BasePrice < 1)
				entry.BasePrice = 1;
			if (entry.MaxBuyPerPlayer < 0)
				entry.MaxBuyPerPlayer = 0;

			for (int p = entry.Purchases.Count() - 1; p >= 0; p--)
			{
				SM_ServerMarketPurchase purchase = entry.Purchases[p];
				if (!purchase || purchase.Uid == "" || purchase.Bought < 1)
					entry.Purchases.Remove(p);
			}
		}
	}

	protected void SaveServerMarketState()
	{
		if (!m_Config.ServerMarket.SaveState)
			return;
		if (!FileExist(SERVER_MARKET_DIR))
			MakeDirectory(SERVER_MARKET_DIR);

		string errorMessage;
		if (!JsonFileLoader<SM_ServerMarketState>.SaveFile(SERVER_MARKET_STATE_PATH, m_ServerMarketState, errorMessage))
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00190" + errorMessage));
	}

	protected bool IsServerMarketExpired()
	{
		if (!m_ServerMarketState)
			return true;
		if (m_ServerMarketState.NextRefreshStampSeconds <= 0)
			return true;
		if (GetNowAbsSeconds() >= m_ServerMarketState.NextRefreshStampSeconds)
			return true;
		return false;
	}

	protected void EnsureServerMarketReady(bool broadcast)
	{
		if (!m_Config.ServerMarket.Enabled)
			return;
		if (!m_ServerMarketState)
			m_ServerMarketState = new SM_ServerMarketState();
		PrepareServerMarketState();

		if (m_ServerMarketState.Items.Count() == 0)
		{
			if (HasServerMarketEligibleItems())
				GenerateServerMarketRotation(broadcast);
			return;
		}
		if (IsServerMarketExpired())
			GenerateServerMarketRotation(broadcast);
	}

	protected bool ServerMarketConfigAlreadySelected(array<ref SM_ServerMarketItemConfig> selected, SM_ServerMarketItemConfig config)
	{
		if (!selected || !config)
			return true;
		foreach (SM_ServerMarketItemConfig selectedConfig : selected)
		{
			if (selectedConfig && selectedConfig.ClassName == config.ClassName)
				return true;
		}
		return false;
	}

	protected bool HasServerMarketEligibleItems()
	{
		foreach (SM_ServerMarketItemConfig itemConfig : m_Config.ServerMarket.Items)
		{
			if (!itemConfig)
				continue;
			if (itemConfig.ClassName == "")
				continue;
			if (itemConfig.SpawnChance <= 0.0)
				continue;
			return true;
		}
		return false;
	}

	protected void GenerateServerMarketRotation(bool broadcast)
	{
		if (!HasServerMarketEligibleItems())
		{
			int oldEmptyRotationId = 0;
			if (m_ServerMarketState)
				oldEmptyRotationId = m_ServerMarketState.RotationId;
			m_ServerMarketState = new SM_ServerMarketState();
			m_ServerMarketState.RotationId = oldEmptyRotationId + 1;
			SetServerMarketNextRefreshStamp();
			SaveServerMarketState();
			if (broadcast)
				BroadcastServerMarket();
			return;
		}

		int oldRotationId = 0;
		if (m_ServerMarketState)
			oldRotationId = m_ServerMarketState.RotationId;

		m_ServerMarketState = new SM_ServerMarketState();
		m_ServerMarketState.RotationId = oldRotationId + 1;
		if (m_ServerMarketState.RotationId < 1)
			m_ServerMarketState.RotationId = 1;
		SetServerMarketNextRefreshStamp();

		array<ref SM_ServerMarketItemConfig> candidates = new array<ref SM_ServerMarketItemConfig>;
		foreach (SM_ServerMarketItemConfig itemConfig : m_Config.ServerMarket.Items)
		{
			if (!itemConfig)
				continue;
			if (itemConfig.ClassName == "")
				continue;
			if (itemConfig.SpawnChance <= 0.0)
				continue;
			if (Math.RandomFloat01() <= itemConfig.SpawnChance)
				candidates.Insert(itemConfig);
		}

		array<ref SM_ServerMarketItemConfig> selected = new array<ref SM_ServerMarketItemConfig>;
		int need = m_Config.ServerMarket.GeneratedItemsCount;
		while (selected.Count() < need && candidates.Count() > 0)
		{
			int idx = Math.RandomInt(0, candidates.Count());
			SM_ServerMarketItemConfig picked = candidates[idx];
			candidates.Remove(idx);
			if (ServerMarketConfigAlreadySelected(selected, picked))
				continue;
			selected.Insert(picked);
		}

		if (selected.Count() < need)
		{
			foreach (SM_ServerMarketItemConfig fallbackConfig : m_Config.ServerMarket.Items)
			{
				if (selected.Count() >= need)
					break;
				if (!fallbackConfig || fallbackConfig.ClassName == "")
					continue;
				if (fallbackConfig.SpawnChance <= 0.0)
					continue;
				if (ServerMarketConfigAlreadySelected(selected, fallbackConfig))
					continue;
				selected.Insert(fallbackConfig);
			}
		}

		foreach (SM_ServerMarketItemConfig selectedConfig : selected)
		{
			SM_ServerMarketEntry entry = new SM_ServerMarketEntry();
			entry.Id = m_ServerMarketState.NextItemId;
			m_ServerMarketState.NextItemId++;
			entry.ClassName = selectedConfig.ClassName;
			entry.InitialQuantity = Math.RandomIntInclusive(selectedConfig.MinQuantity, selectedConfig.MaxQuantity);
			entry.Quantity = entry.InitialQuantity;
			entry.BasePrice = Math.RandomIntInclusive(selectedConfig.MinPrice, selectedConfig.MaxPrice);
			entry.MaxBuyPerPlayer = selectedConfig.MaxBuyPerPlayer;
			m_ServerMarketState.Items.Insert(entry);
		}

		SaveServerMarketState();

		Print(SM_PartyLoc.Text("#STR_SMP_00217" + m_ServerMarketState.RotationId + "#STR_SMP_00162" + m_ServerMarketState.Items.Count()));
		if (broadcast)
			BroadcastServerMarket();
	}

	protected void ServerMarketTick()
	{
		if (!m_Config.ServerMarket.Enabled)
			return;
		if (IsServerMarketExpired())
			GenerateServerMarketRotation(true);
	}

	protected SM_ServerMarketEntry FindServerMarketEntry(int itemId)
	{
		if (!m_ServerMarketState || !m_ServerMarketState.Items)
			return null;
		foreach (SM_ServerMarketEntry entry : m_ServerMarketState.Items)
		{
			if (entry && entry.Id == itemId)
				return entry;
		}
		return null;
	}

	protected int GetServerMarketEntryIndex(SM_ServerMarketEntry entry)
	{
		if (!entry || !m_ServerMarketState || !m_ServerMarketState.Items)
			return -1;
		for (int i = 0; i < m_ServerMarketState.Items.Count(); i++)
		{
			if (m_ServerMarketState.Items[i] == entry)
				return i;
		}
		return -1;
	}

	protected int GetServerMarketPlayerClanLevel(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return 0;
		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
			return 0;
		if (!m_Config.Levels.Enabled)
			return 0;
		return clan.Level;
	}

	protected int GetServerMarketVisibleLimit(int clanLevel)
	{
		int visible;
		if (clanLevel > 0)
			visible = m_Config.GetServerMarketItems(clanLevel);
		else
			visible = m_Config.ServerMarket.DefaultVisibleItems;
		if (visible < 1)
			visible = 1;
		return visible;
	}

	protected int GetServerMarketRequiredLevelForSlot(int slotNumber)
	{
		if (slotNumber <= m_Config.ServerMarket.DefaultVisibleItems)
			return 0;

		int required = -1;
		int maxLevel = m_Config.GetMaxLevel();
		for (int level = 1; level <= maxLevel; level++)
		{
			int levelVisibleItems = m_Config.GetServerMarketItems(level);
			if (levelVisibleItems < slotNumber)
				continue;
			if (required < 0 || level < required)
				required = level;
		}

		if (required < 0)
			return -1;
		return required;
	}

	protected int GetServerMarketCurrentPrice(SM_ServerMarketEntry entry)
	{
		if (!entry)
			return 0;
		int price = entry.BasePrice;
		if (!m_Config.ServerMarket.EnableDynamicPrice)
			return price;
		if (entry.InitialQuantity <= 0 || entry.Quantity <= 0)
			return price;

		int stockPercent = Math.Round(entry.Quantity * 100.0 / entry.InitialQuantity);
		float multiplier = 1.0;
		foreach (SM_ServerMarketPriceStep step : m_Config.ServerMarket.DynamicPriceSteps)
		{
			if (!step)
				continue;
			if (stockPercent <= step.StockPercentBelow && step.PriceMultiplier > multiplier)
				multiplier = step.PriceMultiplier;
		}

		price = Math.Round(entry.BasePrice * multiplier);
		if (price < 1)
			price = 1;
		return price;
	}

	protected SM_ServerMarketPurchase FindServerMarketPurchase(SM_ServerMarketEntry entry, string uid)
	{
		if (!entry || !entry.Purchases)
			return null;
		foreach (SM_ServerMarketPurchase purchase : entry.Purchases)
		{
			if (purchase && purchase.Uid == uid)
				return purchase;
		}
		return null;
	}

	protected int GetServerMarketBought(SM_ServerMarketEntry entry, string uid)
	{
		SM_ServerMarketPurchase purchase = FindServerMarketPurchase(entry, uid);
		if (purchase)
			return purchase.Bought;
		return 0;
	}

	protected void AddServerMarketBought(SM_ServerMarketEntry entry, string uid, int amount)
	{
		if (!entry || uid == "" || amount <= 0)
			return;
		if (!entry.Purchases)
			entry.Purchases = new array<ref SM_ServerMarketPurchase>;

		SM_ServerMarketPurchase purchase = FindServerMarketPurchase(entry, uid);
		if (!purchase)
		{
			purchase = new SM_ServerMarketPurchase(uid, 0);
			entry.Purchases.Insert(purchase);
		}
		purchase.Bought += amount;
	}

	protected int GetPlayerCurrencyTotal(PlayerBase player, array<ref SM_CurrencyItem> currency)
	{
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		int total = 0;
		foreach (EntityAI invItem : items)
		{
			if (!invItem)
				continue;
			int value = GetCurrencyValue(invItem.GetType(), currency);
			if (value <= 0)
				continue;

			int count = 1;
			ItemBase moneyIb = ItemBase.Cast(invItem);
			if (moneyIb && moneyIb.CanBeSplit())
			{
				count = Math.Floor(moneyIb.GetQuantity());
				if (count < 1)
					count = 1;
			}

			total += value * count;
		}
		return total;
	}

	protected void ClearServerMarketReservation(SM_ServerMarketEntry entry)
	{
		if (!entry)
			return;
		entry.ReservedUid = "";
		entry.ReservedUntil = 0;
	}

	protected bool IsServerMarketReservedByOther(SM_ServerMarketEntry entry, string uid)
	{
		if (!entry)
			return false;
		if (entry.ReservedUid == "")
			return false;
		if (entry.ReservedUid == uid)
			return false;
		if (entry.ReservedUntil <= GetGame().GetTickTime())
		{
			ClearServerMarketReservation(entry);
			return false;
		}
		return true;
	}

	protected SM_MarketItem CreateServerMarketItemData(SM_ServerMarketEntry entry)
	{
		SM_MarketItem item = new SM_MarketItem();
		item.ClassName = entry.ClassName;
		item.Health = 1;
		item.Quantity = -1;
		item.LiquidType = -1;
		item.AmmoCount = -1;
		return item;
	}

	protected int GetServerMarketNextRefreshSeconds()
	{
		if (!m_ServerMarketState)
			return 0;
		int nextSeconds = m_ServerMarketState.NextRefreshStampSeconds;
		if (nextSeconds <= 0 && m_ServerMarketState.NextRefreshStampMinutes > 0)
			nextSeconds = m_ServerMarketState.NextRefreshStampMinutes * 60;
		int seconds = nextSeconds - GetNowAbsSeconds();
		if (seconds < 0)
			seconds = 0;
		return seconds;
	}

	void SendServerMarket(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;
		if (!m_Config.ServerMarket.Enabled)
			return;

		EnsureServerMarketReady(false);

		string uid = player.GetIdentity().GetPlainId();
		int clanLevel = GetServerMarketPlayerClanLevel(player);
		int visibleLimit = GetServerMarketVisibleLimit(clanLevel);
		if (visibleLimit > m_ServerMarketState.Items.Count())
			visibleLimit = m_ServerMarketState.Items.Count();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_ServerMarketState.RotationId);
		rpc.Write(GetServerMarketNextRefreshSeconds());
		rpc.Write(visibleLimit);
		rpc.Write(m_ServerMarketState.Items.Count());
		rpc.Write(m_ServerMarketState.Items.Count());

		for (int i = 0; i < m_ServerMarketState.Items.Count(); i++)
		{
			SM_ServerMarketEntry entry = m_ServerMarketState.Items[i];
			int slotNumber = i + 1;
			bool locked = slotNumber > visibleLimit;
			int requiredClanLevel = GetServerMarketRequiredLevelForSlot(slotNumber);

			rpc.Write(entry.Id);
			rpc.Write(locked);
			rpc.Write(requiredClanLevel);
			if (!locked)
			{
				rpc.Write(entry.ClassName);
				rpc.Write(entry.BasePrice);
				rpc.Write(GetServerMarketCurrentPrice(entry));
				rpc.Write(entry.InitialQuantity);
				rpc.Write(entry.Quantity);
				rpc.Write(entry.MaxBuyPerPlayer);
				rpc.Write(GetServerMarketBought(entry, uid));
			}
		}

		rpc.Send(player, SM_PartyRPC.SYNC_SERVER_MARKET, true, player.GetIdentity());
	}

	protected void BroadcastServerMarket()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man man : players)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
				SendServerMarket(pb);
		}
	}

	protected void HandleServerMarketBuy(PlayerBase player, int itemId)
	{
		if (!m_Config.ServerMarket.Enabled)
			return;
		if (!player || !player.GetIdentity())
			return;

		EnsureServerMarketReady(false);

		SM_ServerMarketEntry entry = FindServerMarketEntry(itemId);
		if (!entry)
		{
			Notify(player, "#STR_SMP_00913", "#STR_SMP_00995");
			SendServerMarket(player);
			return;
		}

		string uid = player.GetIdentity().GetPlainId();
		int index = GetServerMarketEntryIndex(entry);
		int slotNumber = index + 1;
		int clanLevel = GetServerMarketPlayerClanLevel(player);
		int visibleLimit = GetServerMarketVisibleLimit(clanLevel);
		if (slotNumber < 1 || slotNumber > visibleLimit)
		{
			int requiredClanLevel = GetServerMarketRequiredLevelForSlot(slotNumber);
			if (requiredClanLevel >= 0)
				Notify(player, "#STR_SMP_00913", "#STR_SMP_01080" + requiredClanLevel.ToString());
			else
				Notify(player, "#STR_SMP_00913", "#STR_SMP_01081");
			SendServerMarket(player);
			return;
		}

		if (entry.Quantity <= 0)
		{
			Notify(player, "#STR_SMP_00913", "#STR_SMP_00996");
			SendServerMarket(player);
			return;
		}

		if (IsServerMarketReservedByOther(entry, uid))
		{
			Notify(player, "#STR_SMP_00913", "#STR_SMP_00997");
			SendServerMarket(player);
			return;
		}

		int bought = GetServerMarketBought(entry, uid);
		if (entry.MaxBuyPerPlayer > 0 && bought >= entry.MaxBuyPerPlayer)
		{
			Notify(player, "#STR_SMP_00913", "#STR_SMP_00305");
			SendServerMarket(player);
			return;
		}

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (currency.Count() == 0)
		{
			Notify(player, "#STR_SMP_00913", "#STR_SMP_00294");
			return;
		}

		int price = GetServerMarketCurrentPrice(entry);
		int total = GetPlayerCurrencyTotal(player, currency);
		if (total < price)
		{
			Notify(player, "#STR_SMP_00913", "#STR_SMP_00681" + price + "#STR_SMP_00163" + total);
			return;
		}

		entry.ReservedUid = uid;
		entry.ReservedUntil = GetGame().GetTickTime() + m_Config.ServerMarket.PurchaseReservationSeconds;

		int consumed = ConsumeAllCurrency(player, currency);
		int change = consumed - price;
		if (consumed < price)
		{
			if (consumed > 0)
				IssueCurrency(player, consumed);
			ClearServerMarketReservation(entry);
			Notify(player, "#STR_SMP_00913", "#STR_SMP_00677");
			SendServerMarket(player);
			return;
		}

		SM_MarketItem itemData = CreateServerMarketItemData(entry);
		EntityAI restored = RestoreItemTree(itemData, player, true);
		if (!restored)
		{
			if (consumed > 0)
				IssueCurrency(player, consumed);
			ClearServerMarketReservation(entry);
			Notify(player, "#STR_SMP_00913", "#STR_SMP_00771" + entry.ClassName);
			SendServerMarket(player);
			return;
		}

		if (change > 0)
			IssueCurrency(player, change);

		entry.Quantity--;
		AddServerMarketBought(entry, uid, 1);
		ClearServerMarketReservation(entry);
		SaveServerMarketState();

		string itemName = SM_PartyUtil.GetItemDisplayName(entry.ClassName);
		Notify(player, "#STR_SMP_00913", "#STR_SMP_00595" + itemName + "#STR_SMP_00042" + price);
		Print(SM_PartyLoc.Text("#STR_SMP_00216" + player.GetIdentity().GetName() + "#STR_SMP_00053" + entry.ClassName + "#STR_SMP_00042" + price));
		BroadcastServerMarket();
	}


	int GetClanStorageCap(SM_Clan clan)
	{
		if (!clan)
			return 0;
		int level = 1;
		if (m_Config.Levels.Enabled)
			level = clan.Level;
		return m_Config.GetStorageSlots(level);
	}

	int GetClanLotCap(SM_Clan clan)
	{
		if (!clan)
			return m_Config.Market.MaxLotsPerClan;
		int level = 1;
		if (m_Config.Levels.Enabled)
			level = clan.Level;
		return m_Config.GetMarketLots(level);
	}


	static const string STORAGE_DB_DIR = "$profile:SM_PartyMod\\Storage";
	static const string STORAGE_INDEX_PATH = "$profile:SM_PartyMod\\Storage\\Index.json";
	static const string STORAGE_PENDING_PATH = "$profile:SM_PartyMod\\Storage\\PendingReturns.json";

	protected void LoadStorageDB()
	{
		if (!FileExist(STORAGE_DB_DIR))
			MakeDirectory(STORAGE_DB_DIR);

		m_StorageDB = new SM_StorageDB();

		if (FileExist(STORAGE_INDEX_PATH))
			LoadStorageFileDB();

		PrepareStorageDB();
	}

	protected bool LoadStorageFileDB()
	{
		SM_StorageIndex index = new SM_StorageIndex();
		string errorMessage;
		if (!JsonFileLoader<SM_StorageIndex>.LoadFile(STORAGE_INDEX_PATH, index, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00208" + errorMessage));
			return false;
		}
		if (!index)
			return false;
		if (!index.Clans)
			index.Clans = new array<ref SM_StorageClanIndexEntry>;

		m_StorageDB.NextItemId = index.NextItemId;
		foreach (SM_StorageClanIndexEntry entry : index.Clans)
		{
			if (!entry)
				continue;
			if (!IsSafeClanFileName(entry.FileName))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00214" + entry.FileName));
				continue;
			}

			SM_StorageClanFile storageFile = new SM_StorageClanFile();
			string storagePath = STORAGE_DB_DIR + "\\" + entry.FileName;
			if (!JsonFileLoader<SM_StorageClanFile>.LoadFile(storagePath, storageFile, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00212" + storagePath + ": " + errorMessage));
				continue;
			}
			if (!storageFile || !storageFile.Items)
				continue;
			foreach (SM_StorageItem item : storageFile.Items)
			{
				if (!item)
					continue;
				if (item.ClanName == "")
				{
					if (storageFile.ClanName != "")
						item.ClanName = storageFile.ClanName;
					else
						item.ClanName = entry.ClanName;
				}
				m_StorageDB.Items.Insert(item);
			}
		}

		LoadStoragePendingReturns();
		return true;
	}

	protected void LoadStoragePendingReturns()
	{
		if (!FileExist(STORAGE_PENDING_PATH))
			return;

		SM_StoragePendingReturns pending = new SM_StoragePendingReturns();
		string errorMessage;
		if (!JsonFileLoader<SM_StoragePendingReturns>.LoadFile(STORAGE_PENDING_PATH, pending, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00209" + errorMessage));
			return;
		}
		if (pending && pending.PendingReturns)
			m_StorageDB.PendingReturns = pending.PendingReturns;
	}

	protected void PrepareStorageDB()
	{
		if (!m_StorageDB)
			m_StorageDB = new SM_StorageDB();
		if (!m_StorageDB.Items)
			m_StorageDB.Items = new array<ref SM_StorageItem>;
		if (!m_StorageDB.PendingReturns)
			m_StorageDB.PendingReturns = new array<ref SM_StorageItem>;

		for (int i = m_StorageDB.Items.Count() - 1; i >= 0; i--)
		{
			if (!m_StorageDB.Items[i] || !m_StorageDB.Items[i].Item)
				m_StorageDB.Items.Remove(i);
		}
		for (int p = m_StorageDB.PendingReturns.Count() - 1; p >= 0; p--)
		{
			if (!m_StorageDB.PendingReturns[p] || !m_StorageDB.PendingReturns[p].Item)
				m_StorageDB.PendingReturns.Remove(p);
		}

		if (m_StorageDB.NextItemId < 1)
			m_StorageDB.NextItemId = 1;
		foreach (SM_StorageItem item : m_StorageDB.Items)
		{
			if (item.Id >= m_StorageDB.NextItemId)
				m_StorageDB.NextItemId = item.Id + 1;
		}
		foreach (SM_StorageItem pendingItem : m_StorageDB.PendingReturns)
		{
			if (pendingItem.Id >= m_StorageDB.NextItemId)
				m_StorageDB.NextItemId = pendingItem.Id + 1;
		}
	}

	protected string GetStorageClanId(string clanName)
	{
		SM_Clan clan = FindClan(clanName);
		if (clan && clan.Id != "")
			return clan.Id;
		return MakeClanId(clanName);
	}

	protected string GetStorageFileName(string clanId)
	{
		return "storage_" + NormalizeClanId(clanId) + ".json";
	}

	protected SM_StorageIndex LoadExistingStorageIndex()
	{
		if (!FileExist(STORAGE_INDEX_PATH))
			return null;

		SM_StorageIndex index = new SM_StorageIndex();
		string errorMessage;
		if (!JsonFileLoader<SM_StorageIndex>.LoadFile(STORAGE_INDEX_PATH, index, errorMessage))
			return null;
		if (!index || !index.Clans)
			return null;
		return index;
	}

	protected bool HasStorageFileInIndex(SM_StorageIndex index, string fileName)
	{
		if (!index || !index.Clans)
			return false;

		foreach (SM_StorageClanIndexEntry entry : index.Clans)
		{
			if (entry && entry.FileName == fileName)
				return true;
		}
		return false;
	}

	protected void CleanupDeletedStorageFiles(SM_StorageIndex oldIndex, SM_StorageIndex newIndex)
	{
		if (!oldIndex || !oldIndex.Clans)
			return;

		foreach (SM_StorageClanIndexEntry entry : oldIndex.Clans)
		{
			if (!entry)
				continue;
			if (!IsSafeClanFileName(entry.FileName))
				continue;
			if (HasStorageFileInIndex(newIndex, entry.FileName))
				continue;
			DeleteFile(STORAGE_DB_DIR + "\\" + entry.FileName);
		}
	}

	protected void SaveStorageDB()
	{
		string errorMessage;
		if (!FileExist(STORAGE_DB_DIR))
			MakeDirectory(STORAGE_DB_DIR);

		PrepareStorageDB();

		SM_StorageIndex oldIndex = LoadExistingStorageIndex();
		SM_StorageIndex index = new SM_StorageIndex();
		index.NextItemId = m_StorageDB.NextItemId;

		map<string, ref SM_StorageClanFile> storageFiles = new map<string, ref SM_StorageClanFile>;
		foreach (SM_StorageItem item : m_StorageDB.Items)
		{
			if (!item)
				continue;

			string clanName = item.ClanName;
			if (clanName == "")
				clanName = "unknown";

			string clanId = GetStorageClanId(clanName);
			string fileName = GetStorageFileName(clanId);

			SM_StorageClanFile storageFile;
			if (!storageFiles.Find(fileName, storageFile))
			{
				storageFile = new SM_StorageClanFile();
				storageFile.ClanId = clanId;
				storageFile.ClanName = clanName;
				storageFiles.Set(fileName, storageFile);
				index.Clans.Insert(new SM_StorageClanIndexEntry(clanId, clanName, fileName));
			}
			storageFile.Items.Insert(item);
		}

		bool saveFailed = false;
		for (int i = 0; i < storageFiles.Count(); i++)
		{
			string fileNameToSave = storageFiles.GetKey(i);
			SM_StorageClanFile fileData = storageFiles.GetElement(i);
			if (!JsonFileLoader<SM_StorageClanFile>.SaveFile(STORAGE_DB_DIR + "\\" + fileNameToSave, fileData, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00196" + fileNameToSave + ": " + errorMessage));
				saveFailed = true;
			}
		}

		if (saveFailed)
			return;

		SM_StoragePendingReturns pending = new SM_StoragePendingReturns();
		pending.PendingReturns = m_StorageDB.PendingReturns;
		if (!JsonFileLoader<SM_StoragePendingReturns>.SaveFile(STORAGE_PENDING_PATH, pending, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00193" + errorMessage));
			return;
		}

		if (!JsonFileLoader<SM_StorageIndex>.SaveFile(STORAGE_INDEX_PATH, index, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00192" + errorMessage));
			return;
		}

		CleanupDeletedStorageFiles(oldIndex, index);
	}

	protected void SaveStorageDBForClan(string clanName)
	{
		string errorMessage;
		if (!FileExist(STORAGE_DB_DIR))
			MakeDirectory(STORAGE_DB_DIR);

		PrepareStorageDB();

		string targetClanName = clanName;
		if (targetClanName == "")
			targetClanName = "unknown";
		string targetClanId = GetStorageClanId(targetClanName);
		string targetFileName = GetStorageFileName(targetClanId);

		SM_StorageIndex oldIndex = LoadExistingStorageIndex();
		SM_StorageIndex index = new SM_StorageIndex();
		index.NextItemId = m_StorageDB.NextItemId;

		SM_StorageClanFile targetFile = new SM_StorageClanFile();
		targetFile.ClanId = targetClanId;
		targetFile.ClanName = targetClanName;

		foreach (SM_StorageItem item : m_StorageDB.Items)
		{
			if (!item)
				continue;

			string itemClanName = item.ClanName;
			if (itemClanName == "")
				itemClanName = "unknown";

			string itemClanId = GetStorageClanId(itemClanName);
			string fileName = GetStorageFileName(itemClanId);
			if (!HasStorageFileInIndex(index, fileName))
				index.Clans.Insert(new SM_StorageClanIndexEntry(itemClanId, itemClanName, fileName));

			if (itemClanName == targetClanName)
				targetFile.Items.Insert(item);
		}

		if (targetFile.Items.Count() > 0)
		{
			if (!JsonFileLoader<SM_StorageClanFile>.SaveFile(STORAGE_DB_DIR + "\\" + targetFileName, targetFile, errorMessage))
			{
				ErrorEx(SM_PartyLoc.Text("#STR_SMP_00196" + targetFileName + ": " + errorMessage));
				return;
			}
		}

		if (!JsonFileLoader<SM_StorageIndex>.SaveFile(STORAGE_INDEX_PATH, index, errorMessage))
		{
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00192" + errorMessage));
			return;
		}

		CleanupDeletedStorageFiles(oldIndex, index);
	}

	protected SM_StorageItem FindStorageItem(int itemId)
	{
		foreach (SM_StorageItem item : m_StorageDB.Items)
		{
			if (item.Id == itemId)
				return item;
		}
		return null;
	}

	protected int CountClanStorage(string clanName)
	{
		int total = 0;
		foreach (SM_StorageItem item : m_StorageDB.Items)
		{
			if (item.ClanName == clanName)
				total++;
		}
		return total;
	}

	void SetClanBaseFromFlag(PlayerBase player, TerritoryFlag flag)
	{
		if (!m_Config.Storage.Enabled)
			return;
		if (!player || !player.GetIdentity() || !flag)
			return;
		if (flag.IsDamageDestroyed())
		{
			Notify(player, "#STR_SMP_00261", "#STR_SMP_00698");
			return;
		}
		if (!flag.SM_IsRaisedForClanBase())
		{
			Notify(player, "#STR_SMP_00261", "#STR_SMP_00950");
			return;
		}

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
		{
			Notify(player, "#STR_SMP_00261", "#STR_SMP_00831");
			return;
		}

		if (!HasActionAccess(player, clan, SM_ClanAction.SET_BASE, "#STR_SMP_00261", "#STR_SMP_00691"))
			return;

		SM_Clan ownerClan = FindClanByBaseFlag(flag);
		if (ownerClan)
		{
			if (ownerClan == clan)
				Notify(player, "#STR_SMP_00261", "#STR_SMP_01082");
			else
				Notify(player, "#STR_SMP_00261", "#STR_SMP_01083");
			return;
		}

		bool staleBaseRebind = false;
		if (clan.HasBase)
		{
			if (ClanBaseFlagExists(clan))
			{
				Notify(player, "#STR_SMP_00261", "#STR_SMP_00948");
				return;
			}

			staleBaseRebind = true;
			Notify(player, "#STR_SMP_00261", "#STR_SMP_00976");
		}

		clan.BasePos = flag.GetPosition();
		clan.HasBase = true;
		string actorName = GetActorName(player, clan);
		if (staleBaseRebind)
			AddLog(clan, actorName + "#STR_SMP_00070");
		else
			AddLog(clan, actorName + "#STR_SMP_00076");
		SendBaseRegisterAudit(clan, player, staleBaseRebind);
		SaveDB();

		Notify(player, "#STR_SMP_00261", "#STR_SMP_01045");
		if (staleBaseRebind)
			NotifyClan(clan, "#STR_SMP_00261", actorName + "#STR_SMP_00069", uid);
		else
			NotifyClan(clan, "#STR_SMP_00261", actorName + "#STR_SMP_00076", uid);
		SyncClanState(clan);
		SyncAdminStates();
	}

	void UnsetClanBaseFromFlag(PlayerBase player, TerritoryFlag flag)
	{
		if (!m_Config.Storage.Enabled)
			return;
		if (!player || !player.GetIdentity() || !flag)
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);
		if (!clan)
		{
			Notify(player, "#STR_SMP_00261", "#STR_SMP_00747");
			return;
		}

		if (!HasActionAccess(player, clan, SM_ClanAction.SET_BASE, "#STR_SMP_00261", "#STR_SMP_00690"))
			return;

		SM_Clan ownerClan = FindClanByBaseFlag(flag);
		if (ownerClan && ownerClan != clan)
		{
			Notify(player, "#STR_SMP_00261", "#STR_SMP_00697");
			return;
		}

		if (!clan.HasBase)
		{
			Notify(player, "#STR_SMP_00261", "#STR_SMP_01016");
			return;
		}

		if (!IsSameBaseFlag(clan.BasePos, flag.GetPosition()))
		{
			Notify(player, "#STR_SMP_00261", "#STR_SMP_00748");
			return;
		}

		string unsetActorName = GetActorName(player, clan);
		AddLog(clan, unsetActorName + "#STR_SMP_00065");
		SendBaseUnregisterAudit(clan, player);
		clan.HasBase = false;
		clan.BasePos = "0 0 0";
		SaveDB();

		Notify(player, "#STR_SMP_00261", "#STR_SMP_00265");
		NotifyClan(clan, "#STR_SMP_00261", unsetActorName + "#STR_SMP_00065", uid);
		SyncClanState(clan);
		SyncAdminStates();
	}

	protected bool IsPlayerAtClanBase(PlayerBase player, SM_Clan clan)
	{
		if (!player || !clan || !clan.HasBase)
			return false;

		float radius = m_Config.Storage.RadiusMeters;
		if (vector.Distance(player.GetPosition(), clan.BasePos) > radius)
			return false;

		if (ClanBaseFlagExists(clan))
			return true;
		return false;
	}

	protected void HandleStorageDeposit(PlayerBase player)
	{
		if (!m_Config.Storage.Enabled)
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00809");
			return;
		}

		if (!IsPlayerAtClanBase(player, clan))
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00567");
			return;
		}

		int cap = GetClanStorageCap(clan);
		int used = CountClanStorage(clan.Name);
		if (used >= cap)
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00921" + used + "/" + cap + "#STR_SMP_00141");
			return;
		}

		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		if (!inHands)
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00336");
			return;
		}

		string itemType = inHands.GetType();
		foreach (string banned : m_Config.Market.Blacklist)
		{
			if (banned == itemType)
			{
				Notify(player, "#STR_SMP_00919", "#STR_SMP_01078");
				return;
			}
		}

		array<ref SM_CurrencyItem> currency;
		m_Config.GetValidCurrency(currency);
		if (GetCurrencyValue(itemType, currency) > 0)
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00298");
			return;
		}

		SM_ClanMember actor = clan.FindMember(uid);

		int nested = 0;
		SM_StorageItem stored = new SM_StorageItem();
		stored.Item = CaptureItemTree(inHands, nested);
		stored.Id = m_StorageDB.NextItemId;
		m_StorageDB.NextItemId++;
		stored.ClanName = clan.Name;
		stored.DepositorUid = uid;
		stored.DepositorName = actor.Name;
		stored.Date = MakeDateStamp();

		GetGame().ObjectDelete(inHands);

		m_StorageDB.Items.Insert(stored);
		SaveStorageDBForClan(clan.Name);

		string itemName = SM_PartyUtil.GetItemDisplayName(stored.Item.ClassName);
		AddLog(clan, actor.Name + "#STR_SMP_00073" + itemName);
		SaveDB();
		AddPersonalAchievementValue(uid, actor.Name, "StorageDeposits", 1, player);
		clan.Stats.StorageDeposits++;
		m_StatsDirty = true;
		CheckClanAchievementNotifications(clan);

		Notify(player, "#STR_SMP_00919", "#STR_SMP_00803" + itemName);
		NotifyClan(clan, "#STR_SMP_00919", actor.Name + "#STR_SMP_00073" + itemName, uid);
		BroadcastStorage(clan);
		SyncClanState(clan);
	}

	protected void HandleStorageTake(PlayerBase player, int itemId)
	{
		if (!m_Config.Storage.Enabled)
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = FindClanByMember(uid);
		if (!clan)
			return;

		if (clan.FindMember(uid).Rank < clan.GetMinRank(SM_ClanAction.STORAGE_TAKE))
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00686");
			return;
		}

		if (!IsPlayerAtClanBase(player, clan))
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00283");
			return;
		}

		SM_StorageItem stored = FindStorageItem(itemId);
		if (!stored || stored.ClanName != clan.Name)
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00829");
			SendStorage(player);
			return;
		}

		EntityAI restored = RestoreItemTree(stored.Item, player, true);
		if (!restored)
		{
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00770" + stored.Id + ")");
			return;
		}

		SM_ClanMember actor = clan.FindMember(uid);
		string itemName = SM_PartyUtil.GetItemDisplayName(stored.Item.ClassName);

		m_StorageDB.Items.RemoveItem(stored);
		SaveStorageDBForClan(clan.Name);

		AddLog(clan, actor.Name + "#STR_SMP_00029" + itemName);
		SaveDB();

		Notify(player, "#STR_SMP_00919", "#STR_SMP_00320" + itemName);
		NotifyClan(clan, "#STR_SMP_00919", actor.Name + "#STR_SMP_00029" + itemName, uid);
		BroadcastStorage(clan);
		SyncClanState(clan);
	}

	void SendStorage(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetPlainId();
		SM_Clan clan = GetActorClan(uid);

		ScriptRPC rpc = new ScriptRPC();
		if (!clan)
		{
			rpc.Write(0);
			rpc.Send(player, SM_PartyRPC.SYNC_STORAGE, true, player.GetIdentity());
			return;
		}

		array<ref SM_StorageItem> clanItems = new array<ref SM_StorageItem>;
		foreach (SM_StorageItem item : m_StorageDB.Items)
		{
			if (item.ClanName == clan.Name)
				clanItems.Insert(item);
		}

		rpc.Write(clanItems.Count());
		foreach (SM_StorageItem si : clanItems)
		{
			rpc.Write(si.Id);
			rpc.Write(si.Item.ClassName);
			rpc.Write(si.Item.Quantity);
			rpc.Write(si.Item.CountNested());
			rpc.Write(si.Item.Health);
			rpc.Write(si.DepositorName);
			rpc.Write(si.Date);
			rpc.Write(si.Item.Attachments.Count());
			foreach (SM_MarketItem attData : si.Item.Attachments)
				rpc.Write(attData.ClassName);
		}
		rpc.Send(player, SM_PartyRPC.SYNC_STORAGE, true, player.GetIdentity());
	}

	protected void BroadcastStorage(SM_Clan clan)
	{
		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);
		foreach (SM_ClanMember member : clan.Members)
		{
			PlayerBase pb;
			if (onlineMap.Find(member.Uid, pb))
				SendStorage(pb);
		}
	}

	protected void ReturnClanStorage(string clanName)
	{
		map<string, PlayerBase> onlineMap;
		BuildOnlineMap(onlineMap);

		bool changed = false;
		for (int i = m_StorageDB.Items.Count() - 1; i >= 0; i--)
		{
			SM_StorageItem item = m_StorageDB.Items[i];
			if (item.ClanName != clanName)
				continue;

			m_StorageDB.Items.Remove(i);
			changed = true;

			PlayerBase depositor;
			if (onlineMap.Find(item.DepositorUid, depositor))
			{
				RestoreItemTree(item.Item, depositor, false);
				Notify(depositor, "#STR_SMP_00919", "#STR_SMP_00538");
			}
			else
			{
				m_StorageDB.PendingReturns.Insert(item);
			}
		}

		if (changed)
			SaveStorageDB();
	}

	protected void GiveStoragePendingReturns(PlayerBase player, string uid)
	{
		bool changed = false;
		for (int i = m_StorageDB.PendingReturns.Count() - 1; i >= 0; i--)
		{
			SM_StorageItem item = m_StorageDB.PendingReturns[i];
			if (item.DepositorUid != uid)
				continue;

			m_StorageDB.PendingReturns.Remove(i);
			changed = true;

			RestoreItemTree(item.Item, player, false);
			Notify(player, "#STR_SMP_00919", "#STR_SMP_00511");
		}

		if (changed)
			SaveStorageDB();
	}

	protected void ReconcileOrphanedItems()
	{
		int orphanLots = 0;
		for (int i = m_MarketDB.Lots.Count() - 1; i >= 0; i--)
		{
			SM_MarketLot lot = m_MarketDB.Lots[i];
			if (FindClan(lot.ClanName))
				continue;
			m_MarketDB.Lots.Remove(i);
			m_MarketDB.PendingReturns.Insert(lot);
			orphanLots++;
		}
		if (orphanLots > 0)
			SaveMarketDB();

		int orphanItems = 0;
		for (int s = m_StorageDB.Items.Count() - 1; s >= 0; s--)
		{
			SM_StorageItem item = m_StorageDB.Items[s];
			if (FindClan(item.ClanName))
				continue;
			m_StorageDB.Items.Remove(s);
			m_StorageDB.PendingReturns.Insert(item);
			orphanItems++;
		}
		if (orphanItems > 0)
			SaveStorageDB();

		if (orphanLots > 0 || orphanItems > 0)
			Print(SM_PartyLoc.Text("#STR_SMP_00173" + orphanLots + "#STR_SMP_00159" + orphanItems));
	}
}

class SM_RestCallback extends RestCallback
{
	override void OnError(int errorCode) {}
	override void OnTimeout() {}
	override void OnSuccess(string data, int dataSize) {}
}
