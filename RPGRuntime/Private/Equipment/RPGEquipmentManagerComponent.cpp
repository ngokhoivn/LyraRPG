#include "Equipment/RPGEquipmentManagerComponent.h"
#include "Equipment/RPGEquipmentDefinition.h"
#include "Equipment/RPGWeaponInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGEquipmentManagerComponent)

//////////////////////////////////////////////////////////////////////
// FRPGEquipmentList

void FRPGEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		const FRPGAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.Instance) Entry.Instance->Unequip();
	}
}

void FRPGEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FRPGAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.Instance) Entry.Instance->Equip();
	}
}

void FRPGEquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}

URPGWeaponInstance* FRPGEquipmentList::AddEntry(TSubclassOf<URPGEquipmentDefinition> EquipmentDefinition)
{
	if (!EquipmentDefinition || !OwnerComponent) return nullptr;

	AActor* OwningActor = OwnerComponent->GetOwner();
	if (!OwningActor->HasAuthority()) return nullptr;

	const URPGEquipmentDefinition* EquipmentCDO = GetDefault<URPGEquipmentDefinition>(EquipmentDefinition);
	TSubclassOf<URPGWeaponInstance> InstanceType = EquipmentCDO->WeaponInstanceClass;
	if (!InstanceType) InstanceType = URPGWeaponInstance::StaticClass();

	FRPGAppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.EquipmentDefinition = EquipmentDefinition;
	NewEntry.Instance = NewObject<URPGWeaponInstance>(OwningActor, InstanceType);
	NewEntry.Instance->Initialize(OwningActor, const_cast<URPGEquipmentDefinition*>(EquipmentCDO));
	NewEntry.Instance->Equip();

	MarkItemDirty(NewEntry);
	return NewEntry.Instance;
}

void FRPGEquipmentList::RemoveEntry(URPGWeaponInstance* Instance)
{
	for (auto It = Entries.CreateIterator(); It; ++It)
	{
		if (It->Instance == Instance)
		{
			Instance->Unequip();
			It.RemoveCurrent();
			MarkArrayDirty();
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// URPGEquipmentManagerComponent

URPGEquipmentManagerComponent::URPGEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), EquipmentList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void URPGEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, EquipmentList);
}

URPGWeaponInstance* URPGEquipmentManagerComponent::EquipItem(TSubclassOf<URPGEquipmentDefinition> EquipmentDefinition)
{
	URPGWeaponInstance* Result = EquipmentList.AddEntry(EquipmentDefinition);
	if (Result)
	{
		AddReplicatedSubObject(Result);
	}
	return Result;
}

void URPGEquipmentManagerComponent::UnequipItem(URPGWeaponInstance* ItemInstance)
{
	if (ItemInstance)
	{
		RemoveReplicatedSubObject(ItemInstance);
		EquipmentList.RemoveEntry(ItemInstance);
	}
}

URPGWeaponInstance* URPGEquipmentManagerComponent::GetFirstInstanceOfType(TSubclassOf<URPGWeaponInstance> InstanceType)
{
	for (FRPGAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (Entry.Instance && Entry.Instance->IsA(InstanceType)) return Entry.Instance;
	}
	return nullptr;
}

bool URPGEquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (FRPGAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (IsValid(Entry.Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Entry.Instance, *Bunch, *RepFlags);
		}
	}
	return WroteSomething;
}

void URPGEquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	for (const FRPGAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (IsValid(Entry.Instance))
		{
			AddReplicatedSubObject(Entry.Instance);
		}
	}
}

void URPGEquipmentManagerComponent::UninitializeComponent()
{
	TArray<URPGWeaponInstance*> AllInstances;
	for (const FRPGAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (Entry.Instance) AllInstances.Add(Entry.Instance);
	}

	for (URPGWeaponInstance* Instance : AllInstances)
	{
		UnequipItem(Instance);
	}

	Super::UninitializeComponent();
}
