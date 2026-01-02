// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "RPGGameData.generated.h"

/**
 * URPGGameData
 *
 * Non-mutable data asset that contains global game data for the RPG plugin.
 * Standalone version of LyraGameData.
 */
UCLASS(BlueprintType, Const)
class RPGRUNTIME_API URPGGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	URPGGameData();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DefaultAttributes")
	float DefaultHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DefaultAttributes")
	float DefaultMaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DefaultAttributes")
	float DefaultMana = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DefaultAttributes")
	float DefaultMaxMana = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DefaultAttributes")
	float DefaultStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DefaultAttributes")
	float DefaultMaxStamina = 100.0f;
};
