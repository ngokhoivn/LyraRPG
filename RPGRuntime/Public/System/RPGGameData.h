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

	// Add global gameplay tags or other static data here if needed.
};
