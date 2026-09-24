// Публичный API для интеграции с другими модами.
//
//   BBP_API.ReportEvent(player, "MyMod_AirdropLooted");        // прогресс заданий Type=Custom
//   BBP_API.AddXp(player, 250);
//   BBP_API.AddCoins("7656119...", 100);
//   BBP_API.GivePremium("7656119...");                         // текущий сезон
//   BBP_API.OnCustomReward.Insert(MyHandler);                   // награды Type=Custom
//
//   void MyHandler(PlayerBase player, string data, int amount) { ... }
class BBP_API
{
	// Вызывается при выдаче награды с Type = "Custom": (PlayerBase player, string data, int amount)
	static ref ScriptInvoker OnCustomReward = new ScriptInvoker();
	// Вызывается при выполнении задания: (PlayerBase player, string seasonId, string questId)
	static ref ScriptInvoker OnQuestCompleted = new ScriptInvoker();
	// Вызывается при повышении уровня: (PlayerBase player, string seasonId, int newLevel)
	static ref ScriptInvoker OnLevelUp = new ScriptInvoker();

	static void ReportEvent(PlayerBase player, string eventName, int amount = 1)
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.OnCustomEvent(player, eventName, amount);
	}

	static void AddXp(PlayerBase player, int amount)
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (manager && player && player.GetIdentity())
			manager.AdminAddXp(player.GetIdentity().GetPlainId(), amount);
	}

	static void AddXpByUid(string uid, int amount)
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.AdminAddXp(uid, amount);
	}

	static void AddCoins(string uid, int amount)
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.AdminAddCoins(uid, amount);
	}

	static void GiveCase(string uid, string caseId, int amount = 1)
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (manager)
			manager.AdminGiveCase(uid, caseId, amount);
	}

	static bool GivePremium(string uid, string seasonId = "")
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (!manager)
			return false;
		return manager.AdminGivePremium(uid, seasonId);
	}

	static bool HasPremium(string uid)
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (!manager)
			return false;
		return manager.HasPremium(uid);
	}

	static int GetLevel(string uid)
	{
		BBP_Manager manager = BBP_Manager.Get();
		if (!manager)
			return 0;
		return manager.GetPlayerLevel(uid);
	}
}
