#pragma once

#include "Equipment/RPGWeaponInstance.h"
#include "RPGGreatswordInstance.generated.h"

/**
 * URPGGreatswordInstance
 * 
 * Specialized weapon instance for Greatswords with unique properties and animation layers.
 */
UCLASS(Blueprintable)
class RPGRUNTIME_API URPGGreatswordInstance : public URPGWeaponInstance
{
    GENERATED_BODY()

public:
    URPGGreatswordInstance();

    // Greatsword-specific properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Greatsword")
    float HeavyAttackDamageMultiplier = 1.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Greatsword")
    float BlockEffectiveness = 0.7f;

    // Base animation layers (to be set in Editor)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    TSoftClassPtr<UAnimInstance> EquippedAnimLayerSoft;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    TSoftClassPtr<UAnimInstance> UnequippedAnimLayerSoft;

    // Override animation layer selection
    virtual TSubclassOf<UAnimInstance> PickBestAnimLayer_Implementation(bool bEquip, const FGameplayTagContainer& InCosmeticTags) override;
};
