#pragma once

#include "Engine/DataAsset.h"
#include "RPGEquipmentDefinition.generated.h"

class URPGWeaponInstance;

UCLASS(BlueprintType, Const)
class RPGRUNTIME_API URPGEquipmentDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    URPGEquipmentDefinition();

    // Weapon instance class to spawn
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
    TSubclassOf<URPGWeaponInstance> WeaponInstanceClass;

    // Display info
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
    FText Description;

    // Create weapon instance
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    URPGWeaponInstance* CreateWeaponInstance(UObject* Instigator) const;
};