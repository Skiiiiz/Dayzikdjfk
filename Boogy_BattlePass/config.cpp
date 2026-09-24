class CfgPatches
{
	class Boogy_BattlePass
	{
		units[] = {"BBP_PremiumPass"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Consumables"};
	};
};

class CfgMods
{
	class Boogy_BattlePass
	{
		dir = "Boogy_BattlePass";
		name = "Boogy_BattlePass";
		credits = "Boogy";
		author = "Boogy";
		version = "1.0";
		type = "mod";
		inputs = "Boogy_BattlePass/inputs.xml";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"Boogy_BattlePass/Scripts/Common", "Boogy_BattlePass/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"Boogy_BattlePass/Scripts/Common", "Boogy_BattlePass/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"Boogy_BattlePass/Scripts/Common", "Boogy_BattlePass/Scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	class Inventory_Base;

	// Предмет активации премиум-пропуска. Любой другой класс тоже можно
	// сделать активатором через Settings.json -> PremiumItems.
	class BBP_PremiumPass: Inventory_Base
	{
		scope = 2;
		displayName = "Премиум боевой пропуск";
		descriptionShort = "Используйте предмет в руках, чтобы активировать премиум-пропуск текущего сезона.";
		model = "\dz\gear\consumables\Paper.p3d";
		weight = 10;
		itemSize[] = {1, 1};
		rotationFlags = 16;
		absorbency = 0;
		varWetMax = 0;
	};
};
