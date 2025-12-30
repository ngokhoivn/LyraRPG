// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/Weapons/RPGAxeInstance.h"
#include "System/RPGAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGAxeInstance)

URPGAxeInstance::URPGAxeInstance()
{
}

TSubclassOf<UAnimInstance> URPGAxeInstance::PickBestAnimLayer_Implementation(bool bEquip, const FGameplayTagContainer& InCosmeticTags)
{
    if (bEquip)
    {
        return URPGAssetManager::GetSubclass(EquippedAnimLayerSoft);
    }
    
    return URPGAssetManager::GetSubclass(UnequippedAnimLayerSoft);
}
