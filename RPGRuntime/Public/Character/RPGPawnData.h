// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "RPGPawnData.generated.h"

class APawn;
class URPGAbilitySet;
class URPGAbilityTagRelationshipMapping;
class URPGCameraMode;
class URPGInputConfig;

/**
 * URPGPawnData
 *
 * Non-mutable data asset that contains properties used to define a pawn.
 * Standalone version of LyraPawnData for the RPG plugin.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "RPG Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class RPGRUNTIME_API URPGPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	URPGPawnData(const FObjectInitializer& ObjectInitializer);

public:
	// Class to instantiate for this pawn (should usually derive from ALyraPawn or ALyraCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Abilities")
	TArray<TObjectPtr<URPGAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Abilities")
	TObjectPtr<URPGAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Input")
	TObjectPtr<URPGInputConfig> InputConfig;

	// Default camera mode used by player controlled pawns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Camera")
	TSubclassOf<URPGCameraMode> DefaultCameraMode;

	// Attribute sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Attributes")
	TArray<TSubclassOf<class UAttributeSet>> Attributes;

	// Equipment to grant to this pawn at startup.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Equipment")
	TArray<TSubclassOf<class URPGEquipmentDefinition>> InitialEquipment;
};
