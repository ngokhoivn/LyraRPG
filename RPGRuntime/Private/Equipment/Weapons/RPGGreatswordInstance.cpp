#include "Equipment/Weapons/RPGGreatswordInstance.h"
#include "Animation/AnimInstance.h"
#include "UObject/ConstructorHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGreatswordInstance)

URPGGreatswordInstance::URPGGreatswordInstance()
{
    // Set default animation layers (Note: These paths must exist in your project content)
    static ConstructorHelpers::FClassFinder<UAnimInstance> GreatswordAnimLayerClass(TEXT("/Game/RPG/Animations/ABP_Player_Greatsword.ABP_Player_Greatsword_C"));
    if (GreatswordAnimLayerClass.Succeeded())
    {
        EquippedAnimLayer = GreatswordAnimLayerClass.Class;
    }

    static ConstructorHelpers::FClassFinder<UAnimInstance> UnarmedAnimLayerClass(TEXT("/Game/RPG/Animations/ABP_Player_Unarmed.ABP_Player_Unarmed_C"));
    if (UnarmedAnimLayerClass.Succeeded())
    {
        UnequippedAnimLayer = UnarmedAnimLayerClass.Class;
    }
}

TSubclassOf<UAnimInstance> URPGGreatswordInstance::PickBestAnimLayer_Implementation(bool bEquip, const FGameplayTagContainer& InCosmeticTags)
{
    // Greatsword-specific logic here (can be expanded to use CosmeticTags)
    return bEquip ? EquippedAnimLayer : UnequippedAnimLayer;
}
