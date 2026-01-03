// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGCharacter.h"

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Camera/RPGCameraComponent.h"
#include "Character/RPGHealthComponent.h"
#include "Character/RPGPawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Player/RPGPlayerController.h"
#include "Player/RPGPlayerState.h"
#include "Character/RPGCharacterMovementComponent.h"
#include "System/RPGGameplayTags.h"
#include "System/RPGLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGCharacter)

class AActor;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;

static FName NAME_RPGCharacterCollisionProfile_Capsule(TEXT("RPGPawnCapsule"));
static FName NAME_RPGCharacterCollisionProfile_Mesh(TEXT("RPGPawnMesh"));

ARPGCharacter::ARPGCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URPGCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Enable tick to monitor movement for Stamina system
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetNetCullDistanceSquared(900000000.0f);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_RPGCharacterCollisionProfile_Capsule);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	MeshComp->SetCollisionProfileName(NAME_RPGCharacterCollisionProfile_Mesh);

	URPGCharacterMovementComponent* RPGMoveComp = CastChecked<URPGCharacterMovementComponent>(GetCharacterMovement());
	RPGMoveComp->GravityScale = 1.0f;
	RPGMoveComp->MaxAcceleration = 2400.0f;
	RPGMoveComp->BrakingFrictionFactor = 1.0f;
	RPGMoveComp->BrakingFriction = 6.0f;
	RPGMoveComp->GroundFriction = 8.0f;
	RPGMoveComp->BrakingDecelerationWalking = 1400.0f;
	RPGMoveComp->bUseControllerDesiredRotation = false;
	RPGMoveComp->bOrientRotationToMovement = false;
	RPGMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	RPGMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	RPGMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	RPGMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	RPGMoveComp->SetCrouchedHalfHeight(65.0f);

	PawnExtComponent = CreateDefaultSubobject<URPGPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<URPGHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	CameraComponent = CreateDefaultSubobject<URPGCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

void ARPGCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ARPGCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ARPGCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMovementStateTags();
}

void ARPGCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ARPGCharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}

void ARPGCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
}

void ARPGCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
		const double MaxAccel = MovementComponent->MaxAcceleration;
		const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

		ReplicatedAcceleration.AccelXYRadians   = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     // [0, 2PI] -> [0, 255]
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	// [0, MaxAccel] -> [0, 255]
		ReplicatedAcceleration.AccelZ           = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   // [-MaxAccel, MaxAccel] -> [-127, 127]
	}
}

void ARPGCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
}

ARPGPlayerController* ARPGCharacter::GetRPGPlayerController() const
{
	return Cast<ARPGPlayerController>(GetController());
}

ARPGPlayerState* ARPGCharacter::GetRPGPlayerState() const
{
	return CastChecked<ARPGPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

URPGAbilitySystemComponent* ARPGCharacter::GetRPGAbilitySystemComponent() const
{
	return Cast<URPGAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* ARPGCharacter::GetAbilitySystemComponent() const
{
	if (PawnExtComponent == nullptr)
	{
		return nullptr;
	}

	return PawnExtComponent->GetRPGAbilitySystemComponent();
}

void ARPGCharacter::OnAbilitySystemInitialized()
{
	URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent();
	check(RPGASC);

	HealthComponent->InitializeWithAbilitySystem(RPGASC);

	InitializeGameplayTags();
}

void ARPGCharacter::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}

void ARPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtComponent->HandleControllerChanged();
}

void ARPGCharacter::UnPossessed()
{
	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();
}

void ARPGCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->HandleControllerChanged();
}

void ARPGCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->HandlePlayerStateReplicated();
}

void ARPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}

void ARPGCharacter::InitializeGameplayTags()
{
	// Clear tags that may be lingering on the ability system from the previous pawn.
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		// Note: We need to define RPG specific movement mode tags if we want to break Lyra dependency
		// For now, we stub this or assume the user will provide FRPGGameplayTags
	}
}

void ARPGCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		RPGASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool ARPGCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		return RPGASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool ARPGCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		return RPGASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool ARPGCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		return RPGASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

void ARPGCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	HealthComponent->DamageSelfDestruct(/*bFellOutOfWorld=*/ true);
}

void ARPGCharacter::OnDeathStarted(AActor*)
{
	DisableMovementAndCollision();
}

void ARPGCharacter::OnDeathFinished(AActor*)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}


void ARPGCharacter::DisableMovementAndCollision()
{
	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	URPGCharacterMovementComponent* RPGMoveComp = CastChecked<URPGCharacterMovementComponent>(GetCharacterMovement());
	RPGMoveComp->StopMovementImmediately();
	RPGMoveComp->DisableMovement();
}

void ARPGCharacter::DestroyDueToDeath()
{
	K2_OnDeathFinished();

	UninitAndDestroy();
}


void ARPGCharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		if (RPGASC->GetAvatarActor() == this)
		{
			PawnExtComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void ARPGCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	URPGCharacterMovementComponent* RPGMoveComp = CastChecked<URPGCharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
	SetMovementModeTag(RPGMoveComp->MovementMode, RPGMoveComp->CustomMovementMode, true);
}

void ARPGCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
		const FGameplayTag* TagPtr = RPGGameplayTags::MovementModeTagMap.Find(MovementMode);

		if (TagPtr && TagPtr->IsValid())
		{
			RPGASC->SetLooseGameplayTagCount(*TagPtr, (bTagEnabled ? 1 : 0));
		}

		if (MovementMode == MOVE_Custom)
		{
			const FGameplayTag* CustomTagPtr = RPGGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
			if (CustomTagPtr && CustomTagPtr->IsValid())
			{
				RPGASC->SetLooseGameplayTagCount(*CustomTagPtr, (bTagEnabled ? 1 : 0));
			}
		}
	}
}

void ARPGCharacter::UpdateMovementStateTags()
{
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
		const FVector Velocity = GetVelocity();
		const bool bIsMoving = !Velocity.IsNearlyZero(0.1f);

		RPGASC->SetLooseGameplayTagCount(GameplayTags.Status_Movement_Moving, (bIsMoving ? 1 : 0));
		RPGASC->SetLooseGameplayTagCount(GameplayTags.Status_Movement_Idle, (bIsMoving ? 0 : 1));
	}
}

void ARPGCharacter::ToggleCrouch()
{
	const URPGCharacterMovementComponent* RPGMoveComp = CastChecked<URPGCharacterMovementComponent>(GetCharacterMovement());

	if (IsCrouched() || RPGMoveComp->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (RPGMoveComp->IsMovingOnGround())
	{
		Crouch();
	}
}

void ARPGCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		// Stub: LyraGameplayTags::Status_Crouching
	}

	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void ARPGCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		// Stub: LyraGameplayTags::Status_Crouching
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool ARPGCharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}

void ARPGCharacter::OnRep_ReplicatedAcceleration()
{
	if (URPGCharacterMovementComponent* RPGMovementComponent = Cast<URPGCharacterMovementComponent>(GetCharacterMovement()))
	{
		// Decompress Acceleration
		const double MaxAccel         = RPGMovementComponent->MaxAcceleration;
		const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
		const double AccelXYRadians   = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     // [0, 255] -> [0, 2PI]

		FVector UnpackedAcceleration(FVector::ZeroVector);
		FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
		UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]

		RPGMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
	}
}

bool ARPGCharacter::UpdateSharedReplication()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FRPGSharedRepMovement SharedMovement;
		if (SharedMovement.FillForCharacter(this))
		{
			if (!SharedMovement.Equals(LastSharedReplication, this))
			{
				LastSharedReplication = SharedMovement;
				SetReplicatedMovementMode(SharedMovement.RepMovementMode);

				FastSharedReplication(SharedMovement);
			}
			return true;
		}
	}

	return false;
}

void ARPGCharacter::FastSharedReplication_Implementation(const FRPGSharedRepMovement& SharedRepMovement)
{
	if (GetWorld()->IsPlayingReplay())
	{
		return;
	}

	// Timestamp is checked to reject old moves.
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Timestamp
		SetReplicatedServerLastTransformUpdateTimeStamp(SharedRepMovement.RepTimeStamp);

		// Movement mode
		if (GetReplicatedMovementMode() != SharedRepMovement.RepMovementMode)
		{
			SetReplicatedMovementMode(SharedRepMovement.RepMovementMode);
			GetCharacterMovement()->bNetworkMovementModeChanged = true;
			GetCharacterMovement()->bNetworkUpdateReceived = true;
		}

		// Location, Rotation, Velocity, etc.
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
		MutableRepMovement = SharedRepMovement.RepMovement;

		// This also sets LastRepMovement
		OnRep_ReplicatedMovement();

		// Jump force
		SetProxyIsJumpForceApplied(SharedRepMovement.bProxyIsJumpForceApplied);

		// Crouch
		if (IsCrouched() != SharedRepMovement.bIsCrouched)
		{
			SetIsCrouched(SharedRepMovement.bIsCrouched);
			OnRep_IsCrouched();
		}
	}
}

FRPGSharedRepMovement::FRPGSharedRepMovement()
{
	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
}

bool FRPGSharedRepMovement::FillForCharacter(ACharacter* Character)
{
	if (USceneComponent* PawnRootComponent = Character->GetRootComponent())
	{
		UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

		RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);
		RepMovement.Rotation = PawnRootComponent->GetComponentRotation();
		RepMovement.LinearVelocity = CharacterMovement->Velocity;
		RepMovementMode = CharacterMovement->PackNetworkMovementMode();
		bProxyIsJumpForceApplied = Character->GetProxyIsJumpForceApplied() || (Character->JumpForceTimeRemaining > 0.0f);
		bIsCrouched = Character->IsCrouched();

		// Timestamp is sent as zero if unused
		if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
		{
			RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
		}
		else
		{
			RepTimeStamp = 0.f;
		}

		return true;
	}
	return false;
}

bool FRPGSharedRepMovement::Equals(const FRPGSharedRepMovement& Other, ACharacter* Character) const
{
	if (RepMovement.Location != Other.RepMovement.Location) return false;
	if (RepMovement.Rotation != Other.RepMovement.Rotation) return false;
	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity) return false;
	if (RepMovementMode != Other.RepMovementMode) return false;
	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied) return false;
	if (bIsCrouched != Other.bIsCrouched) return false;

	return true;
}

bool FRPGSharedRepMovement::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	RepMovement.NetSerialize(Ar, Map, bOutSuccess);
	Ar << RepMovementMode;
	Ar << bProxyIsJumpForceApplied;
	Ar << bIsCrouched;

	// Timestamp, if non-zero.
	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
	Ar.SerializeBits(&bHasTimeStamp, 1);
	if (bHasTimeStamp)
	{
		Ar << RepTimeStamp;
	}
	else
	{
		RepTimeStamp = 0.f;
	}

	return true;
}
