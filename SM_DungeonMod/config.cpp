class CfgPatches
{
	class SM_DungeonMod
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};

class CfgMods
{
	class SM_DungeonMod
	{
		dir = "SM_DungeonMod";
		hideName = 1;
		hidePicture = 1;
		name = "SM_DungeonMod";
		credits = "SobrMods";
		author = "SobrMods";
		defines[] = {"SM_DUNGEONMOD"};
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] =
				{
					"SM_DungeonMod/Scripts/3_Game",
					"SM_DungeonMod/Scripts/Common"
				};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"SM_DungeonMod/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"SM_DungeonMod/Scripts/5_Mission"};
			};
		};
	};
};

// ------------------------------------------------------------------------
// Наградной билет: запечатанная награда за прохождение данжа.
// Раскрывается действием "Открыть билет" (ActionOpenDungeonTicket),
// содержимое хранится в самом предмете (см. SM_DungeonRewardTicket.c).
// Наследуется от ванильного класса Slip (лёгкий "бумажный" предмет,
// на нём же построены денежные ноуты в большинстве трейдер-модов).
// ------------------------------------------------------------------------
class CfgVehicles
{
	class Slip;
	class SM_DungeonRewardTicket: Slip
	{
		scope = 2;
		displayName = "#STR_SMD_TICKET_NAME";
		descriptionShort = "#STR_SMD_TICKET_DESC";
		weight = 10;
		itemSize[] = {1, 1};
		rotationFlags = 1;

		class DayZProperties
		{
			isUnique = 0;
		};
	};
};
