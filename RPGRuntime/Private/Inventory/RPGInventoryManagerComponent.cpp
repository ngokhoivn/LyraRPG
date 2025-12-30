#include "Inventory/RPGInventoryManagerComponent.h"
#include "Inventory/RPGInventoryItemDefinition.h"
#include "Inventory/RPGInventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "System/RPGGameplayTags.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGInventoryManagerComponent)

//////////////////////////////////////////////////////////////////////
// FRPGInventoryList

void FRPGInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FRPGInventoryEntry& Entry = Entries[Index];
		BroadcastChangeMessage(Entry, Entry.StackCount, 0);
		Entry.LastObservedCount = 0;
	}
}

void FRPGInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FRPGInventoryEntry& Entry = Entries[Index];
		BroadcastChangeMessage(Entry, 0, Entry.StackCount);
		Entry.LastObservedCount = Entry.StackCount;
	}
}

void FRPGInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FRPGInventoryEntry& Entry = Entries[Index];
		BroadcastChangeMessage(Entry, Entry.LastObservedCount, Entry.StackCount);
		Entry.LastObservedCount = Entry.StackCount;
	}
}

void FRPGInventoryList::BroadcastChangeMessage(FRPGInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FRPGInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	if (UWorld* World = OwnerComponent->GetWorld())
	{
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(World);
		MessageSystem.BroadcastMessage(FRPGGameplayTags::Get().Message_Inventory_StackChanged, Message);
	}
}

URPGInventoryItemInstance* FRPGInventoryList::AddEntry(TSubclassOf<URPGInventoryItemDefinition> ItemDef, int32 StackCount)
{
	if (!ItemDef || !OwnerComponent) return nullptr;

	AActor* OwningActor = OwnerComponent->GetOwner();
	if (!OwningActor->HasAuthority()) return nullptr;

	FRPGInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<URPGInventoryItemInstance>(OwningActor);
	NewEntry.Instance->SetItemDef(ItemDef);
	
	for (URPGInventoryItemFragment* Fragment : GetDefault<URPGInventoryItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment) Fragment->OnInstanceCreated(NewEntry.Instance);
	}

	NewEntry.StackCount = StackCount;
	MarkItemDirty(NewEntry);

	return NewEntry.Instance;
}

void FRPGInventoryList::RemoveEntry(URPGInventoryItemInstance* Instance)
{
	for (auto It = Entries.CreateIterator(); It; ++It)
	{
		if (It->Instance == Instance)
		{
			It.RemoveCurrent();
			MarkArrayDirty();
			return;
		}
	}
}

TArray<URPGInventoryItemInstance*> FRPGInventoryList::GetAllItems() const
{
	TArray<URPGInventoryItemInstance*> Results;
	for (const FRPGInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance) Results.Add(Entry.Instance);
	}
	return Results;
}

//////////////////////////////////////////////////////////////////////
// URPGInventoryManagerComponent

URPGInventoryManagerComponent::URPGInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void URPGInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, InventoryList);
}

bool URPGInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<URPGInventoryItemDefinition> ItemDef, int32 StackCount)
{
	return true;
}

URPGInventoryItemInstance* URPGInventoryManagerComponent::AddItemDefinition(TSubclassOf<URPGInventoryItemDefinition> ItemDef, int32 StackCount)
{
	URPGInventoryItemInstance* Result = InventoryList.AddEntry(ItemDef, StackCount);
	if (Result)
	{
		AddReplicatedSubObject(Result);
	}
	return Result;
}

void URPGInventoryManagerComponent::RemoveItemInstance(URPGInventoryItemInstance* ItemInstance)
{
	RemoveReplicatedSubObject(ItemInstance);
	InventoryList.RemoveEntry(ItemInstance);
}

TArray<URPGInventoryItemInstance*> URPGInventoryManagerComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

URPGInventoryItemInstance* URPGInventoryManagerComponent::FindFirstItemStackByDefinition(TSubclassOf<URPGInventoryItemDefinition> ItemDef) const
{
	for (const FRPGInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->GetItemDef() == ItemDef) return Entry.Instance;
	}
	return nullptr;
}

bool URPGInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (FRPGInventoryEntry& Entry : InventoryList.Entries)
	{
		if (IsValid(Entry.Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Entry.Instance, *Bunch, *RepFlags);
		}
	}
	return WroteSomething;
}

void URPGInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	for (const FRPGInventoryEntry& Entry : InventoryList.Entries)
	{
		if (IsValid(Entry.Instance))
		{
			AddReplicatedSubObject(Entry.Instance);
		}
	}
}
