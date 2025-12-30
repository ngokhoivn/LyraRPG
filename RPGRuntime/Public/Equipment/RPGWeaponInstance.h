#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "RPGWeaponInstance.generated.h"

class AActor;
class APawn;
class USkeletalMeshComponent;
class UAnimInstance;

USTRUCT(BlueprintType)
struct FRPGWeaponActorToSpawn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AActor> ActorToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName AttachSocket;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FTransform AttachTransform;

    FRPGWeaponActorToSpawn()
        : ActorToSpawn(nullptr)
        , AttachSocket(NAME_None)
        , AttachTransform(FTransform::Identity)
    {}
};

UCLASS(Blueprintable, BlueprintType)
class RPGRUNTIME_API URPGWeaponInstance : public UObject
{
    GENERATED_BODY()

public:
    URPGWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~UObject interface
    virtual bool IsSupportedForNetworking() const override { return true; }
    virtual UWorld* GetWorld() const override;
    //~End of UObject interface

    // Equipment Definition
    UFUNCTION(BlueprintCallable, Category = "RPG|Weapon")
    void Initialize(UObject* InInstigator, class URPGEquipmentDefinition* InDefinition);

    // Equipment State
    UFUNCTION(BlueprintCallable, Category = "RPG|Weapon")
    virtual void Equip();

    UFUNCTION(BlueprintCallable, Category = "RPG|Weapon")
    virtual void Unequip();

    UFUNCTION(BlueprintPure, Category = "RPG|Weapon")
    bool IsEquipped() const { return bIsEquipped; }

    // Animation
    UFUNCTION(BlueprintCallable, Category = "RPG|Weapon|Animation")
    void ActivateAnimLayer(bool bEquip);

    UFUNCTION(BlueprintNativeEvent, Category = "RPG|Weapon|Animation")
    TSubclassOf<UAnimInstance> PickBestAnimLayer(bool bEquip, const FGameplayTagContainer& InCosmeticTags);

    // Actor Management
    UFUNCTION(BlueprintCallable, Category = "RPG|Weapon")
    void SpawnWeaponActors();

    UFUNCTION(BlueprintCallable, Category = "RPG|Weapon")
    void DestroyWeaponActors();

    UFUNCTION(BlueprintCallable, Category = "RPG|Weapon")
    void SetActorsHidden(bool bHidden);

    // Owner Access
    UFUNCTION(BlueprintPure, Category = "RPG|Weapon")
    UObject* GetInstigator() const { return Instigator; }

    UFUNCTION(BlueprintPure, Category = "RPG|Weapon")
    APawn* GetPawn() const;

    template<typename T>
    T* GetTypedPawn() const
    {
        return Cast<T>(GetPawn());
    }

    // Cosmetic Tags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Weapon|Animation")
    FGameplayTagContainer CosmeticTags;

    // Actors to spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Weapon")
    TArray<FRPGWeaponActorToSpawn> ActorsToSpawn;

    // Equipped Animation Layer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Weapon|Animation")
    TSubclassOf<UAnimInstance> EquippedAnimLayer;

    // Unequipped Animation Layer  
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Weapon|Animation")
    TSubclassOf<UAnimInstance> UnequippedAnimLayer;

protected:
    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "RPG|Weapon", meta = (DisplayName = "OnEquipped"))
    void K2_OnEquipped();

    UFUNCTION(BlueprintImplementableEvent, Category = "RPG|Weapon", meta = (DisplayName = "OnUnequipped"))
    void K2_OnUnequipped();

private:
    UPROPERTY(Replicated)
    TObjectPtr<UObject> Instigator;

    UPROPERTY(Replicated)
    TArray<TObjectPtr<AActor>> SpawnedActors;

    UPROPERTY()
    TObjectPtr<class URPGEquipmentDefinition> EquipmentDefinition;

    bool bIsEquipped = false;
};