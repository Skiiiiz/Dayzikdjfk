# Boogy_BattlePass — интеграция с донат-системой (HTTP)

Сервер периодически опрашивает ваш сайт/бота и получает список оплаченных покупок.
Покупки применяются даже к игрокам не в сети (сохраняются в их файл прогресса).

## Настройка

```json
"Donate": {
    "Enabled": 1,
    "BaseUrl": "https://your-shop.example.com",
    "PendingPath": "/api/battlepass/pending",
    "AckPath": "/api/battlepass/ack",
    "ApiKey": "secret-key",
    "ServerId": "server-1",
    "PollIntervalSec": 60
}
```

`ApiKey` и `ServerId` передаются в query-строке — используйте только символы `A-Z a-z 0-9 - _`.
Обязательно используйте HTTPS.

## 1. Получение покупок

```http
GET {BaseUrl}{PendingPath}?server={ServerId}&key={ApiKey}
```

Ответ (`200 OK`, JSON):

```json
{
    "Entries": [
        { "Id": "order-1001", "Uid": "76561198000000001", "Type": "Premium", "Data": "" },
        { "Id": "order-1002", "Uid": "76561198000000002", "Type": "Coins", "Amount": 500 },
        { "Id": "order-1003", "Uid": "76561198000000003", "Type": "Xp", "Amount": 3000 },
        { "Id": "order-1004", "Uid": "76561198000000004", "Type": "Case", "Data": "case_weapon", "Amount": 2 }
    ]
}
```

| Поле | Описание |
| --- | --- |
| `Id` | Уникальный Id платежа (защита от повторной выдачи) |
| `Uid` | SteamID64 игрока |
| `Type` | `Premium`, `Coins`, `Xp`, `Case` (регистр не важен) |
| `Amount` | Количество (монеты, опыт, кейсы) |
| `Data` | `Premium`: Id сезона (`""` = текущий, а если сезона нет — ближайший будущий); `Case`: Id кейса |

`Xp` начисляется в текущий сезон; если активного сезона нет, запись не подтверждается и будет
повторена при следующем опросе.

## 2. Подтверждение

После применения сервер отправляет:

```http
POST {BaseUrl}{AckPath}
Content-Type: application/json

{ "ServerId": "server-1", "ApiKey": "secret-key", "Ids": ["order-1001", "order-1002"] }
```

Помечайте эти записи выданными и больше не возвращайте их. Если подтверждение не дошло,
запись придёт снова, но повторно выдана не будет: обработанные `Id` хранятся в
`$profile\Boogy_BattlePass\Donations.json` (последние 2000), и сервер подтвердит её ещё раз.

## Минимальный пример сервера (Node.js / Express)

```js
const express = require("express");
const app = express();
app.use(express.json());

const KEY = "secret-key";
const pending = new Map(); // id -> entry, заполняется вашим платёжным обработчиком

app.get("/api/battlepass/pending", (req, res) => {
  if (req.query.key !== KEY) return res.sendStatus(403);
  res.json({ Entries: [...pending.values()] });
});

app.post("/api/battlepass/ack", (req, res) => {
  if (req.body.ApiKey !== KEY) return res.sendStatus(403);
  for (const id of req.body.Ids || []) pending.delete(id);
  res.json({ ok: true });
});

app.listen(8080);
```

## Другие способы выдачи премиума

- Предмет `BBP_PremiumPass` (или свой класс из `PremiumItems`) — можно продавать у трейдера.
- Команда администратора: `/bp premium <SteamID64> [seasonId]`.
- Из другого мода: `BBP_API.GivePremium(uid)`.
