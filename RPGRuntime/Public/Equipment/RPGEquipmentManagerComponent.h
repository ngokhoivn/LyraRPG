#pragma once

#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Templates/SubclassOf.h"
#include "RPGEquipmentManagerComponent.generated.h"

class URPGEquipmentDefinition;
class URPGWeaponInstance;
class URPGEquipmentManagerComponent;
struct FRPGEquipmentList;

/**
 * FRPGAppliedEquipmentEntry
 * A single piece of applied equipment.
 */
USTRUCT(BlueprintType)
struct FRPGAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FRPGAppliedEquipmentEntry() {}

private:
	friend struct FRPGEquipmentList;
	friend class URPGEquipmentManagerComponent;

	UPROPERTY()
	TSubclassOf<URPGEquipmentDefinition> EquipmentDefinition;

	UPROPERTY()
	TObjectPtr<URPGWeaponInstance> Instance = nullptr;
};

/**
 * FRPGEquipmentList
 * Replicated list of equipped items.
 */
USTRUCT(BlueprintType)
struct FRPGEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	FRPGEquipmentList() : OwnerComponent(nullptr) {}
	FRPGEquipmentList(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FRPGAppliedEquipmentEntry, FRPGEquipmentList>(Entries, DeltaParms, *this);
	}

	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	URPGWeaponInstance* AddEntry(TSubclassOf<URPGEquipmentDefinition> EquipmentDefinition);
	void RemoveEntry(URPGWeaponInstance* Instance);

private:
	friend class URPGEquipmentManagerComponent;

	UPROPERTY()
	TArray<FRPGAppliedEquipmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FRPGEquipmentList> : public TStructOpsTypeTraitsBase2<FRPGEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * URPGEquipmentManagerComponent
 * Manages equipment applied to a pawn.
 */
UCLASS(BlueprintType)
class RPGRUNTIME_API URPGEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	URPGEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	URPGWeaponInstance* EquipItem(TSubclassOf<URPGEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void UnequipItem(URPGWeaponInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	URPGWeaponInstance* GetFirstInstanceOfType(TSubclassOf<URPGWeaponInstance> InstanceType);

	// Replication
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;

	// Lifecycle
	virtual void UninitializeComponent() override;

private:
	UPROPERTY(Replicated)
	FRPGEquipmentList EquipmentList;
};
