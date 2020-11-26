#include "WaterBullet.h"

void WaterBulletComponent::Fire(Cry::DefaultComponents::CAdvancedAnimationComponent* m_pAnimationComponent, float AmmoCount)
{
	if (ICharacterInstance* pCharacter = m_pAnimationComponent->GetCharacter())
	{
		IAttachment* pBarrelOutAttachment = pCharacter->GetIAttachmentManager()->GetInterfaceByName("barrel_out");

		if (pBarrelOutAttachment != nullptr)
		{
			QuatTS bulletOrigin = pBarrelOutAttachment->GetAttWorldAbsolute();
			// Setting our spawn params for the bullet that will be fired.
			SEntitySpawnParams spawnParams;
			spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
			spawnParams.qRotation = bulletOrigin.q;
			spawnParams.vPosition = bulletOrigin.t;
			const float bulletScale = 0.05f;
			spawnParams.vScale = Vec3(bulletScale);
			// See RegularGun.h, bullet is propelled in the rotation and position the entity was spawned with
			if (IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams))
			{
				pEntity->CreateComponentClass<WaterBulletComponent>();
			}
		}
	}
}