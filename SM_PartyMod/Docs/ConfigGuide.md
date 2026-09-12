# SM_PartyMod - гайд по конфигу

Файл конфига:

```text
$profile\SM_PartyMod\Settings.json
```

Правьте конфиг только при выключенном сервере.

## Главное про значения

| Значение | Что значит |
| --- | --- |
| `true` | включено |
| `false` | выключено |
| `0` | часто значит "без лимита" или "нет награды", если это указано ниже |
| `1.0` | 100% шанс |
| `0.5` | 50% шанс |
| `3600` | 1 час в секундах |
| `1000` | 1 км в метрах |

Цвета в чате пишутся так:

```json
"#E0E0E0"
```

## ConfigVersion

```json
"ConfigVersion": "1.15"
```

Версия структуры конфига. Менять вручную обычно не нужно.

## General

Общие лимиты кланов.

| Параметр | Что делает |
| --- | --- |
| `MaxClans` | максимум кланов на сервере. `0` = без лимита |
| `MaxClanMembers` | максимум участников в одном клане |
| `MinClanNameLength` | минимальная длина названия клана |
| `MaxClanNameLength` | максимальная длина названия клана |
| `MaxClanTagLength` | максимальная длина тега |
| `MaxClanDescriptionLength` | максимальная длина описания |
| `CreateClanLogoPaa` | картинка/логотип по умолчанию |
| `MaxApplicationsPerClan` | максимум заявок в один клан |
| `InviteTimeoutSeconds` | сколько живет приглашение в секундах |
| `ShowClanListToEveryone` | показывать список кланов всем игрокам |

Пример:

```json
"General": {
  "MaxClans": 0,
  "MaxClanMembers": 10,
  "MinClanNameLength": 3,
  "MaxClanNameLength": 24,
  "MaxClanTagLength": 6,
  "MaxClanDescriptionLength": 200,
  "CreateClanLogoPaa": "SM_PartyMod\\GUI\\logo\\logo.paa",
  "MaxApplicationsPerClan": 20,
  "InviteTimeoutSeconds": 180,
  "ShowClanListToEveryone": true
}
```

## Ranks

Ранги идут от младшего к старшему.

Пример:

```json
"RankNames": ["Новобранец", "Боец", "Офицер", "Лидер"]
```

Номера рангов в этом примере:

| Номер | Ранг |
| --- | --- |
| `0` | Новобранец |
| `1` | Боец |
| `2` | Офицер |
| `3` | Лидер |

| Параметр | Что делает |
| --- | --- |
| `AdminUids` | SteamID64 админов клановой системы |
| `MinRankToInvite` | минимальный ранг для приглашений |
| `MinRankToKick` | минимальный ранг для кика |

Пример:

```json
"Ranks": {
  "RankNames": ["Новобранец", "Боец", "Офицер", "Лидер"],
  "AdminUids": ["76561198000000000"],
  "MinRankToInvite": 2,
  "MinRankToKick": 2
}
```

`MinRankToInvite: 2` значит: приглашать могут Офицер и Лидер.

Админы из `AdminUids` могут обновить `Settings.json` без перезапуска сервера командой в чат:

```text
!smpreload
```

Также работают алиасы: `!partyreload`, `!smreload`, `/smpreload`, `/partyreload`, `/smreload`.

## MemberHud

Серверное включение HUD участников клана.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает HUD клана на сервере |
| `UpdateIntervalSeconds` | как часто сервер обновляет данные HUD |

Пример:

```json
"MemberHud": {
  "Enabled": true,
  "UpdateIntervalSeconds": 2
}
```

Положение, прозрачность, компактный режим и количество игроков в HUD настраивает сам игрок у себя в меню.

## ChatSystem

Настройки серверного чата.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает чат мода |
| `MessageMaxLength` | максимальная длина сообщения |
| `LocalEnabled` | включает локальный чат |
| `GlobalEnabled` | включает глобальный чат |
| `LocalRangeMeters` | радиус локального чата в метрах |

Пример:

```json
"ChatSystem": {
  "Enabled": true,
  "MessageMaxLength": 1024,
  "LocalEnabled": true,
  "GlobalEnabled": true,
  "LocalRangeMeters": 75
}
```

### Цвета чата

Базовые цвета задаются RGB-параметрами прямо в `ChatSystem`:

```json
"ColorClan": {
  "R": 224,
  "G": 224,
  "B": 224
},
"ColorClanPlayer": {
  "R": 100,
  "G": 180,
  "B": 255
},
"ColorDirect": {
  "R": 255,
  "G": 255,
  "B": 255
},
"ColorDirectPlayer": {
  "R": 255,
  "G": 255,
  "B": 255
},
"ColorGlobal": {
  "R": 255,
  "G": 255,
  "B": 255
},
"ColorGlobalPlayer": {
  "R": 100,
  "G": 255,
  "B": 100
},
"ColorServer": {
  "R": 255,
  "G": 50,
  "B": 50
},
"ColorAlert": {
  "R": 100,
  "G": 200,
  "B": 255
}
```

| Параметр | Что делает |
| --- | --- |
| `ColorClan` | цвет текста кланового чата |
| `ColorClanPlayer` | цвет ника игрока в клановом чате |
| `ColorDirect` | цвет текста и метки прямого/локального чата |
| `ColorDirectPlayer` | цвет ника игрока в прямом/локальном чате |
| `ColorGlobal` | цвет текста и метки глобального чата |
| `ColorGlobalPlayer` | цвет ника игрока в глобальном чате |
| `ColorServer` | цвет серверных сообщений |
| `ColorAlert` | цвет важных уведомлений и предупреждений |

Внутри `ChatSystem` есть секции:

```json
"Clan": {},
"Local": {},
"Global": {}
```

В каждой секции можно настроить:

| Параметр | Что делает |
| --- | --- |
| `PlayerStyles` | индивидуальные стили игроков |

Пример индивидуального стиля:

```json
{
  "SteamId": "76561198000000000",
  "Prefix": "[ADMIN]",
  "NameColorRGB": {
    "R": 255,
    "G": 204,
    "B": 51
  },
  "TextColorRGB": {
    "R": 255,
    "G": 255,
    "B": 255
  },
  "PrefixColorRGB": {
    "R": 255,
    "G": 51,
    "B": 51
  }
}
```

Если `Prefix` пустой, префикса не будет.

## Discord

Отправка чата в Discord webhook.

| Параметр | Что делает |
| --- | --- |
| `WebhookUrl` | общий webhook, если отдельный не указан |
| `ClanWebhookUrl` | webhook для кланового чата |
| `LocalWebhookUrl` | webhook для локального чата |
| `GlobalWebhookUrl` | webhook для глобального чата |
| `ServerLabel` | название сервера в Discord сообщении |
| `RelayClan` | отправлять клановый чат в Discord |
| `RelayLocal` | отправлять локальный чат в Discord |
| `RelayGlobal` | отправлять глобальный чат в Discord |

Пример:

```json
"Discord": {
  "WebhookUrl": "https://discord.com/api/webhooks/000000000000000000/TOKEN",
  "ClanWebhookUrl": "",
  "LocalWebhookUrl": "",
  "GlobalWebhookUrl": "",
  "ServerLabel": "My DayZ Server",
  "RelayClan": true,
  "RelayLocal": false,
  "RelayGlobal": true
}
```

Кланы могут поставить свой webhook через меню клана. Если у клана есть свой webhook, клановый чат будет уходить туда.

## BaseRegistrationAudit

Админский контроль зарегистрированных баз через Discord.

Это отдельный аудит для серверодержателей. Лучше указывать отдельный закрытый webhook, не клановый webhook игроков.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает аудит баз |
| `WebhookUrl` | Discord webhook, куда отправлять embed-сообщения |
| `UseMainWebhookFallback` | если `WebhookUrl` пустой, использовать `Discord.WebhookUrl` |
| `SendRegister` | отправлять регистрацию/перепривязку базы |
| `SendUnregister` | отправлять отвязку базы |
| `SendMemberChanges` | отправлять вступление, выход, кик, роспуск клана |
| `SendRankChanges` | отправлять смену рангов и передачу лидерства |
| `IncludeMemberList` | добавлять список жильцов клана в embed |

Пример:

```json
"BaseRegistrationAudit": {
  "Enabled": true,
  "WebhookUrl": "https://discord.com/api/webhooks/000000000000000000/TOKEN",
  "UseMainWebhookFallback": false,
  "SendRegister": true,
  "SendUnregister": true,
  "SendMemberChanges": true,
  "SendRankChanges": true,
  "IncludeMemberList": true
}
```

Что будет в embed:

- клан;
- тег;
- уровень;
- казна;
- координаты базы;
- кто сделал действие;
- SteamID64;
- кликабельная ссылка на Steam профиль;
- хозяин/лидер клана;
- жильцы клана;
- кто онлайн;
- дата и время.

События отправляются только по кланам, у которых зарегистрирована база.

Базу можно зарегистрировать только на флагштоке, где установлен флаг и он поднят до конца.

## Pings

Быстрые метки клана.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает пинги |
| `DurationSeconds` | сколько секунд живет пинг |
| `CooldownSeconds` | задержка между пингами |

Пример:

```json
"Pings": {
  "Enabled": true,
  "DurationSeconds": 30,
  "CooldownSeconds": 0.7
}
```

Дистанцию показа пингов игрок настраивает у себя. Значение `0` у игрока значит показывать на любой дистанции.

## ClanMap

Клановая карта и серверные метки.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/отключает карту мода целиком |
| `MaxMarkersPerClan` | максимум меток клана |
| `MarkerMaxNameLength` | максимальная длина названия метки |
| `BaseRadiusEnabled` | показывать круг радиуса базы на карте |
| `BaseRadiusMeters` | радиус круга вокруг базы в метрах |
| `ServerMarkers` | серверные метки, которые видят игроки |

Чтобы отключить круги вокруг баз:

```json
"ClanMap": {
  "Enabled": true,
  "MaxMarkersPerClan": 30,
  "MarkerMaxNameLength": 32,
  "BaseRadiusEnabled": false,
  "BaseRadiusMeters": 150,
  "ServerMarkers": []
}
```

Пример серверной метки:

```json
{
  "Name": "Трейдер",
  "Position": "7500 0 7500",
  "ColorRGB": {
    "R": 235,
    "G": 140,
    "B": 30
  },
  "IconPaa": "\\dz\\gear\\navigation\\data\\map_cross_ca.paa",
  "Enabled": true
}
```

`Position` пишется как `X Y Z`. Для обычной метки можно ставить `Y = 0`.
`IconPaa` можно писать как ванильный путь `\\dz\\...` или как путь внутри мода `SM_PartyMod\\GUI\\...`.

Серверные метки по умолчанию показываются и на карте, и в 3D. Игрок может скрыть 3D у себя.

## Logs

Логи действий клана.

| Параметр | Что делает |
| --- | --- |
| `MaxEntries` | сколько последних записей хранить в логе клана |

Пример:

```json
"Logs": {
  "MaxEntries": 30
}
```

## Market

Рынок игроков.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает рынок игроков |
| `MaxLotsPerClan` | максимум лотов клана, если уровни клана выключены |
| `FeePercent` | комиссия с продажи в процентах |
| `ListingFeePercent` | комиссия за выставление лота, списывается сразу |
| `MinCancelAgeMinutes` | через сколько минут после выставления можно снять лот |
| `CancelPenaltyPercent` | штраф за снятие лота от цены |
| `ReturnDelaySeconds` | задержка возврата предмета после снятия |
| `MinRankToSell` | минимальный ранг для продажи |
| `MaxPrice` | максимальная цена лота |
| `Blacklist` | предметы, которые нельзя продавать и класть в хранилище |
| `TradeZones` | зоны, где разрешены выставление/получение |

Пример:

```json
"Market": {
  "Enabled": true,
  "MaxLotsPerClan": 10,
  "FeePercent": 5,
  "ListingFeePercent": 5,
  "MinCancelAgeMinutes": 60,
  "CancelPenaltyPercent": 10,
  "ReturnDelaySeconds": 900,
  "MinRankToSell": 2,
  "MaxPrice": 1000000,
  "Blacklist": ["ExpansionCodeLock", "Codelock"],
  "TradeZones": {
    "Enabled": true,
    "RequireForListing": true,
    "RequireForTaking": true,
    "Zones": [
      {
        "Name": "Trader",
        "Position": "7500 0 7500",
        "RadiusMeters": 30,
        "Enabled": true
      }
    ]
  }
}
```

`RequireForListing` ограничивает выставление лота зоной. `RequireForTaking` ограничивает покупку, снятие и возврат предмета зоной.

## ServerMarket

Серверный рынок с автоматической ротацией товаров.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает серверный рынок |
| `SaveState` | сохранять текущую ротацию после рестарта |
| `RefreshOnServerStart` | обновлять товары при каждом старте сервера |
| `EnableDynamicPrice` | включить рост цены при малом остатке |
| `RefreshIntervalSeconds` | интервал обновления товаров |
| `GeneratedItemsCount` | сколько товаров генерировать в ротации |
| `DefaultVisibleItems` | сколько товаров видно без уровней клана |
| `PurchaseReservationSeconds` | бронь товара на время покупки |
| `DynamicPriceSteps` | правила повышения цены |
| `Items` | список товаров, из которых собирается ротация |

Пример товара:

```json
{
  "ClassName": "BandageDressing",
  "MinQuantity": 5,
  "MaxQuantity": 12,
  "MinPrice": 100,
  "MaxPrice": 250,
  "SpawnChance": 1.0,
  "MaxBuyPerPlayer": 3
}
```

Как настраивать:

| Параметр товара | Что значит |
| --- | --- |
| `ClassName` | класс предмета |
| `MinQuantity` | минимальное количество |
| `MaxQuantity` | максимальное количество |
| `MinPrice` | минимальная цена |
| `MaxPrice` | максимальная цена |
| `SpawnChance` | шанс попасть в ротацию |
| `MaxBuyPerPlayer` | лимит покупок на игрока. `0` = без лимита |

Пример динамической цены:

```json
{
  "StockPercentBelow": 25,
  "PriceMultiplier": 1.25
}
```

Это значит: если осталось меньше 25% товара, цена станет выше в 1.25 раза.

## Tops

Топы кланов.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает топы |
| `Categories` | какие категории показывать |
| `MinShotsForAccuracyTop` | минимум выстрелов для топа точности |
| `RewardsEnabled` | включить награды за топы |
| `RewardSchedule` | `monthly` или `hours` |
| `RewardIntervalHours` | период наград, если выбран `hours` |
| `RewardTreasuryPoints` | сколько денег добавить в казну победителю |
| `RewardLootEnabled` | выдавать предметную награду за топ |
| `RewardLootContainers` | список предметных наград |
| `ResetStatsAfterReward` | сбрасывать статистику после выдачи наград |

Категории:

| Category | Что показывает |
| --- | --- |
| `PlayerKills` | убийства игроков |
| `ZombieKills` | убийства зараженных |
| `OnlineTime` | онлайн |
| `BestLife` | лучшая жизнь без смерти |
| `Distance` | пройденная дистанция |
| `Accuracy` | точность |
| `Headshots` | попадания в голову |
| `KillDeath` | K/D |
| `LongestPlayerKill` | самый дальний килл игрока |

Пример категории:

```json
{
  "Category": "PlayerKills",
  "Enabled": true
}
```

Пример предметной награды за топ:

```json
{
  "ClassName": "WoodenCrate",
  "MinQuantity": 1,
  "MaxQuantity": 1,
  "SpawnChance": 1.0,
  "Loot": [
    {
      "ClassName": "BandageDressing",
      "MinQuantity": 2,
      "MaxQuantity": 4,
      "SpawnChance": 1.0
    },
    {
      "ClassName": "Ammo_762x39",
      "MinQuantity": 1,
      "MaxQuantity": 3,
      "SpawnChance": 0.5
    }
  ]
}
```

Предметная награда топов кладется в клановое хранилище. Для этого должно быть включено `Storage.Enabled`.

## AchievementSettings

Достижения.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает достижения |
| `NotifyOnComplete` | показывать уведомление при выполнении |
| `ClaimRewardManually` | оставьте `true`: игрок забирает награду кнопкой |
| `Personal` | личные достижения |
| `Clan` | клановые достижения |

### Параметры одного достижения

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включено ли достижение |
| `Id` | уникальный ID достижения |
| `Name` | название в меню |
| `Description` | описание |
| `Type` | что считать |
| `Target` | сколько нужно набрать |
| `Hidden` | скрытое достижение |
| `RewardMoney` | деньги игроку |
| `RewardClanTreasury` | деньги в казну клана |
| `RewardContainer` | предметная награда |

### Личные типы достижений

| Type | Что считает |
| --- | --- |
| `CreatedClan` | создание клана |
| `JoinedClan` | вступление в клан |
| `PlayerKills` | убийства игроков |
| `ZombieKills` | убийства зараженных |
| `OnlineSeconds` | онлайн в секундах |
| `BestLifeSeconds` | лучшая жизнь в секундах |
| `DistanceWalked` | пройденная дистанция в метрах |
| `ShotsFired` | выстрелы |
| `HitsLanded` | попадания |
| `Headshots` | попадания в голову |
| `LongestKill` | самый дальний килл в метрах |
| `TreasuryDeposited` | сколько внесено в казну |
| `MarketBuys` | покупки на рынке игроков |
| `MarketSells` | продажи на рынке игроков |
| `StorageDeposits` | предметы, положенные в хранилище |
| `AuctionWins` | выигранные аукционы |
| `AuctionSales` | продажи на аукционе |

### Клановые типы достижений

| Type | Что считает |
| --- | --- |
| `Members` | количество участников |
| `Level` | уровень клана |
| `Treasury` | текущая казна |
| `TreasuryDeposited` | сколько всего внесено в казну |
| `PlayerKills` | убийства игроков |
| `ZombieKills` | убийства зараженных |
| `OnlineSeconds` | общий онлайн клана |
| `BestLifeSeconds` | лучшая жизнь участника |
| `DistanceWalked` | общая дистанция |
| `ShotsFired` | выстрелы |
| `HitsLanded` | попадания |
| `Headshots` | попадания в голову |
| `LongestKill` | самый дальний килл |
| `MarketSells` | продажи на рынке игроков |
| `StorageDeposits` | предметы, положенные в хранилище |
| `AuctionSales` | продажи на аукционе |
| `AuctionWins` | выигранные аукционы |

## PlayerExperience

Личные очки опыта и звания игроков.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает систему опыта |
| `ShowNotifications` | показывать игроку уведомления за XP и новые звания |
| `ShowInClanList` | показывать звание участника в списке клана |
| `ShowPublicTab` | показывать вкладку `Звания` со списком игроков |
| `RestrictedItemsEnabled` | запрещать предметы, если звание игрока ниже нужного |
| `ZombieKillXP` | сколько XP давать за обычного зараженного |
| `PlayerKillXP` | сколько XP давать за убийство игрока |
| `OnlineRewardIntervalSeconds` | раз в сколько секунд давать XP за онлайн |
| `OnlineRewardXP` | сколько XP давать за один онлайн-интервал |
| `MaxPublicPlayers` | сколько игроков максимум выводить во вкладке званий |
| `Ranks` | список званий от младшего к старшему |
| `ZombieClassRewards` | отдельный XP за конкретные классы зараженных |
| `RestrictedItems` | список предметов, доступных только с указанного звания |

Пример звания:

```json
{
  "Name": "Сталкер",
  "RequiredXP": 700,
  "ColorRGB": {
    "R": 166,
    "G": 146,
    "B": 238
  }
}
```

`RequiredXP` - сколько XP нужно для звания. Первое звание должно начинаться с `0`.

Пример отдельной награды за зараженного:

```json
{
  "ClassName": "ZmbM_NBC_Yellow",
  "XP": 3
}
```

Если класс зараженного не указан в `ZombieClassRewards`, используется `ZombieKillXP`.

### Ограничение предметов по званию

```json
{
  "ClassName": "M4A1",
  "RequiredTitle": "Сталкер",
  "MatchInherited": true
}
```

| Параметр | Что делает |
| --- | --- |
| `ClassName` | класс предмета |
| `RequiredTitle` | минимальное звание из `PlayerExperience.Ranks` |
| `MatchInherited` | `true` - правило работает и на наследников класса, `false` - только на точное имя класса |

Если игрок ниже нужного звания, предмет сбрасывается под ноги и игрок получает уведомление.

### Пример денежной награды

```json
{
  "Enabled": true,
  "Id": "personal_zombies_100",
  "Name": "Чистильщик",
  "Description": "Убейте 100 зараженных",
  "Type": "ZombieKills",
  "Target": 100,
  "Hidden": false,
  "RewardMoney": 5000,
  "RewardClanTreasury": 0,
  "RewardContainer": null
}
```

### Пример предметной награды вместо денег

```json
{
  "Enabled": true,
  "Id": "personal_zombies_box",
  "Name": "Запас выжившего",
  "Description": "Убейте 100 зараженных",
  "Type": "ZombieKills",
  "Target": 100,
  "Hidden": false,
  "RewardMoney": 0,
  "RewardClanTreasury": 0,
  "RewardContainer": {
    "ClassName": "WoodenCrate",
    "MinQuantity": 1,
    "MaxQuantity": 1,
    "SpawnChance": 1.0,
    "Loot": [
      {
        "ClassName": "BandageDressing",
        "MinQuantity": 2,
        "MaxQuantity": 4,
        "SpawnChance": 1.0
      },
      {
        "ClassName": "TacticalBaconCan",
        "MinQuantity": 2,
        "MaxQuantity": 4,
        "SpawnChance": 1.0
      }
    ]
  }
}
```

`RewardMoney: 0` значит денег за достижение не будет.

`RewardContainer` выдаст предмет игроку, который забрал награду.

## Auction

Аукцион игроков.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает аукцион |
| `SaveState` | сохранять аукцион после рестарта |
| `MaxActiveLotsPerPlayer` | максимум активных лотов на игрока |
| `MinDurationMinutes` | минимальная длительность лота |
| `MaxDurationMinutes` | максимальная длительность лота |
| `DefaultDurationMinutes` | длительность по умолчанию |
| `MinBidStep` | минимальный шаг ставки |
| `ListingFee` | фиксированная комиссия за выставление лота |
| `ListingFeePercent` | процентная комиссия за выставление от стартовой цены |
| `SaleTaxPercent` | налог с продажи |
| `AntiSnipeSeconds` | если ставка сделана в последние N секунд |
| `AntiSnipeExtendSeconds` | на сколько продлить лот |
| `MaxStartPrice` | максимальная стартовая цена |
| `MinCancelAgeMinutes` | через сколько минут после выставления можно отменить лот без ставок |
| `CancelPenaltyPercent` | штраф за отмену лота без ставок от стартовой цены |
| `ReturnDelaySeconds` | задержка возврата предмета после отмены или конца без ставок |
| `AllowCancelWithoutBids` | разрешить отмену без ставок |
| `ReturnItemIfNoBids` | вернуть предмет, если ставок не было |
| `TradeZones` | зоны, где разрешены выставление/получение |

Пример:

```json
"Auction": {
  "Enabled": true,
  "SaveState": true,
  "MaxActiveLotsPerPlayer": 5,
  "MinDurationMinutes": 30,
  "MaxDurationMinutes": 1440,
  "DefaultDurationMinutes": 360,
  "MinBidStep": 100,
  "ListingFee": 0,
  "ListingFeePercent": 5,
  "SaleTaxPercent": 5,
  "AntiSnipeSeconds": 30,
  "AntiSnipeExtendSeconds": 60,
  "MaxStartPrice": 1000000,
  "MinCancelAgeMinutes": 60,
  "CancelPenaltyPercent": 10,
  "ReturnDelaySeconds": 900,
  "AllowCancelWithoutBids": true,
  "ReturnItemIfNoBids": true,
  "TradeZones": {
    "Enabled": true,
    "RequireForListing": true,
    "RequireForTaking": true,
    "Zones": [
      {
        "Name": "Trader",
        "Position": "7500 0 7500",
        "RadiusMeters": 30,
        "Enabled": true
      }
    ]
  }
}
```

## Treasury

Казна клана и деньги.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает казну |
| `CurrencyItems` | список предметов-денег и их ценность |

Пример:

```json
"CurrencyItems": [
  {
    "Classname": "TraderPlus_Money_Ruble10000",
    "Value": 10000
  },
  {
    "Classname": "TraderPlus_Money_Ruble1000",
    "Value": 1000
  }
]
```

Эти же деньги используются для рынка, аукциона и денежных наград достижений.

Если у вас другая экономика, замените `Classname` на классы вашей валюты.

## Storage

Клановое хранилище.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает хранилище |
| `RadiusMeters` | радиус доступа к хранилищу от базы клана |

Пример:

```json
"Storage": {
  "Enabled": true,
  "RadiusMeters": 30
}
```

Количество слотов хранилища задается в `Levels.Items`.

## Levels

Уровни клана.

| Параметр | Что делает |
| --- | --- |
| `Enabled` | включает/выключает прокачку клана |
| `Items` | настройки каждого уровня |

Один уровень:

| Параметр | Что делает |
| --- | --- |
| `UpgradeCost` | стоимость перехода на этот уровень |
| `StorageSlots` | слоты хранилища |
| `MarketLots` | лимит лотов на рынке игроков |
| `ServerMarketItems` | сколько товаров видно в серверном маркете |

Пример:

```json
"Levels": {
  "Enabled": true,
  "Items": [
    {
      "UpgradeCost": 0,
      "StorageSlots": 15,
      "MarketLots": 5,
      "ServerMarketItems": 3
    },
    {
      "UpgradeCost": 10000,
      "StorageSlots": 25,
      "MarketLots": 8,
      "ServerMarketItems": 5
    }
  ]
}
```

Первый элемент - это 1 уровень клана. Его `UpgradeCost` всегда должен быть `0`.

## Частые настройки

### Отключить глобальный чат

```json
"GlobalEnabled": false
```

### Ограничить кланы до 6 игроков

```json
"MaxClanMembers": 6
```

### Сделать приглашения только для лидера

Если лидер - ранг `3`:

```json
"MinRankToInvite": 3
```

### Увеличить локальный чат до 150 метров

```json
"LocalRangeMeters": 150
```

### Отключить аукцион

```json
"Auction": {
  "Enabled": false
}
```

### Отключить серверный рынок

```json
"ServerMarket": {
  "Enabled": false
}
```

### Сделать достижение с наградой предметами

Поставьте:

```json
"RewardMoney": 0
```

И заполните:

```json
"RewardContainer": {
  "ClassName": "WoodenCrate",
  "MinQuantity": 1,
  "MaxQuantity": 1,
  "SpawnChance": 1.0,
  "Loot": []
}
```

Если нужен один предмет без ящика, укажите его в `ClassName`, а `Loot` оставьте пустым.
