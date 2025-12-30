#pragma once

#include "Components/ControllerComponent.h"
#include "RPGQuickbarComponent.generated.h"

class URPGInventoryItemInstance;
class URPGWeaponInstance;
class URPGEquipmentManagerComponent;

/**
 * URPGQuickbarComponent
 * Manages quick-access slots for items.
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class RPGRUNTIME_API URPGQuickbarComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	URPGQuickbarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "RPG|Quickbar")
	void CycleActiveSlotForward();

	UFUNCTION(BlueprintCallable, Category = "RPG|Quickbar")
	void CycleActiveSlotBackward();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "RPG|Quickbar")
	void SetActiveSlotIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "RPG|Quickbar")
	TArray<URPGInventoryItemInstance*> GetSlots() const { return Slots; }

	UFUNCTION(BlueprintCallable, Category = "RPG|Quickbar")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	UFUNCTION(BlueprintCallable, Category = "RPG|Quickbar")
	URPGInventoryItemInstance* GetActiveSlotItem() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "RPG|Quickbar")
	void AddItemToSlot(int32 SlotIndex, URPGInventoryItemInstance* Item);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "RPG|Quickbar")
	URPGInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

	virtual void BeginPlay() override;

private:
	void UnequipItemInSlot();
	void EquipItemInSlot();

	URPGEquipmentManagerComponent* FindEquipmentManager() const;

protected:
	UPROPERTY(EditAnywhere, Category = "RPG|Quickbar")
	int32 NumSlots = 3;

	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_ActiveSlotIndex();

private:
	UPROPERTY(ReplicatedUsing = OnRep_Slots)
	TArray<TObjectPtr<URPGInventoryItemInstance>> Slots;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveSlotIndex)
	int32 ActiveSlotIndex = -1;

	UPROPERTY()
	TObjectPtr<URPGWeaponInstance> EquippedItem;
};

/**
 * Messages for UI/Other systems
 */
USTRUCT(BlueprintType)
struct FRPGQuickBarSlotsChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<URPGInventoryItemInstance>> Slots;
};

USTRUCT(BlueprintType)
struct FRPGQuickBarActiveIndexChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 ActiveIndex = 0;
};
