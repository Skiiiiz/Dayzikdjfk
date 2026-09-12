class SM_ClanMember
{
	string Uid;
	string Name;
	int Rank;
	string LastSeen;

	void SM_ClanMember(string uid = "", string name = "", int rank = 0, string lastSeen = "")
	{
		Uid = uid;
		Name = name;
		Rank = rank;
		LastSeen = lastSeen;
	}
}

class SM_ClanApplication
{
	string Uid;
	string Name;
	string Date;

	void SM_ClanApplication(string uid = "", string name = "", string date = "")
	{
		Uid = uid;
		Name = name;
		Date = date;
	}
}

class SM_AdminChatLogEntry
{
	string Uid;
	string Name;
	string Channel;
	string Text;
	string Stamp;

	void SM_AdminChatLogEntry(string uid = "", string name = "", string channel = "", string text = "", string stamp = "")
	{
		Uid = uid;
		Name = name;
		Channel = channel;
		Text = text;
		Stamp = stamp;
	}
}

class SM_ClanMapMarker
{
	int Id;
	string Name;
	string AuthorUid;
	string AuthorName;
	vector Position;
	int Color;
	int Icon;

	void SM_ClanMapMarker(int id = 0, string name = "", string authorUid = "", string authorName = "", vector position = "0 0 0", int color = 0, int icon = 0)
	{
		Id = id;
		Name = name;
		AuthorUid = authorUid;
		AuthorName = authorName;
		Position = position;
		Color = color;
		Icon = icon;
	}
}

class SM_PersonalMapMarker
{
	int Id;
	string Name;
	vector Position;
	int Color;
	bool Show3D;
	int Icon;

	void SM_PersonalMapMarker(int id = 0, string name = "", vector position = "0 0 0", int color = 0, bool show3D = false, int icon = 0)
	{
		Id = id;
		Name = name;
		Position = position;
		Color = color;
		Show3D = show3D;
		Icon = icon;
	}
}

class SM_PersonalMapMarkersFile
{
	int NextId = 1;
	ref array<ref SM_PersonalMapMarker> Markers = new array<ref SM_PersonalMapMarker>;
}

class SM_DeathMapMarkerView
{
	string Name;
	vector Position;
	int Color;
	int Icon;

	void SM_DeathMapMarkerView(string name = "", vector position = "0 0 0", int color = 0, int icon = 0)
	{
		Name = name;
		Position = position;
		Color = color;
		Icon = icon;
	}
}

// Клиентские настройки игрока (персист в $profile, сервером не управляются)
class SM_ClientSettingsData
{
	int ChatVisibleMessages = 6;
	bool ChatHistoryEnabled = false;   // по умолчанию выкл
	bool ChatTimestamps = false;       // по умолчанию выкл
	int ChatFadeSeconds = 8;
	int ChatOpacity = 100;             // проценты
	int ChatOffsetX = 0;
	int ChatOffsetY = 0;
	bool HudEnabled = true;
	int HudMaxVisibleMembers = 8;
	int HudOpacity = 100;              // проценты
	int HudOffsetX = 12;
	int HudOffsetY = 120;
	bool HudCompact = false;
	bool HudShowSelf = false;
	bool HudShowDistance = true;
	bool HudShowDirection = true;
	bool MarkersEnabled = true;
	bool Markers3D = true;
	int MarkerDistance = 500;      // 3D-метки над игроками
	int PingMarkerDistance = 500;  // пинги игроков
	bool PingMarkerDistanceReady = false;
	ref array<string> HudHiddenUids = new array<string>;     // кого НЕ показывать в HUD (по умолчанию пусто = все)
	ref array<string> Clan3DMarkerKeys = new array<string>;  // выбранные клановые 3D-метки: ClanName|MarkerId
}

class SM_ServerMapMarker
{
	string Name;
	vector Position;
	ref SM_RGBColorConfig ColorRGB = new SM_RGBColorConfig;
	string IconPaa;
	bool Enabled;

	void SM_ServerMapMarker(string name = "", vector position = "0 0 0", int color = 0, string iconPaa = "", bool enabled = true)
	{
		Name = name;
		Position = position;
		if (!ColorRGB)
			ColorRGB = new SM_RGBColorConfig;
		if (color != 0)
			ColorRGB.FromARGB(color);
		IconPaa = iconPaa;
		Enabled = enabled;
	}

	int GetColor(int fallback = 0)
	{
		if (ColorRGB && ColorRGB.IsConfigured())
			return ColorRGB.ToARGB(fallback);
		return fallback;
	}

	string GetMapIconPath()
	{
		return SM_MapMarkerIconPath.ForMapWidget(IconPaa);
	}

	string GetIconPath()
	{
		return SM_MapMarkerIconPath.ForImageWidget(IconPaa);
	}
}

class SM_PartyTradeZoneConfig
{
	string Name;
	vector Position;
	float RadiusMeters = 30.0;
	bool Enabled = true;

	void SM_PartyTradeZoneConfig(string name = "", vector position = "0 0 0", float radiusMeters = 30.0, bool enabled = true)
	{
		Name = name;
		Position = position;
		RadiusMeters = radiusMeters;
		Enabled = enabled;
	}
}

class SM_PartyTradeZoneRestrictionConfig
{
	bool Enabled = false;
	bool RequireForListing = true;
	bool RequireForTaking = true;
	ref array<ref SM_PartyTradeZoneConfig> Zones = new array<ref SM_PartyTradeZoneConfig>;
}

class SM_ExternalServerMapMarker
{
	string Source;
	string Id;
	string Name;
	vector Position;
	int Color;
	string IconPaa;

	void SM_ExternalServerMapMarker(string source = "", string id = "", string name = "", vector position = "0 0 0", int color = 0, string iconPaa = "")
	{
		Source = source;
		Id = id;
		Name = name;
		Position = position;
		Color = color;
		IconPaa = iconPaa;
	}

	string GetMapIconPath()
	{
		return SM_MapMarkerIconPath.ForMapWidget(IconPaa);
	}
}

class SM_AdminBaseMapMarker
{
	string Label;
	string ClanName;
	string OwnerName;
	string OwnerUid;
	vector Position;
	int Color;
	string IconPaa;

	void SM_AdminBaseMapMarker(string label = "", string clanName = "", string ownerName = "", string ownerUid = "", vector position = "0 0 0", int color = 0, string iconPaa = "")
	{
		Label = label;
		ClanName = clanName;
		OwnerName = ownerName;
		OwnerUid = ownerUid;
		Position = position;
		Color = color;
		IconPaa = iconPaa;
	}

	string GetMapIconPath()
	{
		return SM_MapMarkerIconPath.ForMapWidget(IconPaa, "\\dz\\gear\\navigation\\data\\map_camp_ca.paa");
	}

	string GetIconPath()
	{
		return SM_MapMarkerIconPath.ForImageWidget(IconPaa, "\\dz\\gear\\navigation\\data\\map_camp_ca.paa");
	}
}

class SM_ClanStats
{
	int PlayerKills;
	int PlayerDeaths;
	int ZombieKills;
	int OnlineSeconds;
	int BestLifeSeconds;
	string BestLifeName;
	int DistanceWalked;
	int ShotsFired;
	int HitsLanded;
	int Headshots;
	int LongestPlayerKillDistance;
	string LongestPlayerKillName;
	int TreasuryDeposited;
	int MarketSells;
	int StorageDeposits;
	int AuctionSales;
	int AuctionWins;

	int GetValue(int category)
	{
		switch (category)
		{
			case SM_TopCategory.PLAYER_KILLS: return PlayerKills;
			case SM_TopCategory.KILL_DEATH:   return PlayerKills;
			case SM_TopCategory.ZOMBIE_KILLS: return ZombieKills;
			case SM_TopCategory.ONLINE_TIME:  return OnlineSeconds;
			case SM_TopCategory.BEST_LIFE:    return BestLifeSeconds;
			case SM_TopCategory.DISTANCE:     return DistanceWalked;
			case SM_TopCategory.ACCURACY:     return HitsLanded;
			case SM_TopCategory.HEADSHOTS:    return Headshots;
			case SM_TopCategory.LONGEST_PLAYER_KILL: return LongestPlayerKillDistance;
		}
		return 0;
	}
}

class SM_Clan
{
	string Id;
	string Name;
	string Tag;
	int Color;
	string Description;
	int Treasury;
	int Level = 1;
	int NextMapMarkerId = 1;
	bool HasBase = false;
	vector BasePos = "0 0 0";
	bool DiscordWebhookEnabled = false;
	string DiscordWebhookUrl = "";
	bool DiscordAuditEnabled = false;
	ref SM_ClanStats Stats = new SM_ClanStats;
	ref array<string> ClaimedAchievements = new array<string>;
	ref array<string> NotifiedAchievements = new array<string>;
	ref array<ref SM_ClanMember> Members = new array<ref SM_ClanMember>;
	ref array<ref SM_ClanApplication> Applications = new array<ref SM_ClanApplication>;
	ref array<ref SM_ClanMapMarker> MapMarkers = new array<ref SM_ClanMapMarker>;
	ref array<string> Log = new array<string>;
	ref array<int> Perms = new array<int>;

	int GetMinRank(int action)
	{
		if (Perms && action >= 0 && action < Perms.Count())
			return Perms[action];
		return 0;
	}

	SM_ClanApplication FindApplication(string uid)
	{
		foreach (SM_ClanApplication app : Applications)
		{
			if (app.Uid == uid)
				return app;
		}
		return null;
	}

	SM_ClanMember FindMember(string uid)
	{
		foreach (SM_ClanMember m : Members)
		{
			if (m.Uid == uid)
				return m;
		}
		return null;
	}

	SM_ClanMember FindMemberByRank(int rank)
	{
		foreach (SM_ClanMember m : Members)
		{
			if (m.Rank == rank)
				return m;
		}
		return null;
	}
}

class SM_ClanDatabase
{
	ref array<ref SM_Clan> Clans = new array<ref SM_Clan>;

	int LastRewardYear;
	int LastRewardMonth;
	int LastRewardStampMinutes;
	int LastRewardStampSeconds;
}

class SM_ClanIndexEntry
{
	string Id;
	string Name;
	string FileName;

	void SM_ClanIndexEntry(string id = "", string name = "", string file = "")
	{
		Id = id;
		Name = name;
		FileName = file;
	}
}

class SM_ClanIndex
{
	ref array<ref SM_ClanIndexEntry> Clans = new array<ref SM_ClanIndexEntry>;

	int LastRewardYear;
	int LastRewardMonth;
	int LastRewardStampMinutes;
	int LastRewardStampSeconds;
}

class SM_CurrencyItem
{
	string Classname;
	int Value;

	void SM_CurrencyItem(string classname = "", int value = 1)
	{
		Classname = classname;
		Value = value;
	}
}

class SM_ClanLevel
{
	int UpgradeCost;
	int StorageSlots;
	int MarketLots;
	int ServerMarketItems;

	void SM_ClanLevel(int upgradeCost = 0, int storageSlots = 0, int marketLots = 0, int serverMarketItems = 0)
	{
		UpgradeCost = upgradeCost;
		StorageSlots = storageSlots;
		MarketLots = marketLots;
		ServerMarketItems = serverMarketItems;
	}
}


class SM_ClanMemberView
{
	string Uid;
	string Name;
	int Rank;
	bool Online;
	float Health;
	int Status;
	vector Position;
	string LastSeen;
	string PlayerTitle;
	int PlayerXP;
	int PlayerTitleColor;

	void SM_ClanMemberView(string uid = "", string name = "", int rank = 0, bool online = false, float health = 0, int status = 0, vector position = "0 0 0", string lastSeen = "", string playerTitle = "", int playerXP = 0, int playerTitleColor = 0)
	{
		Uid = uid;
		Name = name;
		Rank = rank;
		Online = online;
		Health = health;
		Status = status;
		Position = position;
		LastSeen = lastSeen;
		PlayerTitle = playerTitle;
		PlayerXP = playerXP;
		PlayerTitleColor = playerTitleColor;
	}
}

class SM_PlayerExperienceRecord
{
	string Uid;
	string Name;
	int XP;
	int ZombieKills;
	int PlayerKills;
	int OnlineSeconds;

	void SM_PlayerExperienceRecord(string uid = "", string name = "")
	{
		Uid = uid;
		Name = name;
	}
}

class SM_PlayerExperienceDB
{
	ref array<ref SM_PlayerExperienceRecord> Players = new array<ref SM_PlayerExperienceRecord>;
}

class SM_PlayerTitleView
{
	string Uid;
	string Name;
	string Title;
	int XP;
	int RankIndex;
	int NextXP;
	int Color;
	string ClanName;
	bool Online;

	void SM_PlayerTitleView(string uid = "", string name = "", string title = "", int xp = 0, int rankIndex = 0, int nextXP = 0, int color = 0, string clanName = "", bool online = false)
	{
		Uid = uid;
		Name = name;
		Title = title;
		XP = xp;
		RankIndex = rankIndex;
		NextXP = nextXP;
		Color = color;
		ClanName = clanName;
		Online = online;
	}
}

class SM_ClanListEntry
{
	string Name;
	string Tag;
	int MemberCount;
	string LeaderName;
	string LeaderTitle;
	int LeaderTitleColor;

	void SM_ClanListEntry(string name = "", string tag = "", int count = 0, string leader = "", string leaderTitle = "", int leaderTitleColor = 0)
	{
		Name = name;
		Tag = tag;
		MemberCount = count;
		LeaderName = leader;
		LeaderTitle = leaderTitle;
		LeaderTitleColor = leaderTitleColor;
	}
}

class SM_PlayerEntry
{
	string Uid;
	string Name;

	void SM_PlayerEntry(string uid = "", string name = "")
	{
		Uid = uid;
		Name = name;
	}
}

class SM_OnlinePlayerView
{
	string Uid;
	string Name;
	string ClanName;
	string Title;
	int TitleColor;

	void SM_OnlinePlayerView(string uid = "", string name = "", string clanName = "", string title = "", int titleColor = 0)
	{
		Uid = uid;
		Name = name;
		ClanName = clanName;
		Title = title;
		TitleColor = titleColor;
	}
}

class SM_InviteView
{
	string ClanName;
	string InviterName;

	void SM_InviteView(string clanName = "", string inviter = "")
	{
		ClanName = clanName;
		InviterName = inviter;
	}
}

class SM_ApplicationView
{
	string Uid;
	string Name;
	string Date;

	void SM_ApplicationView(string uid = "", string name = "", string date = "")
	{
		Uid = uid;
		Name = name;
		Date = date;
	}
}

class SM_ClanTopEntry
{
	string Name;
	string Tag;
	int ColorIndex;
	ref SM_ClanStats Stats = new SM_ClanStats;
}


class SM_Invite
{
	string ClanName;
	string InviterName;
	int SecondsLeft;

	void SM_Invite(string clanName = "", string inviter = "", int secondsLeft = 120)
	{
		ClanName = clanName;
		InviterName = inviter;
		SecondsLeft = secondsLeft;
	}
}


class SM_ClanPing
{
	int Id;
	string AuthorUid;
	string AuthorName;
	vector Position;
	float TimeLeft;
	string Label;
	string IconPath;
	int Color;
	bool SuppressDefaultChat;

	void SM_ClanPing(string author = "", vector position = "0 0 0", float timeLeft = 30, int id = 0, string authorUid = "", string label = "", string iconPath = "", int color = 0, bool suppressDefaultChat = false)
	{
		Id = id;
		AuthorUid = authorUid;
		AuthorName = author;
		Position = position;
		TimeLeft = timeLeft;
		Label = label;
		IconPath = iconPath;
		Color = color;
		SuppressDefaultChat = suppressDefaultChat;
	}

	string GetHudLabel()
	{
		if (Label != "")
			return Label;
		return AuthorName;
	}

	string GetMapLabel()
	{
		if (Label != "")
			return Label;
		return "#STR_SMP_00641" + AuthorName;
	}

	string GetIconPath()
	{
		string icon = IconPath;
		icon = icon.Trim();
		if (icon != "")
			return icon;
		return SM_ClanMarkerIcon.GetPath();
	}

	int GetColor(int fallback)
	{
		if (Color != 0)
			return Color;
		return fallback;
	}
}

class SM_ChatPlayerStyle
{
	string SteamId;
	string Prefix;
	ref SM_RGBColorConfig NameColorRGB = new SM_RGBColorConfig;
	ref SM_RGBColorConfig TextColorRGB = new SM_RGBColorConfig;
	ref SM_RGBColorConfig PrefixColorRGB = new SM_RGBColorConfig;   // если пусто — берётся цвет ника

	void SM_ChatPlayerStyle(string steamId = "", string prefix = "", string nameColor = "", string textColor = "", string prefixColor = "")
	{
		SteamId = steamId;
		Prefix = prefix;
		if (!NameColorRGB)
			NameColorRGB = new SM_RGBColorConfig;
		if (!TextColorRGB)
			TextColorRGB = new SM_RGBColorConfig;
		if (!PrefixColorRGB)
			PrefixColorRGB = new SM_RGBColorConfig;
		if (nameColor != "")
			NameColorRGB.FromHex(nameColor, 0);
		if (textColor != "")
			TextColorRGB.FromHex(textColor, 0);
		if (prefixColor != "")
			PrefixColorRGB.FromHex(prefixColor, 0);
	}

	int GetNameColor(int fallback)
	{
		if (NameColorRGB && NameColorRGB.IsConfigured())
			return NameColorRGB.ToARGB(fallback);
		return fallback;
	}

	int GetTextColor(int fallback)
	{
		if (TextColorRGB && TextColorRGB.IsConfigured())
			return TextColorRGB.ToARGB(fallback);
		return fallback;
	}

	int GetPrefixColor(int fallback)
	{
		if (PrefixColorRGB && PrefixColorRGB.IsConfigured())
			return PrefixColorRGB.ToARGB(fallback);
		return fallback;
	}
}

class SM_ChatHistoryEntry
{
	int Channel;
	string Label;
	string Prefix;
	string AuthorName;
	string Text;
	string Stamp;
	int Color;
	int ChannelColor;
	int NameColor;
	int TextColor;
	int PrefixColor;

	void SM_ChatHistoryEntry(int channel = 0, string label = "", string prefix = "", string author = "", string text = "", string stamp = "", int channelColor = 0, int nameColor = 0, int textColor = 0, int prefixColor = 0)
	{
		Channel = channel;
		Label = label;
		Prefix = prefix;
		AuthorName = author;
		Text = text;
		Stamp = stamp;
		ChannelColor = channelColor;
		NameColor = nameColor;
		TextColor = textColor;
		PrefixColor = prefixColor;
		Color = channelColor;
	}

}

class SM_ChatMuteRecord
{
	string Uid;
	string Name;
	string AdminUid;
	string AdminName;
	string Reason;
	int MutedAt;
	int Until;

	void SM_ChatMuteRecord(string uid = "", string name = "", string adminUid = "", string adminName = "", string reason = "", int mutedAt = 0, int until = 0)
	{
		Uid = uid;
		Name = name;
		AdminUid = adminUid;
		AdminName = adminName;
		Reason = reason;
		MutedAt = mutedAt;
		Until = until;
	}
}

class SM_ChatMutesDB
{
	ref array<ref SM_ChatMuteRecord> Mutes = new array<ref SM_ChatMuteRecord>;
}

class SM_MarketItem
{
	string ClassName;
	float Health = 1;
	float Quantity = -1;
	int LiquidType = -1;
	int AmmoCount = -1;
	ref array<ref SM_MarketItem> Attachments = new array<ref SM_MarketItem>;
	ref array<ref SM_MarketItem> Cargo = new array<ref SM_MarketItem>;

	int CountNested()
	{
		int total = Attachments.Count() + Cargo.Count();
		foreach (SM_MarketItem att : Attachments)
			total += att.CountNested();
		foreach (SM_MarketItem cargoItem : Cargo)
			total += cargoItem.CountNested();
		return total;
	}
}

class SM_MarketLot
{
	int Id;
	string ClanName;
	string SellerUid;
	string SellerName;
	int Price;
	string Date;
	int CreatedAtSeconds;
	int ReturnAvailableAtSeconds;
	ref SM_MarketItem Item;
}

class SM_MarketDB
{
	int NextLotId = 1;
	ref array<ref SM_MarketLot> Lots = new array<ref SM_MarketLot>;
	ref array<ref SM_MarketLot> PendingReturns = new array<ref SM_MarketLot>;
}

class SM_MarketIndex
{
	int NextLotId = 1;
	ref array<int> Lots = new array<int>;
}

class SM_MarketPendingReturns
{
	ref array<ref SM_MarketLot> PendingReturns = new array<ref SM_MarketLot>;
}

class SM_MarketLotView
{
	int Id;
	string ClassName;
	float Quantity;
	int AttachCount;
	float Health;
	int Price;
	string ClanName;
	string SellerUid;
	string SellerName;
	string Date;
	ref array<string> AttachClasses = new array<string>;

	void SM_MarketLotView(int id = 0, string className = "", float quantity = -1, int attachCount = 0, float health = 1, int price = 0, string clanName = "", string sellerUid = "", string sellerName = "", string date = "")
	{
		Id = id;
		ClassName = className;
		Quantity = quantity;
		AttachCount = attachCount;
		Health = health;
		Price = price;
		ClanName = clanName;
		SellerUid = sellerUid;
		SellerName = sellerName;
		Date = date;
	}

	string GetDisplayName()
	{
		return SM_PartyUtil.GetItemDisplayName(ClassName);
	}
}

class SM_ServerMarketPurchase
{
	string Uid;
	int Bought;

	void SM_ServerMarketPurchase(string uid = "", int bought = 0)
	{
		Uid = uid;
		Bought = bought;
	}
}

class SM_ServerMarketEntry
{
	int Id;
	string ClassName;
	int BasePrice;
	int InitialQuantity;
	int Quantity;
	int MaxBuyPerPlayer;
	string ReservedUid;
	float ReservedUntil;
	ref array<ref SM_ServerMarketPurchase> Purchases = new array<ref SM_ServerMarketPurchase>;
}

class SM_ServerMarketState
{
	int RotationId = 1;
	int NextItemId = 1;
	int NextRefreshStampMinutes;
	int NextRefreshStampSeconds;
	ref array<ref SM_ServerMarketEntry> Items = new array<ref SM_ServerMarketEntry>;
}

class SM_ServerMarketItemView
{
	int Id;
	bool Locked;
	int RequiredClanLevel;
	string ClassName;
	int BasePrice;
	int CurrentPrice;
	int InitialQuantity;
	int Quantity;
	int MaxBuyPerPlayer;
	int BoughtByPlayer;

	void SM_ServerMarketItemView(int id = 0, bool locked = false, int requiredClanLevel = 0)
	{
		Id = id;
		Locked = locked;
		RequiredClanLevel = requiredClanLevel;
		ClassName = "";
		BasePrice = 0;
		CurrentPrice = 0;
		InitialQuantity = 0;
		Quantity = 0;
		MaxBuyPerPlayer = 0;
		BoughtByPlayer = 0;
	}

	string GetDisplayName()
	{
		if (Locked)
			return "#STR_SMP_00440";
		return SM_PartyUtil.GetItemDisplayName(ClassName);
	}

	bool IsSoldOut()
	{
		if (Quantity <= 0)
			return true;
		return false;
	}

	int GetLimitLeft()
	{
		if (MaxBuyPerPlayer <= 0)
			return 999999;
		int left = MaxBuyPerPlayer - BoughtByPlayer;
		if (left < 0)
			left = 0;
		return left;
	}
}

class SM_StorageItem
{
	int Id;
	string ClanName;
	string DepositorUid;
	string DepositorName;
	string Date;
	ref SM_MarketItem Item;
}

class SM_StorageDB
{
	int NextItemId = 1;
	ref array<ref SM_StorageItem> Items = new array<ref SM_StorageItem>;
	ref array<ref SM_StorageItem> PendingReturns = new array<ref SM_StorageItem>;
}

class SM_StorageClanIndexEntry
{
	string ClanId;
	string ClanName;
	string FileName;

	void SM_StorageClanIndexEntry(string clanId = "", string clanName = "", string file = "")
	{
		ClanId = clanId;
		ClanName = clanName;
		FileName = file;
	}
}

class SM_StorageIndex
{
	int NextItemId = 1;
	ref array<ref SM_StorageClanIndexEntry> Clans = new array<ref SM_StorageClanIndexEntry>;
}

class SM_StorageClanFile
{
	string ClanId;
	string ClanName;
	ref array<ref SM_StorageItem> Items = new array<ref SM_StorageItem>;
}

class SM_StoragePendingReturns
{
	ref array<ref SM_StorageItem> PendingReturns = new array<ref SM_StorageItem>;
}

class SM_StorageItemView
{
	int Id;
	string ClassName;
	float Quantity;
	int AttachCount;
	float Health;
	string DepositorName;
	string Date;
	ref array<string> AttachClasses = new array<string>;

	void SM_StorageItemView(int id = 0, string className = "", float quantity = -1, int attachCount = 0, float health = 1, string depositorName = "", string date = "")
	{
		Id = id;
		ClassName = className;
		Quantity = quantity;
		AttachCount = attachCount;
		Health = health;
		DepositorName = depositorName;
		Date = date;
	}

	string GetDisplayName()
	{
		return SM_PartyUtil.GetItemDisplayName(ClassName);
	}
}

class SM_AchievementStats
{
	int PlayerKills;
	int PlayerDeaths;
	int ZombieKills;
	int OnlineSeconds;
	int BestLifeSeconds;
	int DistanceWalked;
	int ShotsFired;
	int HitsLanded;
	int Headshots;
	int LongestKillDistance;
	int TreasuryDeposited;
	int MarketBuys;
	int MarketSells;
	int StorageDeposits;
	int AuctionWins;
	int AuctionSales;
	int CreatedClan;
	int JoinedClan;
}

class SM_PlayerAchievementRecord
{
	string Uid;
	string Name;
	ref SM_AchievementStats Stats = new SM_AchievementStats;
	ref array<string> ClaimedAchievements = new array<string>;
	ref array<string> NotifiedAchievements = new array<string>;

	void SM_PlayerAchievementRecord(string uid = "", string name = "")
	{
		Uid = uid;
		Name = name;
	}
}

class SM_AchievementsDB
{
	ref array<ref SM_PlayerAchievementRecord> Players = new array<ref SM_PlayerAchievementRecord>;
}

class SM_AchievementView
{
	int Scope;
	string Id;
	string Name;
	string Description;
	string Type;
	int Target;
	int Progress;
	bool Completed;
	bool Claimed;
	bool Hidden;
	int RewardMoney;
	int RewardClanTreasury;
	string RewardContainerClass;

	void SM_AchievementView(int scope = 0, string id = "", string name = "", string description = "", string type = "", int target = 1, int progress = 0, bool completed = false, bool claimed = false, bool hidden = false, int rewardMoney = 0, int rewardClanTreasury = 0, string rewardContainerClass = "")
	{
		Scope = scope;
		Id = id;
		Name = name;
		Description = description;
		Type = type;
		Target = target;
		Progress = progress;
		Completed = completed;
		Claimed = claimed;
		Hidden = hidden;
		RewardMoney = rewardMoney;
		RewardClanTreasury = rewardClanTreasury;
		RewardContainerClass = rewardContainerClass;
	}
}

class SM_AuctionLot
{
	int Id;
	string SellerUid;
	string SellerName;
	string SellerClanName;
	int StartPrice;
	int BidStep;
	int CurrentBid;
	string CurrentBidderUid;
	string CurrentBidderName;
	int EndStampSeconds;
	string Date;
	int CreatedAtSeconds;
	ref SM_MarketItem Item;
}

class SM_AuctionPendingItem
{
	string OwnerUid;
	string OwnerName;
	string Reason;
	int AvailableAtSeconds;
	ref SM_MarketItem Item;
}

class SM_AuctionPendingMoney
{
	string OwnerUid;
	string OwnerName;
	int Amount;
	string Reason;

	void SM_AuctionPendingMoney(string ownerUid = "", string ownerName = "", int amount = 0, string reason = "")
	{
		OwnerUid = ownerUid;
		OwnerName = ownerName;
		Amount = amount;
		Reason = reason;
	}
}

class SM_AuctionState
{
	int NextLotId = 1;
	ref array<ref SM_AuctionLot> Lots = new array<ref SM_AuctionLot>;
	ref array<ref SM_AuctionPendingItem> PendingItems = new array<ref SM_AuctionPendingItem>;
	ref array<ref SM_AuctionPendingMoney> PendingMoney = new array<ref SM_AuctionPendingMoney>;
}

class SM_AuctionLotView
{
	int Id;
	string ClassName;
	float Quantity;
	int AttachCount;
	float Health;
	string SellerUid;
	string SellerName;
	string SellerClanName;
	int StartPrice;
	int BidStep;
	int CurrentBid;
	string CurrentBidderName;
	int SecondsLeft;
	string Date;
	ref array<string> AttachClasses = new array<string>;

	void SM_AuctionLotView(int id = 0, string className = "", float quantity = -1, int attachCount = 0, float health = 1, string sellerUid = "", string sellerName = "", string sellerClanName = "", int startPrice = 0, int bidStep = 1, int currentBid = 0, string currentBidderName = "", int secondsLeft = 0, string date = "")
	{
		Id = id;
		ClassName = className;
		Quantity = quantity;
		AttachCount = attachCount;
		Health = health;
		SellerUid = sellerUid;
		SellerName = sellerName;
		SellerClanName = sellerClanName;
		StartPrice = startPrice;
		BidStep = bidStep;
		CurrentBid = currentBid;
		CurrentBidderName = currentBidderName;
		SecondsLeft = secondsLeft;
		Date = date;
	}

	string GetDisplayName()
	{
		return SM_PartyUtil.GetItemDisplayName(ClassName);
	}

	int GetNextBid()
	{
		if (CurrentBid <= 0)
			return StartPrice;
		return CurrentBid + BidStep;
	}
}

class SM_Contract
{
	int Id;
	int Type;
	int Status;
	string CreatorUid;
	string CreatorName;
	string CreatorClanName;
	string TargetUid;
	string TargetName;
	string TargetClanName;
	string ItemClassName;
	string ItemDisplayName;
	int ItemQuantity;
	int Price;
	int Reward;
	int Fee;
	int Penalty;
	string ExecutorUid;
	string ExecutorName;
	string ExecutorClanName;
	int CreatedAt;
	int AcceptedAt;
	int EndsAt;
	int CompletedAt;
	vector LastTargetPos;
	int LastMarkerAt;
	bool TargetOnline;
	string Date;
	string AcceptedDate;
	string CompletedDate;
}

class SM_ContractPendingMoney
{
	string OwnerUid;
	string OwnerName;
	int Amount;
	string Reason;

	void SM_ContractPendingMoney(string ownerUid = "", string ownerName = "", int amount = 0, string reason = "")
	{
		OwnerUid = ownerUid;
		OwnerName = ownerName;
		Amount = amount;
		Reason = reason;
	}
}

class SM_ContractPendingItem
{
	string OwnerUid;
	string OwnerName;
	string Reason;
	ref SM_MarketItem Item;
}

class SM_ContractsDB
{
	int NextContractId = 1;
	ref array<ref SM_Contract> Contracts = new array<ref SM_Contract>;
	ref array<ref SM_ContractPendingMoney> PendingMoney = new array<ref SM_ContractPendingMoney>;
	ref array<ref SM_ContractPendingItem> PendingItems = new array<ref SM_ContractPendingItem>;
}

class SM_ContractView
{
	int Id;
	int Type;
	int Status;
	string CreatorName;
	string CreatorClanName;
	string TargetName;
	string TargetClanName;
	string ItemClassName;
	string ItemDisplayName;
	int ItemQuantity;
	int Price;
	int Reward;
	int Fee;
	int Penalty;
	string ExecutorName;
	string ExecutorClanName;
	int SecondsLeft;
	string Date;
	bool Mine;
	bool IsCreator;
	bool IsExecutor;

	void SM_ContractView(int id = 0, int type = 0, int status = 0)
	{
		Id = id;
		Type = type;
		Status = status;
	}

	string GetSubject()
	{
		if (Type == SM_ContractType.KILL)
			return TargetName;
		if (ItemDisplayName != "")
			return ItemDisplayName + " x" + ItemQuantity.ToString();
		return ItemClassName + " x" + ItemQuantity.ToString();
	}
}

class SM_ContractTargetView
{
	string Uid;
	string Name;
	string ClanName;
	string Title;
	bool Online;

	void SM_ContractTargetView(string uid = "", string name = "", string clanName = "", string title = "", bool online = true)
	{
		Uid = uid;
		Name = name;
		ClanName = clanName;
		Title = title;
		Online = online;
	}
}

class SM_ContractItemOptionView
{
	string ClassName;
	string DisplayName;

	void SM_ContractItemOptionView(string className = "", string displayName = "")
	{
		ClassName = className;
		DisplayName = displayName;
	}
}

class SM_ContractMarkerView
{
	int ContractId;
	int Type;
	string Label;
	vector Position;
	int SecondsLeft;

	void SM_ContractMarkerView(int contractId = 0, int type = 0, string label = "", vector position = "0 0 0", int secondsLeft = 0)
	{
		ContractId = contractId;
		Type = type;
		Label = label;
		Position = position;
		SecondsLeft = secondsLeft;
	}
}
