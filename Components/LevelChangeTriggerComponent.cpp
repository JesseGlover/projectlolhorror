#include "LevelChangeTriggerComponent.h"
#include "StdAfx.h"
#include "Player.h"
#include "GamePlugin.h"
#include <DefaultComponents/Cameras/CameraComponent.h>
#include <CrySchematyc\Env\Elements\EnvComponent.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterLevelChangeTriggerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CLevelChangeTriggerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterLevelChangeTriggerComponent);
}

void CLevelChangeTriggerComponent::Initialize()
{
	// Create a new IEntityTriggerComponent instance, responsible for registering our entity in the proximity grid
	IEntityTriggerComponent* pTriggerComponent = m_pEntity->CreateComponent<IEntityTriggerComponent>();
	// Listen to area events in a 2m^3 box around the entity
	const Vec3 triggerBoxSize = Vec3(2, 2, 2);

	// Create an axis aligned bounding box, ensuring that we listen to events around the entity translation
	const AABB triggerBounds = AABB(triggerBoxSize * -0.5f, triggerBoxSize * 0.5f);
	// Now set the trigger bounds on the trigger component
	pTriggerComponent->SetTriggerBounds(triggerBounds);
}

void CLevelChangeTriggerComponent::ProcessEvent(const SEntityEvent& event)
{
	if (event.event == ENTITY_EVENT_ENTERAREA)
	{
		//player = m_pEntity->GetComponent<CPlayerComponent>();
		// Get the entity identifier of the entity that just entered our shape
		const EntityId enteredEntityId = static_cast<EntityId>(event.nParam[0]);
		//gEnv->pConsole->ExecuteString("map sidescrolllevel", false, true);
		if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(enteredEntityId))
		{
			player = pEntity->GetComponent<CPlayerComponent>();
			player->cameraSelection = 4;
		}
		CryLog("Entity event area entered triggered");
	}
}

Cry::Entity::EventFlags CLevelChangeTriggerComponent::GetEventMask() const
{
	// Listen to the enter and leave events, in order to receive callbacks above when entities enter our trigger box
	return { ENTITY_EVENT_ENTERAREA, ENTITY_EVENT_LEAVEAREA };
}