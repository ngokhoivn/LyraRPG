// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/Weapons/RPGAxeInstance.h"
#include "Animation/AnimInstance.h"
#include "UObject/ConstructorHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGAxeInstance)

URPGAxeInstance::URPGAxeInstance()
{
    // Set default animation layers (Note: These paths must exist in your project content)
    // For now, using similar paths to Greatsword, can be updated in editor or here
    static ConstructorHelpers::FClassFinder<UAnimInstance> AxeAnimLayerClass(TEXT("/Game/RPG/Animations/ABP_Player_Axe.ABP_Player_Axe_C"));
    if (AxeAnimLayerClass.Succeeded())
    {
        EquippedAnimLayer = AxeAnimLayerClass.Class;
    }

    static ConstructorHelpers::FClassFinder<UAnimInstance> UnarmedAnimLayerClass(TEXT("/Game/RPG/Animations/ABP_Player_Unarmed.ABP_Player_Unarmed_C"));
    if (UnarmedAnimLayerClass.Succeeded())
    {
        UnequippedAnimLayer = UnarmedAnimLayerClass.Class;
    }
}

TSubclassOf<UAnimInstance> URPGAxeInstance::PickBestAnimLayer_Implementation(bool bEquip, const FGameplayTagContainer& InCosmeticTags)
{
    // Axe-specific logic here (can be expanded to use CosmeticTags)
    return bEquip ? EquippedAnimLayer : UnequippedAnimLayer;
}
