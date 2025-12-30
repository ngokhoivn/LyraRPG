// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGHeroComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Logging/MessageLog.h"
#include "System/RPGLogChannels.h"
#include "EnhancedInputSubsystems.h"
#include "Player/RPGPlayerController.h"
#include "Player/RPGPlayerState.h"
#include "Character/RPGPawnExtensionComponent.h"
#include "Character/RPGPawnData.h"
#include "Character/RPGCharacter.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Input/RPGInputConfig.h"
#include "Input/RPGInputComponent.h"
#include "Camera/RPGCameraComponent.h"
#include "System/RPGGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Camera/RPGCameraMode.h"
#include "InputMappingContext.h"
#include "System/RPGAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGHeroComponent)

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

namespace RPGHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName URPGHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName URPGHeroComponent::NAME_ActorFeatureName("Hero");

URPGHeroComponent::URPGHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityCameraMode = nullptr;
	bReadyToBindInputs = false;
}

void URPGHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogRPG, Error, TEXT("[URPGHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));
	}
	else
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
	}
}

bool URPGHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == RPGGameplayTags::InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == RPGGameplayTags::InitState_Spawned && DesiredState == RPGGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<ARPGPlayerState>())
		{
			return false;
		}

		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			ARPGPlayerController* RPGPC = GetController<ARPGPlayerController>();

			// The input component and local player is required when locally controlled.
			if (!Pawn->InputComponent || !RPGPC || !RPGPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == RPGGameplayTags::InitState_DataAvailable && DesiredState == RPGGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		ARPGPlayerState* RPGPS = GetPlayerState<ARPGPlayerState>();

		return RPGPS && Manager->HasFeatureReachedInitState(Pawn, URPGPawnExtensionComponent::NAME_ActorFeatureName, RPGGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == RPGGameplayTags::InitState_DataInitialized && DesiredState == RPGGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void URPGHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == RPGGameplayTags::InitState_DataAvailable && DesiredState == RPGGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		ARPGPlayerState* RPGPS = GetPlayerState<ARPGPlayerState>();
		if (!ensure(Pawn && RPGPS))
		{
			return;
		}

		const URPGPawnData* PawnData = nullptr;

		if (URPGPawnExtensionComponent* PawnExtComp = URPGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<URPGPawnData>();

			// The player state holds the persistent data for this player.
			PawnExtComp->InitializeAbilitySystem(RPGPS->GetRPGAbilitySystemComponent(), RPGPS);
		}

		if (ARPGPlayerController* RPGPC = GetController<ARPGPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		// Hook up the delegate for all pawns, in case we spectate later
		if (PawnData)
		{
			if (URPGCameraComponent* CameraComponent = URPGCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
			}
		}
	}
}

void URPGHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == URPGPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == RPGGameplayTags::InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void URPGHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { RPGGameplayTags::InitState_Spawned, RPGGameplayTags::InitState_DataAvailable, RPGGameplayTags::InitState_DataInitialized, RPGGameplayTags::InitState_GameplayReady };

	ContinueInitStateChain(StateChain);
}

void URPGHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(URPGPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	ensure(TryToChangeInitState(RPGGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void URPGHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void URPGHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const URPGPawnExtensionComponent* PawnExtComp = URPGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const URPGPawnData* PawnData = PawnExtComp->GetPawnData<URPGPawnData>())
		{
			if (const URPGInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FRPGInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = URPGAssetManager::GetAsset(Mapping.InputMapping))
					{
						Subsystem->AddMappingContext(IMC, Mapping.Priority);
					}
				}

				URPGInputComponent* RPGIC = Cast<URPGInputComponent>(PlayerInputComponent);
				if (ensureMsgf(RPGIC, TEXT("Unexpected Input Component class!")))
				{
					// Add the key mappings
					RPGIC->AddInputMappings(InputConfig, Subsystem);

					TArray<uint32> BindHandles;
					RPGIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					RPGIC->BindNativeAction(InputConfig, RPGGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					RPGIC->BindNativeAction(InputConfig, RPGGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					RPGIC->BindNativeAction(InputConfig, RPGGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
					RPGIC->BindNativeAction(InputConfig, RPGGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
					RPGIC->BindNativeAction(InputConfig, RPGGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
				}
			}
		}
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}
 
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void URPGHeroComponent::AddAdditionalInputConfig(const URPGInputConfig* InputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	
	const APlayerController* PC = GetController<APlayerController>();
	if (!PC)
	{
		return;
	}

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	if (const URPGPawnExtensionComponent* PawnExtComp = URPGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		URPGInputComponent* RPGIC = Pawn->FindComponentByClass<URPGInputComponent>();
		if (RPGIC)
		{
			RPGIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
		}
	}
}

void URPGHeroComponent::RemoveAdditionalInputConfig(const URPGInputConfig* InputConfig)
{
}

bool URPGHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void URPGHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const URPGPawnExtensionComponent* PawnExtComp = URPGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (URPGAbilitySystemComponent* RPGASC = PawnExtComp->GetRPGAbilitySystemComponent())
			{
				RPGASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

void URPGHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const URPGPawnExtensionComponent* PawnExtComp = URPGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (URPGAbilitySystemComponent* RPGASC = PawnExtComp->GetRPGAbilitySystemComponent())
		{
			RPGASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void URPGHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	if (ARPGPlayerController* RPGController = Cast<ARPGPlayerController>(Controller))
	{
		RPGController->SetIsAutoRunning(false);
	}
	
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void URPGHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void URPGHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * RPGHero::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * RPGHero::LookPitchRate * World->GetDeltaSeconds());
	}
}

void URPGHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (ARPGCharacter* Character = GetPawn<ARPGCharacter>())
	{
		Character->ToggleCrouch();
	}
}

void URPGHeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (ARPGPlayerController* Controller = Cast<ARPGPlayerController>(Pawn->GetController()))
		{
			Controller->SetIsAutoRunning(!Controller->GetIsAutoRunning());
		}	
	}
}

TSubclassOf<URPGCameraMode> URPGHeroComponent::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	if (URPGPawnExtensionComponent* PawnExtComp = URPGPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const URPGPawnData* PawnData = PawnExtComp->GetPawnData<URPGPawnData>())
		{
			return PawnData->DefaultCameraMode;
		}
	}

	return nullptr;
}

void URPGHeroComponent::SetAbilityCameraMode(TSubclassOf<URPGCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void URPGHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}
