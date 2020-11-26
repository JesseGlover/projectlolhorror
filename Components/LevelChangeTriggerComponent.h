#pragma once
#include "StdAfx.h"
#include "Player.h"
#include <CryEntitySystem/IEntitySystem.h>
#include <CryEntitySystem/IEntityComponent.h>

// Example of a component that receives enter and leave events from a virtual box positioned on the entity
class CLevelChangeTriggerComponent : public IEntityComponent
{
public:
	CLevelChangeTriggerComponent() = default;
	virtual ~CLevelChangeTriggerComponent() = default;
	static void ReflectType(Schematyc::CTypeDesc<CLevelChangeTriggerComponent>& desc) 
	{ 
		desc.SetGUID("{D34659E5-FD99-4B5E-A7CA-C5834E1D0E80}"_cry_guid); 
	}

	virtual void Initialize() override;

	virtual void ProcessEvent(const SEntityEvent& event) override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	CPlayerComponent* player = nullptr;
};