# Boogy_BattlePass — конфиги

Все файлы лежат в `$profile\Boogy_BattlePass`. Полные примеры — в папке `Examples`.
Логические значения можно писать как `1`/`0` или `true`/`false`.

## Settings.json

| Поле | По умолчанию | Описание |
| --- | --- | --- |
| `Enabled` | `1` | Включить боевой пропуск |
| `TimezoneOffsetMinutes` | `180` | Смещение от UTC для дат сезонов (Москва = 180, Киев летом = 180, UTC = 0) |
| `AdminIds` | `[]` | SteamID64 администраторов (команды `/bp`, кнопка перезагрузки) |
| `PremiumItems` | `BBP_PremiumPass` | Предметы, активирующие премиум (см. ниже) |
| `NotifyQuestComplete` | `1` | Уведомление о выполнении задания |
| `NotifyLevelUp` | `1` | Уведомление о новом уровне |
| `AutoClaimRewards` | `0` | Выдавать награды уровней автоматически |
| `DropRewardsOnGroundIfFull` | `1` | Если инвентарь полон — предмет появляется под игроком |
| `AllowClaimAfterSeasonEnd` | `1` | Забирать награды завершившегося сезона, пока не начался новый |
| `VisitCheckIntervalSec` | `5` | Как часто проверять позиции игроков для заданий `Visit` |
| `AutoSaveIntervalSec` | `300` | Период автосохранения прогресса |
| `Donate` | — | Интеграция с донат-системой, см. [DonateApi.md](DonateApi.md) |

### PremiumItems

```json
"PremiumItems": [
    { "ClassName": "BBP_PremiumPass", "SeasonId": "", "Consume": 1 },
    { "ClassName": "MyShop_VipToken", "SeasonId": "season_2", "Consume": 1 }
]
```

- `ClassName` — класс предмета (учитывается наследование). Действие «Активировать боевой пропуск» появляется на любом таком предмете.
- `SeasonId` — для какого сезона активировать (`""` = текущий).
- `Consume` — удалить предмет после активации.

## Seasons\*.json

Каждый файл — один сезон. Имя файла любое, важен `Id` (должен быть уникальным и не меняться:
по нему хранится прогресс игроков).

```json
{
    "Id": "season_1",
    "Name": "Сезон 1: Выживший",
    "Description": "Текст в подвале меню",
    "Enabled": 1,
    "StartDate": "2026-10-01 00:00",
    "EndDate": "2026-11-30 23:59",
    "XpPerLevel": 1000,
    "Levels": [ ... ],
    "Quests": [ ... ]
}
```

- Даты — в формате `YYYY-MM-DD HH:MM` (или `YYYY-MM-DD`), в часовом поясе `TimezoneOffsetMinutes`.
- Активен тот сезон, в интервал которого попадает текущее время. Сезоны не должны пересекаться;
  если пересекаются — активен тот, что начался раньше.
- Смена сезона происходит автоматически. Между сезонами меню показывает последний завершённый сезон
  (для получения наград) и таймер до следующего.
- Чтобы временно отключить сезон — `"Enabled": 0`.

### Уровни

```json
{
    "Level": 3,
    "XpRequired": 1500,
    "FreeRewards": [ { "Type": "Item", "ClassName": "WaterBottle" } ],
    "PremiumRewards": [ { "Type": "Case", "Data": "case_weapon", "Amount": 2 } ]
}
```

- `XpRequired` — сколько опыта нужно именно на этот уровень (`0` = `XpPerLevel` сезона).
- Любая из веток может быть пустой.

## Награды (Reward)

Один формат используется в уровнях, заданиях (`Rewards`) и кейсах.

| Поле | Описание |
| --- | --- |
| `Type` | `Item`, `Coins`, `Xp`, `Case`, `Premium`, `Custom` |
| `ClassName` | `Item`: класс предмета |
| `Amount` | `Item`: количество экземпляров; `Coins`/`Xp`: сумма; `Case`: число кейсов |
| `Quantity` | `Item`: количество в стаке / патронов в магазине / объём жидкости (`-1` = по умолчанию) |
| `Health` | `Item`: состояние 0..1 (`-1` = по умолчанию) |
| `Attachments` | `Item`: классы вложений/содержимого, создаются внутри предмета |
| `Data` | `Case`: Id кейса; `Premium`: Id сезона (`""` = текущий); `Custom`: строка для другого мода |
| `DisplayName` | Подпись в меню (по умолчанию — имя предмета) |
| `Weight` | Вес выпадения (только в кейсах) |

Примеры:

```json
{ "Type": "Item", "ClassName": "M4A1", "Health": 1.0, "Attachments": ["M4_OEBttstck", "M4_PlasticHndgrd", "Mag_STANAG_30Rnd"] }
{ "Type": "Item", "ClassName": "Ammo_556x45", "Amount": 2, "Quantity": 20 }
{ "Type": "Coins", "Amount": 100 }
{ "Type": "Xp", "Amount": 500, "DisplayName": "Бонус опыта" }
{ "Type": "Case", "Data": "case_medic", "Amount": 1 }
{ "Type": "Premium", "Data": "", "DisplayName": "Премиум-пропуск" }
{ "Type": "Custom", "Data": "vip_title:Ветеран", "DisplayName": "Титул \"Ветеран\"" }
```

`Custom` ничего не выдаёт сам — его обрабатывает другой мод через `BBP_API.OnCustomReward`
(см. [Integration.md](Integration.md)). Так можно выдавать деньги трейдера, титулы, VIP-статусы и т.п.

## Задания (Quest)

Общие поля:

| Поле | Описание |
| --- | --- |
| `Id` | Уникальный Id внутри сезона (не меняйте — по нему хранится прогресс) |
| `Name`, `Description` | Текст в меню |
| `Type` | `Kill`, `Action`, `Craft`, `Visit`, `Custom` |
| `Count` | Сколько раз выполнить |
| `Xp`, `Coins` | Награда за выполнение |
| `Rewards` | Дополнительные награды (формат Reward) |
| `PremiumOnly` | Задание только для премиума |
| `Period` | `Season` (один раз за сезон), `Daily`, `Weekly` (сброс в 00:00 / по понедельникам) |
| `Zones` | Необязательно: выполнять только внутри зон (`Kill`, `Action`, `Craft`, `Custom`) |

### Kill

| Поле | Описание |
| --- | --- |
| `KillKind` | `Any`, `Zombie`, `Animal`, `Player` |
| `Targets` | Классы жертв (с наследованием), например `Animal_CanisLupus`, `ZmbM_SoldierNormal` |
| `Weapons` | Оружие убийства (с наследованием): `HuntingKnife`, `M4A1`, `Rifle_Base`… |
| `MinDistance`, `MaxDistance` | Дистанция убийства в метрах (`0` = без ограничения) |

```json
{ "Id": "sniper", "Name": "Снайпер", "Type": "Kill", "KillKind": "Player",
  "Weapons": ["Rifle_Base"], "MinDistance": 300, "Count": 3, "Xp": 1500 }
```

### Action

| Поле | Описание |
| --- | --- |
| `Targets` | Классы действий (с наследованием) |
| `Items` | Предмет в руках / используемый предмет (с наследованием) |

Полезные действия (ванильные классы; для модовых действий смотрите их скрипты):

| Что | Классы |
| --- | --- |
| Перевязка | `ActionBandageSelf`, `ActionBandageTarget` |
| Уколы | `ActionInjectSelf`, `ActionInjectTarget` |
| Капельница | `ActionGiveSalineSelf`, `ActionGiveSalineTarget` |
| Розжиг огня | `ActionLightItemOnFire` |
| Строительство | `ActionBuildPart` |
| Еда / питьё | `ActionEatBig`, `ActionEat`, `ActionDrink` |
| Разделка туш | `ActionSkinning` |

Продолжительные действия засчитываются за каждый завершённый цикл (например, каждый укус при еде),
одиночные — за каждое выполнение.

### Craft

`Targets` — классы созданных предметов (с наследованием) **или** классы рецептов. Пустой список — любой крафт.

### Visit

| Поле | Описание |
| --- | --- |
| `VisitMode` | `Any` — вход в любую точку засчитывает +1 (повторно — после выхода); `All` — посетить все точки в любом порядке; `Route` — строго по порядку |
| `Points` | `[{ "Name": "...", "Position": [x, y, z], "Radius": 300 }]` — `y` игнорируется |

Для `All` и `Route` `Count` считается автоматически (= числу точек).

### Custom

`Targets` — имена событий, которые другие моды отправляют через `BBP_API.ReportEvent(player, "Имя", amount)`.
Проверить задание можно командой `/bp event <Имя>`.

## Shop.json

```json
{
    "Enabled": 1,
    "Cases": [
        {
            "Id": "case_weapon",
            "Name": "Оружейный кейс",
            "Description": "...",
            "Price": 400,
            "Currency": "Coins",
            "CurrencyItem": "",
            "PremiumOnly": 0,
            "OpenOnPurchase": 0,
            "Rewards": [ { "Type": "Item", "ClassName": "SKS", "Weight": 15 }, ... ]
        }
    ]
}
```

- `Currency`: `Coins` — монеты пропуска (из наград, заданий, доната); `Item` — предметы из инвентаря
  класса `CurrencyItem` (стаки считаются по количеству, например рубли/деньги трейдера).
- Шанс = `Weight` / сумма всех `Weight`. Шансы видны игрокам в меню.
- Купленные кейсы хранятся у игрока и открываются кнопкой «Открыть». `OpenOnPurchase` — открыть сразу.
- Кейсы также можно выдавать наградой `{ "Type": "Case", "Data": "<Id>" }` или через донат.
