// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/RPGCameraAssistInterface.h"
#include "GameFramework/PlayerController.h"
#include "Teams/RPGTeamAgentInterface.h"
#include "RPGPlayerController.generated.h"

class ARPGHUD;
class ARPGPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class URPGAbilitySystemComponent;
class UObject;
class UPlayer;
struct FFrame;

/**
 * ARPGPlayerController
 *
 *	The base player controller class used by this project.
 *  Standalone version of LyraPlayerController.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class RPGRUNTIME_API ARPGPlayerController : public APlayerController, public IRPGCameraAssistInterface, public IRPGTeamAgentInterface
{
	GENERATED_BODY()

public:

	ARPGPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "RPG|PlayerController")
	ARPGPlayerState* GetRPGPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "RPG|PlayerController")
	URPGAbilitySystemComponent* GetRPGAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "RPG|PlayerController")
	ARPGHUD* GetRPGHUD() const;

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~AController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetPlayer(UPlayer* InPlayer) override;
	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	//~IRPGCameraAssistInterface interface
	virtual void OnCameraPenetratingTarget() override;
	//~End of IRPGCameraAssistInterface interface
	
	//~IRPGTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~End of IRPGTeamAgentInterface interface

	UFUNCTION(BlueprintCallable, Category = "RPG|Character")
	void SetIsAutoRunning(const bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "RPG|Character")
	bool GetIsAutoRunning() const;

protected:
	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

private:
	void BroadcastOnPlayerStateChanged();

protected:
	void OnStartAutoRun();
	void OnEndAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartAutoRun"))
	void K2_OnStartAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndAutoRun"))
	void K2_OnEndAutoRun();

	bool bHideViewTargetPawnNextFrame = false;

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;
};
