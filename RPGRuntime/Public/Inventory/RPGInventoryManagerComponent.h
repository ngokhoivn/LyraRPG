#pragma once

#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Templates/SubclassOf.h"
#include "RPGInventoryManagerComponent.generated.h"

class URPGInventoryItemDefinition;
class URPGInventoryItemInstance;
class URPGInventoryManagerComponent;

/**
 * FRPGInventoryChangeMessage
 * Message sent when an item stack changes.
 */
USTRUCT(BlueprintType)
struct FRPGInventoryChangeMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<URPGInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Delta = 0;
};

/**
 * FRPGInventoryEntry
 * A single entry in the inventory list.
 */
USTRUCT(BlueprintType)
struct FRPGInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FRPGInventoryEntry() {}

private:
	friend struct FRPGInventoryList;
	friend class URPGInventoryManagerComponent;

	UPROPERTY()
	TObjectPtr<URPGInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};

/**
 * FRPGInventoryList
 * Replicated list of inventory items.
 */
USTRUCT(BlueprintType)
struct FRPGInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FRPGInventoryList() : OwnerComponent(nullptr) {}
	FRPGInventoryList(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	TArray<URPGInventoryItemInstance*> GetAllItems() const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FRPGInventoryEntry, FRPGInventoryList>(Entries, DeltaParms, *this);
	}

	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	URPGInventoryItemInstance* AddEntry(TSubclassOf<URPGInventoryItemDefinition> ItemDef, int32 StackCount);
	void RemoveEntry(URPGInventoryItemInstance* Instance);

private:
	void BroadcastChangeMessage(FRPGInventoryEntry& Entry, int32 OldCount, int32 NewCount);

private:
	friend class URPGInventoryManagerComponent;

	UPROPERTY()
	TArray<FRPGInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FRPGInventoryList> : public TStructOpsTypeTraitsBase2<FRPGInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * URPGInventoryManagerComponent
 * Manages an inventory of items.
 */
UCLASS(BlueprintType)
class RPGRUNTIME_API URPGInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URPGInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool CanAddItemDefinition(TSubclassOf<URPGInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	URPGInventoryItemInstance* AddItemDefinition(TSubclassOf<URPGInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void RemoveItemInstance(URPGInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Inventory", BlueprintPure = false)
	TArray<URPGInventoryItemInstance*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory", BlueprintPure)
	URPGInventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<URPGInventoryItemDefinition> ItemDef) const;

	// Replication
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;

private:
	UPROPERTY(Replicated)
	FRPGInventoryList InventoryList;
};
