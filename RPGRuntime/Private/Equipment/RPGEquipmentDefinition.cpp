#include "Equipment/RPGEquipmentDefinition.h"
#include "Equipment/RPGWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGEquipmentDefinition)

URPGEquipmentDefinition::URPGEquipmentDefinition()
    : WeaponInstanceClass(nullptr)
{
}

URPGWeaponInstance* URPGEquipmentDefinition::CreateWeaponInstance(UObject* Instigator) const
{
    if (!WeaponInstanceClass) return nullptr;

    UWorld* World = Instigator ? Instigator->GetWorld() : nullptr;
    if (!World) return nullptr;

    URPGWeaponInstance* WeaponInstance = NewObject<URPGWeaponInstance>(Instigator, WeaponInstanceClass);
    WeaponInstance->Initialize(Instigator, const_cast<URPGEquipmentDefinition*>(this));
    
    return WeaponInstance;
}