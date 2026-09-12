class SM_PartyGeneralConfig
{
	int MaxClans = 0;
	int MaxClanMembers = 10;
	int MinClanNameLength = 3;
	int MaxClanNameLength = 24;
	int MaxClanTagLength = 6;
	int MaxClanDescriptionLength = 200;
	string CreateClanLogoPaa = "SM_PartyMod\\GUI\\logo\\logo.paa";
	int MaxApplicationsPerClan = 20;
	int InviteTimeoutSeconds = 180;
	bool ShowClanListToEveryone = true;
}

class SM_PartyRanksConfig
{
	ref array<string> RankNames = {"#STR_SMP_00710", "#STR_SMP_00280", "#STR_SMP_00761", "#STR_SMP_00599"};
	ref array<string> AdminUids = {"REPLACE_WITH_STEAMID64"};
	int MinRankToInvite = 2;
	int MinRankToKick = 2;
}

class SM_PartyHudConfig
{
	bool Enabled = true;
	int UpdateIntervalSeconds = 2;
}

class SM_RGBColorConfig
{
	int R = -1;
	int G = -1;
	int B = -1;

	void SM_RGBColorConfig(int r = -1, int g = -1, int b = -1)
	{
		R = r;
		G = g;
		B = b;
	}

	bool IsConfigured()
	{
		if (R < 0 || G < 0 || B < 0)
			return false;
		return true;
	}

	void SetRGB(int r, int g, int b)
	{
		R = r;
		G = g;
		B = b;
		Clamp();
	}

	void Clamp()
	{
		R = SM_ClanColors.ClampChannel(R);
		G = SM_ClanColors.ClampChannel(G);
		B = SM_ClanColors.ClampChannel(B);
	}

	int ToARGB(int fallback = 0)
	{
		if (!IsConfigured())
			return fallback;

		Clamp();
		return SM_ClanColors.FromRGB(R, G, B);
	}

	void FromARGB(int color)
	{
		R = SM_ClanColors.GetRed(color);
		G = SM_ClanColors.GetGreen(color);
		B = SM_ClanColors.GetBlue(color);
		Clamp();
	}

	void FromHex(string value, int fallback)
	{
		int parsed = SM_PartyUtil.ParseChatColor(value, fallback);
		FromARGB(parsed);
	}
}

class SM_PartyChatChannelConfig
{
	ref array<ref SM_ChatPlayerStyle> PlayerStyles = new array<ref SM_ChatPlayerStyle>;
}

class SM_PartyChatConfig
{
	bool Enabled = true;
	int MessageMaxLength = 1024;
	bool LocalEnabled = true;
	bool GlobalEnabled = true;
	int LocalRangeMeters = 75;
	ref SM_PartyChatChannelConfig Clan = new SM_PartyChatChannelConfig;
	ref SM_PartyChatChannelConfig Local = new SM_PartyChatChannelConfig;
	ref SM_PartyChatChannelConfig Global = new SM_PartyChatChannelConfig;
	ref SM_RGBColorConfig ColorClan = new SM_RGBColorConfig;
	ref SM_RGBColorConfig ColorClanPlayer = new SM_RGBColorConfig;
	ref SM_RGBColorConfig ColorDirect = new SM_RGBColorConfig;
	ref SM_RGBColorConfig ColorDirectPlayer = new SM_RGBColorConfig;
	ref SM_RGBColorConfig ColorGlobal = new SM_RGBColorConfig;
	ref SM_RGBColorConfig ColorGlobalPlayer = new SM_RGBColorConfig;
	ref SM_RGBColorConfig ColorServer = new SM_RGBColorConfig;
	ref SM_RGBColorConfig ColorAlert = new SM_RGBColorConfig;

	int GetClanColor()
	{
		if (!ColorClan)
			return ARGB(255, 224, 224, 224);
		return ColorClan.ToARGB(ARGB(255, 224, 224, 224));
	}

	int GetClanPlayerColor()
	{
		if (!ColorClanPlayer)
			return ARGB(255, 100, 180, 255);
		return ColorClanPlayer.ToARGB(ARGB(255, 100, 180, 255));
	}

	int GetDirectColor()
	{
		if (!ColorDirect)
			return ARGB(255, 255, 255, 255);
		return ColorDirect.ToARGB(ARGB(255, 255, 255, 255));
	}

	int GetDirectPlayerColor()
	{
		if (!ColorDirectPlayer)
			return ARGB(255, 255, 255, 255);
		return ColorDirectPlayer.ToARGB(ARGB(255, 255, 255, 255));
	}

	int GetGlobalColor()
	{
		if (!ColorGlobal)
			return ARGB(255, 255, 255, 255);
		return ColorGlobal.ToARGB(ARGB(255, 255, 255, 255));
	}

	int GetGlobalPlayerColor()
	{
		if (!ColorGlobalPlayer)
			return ARGB(255, 232, 181, 69);
		return ColorGlobalPlayer.ToARGB(ARGB(255, 232, 181, 69));
	}

	int GetServerColor()
	{
		if (!ColorServer)
			return ARGB(255, 255, 50, 50);
		return ColorServer.ToARGB(ARGB(255, 255, 50, 50));
	}

	int GetAlertColor()
	{
		if (!ColorAlert)
			return ARGB(255, 100, 200, 255);
		return ColorAlert.ToARGB(ARGB(255, 100, 200, 255));
	}
}

class SM_PartyDiscordConfig
{
	string WebhookUrl = "https://discord.com/api/webhooks/000000000000000000/PASTE_WEBHOOK_TOKEN_HERE";
	string ClanWebhookUrl = "https://discord.com/api/webhooks/000000000000000000/PASTE_CLAN_CHAT_TOKEN_HERE";
	string LocalWebhookUrl = "https://discord.com/api/webhooks/000000000000000000/PASTE_LOCAL_CHAT_TOKEN_HERE";
	string GlobalWebhookUrl = "https://discord.com/api/webhooks/000000000000000000/PASTE_GLOBAL_CHAT_TOKEN_HERE";
	string ServerLabel = "SobrMods Clan System";
	bool RelayClan = false;
	bool RelayLocal = false;
	bool RelayGlobal = false;
}

class SM_PartyBaseRegistrationAuditConfig
{
	bool Enabled = false;
	string WebhookUrl = "";
	bool UseMainWebhookFallback = false;
	bool SendRegister = true;
	bool SendUnregister = true;
	bool SendMemberChanges = true;
	bool SendRankChanges = true;
	bool IncludeMemberList = true;
}

class SM_PartyPingConfig
{
	bool Enabled = true;
	int DurationSeconds = 30;
	float CooldownSeconds = 0.7;
}

class SM_PartyMapConfig
{
	bool Enabled = true;
	int MaxMarkersPerClan = 30;
	int MarkerMaxNameLength = 32;
	bool BaseRadiusEnabled = true;
	int BaseRadiusMeters = 150;
	ref array<ref SM_ServerMapMarker> ServerMarkers = new array<ref SM_ServerMapMarker>;
}

class SM_PartyLogConfig
{
	int MaxEntries = 30;
}

class SM_PartyMarketConfig
{
	bool Enabled = true;
	int MaxLotsPerClan = 10;
	int FeePercent = 5;
	int ListingFeePercent = 5;
	int MinCancelAgeMinutes = 60;
	int CancelPenaltyPercent = 10;
	int ReturnDelaySeconds = 900;
	int MinRankToSell = 2;
	int MaxPrice = 1000000;
	ref array<string> Blacklist = {"REPLACE_WITH_BLOCKED_ITEM_CLASSNAME"};
	ref SM_PartyTradeZoneRestrictionConfig TradeZones = new SM_PartyTradeZoneRestrictionConfig;
}

class SM_ServerMarketItemConfig
{
	string ClassName;
	int MinQuantity = 1;
	int MaxQuantity = 1;
	int MinPrice = 100;
	int MaxPrice = 100;
	float SpawnChance = 1.0;
	int MaxBuyPerPlayer = 1;

	void SM_ServerMarketItemConfig(string className = "", int minQuantity = 1, int maxQuantity = 1, int minPrice = 100, int maxPrice = 100, float spawnChance = 1.0, int maxBuyPerPlayer = 1)
	{
		ClassName = className;
		MinQuantity = minQuantity;
		MaxQuantity = maxQuantity;
		MinPrice = minPrice;
		MaxPrice = maxPrice;
		SpawnChance = spawnChance;
		MaxBuyPerPlayer = maxBuyPerPlayer;
	}
}

class SM_ServerMarketPriceStep
{
	int StockPercentBelow = 50;
	float PriceMultiplier = 1.10;

	void SM_ServerMarketPriceStep(int stockPercentBelow = 50, float priceMultiplier = 1.10)
	{
		StockPercentBelow = stockPercentBelow;
		PriceMultiplier = priceMultiplier;
	}
}

class SM_PartyServerMarketConfig
{
	bool Enabled = true;
	bool SaveState = true;
	bool RefreshOnServerStart = false;
	bool EnableDynamicPrice = true;
	int RefreshIntervalSeconds = 3600;
	int GeneratedItemsCount = 10;
	int DefaultVisibleItems = 3;
	int PurchaseReservationSeconds = 5;
	ref array<ref SM_ServerMarketPriceStep> DynamicPriceSteps = new array<ref SM_ServerMarketPriceStep>;
	ref array<ref SM_ServerMarketItemConfig> Items = new array<ref SM_ServerMarketItemConfig>;
}

class SM_TopCategoryConfig
{
	string Category;
	bool Enabled = true;

	void SM_TopCategoryConfig(string category = "", bool enabled = true)
	{
		Category = category;
		Enabled = enabled;
	}
}

class SM_TopRewardLootItemConfig
{
	string ClassName;
	int MinQuantity = 1;
	int MaxQuantity = 1;
	float SpawnChance = 1.0;

	void SM_TopRewardLootItemConfig(string className = "", int minQuantity = 1, int maxQuantity = 1, float spawnChance = 1.0)
	{
		ClassName = className;
		MinQuantity = minQuantity;
		MaxQuantity = maxQuantity;
		SpawnChance = spawnChance;
	}
}

class SM_TopRewardContainerConfig
{
	string ClassName;
	int MinQuantity = 1;
	int MaxQuantity = 1;
	float SpawnChance = 1.0;
	ref array<ref SM_TopRewardLootItemConfig> Loot = new array<ref SM_TopRewardLootItemConfig>;

	void SM_TopRewardContainerConfig(string className = "", int minQuantity = 1, int maxQuantity = 1, float spawnChance = 1.0)
	{
		ClassName = className;
		MinQuantity = minQuantity;
		MaxQuantity = maxQuantity;
		SpawnChance = spawnChance;
	}
}

class SM_PartyTopConfig
{
	bool Enabled = true;
	ref array<ref SM_TopCategoryConfig> Categories = new array<ref SM_TopCategoryConfig>;
	int MinShotsForAccuracyTop = 100;
	bool RewardsEnabled = false;
	string RewardSchedule = "monthly";
	int RewardIntervalHours = 168;
	int RewardTreasuryPoints = 1000;
	bool RewardLootEnabled = false;
	ref array<ref SM_TopRewardContainerConfig> RewardLootContainers = new array<ref SM_TopRewardContainerConfig>;
	bool ResetStatsAfterReward = true;
}

class SM_AchievementConfig
{
	bool Enabled = true;
	string Id;
	string Name;
	string Description;
	string Type;
	int Target = 1;
	bool Hidden = false;
	int RewardMoney = 0;
	int RewardClanTreasury = 0;
	ref SM_TopRewardContainerConfig RewardContainer;

	void SM_AchievementConfig(string id = "", string name = "", string description = "", string type = "", int target = 1)
	{
		Id = id;
		Name = name;
		Description = description;
		Type = type;
		Target = target;
	}
}

class SM_PartyAchievementsConfig
{
	bool Enabled = true;
	bool NotifyOnComplete = true;
	bool ClaimRewardManually = true;
	ref array<ref SM_AchievementConfig> Personal = new array<ref SM_AchievementConfig>;
	ref array<ref SM_AchievementConfig> Clan = new array<ref SM_AchievementConfig>;
}

class SM_PlayerTitleRankConfig
{
	string Name;
	int RequiredXP;
	int Color;
	ref SM_RGBColorConfig ColorRGB = new SM_RGBColorConfig;

	void SM_PlayerTitleRankConfig(string name = "", int requiredXP = 0, int color = 0)
	{
		Name = name;
		RequiredXP = requiredXP;
		Color = color;
		if (!ColorRGB)
			ColorRGB = new SM_RGBColorConfig;
		if (color != 0)
			ColorRGB.FromARGB(color);
	}

	int GetColor(int fallback)
	{
		if (ColorRGB && ColorRGB.IsConfigured())
			return ColorRGB.ToARGB(fallback);
		if (Color != 0)
			return Color;
		return fallback;
	}
}

class SM_PlayerExperienceZombieRewardConfig
{
	string ClassName;
	int XP;
	bool MatchInherited = true;

	void SM_PlayerExperienceZombieRewardConfig(string className = "", int xp = 1, bool matchInherited = true)
	{
		ClassName = className;
		XP = xp;
		MatchInherited = matchInherited;
	}
}

class SM_PlayerExperienceRestrictedItemConfig
{
	string ClassName;
	string RequiredTitle;
	bool MatchInherited = true;

	void SM_PlayerExperienceRestrictedItemConfig(string className = "", string requiredTitle = "", bool matchInherited = true)
	{
		ClassName = className;
		RequiredTitle = requiredTitle;
		MatchInherited = matchInherited;
	}
}

class SM_PlayerExperienceConfig
{
	bool Enabled = true;
	bool ShowNotifications = true;
	bool ShowInClanList = true;
	bool ShowPublicTab = true;
	bool RestrictedItemsEnabled = true;
	int ZombieKillXP = 1;
	int PlayerKillXP = 15;
	int OnlineRewardIntervalSeconds = 600;
	int OnlineRewardXP = 5;
	int MaxPublicPlayers = 200;
	ref array<ref SM_PlayerTitleRankConfig> Ranks = new array<ref SM_PlayerTitleRankConfig>;
	ref array<ref SM_PlayerExperienceZombieRewardConfig> ZombieClassRewards = new array<ref SM_PlayerExperienceZombieRewardConfig>;
	ref array<ref SM_PlayerExperienceRestrictedItemConfig> RestrictedItems = new array<ref SM_PlayerExperienceRestrictedItemConfig>;
}

class SM_ContractItemConfig
{
	string ClassName;
	string DisplayName;
	bool MatchInherited = true;

	void SM_ContractItemConfig(string className = "", string displayName = "", bool matchInherited = true)
	{
		ClassName = className;
		DisplayName = displayName;
		MatchInherited = matchInherited;
	}
}

class SM_PartyContractsConfig
{
	bool Enabled = true;
	string RequiredCreatorTitle = "#STR_SMP_00284";
	int MinPrice = 1000;
	int MaxPrice = 1000000;
	int FeePercent = 10;
	int AbandonPenaltyPercent = 20;
	int MinDurationMinutes = 30;
	int MaxDurationMinutes = 1440;
	int TargetMarkerIntervalSeconds = 120;
	int TickSeconds = 30;
	int MaxActiveCreatedPerPlayer = 3;
	int MaxActiveAcceptedPerPlayer = 1;
	ref array<ref SM_ContractItemConfig> SearchItems = new array<ref SM_ContractItemConfig>;
}

class SM_PartyAuctionConfig
{
	bool Enabled = true;
	bool SaveState = true;
	int MaxActiveLotsPerPlayer = 5;
	int MinDurationMinutes = 30;
	int MaxDurationMinutes = 1440;
	int DefaultDurationMinutes = 360;
	int MinBidStep = 100;
	int ListingFee = 0;
	int ListingFeePercent = 5;
	int SaleTaxPercent = 5;
	int AntiSnipeSeconds = 30;
	int AntiSnipeExtendSeconds = 60;
	int MaxStartPrice = 1000000;
	int MinCancelAgeMinutes = 60;
	int CancelPenaltyPercent = 10;
	int ReturnDelaySeconds = 900;
	bool AllowCancelWithoutBids = true;
	bool ReturnItemIfNoBids = true;
	ref SM_PartyTradeZoneRestrictionConfig TradeZones = new SM_PartyTradeZoneRestrictionConfig;
}

class SM_PartyTreasuryConfig
{
	bool Enabled = true;
	ref array<ref SM_CurrencyItem> CurrencyItems = new array<ref SM_CurrencyItem>;
}

class SM_PartyStorageConfig
{
	bool Enabled = true;
	int RadiusMeters = 30;
}

class SM_PartyLevelConfig
{
	bool Enabled = true;
	ref array<ref SM_ClanLevel> Items = new array<ref SM_ClanLevel>;
}

class SM_PartyLegacyConfig
{
	string ConfigVersion = "1.0";

	int MaxClans = 0;
	int MaxClanMembers = 10;
	int MinClanNameLength = 3;
	int MaxClanNameLength = 24;
	int MaxClanTagLength = 6;
	int MaxClanDescriptionLength = 200;
	string CreateClanLogoPaa = "SM_PartyMod\\GUI\\logo\\logo.paa";
	int MaxApplicationsPerClan = 20;
	int InviteTimeoutSeconds = 180;

	ref array<string> RankNames = {"#STR_SMP_00710", "#STR_SMP_00280", "#STR_SMP_00761", "#STR_SMP_00599"};
	ref array<string> AdminUids = new array<string>;
	int MinRankToInvite = 2;
	int MinRankToKick = 2;
	bool ShowClanListToEveryone = true;

	bool HudEnabled = true;
	int HudUpdateIntervalSeconds = 2;
	int HudMaxVisibleMembers = 8;
	int HudOffsetX = 12;
	int HudOffsetY = 120;
	bool ShowDistanceInHud = true;
	bool ShowDirectionInHud = true;
	bool MarkersEnabled = true;
	int MarkerMaxDistance = 500;

	bool ChatEnabled = true;
	int ChatMessageMaxLength = 1024;
	string ChatClanDefaultNameColor = "#64B4FF";
	string ChatClanDefaultTextColor = "#E0E0E0";
	ref array<ref SM_ChatPlayerStyle> ChatClanPlayerStyles = new array<ref SM_ChatPlayerStyle>;
	string ChatLocalDefaultNameColor = "#FFFFFF";
	string ChatLocalDefaultTextColor = "#FFFFFF";
	ref array<ref SM_ChatPlayerStyle> ChatLocalPlayerStyles = new array<ref SM_ChatPlayerStyle>;
	string ChatGlobalDefaultNameColor = "#64FF64";
	string ChatGlobalDefaultTextColor = "#E0E0E0";
	ref array<ref SM_ChatPlayerStyle> ChatGlobalPlayerStyles = new array<ref SM_ChatPlayerStyle>;
	bool LocalChatEnabled = true;
	bool GlobalChatEnabled = true;
	int LocalChatRangeMeters = 75;

	string DiscordWebhookUrl = "";
	string DiscordClanWebhookUrl = "";
	string DiscordLocalWebhookUrl = "";
	string DiscordGlobalWebhookUrl = "";
	string DiscordServerLabel = "";
	bool DiscordRelayClan = false;
	bool DiscordRelayLocal = false;
	bool DiscordRelayGlobal = false;

	bool PingsEnabled = true;
	int PingDurationSeconds = 30;
	float PingCooldownSeconds = 0.7;
	int MaxMapMarkersPerClan = 30;
	int MapMarkerMaxNameLength = 32;
	int LogMaxEntries = 30;

	bool MarketEnabled = true;
	int MaxLotsPerClan = 10;
	int MarketFeePercent = 5;
	int MinRankToSell = 2;
	int MarketMaxPrice = 1000000;
	ref array<string> MarketBlacklist = new array<string>;

	bool TopsEnabled = true;
	int MinShotsForAccuracyTop = 100;
	bool TopRewardsEnabled = false;
	string TopRewardSchedule = "monthly";
	int TopRewardIntervalHours = 168;
	int TopRewardTreasuryPoints = 1000;
	bool ResetStatsAfterReward = true;

	bool TreasuryEnabled = true;
	ref array<ref SM_CurrencyItem> CurrencyItems = new array<ref SM_CurrencyItem>;

	bool StorageEnabled = true;
	int StorageRadiusMeters = 30;

	bool ClanLevelsEnabled = true;
	ref array<ref SM_ClanLevel> ClanLevels = new array<ref SM_ClanLevel>;
}

class SM_PartyConfig
{
	string ConfigVersion = "1.15";

	ref SM_PartyGeneralConfig General = new SM_PartyGeneralConfig;
	ref SM_PartyRanksConfig Ranks = new SM_PartyRanksConfig;
	ref SM_PartyHudConfig MemberHud = new SM_PartyHudConfig;
	ref SM_PartyChatConfig ChatSystem = new SM_PartyChatConfig;
	ref SM_PartyDiscordConfig Discord = new SM_PartyDiscordConfig;
	ref SM_PartyBaseRegistrationAuditConfig BaseRegistrationAudit = new SM_PartyBaseRegistrationAuditConfig;
	ref SM_PartyPingConfig Pings = new SM_PartyPingConfig;
	ref SM_PartyMapConfig ClanMap = new SM_PartyMapConfig;
	ref SM_PartyLogConfig Logs = new SM_PartyLogConfig;
	ref SM_PartyMarketConfig Market = new SM_PartyMarketConfig;
	ref SM_PartyServerMarketConfig ServerMarket = new SM_PartyServerMarketConfig;
	ref SM_PartyTopConfig Tops = new SM_PartyTopConfig;
	ref SM_PartyAchievementsConfig AchievementSettings = new SM_PartyAchievementsConfig;
	ref SM_PlayerExperienceConfig PlayerExperience = new SM_PlayerExperienceConfig;
	ref SM_PartyContractsConfig Contracts = new SM_PartyContractsConfig;
	ref SM_PartyAuctionConfig Auction = new SM_PartyAuctionConfig;
	ref SM_PartyTreasuryConfig Treasury = new SM_PartyTreasuryConfig;
	ref SM_PartyStorageConfig Storage = new SM_PartyStorageConfig;
	ref SM_PartyLevelConfig Levels = new SM_PartyLevelConfig;

	void EnsureSections()
	{
		if (!General)
			General = new SM_PartyGeneralConfig;
		if (!Ranks)
			Ranks = new SM_PartyRanksConfig;
		if (!MemberHud)
			MemberHud = new SM_PartyHudConfig;
		if (!ChatSystem)
			ChatSystem = new SM_PartyChatConfig;
		if (!Discord)
			Discord = new SM_PartyDiscordConfig;
		if (!BaseRegistrationAudit)
			BaseRegistrationAudit = new SM_PartyBaseRegistrationAuditConfig;
		if (!Pings)
			Pings = new SM_PartyPingConfig;
		if (!ClanMap)
			ClanMap = new SM_PartyMapConfig;
		if (!ClanMap.ServerMarkers)
			ClanMap.ServerMarkers = new array<ref SM_ServerMapMarker>;
		if (!Logs)
			Logs = new SM_PartyLogConfig;
		if (!Market)
			Market = new SM_PartyMarketConfig;
		if (!Market.TradeZones)
			Market.TradeZones = new SM_PartyTradeZoneRestrictionConfig;
		if (!Market.TradeZones.Zones)
			Market.TradeZones.Zones = new array<ref SM_PartyTradeZoneConfig>;
		if (!ServerMarket)
			ServerMarket = new SM_PartyServerMarketConfig;
		if (!Tops)
			Tops = new SM_PartyTopConfig;
		if (!AchievementSettings)
			AchievementSettings = new SM_PartyAchievementsConfig;
		if (!PlayerExperience)
			PlayerExperience = new SM_PlayerExperienceConfig;
		if (!Contracts)
			Contracts = new SM_PartyContractsConfig;
		if (!Auction)
			Auction = new SM_PartyAuctionConfig;
		if (!Auction.TradeZones)
			Auction.TradeZones = new SM_PartyTradeZoneRestrictionConfig;
		if (!Auction.TradeZones.Zones)
			Auction.TradeZones.Zones = new array<ref SM_PartyTradeZoneConfig>;
		if (!Treasury)
			Treasury = new SM_PartyTreasuryConfig;
		if (!Storage)
			Storage = new SM_PartyStorageConfig;
		if (!Levels)
			Levels = new SM_PartyLevelConfig;

		if (!ChatSystem.Clan)
			ChatSystem.Clan = new SM_PartyChatChannelConfig;
		if (!ChatSystem.Local)
			ChatSystem.Local = new SM_PartyChatChannelConfig;
		if (!ChatSystem.Global)
			ChatSystem.Global = new SM_PartyChatChannelConfig;
	}

	void ImportLegacy(SM_PartyLegacyConfig legacy)
	{
		if (!legacy)
			return;

		EnsureSections();
		ConfigVersion = "1.15";

		General.MaxClans = legacy.MaxClans;
		General.MaxClanMembers = legacy.MaxClanMembers;
		General.MinClanNameLength = legacy.MinClanNameLength;
		General.MaxClanNameLength = legacy.MaxClanNameLength;
		General.MaxClanTagLength = legacy.MaxClanTagLength;
		General.MaxClanDescriptionLength = legacy.MaxClanDescriptionLength;
		General.CreateClanLogoPaa = legacy.CreateClanLogoPaa;
		General.MaxApplicationsPerClan = legacy.MaxApplicationsPerClan;
		General.InviteTimeoutSeconds = legacy.InviteTimeoutSeconds;
		General.ShowClanListToEveryone = legacy.ShowClanListToEveryone;

		if (legacy.RankNames)
			Ranks.RankNames = legacy.RankNames;
		if (legacy.AdminUids)
			Ranks.AdminUids = legacy.AdminUids;
		Ranks.MinRankToInvite = legacy.MinRankToInvite;
		Ranks.MinRankToKick = legacy.MinRankToKick;

		MemberHud.Enabled = legacy.HudEnabled;
		MemberHud.UpdateIntervalSeconds = legacy.HudUpdateIntervalSeconds;

		ChatSystem.Enabled = legacy.ChatEnabled;
		ChatSystem.MessageMaxLength = legacy.ChatMessageMaxLength;
		ChatSystem.LocalEnabled = legacy.LocalChatEnabled;
		ChatSystem.GlobalEnabled = legacy.GlobalChatEnabled;
		ChatSystem.LocalRangeMeters = legacy.LocalChatRangeMeters;
		ChatSystem.ColorClan.FromHex(legacy.ChatClanDefaultTextColor, ARGB(255, 224, 224, 224));
		ChatSystem.ColorClanPlayer.FromHex(legacy.ChatClanDefaultNameColor, ARGB(255, 100, 180, 255));
		ChatSystem.ColorDirect.FromHex(legacy.ChatLocalDefaultTextColor, ARGB(255, 255, 255, 255));
		ChatSystem.ColorDirectPlayer.FromHex(legacy.ChatLocalDefaultNameColor, ARGB(255, 255, 255, 255));
		ChatSystem.ColorGlobal.FromHex(legacy.ChatGlobalDefaultTextColor, ARGB(255, 255, 255, 255));
		ChatSystem.ColorGlobalPlayer.FromHex(legacy.ChatGlobalDefaultNameColor, ARGB(255, 232, 181, 69));
		ChatSystem.ColorServer.SetRGB(255, 50, 50);
		ChatSystem.ColorAlert.SetRGB(100, 200, 255);
		if (legacy.ChatClanPlayerStyles)
			ChatSystem.Clan.PlayerStyles = legacy.ChatClanPlayerStyles;
		if (legacy.ChatLocalPlayerStyles)
			ChatSystem.Local.PlayerStyles = legacy.ChatLocalPlayerStyles;
		if (legacy.ChatGlobalPlayerStyles)
			ChatSystem.Global.PlayerStyles = legacy.ChatGlobalPlayerStyles;

		Discord.WebhookUrl = legacy.DiscordWebhookUrl;
		Discord.ClanWebhookUrl = legacy.DiscordClanWebhookUrl;
		Discord.LocalWebhookUrl = legacy.DiscordLocalWebhookUrl;
		Discord.GlobalWebhookUrl = legacy.DiscordGlobalWebhookUrl;
		Discord.ServerLabel = legacy.DiscordServerLabel;
		Discord.RelayClan = legacy.DiscordRelayClan;
		Discord.RelayLocal = legacy.DiscordRelayLocal;
		Discord.RelayGlobal = legacy.DiscordRelayGlobal;

		Pings.Enabled = legacy.PingsEnabled;
		Pings.DurationSeconds = legacy.PingDurationSeconds;
		Pings.CooldownSeconds = legacy.PingCooldownSeconds;
		ClanMap.MaxMarkersPerClan = legacy.MaxMapMarkersPerClan;
		ClanMap.MarkerMaxNameLength = legacy.MapMarkerMaxNameLength;
		Logs.MaxEntries = legacy.LogMaxEntries;

		Market.Enabled = legacy.MarketEnabled;
		Market.MaxLotsPerClan = legacy.MaxLotsPerClan;
		Market.FeePercent = legacy.MarketFeePercent;
		Market.MinRankToSell = legacy.MinRankToSell;
		Market.MaxPrice = legacy.MarketMaxPrice;
		if (legacy.MarketBlacklist)
			Market.Blacklist = legacy.MarketBlacklist;

		Tops.Enabled = legacy.TopsEnabled;
		Tops.MinShotsForAccuracyTop = legacy.MinShotsForAccuracyTop;
		Tops.RewardsEnabled = legacy.TopRewardsEnabled;
		Tops.RewardSchedule = legacy.TopRewardSchedule;
		Tops.RewardIntervalHours = legacy.TopRewardIntervalHours;
		Tops.RewardTreasuryPoints = legacy.TopRewardTreasuryPoints;
		Tops.ResetStatsAfterReward = legacy.ResetStatsAfterReward;

		Treasury.Enabled = legacy.TreasuryEnabled;
		if (legacy.CurrencyItems)
			Treasury.CurrencyItems = legacy.CurrencyItems;

		Storage.Enabled = legacy.StorageEnabled;
		Storage.RadiusMeters = legacy.StorageRadiusMeters;

		Levels.Enabled = legacy.ClanLevelsEnabled;
		if (legacy.ClanLevels)
			Levels.Items = legacy.ClanLevels;
	}

	int GetMaxLevel()
	{
		if (!Levels || !Levels.Items || Levels.Items.Count() < 1)
			return 1;
		return Levels.Items.Count();
	}

	SM_ClanLevel GetLevelData(int level)
	{
		int maxLevel = GetMaxLevel();
		if (level < 1)
			level = 1;
		if (level > maxLevel)
			level = maxLevel;
		return Levels.Items[level - 1];
	}

	int GetStorageSlots(int level)
	{
		SM_ClanLevel data = GetLevelData(level);
		if (data)
			return data.StorageSlots;
		return 0;
	}

	int GetMarketLots(int level)
	{
		if (!Levels.Enabled)
			return Market.MaxLotsPerClan;
		SM_ClanLevel data = GetLevelData(level);
		if (data)
			return data.MarketLots;
		return Market.MaxLotsPerClan;
	}

	int GetServerMarketItems(int level)
	{
		if (!Levels.Enabled)
			return ServerMarket.DefaultVisibleItems;
		SM_ClanLevel data = GetLevelData(level);
		if (data)
			return data.ServerMarketItems;
		return ServerMarket.DefaultVisibleItems;
	}

	bool IsTopCategoryEnabled(int category)
	{
		if (!Tops || !Tops.Categories)
			return true;

		string categoryName = SM_TopCategory.GetConfigName(category);
		foreach (SM_TopCategoryConfig topCategory : Tops.Categories)
		{
			if (!topCategory)
				continue;
			if (topCategory.Category == categoryName)
				return topCategory.Enabled;
		}
		return true;
	}

	int GetUpgradeCost(int level)
	{
		if (level >= GetMaxLevel())
			return -1;
		SM_ClanLevel next = GetLevelData(level + 1);
		if (!next)
			return -1;
		return next.UpgradeCost;
	}

	void GetDefaultPerms(out array<int> perms)
	{
		perms = new array<int>;
		for (int i = 0; i < SM_ClanAction.COUNT; i++)
			perms.Insert(0);

		int leader = GetLeaderRank();
		perms.Set(SM_ClanAction.INVITE, Ranks.MinRankToInvite);
		perms.Set(SM_ClanAction.KICK, Ranks.MinRankToKick);
		perms.Set(SM_ClanAction.APPLICATIONS, Ranks.MinRankToInvite);
		perms.Set(SM_ClanAction.MARKET_SELL, Market.MinRankToSell);
		perms.Set(SM_ClanAction.TREASURY_WITHDRAW, leader);
		perms.Set(SM_ClanAction.STORAGE_TAKE, 0);
		perms.Set(SM_ClanAction.MAP_MARKERS, 0);
		perms.Set(SM_ClanAction.DESCRIPTION, leader);
		perms.Set(SM_ClanAction.COLOR, leader);
		perms.Set(SM_ClanAction.UPGRADE, leader);
		perms.Set(SM_ClanAction.SET_BASE, leader);
		perms.Set(SM_ClanAction.MANAGE_WEBHOOK, leader);
	}

	void GetValidCurrency(out array<ref SM_CurrencyItem> result)
	{
		result = new array<ref SM_CurrencyItem>;
		if (!Treasury || !Treasury.CurrencyItems)
			return;
		foreach (SM_CurrencyItem currency : Treasury.CurrencyItems)
		{
			if (currency && currency.Classname != "" && currency.Value > 0)
				result.Insert(currency);
		}
	}

	int GetLeaderRank()
	{
		if (!Ranks || !Ranks.RankNames || Ranks.RankNames.Count() == 0)
			return 0;
		return Ranks.RankNames.Count() - 1;
	}

	string GetRankName(int rank)
	{
		if (!Ranks || !Ranks.RankNames || rank < 0 || rank >= Ranks.RankNames.Count())
			return "?";
		return Ranks.RankNames[rank];
	}

	void Validate()
	{
		EnsureSections();
		ConfigVersion = "1.15";

		if (!Ranks.RankNames || Ranks.RankNames.Count() < 2)
		{
			Ranks.RankNames = new array<string>;
			Ranks.RankNames.Insert("#STR_SMP_00280");
			Ranks.RankNames.Insert("#STR_SMP_00599");
		}
		if (!Ranks.AdminUids)
			Ranks.AdminUids = new array<string>;
		for (int au = Ranks.AdminUids.Count() - 1; au >= 0; au--)
		{
			Ranks.AdminUids.Set(au, Ranks.AdminUids[au].Trim());
			if (Ranks.AdminUids[au] == "")
				Ranks.AdminUids.Remove(au);
		}
		if (Ranks.AdminUids.Count() == 0)
			Ranks.AdminUids.Insert("REPLACE_WITH_STEAMID64");

		if (General.MaxClanMembers < 2)
			General.MaxClanMembers = 2;
		if (General.MinClanNameLength < 1)
			General.MinClanNameLength = 1;
		if (General.MaxClanNameLength < General.MinClanNameLength)
			General.MaxClanNameLength = General.MinClanNameLength;
		if (General.MaxClanTagLength < 1)
			General.MaxClanTagLength = 1;
		if (General.MaxClanDescriptionLength < 20)
			General.MaxClanDescriptionLength = 20;
		General.CreateClanLogoPaa = General.CreateClanLogoPaa.Trim();
		if (General.CreateClanLogoPaa == "")
			General.CreateClanLogoPaa = "SM_PartyMod\\GUI\\logo\\logo.paa";
		if (General.CreateClanLogoPaa == "SM_PartyMod\\GUI\\pings\\ping.paa")
			General.CreateClanLogoPaa = "SM_PartyMod\\GUI\\logo\\logo.paa";
		if (General.MaxApplicationsPerClan < 1)
			General.MaxApplicationsPerClan = 1;
		if (General.InviteTimeoutSeconds < 10)
			General.InviteTimeoutSeconds = 10;

		if (MemberHud.UpdateIntervalSeconds < 1)
			MemberHud.UpdateIntervalSeconds = 1;

		if (ChatSystem.MessageMaxLength < 1024)
			ChatSystem.MessageMaxLength = 1024;
		if (ChatSystem.LocalRangeMeters < 5)
			ChatSystem.LocalRangeMeters = 5;
		ValidateDiscordExamples();
		ValidateChatStyles();

		if (Storage.RadiusMeters < 3)
			Storage.RadiusMeters = 3;
		if (Pings.DurationSeconds < 5)
			Pings.DurationSeconds = 5;
		if (Pings.CooldownSeconds < 0.2)
			Pings.CooldownSeconds = 0.2;
		if (Pings.CooldownSeconds > 1.0)
			Pings.CooldownSeconds = 1.0;
		if (ClanMap.MaxMarkersPerClan < 1)
			ClanMap.MaxMarkersPerClan = 1;
		if (ClanMap.MarkerMaxNameLength < 3)
			ClanMap.MarkerMaxNameLength = 3;
		if (ClanMap.BaseRadiusMeters <= 0)
			ClanMap.BaseRadiusMeters = 150;
		if (!ClanMap.ServerMarkers)
			ClanMap.ServerMarkers = new array<ref SM_ServerMapMarker>;
		if (ClanMap.ServerMarkers.Count() == 0)
		{
			ClanMap.ServerMarkers.Insert(new SM_ServerMapMarker("#STR_SMP_01015", Vector(7500, 0, 7500), SM_ClanColors.GetColor(2), "\\dz\\gear\\navigation\\data\\map_cross_ca.paa", false));
			ClanMap.ServerMarkers.Insert(new SM_ServerMapMarker("#STR_SMP_00277", Vector(8200, 0, 9200), SM_ClanColors.GetColor(3), "\\dz\\gear\\navigation\\data\\map_camp_ca.paa", false));
		}
		foreach (SM_ServerMapMarker serverMarker : ClanMap.ServerMarkers)
		{
			if (!serverMarker)
				continue;

			serverMarker.Name = serverMarker.Name.Trim();
			if (serverMarker.Name == "")
				serverMarker.Name = "#STR_SMP_00910";
			if (serverMarker.Name.LengthUtf8() > ClanMap.MarkerMaxNameLength)
				serverMarker.Name = serverMarker.Name.Substring(0, ClanMap.MarkerMaxNameLength);
			serverMarker.IconPaa = SM_MapMarkerIconPath.Normalize(serverMarker.IconPaa);
			if (serverMarker.IconPaa == "")
				serverMarker.IconPaa = SM_ClanMarkerIcon.GetPath();
			if (!serverMarker.ColorRGB)
				serverMarker.ColorRGB = new SM_RGBColorConfig;
			if (!serverMarker.ColorRGB.IsConfigured())
				serverMarker.ColorRGB.FromARGB(SM_ClanColors.GetColor(0));
			serverMarker.ColorRGB.Clamp();
		}
		if (Logs.MaxEntries < 5)
			Logs.MaxEntries = 5;

		if (!Market.Blacklist)
			Market.Blacklist = new array<string>;
		if (Market.Blacklist.Count() == 0)
			Market.Blacklist.Insert("REPLACE_WITH_BLOCKED_ITEM_CLASSNAME");
		if (Market.MaxLotsPerClan < 1)
			Market.MaxLotsPerClan = 1;
		if (Market.FeePercent < 0)
			Market.FeePercent = 0;
		if (Market.FeePercent > 90)
			Market.FeePercent = 90;
		if (Market.ListingFeePercent < 0)
			Market.ListingFeePercent = 0;
		if (Market.ListingFeePercent > 90)
			Market.ListingFeePercent = 90;
		if (Market.MinCancelAgeMinutes < 0)
			Market.MinCancelAgeMinutes = 0;
		if (Market.CancelPenaltyPercent < 0)
			Market.CancelPenaltyPercent = 0;
		if (Market.CancelPenaltyPercent > 100)
			Market.CancelPenaltyPercent = 100;
		if (Market.ReturnDelaySeconds < 0)
			Market.ReturnDelaySeconds = 0;
		if (Market.MaxPrice < 1)
			Market.MaxPrice = 1;
		if (Market.MinRankToSell < 0 || Market.MinRankToSell > GetLeaderRank())
			Market.MinRankToSell = GetLeaderRank();
		ValidateTradeZones(Market.TradeZones);

		ValidateServerMarket();

		ValidateTops();
		ValidateAchievements();
		ValidatePlayerExperience();
		ValidateContracts();
		ValidateAuction();

		if (!Treasury.CurrencyItems)
			Treasury.CurrencyItems = new array<ref SM_CurrencyItem>;
		if (Treasury.CurrencyItems.Count() == 0)
		{
			Treasury.CurrencyItems.Insert(new SM_CurrencyItem("TraderPlus_Money_Ruble10000", 10000));
			Treasury.CurrencyItems.Insert(new SM_CurrencyItem("TraderPlus_Money_Ruble5000", 5000));
			Treasury.CurrencyItems.Insert(new SM_CurrencyItem("TraderPlus_Money_Ruble1000", 1000));
			Treasury.CurrencyItems.Insert(new SM_CurrencyItem("TraderPlus_Money_Ruble100", 100));
			Treasury.CurrencyItems.Insert(new SM_CurrencyItem("TraderPlus_Money_Ruble10", 10));
			Treasury.CurrencyItems.Insert(new SM_CurrencyItem("TraderPlus_Money_Ruble1", 1));
		}

		if (!Levels.Items)
			Levels.Items = new array<ref SM_ClanLevel>;
		if (Levels.Items.Count() == 0)
		{
			Levels.Items.Insert(new SM_ClanLevel(0,      15, 5, 3));
			Levels.Items.Insert(new SM_ClanLevel(10000,  25, 8, 5));
			Levels.Items.Insert(new SM_ClanLevel(30000,  40, 12, 7));
			Levels.Items.Insert(new SM_ClanLevel(75000,  60, 16, 10));
			Levels.Items.Insert(new SM_ClanLevel(150000, 90, 20, 12));
		}
		if (!Levels.Items[0])
			Levels.Items.Set(0, new SM_ClanLevel(0, 15, 5, ServerMarket.DefaultVisibleItems));
		Levels.Items[0].UpgradeCost = 0;
		for (int levelIndex = 0; levelIndex < Levels.Items.Count(); levelIndex++)
		{
			SM_ClanLevel level = Levels.Items[levelIndex];
			if (!level)
				continue;
			if (level.UpgradeCost < 0)
				level.UpgradeCost = 0;
			if (level.StorageSlots < 0)
				level.StorageSlots = 0;
			if (level.MarketLots < 1)
				level.MarketLots = 1;
			if (level.ServerMarketItems < 1)
				level.ServerMarketItems = ServerMarket.DefaultVisibleItems + levelIndex * 2;
		}

		int leaderRank = GetLeaderRank();
		if (Ranks.MinRankToInvite < 0 || Ranks.MinRankToInvite > leaderRank)
			Ranks.MinRankToInvite = leaderRank;
		if (Ranks.MinRankToKick < 0 || Ranks.MinRankToKick > leaderRank)
			Ranks.MinRankToKick = leaderRank;
	}

	protected void ValidateDiscordExamples()
	{
		Discord.WebhookUrl = Discord.WebhookUrl.Trim();
		Discord.ClanWebhookUrl = Discord.ClanWebhookUrl.Trim();
		Discord.LocalWebhookUrl = Discord.LocalWebhookUrl.Trim();
		Discord.GlobalWebhookUrl = Discord.GlobalWebhookUrl.Trim();
		Discord.ServerLabel = Discord.ServerLabel.Trim();

		if (Discord.WebhookUrl == "" || Discord.WebhookUrl == "PASTE_DISCORD_WEBHOOK_URL_HERE")
			Discord.WebhookUrl = "https://discord.com/api/webhooks/000000000000000000/PASTE_WEBHOOK_TOKEN_HERE";
		if (Discord.ClanWebhookUrl == "" || Discord.ClanWebhookUrl == "PASTE_CLAN_CHAT_WEBHOOK_URL_HERE")
			Discord.ClanWebhookUrl = "https://discord.com/api/webhooks/000000000000000000/PASTE_CLAN_CHAT_TOKEN_HERE";
		if (Discord.LocalWebhookUrl == "" || Discord.LocalWebhookUrl == "PASTE_LOCAL_CHAT_WEBHOOK_URL_HERE")
			Discord.LocalWebhookUrl = "https://discord.com/api/webhooks/000000000000000000/PASTE_LOCAL_CHAT_TOKEN_HERE";
		if (Discord.GlobalWebhookUrl == "" || Discord.GlobalWebhookUrl == "PASTE_GLOBAL_CHAT_WEBHOOK_URL_HERE")
			Discord.GlobalWebhookUrl = "https://discord.com/api/webhooks/000000000000000000/PASTE_GLOBAL_CHAT_TOKEN_HERE";
		if (Discord.ServerLabel == "")
			Discord.ServerLabel = "SobrMods Clan System";

		BaseRegistrationAudit.WebhookUrl = BaseRegistrationAudit.WebhookUrl.Trim();
	}

	protected void ValidateServerMarket()
	{
		if (!ServerMarket)
			ServerMarket = new SM_PartyServerMarketConfig;
		if (!ServerMarket.DynamicPriceSteps)
			ServerMarket.DynamicPriceSteps = new array<ref SM_ServerMarketPriceStep>;
		if (!ServerMarket.Items)
			ServerMarket.Items = new array<ref SM_ServerMarketItemConfig>;

		if (ServerMarket.RefreshIntervalSeconds < 60)
			ServerMarket.RefreshIntervalSeconds = 60;
		if (ServerMarket.GeneratedItemsCount < 1)
			ServerMarket.GeneratedItemsCount = 1;
		if (ServerMarket.DefaultVisibleItems < 1)
			ServerMarket.DefaultVisibleItems = 1;
		if (ServerMarket.PurchaseReservationSeconds < 1)
			ServerMarket.PurchaseReservationSeconds = 1;

		if (ServerMarket.DynamicPriceSteps.Count() == 0)
		{
			ServerMarket.DynamicPriceSteps.Insert(new SM_ServerMarketPriceStep(50, 1.10));
			ServerMarket.DynamicPriceSteps.Insert(new SM_ServerMarketPriceStep(25, 1.25));
			ServerMarket.DynamicPriceSteps.Insert(new SM_ServerMarketPriceStep(10, 1.50));
		}
		foreach (SM_ServerMarketPriceStep step : ServerMarket.DynamicPriceSteps)
		{
			if (!step)
				continue;
			if (step.StockPercentBelow < 1)
				step.StockPercentBelow = 1;
			if (step.StockPercentBelow > 100)
				step.StockPercentBelow = 100;
			if (step.PriceMultiplier < 1.0)
				step.PriceMultiplier = 1.0;
		}

		if (ServerMarket.Items.Count() == 0)
		{
			ServerMarket.Items.Insert(new SM_ServerMarketItemConfig("BandageDressing", 5, 12, 100, 250, 1.0, 3));
			ServerMarket.Items.Insert(new SM_ServerMarketItemConfig("TacticalBaconCan", 3, 8, 150, 350, 0.8, 2));
			ServerMarket.Items.Insert(new SM_ServerMarketItemConfig("Ammo_762x39", 2, 5, 500, 1200, 0.45, 1));
			ServerMarket.Items.Insert(new SM_ServerMarketItemConfig("M67Grenade", 1, 2, 2500, 5000, 0.15, 1));
		}
		for (int i = ServerMarket.Items.Count() - 1; i >= 0; i--)
		{
			SM_ServerMarketItemConfig item = ServerMarket.Items[i];
			if (!item)
			{
				ServerMarket.Items.Remove(i);
				continue;
			}

			item.ClassName = item.ClassName.Trim();
			if (item.ClassName == "")
			{
				ServerMarket.Items.Remove(i);
				continue;
			}
			if (item.MinQuantity < 1)
				item.MinQuantity = 1;
			if (item.MaxQuantity < item.MinQuantity)
				item.MaxQuantity = item.MinQuantity;
			if (item.MinPrice < 1)
				item.MinPrice = 1;
			if (item.MaxPrice < item.MinPrice)
				item.MaxPrice = item.MinPrice;
			if (item.SpawnChance < 0.0)
				item.SpawnChance = 0.0;
			if (item.SpawnChance > 1.0)
				item.SpawnChance = 1.0;
			if (item.MaxBuyPerPlayer < 0)
				item.MaxBuyPerPlayer = 0;
		}
	}

	protected bool TopCategoryExists(string categoryName)
	{
		if (!Tops || !Tops.Categories)
			return false;
		foreach (SM_TopCategoryConfig topCategory : Tops.Categories)
		{
			if (topCategory && topCategory.Category == categoryName)
				return true;
		}
		return false;
	}

	protected void ValidateTops()
	{
		if (!Tops)
			Tops = new SM_PartyTopConfig;
		if (!Tops.Categories)
			Tops.Categories = new array<ref SM_TopCategoryConfig>;
		if (!Tops.RewardLootContainers)
			Tops.RewardLootContainers = new array<ref SM_TopRewardContainerConfig>;

		for (int category = 0; category < SM_TopCategory.COUNT; category++)
		{
			string categoryName = SM_TopCategory.GetConfigName(category);
			if (!TopCategoryExists(categoryName))
				Tops.Categories.Insert(new SM_TopCategoryConfig(categoryName, true));
		}

		for (int i = Tops.Categories.Count() - 1; i >= 0; i--)
		{
			SM_TopCategoryConfig topCategory = Tops.Categories[i];
			if (!topCategory)
			{
				Tops.Categories.Remove(i);
				continue;
			}
			topCategory.Category = topCategory.Category.Trim();
			int categoryId = SM_TopCategory.GetByConfigName(topCategory.Category);
			if (categoryId < 0)
				Tops.Categories.Remove(i);
			else
				topCategory.Category = SM_TopCategory.GetConfigName(categoryId);
		}

		if (Tops.MinShotsForAccuracyTop < 1)
			Tops.MinShotsForAccuracyTop = 1;
		if (Tops.RewardSchedule != "monthly" && Tops.RewardSchedule != "hours")
			Tops.RewardSchedule = "monthly";
		if (Tops.RewardIntervalHours < 1)
			Tops.RewardIntervalHours = 1;
		if (Tops.RewardTreasuryPoints < 0)
			Tops.RewardTreasuryPoints = 0;

		if (Tops.RewardLootContainers.Count() == 0)
		{
			SM_TopRewardContainerConfig defaultContainer = new SM_TopRewardContainerConfig("WoodenCrate", 1, 1, 1.0);
			defaultContainer.Loot.Insert(new SM_TopRewardLootItemConfig("BandageDressing", 2, 4, 1.0));
			defaultContainer.Loot.Insert(new SM_TopRewardLootItemConfig("TacticalBaconCan", 2, 4, 1.0));
			defaultContainer.Loot.Insert(new SM_TopRewardLootItemConfig("Ammo_762x39", 1, 3, 0.65));
			Tops.RewardLootContainers.Insert(defaultContainer);
		}

		for (int c = Tops.RewardLootContainers.Count() - 1; c >= 0; c--)
		{
			SM_TopRewardContainerConfig container = Tops.RewardLootContainers[c];
			if (!container)
			{
				Tops.RewardLootContainers.Remove(c);
				continue;
			}
			container.ClassName = container.ClassName.Trim();
			if (container.ClassName == "")
			{
				Tops.RewardLootContainers.Remove(c);
				continue;
			}
			if (container.MinQuantity < 1)
				container.MinQuantity = 1;
			if (container.MaxQuantity < container.MinQuantity)
				container.MaxQuantity = container.MinQuantity;
			if (container.SpawnChance < 0.0)
				container.SpawnChance = 0.0;
			if (container.SpawnChance > 1.0)
				container.SpawnChance = 1.0;
			if (!container.Loot)
				container.Loot = new array<ref SM_TopRewardLootItemConfig>;

			for (int l = container.Loot.Count() - 1; l >= 0; l--)
			{
				SM_TopRewardLootItemConfig lootItem = container.Loot[l];
				if (!lootItem)
				{
					container.Loot.Remove(l);
					continue;
				}
				lootItem.ClassName = lootItem.ClassName.Trim();
				if (lootItem.ClassName == "")
				{
					container.Loot.Remove(l);
					continue;
				}
				if (lootItem.MinQuantity < 1)
					lootItem.MinQuantity = 1;
				if (lootItem.MaxQuantity < lootItem.MinQuantity)
					lootItem.MaxQuantity = lootItem.MinQuantity;
				if (lootItem.SpawnChance < 0.0)
					lootItem.SpawnChance = 0.0;
				if (lootItem.SpawnChance > 1.0)
					lootItem.SpawnChance = 1.0;
			}
		}
	}

	protected void ValidateAchievementRewardContainer(SM_TopRewardContainerConfig container)
	{
		if (!container)
			return;

		container.ClassName = container.ClassName.Trim();
		if (container.MinQuantity < 1)
			container.MinQuantity = 1;
		if (container.MaxQuantity < container.MinQuantity)
			container.MaxQuantity = container.MinQuantity;
		if (container.SpawnChance < 0.0)
			container.SpawnChance = 0.0;
		if (container.SpawnChance > 1.0)
			container.SpawnChance = 1.0;
		if (!container.Loot)
			container.Loot = new array<ref SM_TopRewardLootItemConfig>;

		for (int l = container.Loot.Count() - 1; l >= 0; l--)
		{
			SM_TopRewardLootItemConfig lootItem = container.Loot[l];
			if (!lootItem)
			{
				container.Loot.Remove(l);
				continue;
			}

			lootItem.ClassName = lootItem.ClassName.Trim();
			if (lootItem.ClassName == "")
			{
				container.Loot.Remove(l);
				continue;
			}
			if (lootItem.MinQuantity < 1)
				lootItem.MinQuantity = 1;
			if (lootItem.MaxQuantity < lootItem.MinQuantity)
				lootItem.MaxQuantity = lootItem.MinQuantity;
			if (lootItem.SpawnChance < 0.0)
				lootItem.SpawnChance = 0.0;
			if (lootItem.SpawnChance > 1.0)
				lootItem.SpawnChance = 1.0;
		}
	}

	protected bool AchievementConfigExists(array<ref SM_AchievementConfig> list, string id)
	{
		if (!list)
			return false;

		foreach (SM_AchievementConfig achievement : list)
		{
			if (!achievement)
				continue;

			string existingId = achievement.Id;
			existingId = existingId.Trim();
			if (existingId == id)
				return true;
		}

		return false;
	}

	protected void AddDefaultPersonalAchievement(string id, string name, string description, string type, int target, int rewardMoney)
	{
		if (AchievementConfigExists(AchievementSettings.Personal, id))
			return;

		SM_AchievementConfig achievement = new SM_AchievementConfig(id, name, description, type, target);
		achievement.RewardMoney = rewardMoney;
		AchievementSettings.Personal.Insert(achievement);
	}

	protected void AddDefaultClanAchievement(string id, string name, string description, string type, int target, int rewardTreasury)
	{
		if (AchievementConfigExists(AchievementSettings.Clan, id))
			return;

		SM_AchievementConfig achievement = new SM_AchievementConfig(id, name, description, type, target);
		achievement.RewardClanTreasury = rewardTreasury;
		AchievementSettings.Clan.Insert(achievement);
	}

	protected void ValidateAchievementList(array<ref SM_AchievementConfig> list)
	{
		if (!list)
			return;

		for (int i = list.Count() - 1; i >= 0; i--)
		{
			SM_AchievementConfig achievement = list[i];
			if (!achievement)
			{
				list.Remove(i);
				continue;
			}

			achievement.Id = achievement.Id.Trim();
			achievement.Name = achievement.Name.Trim();
			achievement.Description = achievement.Description.Trim();
			achievement.Type = achievement.Type.Trim();
			if (achievement.Id == "" || achievement.Type == "")
			{
				list.Remove(i);
				continue;
			}
			if (achievement.Name == "")
				achievement.Name = achievement.Id;
			if (achievement.Target < 1)
				achievement.Target = 1;
			if (achievement.RewardMoney < 0)
				achievement.RewardMoney = 0;
			if (achievement.RewardClanTreasury < 0)
				achievement.RewardClanTreasury = 0;
			ValidateAchievementRewardContainer(achievement.RewardContainer);
		}
	}

	protected void ValidateAchievements()
	{
		if (!AchievementSettings)
			AchievementSettings = new SM_PartyAchievementsConfig;
		if (!AchievementSettings.Personal)
			AchievementSettings.Personal = new array<ref SM_AchievementConfig>;
		if (!AchievementSettings.Clan)
			AchievementSettings.Clan = new array<ref SM_AchievementConfig>;

		AddDefaultPersonalAchievement("personal_created_clan", "#STR_SMP_00740", "#STR_SMP_00960", "CreatedClan", 1, 1500);
		AddDefaultPersonalAchievement("personal_joined_clan", "#STR_SMP_00291", "#STR_SMP_00342", "JoinedClan", 1, 1000);
		AddDefaultPersonalAchievement("personal_first_blood", "#STR_SMP_00782", "#STR_SMP_01029", "PlayerKills", 1, 1000);
		AddDefaultPersonalAchievement("personal_hunter_5", "#STR_SMP_00763", "#STR_SMP_01025", "PlayerKills", 5, 2500);
		AddDefaultPersonalAchievement("personal_hunter_10", "#STR_SMP_00978", "#STR_SMP_01019", "PlayerKills", 10, 5000);
		AddDefaultPersonalAchievement("personal_hunter_25", "#STR_SMP_00729", "#STR_SMP_01023", "PlayerKills", 25, 10000);
		AddDefaultPersonalAchievement("personal_hunter_50", "#STR_SMP_00386", "#STR_SMP_01027", "PlayerKills", 50, 20000);
		AddDefaultPersonalAchievement("personal_hunter_100", "#STR_SMP_00598", "#STR_SMP_01021", "PlayerKills", 100, 40000);
		AddDefaultPersonalAchievement("personal_zombies_10", "#STR_SMP_00781", "#STR_SMP_01018", "ZombieKills", 10, 1000);
		AddDefaultPersonalAchievement("personal_zombies_50", "#STR_SMP_00897", "#STR_SMP_01026", "ZombieKills", 50, 2500);
		AddDefaultPersonalAchievement("personal_zombies_100", "#STR_SMP_01070", "#STR_SMP_01020", "ZombieKills", 100, 5000);
		AddDefaultPersonalAchievement("personal_zombies_250", "#STR_SMP_00496", "#STR_SMP_01024", "ZombieKills", 250, 10000);
		AddDefaultPersonalAchievement("personal_zombies_500", "#STR_SMP_00655", "#STR_SMP_01028", "ZombieKills", 500, 20000);
		AddDefaultPersonalAchievement("personal_zombies_1000", "#STR_SMP_00372", "#STR_SMP_01022", "ZombieKills", 1000, 40000);
		AddDefaultPersonalAchievement("personal_online_1h", "#STR_SMP_00783", "#STR_SMP_00846", "OnlineSeconds", 3600, 1000);
		AddDefaultPersonalAchievement("personal_online_6h", "#STR_SMP_00411", "#STR_SMP_00848", "OnlineSeconds", 21600, 3000);
		AddDefaultPersonalAchievement("personal_online_24h", "#STR_SMP_00980", "#STR_SMP_00847", "OnlineSeconds", 86400, 10000);
		AddDefaultPersonalAchievement("personal_online_72h", "#STR_SMP_00433", "#STR_SMP_00849", "OnlineSeconds", 259200, 25000);
		AddDefaultPersonalAchievement("personal_life_30m", "#STR_SMP_00742", "#STR_SMP_00863", "BestLifeSeconds", 1800, 1000);
		AddDefaultPersonalAchievement("personal_life_2h", "#STR_SMP_00587", "#STR_SMP_00862", "BestLifeSeconds", 7200, 5000);
		AddDefaultPersonalAchievement("personal_life_6h", "#STR_SMP_00745", "#STR_SMP_00864", "BestLifeSeconds", 21600, 15000);
		AddDefaultPersonalAchievement("personal_life_12h", "#STR_SMP_00707", "#STR_SMP_00861", "BestLifeSeconds", 43200, 30000);
		AddDefaultPersonalAchievement("personal_distance_1km", "#STR_SMP_00777", "#STR_SMP_00868", "DistanceWalked", 1000, 1000);
		AddDefaultPersonalAchievement("personal_distance_5km", "#STR_SMP_00785", "#STR_SMP_00871", "DistanceWalked", 5000, 2500);
		AddDefaultPersonalAchievement("personal_distance_10km", "#STR_SMP_00878", "#STR_SMP_00869", "DistanceWalked", 10000, 5000);
		AddDefaultPersonalAchievement("personal_distance_25km", "#STR_SMP_00394", "#STR_SMP_00870", "DistanceWalked", 25000, 10000);
		AddDefaultPersonalAchievement("personal_distance_50km", "#STR_SMP_00629", "#STR_SMP_00872", "DistanceWalked", 50000, 20000);
		AddDefaultPersonalAchievement("personal_shots_100", "#STR_SMP_00881", "#STR_SMP_00901", "ShotsFired", 100, 1000);
		AddDefaultPersonalAchievement("personal_shots_500", "#STR_SMP_00387", "#STR_SMP_00905", "ShotsFired", 500, 5000);
		AddDefaultPersonalAchievement("personal_shots_1000", "#STR_SMP_00818", "#STR_SMP_00902", "ShotsFired", 1000, 10000);
		AddDefaultPersonalAchievement("personal_hits_50", "#STR_SMP_01012", "#STR_SMP_00815", "HitsLanded", 50, 2500);
		AddDefaultPersonalAchievement("personal_hits_250", "#STR_SMP_00968", "#STR_SMP_00814", "HitsLanded", 250, 10000);
		AddDefaultPersonalAchievement("personal_hits_1000", "#STR_SMP_00427", "#STR_SMP_00813", "HitsLanded", 1000, 30000);
		AddDefaultPersonalAchievement("personal_headshots_10", "#STR_SMP_00382", "#STR_SMP_00900", "Headshots", 10, 3000);
		AddDefaultPersonalAchievement("personal_headshots_50", "#STR_SMP_01048", "#STR_SMP_00904", "Headshots", 50, 12000);
		AddDefaultPersonalAchievement("personal_headshots_150", "#STR_SMP_00944", "#STR_SMP_00903", "Headshots", 150, 30000);
		AddDefaultPersonalAchievement("personal_long_100", "#STR_SMP_00397", "#STR_SMP_01030", "LongestKill", 100, 5000);
		AddDefaultPersonalAchievement("personal_long_300", "#STR_SMP_00391", "#STR_SMP_01031", "LongestKill", 300, 15000);
		AddDefaultPersonalAchievement("personal_long_500", "#STR_SMP_01068", "#STR_SMP_01032", "LongestKill", 500, 30000);
		AddDefaultPersonalAchievement("personal_treasury_1k", "#STR_SMP_00778", "#STR_SMP_00326", "TreasuryDeposited", 1000, 1000);
		AddDefaultPersonalAchievement("personal_treasury_10k", "#STR_SMP_00661", "#STR_SMP_00327", "TreasuryDeposited", 10000, 5000);
		AddDefaultPersonalAchievement("personal_treasury_50k", "#STR_SMP_00737", "#STR_SMP_00328", "TreasuryDeposited", 50000, 20000);
		AddDefaultPersonalAchievement("personal_market_buy_1", "#STR_SMP_00774", "#STR_SMP_00592", "MarketBuys", 1, 1000);
		AddDefaultPersonalAchievement("personal_market_buy_10", "#STR_SMP_00821", "#STR_SMP_00593", "MarketBuys", 10, 5000);
		AddDefaultPersonalAchievement("personal_market_sell_1", "#STR_SMP_00780", "#STR_SMP_00856", "MarketSells", 1, 1000);
		AddDefaultPersonalAchievement("personal_market_sell_10", "#STR_SMP_01007", "#STR_SMP_00857", "MarketSells", 10, 7000);
		AddDefaultPersonalAchievement("personal_storage_10", "#STR_SMP_00503", "#STR_SMP_00804", "StorageDeposits", 10, 3000);
		AddDefaultPersonalAchievement("personal_storage_50", "#STR_SMP_00443", "#STR_SMP_00805", "StorageDeposits", 50, 10000);
		AddDefaultPersonalAchievement("personal_auction_win", "#STR_SMP_00775", "#STR_SMP_00373", "AuctionWins", 1, 2500);
		AddDefaultPersonalAchievement("personal_auction_sales_5", "#STR_SMP_00260", "#STR_SMP_00858", "AuctionSales", 5, 10000);

		AddDefaultClanAchievement("clan_members_2", "#STR_SMP_00279", "#STR_SMP_00288", "Members", 2, 2000);
		AddDefaultClanAchievement("clan_members_4", "#STR_SMP_00628", "#STR_SMP_00289", "Members", 4, 5000);
		AddDefaultClanAchievement("clan_members_6", "#STR_SMP_00958", "#STR_SMP_00290", "Members", 6, 10000);
		AddDefaultClanAchievement("clan_members_10", "#STR_SMP_00802", "#STR_SMP_00287", "Members", 10, 20000);
		AddDefaultClanAchievement("clan_level_2", "#STR_SMP_00776", "#STR_SMP_00516", "Level", 2, 5000);
		AddDefaultClanAchievement("clan_level_3", "#STR_SMP_00879", "#STR_SMP_00517", "Level", 3, 15000);
		AddDefaultClanAchievement("clan_level_5", "#STR_SMP_00381", "#STR_SMP_00518", "Level", 5, 40000);
		AddDefaultClanAchievement("clan_treasury_10k", "#STR_SMP_00717", "#STR_SMP_00485", "Treasury", 10000, 5000);
		AddDefaultClanAchievement("clan_treasury_50k", "#STR_SMP_00444", "#STR_SMP_00488", "Treasury", 50000, 15000);
		AddDefaultClanAchievement("clan_treasury_100k", "#STR_SMP_00494", "#STR_SMP_00486", "Treasury", 100000, 30000);
		AddDefaultClanAchievement("clan_treasury_250k", "#STR_SMP_00281", "#STR_SMP_00487", "Treasury", 250000, 60000);
		AddDefaultClanAchievement("clan_treasury_500k", "#STR_SMP_01044", "#STR_SMP_00489", "Treasury", 500000, 100000);
		AddDefaultClanAchievement("clan_deposited_10k", "#STR_SMP_00721", "#STR_SMP_00512", "TreasuryDeposited", 10000, 5000);
		AddDefaultClanAchievement("clan_deposited_50k", "#STR_SMP_00588", "#STR_SMP_00514", "TreasuryDeposited", 50000, 15000);
		AddDefaultClanAchievement("clan_deposited_100k", "#STR_SMP_00403", "#STR_SMP_00513", "TreasuryDeposited", 100000, 30000);
		AddDefaultClanAchievement("clan_kills_10", "#STR_SMP_00779", "#STR_SMP_00550", "PlayerKills", 10, 5000);
		AddDefaultClanAchievement("clan_kills_25", "#STR_SMP_00760", "#STR_SMP_00554", "PlayerKills", 25, 10000);
		AddDefaultClanAchievement("clan_kills_50", "#STR_SMP_00915", "#STR_SMP_00557", "PlayerKills", 50, 20000);
		AddDefaultClanAchievement("clan_kills_100", "#STR_SMP_00728", "#STR_SMP_00552", "PlayerKills", 100, 40000);
		AddDefaultClanAchievement("clan_kills_250", "#STR_SMP_00331", "#STR_SMP_00555", "PlayerKills", 250, 90000);
		AddDefaultClanAchievement("clan_zombies_100", "#STR_SMP_00448", "#STR_SMP_00551", "ZombieKills", 100, 5000);
		AddDefaultClanAchievement("clan_zombies_500", "#STR_SMP_00447", "#STR_SMP_00558", "ZombieKills", 500, 15000);
		AddDefaultClanAchievement("clan_zombies_1000", "#STR_SMP_00497", "#STR_SMP_00553", "ZombieKills", 1000, 30000);
		AddDefaultClanAchievement("clan_zombies_2500", "#STR_SMP_00638", "#STR_SMP_00556", "ZombieKills", 2500, 70000);
		AddDefaultClanAchievement("clan_zombies_5000", "#STR_SMP_01069", "#STR_SMP_00559", "ZombieKills", 5000, 120000);
		AddDefaultClanAchievement("clan_online_10h", "#STR_SMP_00722", "#STR_SMP_00525", "OnlineSeconds", 36000, 5000);
		AddDefaultClanAchievement("clan_online_50h", "#STR_SMP_00418", "#STR_SMP_00528", "OnlineSeconds", 180000, 15000);
		AddDefaultClanAchievement("clan_online_100h", "#STR_SMP_00432", "#STR_SMP_00526", "OnlineSeconds", 360000, 30000);
		AddDefaultClanAchievement("clan_online_250h", "#STR_SMP_00820", "#STR_SMP_00527", "OnlineSeconds", 900000, 70000);
		AddDefaultClanAchievement("clan_distance_50km", "#STR_SMP_00633", "#STR_SMP_00535", "DistanceWalked", 50000, 10000);
		AddDefaultClanAchievement("clan_distance_100km", "#STR_SMP_00395", "#STR_SMP_00533", "DistanceWalked", 100000, 25000);
		AddDefaultClanAchievement("clan_distance_250km", "#STR_SMP_01067", "#STR_SMP_00534", "DistanceWalked", 250000, 60000);
		AddDefaultClanAchievement("clan_shots_1000", "#STR_SMP_00724", "#STR_SMP_00542", "ShotsFired", 1000, 10000);
		AddDefaultClanAchievement("clan_shots_5000", "#STR_SMP_00388", "#STR_SMP_00545", "ShotsFired", 5000, 30000);
		AddDefaultClanAchievement("clan_shots_10000", "#STR_SMP_00817", "#STR_SMP_00543", "ShotsFired", 10000, 60000);
		AddDefaultClanAchievement("clan_hits_500", "#STR_SMP_00718", "#STR_SMP_00524", "HitsLanded", 500, 15000);
		AddDefaultClanAchievement("clan_hits_2500", "#STR_SMP_00931", "#STR_SMP_00523", "HitsLanded", 2500, 50000);
		AddDefaultClanAchievement("clan_headshots_100", "#STR_SMP_01047", "#STR_SMP_00541", "Headshots", 100, 30000);
		AddDefaultClanAchievement("clan_headshots_500", "#STR_SMP_01014", "#STR_SMP_00544", "Headshots", 500, 90000);
		AddDefaultClanAchievement("clan_long_300", "#STR_SMP_00396", "#STR_SMP_00546", "LongestKill", 300, 15000);
		AddDefaultClanAchievement("clan_long_500", "#STR_SMP_00945", "#STR_SMP_00547", "LongestKill", 500, 30000);
		AddDefaultClanAchievement("clan_long_800", "#STR_SMP_00437", "#STR_SMP_00548", "LongestKill", 800, 70000);
		AddDefaultClanAchievement("clan_life_6h", "#STR_SMP_00429", "#STR_SMP_00624", "BestLifeSeconds", 21600, 15000);
		AddDefaultClanAchievement("clan_life_12h", "#STR_SMP_00744", "#STR_SMP_00622", "BestLifeSeconds", 43200, 30000);
		AddDefaultClanAchievement("clan_life_24h", "#STR_SMP_00702", "#STR_SMP_00623", "BestLifeSeconds", 86400, 70000);
		AddDefaultClanAchievement("clan_market_sales_10", "#STR_SMP_01006", "#STR_SMP_00530", "MarketSells", 10, 10000);
		AddDefaultClanAchievement("clan_market_sales_50", "#STR_SMP_01008", "#STR_SMP_00531", "MarketSells", 50, 40000);
		AddDefaultClanAchievement("clan_storage_100", "#STR_SMP_00927", "#STR_SMP_00522", "StorageDeposits", 100, 20000);
		AddDefaultClanAchievement("clan_auction_sales_10", "#STR_SMP_00259", "#STR_SMP_00529", "AuctionSales", 10, 30000);
		AddDefaultClanAchievement("clan_auction_wins_10", "#STR_SMP_00374", "#STR_SMP_00515", "AuctionWins", 10, 30000);

		ValidateAchievementList(AchievementSettings.Personal);
		ValidateAchievementList(AchievementSettings.Clan);
	}

	protected int GetDefaultPlayerTitleColor(int index)
	{
		switch (index)
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

	protected void AddDefaultPlayerTitleRank(string name, int requiredXP, int colorIndex)
	{
		SM_PlayerTitleRankConfig rank = new SM_PlayerTitleRankConfig(name, requiredXP, GetDefaultPlayerTitleColor(colorIndex));
		rank.Color = 0;
		PlayerExperience.Ranks.Insert(rank);
	}

	protected void SortPlayerTitleRanks()
	{
		for (int a = 0; a < PlayerExperience.Ranks.Count(); a++)
		{
			for (int b = a + 1; b < PlayerExperience.Ranks.Count(); b++)
			{
				SM_PlayerTitleRankConfig left = PlayerExperience.Ranks[a];
				SM_PlayerTitleRankConfig right = PlayerExperience.Ranks[b];
				if (left && right && right.RequiredXP < left.RequiredXP)
				{
					PlayerExperience.Ranks.Set(a, right);
					PlayerExperience.Ranks.Set(b, left);
				}
			}
		}
	}

	protected int FindPlayerTitleRankConfigIndex(string title)
	{
		if (!PlayerExperience || !PlayerExperience.Ranks)
			return -1;

		string searched = title.Trim();
		searched.ToLower();
		if (searched == "")
			return -1;

		for (int i = 0; i < PlayerExperience.Ranks.Count(); i++)
		{
			SM_PlayerTitleRankConfig rank = PlayerExperience.Ranks[i];
			if (!rank)
				continue;

			string current = rank.Name;
			current.ToLower();
			if (current == searched)
				return i;
		}
		return -1;
	}

	protected void ValidatePlayerExperience()
	{
		if (!PlayerExperience)
			PlayerExperience = new SM_PlayerExperienceConfig;
		if (!PlayerExperience.Ranks)
			PlayerExperience.Ranks = new array<ref SM_PlayerTitleRankConfig>;
		if (!PlayerExperience.ZombieClassRewards)
			PlayerExperience.ZombieClassRewards = new array<ref SM_PlayerExperienceZombieRewardConfig>;
		if (!PlayerExperience.RestrictedItems)
			PlayerExperience.RestrictedItems = new array<ref SM_PlayerExperienceRestrictedItemConfig>;

		if (PlayerExperience.ZombieKillXP < 0)
			PlayerExperience.ZombieKillXP = 0;
		if (PlayerExperience.PlayerKillXP < 0)
			PlayerExperience.PlayerKillXP = 0;
		if (PlayerExperience.OnlineRewardIntervalSeconds < 10)
			PlayerExperience.OnlineRewardIntervalSeconds = 10;
		if (PlayerExperience.OnlineRewardXP < 0)
			PlayerExperience.OnlineRewardXP = 0;
		if (PlayerExperience.MaxPublicPlayers < 10)
			PlayerExperience.MaxPublicPlayers = 10;
		if (PlayerExperience.MaxPublicPlayers > 1000)
			PlayerExperience.MaxPublicPlayers = 1000;

		if (PlayerExperience.Ranks.Count() == 0)
		{
			AddDefaultPlayerTitleRank("#STR_SMP_00709", 0, 0);
			AddDefaultPlayerTitleRank("#STR_SMP_00284", 50, 1);
			AddDefaultPlayerTitleRank("#STR_SMP_00478", 150, 2);
			AddDefaultPlayerTitleRank("#STR_SMP_00932", 350, 3);
			AddDefaultPlayerTitleRank("#STR_SMP_00971", 700, 4);
			AddDefaultPlayerTitleRank("#STR_SMP_00739", 1200, 5);
			AddDefaultPlayerTitleRank("#STR_SMP_00319", 2000, 6);
			AddDefaultPlayerTitleRank("#STR_SMP_00634", 3200, 7);
			AddDefaultPlayerTitleRank("#STR_SMP_00571", 5000, 8);
			AddDefaultPlayerTitleRank("#STR_SMP_00597", 7500, 9);
			AddDefaultPlayerTitleRank("#STR_SMP_00428", 11000, 10);
		}

		for (int i = PlayerExperience.Ranks.Count() - 1; i >= 0; i--)
		{
			SM_PlayerTitleRankConfig rank = PlayerExperience.Ranks[i];
			if (!rank)
			{
				PlayerExperience.Ranks.Remove(i);
				continue;
			}

			rank.Name = rank.Name.Trim();
			if (rank.Name == "")
				rank.Name = "#STR_SMP_00457";
			if (rank.RequiredXP < 0)
				rank.RequiredXP = 0;
			if (!rank.ColorRGB)
				rank.ColorRGB = new SM_RGBColorConfig;
			if (rank.Color != 0)
			{
				rank.ColorRGB.FromARGB(rank.Color);
				rank.Color = 0;
			}
			if (!rank.ColorRGB.IsConfigured())
				rank.ColorRGB.FromARGB(GetDefaultPlayerTitleColor(i));
			rank.ColorRGB.Clamp();
		}

		if (PlayerExperience.Ranks.Count() == 0)
			AddDefaultPlayerTitleRank("#STR_SMP_00709", 0, 0);

		SortPlayerTitleRanks();

		if (PlayerExperience.Ranks[0].RequiredXP > 0)
		{
			SM_PlayerTitleRankConfig firstRank = new SM_PlayerTitleRankConfig("#STR_SMP_00709", 0, GetDefaultPlayerTitleColor(0));
			firstRank.Color = 0;
			PlayerExperience.Ranks.InsertAt(firstRank, 0);
		}

		for (int r = 1; r < PlayerExperience.Ranks.Count(); r++)
		{
			SM_PlayerTitleRankConfig prevRank = PlayerExperience.Ranks[r - 1];
			SM_PlayerTitleRankConfig currentRank = PlayerExperience.Ranks[r];
			if (!prevRank || !currentRank)
				continue;
			if (currentRank.RequiredXP <= prevRank.RequiredXP)
				currentRank.RequiredXP = prevRank.RequiredXP + 1;
		}

		string firstTitle = PlayerExperience.Ranks[0].Name;
		for (int ri = PlayerExperience.RestrictedItems.Count() - 1; ri >= 0; ri--)
		{
			SM_PlayerExperienceRestrictedItemConfig rule = PlayerExperience.RestrictedItems[ri];
			if (!rule)
			{
				PlayerExperience.RestrictedItems.Remove(ri);
				continue;
			}

			rule.ClassName = rule.ClassName.Trim();
			rule.RequiredTitle = rule.RequiredTitle.Trim();
			if (rule.ClassName == "")
			{
				PlayerExperience.RestrictedItems.Remove(ri);
				continue;
			}
			if (rule.RequiredTitle == "" || FindPlayerTitleRankConfigIndex(rule.RequiredTitle) < 0)
				rule.RequiredTitle = firstTitle;
		}

		if (PlayerExperience.RestrictedItems.Count() == 0)
		{
			string exampleTitle = "#STR_SMP_00971";
			if (FindPlayerTitleRankConfigIndex(exampleTitle) < 0)
				exampleTitle = firstTitle;
			PlayerExperience.RestrictedItems.Insert(new SM_PlayerExperienceRestrictedItemConfig("REPLACE_WITH_ITEM_CLASSNAME", exampleTitle, true));
		}

		for (int z = PlayerExperience.ZombieClassRewards.Count() - 1; z >= 0; z--)
		{
			SM_PlayerExperienceZombieRewardConfig reward = PlayerExperience.ZombieClassRewards[z];
			if (!reward)
			{
				PlayerExperience.ZombieClassRewards.Remove(z);
				continue;
			}

			reward.ClassName = reward.ClassName.Trim();
			if (reward.ClassName == "")
			{
				PlayerExperience.ZombieClassRewards.Remove(z);
				continue;
			}
			if (reward.XP < 0)
				reward.XP = 0;
		}

		if (PlayerExperience.ZombieClassRewards.Count() == 0)
		{
			PlayerExperience.ZombieClassRewards.Insert(new SM_PlayerExperienceZombieRewardConfig("ZmbM_NBC_Yellow", 3));
			PlayerExperience.ZombieClassRewards.Insert(new SM_PlayerExperienceZombieRewardConfig("ZmbF_NBC_Yellow", 3));
			PlayerExperience.ZombieClassRewards.Insert(new SM_PlayerExperienceZombieRewardConfig("ZmbM_SoldierNormal", 2));
		}
	}

	protected void ValidateContracts()
	{
		if (!Contracts)
			Contracts = new SM_PartyContractsConfig;
		if (!Contracts.SearchItems)
			Contracts.SearchItems = new array<ref SM_ContractItemConfig>;

		Contracts.RequiredCreatorTitle = Contracts.RequiredCreatorTitle.Trim();
		if (Contracts.RequiredCreatorTitle == "")
			Contracts.RequiredCreatorTitle = "#STR_SMP_00709";

		if (Contracts.MinPrice < 1)
			Contracts.MinPrice = 1;
		if (Contracts.MaxPrice < Contracts.MinPrice)
			Contracts.MaxPrice = Contracts.MinPrice;
		if (Contracts.FeePercent < 0)
			Contracts.FeePercent = 0;
		if (Contracts.FeePercent > 90)
			Contracts.FeePercent = 90;
		if (Contracts.AbandonPenaltyPercent < 0)
			Contracts.AbandonPenaltyPercent = 0;
		if (Contracts.AbandonPenaltyPercent > 100)
			Contracts.AbandonPenaltyPercent = 100;
		if (Contracts.MinDurationMinutes < 1)
			Contracts.MinDurationMinutes = 1;
		if (Contracts.MaxDurationMinutes < Contracts.MinDurationMinutes)
			Contracts.MaxDurationMinutes = Contracts.MinDurationMinutes;
		if (Contracts.TargetMarkerIntervalSeconds < 10)
			Contracts.TargetMarkerIntervalSeconds = 10;
		if (Contracts.TickSeconds < 5)
			Contracts.TickSeconds = 5;
		if (Contracts.MaxActiveCreatedPerPlayer < 1)
			Contracts.MaxActiveCreatedPerPlayer = 1;
		if (Contracts.MaxActiveAcceptedPerPlayer < 1)
			Contracts.MaxActiveAcceptedPerPlayer = 1;

		for (int i = Contracts.SearchItems.Count() - 1; i >= 0; i--)
		{
			SM_ContractItemConfig item = Contracts.SearchItems[i];
			if (!item)
			{
				Contracts.SearchItems.Remove(i);
				continue;
			}
			item.ClassName = item.ClassName.Trim();
			item.DisplayName = item.DisplayName.Trim();
			if (item.ClassName == "")
			{
				Contracts.SearchItems.Remove(i);
				continue;
			}
			if (item.DisplayName == "")
				item.DisplayName = item.ClassName;
		}

		if (Contracts.SearchItems.Count() == 0)
		{
			Contracts.SearchItems.Insert(new SM_ContractItemConfig("BandageDressing", "#STR_SMP_00278", true));
			Contracts.SearchItems.Insert(new SM_ContractItemConfig("TacticalBaconCan", "#STR_SMP_00983", true));
			Contracts.SearchItems.Insert(new SM_ContractItemConfig("Ammo_762x39", "#STR_SMP_00773", true));
		}
	}

	protected void ValidateTradeZones(SM_PartyTradeZoneRestrictionConfig tradeZones)
	{
		if (!tradeZones)
			return;
		if (!tradeZones.Zones)
			tradeZones.Zones = new array<ref SM_PartyTradeZoneConfig>;

		for (int i = tradeZones.Zones.Count() - 1; i >= 0; i--)
		{
			SM_PartyTradeZoneConfig zone = tradeZones.Zones[i];
			if (!zone)
			{
				tradeZones.Zones.Remove(i);
				continue;
			}

			zone.Name = zone.Name.Trim();
			if (zone.Name == "")
				zone.Name = "#STR_SMP_01015";
			if (zone.RadiusMeters < 1.0)
				zone.RadiusMeters = 1.0;
		}

		if (tradeZones.Zones.Count() == 0)
			tradeZones.Zones.Insert(new SM_PartyTradeZoneConfig("#STR_SMP_01015", Vector(7500, 0, 7500), 30.0, true));
	}

	protected void ValidateAuction()
	{
		if (!Auction)
			Auction = new SM_PartyAuctionConfig;
		if (!Auction.TradeZones)
			Auction.TradeZones = new SM_PartyTradeZoneRestrictionConfig;
		if (Auction.MaxActiveLotsPerPlayer < 1)
			Auction.MaxActiveLotsPerPlayer = 1;
		if (Auction.MinDurationMinutes < 1)
			Auction.MinDurationMinutes = 1;
		if (Auction.MaxDurationMinutes < Auction.MinDurationMinutes)
			Auction.MaxDurationMinutes = Auction.MinDurationMinutes;
		if (Auction.DefaultDurationMinutes < Auction.MinDurationMinutes)
			Auction.DefaultDurationMinutes = Auction.MinDurationMinutes;
		if (Auction.DefaultDurationMinutes > Auction.MaxDurationMinutes)
			Auction.DefaultDurationMinutes = Auction.MaxDurationMinutes;
		if (Auction.MinBidStep < 1)
			Auction.MinBidStep = 1;
		if (Auction.ListingFee < 0)
			Auction.ListingFee = 0;
		if (Auction.ListingFeePercent < 0)
			Auction.ListingFeePercent = 0;
		if (Auction.ListingFeePercent > 90)
			Auction.ListingFeePercent = 90;
		if (Auction.SaleTaxPercent < 0)
			Auction.SaleTaxPercent = 0;
		if (Auction.SaleTaxPercent > 90)
			Auction.SaleTaxPercent = 90;
		if (Auction.AntiSnipeSeconds < 0)
			Auction.AntiSnipeSeconds = 0;
		if (Auction.AntiSnipeExtendSeconds < 0)
			Auction.AntiSnipeExtendSeconds = 0;
		if (Auction.MaxStartPrice < 1)
			Auction.MaxStartPrice = 1;
		if (Auction.MinCancelAgeMinutes < 0)
			Auction.MinCancelAgeMinutes = 0;
		if (Auction.CancelPenaltyPercent < 0)
			Auction.CancelPenaltyPercent = 0;
		if (Auction.CancelPenaltyPercent > 100)
			Auction.CancelPenaltyPercent = 100;
		if (Auction.ReturnDelaySeconds < 0)
			Auction.ReturnDelaySeconds = 0;
		ValidateTradeZones(Auction.TradeZones);
	}

	protected void ValidateChatStyles()
	{
		ValidateChatChannel(ChatSystem.Clan);
		ValidateChatChannel(ChatSystem.Local);
		ValidateChatChannel(ChatSystem.Global);
		ValidateChatRgbColors();

		if (ChatSystem.Clan.PlayerStyles.Count() == 0)
			ChatSystem.Clan.PlayerStyles.Insert(new SM_ChatPlayerStyle("REPLACE_WITH_STEAMID64", "[CLAN VIP]", "#6EE7B7", "#FFFFFF", "#22D3EE"));
		if (ChatSystem.Local.PlayerStyles.Count() == 0)
			ChatSystem.Local.PlayerStyles.Insert(new SM_ChatPlayerStyle("REPLACE_WITH_STEAMID64", "[LOCAL]", "#FACC15", "#FFFFFF", "#D89A32"));
		if (ChatSystem.Global.PlayerStyles.Count() == 0)
			ChatSystem.Global.PlayerStyles.Insert(new SM_ChatPlayerStyle("REPLACE_WITH_STEAMID64", "[VIP]", "#FFD34D", "#FFFFFF", "#FF8A3D"));

		ValidateChatStyleList(ChatSystem.Clan.PlayerStyles);
		ValidateChatStyleList(ChatSystem.Local.PlayerStyles);
		ValidateChatStyleList(ChatSystem.Global.PlayerStyles);
	}

	protected void ValidateChatChannel(SM_PartyChatChannelConfig channel)
	{
		if (!channel.PlayerStyles)
			channel.PlayerStyles = new array<ref SM_ChatPlayerStyle>;
	}

	protected void ValidateChatRgbColors()
	{
		if (!ChatSystem.ColorClan)
			ChatSystem.ColorClan = new SM_RGBColorConfig;
		if (!ChatSystem.ColorClan.IsConfigured())
			ChatSystem.ColorClan.SetRGB(224, 224, 224);
		ChatSystem.ColorClan.Clamp();

		if (!ChatSystem.ColorClanPlayer)
			ChatSystem.ColorClanPlayer = new SM_RGBColorConfig;
		if (!ChatSystem.ColorClanPlayer.IsConfigured())
			ChatSystem.ColorClanPlayer.SetRGB(100, 180, 255);
		ChatSystem.ColorClanPlayer.Clamp();

		if (!ChatSystem.ColorDirect)
			ChatSystem.ColorDirect = new SM_RGBColorConfig;
		if (!ChatSystem.ColorDirect.IsConfigured() || IsRgbColor(ChatSystem.ColorDirect, 224, 224, 224))
			ChatSystem.ColorDirect.SetRGB(255, 255, 255);
		ChatSystem.ColorDirect.Clamp();

		if (!ChatSystem.ColorDirectPlayer)
			ChatSystem.ColorDirectPlayer = new SM_RGBColorConfig;
		if (!ChatSystem.ColorDirectPlayer.IsConfigured() || IsRgbColor(ChatSystem.ColorDirectPlayer, 224, 224, 224))
			ChatSystem.ColorDirectPlayer.SetRGB(255, 255, 255);
		ChatSystem.ColorDirectPlayer.Clamp();

		if (!ChatSystem.ColorGlobal)
			ChatSystem.ColorGlobal = new SM_RGBColorConfig;
		if (!ChatSystem.ColorGlobal.IsConfigured() || IsRgbColor(ChatSystem.ColorGlobal, 224, 224, 224))
			ChatSystem.ColorGlobal.SetRGB(255, 255, 255);
		ChatSystem.ColorGlobal.Clamp();

		if (!ChatSystem.ColorGlobalPlayer)
			ChatSystem.ColorGlobalPlayer = new SM_RGBColorConfig;
		if (!ChatSystem.ColorGlobalPlayer.IsConfigured() || IsRgbColor(ChatSystem.ColorGlobalPlayer, 224, 224, 224))
			ChatSystem.ColorGlobalPlayer.SetRGB(232, 181, 69);
		ChatSystem.ColorGlobalPlayer.Clamp();

		if (!ChatSystem.ColorServer)
			ChatSystem.ColorServer = new SM_RGBColorConfig;
		if (!ChatSystem.ColorServer.IsConfigured())
			ChatSystem.ColorServer.SetRGB(255, 50, 50);
		ChatSystem.ColorServer.Clamp();

		if (!ChatSystem.ColorAlert)
			ChatSystem.ColorAlert = new SM_RGBColorConfig;
		if (!ChatSystem.ColorAlert.IsConfigured())
			ChatSystem.ColorAlert.SetRGB(100, 200, 255);
		ChatSystem.ColorAlert.Clamp();
	}

	protected bool IsRgbColor(SM_RGBColorConfig color, int r, int g, int b)
	{
		if (!color)
			return false;
		if (!color.IsConfigured())
			return false;
		if (color.R != r)
			return false;
		if (color.G != g)
			return false;
		if (color.B != b)
			return false;
		return true;
	}

	protected void ValidateChatStyleList(array<ref SM_ChatPlayerStyle> styles)
	{
		if (!styles)
			return;

		for (int i = styles.Count() - 1; i >= 0; i--)
		{
			SM_ChatPlayerStyle style = styles[i];
			if (!style)
			{
				styles.Remove(i);
				continue;
			}

			style.SteamId = style.SteamId.Trim();
			style.Prefix = style.Prefix.Trim();
			if (style.SteamId == "")
			{
				styles.Remove(i);
				continue;
			}

			if (style.Prefix.LengthUtf8() > 24)
				style.Prefix = style.Prefix.SubstringUtf8(0, 24);
			if (!style.NameColorRGB)
				style.NameColorRGB = new SM_RGBColorConfig;
			if (!style.TextColorRGB)
				style.TextColorRGB = new SM_RGBColorConfig;
			if (!style.PrefixColorRGB)
				style.PrefixColorRGB = new SM_RGBColorConfig;
			if (style.NameColorRGB.IsConfigured())
				style.NameColorRGB.Clamp();
			if (style.TextColorRGB.IsConfigured())
				style.TextColorRGB.Clamp();
			if (style.PrefixColorRGB.IsConfigured())
				style.PrefixColorRGB.Clamp();
		}
	}
}

class SM_PartyConfigLoader
{
	static const string DIR_PATH = "$profile:SM_PartyMod";
	static const string CONFIG_PATH = "$profile:SM_PartyMod\\Settings.json";
	static const int CONFIG_READ_FILE_LENGTH = 100000000;

	protected static ref SM_PartyConfig s_Config;

	static SM_PartyConfig Get()
	{
		if (!s_Config)
			Load();
		return s_Config;
	}

	static bool Reload(out string reloadMessage)
	{
		reloadMessage = "";

		if (!FileExist(DIR_PATH))
			MakeDirectory(DIR_PATH);

		SM_PartyConfig loadedConfig = new SM_PartyConfig();
		string errorMessage;
		if (FileExist(CONFIG_PATH))
		{
			if (IsGroupedConfig())
			{
				if (!JsonFileLoader<SM_PartyConfig>.LoadFile(CONFIG_PATH, loadedConfig, errorMessage))
				{
					reloadMessage = "#STR_SMP_00772" + errorMessage;
					return false;
				}
			}
			else
			{
				SM_PartyLegacyConfig legacy = new SM_PartyLegacyConfig();
				if (JsonFileLoader<SM_PartyLegacyConfig>.LoadFile(CONFIG_PATH, legacy, errorMessage))
				{
					loadedConfig.ImportLegacy(legacy);
				}
				else
				{
					reloadMessage = "#STR_SMP_00767" + errorMessage;
					return false;
				}
			}
		}

		if (!loadedConfig)
			loadedConfig = new SM_PartyConfig();

		loadedConfig.EnsureSections();
		loadedConfig.Validate();

		if (!JsonFileLoader<SM_PartyConfig>.SaveFile(CONFIG_PATH, loadedConfig, errorMessage))
		{
			reloadMessage = "#STR_SMP_00766" + errorMessage;
			return false;
		}

		s_Config = loadedConfig;
		reloadMessage = "#STR_SMP_00230";
		return true;
	}

	static void Load()
	{
		if (!FileExist(DIR_PATH))
			MakeDirectory(DIR_PATH);

		s_Config = new SM_PartyConfig();

		string errorMessage;
		if (FileExist(CONFIG_PATH))
		{
			if (IsGroupedConfig())
			{
				if (!JsonFileLoader<SM_PartyConfig>.LoadFile(CONFIG_PATH, s_Config, errorMessage))
				{
					ErrorEx(SM_PartyLoc.Text("#STR_SMP_00207" + errorMessage));
					s_Config = new SM_PartyConfig();
				}
			}
			else
			{
				SM_PartyLegacyConfig legacy = new SM_PartyLegacyConfig();
				if (JsonFileLoader<SM_PartyLegacyConfig>.LoadFile(CONFIG_PATH, legacy, errorMessage))
					s_Config.ImportLegacy(legacy);
				else
					ErrorEx(SM_PartyLoc.Text("#STR_SMP_00197" + errorMessage));
			}

			if (!s_Config)
				s_Config = new SM_PartyConfig();
		}

		s_Config.EnsureSections();
		s_Config.Validate();

		if (!JsonFileLoader<SM_PartyConfig>.SaveFile(CONFIG_PATH, s_Config, errorMessage))
			ErrorEx(SM_PartyLoc.Text("#STR_SMP_00191" + errorMessage));
	}

	protected static bool ReadConfigFile(out string fileContent)
	{
		fileContent = "";
		if (!FileExist(CONFIG_PATH))
			return false;

		FileHandle handle = OpenFile(CONFIG_PATH, FileMode.READ);
		if (handle == 0)
			return false;

		ReadFile(handle, fileContent, CONFIG_READ_FILE_LENGTH);
		CloseFile(handle);
		return true;
	}

	protected static bool IsGroupedConfig()
	{
		string fileContent;
		if (!ReadConfigFile(fileContent))
			return false;

		if (fileContent.Contains("\"General\""))
			return true;
		if (fileContent.Contains("\"Ranks\""))
			return true;
		if (fileContent.Contains("\"ChatSystem\""))
			return true;
		return false;
	}
}
