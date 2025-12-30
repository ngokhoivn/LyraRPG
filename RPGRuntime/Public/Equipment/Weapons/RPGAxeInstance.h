// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Equipment/RPGWeaponInstance.h"
#include "RPGAxeInstance.generated.h"

/**
 * URPGAxeInstance
 * 
 * Specialized weapon instance for Axes with unique properties and animation layers.
 */
UCLASS(Blueprintable)
class RPGRUNTIME_API URPGAxeInstance : public URPGWeaponInstance
{
    GENERATED_BODY()

public:
    URPGAxeInstance();

    // Axe-specific properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Axe")
    float BleedChance = 0.25f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Axe")
    float AttackSpeed = 1.1f;

    // Base animation layers (to be set in Editor)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    TSoftClassPtr<UAnimInstance> EquippedAnimLayerSoft;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    TSoftClassPtr<UAnimInstance> UnequippedAnimLayerSoft;

    // Override animation layer selection
    virtual TSubclassOf<UAnimInstance> PickBestAnimLayer_Implementation(bool bEquip, const FGameplayTagContainer& InCosmeticTags) override;
};
