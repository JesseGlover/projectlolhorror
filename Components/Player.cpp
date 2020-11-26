#include "StdAfx.h"
#include "Player.h"
#include "RegularBullet.h"
#include "WaterBullet.h"
#include "GamePlugin.h"
#include <CryRenderer/IRenderAuxGeom.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CryCore/StaticInstanceList.h>

// Namespace to store our registration component which is needed for our class to show up in the editor.
namespace
{
	static void RegisterPlayerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CPlayerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterPlayerComponent);
}

void CPlayerComponent::Initialize()
{
	// The character controller is responsible for maintaining player physics
	m_pCharacterController = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCharacterControllerComponent>();
	// Offset the default character controller up by one unit
	m_pCharacterController->SetTransformMatrix(Matrix34::Create(Vec3(1.f), IDENTITY, Vec3(0, 0, 1.f)));

	// Create the advanced animation component, responsible for updating Mannequin and animating the player
	m_pAnimationComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();

	// Set the player geometry, this also triggers physics proxy creation
	m_pAnimationComponent->SetMannequinAnimationDatabaseFile("Animations/Mannequin/ADB/FirstPerson.adb");
	// Set the player geometry based on the model stored in the engine.
	m_pAnimationComponent->SetCharacterFile("Objects/Characters/SampleCharacter/thirdperson.cdf");
	// Controller Definition sets our blend spaces and animations that are available for our model.
	m_pAnimationComponent->SetControllerDefinitionFile("Animations/Mannequin/ADB/FirstPersonControllerDefinition.xml");
	// Sets the context name based off what is set in the blendspace editor / xml file.
	m_pAnimationComponent->SetDefaultScopeContextName("FirstPersonCharacter");
	// Queue the idle fragment to start playing immediately on next update
	m_pAnimationComponent->SetDefaultFragmentName("Idle");

	// Disable movement coming from the animation (root joint offset), we control this entirely via physics
	m_pAnimationComponent->SetAnimationDrivenMotion(true);
	// Sets the model, animation and physics to align with the ground.
	m_pAnimationComponent->EnableGroundAlignment(true);
	// Load the character and Mannequin data from file
	m_pAnimationComponent->LoadFromDisk();

	// Acquire tag identifiers to avoid doing so each update
	m_walkTagId = m_pAnimationComponent->GetTagId("Walk");
	// Initializes the remaining items we need.
	InitializePlayer();
}

void CPlayerComponent::InitializePlayer()
{
	// Create the camera component, will automatically update the viewport every frame
	m_pCameraComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCameraComponent>();

	// Create the audio listener component.
	m_pAudioListenerComponent = m_pEntity->GetOrCreateComponent<Cry::Audio::DefaultComponents::CListenerComponent>();

	// Get the input component, wraps access to action mapping so we can easily get callbacks when inputs are triggered
	m_pInputComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CInputComponent>();
	InitializeCamera();
	InitializeLeftMovement();
	InitializeRightMovement();
	InitializeForwardMovement();
	InitializeBackMovement();
	InitializeShooting();
	InitializeWeaponSelection();
	// Spawn the cursor
	SpawnCursorEntity();
}

void CPlayerComponent::InitializeLeftMovement()
{
		/* Left Movement*/
		// Register an action, and the callback that will be sent when it's triggered
		m_pInputComponent->RegisterAction("player", "moveleft", [this](int activationMode, float value) { HandleInputFlagChange(EInputFlag::MoveLeft, (EActionActivationMode)activationMode);  });
		// Bind the 'A' key the "moveleft" action and so on.
		//29U
		m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, EKeyId::eKI_A);
		//532U
		m_pInputComponent->BindAction("player", "moveleft", eAID_XboxPad, EKeyId::eKI_XI_ThumbLLeft);
}

void CPlayerComponent::InitializeRightMovement()
{
		/* Right Movement*/
		m_pInputComponent->RegisterAction("player", "moveright", [this](int activationMode, float value) { HandleInputFlagChange(EInputFlag::MoveRight, (EActionActivationMode)activationMode);  });
		//31U
		m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, EKeyId::eKI_D);
		//533U
		m_pInputComponent->BindAction("player", "moveright", eAID_XboxPad, EKeyId::eKI_XI_ThumbLRight);
}

void CPlayerComponent::InitializeForwardMovement()
{
	/* Forward Movement*/
	m_pInputComponent->RegisterAction("player", "moveforward", [this](int activationMode, float value) { HandleInputFlagChange(EInputFlag::MoveForward, (EActionActivationMode)activationMode);  });
	//16U
	m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, EKeyId::eKI_W);
	//530U
	m_pInputComponent->BindAction("player", "moveforward", eAID_XboxPad, EKeyId::eKI_XI_ThumbLUp);
}

void CPlayerComponent::InitializeBackMovement()
{
	/* Back Movement*/
	m_pInputComponent->RegisterAction("player", "moveback", [this](int activationMode, float value) { HandleInputFlagChange(EInputFlag::MoveBack, (EActionActivationMode)activationMode);  });
	//30U
	m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, EKeyId::eKI_S);
	//531U
	m_pInputComponent->BindAction("player", "moveback", eAID_XboxPad, EKeyId::eKI_XI_ThumbLDown);
}

void CPlayerComponent::InitializeCursorPitchMovement()
{
	m_pInputComponent->RegisterAction("player", "mouse_rotate_negative_pitch", [this](int activationMode, float value) { m_cursorPositionInWorld.y -= value;});
	m_pInputComponent->RegisterAction("player", "mouse_rotate_positive_pitch", [this](int activationMode, float value) {m_cursorPositionInWorld.y += value;});
	m_pInputComponent->BindAction("player", "mouse_rotate_negative_pitch", eAID_KeyboardMouse, EKeyId::eKI_MouseY);
	m_pInputComponent->BindAction("player", "mouse_rotate_negative_pitch", eAID_XboxPad, EKeyId::eKI_XI_ThumbRDown);
	m_pInputComponent->BindAction("player", "mouse_rotate_positive_pitch", eAID_XboxPad, EKeyId::eKI_XI_ThumbRUp);
	m_pInputComponent->BindAction("player", "mouse_rotate_negative_pitch", eAID_KeyboardMouse, EKeyId::eKI_MouseY);
}

void CPlayerComponent::InitializeCursorYawMovement()
{
	m_pInputComponent->RegisterAction("player", "mouse_negative_rotateyaw", [this](int activationMode, float value) {m_cursorPositionInWorld.x -= value; });
	m_pInputComponent->RegisterAction("player", "mouse_positive_rotateyaw", [this](int activationMode, float value) {m_cursorPositionInWorld.x += value; });
	m_pInputComponent->BindAction("player", "mouse_negative_rotateyaw", eAID_KeyboardMouse, EKeyId::eKI_MouseX);
	m_pInputComponent->BindAction("player", "mouse_positive_rotateyaw", eAID_KeyboardMouse, EKeyId::eKI_MouseX);
	m_pInputComponent->BindAction("player", "mouse_negative_rotateyaw", eAID_XboxPad, EKeyId::eKI_XI_ThumbRLeft);
	m_pInputComponent->BindAction("player", "mouse_positive_rotateyaw", eAID_XboxPad, EKeyId::eKI_XI_ThumbRRight);
}

void CPlayerComponent::InitializeShooting()
{
	// Register the shoot action
	m_pInputComponent->RegisterAction("player", "shoot", [this](int activationMode, float value)
	{
		// Only fire on press, not release
		if (activationMode == eAAM_OnPress)
		{
				WeaponSelection();
		}
	});

	/* Weapon fire*/
	// Bind the shoot action to left mouse click
	//536U
	m_pInputComponent->BindAction("player", "shoot", eAID_KeyboardMouse, EKeyId::eKI_Mouse1);
	//536U
	m_pInputComponent->BindAction("player", "shoot", eAID_XboxPad, EKeyId::eKI_XI_TriggerR);
}

void CPlayerComponent::InitializeWeaponSelection()
{
	/* Weapon Selection*/

	// Register the selection action
	m_pInputComponent->RegisterAction("player", "selectweaponone", [this](int activationMode, float value)
	{
		selection = 0;
	});
	//1U
	m_pInputComponent->BindAction("player", "selectweaponone", eAID_KeyboardMouse, EKeyId::eKI_1);
	//520U
	m_pInputComponent->BindAction("player", "selectweaponone", eAID_XboxPad, EKeyId::eKI_XI_ShoulderL);
	// Register the selection action
	m_pInputComponent->RegisterAction("player", "selectweapontwo", [this](int activationMode, float value)
	{
		selection = 1;
	});
	//2U
	m_pInputComponent->BindAction("player", "selectweapontwo", eAID_KeyboardMouse, EKeyId::eKI_2);
	//526U
	m_pInputComponent->BindAction("player", "selectweapontwo", eAID_XboxPad, EKeyId::eKI_XI_TriggerL);
}

// Processing which Event flags we need.
Cry::Entity::EventFlags CPlayerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::Initialize |
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

// We handle each of the event flags we need.
void CPlayerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
		// Initialization step will call both of our initialization functions and set our alive check to true.
	case Cry::Entity::EEvent::Initialize:
	{
		m_isAlive = true;
		Initialize();
	}
	break;
	// Game play handling for when it has just started, we want to reset the player position, animation and input flags.
	case Cry::Entity::EEvent::GameplayStarted:
	{
		ResetPlayer();
	}
	break;
	case Cry::Entity::EEvent::Update:
	{
		// Don't update the player if we haven't spawned yet
		if (!m_isAlive)
			return;
		// C++ version of implementing / creating frame time in CE.
		const float frameTime = event.fParam[0];

		// Update the in-world cursor position
		UpdateCursor(frameTime);

		// Start by updating the movement request we want to send to the character controller
		// This results in the physical representation of the character moving
		UpdateMovementRequest(frameTime);

		// Update the animation state of the character
		UpdateAnimation(frameTime);

		// Update the camera component offset
		CameraMode(frameTime);
	}
	break;
	case Cry::Entity::EEvent::Reset:
	{
		ResetPlayer();
	}
	break;
	}
}

void CPlayerComponent::SpawnCursorEntity()
{
	if (m_pCursorEntity)
	{
		gEnv->pEntitySystem->RemoveEntity(m_pCursorEntity->GetId());
	}

	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();

	// Spawn the cursor
	m_pCursorEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams);

	// Load geometry for the cursor, in our case, it is just a sphere.
	const int geometrySlot = 0;
	m_pCursorEntity->LoadGeometry(geometrySlot, "%ENGINE%/EngineAssets/Objects/primitive_sphere.cgf");

	// Scale the cursor down a bit
	m_pCursorEntity->SetScale(Vec3(0.1f));
	m_pCursorEntity->SetViewDistRatio(255);

	// Load the custom cursor material
	IMaterial* pCursorMaterial = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/cursor");
	m_pCursorEntity->SetMaterial(pCursorMaterial);
}

void CPlayerComponent::InitializeCamera()
{
	//m_pInputComponent->RegisterAction("player", "TopDownCameraMode", [this](int activationMode, float value)
	//{
	//	cameraSelection = 3;
	//});
	//m_pInputComponent->RegisterAction("player", "SideScrollCameraMode", [this](int activationMode, float value)
	//{
	//	cameraSelection = 4;
	//});
	//m_pInputComponent->BindAction("player", "TopDownCameraMode", eAID_KeyboardMouse, EKeyId::eKI_3);
	//m_pInputComponent->BindAction("player", "SideScrollCameraMode", eAID_KeyboardMouse, EKeyId::eKI_4);
}

void CPlayerComponent::CameraMode(float frameTime)
{
	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
	if (m_pCameraComponent != nullptr) {
		if (IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams))
		{
			switch (cameraSelection)
			{
			case 3:
			{
				UpdateTopDownCamera(frameTime);
				m_SideView = false;
				m_TopDown = true;
			}
			break;
			case 4:
			{
				UpdateSideViewCamera(frameTime);
				m_TopDown = false;
				m_SideView = true;
			}
			break;
			}
		}
	}
}

void CPlayerComponent::UpdateMovementRequest(float frameTime)
{
	// Don't handle input if we are in air
	if (!m_pCharacterController->IsOnGround())
		return;
	// initialize our velocity variable to be a zero vector
	Vec3 velocity = ZERO;
	// create a move speed float value, 20.5 is a smooth movement speed.
	const float moveSpeed = 20.5f;
	// Utilizing our input flags, we can manipulate how the player moves.
	if (m_TopDown != false)
	{
		if (m_inputFlags & EInputFlag::MoveLeft)
		{
			velocity.x -= moveSpeed * frameTime;
		}
		if (m_inputFlags & EInputFlag::MoveRight)
		{
			velocity.x += moveSpeed * frameTime;
		}
	}
	if (m_TopDown != false)
	{
		if (m_inputFlags & EInputFlag::MoveForward)
		{
			velocity.y += moveSpeed * frameTime;
		}
		if (m_inputFlags & EInputFlag::MoveBack)
		{
			velocity.y -= moveSpeed * frameTime;
		}
	}
	else
	{
		if (m_inputFlags & EInputFlag::MoveForward)
		{
			velocity.y -= moveSpeed * frameTime;
		}
		if (m_inputFlags & EInputFlag::MoveBack)
		{
			velocity.y += moveSpeed * frameTime;
		}
	}
	// update the character controller's velocity based off the velocity value as it changes.
	m_pCharacterController->AddVelocity(velocity);
}

void CPlayerComponent::UpdateAnimation(float frameTime)
{
	// Update the Mannequin tags
	m_pAnimationComponent->SetTagWithId(m_walkTagId, true);
	// if the cursor is null, don't update the animation.
	if (m_pCursorEntity == nullptr)
	{
		return;
	}
	// Dir is a direction vector 3 value, it will be the difference between the cursor's world position and 
	// the player character' world position.
	Vec3 dir = m_pCursorEntity->GetWorldPos() - m_pEntity->GetWorldPos();
	// normalize the results.
	dir = dir.Normalize();
	// newRotation is a Quaternion which will be a rotation v direction
	// which is based off the direction vector we created.
	Quat newRotation = Quat::CreateRotationVDir(dir);
	// ypr is Yaw Pit Rotation from Angles3. It will be created from the Camera Components'
	// function to create angles YPR. The parameter is a 3x3 matrix based off our newRotation variable.
	Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(newRotation));

	// We only want to affect Z-axis rotation, zero pitch and roll
	ypr.y = 0;
	ypr.z = 0;

	// Re-calculate the quaternion based on the corrected yaw
	newRotation = Quat(CCamera::CreateOrientationYPR(ypr));

	// If the character controller is walking
	if (m_pCharacterController->IsWalking())
	{
		// Send updated transform to the entity, only orientation changes
		m_pEntity->SetPosRotScale(m_pEntity->GetWorldPos(), newRotation, Vec3(1, 1, 1));
	}
	// If the character controller is not walking
	else
	{
		// Update only the player rotation
		m_pEntity->SetRotation(newRotation);
	}
}

void CPlayerComponent::UpdateTopDownCamera(float frameTime)
{
	// Start with rotating the camera to face downwards
	Matrix34 localTransform = IDENTITY;
	localTransform.SetRotation33(Matrix33(m_pEntity->GetWorldRotation().GetInverted()) * Matrix33::CreateRotationX(DEG2RAD(-90)));

	// change this to have fun results of the camera's distance from the character.
	const float viewDistanceFromPlayer = 5.f;

	// Offset the player along the forward axis (normally back)
	// Also offset upwards. This affects the camera and the audio components.
	localTransform.SetTranslation(Vec3(0, 0, viewDistanceFromPlayer));
	m_pCameraComponent->SetTransformMatrix(localTransform);
	m_pAudioListenerComponent->SetOffset(localTransform.GetTranslation());
}

void CPlayerComponent::UpdateSideViewCamera(float frameTime)
{
	Matrix34 localTransform = IDENTITY;

	const float viewDistance = 5;
	const float viewOffsetUp = 2.f;

	// Offset the player along the forward axis (normally back)
	// Also offset upwards
	localTransform.SetTranslation(Vec3(viewDistance, 0, viewOffsetUp));

	localTransform.SetRotation33(Matrix33::CreateRotationZ(DEG2RAD(90)));

	m_pCameraComponent->SetTransformMatrix(localTransform);
	m_pAudioListenerComponent->SetOffset(localTransform.GetTranslation());
}

void CPlayerComponent::UpdateCursor(float frameTime)
{
	offset = Vec3(0, 0, 10.f);
	m_cursorPositionInWorld = m_pEntity->GetWorldPos() + offset;
	// Don't handle input if we are in air
	if (!m_pCharacterController->IsOnGround())
		return;
}

void CPlayerComponent::ResetPlayer()
{
	// Apply character to the entity
	m_pAnimationComponent->ResetCharacter();
	m_pCharacterController->Physicalize();
	// Reset input now that the player respawned
	m_inputFlags.Clear();
	cameraSelection = 3;
	maxRegularAmmo = 5;
	regularAmmoCount = maxRegularAmmo;
	maxWaterAmmo = 5;
	waterAmmoCount = maxWaterAmmo;
}

void CPlayerComponent::WeaponSelection() 
{
	switch (selection)
	{
		case 0:
		{
			if (regularAmmoCount > 0) {
				m_pRegularBullet->Fire(m_pAnimationComponent, regularAmmoCount);
				regularAmmoCount -= 1;
			}
		}
		break;
		case 1:
		{
			if (waterAmmoCount > 0) {
			m_pWaterBullet->Fire(m_pAnimationComponent, waterAmmoCount);
			waterAmmoCount -= 1;
			}
		}
		break;
	}
}

void CPlayerComponent::HandleInputFlagChange(const CEnumFlags<EInputFlag> flags, const CEnumFlags<EActionActivationMode> activationMode, const EInputFlagType type)
{
	switch (type)
	{
	case EInputFlagType::Hold:
	{
		if (activationMode == eAAM_OnRelease)
		{
			m_inputFlags &= ~flags;
		}
		else
		{
			m_inputFlags |= flags;
		}
	}
	break;
	case EInputFlagType::Toggle:
	{
		if (activationMode == eAAM_OnRelease)
		{
			// Toggle the bit(s)
			m_inputFlags ^= flags;
		}
	}
	break;
	}
}