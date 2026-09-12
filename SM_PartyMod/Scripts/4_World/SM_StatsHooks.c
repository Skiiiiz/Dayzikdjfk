modded class ZombieBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame().IsServer())
			return;

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager)
			manager.OnStatZombieKilled(this, killer);
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!GetGame().IsServer() || damageType != DamageType.FIRE_ARM)
			return;

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager && manager.IsZombieStatTarget(this))
			manager.OnStatHit(this, source, dmgZone);
	}
}

modded class Weapon_Base
{
	override void EEFired(int muzzleType, int mode, string ammoType)
	{
		super.EEFired(muzzleType, mode, ammoType);

		if (!GetGame().IsServer())
			return;

		SM_ClanManager manager = SM_ClanManager.Get();
		if (manager)
			manager.OnStatShotFired(GetHierarchyRootPlayer());
	}
}
