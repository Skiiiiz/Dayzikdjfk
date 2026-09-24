// ============================================================
//  Данные игроков (сервер) — $profile:Boogy_BattlePass\Players\<uid>.json
// ============================================================

class BBP_QuestProgress
{
	string Id = "";
	int Value = 0;
	bool Completed = false;
	int PeriodKey = 0;
	// Visit: индексы уже посещённых точек (All/Route)
	ref array<int> VisitedPoints = new array<int>;
	// Visit Any: точка, в которой игрок находится сейчас (чтобы не засчитывать повторно, пока не выйдет)
	int InsidePoint = -1;
}

class BBP_SeasonProgress
{
	string SeasonId = "";
	int Xp = 0;
	bool Premium = false;
	ref array<int> ClaimedFree = new array<int>;
	ref array<int> ClaimedPremium = new array<int>;
	ref array<ref BBP_QuestProgress> Quests = new array<ref BBP_QuestProgress>;

	BBP_QuestProgress FindQuest(string id)
	{
		foreach (BBP_QuestProgress q : Quests)
		{
			if (q.Id == id)
				return q;
		}
		return null;
	}
}

class BBP_PlayerData
{
	string Uid = "";
	string Name = "";
	int Coins = 0;
	// Купленные/полученные, но ещё не открытые кейсы (Id кейсов)
	ref array<string> Cases = new array<string>;
	ref array<ref BBP_SeasonProgress> Seasons = new array<ref BBP_SeasonProgress>;

	BBP_SeasonProgress GetSeason(string seasonId, bool create)
	{
		foreach (BBP_SeasonProgress sp : Seasons)
		{
			if (sp.SeasonId == seasonId)
				return sp;
		}
		if (!create)
			return null;

		BBP_SeasonProgress created = new BBP_SeasonProgress;
		created.SeasonId = seasonId;
		Seasons.Insert(created);
		return created;
	}

	int CountCases(string caseId)
	{
		int n = 0;
		foreach (string id : Cases)
		{
			if (id == caseId)
				n++;
		}
		return n;
	}
}

class BBP_DonationHistory
{
	ref array<string> ProcessedIds = new array<string>;
}

// Ответ донат-API (см. Docs/DonateApi.md)
class BBP_DonateEntry
{
	string Id = "";
	string Uid = "";
	// Premium | Coins | Xp | Case
	string Type = "";
	int Amount = 1;
	string Data = "";
}

class BBP_DonateResponse
{
	ref array<ref BBP_DonateEntry> Entries = new array<ref BBP_DonateEntry>;
}

class BBP_DonateAck
{
	string ServerId = "";
	string ApiKey = "";
	ref array<string> Ids = new array<string>;
}

// ============================================================
//  Представление для клиента (передаётся JSON-строкой по RPC)
// ============================================================

class BBP_RewardView
{
	string Type = "";
	string ClassName = "";
	int Amount = 0;
	string DisplayName = "";
	float Chance = 0; // для кейсов, в процентах
}

class BBP_LevelView
{
	int Level;
	int XpFrom;
	int XpTo;
	bool FreeClaimed;
	bool PremiumClaimed;
	ref array<ref BBP_RewardView> Free = new array<ref BBP_RewardView>;
	ref array<ref BBP_RewardView> Premium = new array<ref BBP_RewardView>;
}

class BBP_QuestView
{
	string Id;
	string Name;
	string Description;
	string Type;
	string Period;
	int Value;
	int Count;
	int Xp;
	int Coins;
	bool Completed;
	bool PremiumOnly;
	bool Locked;
	ref array<ref BBP_RewardView> Rewards = new array<ref BBP_RewardView>;
}

class BBP_CaseView
{
	string Id;
	string Name;
	string Description;
	int Price;
	string Currency;
	string CurrencyItem;
	bool PremiumOnly;
	int Owned;
	ref array<ref BBP_RewardView> Rewards = new array<ref BBP_RewardView>;
}

class BBP_StateView
{
	bool Enabled;
	bool HasSeason;
	bool SeasonEnded;
	string SeasonId;
	string SeasonName;
	string SeasonDescription;
	int SecondsLeft;
	string NextSeasonName;
	int NextSeasonSeconds;
	int Xp;
	int Level;
	int MaxLevel;
	int LevelXpFrom;
	int LevelXpTo;
	bool Premium;
	int Coins;
	int UnclaimedRewards;
	bool IsAdmin;
	bool ShopEnabled;
	ref array<ref BBP_LevelView> Levels = new array<ref BBP_LevelView>;
	ref array<ref BBP_QuestView> Quests = new array<ref BBP_QuestView>;
	ref array<ref BBP_CaseView> Cases = new array<ref BBP_CaseView>;
	ref array<string> PremiumItemClasses = new array<string>;
}

class BBP_Json
{
	static string Write(Class obj)
	{
		JsonSerializer js = new JsonSerializer();
		string result;
		js.WriteToString(obj, false, result);
		return result;
	}
}
