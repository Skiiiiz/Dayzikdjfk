// ============================================================================
// SM_DungeonMapObject.c
//
// Импорт "доп. наполнения" данжа (объекты/заражённые/животные), размеченного
// в COT (Community Online Tools) или VPP Admin Tools при подготовке
// локации. Так как оба инструмента - сторонние Workshop-моды с собственным
// (и иногда меняющимся) форматом экспорта, официально поддерживается один
// канонический JSON-формат:
//
//   { "Objects": [
//       { "ClassName": "Wooden_Crate", "Position": [7500.1, 12.4, 8300.5], "Orientation": [0, 90, 0] },
//       ...
//   ]}
//
// Если ваш экспорт из COT/VPP выглядит иначе - один раз прогоните его через
// TOOLS/convert_location.py, который приводит произвольные ключи
// (Name/Pos/Ori, ClassName/Position/Orientation и т.п.) к этому формату.
// Для удобства загрузчик ниже также сам понимает несколько наиболее часто
// встречающихся альтернативных ключей и оберток, но гарантированно
// поддерживается только каноническая форма выше.
// ============================================================================

class SM_DungeonMapObject
{
	// Каноническое имя поля + самые частые алиасы сторонних экспортов.
	string ClassName;
	string Name;
	string Type;
	vector Position;
	vector Pos;
	vector Orientation;
	vector Ori;

	string ResolveClassName()
	{
		if (ClassName != "")
			return ClassName;
		if (Name != "")
			return Name;
		return Type;
	}

	vector ResolvePosition()
	{
		if (!IsZeroVector(Position))
			return Position;
		return Pos;
	}

	vector ResolveOrientation()
	{
		if (!IsZeroVector(Orientation))
			return Orientation;
		return Ori;
	}

	protected bool IsZeroVector(vector v)
	{
		return v[0] == 0 && v[1] == 0 && v[2] == 0;
	}
}

class SM_DungeonMapObjectFileObjects
{
	ref array<ref SM_DungeonMapObject> Objects = new array<ref SM_DungeonMapObject>;
}

class SM_DungeonMapObjectFileData
{
	ref array<ref SM_DungeonMapObject> Data = new array<ref SM_DungeonMapObject>;
}

class SM_DungeonMapObjectFileItems
{
	ref array<ref SM_DungeonMapObject> Items = new array<ref SM_DungeonMapObject>;
}

class SM_DungeonLocationImporter
{
	// Резолвит путь конфигурации ($mission:/$profile:/относительный) и
	// пытается загрузить список точек. Возвращает false, если файл
	// отсутствует или не подошёл ни под одну из поддерживаемых оберток.
	static bool LoadObjects(string path, out array<ref SM_DungeonMapObject> result)
	{
		result = new array<ref SM_DungeonMapObject>;

		if (path == "" || !FileExist(path))
			return false;

		string errorMessage;

		SM_DungeonMapObjectFileObjects wrappedObjects = new SM_DungeonMapObjectFileObjects();
		if (JsonFileLoader<SM_DungeonMapObjectFileObjects>.LoadFile(path, wrappedObjects, errorMessage) && wrappedObjects.Objects && wrappedObjects.Objects.Count() > 0)
		{
			result = wrappedObjects.Objects;
			return true;
		}

		SM_DungeonMapObjectFileData wrappedData = new SM_DungeonMapObjectFileData();
		if (JsonFileLoader<SM_DungeonMapObjectFileData>.LoadFile(path, wrappedData, errorMessage) && wrappedData.Data && wrappedData.Data.Count() > 0)
		{
			result = wrappedData.Data;
			return true;
		}

		SM_DungeonMapObjectFileItems wrappedItems = new SM_DungeonMapObjectFileItems();
		if (JsonFileLoader<SM_DungeonMapObjectFileItems>.LoadFile(path, wrappedItems, errorMessage) && wrappedItems.Items && wrappedItems.Items.Count() > 0)
		{
			result = wrappedItems.Items;
			return true;
		}

		return false;
	}

	// Превращает импортированные точки + ручные точки из конфига в единый
	// список спавн-точек данжа (см. SM_DungeonSpawnPointConfig).
	static void BuildSpawnPoints(SM_DungeonExtraContentConfig extraContent, out array<ref SM_DungeonSpawnPointConfig> result)
	{
		result = new array<ref SM_DungeonSpawnPointConfig>;
		if (!extraContent || !extraContent.Enabled)
			return;

		if (extraContent.ImportFile != "")
		{
			array<ref SM_DungeonMapObject> imported;
			if (LoadObjects(extraContent.ImportFile, imported))
			{
				foreach (SM_DungeonMapObject obj : imported)
				{
					if (!obj)
						continue;
					string className = obj.ResolveClassName();
					if (className == "")
						continue;
					result.Insert(new SM_DungeonSpawnPointConfig(SM_DungeonSpawnType.STATIC_OBJECT, className, obj.ResolvePosition(), obj.ResolveOrientation()));
				}
			}
			else
			{
				ErrorEx("[SM_DungeonMod] Could not import location file: " + extraContent.ImportFile);
			}
		}

		if (extraContent.ManualPoints)
		{
			foreach (SM_DungeonSpawnPointConfig manual : extraContent.ManualPoints)
			{
				if (manual)
					result.Insert(manual);
			}
		}
	}
}
