// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/RPGPlayerController.h"
#include "Player/RPGPlayerState.h"
#include "UI/RPGHUD.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "System/RPGGameplayTags.h"
#include "System/RPGLogChannels.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGPlayerController)

ARPGPlayerController::ARPGPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ARPGPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(false);
}

void ARPGPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ARPGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ARPGPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
}

void ARPGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// If we are auto running then add some player input
	if (GetIsAutoRunning())
	{
		if (APawn* CurrentPawn = GetPawn())
		{
			const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			CurrentPawn->AddMovementInput(MovementDirection, 1.0f);	
		}
	}
}

ARPGPlayerState* ARPGPlayerController::GetRPGPlayerState() const
{
	return CastChecked<ARPGPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

URPGAbilitySystemComponent* ARPGPlayerController::GetRPGAbilitySystemComponent() const
{
	const ARPGPlayerState* RPGPS = GetRPGPlayerState();
	return (RPGPS ? RPGPS->GetRPGAbilitySystemComponent() : nullptr);
}

ARPGHUD* ARPGPlayerController::GetRPGHUD() const
{
	return CastChecked<ARPGHUD>(GetHUD(), ECastCheckedType::NullAllowed);
}

void ARPGPlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement
}

void ARPGPlayerController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();

	// Unbind from the old player state if needed (Team system stubs)
	
	// Bind to the new player state if needed

	LastSeenPlayerState = PlayerState;
}

void ARPGPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ARPGPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ARPGPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();

	// When we're a client connected to a remote server, we might need to refresh ASC info
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		if (ARPGPlayerState* RPGPS = GetPlayerState<ARPGPlayerState>())
		{
			if (URPGAbilitySystemComponent* RPGASC = RPGPS->GetRPGAbilitySystemComponent())
			{
				RPGASC->RefreshAbilityActorInfo();
				// Potential call to activate on-spawn abilities if we implement that logic
			}
		}
	}
}

void ARPGPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);
}

void ARPGPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

void ARPGPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		RPGASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ARPGPlayerController::OnCameraPenetratingTarget()
{
	bHideViewTargetPawnNextFrame = true;
}

void ARPGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetIsAutoRunning(false);
}

void ARPGPlayerController::SetIsAutoRunning(const bool bEnabled)
{
	const bool bIsAutoRunning = GetIsAutoRunning();
	if (bEnabled != bIsAutoRunning)
	{
		if (!bEnabled)
		{
			OnEndAutoRun();
		}
		else
		{
			OnStartAutoRun();
		}
	}
}

bool ARPGPlayerController::GetIsAutoRunning() const
{
	bool bIsAutoRunning = false;
	if (const URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		// Stub: We need to define RPG specific tags
		// bIsAutoRunning = RPGASC->GetTagCount(RPGGameplayTags::Status_AutoRunning) > 0;
	}
	return bIsAutoRunning;
}

void ARPGPlayerController::OnStartAutoRun()
{
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		// RPGASC->SetLooseGameplayTagCount(RPGGameplayTags::Status_AutoRunning, 1);
		K2_OnStartAutoRun();
	}	
}

void ARPGPlayerController::OnEndAutoRun()
{
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent())
	{
		// RPGASC->SetLooseGameplayTagCount(RPGGameplayTags::Status_AutoRunning, 0);
		K2_OnEndAutoRun();
	}
}

void ARPGPlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	if (bHideViewTargetPawnNextFrame)
	{
		// Simple implementation to hide view target pawn if needed
		bHideViewTargetPawnNextFrame = false;
	}
}

void ARPGPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	// Driven by player state usually in Lyra
}

FGenericTeamId ARPGPlayerController::GetGenericTeamId() const
{
	if (const IRPGTeamAgentInterface* PSWithTeamInterface = Cast<IRPGTeamAgentInterface>(PlayerState))
	{
		return PSWithTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

void ARPGPlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		const APlayerState* ThePlayerState = PlayerState.Get();
		if (IsValid(ThePlayerState))
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ThePlayerState))
			{
				if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
				{
					ASC->SetAvatarActor(nullptr);
				}
			}
		}
	}

	Super::OnUnPossess();
}
