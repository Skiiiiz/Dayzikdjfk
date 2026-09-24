# Boogy_BattlePass — установка

## Сборка

1. Упакуйте папку `Boogy_BattlePass` в PBO (Addon Builder / PboProject) с префиксом `Boogy_BattlePass`.
   `config.cpp` будет бинаризован при упаковке.
2. Подпишите PBO своим ключом.
3. Структура релиза:

```text
@Boogy_BattlePass\addons\Boogy_BattlePass.pbo
@Boogy_BattlePass\addons\Boogy_BattlePass.pbo.<key>.bisign
@Boogy_BattlePass\keys\<key>.bikey
```

Папки `Docs`, `Examples`, `TOOLS` в PBO включать не обязательно.

## Установка на сервер

1. Остановите сервер.
2. Скопируйте `@Boogy_BattlePass` в корень сервера, `.bikey` — в папку `keys`.
3. Добавьте мод в параметры запуска (мод нужен и серверу, и клиентам):

```text
-mod=@CF;@Boogy_BattlePass
```

4. Запустите сервер один раз. Будут созданы файлы:

```text
$profile\Boogy_BattlePass\Settings.json     — общие настройки
$profile\Boogy_BattlePass\Shop.json         — магазин кейсов
$profile\Boogy_BattlePass\Seasons\Season1.json — сезон 1 (стартует в день первого запуска, 60 дней)
$profile\Boogy_BattlePass\Seasons\Season2.json — сезон 2 (сразу после первого)
$profile\Boogy_BattlePass\Players\<SteamID>.json — прогресс игроков
$profile\Boogy_BattlePass\Donations.json    — обработанные донат-платежи
```

5. Остановите сервер, пропишите свой SteamID64 в `AdminIds`, настройте сезоны и запустите снова.

Большинство изменений можно применить без рестарта: командой `/bp reload` в чате
или кнопкой «Перезагрузить» в меню (видна только администраторам).

## Предмет премиум-пропуска

Класс `BBP_PremiumPass` можно добавить в `types.xml`, трейдер или выдавать как награду.
Игрок берёт предмет в руки и выбирает действие «Активировать боевой пропуск».

Пример для `types.xml` (если нужен спавн — обычно не нужен):

```xml
<type name="BBP_PremiumPass">
    <nominal>0</nominal>
    <lifetime>3888000</lifetime>
    <restock>0</restock>
    <min>0</min>
    <quantmin>-1</quantmin>
    <quantmax>-1</quantmax>
    <cost>100</cost>
    <flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" count_in_player="0" crafted="0" deloot="0"/>
</type>
```

## Клавиша меню

По умолчанию **J**. Игрок может переназначить её в «Настройки → Управление → Boogy BattlePass».
