// Конфиги по умолчанию — создаются при первом запуске сервера.
class BBP_Defaults
{
	static BBP_Settings CreateSettings()
	{
		BBP_Settings s = new BBP_Settings;
		s.PremiumItems.Insert(new BBP_PremiumItemConfig("BBP_PremiumPass"));
		return s;
	}

	static BBP_ShopConfig CreateShop()
	{
		BBP_ShopConfig shop = new BBP_ShopConfig;

		BBP_CaseConfig medCase = new BBP_CaseConfig("case_medic", "Медицинский кейс", 150);
		medCase.Description = "Медикаменты и перевязка.";
		medCase.Rewards.Insert(MakeCaseItem("BandageDressing", 3, 40));
		medCase.Rewards.Insert(MakeCaseItem("Morphine", 1, 25));
		medCase.Rewards.Insert(MakeCaseItem("Epinephrine", 1, 20));
		medCase.Rewards.Insert(MakeCaseItem("SalineBagIV", 1, 10));
		medCase.Rewards.Insert(MakeCaseItem("FirstAidKit", 1, 5));
		shop.Cases.Insert(medCase);

		BBP_CaseConfig weaponCase = new BBP_CaseConfig("case_weapon", "Оружейный кейс", 400);
		weaponCase.Description = "Шанс получить оружие или патроны.";
		BBP_RewardConfig akm = MakeCaseItem("AKM", 1, 5);
		akm.Attachments.Insert("AK_WoodBttstck");
		akm.Attachments.Insert("AK_WoodHndgrd");
		akm.Attachments.Insert("Mag_AKM_30Rnd");
		weaponCase.Rewards.Insert(akm);
		weaponCase.Rewards.Insert(MakeCaseItem("SKS", 1, 15));
		BBP_RewardConfig ammo = MakeCaseItem("Ammo_762x39", 1, 50);
		ammo.Quantity = 20;
		weaponCase.Rewards.Insert(ammo);
		BBP_RewardConfig coinsBack = new BBP_RewardConfig(BBP_RewardType.COINS, "", 200);
		coinsBack.Weight = 30;
		weaponCase.Rewards.Insert(coinsBack);
		shop.Cases.Insert(weaponCase);

		BBP_CaseConfig premiumCase = new BBP_CaseConfig("case_premium", "Премиум кейс", 5);
		premiumCase.Description = "Только для владельцев премиум-пропуска. Оплата предметом-валютой.";
		premiumCase.PremiumOnly = true;
		premiumCase.Currency = BBP_Currency.ITEM;
		premiumCase.CurrencyItem = "Nail";
		premiumCase.Rewards.Insert(MakeCaseItem("PlateCarrierVest", 1, 30));
		premiumCase.Rewards.Insert(MakeCaseItem("NVGoggles", 1, 10));
		premiumCase.Rewards.Insert(MakeCaseItem("GhillieSuit_Woodland", 1, 20));
		BBP_RewardConfig xpBonus = new BBP_RewardConfig(BBP_RewardType.XP, "", 2000);
		xpBonus.Weight = 40;
		premiumCase.Rewards.Insert(xpBonus);
		shop.Cases.Insert(premiumCase);

		return shop;
	}

	static BBP_RewardConfig MakeCaseItem(string cls, int amount, int weight)
	{
		BBP_RewardConfig r = new BBP_RewardConfig(BBP_RewardType.ITEM, cls, amount);
		r.Weight = weight;
		return r;
	}

	static BBP_SeasonConfig CreateSeason(string id, string name, int startMinutes, int durationDays)
	{
		BBP_SeasonConfig season = new BBP_SeasonConfig;
		season.Id = id;
		season.Name = name;
		season.Description = "Выполняйте задания, получайте опыт и забирайте награды каждого уровня.";
		season.StartDate = BBP_Time.Format(startMinutes);
		season.EndDate = BBP_Time.Format(startMinutes + durationDays * 1440);
		season.XpPerLevel = 1000;

		array<string> freeItems = {"BandageDressing", "Apple", "CanOpener", "WaterBottle", "Matchbox", "TacticalBaconCan", "Rag", "HuntingKnife", "Compass", "Canteen"};
		array<string> premiumItems = {"Morphine", "SalineBagIV", "PlateCarrierVest", "MilitaryBoots_Black", "NVGoggles", "M4A1", "Mag_STANAG_30Rnd", "GhillieHood_Woodland", "Binoculars", "FirstAidKit"};

		for (int lvl = 1; lvl <= 30; lvl++)
		{
			BBP_LevelConfig level = new BBP_LevelConfig(lvl);
			if (lvl % 5 == 0)
			{
				level.FreeRewards.Insert(new BBP_RewardConfig(BBP_RewardType.COINS, "", 100));
				level.PremiumRewards.Insert(new BBP_RewardConfig(BBP_RewardType.CASE, "", 1));
				level.PremiumRewards[0].Data = "case_weapon";
				level.PremiumRewards[0].DisplayName = "Оружейный кейс";
			}
			else
			{
				level.FreeRewards.Insert(new BBP_RewardConfig(BBP_RewardType.ITEM, freeItems[lvl % freeItems.Count()], 1));
				level.PremiumRewards.Insert(new BBP_RewardConfig(BBP_RewardType.ITEM, premiumItems[lvl % premiumItems.Count()], 1));
			}
			season.Levels.Insert(level);
		}

		// --- Убийства ---
		BBP_QuestConfig zombies = new BBP_QuestConfig("kill_zombies_50", "Зачистка", BBP_QuestType.KILL, 50, 800);
		zombies.Description = "Убейте 50 заражённых.";
		zombies.KillKind = BBP_KillKind.ZOMBIE;
		season.Quests.Insert(zombies);

		BBP_QuestConfig daily = new BBP_QuestConfig("daily_zombies_15", "Ежедневная охота", BBP_QuestType.KILL, 15, 300);
		daily.Description = "Убейте 15 заражённых за день.";
		daily.KillKind = BBP_KillKind.ZOMBIE;
		daily.Period = BBP_Period.DAILY;
		season.Quests.Insert(daily);

		BBP_QuestConfig melee = new BBP_QuestConfig("kill_zombies_knife", "Тихий охотник", BBP_QuestType.KILL, 20, 600);
		melee.Description = "Убейте 20 заражённых ножом.";
		melee.KillKind = BBP_KillKind.ZOMBIE;
		melee.Weapons.Insert("HuntingKnife");
		melee.Weapons.Insert("KitchenKnife");
		melee.Weapons.Insert("CombatKnife");
		melee.Weapons.Insert("SteakKnife");
		season.Quests.Insert(melee);

		BBP_QuestConfig animals = new BBP_QuestConfig("kill_animals", "Охотник", BBP_QuestType.KILL, 10, 700);
		animals.Description = "Добудьте 10 животных.";
		animals.KillKind = BBP_KillKind.ANIMAL;
		season.Quests.Insert(animals);

		BBP_QuestConfig sniper = new BBP_QuestConfig("kill_players_long", "Снайпер", BBP_QuestType.KILL, 3, 1500);
		sniper.Description = "Убейте 3 игроков с дистанции от 300 м.";
		sniper.KillKind = BBP_KillKind.PLAYER;
		sniper.MinDistance = 300;
		sniper.PremiumOnly = true;
		season.Quests.Insert(sniper);

		// --- Действия ---
		BBP_QuestConfig heal = new BBP_QuestConfig("heal_bandage", "Полевой медик", BBP_QuestType.ACTION, 10, 400);
		heal.Description = "Перевяжите себя или других 10 раз.";
		heal.Targets.Insert("ActionBandageSelf");
		heal.Targets.Insert("ActionBandageTarget");
		season.Quests.Insert(heal);

		BBP_QuestConfig fire = new BBP_QuestConfig("light_fire", "Костровой", BBP_QuestType.ACTION, 5, 300);
		fire.Description = "Разожгите 5 костров.";
		fire.Targets.Insert("ActionLightItemOnFire");
		season.Quests.Insert(fire);

		BBP_QuestConfig build = new BBP_QuestConfig("build_parts", "Строитель", BBP_QuestType.ACTION, 15, 900);
		build.Description = "Постройте 15 частей базы.";
		build.Targets.Insert("ActionBuildPart");
		season.Quests.Insert(build);

		BBP_QuestConfig eat = new BBP_QuestConfig("eat_cans", "Консервы", BBP_QuestType.ACTION, 5, 200);
		eat.Description = "Съешьте 5 банок консервов.";
		eat.Targets.Insert("ActionEatBig");
		eat.Targets.Insert("ActionEat");
		eat.Items.Insert("BakedBeansCan_Opened");
		eat.Items.Insert("PeachesCan_Opened");
		eat.Items.Insert("SpaghettiCan_Opened");
		eat.Items.Insert("TacticalBaconCan_Opened");
		eat.Period = BBP_Period.WEEKLY;
		season.Quests.Insert(eat);

		// --- Крафт ---
		BBP_QuestConfig craft = new BBP_QuestConfig("craft_splints", "Мастер", BBP_QuestType.CRAFT, 3, 400);
		craft.Description = "Скрафтите 3 шины.";
		craft.Targets.Insert("Splint");
		season.Quests.Insert(craft);

		BBP_QuestConfig torch = new BBP_QuestConfig("craft_torch", "Факельщик", BBP_QuestType.CRAFT, 2, 250);
		torch.Description = "Скрафтите 2 факела.";
		torch.Targets.Insert("Torch");
		season.Quests.Insert(torch);

		// --- Локации (Chernarus) ---
		BBP_QuestConfig visit = new BBP_QuestConfig("visit_airfield", "Разведка аэродрома", BBP_QuestType.VISIT, 1, 500);
		visit.Description = "Доберитесь до Северо-Западного аэродрома.";
		visit.Points.Insert(new BBP_LocationPoint("NWAF", "4600 0 10300", 400));
		season.Quests.Insert(visit);

		BBP_QuestConfig route = new BBP_QuestConfig("route_coast", "Береговой маршрут", BBP_QuestType.VISIT, 1, 1200);
		route.Description = "Пройдите маршрут Черногорск -> Электрозаводск -> Балота по порядку.";
		route.VisitMode = BBP_VisitMode.ROUTE;
		route.Points.Insert(new BBP_LocationPoint("Черногорск", "6650 0 2550", 300));
		route.Points.Insert(new BBP_LocationPoint("Электрозаводск", "10400 0 2300", 300));
		route.Points.Insert(new BBP_LocationPoint("Балота", "4450 0 2400", 250));
		season.Quests.Insert(route);

		BBP_QuestConfig towns = new BBP_QuestConfig("visit_military", "Военные объекты", BBP_QuestType.VISIT, 1, 1000);
		towns.Description = "Посетите все военные базы в любом порядке.";
		towns.VisitMode = BBP_VisitMode.ALL;
		towns.Points.Insert(new BBP_LocationPoint("Тисы", "1700 0 14000", 300));
		towns.Points.Insert(new BBP_LocationPoint("Зелёногорск (военка)", "2500 0 5100", 250));
		towns.Points.Insert(new BBP_LocationPoint("Вышное", "6500 0 6100", 250));
		towns.PremiumOnly = true;
		season.Quests.Insert(towns);

		// --- Пользовательские события ---
		BBP_QuestConfig custom = new BBP_QuestConfig("custom_airdrop", "Охотник за грузом", BBP_QuestType.CUSTOM, 1, 1000);
		custom.Description = "Залутайте аирдроп (событие от другого мода).";
		custom.Targets.Insert("AirdropLooted");
		season.Quests.Insert(custom);

		return season;
	}
}
