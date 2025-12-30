// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "RPGExperienceActionSet.generated.h"

class UGameFeatureAction;

/**
 * RPGExperienceActionSet
 *
 * Definition of a set of actions to perform as part of entering an experience.
 * This is a standalone version of Lyra's ExperienceActionSet for the RPG plugin.
 */
UCLASS(BlueprintType, NotBlueprintable)
class RPGRUNTIME_API URPGExperienceActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	URPGExperienceActionSet();

	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	//~UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

public:
	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditAnywhere, Instanced, Category="Actions to Perform")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditAnywhere, Category="Feature Dependencies")
	TArray<FString> GameFeaturesToEnable;
};
