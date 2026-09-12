class SM_ClanClientData
{
	// мой клан
	static bool HasClan = false;
	static string MyUid = "";
	static string ClanName = "";
	static string ClanTag = "";
	static int ClanColorIndex = 0;
	static string ClanDescription = "";
	static int MyRank = 0;
	static bool IsAdmin = false;
	static bool IsAdminMode = false;
	static bool ClanWebhookEnabled = false;
	static bool ClanWebhookConfigured = false;
	static bool ClanWebhookAuditEnabled = false;
	static ref array<ref SM_ClanMemberView> Members = new array<ref SM_ClanMemberView>;
	static ref array<string> LogEntries = new array<string>;

	static ref array<ref SM_ApplicationView> Applications = new array<ref SM_ApplicationView>;

	static string InfoName = "";
	static string InfoTag = "";
	static int InfoColorIndex = 0;
	static string InfoDescription = "";
	static ref array<ref SM_ClanMemberView> InfoMembers = new array<ref SM_ClanMemberView>;

	static ref array<ref SM_InviteView> Invites = new array<ref SM_InviteView>;

	static ref array<ref SM_ClanListEntry> ClanList = new array<ref SM_ClanListEntry>;

	static ref array<ref SM_PlayerEntry> InvitablePlayers = new array<ref SM_PlayerEntry>;

	static ref array<string> RankNames = new array<string>;
	static int MinRankToInvite = 0;
	static int MinRankToKick = 0;
	static bool HudEnabled = true;
	static bool ServerHudEnabled = true;
	static int HudOffsetX = 12;
	static int HudOffsetY = 120;
	static int HudMaxVisibleMembers = 8;
	static bool HudShowSelf = false;
	static bool ShowDistanceInHud = true;
	static bool ShowDirectionInHud = true;
	static bool MarkersEnabled = true;
	static bool Local3DMarkersEnabled = true;
	static bool HudCompact = false;
	// 3D-показ отдельных меток карты (на выбранную, а не на все сразу).
	// Личные хранят флаг в самой метке (Show3D, персист в JSON);
	// клановые — клиентский набор ключей в ClientSettings, серверные — имена скрытых из 3D (без персиста).
	static ref array<string> Clan3DMarkerKeys = new array<string>;
	static ref array<string> Server3DMarkerNames = new array<string>;
	static int MarkerMaxDistance = 500;
	static int PingMarkerMaxDistance = 500;
	static int MinClanNameLength = 3;
	static int MaxClanNameLength = 24;
	static int MaxClanTagLength = 6;
	static int MaxClanDescriptionLength = 200;
	static string CreateClanLogoPaa = "SM_PartyMod\\GUI\\logo\\logo.paa";
	static bool ChatEnabled = true;
	static int ChatColorDirect = ARGB(255, 255, 255, 255);
	static int ChatColorDirectPlayer = ARGB(255, 255, 255, 255);
	static int ChatColorGlobal = ARGB(255, 255, 255, 255);
	static int ChatColorGlobalPlayer = ARGB(255, 100, 255, 100);
	static int ChatColorServer = ARGB(255, 255, 50, 50);
	static int ChatColorAlert = ARGB(255, 100, 200, 255);
	static bool LocalChatEnabled = true;
	static bool GlobalChatEnabled = true;
	static int ChatVisibleMessages = 6;
	static bool ChatHistoryEnabled = false;
	static float ChatScale = 1.0;
	static string ServerSessionKey = "";
	static int ChatMuteRemainingAtSync = 0;
	static float ChatMuteSyncedTick = 0.0;
	static string ChatMuteReason = "";
	static const int ChatScaleMinPercent = 80;
	static const int ChatScaleMaxPercent = 140;
	static const int ChatScaleStepPercent = 10;
	static ref array<ref SM_ChatHistoryEntry> ChatHistory = new array<ref SM_ChatHistoryEntry>;
	static int ChatScrollOffset = 0;       // всегда показываем самые свежие сообщения
	static bool ChatViewDirty = false;     // HUD должен перерисоваться
	static float ChatActivityTime = 0;     // GetTickTime последнего события чата (для затухания)
	// Клиентские настройки игрока (перекрывают серверные, персист в $profile)
	static bool ChatTimestamps = false;
	static int ChatFadeSeconds = 8;
	static int ChatOpacityPercent = 100;
	static int ChatOffsetX = 0;
	static int ChatOffsetY = 0;
	static int HudOpacityPercent = 100;
	static ref SM_ClientSettingsData ClientSettings;
	static bool ClientSettingsLoaded = false;
	static const string CLIENT_SETTINGS_FILE = "$profile:SM_PartyMod\\ClientSettings.json";
	static bool PingsEnabled = true;
	static bool TopsEnabled = true;
	static bool TreasuryEnabled = true;

	static int Treasury = 0;
	static ref array<ref SM_CurrencyItem> CurrencyItems = new array<ref SM_CurrencyItem>;

	static bool MarketEnabled = true;
	static int MarketFeePercent = 5;
	static int MarketListingFeePercent = 5;
	static int MaxLotsPerClan = 10;
	static int MinRankToSell = 2;
	static bool ServerMarketEnabled = true;
	static int ServerMarketRotationId = 0;
	static int ServerMarketNextRefreshSeconds = 0;
	static int ServerMarketVisibleItems = 3;
	static int ServerMarketTotalItems = 0;
	static bool AuctionEnabled = true;
	static int AuctionListingFee = 0;
	static int AuctionListingFeePercent = 5;
	static int AuctionSaleTaxPercent = 5;
	static int AuctionMinDurationMinutes = 30;
	static int AuctionMaxDurationMinutes = 1440;
	static int AuctionDefaultDurationMinutes = 360;
	static int AuctionMinBidStep = 100;
	static int AuctionMaxStartPrice = 1000000;
	static bool AchievementsEnabled = true;
	static bool AchievementsClaimRewardManually = true;
	static bool ContractsEnabled = true;
	static bool ContractsCanCreate = false;
	static int ContractsMinPrice = 1000;
	static int ContractsMaxPrice = 1000000;
	static int ContractsFeePercent = 10;
	static int ContractsPenaltyPercent = 20;
	static int ContractsMinDurationMinutes = 30;
	static int ContractsMaxDurationMinutes = 1440;
	static bool PlayerExperienceEnabled = true;
	static bool PlayerExperienceClanListEnabled = true;
	static string MyPlayerTitle = "#STR_SMP_00709";
	static int MyPlayerXP = 0;
	static int MyPlayerRankIndex = 0;
	static int MyPlayerNextXP = 0;
	static int MyPlayerTitleColor = 0;
	static ref array<ref SM_PlayerTitleView> PlayerTitles = new array<ref SM_PlayerTitleView>;
	static bool MapEnabled = true;
	static int MaxMapMarkersPerClan = 30;
	static int MapMarkerMaxNameLength = 32;
	static ref array<ref SM_MarketLotView> MarketLots = new array<ref SM_MarketLotView>;
	static ref array<ref SM_ServerMarketItemView> ServerMarketItems = new array<ref SM_ServerMarketItemView>;
	static ref array<ref SM_AuctionLotView> AuctionLots = new array<ref SM_AuctionLotView>;
	static ref array<ref SM_AchievementView> AchievementViews = new array<ref SM_AchievementView>;
	static ref array<ref SM_ContractView> ContractViews = new array<ref SM_ContractView>;
	static ref array<ref SM_ContractView> ContractHistoryViews = new array<ref SM_ContractView>;
	static ref array<ref SM_ContractTargetView> ContractTargets = new array<ref SM_ContractTargetView>;
	static ref array<ref SM_ContractItemOptionView> ContractItemOptions = new array<ref SM_ContractItemOptionView>;
	static ref array<ref SM_ContractMarkerView> ContractMarkers = new array<ref SM_ContractMarkerView>;
	static string ContractTargetPreviewUid = "";
	static string ContractTargetPreviewName = "";
	static string ContractTargetPreviewClass = "";
	static string ContractTargetPreviewHandClass = "";
	static ref array<string> ContractTargetPreviewAttachments = new array<string>;
	static ref array<ref SM_AdminChatLogEntry> AdminChatLog = new array<ref SM_AdminChatLogEntry>;
	static ref array<ref SM_OnlinePlayerView> OnlinePlayers = new array<ref SM_OnlinePlayerView>;

	static bool ClanLevelsEnabled = true;
	static int ClanLevel = 1;
	static int LotCap = 10;
	static ref array<ref SM_ClanLevel> ClanLevels = new array<ref SM_ClanLevel>;

	static bool StorageEnabled = true;
	static int StorageCap = 0;
	static int StorageUsed = 0;
	static ref array<ref SM_StorageItemView> StorageItems = new array<ref SM_StorageItemView>;

	static bool HasBase = false;
	static vector BasePos = "0 0 0";
	static int StorageRadius = 30;
	static bool BaseRadiusEnabled = true;
	static int BaseRadius = 150;

	static ref array<int> Perms = new array<int>;

	static int GetActionRank(int action)
	{
		if (action >= 0 && action < Perms.Count())
			return Perms[action];
		return 0;
	}

	static int MinShotsForAccuracyTop = 100;
	static int TopSeasonSecondsLeft = 0;
	static ref array<int> TopCategoryEnabled = new array<int>;
	static ref array<ref SM_ClanTopEntry> TopClans = new array<ref SM_ClanTopEntry>;

	static ref array<ref SM_ClanMemberView> HudMembers = new array<ref SM_ClanMemberView>;
	static ref array<ref SM_ClanMemberView> VisibleHudMembers = new array<ref SM_ClanMemberView>;
	static bool HudDirty = false;

	static ref array<ref SM_ClanPing> Pings = new array<ref SM_ClanPing>;
	static bool PingsDirty = false;

	static ref array<ref SM_ClanMapMarker> MapMarkers = new array<ref SM_ClanMapMarker>;
	static bool MapMarkers3DDirty = false;
	static ref array<ref SM_PersonalMapMarker> PersonalMapMarkers = new array<ref SM_PersonalMapMarker>;
	static ref array<ref SM_ServerMapMarker> ServerMapMarkers = new array<ref SM_ServerMapMarker>;
	static ref array<ref SM_AdminBaseMapMarker> AdminBaseMapMarkers = new array<ref SM_AdminBaseMapMarker>;
	static int NextPersonalMapMarkerId = 1;
	static bool PersonalMapMarkersLoaded = false;
	static string PersonalMapMarkersLoadedPath = "";
	static const string PERSONAL_MARKER_DIR = "$profile:SM_PartyMod";

	static ref ScriptInvoker OnStateChanged = new ScriptInvoker();
	static ref ScriptInvoker OnClanListChanged = new ScriptInvoker();
	static ref ScriptInvoker OnPlayersChanged = new ScriptInvoker();
	static ref ScriptInvoker OnClanInfoChanged = new ScriptInvoker();
	static ref ScriptInvoker OnTopsChanged = new ScriptInvoker();
	static ref ScriptInvoker OnMarketChanged = new ScriptInvoker();
	static ref ScriptInvoker OnServerMarketChanged = new ScriptInvoker();
	static ref ScriptInvoker OnAuctionChanged = new ScriptInvoker();
	static ref ScriptInvoker OnAchievementsChanged = new ScriptInvoker();
	static ref ScriptInvoker OnPlayerTitlesChanged = new ScriptInvoker();
	static ref ScriptInvoker OnContractsChanged = new ScriptInvoker();
	static ref ScriptInvoker OnContractTargetPreviewChanged = new ScriptInvoker();
	static ref ScriptInvoker OnAdminChatLogChanged = new ScriptInvoker();
	static ref ScriptInvoker OnOnlinePlayersChanged = new ScriptInvoker();
	static ref ScriptInvoker OnStorageChanged = new ScriptInvoker();
	static ref ScriptInvoker OnChatChanged = new ScriptInvoker();

	static string GetPersonalMapMarkersPath()
	{
		string worldName = "world";
		if (GetGame())
			worldName = GetGame().GetWorldName();
		if (worldName == "")
			worldName = "world";

		worldName.Replace("\\", "_");
		worldName.Replace("/", "_");
		worldName.Replace(":", "_");
		worldName.Replace(" ", "_");

		string owner = MyUid;
		if (owner == "")
			owner = "local";
		owner.Replace("\\", "_");
		owner.Replace("/", "_");
		owner.Replace(":", "_");
		owner.Replace(" ", "_");

		return PERSONAL_MARKER_DIR + "\\PersonalMapMarkers_" + owner + "_" + worldName + ".json";
	}

	static void EnsurePersonalMapMarkersLoaded()
	{
		string path = GetPersonalMapMarkersPath();
		if (PersonalMapMarkersLoaded && PersonalMapMarkersLoadedPath == path)
			return;

		PersonalMapMarkersLoaded = true;
		PersonalMapMarkersLoadedPath = path;
		PersonalMapMarkers.Clear();
		NextPersonalMapMarkerId = 1;

		if (!FileExist(path))
			return;

		SM_PersonalMapMarkersFile fileData = new SM_PersonalMapMarkersFile;
		string errorMessage;
		if (!JsonFileLoader<SM_PersonalMapMarkersFile>.LoadFile(path, fileData, errorMessage))
		{
			Print(SM_PartyLoc.Text("#STR_SMP_00178" + errorMessage));
			return;
		}

		if (fileData.NextId > 0)
			NextPersonalMapMarkerId = fileData.NextId;

		if (fileData.Markers)
		{
			foreach (SM_PersonalMapMarker marker : fileData.Markers)
			{
				if (!marker)
					continue;

				marker.Name = marker.Name.Trim();
				if (marker.Name == "")
					continue;

				if (marker.Id < 1)
				{
					marker.Id = NextPersonalMapMarkerId;
					NextPersonalMapMarkerId++;
				}

				if (marker.Id >= NextPersonalMapMarkerId)
					NextPersonalMapMarkerId = marker.Id + 1;

				PersonalMapMarkers.Insert(marker);
			}
		}

	}

	static void SavePersonalMapMarkers()
	{
		EnsurePersonalMapMarkersLoaded();

		if (!FileExist(PERSONAL_MARKER_DIR))
			MakeDirectory(PERSONAL_MARKER_DIR);

		SM_PersonalMapMarkersFile fileData = new SM_PersonalMapMarkersFile;
		fileData.NextId = NextPersonalMapMarkerId;
		fileData.Markers.Clear();

		foreach (SM_PersonalMapMarker marker : PersonalMapMarkers)
		{
			if (marker)
				fileData.Markers.Insert(marker);
		}

		string errorMessage;
		if (!JsonFileLoader<SM_PersonalMapMarkersFile>.SaveFile(GetPersonalMapMarkersPath(), fileData, errorMessage))
			Print(SM_PartyLoc.Text("#STR_SMP_00180" + errorMessage));
	}

	static void EnsureClientSettingsLoaded()
	{
		if (ClientSettingsLoaded)
			return;
		ClientSettingsLoaded = true;

		ClientSettings = new SM_ClientSettingsData();
		if (FileExist(CLIENT_SETTINGS_FILE))
		{
			string errorMessage;
			if (!JsonFileLoader<SM_ClientSettingsData>.LoadFile(CLIENT_SETTINGS_FILE, ClientSettings, errorMessage))
			{
				Print(SM_PartyLoc.Text("#STR_SMP_00179" + errorMessage));
				ClientSettings = new SM_ClientSettingsData();
			}
		}
		NormalizeClientSettings();
		ApplyClientSettings();
	}

	static void NormalizeClientSettings()
	{
		if (!ClientSettings)
			return;

		if (!ClientSettings.PingMarkerDistanceReady)
		{
			ClientSettings.PingMarkerDistance = ClientSettings.MarkerDistance;
			ClientSettings.PingMarkerDistanceReady = true;
		}

		if (ClientSettings.MarkerDistance < 0)
			ClientSettings.MarkerDistance = 0;
		if (ClientSettings.PingMarkerDistance < 0)
			ClientSettings.PingMarkerDistance = 0;
		if (!ClientSettings.Clan3DMarkerKeys)
			ClientSettings.Clan3DMarkerKeys = new array<string>;

		for (int c = ClientSettings.Clan3DMarkerKeys.Count() - 1; c >= 0; c--)
		{
			string clanMarkerKey = ClientSettings.Clan3DMarkerKeys[c];
			clanMarkerKey = clanMarkerKey.Trim();
			ClientSettings.Clan3DMarkerKeys.Set(c, clanMarkerKey);
			if (clanMarkerKey == "")
			{
				ClientSettings.Clan3DMarkerKeys.Remove(c);
				continue;
			}
			if (ClientSettings.Clan3DMarkerKeys.Find(clanMarkerKey) != c)
				ClientSettings.Clan3DMarkerKeys.Remove(c);
		}
	}

	static void SaveClientSettings()
	{
		if (!ClientSettings)
			return;
		if (!FileExist(PERSONAL_MARKER_DIR))
			MakeDirectory(PERSONAL_MARKER_DIR);
		string errorMessage;
		if (!JsonFileLoader<SM_ClientSettingsData>.SaveFile(CLIENT_SETTINGS_FILE, ClientSettings, errorMessage))
			Print(SM_PartyLoc.Text("#STR_SMP_00181" + errorMessage));
	}

	// Копируем клиентские значения поверх синхронизированных серверных.
	static void ApplyClientSettings()
	{
		if (!ClientSettings)
			return;
		ChatVisibleMessages = ClientSettings.ChatVisibleMessages;
		ChatHistoryEnabled = ClientSettings.ChatHistoryEnabled;
		if (!ChatHistoryEnabled)
			ChatScrollOffset = 0;
		ChatTimestamps = ClientSettings.ChatTimestamps;
		ChatFadeSeconds = ClientSettings.ChatFadeSeconds;
		ChatOpacityPercent = ClientSettings.ChatOpacity;
		ChatOffsetX = ClientSettings.ChatOffsetX;
		ChatOffsetY = ClientSettings.ChatOffsetY;
		HudEnabled = ServerHudEnabled && ClientSettings.HudEnabled;
		HudMaxVisibleMembers = ClientSettings.HudMaxVisibleMembers;
		HudOpacityPercent = ClientSettings.HudOpacity;
		HudOffsetX = ClientSettings.HudOffsetX;
		HudOffsetY = ClientSettings.HudOffsetY;
		HudCompact = ClientSettings.HudCompact;
		HudShowSelf = ClientSettings.HudShowSelf;
		ShowDistanceInHud = ClientSettings.HudShowDistance;
		ShowDirectionInHud = ClientSettings.HudShowDirection;
		MarkersEnabled = ClientSettings.MarkersEnabled;
		Local3DMarkersEnabled = ClientSettings.Markers3D;
		MarkerMaxDistance = ClientSettings.MarkerDistance;
		PingMarkerMaxDistance = ClientSettings.PingMarkerDistance;
		if (!ClientSettings.Clan3DMarkerKeys)
			ClientSettings.Clan3DMarkerKeys = new array<string>;
		Clan3DMarkerKeys = ClientSettings.Clan3DMarkerKeys;
		RecomputeVisibleHudMembers();
		HudDirty = true;
		ChatViewDirty = true;
		PingsDirty = true;
		MapMarkers3DDirty = true;
	}

	static bool ToggleLocal3DMarkers()
	{
		EnsureClientSettingsLoaded();
		if (!ClientSettings)
			return Local3DMarkersEnabled;

		if (ClientSettings.Markers3D)
			ClientSettings.Markers3D = false;
		else
			ClientSettings.Markers3D = true;

		SaveClientSettings();
		ApplyClientSettings();
		return Local3DMarkersEnabled;
	}

	static void ScrollChatHistory(int direction)
	{
		if (!ChatHistoryEnabled)
		{
			ChatScrollOffset = 0;
			ChatViewDirty = true;
			return;
		}

		int visible = ChatVisibleMessages;
		if (visible < 1)
			visible = 1;
		if (visible > 18)
			visible = 18;

		int maxOffset = ChatHistory.Count() - visible;
		if (maxOffset < 0)
			maxOffset = 0;

		int step = visible;
		if (step < 1)
			step = 1;

		ChatScrollOffset = ChatScrollOffset + direction * step;
		if (ChatScrollOffset < 0)
			ChatScrollOffset = 0;
		if (ChatScrollOffset > maxOffset)
			ChatScrollOffset = maxOffset;

		ChatActivityTime = GetGame().GetTickTime();
		ChatViewDirty = true;
	}

	static bool IsHudMemberHidden(string uid)
	{
		EnsureClientSettingsLoaded();
		if (!ClientSettings || !ClientSettings.HudHiddenUids)
			return false;
		return ClientSettings.HudHiddenUids.Find(uid) >= 0;
	}

	static void SetHudMemberHidden(string uid, bool hidden)
	{
		EnsureClientSettingsLoaded();
		if (!ClientSettings)
			return;
		if (!ClientSettings.HudHiddenUids)
			ClientSettings.HudHiddenUids = new array<string>;
		int idx = ClientSettings.HudHiddenUids.Find(uid);
		if (hidden)
		{
			if (idx < 0)
				ClientSettings.HudHiddenUids.Insert(uid);
		}
		else
		{
			if (idx >= 0)
				ClientSettings.HudHiddenUids.Remove(idx);
		}
		SaveClientSettings();
		RecomputeVisibleHudMembers();
		HudDirty = true;
	}

	static void RecomputeVisibleHudMembers()
	{
		VisibleHudMembers.Clear();
		foreach (SM_ClanMemberView m : HudMembers)
		{
			if (!m)
				continue;
			if (!HudShowSelf && m.Uid == MyUid)
				continue;
			if (IsHudMemberHidden(m.Uid))
				continue;
			VisibleHudMembers.Insert(m);
		}
	}

	static bool AddPersonalMapMarker(string name, vector position, int color, int icon, out string errorText)
	{
		return AddPersonalMapMarkerEx(name, position, color, icon, false, errorText);
	}

	static bool AddPersonalMapMarkerEx(string name, vector position, int color, int icon, bool show3D, out string errorText)
	{
		EnsurePersonalMapMarkersLoaded();

		errorText = "";
		name = name.Trim();

		if (name == "")
		{
			errorText = "#STR_SMP_00315";
			return false;
		}

		if (name.LengthUtf8() > MapMarkerMaxNameLength)
		{
			errorText = "#STR_SMP_00666" + MapMarkerMaxNameLength.ToString() + "#STR_SMP_00082";
			return false;
		}

		color = SM_ClanColors.NormalizeMarkerColor(color);
		icon = SM_MapMarkerIconSet.Normalize(icon);

		PersonalMapMarkers.Insert(new SM_PersonalMapMarker(NextPersonalMapMarkerId, name, position, color, show3D, icon));
		NextPersonalMapMarkerId++;
		SavePersonalMapMarkers();
		MapMarkers3DDirty = true;
		return true;
	}

	static bool RemovePersonalMapMarker(int markerId)
	{
		EnsurePersonalMapMarkersLoaded();

		for (int i = 0; i < PersonalMapMarkers.Count(); i++)
		{
			SM_PersonalMapMarker marker = PersonalMapMarkers[i];
			if (marker && marker.Id == markerId)
			{
				PersonalMapMarkers.Remove(i);
				SavePersonalMapMarkers();
				MapMarkers3DDirty = true;
				return true;
			}
		}

		return false;
	}

	static void RemovePersonalDeathMarkers(int icon)
	{
		EnsurePersonalMapMarkersLoaded();

		bool changed = false;
		for (int i = PersonalMapMarkers.Count() - 1; i >= 0; i--)
		{
			SM_PersonalMapMarker marker = PersonalMapMarkers[i];
			if (!marker)
				continue;
			if (marker.Icon != icon)
				continue;
			if (marker.Name != "#STR_SMP_01100")
				continue;

			PersonalMapMarkers.Remove(i);
			changed = true;
		}

		if (changed)
		{
			SavePersonalMapMarkers();
			MapMarkers3DDirty = true;
		}
	}

	static bool UpdatePersonalMapMarker(int markerId, string name, vector position, int color, int icon, out string errorText)
	{
		EnsurePersonalMapMarkersLoaded();

		errorText = "";
		name = name.Trim();

		if (name == "")
		{
			errorText = "#STR_SMP_00315";
			return false;
		}

		if (name.LengthUtf8() > MapMarkerMaxNameLength)
		{
			errorText = "#STR_SMP_00666" + MapMarkerMaxNameLength.ToString() + "#STR_SMP_00082";
			return false;
		}

		color = SM_ClanColors.NormalizeMarkerColor(color);
		icon = SM_MapMarkerIconSet.Normalize(icon);

		for (int i = 0; i < PersonalMapMarkers.Count(); i++)
		{
			SM_PersonalMapMarker marker = PersonalMapMarkers[i];
			if (marker && marker.Id == markerId)
			{
				marker.Name = name;
				marker.Position = position;
				marker.Color = color;
				marker.Icon = icon;
				SavePersonalMapMarkers();
				MapMarkers3DDirty = true;
				return true;
			}
		}

		errorText = "#STR_SMP_00630";
		return false;
	}

	// --- 3D-показ отдельных меток ---
	static bool IsPersonalMarker3D(int id)
	{
		if (!MapEnabled)
			return false;

		EnsurePersonalMapMarkersLoaded();
		foreach (SM_PersonalMapMarker marker : PersonalMapMarkers)
		{
			if (marker && marker.Id == id)
				return marker.Show3D;
		}
		return false;
	}

	static void TogglePersonalMarker3D(int id)
	{
		if (!MapEnabled)
			return;

		EnsurePersonalMapMarkersLoaded();
		foreach (SM_PersonalMapMarker marker : PersonalMapMarkers)
		{
			if (marker && marker.Id == id)
			{
				marker.Show3D = !marker.Show3D;
				SavePersonalMapMarkers();
				MapMarkers3DDirty = true;
				return;
			}
		}
	}

	static bool IsClanMarker3D(int id)
	{
		if (!MapEnabled)
			return false;

		EnsureClientSettingsLoaded();
		return Clan3DMarkerKeys.Find(GetClan3DMarkerKey(id)) >= 0;
	}

	static void ToggleClanMarker3D(int id)
	{
		if (!MapEnabled)
			return;

		EnsureClientSettingsLoaded();
		string key = GetClan3DMarkerKey(id);
		int idx = Clan3DMarkerKeys.Find(key);
		if (idx >= 0)
			Clan3DMarkerKeys.Remove(idx);
		else
			Clan3DMarkerKeys.Insert(key);
		SaveClientSettings();
		MapMarkers3DDirty = true;
	}

	static void EnsureClanMarker3D(int id)
	{
		if (!MapEnabled)
			return;

		EnsureClientSettingsLoaded();
		string key = GetClan3DMarkerKey(id);
		if (Clan3DMarkerKeys.Find(key) >= 0)
			return;

		Clan3DMarkerKeys.Insert(key);
		SaveClientSettings();
		MapMarkers3DDirty = true;
	}

	static string GetClan3DMarkerKey(int id)
	{
		string clanKey = ClanName;
		if (clanKey == "")
			clanKey = "no_clan";
		return clanKey + "|" + id.ToString();
	}

	static bool HasClanMapMarkerKey(string key)
	{
		foreach (SM_ClanMapMarker marker : MapMarkers)
		{
			if (marker && GetClan3DMarkerKey(marker.Id) == key)
				return true;
		}
		return false;
	}

	static bool IsCurrentClan3DMarkerKey(string key)
	{
		string clanKey = ClanName;
		if (clanKey == "")
			clanKey = "no_clan";
		string prefix = clanKey + "|";
		if (key.IndexOf(prefix) == 0)
			return true;
		return false;
	}

	static void PruneClan3DMarkerKeys()
	{
		if (!MapEnabled)
			return;

		EnsureClientSettingsLoaded();
		if (!Clan3DMarkerKeys)
			return;

		bool changed = false;
		for (int i = Clan3DMarkerKeys.Count() - 1; i >= 0; i--)
		{
			string key = Clan3DMarkerKeys[i];
			key = key.Trim();
			Clan3DMarkerKeys.Set(i, key);
			if (key == "")
			{
				Clan3DMarkerKeys.Remove(i);
				changed = true;
				continue;
			}
			if (HasClan && IsCurrentClan3DMarkerKey(key) && !HasClanMapMarkerKey(key))
			{
				Clan3DMarkerKeys.Remove(i);
				changed = true;
			}
		}

		if (changed)
			SaveClientSettings();
	}

	static bool IsServerMarker3DByIndex(int index)
	{
		if (!MapEnabled)
			return false;

		if (index < 0 || index >= ServerMapMarkers.Count())
			return false;
		SM_ServerMapMarker marker = ServerMapMarkers[index];
		if (!marker)
			return false;
		return Server3DMarkerNames.Find(marker.Name) < 0;
	}

	static void ToggleServerMarker3DByIndex(int index)
	{
		if (!MapEnabled)
			return;

		if (index < 0 || index >= ServerMapMarkers.Count())
			return;
		SM_ServerMapMarker marker = ServerMapMarkers[index];
		if (!marker)
			return;
		int idx = Server3DMarkerNames.Find(marker.Name);
		if (idx >= 0)
			Server3DMarkerNames.Remove(idx);
		else
			Server3DMarkerNames.Insert(marker.Name);
		MapMarkers3DDirty = true;
	}

	static SM_ClanLevel GetLevelData(int level)
	{
		if (ClanLevels.Count() < 1)
			return null;
		if (level < 1)
			level = 1;
		if (level > ClanLevels.Count())
			level = ClanLevels.Count();
		return ClanLevels[level - 1];
	}

	static int GetMaxLevel()
	{
		return ClanLevels.Count();
	}

	static int GetNextUpgradeCost()
	{
		if (ClanLevel >= GetMaxLevel())
			return -1;
		SM_ClanLevel next = GetLevelData(ClanLevel + 1);
		if (!next)
			return -1;
		return next.UpgradeCost;
	}

	static int GetLeaderRank()
	{
		return RankNames.Count() - 1;
	}

	static string GetRankName(int rank)
	{
		if (rank < 0 || rank >= RankNames.Count())
			return "?";
		return RankNames[rank];
	}

	static int GetClanColor()
	{
		return SM_ClanColors.ResolveColor(ClanColorIndex);
	}

	static int NormalizeChatScalePercent(float scale)
	{
		int pct = Math.Round(scale * 100);
		int halfStep = ChatScaleStepPercent / 2;
		pct = ((pct + halfStep) / ChatScaleStepPercent) * ChatScaleStepPercent;

		if (pct < ChatScaleMinPercent)
			pct = ChatScaleMinPercent;
		if (pct > ChatScaleMaxPercent)
			pct = ChatScaleMaxPercent;

		return pct;
	}

	static int GetChatScalePercent()
	{
		return NormalizeChatScalePercent(ChatScale);
	}

	static float NormalizeChatScale(float scale)
	{
		return NormalizeChatScalePercent(scale) * 0.01;
	}

	static void SetChatScalePercent(int pct)
	{
		if (pct < ChatScaleMinPercent)
			pct = ChatScaleMinPercent;
		if (pct > ChatScaleMaxPercent)
			pct = ChatScaleMaxPercent;

		ChatScale = pct * 0.01;
	}

	static void WakeChatView()
	{
		ChatActivityTime = GetGame().GetTickTime();
		ChatViewDirty = true;
	}

	static void ChangeChatScale(int deltaPercent)
	{
		int pct = GetChatScalePercent();
		pct = pct + deltaPercent;
		SetChatScalePercent(pct);
		WakeChatView();
	}

	static void OnRPC(int rpc_type, ParamsReadContext ctx)
	{
		switch (rpc_type)
		{
			case SM_PartyRPC.SYNC_STATE:
				ReadState(ctx);
				break;
			case SM_PartyRPC.SYNC_CLAN_LIST:
				ReadClanList(ctx);
				break;
			case SM_PartyRPC.SYNC_PLAYERS:
				ReadPlayers(ctx);
				break;
			case SM_PartyRPC.HUD_UPDATE:
				ReadHud(ctx);
				break;
			case SM_PartyRPC.MARK_SYNC:
				ReadMark(ctx);
				break;
			case SM_PartyRPC.MARK_CLEAR_SYNC:
				ReadMarkClear(ctx);
				break;
			case SM_PartyRPC.CLAN_INFO:
				ReadClanInfo(ctx);
				break;
			case SM_PartyRPC.SYNC_TOPS:
				ReadTops(ctx);
				break;
			case SM_PartyRPC.SYNC_MARKET:
				ReadMarket(ctx);
				break;
			case SM_PartyRPC.SYNC_SERVER_MARKET:
				ReadServerMarket(ctx);
				break;
			case SM_PartyRPC.SYNC_AUCTION:
				ReadAuction(ctx);
				break;
			case SM_PartyRPC.SYNC_ACHIEVEMENTS:
				ReadAchievements(ctx);
				break;
			case SM_PartyRPC.SYNC_PLAYER_TITLES:
				ReadPlayerTitles(ctx);
				break;
			case SM_PartyRPC.SYNC_CONTRACTS:
				ReadContracts(ctx);
				break;
			case SM_PartyRPC.SYNC_CONTRACT_TARGET_PREVIEW:
				ReadContractTargetPreview(ctx);
				break;
			case SM_PartyRPC.SYNC_CHAT_MUTE:
				ReadChatMuteStatus(ctx);
				break;
			case SM_PartyRPC.SYNC_ADMIN_CHAT_LOG:
				ReadAdminChatLog(ctx);
				break;
			case SM_PartyRPC.SYNC_ONLINE_PLAYERS:
				ReadOnlinePlayers(ctx);
				break;
			case SM_PartyRPC.SYNC_STORAGE:
				ReadStorage(ctx);
				break;
			case SM_PartyRPC.CHAT_RECEIVE:
				ReadChat(ctx);
				break;
			case SM_PartyRPC.LOCAL_NOTIFY:
				ReadLocalNotify(ctx);
				break;
			case SM_PartyRPC.PERSONAL_DEATH_MARKER:
				ReadPersonalDeathMarker(ctx);
				break;
		}
	}

	// Единая точка добавления строки в текущий буфер чата.
	// Настройка ChatVisibleMessages ограничивает только вывод на HUD.
	static void ClearChatBuffer()
	{
		ChatHistory.Clear();
		ChatScrollOffset = 0;
		ChatActivityTime = 0;
		ChatViewDirty = true;
		OnChatChanged.Invoke();
	}

	protected static void ApplyServerSessionKey(string serverSessionKey)
	{
		if (serverSessionKey == "")
			return;

		if (ServerSessionKey == "")
		{
			ServerSessionKey = serverSessionKey;
			if (ChatHistory.Count() > 0)
				ClearChatBuffer();
			return;
		}

		if (ServerSessionKey != serverSessionKey)
		{
			ServerSessionKey = serverSessionKey;
			ClearChatBuffer();
		}
	}

	static void AppendChat(SM_ChatHistoryEntry entry)
	{
		if (!entry)
			return;
		if (!ChatEnabled)
			return;

		ChatHistory.Insert(entry);

		ChatScrollOffset = 0;

		ChatActivityTime = GetGame().GetTickTime();
		ChatViewDirty = true;
		OnChatChanged.Invoke();
	}

	static bool IsChatMuted()
	{
		if (GetChatMuteRemainingSeconds() <= 0)
		{
			ChatMuteRemainingAtSync = 0;
			ChatMuteSyncedTick = 0.0;
			ChatMuteReason = "";
			return false;
		}
		return true;
	}

	static int GetChatMuteRemainingSeconds()
	{
		if (ChatMuteRemainingAtSync <= 0)
			return 0;

		float now = 0.0;
		if (GetGame())
			now = GetGame().GetTickTime();

		float elapsedFloat = now - ChatMuteSyncedTick;
		if (elapsedFloat < 0.0)
			elapsedFloat = 0.0;

		int elapsed = Math.Floor(elapsedFloat);
		int left = ChatMuteRemainingAtSync - elapsed;
		if (left < 0)
			left = 0;
		return left;
	}

	static string FormatSecondsShort(int seconds)
	{
		if (seconds <= 0)
			return "#STR_SMP_00223";

		int hours = seconds / 3600;
		int minutes = (seconds % 3600) / 60;
		int secs = seconds % 60;

		string result = "";
		if (hours > 0)
			result = hours.ToString() + "#STR_SMP_00092";
		if (minutes > 0)
		{
			if (result != "")
				result = result + " ";
			result = result + minutes.ToString() + "#STR_SMP_00058";
		}
		if (hours <= 0 && minutes <= 0)
			result = secs.ToString() + "#STR_SMP_00081";

		return result;
	}

	static string GetChatMuteMessage()
	{
		string msg = "#STR_SMP_01066" + FormatSecondsShort(GetChatMuteRemainingSeconds());
		if (ChatMuteReason != "")
			msg = msg + "#STR_SMP_00167" + ChatMuteReason;
		return msg;
	}

	// Системная/перехваченная строка (без отдельного префикса).
	static void AddSystemChat(int channel, string label, string author, string text, int labelColor, int textColor)
	{
		AppendChat(new SM_ChatHistoryEntry(channel, label, "", author, text, SM_PartyUtil.ChatStamp(), labelColor, labelColor, textColor, labelColor));
	}

	protected static void ReadLocalNotify(ParamsReadContext ctx)
	{
		float showTime;
		string title;
		string text;
		string icon;
		if (!ctx.Read(showTime)) return;
		if (!ctx.Read(title)) return;
		if (!ctx.Read(text)) return;
		if (!ctx.Read(icon)) return;

		SM_PartyLoc.AddNotification(showTime, title, text, icon);
	}

	protected static void ReadPersonalDeathMarker(ParamsReadContext ctx)
	{
		string name;
		vector position;
		int color;
		int icon;
		if (!ctx.Read(name)) return;
		if (!ctx.Read(position)) return;
		if (!ctx.Read(color)) return;
		if (!ctx.Read(icon)) return;

		string errorText;
		RemovePersonalDeathMarkers(icon);
		if (!AddPersonalMapMarkerEx(name, position, color, icon, true, errorText))
			AddPersonalMapMarkerEx("#STR_SMP_01100", position, color, icon, true, errorText);

		OnStateChanged.Invoke();
	}

	protected static void ReadChatMuteStatus(ParamsReadContext ctx)
	{
		bool muted;
		int remainingSeconds;
		string reason;
		if (!ctx.Read(muted)) return;
		if (!ctx.Read(remainingSeconds)) return;
		if (!ctx.Read(reason)) return;

		if (muted && remainingSeconds > 0)
		{
			ChatMuteRemainingAtSync = remainingSeconds;
			ChatMuteSyncedTick = 0.0;
			if (GetGame())
				ChatMuteSyncedTick = GetGame().GetTickTime();
			ChatMuteReason = reason;
		}
		else
		{
			ChatMuteRemainingAtSync = 0;
			ChatMuteSyncedTick = 0.0;
			ChatMuteReason = "";
		}
	}

	protected static void ReadAdminChatLog(ParamsReadContext ctx)
	{
		AdminChatLog.Clear();

		int count;
		if (!ctx.Read(count))
			return;

		for (int i = 0; i < count; i++)
		{
			string uid;
			string name;
			string channel;
			string text;
			string stamp;
			if (!ctx.Read(uid)) return;
			if (!ctx.Read(name)) return;
			if (!ctx.Read(channel)) return;
			if (!ctx.Read(text)) return;
			if (!ctx.Read(stamp)) return;

			AdminChatLog.Insert(new SM_AdminChatLogEntry(uid, name, channel, text, stamp));
		}

		OnAdminChatLogChanged.Invoke();
	}

	protected static void ReadChat(ParamsReadContext ctx)
	{
		int channel;
		string label;
		string prefix;
		string author;
		string text;
		string stamp;
		int channelColor;
		int nameColor;
		int textColor;
		int prefixColor;

		if (!ctx.Read(channel)) return;
		if (!ctx.Read(label)) return;
		if (!ctx.Read(prefix)) return;
		if (!ctx.Read(author)) return;
		if (!ctx.Read(text)) return;
		if (!ctx.Read(stamp)) return;
		if (!ctx.Read(channelColor)) return;
		if (!ctx.Read(nameColor)) return;
		if (!ctx.Read(textColor)) return;
		if (!ctx.Read(prefixColor)) return;

		AppendChat(new SM_ChatHistoryEntry(channel, label, prefix, author, text, stamp, channelColor, nameColor, textColor, prefixColor));
	}

	protected static void ReadState(ParamsReadContext ctx)
	{
		bool hasClan;
		string myUid;
		string clanName;
		string clanTag;
		int clanColor;
		string clanDesc;
		int myRank;
		int count;
		int i;

		if (!ctx.Read(hasClan)) return;
		if (!ctx.Read(myUid)) return;
		if (!ctx.Read(clanName)) return;
		if (!ctx.Read(clanTag)) return;
		if (!ctx.Read(clanColor)) return;
		if (!ctx.Read(clanDesc)) return;
		if (!ctx.Read(myRank)) return;

		HasClan = hasClan;
		MyUid = myUid;
		ClanName = clanName;
		ClanTag = clanTag;
		ClanColorIndex = clanColor;
		ClanDescription = clanDesc;
		MyRank = myRank;

		Members.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			string uid;
			string name;
			int rank;
			bool online;
			float health;
			string lastSeen;
			string playerTitle;
			int playerXP;
			int playerTitleColor;
			if (!ctx.Read(uid)) return;
			if (!ctx.Read(name)) return;
			if (!ctx.Read(rank)) return;
			if (!ctx.Read(online)) return;
			if (!ctx.Read(health)) return;
			if (!ctx.Read(lastSeen)) return;
			if (!ctx.Read(playerTitle)) return;
			if (!ctx.Read(playerXP)) return;
			if (!ctx.Read(playerTitleColor)) return;
			Members.Insert(new SM_ClanMemberView(uid, name, rank, online, health, 0, "0 0 0", lastSeen, playerTitle, playerXP, playerTitleColor));
		}

		Invites.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			string invClan;
			string inviter;
			if (!ctx.Read(invClan)) return;
			if (!ctx.Read(inviter)) return;
			Invites.Insert(new SM_InviteView(invClan, inviter));
		}

		LogEntries.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			string logLine;
			if (!ctx.Read(logLine)) return;
			LogEntries.Insert(logLine);
		}

		Applications.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			string appUid;
			string appName;
			string appDate;
			if (!ctx.Read(appUid)) return;
			if (!ctx.Read(appName)) return;
			if (!ctx.Read(appDate)) return;
			Applications.Insert(new SM_ApplicationView(appUid, appName, appDate));
		}

		RankNames.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			string rankName;
			if (!ctx.Read(rankName)) return;
			RankNames.Insert(rankName);
		}

		if (!ctx.Read(MinRankToInvite)) return;
		if (!ctx.Read(MinRankToKick)) return;
		if (!ctx.Read(ServerHudEnabled)) return;
		HudEnabled = ServerHudEnabled;
		if (!ctx.Read(MinClanNameLength)) return;
		if (!ctx.Read(MaxClanNameLength)) return;
		if (!ctx.Read(MaxClanTagLength)) return;
		if (!ctx.Read(MaxClanDescriptionLength)) return;
		if (!ctx.Read(CreateClanLogoPaa)) return;
		if (!ctx.Read(ChatEnabled)) return;
		if (!ctx.Read(ChatColorDirect)) return;
		if (!ctx.Read(ChatColorDirectPlayer)) return;
		if (!ctx.Read(ChatColorGlobal)) return;
		if (!ctx.Read(ChatColorGlobalPlayer)) return;
		if (!ctx.Read(ChatColorServer)) return;
		if (!ctx.Read(ChatColorAlert)) return;
		if (!ctx.Read(PingsEnabled)) return;

		if (!ctx.Read(TopsEnabled)) return;
		if (!ctx.Read(TreasuryEnabled)) return;
		if (!ctx.Read(Treasury)) return;

		CurrencyItems.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			string currencyClass;
			int currencyValue;
			if (!ctx.Read(currencyClass)) return;
			if (!ctx.Read(currencyValue)) return;
			CurrencyItems.Insert(new SM_CurrencyItem(currencyClass, currencyValue));
		}

		if (!ctx.Read(MarketEnabled)) return;
		if (!ctx.Read(MarketFeePercent)) return;
		if (!ctx.Read(MarketListingFeePercent)) return;
		if (!ctx.Read(MaxLotsPerClan)) return;
		if (!ctx.Read(MinRankToSell)) return;
		if (!ctx.Read(ServerMarketEnabled)) return;
		if (!ctx.Read(AuctionEnabled)) return;
		if (!ctx.Read(AuctionListingFee)) return;
		if (!ctx.Read(AuctionListingFeePercent)) return;
		if (!ctx.Read(AuctionSaleTaxPercent)) return;
		if (!ctx.Read(AuctionMinDurationMinutes)) return;
		if (!ctx.Read(AuctionMaxDurationMinutes)) return;
		if (!ctx.Read(AuctionDefaultDurationMinutes)) return;
		if (!ctx.Read(AuctionMinBidStep)) return;
		if (!ctx.Read(AuctionMaxStartPrice)) return;
		if (!ctx.Read(AchievementsEnabled)) return;
		if (!ctx.Read(AchievementsClaimRewardManually)) return;
		if (!ctx.Read(PlayerExperienceEnabled)) return;
		if (!ctx.Read(PlayerExperienceClanListEnabled)) return;
		if (!ctx.Read(MapEnabled)) return;
		if (!ctx.Read(MaxMapMarkersPerClan)) return;
		if (!ctx.Read(MapMarkerMaxNameLength)) return;

		ServerMapMarkers.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			string serverMarkerName;
			vector serverMarkerPosition;
			int serverMarkerColor;
			string serverMarkerIcon;
			if (!ctx.Read(serverMarkerName)) return;
			if (!ctx.Read(serverMarkerPosition)) return;
			if (!ctx.Read(serverMarkerColor)) return;
			if (!ctx.Read(serverMarkerIcon)) return;
			if (MapEnabled)
				ServerMapMarkers.Insert(new SM_ServerMapMarker(serverMarkerName, serverMarkerPosition, serverMarkerColor, serverMarkerIcon, true));
		}

		AdminBaseMapMarkers.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			string adminBaseMarkerLabel;
			string adminBaseMarkerClanName;
			string adminBaseMarkerOwnerName;
			string adminBaseMarkerOwnerUid;
			vector adminBaseMarkerPosition;
			int adminBaseMarkerColor;
			string adminBaseMarkerIcon;
			if (!ctx.Read(adminBaseMarkerLabel)) return;
			if (!ctx.Read(adminBaseMarkerClanName)) return;
			if (!ctx.Read(adminBaseMarkerOwnerName)) return;
			if (!ctx.Read(adminBaseMarkerOwnerUid)) return;
			if (!ctx.Read(adminBaseMarkerPosition)) return;
			if (!ctx.Read(adminBaseMarkerColor)) return;
			if (!ctx.Read(adminBaseMarkerIcon)) return;
			if (MapEnabled)
				AdminBaseMapMarkers.Insert(new SM_AdminBaseMapMarker(adminBaseMarkerLabel, adminBaseMarkerClanName, adminBaseMarkerOwnerName, adminBaseMarkerOwnerUid, adminBaseMarkerPosition, adminBaseMarkerColor, adminBaseMarkerIcon));
		}

		MapMarkers.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			int markerId;
			string markerName;
			string markerAuthorUid;
			string markerAuthorName;
			vector markerPosition;
			int markerColor;
			int markerIcon;
			if (!ctx.Read(markerId)) return;
			if (!ctx.Read(markerName)) return;
			if (!ctx.Read(markerAuthorUid)) return;
			if (!ctx.Read(markerAuthorName)) return;
			if (!ctx.Read(markerPosition)) return;
			if (!ctx.Read(markerColor)) return;
			if (!ctx.Read(markerIcon)) return;
			if (MapEnabled)
			{
				MapMarkers.Insert(new SM_ClanMapMarker(markerId, markerName, markerAuthorUid, markerAuthorName, markerPosition, markerColor, markerIcon));
				if (markerIcon == SM_MapMarkerIconSet.DeathIcon())
					EnsureClanMarker3D(markerId);
			}
		}
		MapMarkers3DDirty = true;

		if (!ctx.Read(ClanLevelsEnabled)) return;
		if (!ctx.Read(StorageEnabled)) return;
		if (!ctx.Read(ClanLevel)) return;
		if (!ctx.Read(StorageCap)) return;
		if (!ctx.Read(LotCap)) return;
		if (!ctx.Read(StorageUsed)) return;
		if (!ctx.Read(HasBase)) return;
		if (!ctx.Read(BasePos)) return;
		if (!ctx.Read(StorageRadius)) return;
		if (!ctx.Read(BaseRadiusEnabled)) return;
		if (!ctx.Read(BaseRadius)) return;

		ClanLevels.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			int levelCost;
			int levelSlots;
			int levelLots;
			int levelServerMarketItems;
			if (!ctx.Read(levelCost)) return;
			if (!ctx.Read(levelSlots)) return;
			if (!ctx.Read(levelLots)) return;
			if (!ctx.Read(levelServerMarketItems)) return;
			ClanLevels.Insert(new SM_ClanLevel(levelCost, levelSlots, levelLots, levelServerMarketItems));
		}

		Perms.Clear();
		if (!ctx.Read(count)) return;
		for (i = 0; i < count; i++)
		{
			int permRank;
			if (!ctx.Read(permRank)) return;
			Perms.Insert(permRank);
		}

		if (!ctx.Read(LocalChatEnabled)) return;
		if (!ctx.Read(GlobalChatEnabled)) return;
		if (!ctx.Read(IsAdmin)) return;
		if (!ctx.Read(IsAdminMode)) return;
		if (!ctx.Read(ClanWebhookEnabled)) return;
		if (!ctx.Read(ClanWebhookConfigured)) return;
		if (!ctx.Read(ClanWebhookAuditEnabled)) return;
		string serverSessionKey;
		if (!ctx.Read(serverSessionKey)) return;
		ApplyServerSessionKey(serverSessionKey);

		if (!HasClan)
		{
			IsAdminMode = false;
			ClanWebhookEnabled = false;
			ClanWebhookConfigured = false;
			ClanWebhookAuditEnabled = false;
			HudMembers.Clear();
			HudDirty = true;
			Pings.Clear();
			PingsDirty = true;
			MapMarkers.Clear();
			if (!IsAdmin)
				AdminBaseMapMarkers.Clear();
		}

		if (!ChatEnabled)
		{
			LocalChatEnabled = false;
			GlobalChatEnabled = false;
			ClearChatBuffer();
		}

		EnsureClientSettingsLoaded();
		ApplyClientSettings();
		PruneClan3DMarkerKeys();
		OnStateChanged.Invoke();
	}

	protected static void ReadClanList(ParamsReadContext ctx)
	{
		int count;
		ClanList.Clear();
		if (!ctx.Read(count)) return;
		for (int i = 0; i < count; i++)
		{
			string name;
			string tag;
			int memberCount;
			string leaderName;
			string leaderTitle;
			int leaderTitleColor;
			if (!ctx.Read(name)) return;
			if (!ctx.Read(tag)) return;
			if (!ctx.Read(memberCount)) return;
			if (!ctx.Read(leaderName)) return;
			if (!ctx.Read(leaderTitle)) return;
			if (!ctx.Read(leaderTitleColor)) return;
			ClanList.Insert(new SM_ClanListEntry(name, tag, memberCount, leaderName, leaderTitle, leaderTitleColor));
		}
		OnClanListChanged.Invoke();
	}

	protected static void ReadPlayers(ParamsReadContext ctx)
	{
		int count;
		InvitablePlayers.Clear();
		if (!ctx.Read(count)) return;
		for (int i = 0; i < count; i++)
		{
			string uid;
			string name;
			if (!ctx.Read(uid)) return;
			if (!ctx.Read(name)) return;
			InvitablePlayers.Insert(new SM_PlayerEntry(uid, name));
		}
		OnPlayersChanged.Invoke();
	}

	protected static void ReadOnlinePlayers(ParamsReadContext ctx)
	{
		int count;
		OnlinePlayers.Clear();
		if (!ctx.Read(count))
			return;

		for (int i = 0; i < count; i++)
		{
			string uid;
			string name;
			string clanName;
			string title;
			int titleColor;
			if (!ctx.Read(uid)) return;
			if (!ctx.Read(name)) return;
			if (!ctx.Read(clanName)) return;
			if (!ctx.Read(title)) return;
			if (!ctx.Read(titleColor)) return;
			OnlinePlayers.Insert(new SM_OnlinePlayerView(uid, name, clanName, title, titleColor));
		}

		OnOnlinePlayersChanged.Invoke();
	}

	protected static void ReadHud(ParamsReadContext ctx)
	{
		int count;
		HudMembers.Clear();
		if (ctx.Read(count))
		{
			for (int i = 0; i < count; i++)
			{
				string uid;
				string name;
				float health;
				int status;
				vector position;
				if (!ctx.Read(uid)) break;
				if (!ctx.Read(name)) break;
				if (!ctx.Read(health)) break;
				if (!ctx.Read(status)) break;
				if (!ctx.Read(position)) break;
				HudMembers.Insert(new SM_ClanMemberView(uid, name, 0, true, health, status, position));
			}
		}
		RecomputeVisibleHudMembers();
		HudDirty = true;
	}

	protected static void ReadMark(ParamsReadContext ctx)
	{
		string author;
		vector position;
		float duration;
		string authorUid;
		int pingId;
		string label = "";
		string iconPath = "";
		int color = 0;
		bool suppressDefaultChat = false;
		if (!ctx.Read(author)) return;
		if (!ctx.Read(position)) return;
		if (!ctx.Read(duration)) return;
		if (!ctx.Read(authorUid)) return;
		if (!ctx.Read(pingId)) return;
		ctx.Read(label);
		ctx.Read(iconPath);
		ctx.Read(color);
		ctx.Read(suppressDefaultChat);

		for (int i = Pings.Count() - 1; i >= 0; i--)
		{
			if (Pings[i].AuthorUid == authorUid && Pings[i].Id == pingId)
				Pings.Remove(i);
		}

		Pings.Insert(new SM_ClanPing(author, position, duration, pingId, authorUid, label, iconPath, color, suppressDefaultChat));

		if (!suppressDefaultChat)
		{
			int markColor = GetClanColor();
			AddSystemChat(SM_ChatChannel.SERVER, "#STR_SMP_00639", "", author + "#STR_SMP_00075", markColor, ARGB(255, 230, 230, 230));
		}

		while (Pings.Count() > 30)
			Pings.Remove(0);

		PingsDirty = true;
	}

	protected static void ReadMarkClear(ParamsReadContext ctx)
	{
		string authorUid;
		if (!ctx.Read(authorUid)) return;

		for (int i = Pings.Count() - 1; i >= 0; i--)
		{
			if (Pings[i].AuthorUid == authorUid)
				Pings.Remove(i);
		}

		PingsDirty = true;
	}

	protected static void ReadTops(ParamsReadContext ctx)
	{
		int count;

		if (!ctx.Read(TopsEnabled)) return;
		if (!ctx.Read(MinShotsForAccuracyTop)) return;
		if (!ctx.Read(TopSeasonSecondsLeft)) return;

		int categoryCount;
		TopCategoryEnabled.Clear();
		if (!ctx.Read(categoryCount)) return;
		for (int c = 0; c < categoryCount; c++)
		{
			bool categoryEnabled;
			if (!ctx.Read(categoryEnabled)) return;
			if (categoryEnabled)
				TopCategoryEnabled.Insert(1);
			else
				TopCategoryEnabled.Insert(0);
		}

		TopClans.Clear();
		if (!ctx.Read(count)) return;
		for (int i = 0; i < count; i++)
		{
			string topName;
			string topTag;
			int topColor;
			int playerKills;
			int playerDeaths;
			int zombieKills;
			int onlineSeconds;
			int bestLifeSeconds;
			string bestLifeName;
			int distanceWalked;
			int shotsFired;
			int hitsLanded;
			int headshots;
			int longestPlayerKillDistance;
			string longestPlayerKillName;

			if (!ctx.Read(topName)) return;
			if (!ctx.Read(topTag)) return;
			if (!ctx.Read(topColor)) return;
			if (!ctx.Read(playerKills)) return;
			if (!ctx.Read(playerDeaths)) return;
			if (!ctx.Read(zombieKills)) return;
			if (!ctx.Read(onlineSeconds)) return;
			if (!ctx.Read(bestLifeSeconds)) return;
			if (!ctx.Read(bestLifeName)) return;
			if (!ctx.Read(distanceWalked)) return;
			if (!ctx.Read(shotsFired)) return;
			if (!ctx.Read(hitsLanded)) return;
			if (!ctx.Read(headshots)) return;
			if (!ctx.Read(longestPlayerKillDistance)) return;
			if (!ctx.Read(longestPlayerKillName)) return;

			SM_ClanTopEntry entry = new SM_ClanTopEntry();
			entry.Name = topName;
			entry.Tag = topTag;
			entry.ColorIndex = topColor;
			entry.Stats.PlayerKills = playerKills;
			entry.Stats.PlayerDeaths = playerDeaths;
			entry.Stats.ZombieKills = zombieKills;
			entry.Stats.OnlineSeconds = onlineSeconds;
			entry.Stats.BestLifeSeconds = bestLifeSeconds;
			entry.Stats.BestLifeName = bestLifeName;
			entry.Stats.DistanceWalked = distanceWalked;
			entry.Stats.ShotsFired = shotsFired;
			entry.Stats.HitsLanded = hitsLanded;
			entry.Stats.Headshots = headshots;
			entry.Stats.LongestPlayerKillDistance = longestPlayerKillDistance;
			entry.Stats.LongestPlayerKillName = longestPlayerKillName;
			TopClans.Insert(entry);
		}

		OnTopsChanged.Invoke();
	}

	protected static void ReadClanInfo(ParamsReadContext ctx)
	{
		string name;
		string tag;
		int color;
		string desc;
		int count;

		if (!ctx.Read(name)) return;
		if (!ctx.Read(tag)) return;
		if (!ctx.Read(color)) return;
		if (!ctx.Read(desc)) return;

		InfoName = name;
		InfoTag = tag;
		InfoColorIndex = color;
		InfoDescription = desc;

		InfoMembers.Clear();
		if (!ctx.Read(count)) return;
		for (int i = 0; i < count; i++)
		{
			string memberName;
			int rank;
			bool online;
			string lastSeen;
			string playerTitle;
			int playerXP;
			int playerTitleColor;
			if (!ctx.Read(memberName)) return;
			if (!ctx.Read(rank)) return;
			if (!ctx.Read(online)) return;
			if (!ctx.Read(lastSeen)) return;
			if (!ctx.Read(playerTitle)) return;
			if (!ctx.Read(playerXP)) return;
			if (!ctx.Read(playerTitleColor)) return;
			InfoMembers.Insert(new SM_ClanMemberView("", memberName, rank, online, 0, 0, "0 0 0", lastSeen, playerTitle, playerXP, playerTitleColor));
		}

		OnClanInfoChanged.Invoke();
	}

	protected static void ReadMarket(ParamsReadContext ctx)
	{
		int count;
		MarketLots.Clear();
		if (ctx.Read(count))
		{
			for (int i = 0; i < count; i++)
			{
				int id;
				string className;
				float quantity;
				int attachCount;
				float health;
				int price;
				string clanName;
				string sellerName;
				string date;
				if (!ctx.Read(id)) break;
				if (!ctx.Read(className)) break;
				if (!ctx.Read(quantity)) break;
				if (!ctx.Read(attachCount)) break;
				if (!ctx.Read(health)) break;
				if (!ctx.Read(price)) break;
				if (!ctx.Read(clanName)) break;
				string sellerUid;
				if (!ctx.Read(sellerUid)) break;
				if (!ctx.Read(sellerName)) break;
				if (!ctx.Read(date)) break;

				SM_MarketLotView view = new SM_MarketLotView(id, className, quantity, attachCount, health, price, clanName, sellerUid, sellerName, date);

				int attachClassCount;
				if (!ctx.Read(attachClassCount)) break;
				bool attachReadFail = false;
				for (int ac = 0; ac < attachClassCount; ac++)
				{
					string attachClass;
					if (!ctx.Read(attachClass))
					{
						attachReadFail = true;
						break;
					}
					view.AttachClasses.Insert(attachClass);
				}
				if (attachReadFail) break;

				MarketLots.Insert(view);
			}
		}
		OnMarketChanged.Invoke();
	}

	protected static void ReadServerMarket(ParamsReadContext ctx)
	{
		int count;
		ServerMarketItems.Clear();

		if (!ctx.Read(ServerMarketRotationId)) return;
		if (!ctx.Read(ServerMarketNextRefreshSeconds)) return;
		if (!ctx.Read(ServerMarketVisibleItems)) return;
		if (!ctx.Read(ServerMarketTotalItems)) return;
		if (!ctx.Read(count)) return;

		for (int i = 0; i < count; i++)
		{
			int id;
			bool locked;
			int requiredClanLevel;
			if (!ctx.Read(id)) return;
			if (!ctx.Read(locked)) return;
			if (!ctx.Read(requiredClanLevel)) return;

			SM_ServerMarketItemView view = new SM_ServerMarketItemView(id, locked, requiredClanLevel);
			if (!locked)
			{
				if (!ctx.Read(view.ClassName)) return;
				if (!ctx.Read(view.BasePrice)) return;
				if (!ctx.Read(view.CurrentPrice)) return;
				if (!ctx.Read(view.InitialQuantity)) return;
				if (!ctx.Read(view.Quantity)) return;
				if (!ctx.Read(view.MaxBuyPerPlayer)) return;
				if (!ctx.Read(view.BoughtByPlayer)) return;
			}

			ServerMarketItems.Insert(view);
		}

		OnServerMarketChanged.Invoke();
	}

	protected static void ReadAuction(ParamsReadContext ctx)
	{
		int count;
		AuctionLots.Clear();

		if (!ctx.Read(AuctionEnabled)) return;
		if (!ctx.Read(AuctionListingFee)) return;
		if (!ctx.Read(AuctionListingFeePercent)) return;
		if (!ctx.Read(AuctionSaleTaxPercent)) return;
		if (!ctx.Read(AuctionMinDurationMinutes)) return;
		if (!ctx.Read(AuctionMaxDurationMinutes)) return;
		if (!ctx.Read(AuctionDefaultDurationMinutes)) return;
		if (!ctx.Read(AuctionMinBidStep)) return;
		if (!ctx.Read(AuctionMaxStartPrice)) return;
		if (!ctx.Read(count)) return;

		for (int i = 0; i < count; i++)
		{
			int id;
			string className;
			float quantity;
			int attachCount;
			float health;
			string sellerUid;
			string sellerName;
			string sellerClanName;
			int startPrice;
			int bidStep;
			int currentBid;
			string currentBidderName;
			int secondsLeft;
			string date;
			if (!ctx.Read(id)) return;
			if (!ctx.Read(className)) return;
			if (!ctx.Read(quantity)) return;
			if (!ctx.Read(attachCount)) return;
			if (!ctx.Read(health)) return;
			if (!ctx.Read(sellerUid)) return;
			if (!ctx.Read(sellerName)) return;
			if (!ctx.Read(sellerClanName)) return;
			if (!ctx.Read(startPrice)) return;
			if (!ctx.Read(bidStep)) return;
			if (!ctx.Read(currentBid)) return;
			if (!ctx.Read(currentBidderName)) return;
			if (!ctx.Read(secondsLeft)) return;
			if (!ctx.Read(date)) return;

			SM_AuctionLotView view = new SM_AuctionLotView(id, className, quantity, attachCount, health, sellerUid, sellerName, sellerClanName, startPrice, bidStep, currentBid, currentBidderName, secondsLeft, date);

			int attachClassCount;
			if (!ctx.Read(attachClassCount)) return;
			for (int ac = 0; ac < attachClassCount; ac++)
			{
				string attachClass;
				if (!ctx.Read(attachClass)) return;
				view.AttachClasses.Insert(attachClass);
			}

			AuctionLots.Insert(view);
		}

		OnAuctionChanged.Invoke();
	}

	protected static void ReadAchievements(ParamsReadContext ctx)
	{
		int count;
		AchievementViews.Clear();

		if (!ctx.Read(AchievementsEnabled)) return;
		if (!ctx.Read(AchievementsClaimRewardManually)) return;
		if (!ctx.Read(count)) return;

		for (int i = 0; i < count; i++)
		{
			int scope;
			string id;
			string name;
			string description;
			string type;
			int target;
			int progress;
			bool completed;
			bool claimed;
			bool hidden;
			int rewardMoney;
			int rewardClanTreasury;
			string rewardContainerClass;

			if (!ctx.Read(scope)) return;
			if (!ctx.Read(id)) return;
			if (!ctx.Read(name)) return;
			if (!ctx.Read(description)) return;
			if (!ctx.Read(type)) return;
			if (!ctx.Read(target)) return;
			if (!ctx.Read(progress)) return;
			if (!ctx.Read(completed)) return;
			if (!ctx.Read(claimed)) return;
			if (!ctx.Read(hidden)) return;
			if (!ctx.Read(rewardMoney)) return;
			if (!ctx.Read(rewardClanTreasury)) return;
			if (!ctx.Read(rewardContainerClass)) return;

			AchievementViews.Insert(new SM_AchievementView(scope, id, name, description, type, target, progress, completed, claimed, hidden, rewardMoney, rewardClanTreasury, rewardContainerClass));
		}

		OnAchievementsChanged.Invoke();
	}

	protected static void ReadPlayerTitles(ParamsReadContext ctx)
	{
		int count;
		PlayerTitles.Clear();

		if (!ctx.Read(PlayerExperienceEnabled)) return;
		if (!ctx.Read(MyPlayerTitle)) return;
		if (!ctx.Read(MyPlayerXP)) return;
		if (!ctx.Read(MyPlayerRankIndex)) return;
		if (!ctx.Read(MyPlayerNextXP)) return;
		if (!ctx.Read(MyPlayerTitleColor)) return;
		if (!ctx.Read(count)) return;

		for (int i = 0; i < count; i++)
		{
			string uid;
			string name;
			string title;
			int xp;
			int rankIndex;
			int nextXP;
			int color;
			string clanName;
			bool online;

			if (!ctx.Read(uid)) return;
			if (!ctx.Read(name)) return;
			if (!ctx.Read(title)) return;
			if (!ctx.Read(xp)) return;
			if (!ctx.Read(rankIndex)) return;
			if (!ctx.Read(nextXP)) return;
			if (!ctx.Read(color)) return;
			if (!ctx.Read(clanName)) return;
			if (!ctx.Read(online)) return;

			PlayerTitles.Insert(new SM_PlayerTitleView(uid, name, title, xp, rankIndex, nextXP, color, clanName, online));
		}

		OnPlayerTitlesChanged.Invoke();
	}

	protected static bool ReadContractView(ParamsReadContext ctx, out SM_ContractView view)
	{
		view = null;

		int id;
		int type;
		int status;
		string creatorName;
		string creatorClanName;
		string targetName;
		string targetClanName;
		string itemClassName;
		string itemDisplayName;
		int itemQuantity;
		int price;
		int reward;
		int fee;
		int penalty;
		string executorName;
		string executorClanName;
		int secondsLeft;
		string date;
		bool isCreator;
		bool isExecutor;

		if (!ctx.Read(id)) return false;
		if (!ctx.Read(type)) return false;
		if (!ctx.Read(status)) return false;
		if (!ctx.Read(creatorName)) return false;
		if (!ctx.Read(creatorClanName)) return false;
		if (!ctx.Read(targetName)) return false;
		if (!ctx.Read(targetClanName)) return false;
		if (!ctx.Read(itemClassName)) return false;
		if (!ctx.Read(itemDisplayName)) return false;
		if (!ctx.Read(itemQuantity)) return false;
		if (!ctx.Read(price)) return false;
		if (!ctx.Read(reward)) return false;
		if (!ctx.Read(fee)) return false;
		if (!ctx.Read(penalty)) return false;
		if (!ctx.Read(executorName)) return false;
		if (!ctx.Read(executorClanName)) return false;
		if (!ctx.Read(secondsLeft)) return false;
		if (!ctx.Read(date)) return false;
		if (!ctx.Read(isCreator)) return false;
		if (!ctx.Read(isExecutor)) return false;

		view = new SM_ContractView(id, type, status);
		view.CreatorName = creatorName;
		view.CreatorClanName = creatorClanName;
		view.TargetName = targetName;
		view.TargetClanName = targetClanName;
		view.ItemClassName = itemClassName;
		view.ItemDisplayName = itemDisplayName;
		view.ItemQuantity = itemQuantity;
		view.Price = price;
		view.Reward = reward;
		view.Fee = fee;
		view.Penalty = penalty;
		view.ExecutorName = executorName;
		view.ExecutorClanName = executorClanName;
		view.SecondsLeft = secondsLeft;
		view.Date = date;
		view.IsCreator = isCreator;
		view.IsExecutor = isExecutor;
		if (isCreator || isExecutor)
			view.Mine = true;
		else
			view.Mine = false;
		return true;
	}

	protected static void ReadContracts(ParamsReadContext ctx)
	{
		ContractViews.Clear();
		ContractHistoryViews.Clear();
		ContractTargets.Clear();
		ContractItemOptions.Clear();
		ContractMarkers.Clear();

		if (!ctx.Read(ContractsEnabled)) return;
		if (!ctx.Read(ContractsCanCreate)) return;
		if (!ctx.Read(ContractsMinPrice)) return;
		if (!ctx.Read(ContractsMaxPrice)) return;
		if (!ctx.Read(ContractsFeePercent)) return;
		if (!ctx.Read(ContractsPenaltyPercent)) return;
		if (!ctx.Read(ContractsMinDurationMinutes)) return;
		if (!ctx.Read(ContractsMaxDurationMinutes)) return;

		int activeCount;
		if (!ctx.Read(activeCount)) return;
		for (int i = 0; i < activeCount; i++)
		{
			SM_ContractView activeView;
			if (!ReadContractView(ctx, activeView)) return;
			ContractViews.Insert(activeView);
		}

		int historyCount;
		if (!ctx.Read(historyCount)) return;
		for (int h = 0; h < historyCount; h++)
		{
			SM_ContractView historyView;
			if (!ReadContractView(ctx, historyView)) return;
			ContractHistoryViews.Insert(historyView);
		}

		int targetCount;
		if (!ctx.Read(targetCount)) return;
		for (int t = 0; t < targetCount; t++)
		{
			string uid;
			string name;
			string clanName;
			string title;
			bool online;
			if (!ctx.Read(uid)) return;
			if (!ctx.Read(name)) return;
			if (!ctx.Read(clanName)) return;
			if (!ctx.Read(title)) return;
			if (!ctx.Read(online)) return;
			ContractTargets.Insert(new SM_ContractTargetView(uid, name, clanName, title, online));
		}

		int itemCount;
		if (!ctx.Read(itemCount)) return;
		for (int it = 0; it < itemCount; it++)
		{
			string className;
			string displayName;
			if (!ctx.Read(className)) return;
			if (!ctx.Read(displayName)) return;
			ContractItemOptions.Insert(new SM_ContractItemOptionView(className, displayName));
		}

		int markerCount;
		if (!ctx.Read(markerCount)) return;
		for (int m = 0; m < markerCount; m++)
		{
			int contractId;
			int contractType;
			string label;
			vector position;
			int secondsLeft;
			if (!ctx.Read(contractId)) return;
			if (!ctx.Read(contractType)) return;
			if (!ctx.Read(label)) return;
			if (!ctx.Read(position)) return;
			if (!ctx.Read(secondsLeft)) return;
			ContractMarkers.Insert(new SM_ContractMarkerView(contractId, contractType, label, position, secondsLeft));
		}

		OnContractsChanged.Invoke();
	}

	protected static void ReadContractTargetPreview(ParamsReadContext ctx)
	{
		ContractTargetPreviewUid = "";
		ContractTargetPreviewName = "";
		ContractTargetPreviewClass = "";
		ContractTargetPreviewHandClass = "";
		ContractTargetPreviewAttachments.Clear();

		if (!ctx.Read(ContractTargetPreviewUid)) return;
		if (!ctx.Read(ContractTargetPreviewName)) return;
		if (!ctx.Read(ContractTargetPreviewClass)) return;
		if (!ctx.Read(ContractTargetPreviewHandClass)) return;

		int count;
		if (!ctx.Read(count)) return;
		for (int i = 0; i < count; i++)
		{
			string className;
			if (!ctx.Read(className)) return;
			if (className != "")
				ContractTargetPreviewAttachments.Insert(className);
		}

		OnContractTargetPreviewChanged.Invoke();
	}

	protected static void ReadStorage(ParamsReadContext ctx)
	{
		int count;
		StorageItems.Clear();
		if (ctx.Read(count))
		{
			for (int i = 0; i < count; i++)
			{
				int id;
				string className;
				float quantity;
				int attachCount;
				float health;
				string depositorName;
				string date;
				if (!ctx.Read(id)) break;
				if (!ctx.Read(className)) break;
				if (!ctx.Read(quantity)) break;
				if (!ctx.Read(attachCount)) break;
				if (!ctx.Read(health)) break;
				if (!ctx.Read(depositorName)) break;
				if (!ctx.Read(date)) break;

				SM_StorageItemView view = new SM_StorageItemView(id, className, quantity, attachCount, health, depositorName, date);

				int attachClassCount;
				if (!ctx.Read(attachClassCount)) break;
				bool attachReadFail = false;
				for (int ac = 0; ac < attachClassCount; ac++)
				{
					string attachClass;
					if (!ctx.Read(attachClass))
					{
						attachReadFail = true;
						break;
					}
					view.AttachClasses.Insert(attachClass);
				}
				if (attachReadFail) break;

				StorageItems.Insert(view);
			}
		}
		StorageUsed = StorageItems.Count();
		OnStorageChanged.Invoke();
	}
}
