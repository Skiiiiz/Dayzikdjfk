class SM_PartyRPC
{
	static const int RANGE_START       = 887700;
	static const int RANGE_END         = 887799;

	static const int CREATE_CLAN        = 887701;
	static const int INVITE             = 887702;
	static const int ACCEPT_INVITE      = 887703;
	static const int DECLINE_INVITE     = 887704;
	static const int KICK               = 887705;
	static const int PROMOTE            = 887706;
	static const int DEMOTE             = 887707;
	static const int LEAVE              = 887708;
	static const int DISBAND            = 887709;
	static const int REQUEST_STATE      = 887710;
	static const int REQUEST_CLAN_LIST  = 887711;
	static const int REQUEST_PLAYERS    = 887712;
	static const int SET_COLOR          = 887713;
	static const int CHAT_SEND          = 887714;
	static const int MARK_PLACE         = 887715;
	static const int SET_DESCRIPTION    = 887716;
	static const int REQUEST_CLAN_INFO  = 887717;
	static const int APPLY_TO_CLAN      = 887718;
	static const int APPLICATION_ACCEPT = 887719;
	static const int APPLICATION_DECLINE = 887720;
	static const int REQUEST_TOPS       = 887721;
	static const int TREASURY_DEPOSIT   = 887722;
	static const int TREASURY_WITHDRAW  = 887723;
	static const int MARKET_LIST        = 887724;
	static const int MARKET_SELL        = 887725;
	static const int MARKET_BUY         = 887726;
	static const int MARKET_CANCEL      = 887727;
	static const int MAP_MARKER_ADD     = 887728;
	static const int MAP_MARKER_REMOVE  = 887729;
	static const int CLAN_UPGRADE       = 887730;
	static const int STORAGE_DEPOSIT    = 887731;
	static const int STORAGE_TAKE       = 887732;
	static const int REQUEST_STORAGE    = 887733;
	static const int SET_PERMISSION     = 887734;
	static const int SET_CLAN_WEBHOOK   = 887735;
	static const int CLEAR_CLAN_WEBHOOK = 887736;
	static const int TEST_CLAN_WEBHOOK  = 887737;
	static const int ADMIN_ENTER_CLAN   = 887738;
	static const int ADMIN_LEAVE_CLAN   = 887739;
	static const int TOGGLE_CLAN_AUDIT_WEBHOOK = 887740;
	static const int MARK_CLEAR         = 887741;
	static const int HUD_ACTIVITY_STATUS = 887742;
	static const int SERVER_MARKET_LIST = 887743;
	static const int SERVER_MARKET_BUY  = 887744;
	static const int AUCTION_LIST       = 887745;
	static const int AUCTION_SELL       = 887746;
	static const int AUCTION_BID        = 887747;
	static const int AUCTION_CANCEL     = 887748;
	static const int ACHIEVEMENTS       = 887749;

	static const int SYNC_STATE        = 887750;
	static const int SYNC_CLAN_LIST    = 887751;
	static const int SYNC_PLAYERS      = 887752;
	static const int HUD_UPDATE       = 887753;
	static const int MARK_SYNC         = 887754;
	static const int CLAN_INFO         = 887755;
	static const int SYNC_TOPS         = 887756;
	static const int SYNC_MARKET       = 887757;
	static const int SYNC_STORAGE      = 887758;
	static const int CHAT_RECEIVE      = 887759;
	static const int MARK_CLEAR_SYNC   = 887760;
	static const int SYNC_SERVER_MARKET = 887761;
	static const int SYNC_AUCTION      = 887762;
	static const int SYNC_ACHIEVEMENTS = 887763;
	static const int ADMIN_RELOAD_CONFIG = 887764;
	static const int REQUEST_PLAYER_TITLES = 887765;
	static const int SYNC_PLAYER_TITLES = 887766;
	static const int CONTRACTS        = 887767;
	static const int SYNC_CONTRACTS   = 887768;
	static const int SYNC_CONTRACT_TARGET_PREVIEW = 887769;
	static const int ADMIN_CHAT_COMMAND = 887770;
	static const int SYNC_CHAT_MUTE   = 887771;
	static const int REQUEST_ADMIN_CHAT_LOG = 887772;
	static const int SYNC_ADMIN_CHAT_LOG = 887773;
	static const int ADMIN_CHAT_MUTE = 887774;
	static const int ADMIN_CHAT_UNMUTE = 887775;
	static const int REQUEST_ONLINE_PLAYERS = 887776;
	static const int SYNC_ONLINE_PLAYERS = 887777;
	static const int MAP_MARKER_UPDATE = 887778;
	static const int LOCAL_NOTIFY = 887779;
	static const int PERSONAL_DEATH_MARKER = 887780;
	static const int MARK_ITEM = 887781;
}

class SM_PartyAdminCommands
{
	static bool MatchesCommand(string text, string command)
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
		string command = text;
		command = command.Trim();
		command.ToLower();

		if (command == "!smpreload")
			return true;
		if (command == "!partyreload")
			return true;
		if (command == "!smreload")
			return true;
		if (command == "/smpreload")
			return true;
		if (command == "/partyreload")
			return true;
		if (command == "/smreload")
			return true;
		if (command == "#smpreload")
			return true;
		if (command == "#partyreload")
			return true;
		if (command == "#smreload")
			return true;
		return false;
	}

	static bool IsChatAdminCommand(string text)
	{
		if (MatchesCommand(text, "!mute"))
			return true;
		if (MatchesCommand(text, "/mute"))
			return true;
		if (MatchesCommand(text, "#mute"))
			return true;
		if (MatchesCommand(text, "!chatmute"))
			return true;
		if (MatchesCommand(text, "/chatmute"))
			return true;
		if (MatchesCommand(text, "#chatmute"))
			return true;
		if (MatchesCommand(text, "!unmute"))
			return true;
		if (MatchesCommand(text, "/unmute"))
			return true;
		if (MatchesCommand(text, "#unmute"))
			return true;
		if (MatchesCommand(text, "!chatunmute"))
			return true;
		if (MatchesCommand(text, "/chatunmute"))
			return true;
		if (MatchesCommand(text, "#chatunmute"))
			return true;
		if (MatchesCommand(text, "!mutelist"))
			return true;
		if (MatchesCommand(text, "/mutelist"))
			return true;
		if (MatchesCommand(text, "#mutelist"))
			return true;
		if (MatchesCommand(text, "!mutes"))
			return true;
		if (MatchesCommand(text, "/mutes"))
			return true;
		if (MatchesCommand(text, "#mutes"))
			return true;
		return false;
	}
}

class SM_AchievementScope
{
	static const int PERSONAL = 0;
	static const int CLAN = 1;
}

class SM_AchievementRPCAction
{
	static const int REQUEST = 0;
	static const int CLAIM = 1;
}

class SM_ContractRPCAction
{
	static const int REQUEST = 0;
	static const int CREATE_KILL = 1;
	static const int CREATE_ITEM = 2;
	static const int ACCEPT = 3;
	static const int ABANDON = 4;
	static const int TURN_IN_ITEM = 5;
	static const int REQUEST_TARGET_PREVIEW = 6;
}

class SM_ContractType
{
	static const int KILL = 0;
	static const int ITEM = 1;

	static string GetName(int type)
	{
		if (type == KILL)
			return "#STR_SMP_00762";
		if (type == ITEM)
			return "#STR_SMP_00792";
		return "#STR_SMP_00575";
	}
}

class SM_ContractStatus
{
	static const int OPEN = 0;
	static const int ACCEPTED = 1;
	static const int COMPLETED = 2;
	static const int ABANDONED = 3;
	static const int EXPIRED = 4;
	static const int CANCELLED = 5;

	static string GetName(int status)
	{
		if (status == OPEN)
			return "#STR_SMP_00753";
		if (status == ACCEPTED)
			return "#STR_SMP_00843";
		if (status == COMPLETED)
			return "#STR_SMP_00377";
		if (status == ABANDONED)
			return "#STR_SMP_00750";
		if (status == EXPIRED)
			return "#STR_SMP_00876";
		if (status == CANCELLED)
			return "#STR_SMP_00759";
		return "#STR_SMP_00696";
	}
}

class SM_ChatChannel
{
	static const int CLAN   = 0;
	static const int LOCAL  = 1;
	static const int GLOBAL = 2;
	static const int SERVER = 3;
}

class SM_ClanPingTargetType
{
	static const int ITEM = 0;
	static const int VEHICLE = 1;
	static const int CORPSE = 2;

	static int Normalize(int pingType)
	{
		if (pingType == VEHICLE)
			return VEHICLE;
		if (pingType == CORPSE)
			return CORPSE;
		return ITEM;
	}
}

class SM_CFToolsChat
{
	static const string PREFIX = ">";

	static bool ContainsTechnicalPrefix(string text)
	{
		if (text == "")
			return false;
		string t = text;
		t = t.Trim();
		if (t.IndexOf(PREFIX) == 0)
			return true;
		return false;
	}

	static string BuildMirrorMessage(int channel, string text)
	{
		string logText = text;
		logText = logText.Trim();
		if (logText == "")
			return "";

		return PREFIX + " " + logText;
	}
}

class SM_ChatChannelPalette
{
	static int GetBackgroundColor(int channel)
	{
		return ARGB(255, 22, 19, 16);
	}

	static int GetBorderColor(int channel)
	{
		return ARGB(255, 58, 49, 40);
	}

	static int GetTextColor(int channel)
	{
		return ARGB(255, 236, 228, 216);
	}

	static int GetAccentColor(int channel)
	{
		if (channel == SM_ChatChannel.GLOBAL)
			return ARGB(255, 210, 171, 133);
		if (channel == SM_ChatChannel.LOCAL)
			return ARGB(255, 232, 181, 69);
		if (channel == SM_ChatChannel.CLAN)
			return ARGB(255, 184, 115, 51);

		return ARGB(255, 179, 166, 146);
	}
}

// Разбор ванильных сообщений чата (перехватываются клиентом и выводятся
// в постоянное окно мода вместо стандартного игрового чата).
class SM_VanillaChat
{
	// Готовит метку канала и цвета для перехваченного сообщения.
	// channel — битовая маска CC* (CCSystem=1, CCAdmin=2, CCDirect=4,
	// CCMegaphone=8, CCTransmitter=16, CCPublicAddressSystem=32, CCBattlEye=64).
	static void Classify(int channel, out string label, out int labelColor, out int textColor)
	{
		int soft  = SM_ClanClientData.ChatColorServer; // сервер/система
		if (soft == 0)
			soft = ARGB(255, 179, 166, 146);
		int alert = SM_ClanClientData.ChatColorAlert;
		if (alert == 0)
			alert = ARGB(255, 232, 181, 69);
		int direct = SM_ClanClientData.ChatColorDirect;
		if (direct == 0)
			direct = ARGB(255, 236, 228, 216);
		int global = SM_ClanClientData.ChatColorGlobal;
		if (global == 0)
			global = ARGB(255, 210, 171, 133);
		int light = ARGB(255, 236, 228, 216); // светлый — текст

		labelColor = soft;
		textColor = light;
		label = "#STR_SMP_00909";

		if (channel & CCAdmin)
		{
			label = "#STR_SMP_00240";
			labelColor = alert;
			textColor = light;
			return;
		}
		if (channel & CCTransmitter)
		{
			label = "#STR_SMP_00888";
			labelColor = global;
			textColor = light;
			return;
		}
		if (channel & CCDirect || channel & CCMegaphone || channel & CCPublicAddressSystem)
		{
			label = "#STR_SMP_00384";
			labelColor = direct;
			textColor = direct;
			return;
		}
		if (channel & CCSystem || channel & CCBattlEye)
		{
			label = "#STR_SMP_00909";
			labelColor = soft;
			textColor = light;
			return;
		}

		// channel == 0 — локальное сообщение самому себе (уведомления мода)
		label = "#STR_SMP_00917";
		labelColor = soft;
		textColor = light;
	}

	static string NormalizeAuthor(string label, string author)
	{
		string result = author;
		result = result.Trim();
		if (result == "")
			return "";

		string upper = result;
		upper.ToUpper();
		if (label == "#STR_SMP_00909" || label == "#STR_SMP_00917")
		{
			if (upper == "[SERVER]" || upper == "SERVER" || upper == "[СЕРВЕР]" || upper == "СЕРВЕР")
				return "";
		}

		return result;
	}
}

class SM_TopCategory
{
	static const int PLAYER_KILLS = 0;
	static const int ZOMBIE_KILLS = 1;
	static const int ONLINE_TIME  = 2;
	static const int BEST_LIFE    = 3;
	static const int DISTANCE     = 4;
	static const int ACCURACY     = 5;
	static const int HEADSHOTS    = 6;
	static const int KILL_DEATH   = 7;
	static const int LONGEST_PLAYER_KILL = 8;

	static const int COUNT = 9;

	static string GetConfigName(int category)
	{
		switch (category)
		{
			case PLAYER_KILLS: return "PlayerKills";
			case ZOMBIE_KILLS: return "ZombieKills";
			case ONLINE_TIME:  return "OnlineTime";
			case BEST_LIFE:    return "BestLife";
			case DISTANCE:     return "Distance";
			case ACCURACY:     return "Accuracy";
			case HEADSHOTS:    return "Headshots";
			case KILL_DEATH:   return "KillDeath";
			case LONGEST_PLAYER_KILL: return "LongestPlayerKill";
		}
		return "";
	}

	static int GetByConfigName(string categoryName)
	{
		categoryName.ToLower();
		if (categoryName == "playerkills")
			return PLAYER_KILLS;
		if (categoryName == "zombiekills")
			return ZOMBIE_KILLS;
		if (categoryName == "onlinetime")
			return ONLINE_TIME;
		if (categoryName == "bestlife")
			return BEST_LIFE;
		if (categoryName == "distance")
			return DISTANCE;
		if (categoryName == "accuracy")
			return ACCURACY;
		if (categoryName == "headshots")
			return HEADSHOTS;
		if (categoryName == "killdeath")
			return KILL_DEATH;
		if (categoryName == "longestplayerkill")
			return LONGEST_PLAYER_KILL;
		return -1;
	}

	static string GetName(int category)
	{
		switch (category)
		{
			case PLAYER_KILLS: return "#STR_SMP_01034";
			case ZOMBIE_KILLS: return "#STR_SMP_01033";
			case ONLINE_TIME:  return "#STR_SMP_00726";
			case BEST_LIFE:    return "#STR_SMP_00431";
			case DISTANCE:     return "#STR_SMP_00867";
			case ACCURACY:     return "#STR_SMP_01013";
			case HEADSHOTS:    return "#STR_SMP_00812";
			case KILL_DEATH:   return "K/D";
			case LONGEST_PLAYER_KILL: return "#STR_SMP_00393";
		}
		return "?";
	}

	static string GetShortName(int category)
	{
		switch (category)
		{
			case PLAYER_KILLS: return "#STR_SMP_00470";
			case ZOMBIE_KILLS: return "#STR_SMP_00462";
			case ONLINE_TIME:  return "#STR_SMP_00726";
			case BEST_LIFE:    return "#STR_SMP_00430";
			case DISTANCE:     return "#STR_SMP_01072";
			case ACCURACY:     return "#STR_SMP_01011";
			case HEADSHOTS:    return "#STR_SMP_01046";
			case KILL_DEATH:   return "K/D";
			case LONGEST_PLAYER_KILL: return "#STR_SMP_00390";
		}
		return "?";
	}
}

const int SM_PARTY_MENU_ID = 78861;
const int SM_PARTY_CHAT_MENU_ID = 78862;

class SM_ClanAction
{
	static const int INVITE            = 0;
	static const int KICK              = 1;
	static const int APPLICATIONS      = 2;
	static const int MARKET_SELL       = 3;
	static const int TREASURY_WITHDRAW = 4;
	static const int STORAGE_TAKE      = 5;
	static const int MAP_MARKERS       = 6;
	static const int DESCRIPTION       = 7;
	static const int COLOR             = 8;
	static const int UPGRADE           = 9;
	static const int SET_BASE          = 10;
	static const int MANAGE_WEBHOOK    = 11;

	static const int COUNT             = 12;

	static string GetName(int action)
	{
		switch (action)
		{
			case INVITE:            return "#STR_SMP_00837";
			case KICK:              return "#STR_SMP_00479";
			case APPLICATIONS:      return "#STR_SMP_00890";
			case MARKET_SELL:       return "#STR_SMP_00380";
			case TREASURY_WITHDRAW: return "#STR_SMP_00951";
			case STORAGE_TAKE:      return "#STR_SMP_00282";
			case MAP_MARKERS:       return "#STR_SMP_00632";
			case DESCRIPTION:       return "#STR_SMP_00635";
			case COLOR:             return "#STR_SMP_00637";
			case UPGRADE:           return "#STR_SMP_00875";
			case SET_BASE:          return "#STR_SMP_00832";
			case MANAGE_WEBHOOK:    return "#STR_SMP_01039";
		}
		return "?";
	}
}

class SM_MemberStatus
{
	static const int UNCONSCIOUS = 1;
	static const int BLEEDING    = 2;
	static const int IN_VEHICLE  = 4;
	static const int TYPING_CHAT = 8;
	static const int VIEWING_MAP = 16;
}

class SM_ClanColors
{
	static int Count()
	{
		return 8;
	}

	static int GetColor(int index)
	{
		switch (index)
		{
			case 1: return ARGB(255, 224, 50, 50);
			case 2: return ARGB(255, 235, 140, 30);
			case 3: return ARGB(255, 235, 210, 50);
			case 4: return ARGB(255, 80, 200, 80);
			case 5: return ARGB(255, 60, 200, 210);
			case 6: return ARGB(255, 80, 140, 245);
			case 7: return ARGB(255, 190, 90, 230);
		}
		return ARGB(255, 235, 235, 235);
	}

	static int ResolveColor(int value)
	{
		if (value >= 0 && value < Count())
			return GetColor(value);
		if (value >= 0 && value <= 16777215)
		{
			int red = (value >> 16) & 0xFF;
			int green = (value >> 8) & 0xFF;
			int blue = value & 0xFF;
			return ARGB(255, red, green, blue);
		}
		return value;
	}

	static int ClampChannel(int value)
	{
		if (value < 0)
			return 0;
		if (value > 255)
			return 255;
		return value;
	}

	static int FromRGB(int red, int green, int blue)
	{
		red = ClampChannel(red);
		green = ClampChannel(green);
		blue = ClampChannel(blue);
		return ARGB(255, red, green, blue);
	}

	static int NormalizeMarkerColor(int value)
	{
		if (value >= 0 && value < Count())
			return value;
		return ResolveColor(value);
	}

	static int GetRed(int value)
	{
		int color = ResolveColor(value);
		return (color >> 16) & 0xFF;
	}

	static int GetGreen(int value)
	{
		int color = ResolveColor(value);
		return (color >> 8) & 0xFF;
	}

	static int GetBlue(int value)
	{
		int color = ResolveColor(value);
		return color & 0xFF;
	}
}

class SM_ClanMarkerIcon
{
	static string GetPath()
	{
		return "SM_PartyMod\\GUI\\pings\\ping.paa";
	}
}

class SM_MapMarkerIconPath
{
	static string GetSeparator()
	{
		string markerPath = SM_ClanMarkerIcon.GetPath();
		int guiIndex = markerPath.IndexOf("GUI");
		if (guiIndex > 0)
		{
			return markerPath.Substring(guiIndex - 1, 1);
		}

		return "/";
	}

	static string Normalize(string iconPath, string fallback = "")
	{
		string path = iconPath;
		path = path.Trim();
		string separator = GetSeparator();
		path.Replace("/", separator);
		if (fallback == "")
		{
			fallback = SM_ClanMarkerIcon.GetPath();
		}
		fallback.Replace("/", separator);

		string doubleSeparator = separator + separator;
		while (path.Length() > 1 && path.Substring(0, 2) == doubleSeparator)
		{
			path = path.Substring(1, path.Length() - 1);
		}

		if (path == "")
		{
			path = fallback;
		}

		return path;
	}

	static string ForMapWidget(string iconPath, string fallback = "")
	{
		string path = Normalize(iconPath, fallback);
		string lower = path;
		lower.ToLower();
		string separator = GetSeparator();
		if (lower.IndexOf("dz" + separator) == 0)
		{
			path = separator + path;
		}

		return path;
	}

	static string ForImageWidget(string iconPath, string fallback = "")
	{
		if (fallback == "")
		{
			fallback = SM_ClanMarkerIcon.GetPath();
		}

		string path = Normalize(iconPath, fallback);
		string separator = GetSeparator();
		while (path.Length() > 1 && path.Substring(0, 1) == separator)
		{
			path = path.Substring(1, path.Length() - 1);
		}
		if (path == separator)
		{
			path = "";
		}

		if (path == "")
		{
			path = fallback;
			while (path.Length() > 1 && path.Substring(0, 1) == separator)
			{
				path = path.Substring(1, path.Length() - 1);
			}
			if (path == separator)
			{
				path = "";
			}
		}

		return path;
	}
}

class SM_PlayerMarkerIcon
{
	static string GetPath()
	{
		return "SM_PartyMod\\GUI\\pings\\player.paa";
	}
}

// Набор иконок, доступных игроку для пользовательских маркеров на карте (личные и клановые).
// Индекс хранится в данных маркера; 0 — стандартная метка.
class SM_MapMarkerIconSet
{
	static int Count()
	{
		return 12;
	}

	static int DeathIcon()
	{
		return 11;
	}

	static int Normalize(int index)
	{
		if (index < 0 || index >= Count())
			return 0;
		return index;
	}

	static string GetPath(int index)
	{
		switch (Normalize(index))
		{
			case 1: return "SM_PartyMod\\GUI\\pings\\base.paa";
			case 2: return "SM_PartyMod\\GUI\\pings\\danger.paa";
			case 3: return "SM_PartyMod\\GUI\\pings\\enemy.paa";
			case 4: return "SM_PartyMod\\GUI\\pings\\loot.paa";
			case 5: return "SM_PartyMod\\GUI\\pings\\medical.paa";
			case 6: return "SM_PartyMod\\GUI\\pings\\vehicle.paa";
			case 7: return "SM_PartyMod\\GUI\\pings\\fuel.paa";
			case 8: return "SM_PartyMod\\GUI\\pings\\cardrop.paa";
			case 9: return "SM_PartyMod\\GUI\\pings\\airdrop.paa";
			case 10: return "SM_PartyMod\\GUI\\pings\\convoy.paa";
			case 11: return "SM_PartyMod\\GUI\\pings\\dead.paa";
		}
		return "SM_PartyMod\\GUI\\pings\\ping.paa";
	}

	static string GetNameKey(int index)
	{
		switch (Normalize(index))
		{
			case 1: return "#STR_SMP_01088";
			case 2: return "#STR_SMP_01089";
			case 3: return "#STR_SMP_01090";
			case 4: return "#STR_SMP_01091";
			case 5: return "#STR_SMP_01092";
			case 6: return "#STR_SMP_01093";
			case 7: return "#STR_SMP_01094";
			case 8: return "#STR_SMP_01096";
			case 9: return "#STR_SMP_01097";
			case 10: return "#STR_SMP_01098";
			case 11: return "#STR_SMP_01099";
		}
		return "#STR_SMP_01087";
	}
}

class SM_PartyUtil
{
	static const string LATIN_ALNUM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	static const string HEX_CHARS = "0123456789ABCDEF";

	static bool IsLatinAlnum(string s)
	{
		for (int i = 0; i < s.Length(); i++)
		{
			if (!LATIN_ALNUM.Contains(s.Get(i)))
				return false;
		}
		return true;
	}

	// Азимут "from -> to" в градусах, 0..360, 0 = север (+Z).
	static float BearingDegrees(vector from, vector to)
	{
		float dx = to[0] - from[0];
		float dz = to[2] - from[2];
		float angle = Math.Atan2(dx, dz) * Math.RAD2DEG;
		if (angle < 0)
			angle += 360;
		return angle;
	}

	static string CompassDir(vector from, vector to)
	{
		float angle = BearingDegrees(from, to);

		if (angle >= 337.5 || angle < 22.5) return "#STR_SMP_00895";
		if (angle < 67.5) return "#STR_SMP_00898";
		if (angle < 112.5) return "#STR_SMP_00285";
		if (angle < 157.5) return "#STR_SMP_01085";
		if (angle < 202.5) return "#STR_SMP_01084";
		if (angle < 247.5) return "#STR_SMP_01086";
		if (angle < 292.5) return "#STR_SMP_00436";
		return "#STR_SMP_00914";
	}

	static string FormatTwoDigits(int value)
	{
		if (value < 10)
			return "0" + value.ToString();
		return value.ToString();
	}

	// Клиентский таймстамп HH:MM для перехваченных/системных строк чата.
	static string ChatStamp()
	{
		int hour;
		int minute;
		int second;
		GetHourMinuteSecond(hour, minute, second);
		return FormatTwoDigits(hour) + ":" + FormatTwoDigits(minute);
	}

	static string FormatDistance(float meters)
	{
		if (meters >= 1000)
		{
			int km10 = Math.Round(meters / 100);
			float km = km10 / 10.0;
			return km.ToString() + "#STR_SMP_00568";
		}
		int m = Math.Round(meters);
		return m.ToString() + "#STR_SMP_00625";
	}

	static string FormatDuration(int seconds)
	{
		if (seconds < 0)
			seconds = 0;
		int days = seconds / 86400;
		int hours = (seconds % 86400) / 3600;
		int minutes = (seconds % 3600) / 60;
		if (days > 0)
			return days.ToString() + "#STR_SMP_00389" + hours.ToString() + "#STR_SMP_01061";
		if (hours > 0)
			return hours.ToString() + "#STR_SMP_01062" + minutes.ToString() + "#STR_SMP_00058";
		return minutes.ToString() + "#STR_SMP_00058";
	}

	static string FormatPercent(int part, int total)
	{
		if (total <= 0)
			return "0%";
		int tenths = Math.Round(part * 1000.0 / total);
		int whole = tenths / 10;
		int frac = tenths % 10;
		return whole.ToString() + "." + frac.ToString() + "%";
	}

	static string GetItemDisplayName(string className)
	{
		string name;
		if (GetGame().ConfigGetText("CfgVehicles " + className + " displayName", name) && name != "")
			return Widget.TranslateString(name);
		if (GetGame().ConfigGetText("CfgMagazines " + className + " displayName", name) && name != "")
			return Widget.TranslateString(name);
		if (GetGame().ConfigGetText("CfgWeapons " + className + " displayName", name) && name != "")
			return Widget.TranslateString(name);
		return className;
	}

	static string NormalizeColorString(string value)
	{
		string color = value;
		color = color.Trim();
		if (color == "")
			return "";

		if (color.Length() > 0 && color.Substring(0, 1) == "#")
			color = color.Substring(1, color.Length() - 1);

		color.ToUpper();
		if (color.Length() >= 2 && color.Substring(0, 2) == "0X")
			color = color.Substring(2, color.Length() - 2);

		if (color.Length() == 6)
			color = "FF" + color;
		if (color.Length() != 8)
			return "";

		for (int i = 0; i < color.Length(); i++)
		{
			if (!HEX_CHARS.Contains(color.Get(i)))
				return "";
		}
		return color;
	}

	static int ParseChatColor(string value, int fallback)
	{
		string normalized = NormalizeColorString(value);
		if (normalized == "")
			return fallback;

		string hex = "0x" + normalized;
		return hex.HexToInt();
	}
}
