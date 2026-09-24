# Boogy_BattlePass — интеграция и администрирование

## Команды администратора (чат)

Доступны игрокам из `AdminIds`. Вместо SteamID можно писать `me`.

| Команда | Действие |
| --- | --- |
| `/bp reload` | Перечитать Settings.json, Shop.json и все сезоны |
| `/bp xp <uid> <n>` | Добавить (или отнять, если n < 0) опыт в текущем сезоне |
| `/bp coins <uid> <n>` | Добавить монеты |
| `/bp premium <uid> [seasonId]` | Выдать премиум |
| `/bp case <uid> <caseId> [n]` | Выдать кейсы |
| `/bp reset <uid>` | Сбросить прогресс игрока в текущем сезоне |
| `/bp event <name> [n]` | Отправить себе пользовательское событие (для проверки заданий `Custom`) |

## API для других модов (4_World и выше, только сервер)

```c
// Прогресс заданий Type = "Custom"
BBP_API.ReportEvent(player, "AirdropLooted");
BBP_API.ReportEvent(player, "TraderSold", 5);

// Начисления
BBP_API.AddXp(player, 250);
BBP_API.AddXpByUid(uid, 250);
BBP_API.AddCoins(uid, 100);
BBP_API.GiveCase(uid, "case_weapon", 1);
BBP_API.GivePremium(uid);            // текущий сезон
BBP_API.GivePremium(uid, "season_2");

// Запросы
bool premium = BBP_API.HasPremium(uid);
int level = BBP_API.GetLevel(uid);
```

### События

```c
// Награда Type = "Custom": (PlayerBase player, string data, int amount)
BBP_API.OnCustomReward.Insert(OnBattlePassCustomReward);

// Задание выполнено: (PlayerBase player, string seasonId, string questId)
BBP_API.OnQuestCompleted.Insert(OnBattlePassQuest);

// Новый уровень: (PlayerBase player, string seasonId, int level)
BBP_API.OnLevelUp.Insert(OnBattlePassLevelUp);
```

Пример — выдача денег трейдера наградой `{ "Type": "Custom", "Data": "money", "Amount": 5000 }`:

```c
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		BBP_API.OnCustomReward.Insert(MyBattlePassReward);
	}

	void MyBattlePassReward(PlayerBase player, string data, int amount)
	{
		if (data == "money" && player)
			MyTraderApi.AddMoney(player, amount); // ваша функция
	}
}
```

Так как `BBP_API` объявлен в Boogy_BattlePass, мод-интегратор должен указывать
`Boogy_BattlePass` в `requiredAddons`, либо оборачивать вызовы в `#ifdef BOOGY_BATTLEPASS`.

## RPC

Диапазон `887900–887949` (см. `Scripts/3_Game/BBP_Constants.c`). Не пересекается с SM_PartyMod (`887700–887799`).
