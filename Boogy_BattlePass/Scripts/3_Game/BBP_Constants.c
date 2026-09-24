class BBP_RPC
{
	static const int RANGE_START       = 887900;
	static const int RANGE_END         = 887949;

	// Клиент -> сервер
	static const int REQUEST_STATE     = 887901;
	static const int CLAIM_REWARD      = 887902; // int level, bool premium
	static const int CLAIM_ALL         = 887903;
	static const int SHOP_BUY          = 887904; // string caseId
	static const int CASE_OPEN         = 887905; // string caseId
	static const int ADMIN_RELOAD      = 887906;
	static const int ADMIN_COMMAND     = 887907; // string command
	static const int LAST_CLIENT_RPC   = 887929;

	// Сервер -> клиент
	static const int SYNC_STATE        = 887930; // string json (BBP_StateView)
	static const int NOTIFY            = 887931; // string title, string text
	static const int CASE_RESULT       = 887932; // string caseName, string rewardJson (BBP_RewardView)
}

const int BBP_MENU_ID = 78890;

class BBP_QuestType
{
	static const string KILL   = "Kill";   // убийства зомби / животных / игроков
	static const string ACTION = "Action"; // любое игровое действие (лечение, костры, строительство, еда…)
	static const string CRAFT  = "Craft";  // крафт предметов по рецептам
	static const string VISIT  = "Visit";  // посещение точек, зон и маршрутов
	static const string CUSTOM = "Custom"; // пользовательские события других модов
}

class BBP_KillKind
{
	static const string ANY    = "Any";
	static const string ZOMBIE = "Zombie";
	static const string ANIMAL = "Animal";
	static const string PLAYER = "Player";
}

class BBP_VisitMode
{
	static const string ANY   = "Any";   // каждое посещение любой точки = +1
	static const string ALL   = "All";   // посетить все точки в любом порядке
	static const string ROUTE = "Route"; // посетить точки строго по порядку
}

class BBP_Period
{
	static const string SEASON = "Season";
	static const string DAILY  = "Daily";
	static const string WEEKLY = "Weekly";
}

class BBP_RewardType
{
	static const string ITEM    = "Item";
	static const string COINS   = "Coins";
	static const string XP      = "Xp";
	static const string CASE    = "Case";
	static const string PREMIUM = "Premium";
	static const string CUSTOM  = "Custom";
}

class BBP_Currency
{
	static const string COINS = "Coins";
	static const string ITEM  = "Item";
}

class BBP_Paths
{
	static const string DIR            = "$profile:Boogy_BattlePass";
	static const string SETTINGS       = "$profile:Boogy_BattlePass\\Settings.json";
	static const string SHOP           = "$profile:Boogy_BattlePass\\Shop.json";
	static const string SEASONS_DIR    = "$profile:Boogy_BattlePass\\Seasons";
	static const string PLAYERS_DIR    = "$profile:Boogy_BattlePass\\Players";
	static const string DONATIONS      = "$profile:Boogy_BattlePass\\Donations.json";
}

// Общие для клиента и сервера данные (на клиенте приходят с синхронизацией).
class BBP_Shared
{
	static ref array<string> PremiumItemClasses = new array<string>;

	static bool IsPremiumItemClass(string className)
	{
		if (className == "" || !PremiumItemClasses)
			return false;

		foreach (string cls : PremiumItemClasses)
		{
			if (cls == "")
				continue;
			if (cls == className || GetGame().IsKindOf(className, cls))
				return true;
		}
		return false;
	}
}
