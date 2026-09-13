// ============================================================================
// SM_DungeonCreatureHooks.c
//
// Реализация "% исходящего урона игрока" по сложности данжа. Прямая правка
// TotalDamageResult до применения урона слишком завязана на конкретную
// версию движка/API повреждений, поэтому используется более устойчивый
// приём: даём урону примениться как обычно, а затем компенсируем разницу
// через AddHealth - GetHealth/AddHealth являются стабильным, давно
// задокументированным API и не зависят от внутреннего устройства
// TotalDamageResult в конкретной версии DayZ.
//
// Множитель 1.0 (сложность по умолчанию/данж не активен) не делает вообще
// ничего - ноль лишних вызовов на обычных серверных зомби/животных вне
// данжей.
// ============================================================================

modded class ZombieBase
{
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!GetGame().IsServer() || !SM_DungeonManager.Get())
		{
			super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
			return;
		}

		float multiplier = SM_DungeonManager.Get().GetOutgoingDamageMultiplier(source);
		if (multiplier == 1.0)
		{
			super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
			return;
		}

		float healthBefore = GetHealth("", "Health");
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!IsAlive())
			return;

		float healthAfter = GetHealth("", "Health");
		float rawDamage = healthBefore - healthAfter;
		if (rawDamage <= 0)
			return;

		float restore = rawDamage - rawDamage * multiplier;
		if (restore > 0)
			AddHealth("", "Health", restore);
	}
}

modded class AnimalBase
{
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!GetGame().IsServer() || !SM_DungeonManager.Get())
		{
			super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
			return;
		}

		float multiplier = SM_DungeonManager.Get().GetOutgoingDamageMultiplier(source);
		if (multiplier == 1.0)
		{
			super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
			return;
		}

		float healthBefore = GetHealth("", "Health");
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!IsAlive())
			return;

		float healthAfter = GetHealth("", "Health");
		float rawDamage = healthBefore - healthAfter;
		if (rawDamage <= 0)
			return;

		float restore = rawDamage - rawDamage * multiplier;
		if (restore > 0)
			AddHealth("", "Health", restore);
	}
}
