#include "Equipment/Weapons/RPGGreatswordInstance.h"
#include "System/RPGAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGreatswordInstance)

URPGGreatswordInstance::URPGGreatswordInstance()
{
}

TSubclassOf<UAnimInstance> URPGGreatswordInstance::PickBestAnimLayer_Implementation(bool bEquip, const FGameplayTagContainer& InCosmeticTags)
{
    if (bEquip)
    {
        return URPGAssetManager::GetSubclass(EquippedAnimLayerSoft);
    }
    
    return URPGAssetManager::GetSubclass(UnequippedAnimLayerSoft);
}
