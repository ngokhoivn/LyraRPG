// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "RPGExperienceDefinition.generated.h"

class UGameFeatureAction;
class URPGPawnData;
class URPGExperienceActionSet;

/**
 * URPGExperienceDefinition
 * 
 * Definition of an experience.
 * Standalone version of LyraExperienceDefinition for the RPG plugin.
 */
UCLASS(BlueprintType, Const)
class RPGRUNTIME_API URPGExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	URPGExperienceDefinition();

	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

public:
	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;

	/** The default pawn class to spawn for players */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TObjectPtr<const URPGPawnData> DefaultPawnData;

	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// List of additional action sets to compose into this experience
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<TObjectPtr<URPGExperienceActionSet>> ActionSets;
};
