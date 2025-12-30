// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModularGameMode.h"
#include "RPGGameMode.generated.h"

class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class URPGExperienceDefinition;
class URPGPawnData;

/**
 * FOnRPGGameModePlayerInitialized
 * 
 * Delegate called when a player or bot joins the game and finishes initialization.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRPGGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

/**
 * ARPGGameMode
 *
 * Base game mode class for the RPG plugin.
 * Standalone version of LyraGameMode.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class RPGRUNTIME_API ARPGGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:

	ARPGGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "RPG|Pawn")
	const URPGPawnData* GetPawnDataForController(const AController* InController) const;

	//~AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void InitGameState() override;
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	//~End of AGameModeBase interface

	// Restart (respawn) the specified player or bot next frame
	UFUNCTION(BlueprintCallable)
	void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// Agnostic version of PlayerCanRestart that can be used for both player bots and players
	virtual bool ControllerCanRestart(AController* Controller);

	// Delegate called on player initialization
	FOnRPGGameModePlayerInitialized OnGameModePlayerInitialized;

protected:	
	void OnExperienceLoaded(const URPGExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;

	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

	void HandleMatchAssignmentIfNotExpectingOne();
};
