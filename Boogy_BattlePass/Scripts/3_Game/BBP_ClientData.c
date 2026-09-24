// Клиентское состояние боевого пропуска.
class BBP_ClientData
{
	static ref BBP_StateView State;
	static float ReceivedAt; // GetGame().GetTickTime() в момент синхронизации
	static ref ScriptInvoker OnStateChanged = new ScriptInvoker();
	static ref ScriptInvoker OnCaseOpened = new ScriptInvoker(); // (string caseName, string rewardText)

	static void OnRPC(int rpcType, ParamsReadContext ctx)
	{
		switch (rpcType)
		{
			case BBP_RPC.SYNC_STATE:
			{
				string json;
				if (!ctx.Read(json))
					return;

				BBP_StateView view = new BBP_StateView;
				JsonSerializer js = new JsonSerializer();
				string error;
				if (!js.ReadFromString(view, json, error))
				{
					Print("[Boogy_BattlePass] Ошибка разбора состояния: " + error);
					return;
				}

				State = view;
				ReceivedAt = GetGame().GetTickTime();
				BBP_Shared.PremiumItemClasses.Clear();
				BBP_Shared.PremiumItemClasses.InsertAll(view.PremiumItemClasses);
				OnStateChanged.Invoke();
				break;
			}
			case BBP_RPC.NOTIFY:
			{
				string title;
				string text;
				if (!ctx.Read(title) || !ctx.Read(text))
					return;
				NotificationSystem.AddNotificationExtended(4.0, title, text, "set:dayz_gui image:icon_info");
				break;
			}
			case BBP_RPC.CASE_RESULT:
			{
				string caseName;
				string rewardJson;
				if (!ctx.Read(caseName) || !ctx.Read(rewardJson))
					return;
				BBP_RewardView rewardView = new BBP_RewardView;
				JsonSerializer rewardJs = new JsonSerializer();
				string rewardError;
				string rewardText = "?";
				if (rewardJs.ReadFromString(rewardView, rewardJson, rewardError))
					rewardText = DescribeReward(rewardView);
				NotificationSystem.AddNotificationExtended(5.0, caseName, "Выпало: " + rewardText, "set:dayz_gui image:icon_info");
				OnCaseOpened.Invoke(caseName, rewardText);
				break;
			}
		}
	}

	// Сколько секунд осталось с учётом времени, прошедшего после синхронизации
	static int GetSecondsLeft()
	{
		if (!State)
			return 0;
		int elapsed = GetGame().GetTickTime() - ReceivedAt;
		return Math.Max(0, State.SecondsLeft - elapsed);
	}

	static int GetNextSeasonSeconds()
	{
		if (!State)
			return 0;
		int elapsed = GetGame().GetTickTime() - ReceivedAt;
		return Math.Max(0, State.NextSeasonSeconds - elapsed);
	}

	static string GetItemDisplayName(string className)
	{
		if (className == "")
			return "";

		string name;
		if (GetGame().ConfigGetText("CfgVehicles " + className + " displayName", name) && name != "")
			return Widget.TranslateString(name);
		if (GetGame().ConfigGetText("CfgWeapons " + className + " displayName", name) && name != "")
			return Widget.TranslateString(name);
		if (GetGame().ConfigGetText("CfgMagazines " + className + " displayName", name) && name != "")
			return Widget.TranslateString(name);
		return className;
	}

	static string DescribeReward(BBP_RewardView reward)
	{
		if (!reward)
			return "";

		string baseName = reward.DisplayName;
		switch (reward.Type)
		{
			case "Item":
				if (baseName == "")
					baseName = GetItemDisplayName(reward.ClassName);
				if (reward.Amount > 1)
					return baseName + " x" + reward.Amount;
				return baseName;
			case "Coins":
				if (baseName == "")
					baseName = "Монеты";
				return baseName + " +" + reward.Amount;
			case "Xp":
				if (baseName == "")
					baseName = "Опыт";
				return baseName + " +" + reward.Amount;
			case "Case":
				if (baseName == "")
					baseName = "Кейс";
				if (reward.Amount > 1)
					return baseName + " x" + reward.Amount;
				return baseName;
			case "Premium":
				if (baseName == "")
					baseName = "Премиум-пропуск";
				return baseName;
		}
		if (baseName == "")
			baseName = reward.ClassName;
		return baseName;
	}

	static void Request(int rpcType)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(player, rpcType, true, null);
	}

	static void SendClaim(int level, bool premium)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(level);
		rpc.Write(premium);
		rpc.Send(player, BBP_RPC.CLAIM_REWARD, true, null);
	}

	static void SendString(int rpcType, string value)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(value);
		rpc.Send(player, rpcType, true, null);
	}
}
