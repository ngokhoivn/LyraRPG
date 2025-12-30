#include "Inventory/RPGQuickbarComponent.h"
#include "Inventory/RPGInventoryItemInstance.h"
#include "Inventory/RPGInventoryItemDefinition.h"
#include "Equipment/RPGEquipmentManagerComponent.h"
#include "Equipment/RPGWeaponInstance.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "System/RPGGameplayTags.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGQuickbarComponent)

URPGQuickbarComponent::URPGQuickbarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void URPGQuickbarComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Slots);
	DOREPLIFETIME(ThisClass, ActiveSlotIndex);
}

void URPGQuickbarComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner()->HasAuthority() && Slots.Num() < NumSlots)
	{
		Slots.SetNum(NumSlots);
	}
}

void URPGQuickbarComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 2) return;
	int32 SlotToSearch = (ActiveSlotIndex < 0) ? 0 : (ActiveSlotIndex + 1) % Slots.Num();
	SetActiveSlotIndex(SlotToSearch);
}

void URPGQuickbarComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 2) return;
	int32 SlotToSearch = (ActiveSlotIndex <= 0) ? Slots.Num() - 1 : ActiveSlotIndex - 1;
	SetActiveSlotIndex(SlotToSearch);
}

void URPGQuickbarComponent::SetActiveSlotIndex_Implementation(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex) && ActiveSlotIndex != NewIndex)
	{
		UnequipItemInSlot();
		ActiveSlotIndex = NewIndex;
		EquipItemInSlot();
		OnRep_ActiveSlotIndex();
	}
}

URPGInventoryItemInstance* URPGQuickbarComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex].Get() : nullptr;
}

void URPGQuickbarComponent::AddItemToSlot(int32 SlotIndex, URPGInventoryItemInstance* Item)
{
	if (Slots.IsValidIndex(SlotIndex) && Item)
	{
		Slots[SlotIndex] = Item;
		OnRep_Slots();
	}
}

URPGInventoryItemInstance* URPGQuickbarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	URPGInventoryItemInstance* Result = nullptr;
	if (Slots.IsValidIndex(SlotIndex))
	{
		if (ActiveSlotIndex == SlotIndex)
		{
			UnequipItemInSlot();
			ActiveSlotIndex = -1;
		}
		Result = Slots[SlotIndex];
		Slots[SlotIndex] = nullptr;
		OnRep_Slots();
	}
	return Result;
}

void URPGQuickbarComponent::EquipItemInSlot()
{
	if (!Slots.IsValidIndex(ActiveSlotIndex)) return;
	URPGInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex];
	if (!SlotItem) return;

	if (const URPGInventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<URPGInventoryFragment_EquippableItem>())
	{
		if (URPGEquipmentManagerComponent* EquipManager = FindEquipmentManager())
		{
			EquippedItem = EquipManager->EquipItem(EquipInfo->EquipmentDefinition);
		}
	}
}

void URPGQuickbarComponent::UnequipItemInSlot()
{
	if (EquippedItem)
	{
		if (URPGEquipmentManagerComponent* EquipManager = FindEquipmentManager())
		{
			EquipManager->UnequipItem(EquippedItem);
		}
		EquippedItem = nullptr;
	}
}

URPGEquipmentManagerComponent* URPGQuickbarComponent::FindEquipmentManager() const
{
	if (AController* Controller = Cast<AController>(GetOwner()))
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			return Pawn->FindComponentByClass<URPGEquipmentManagerComponent>();
		}
	}
	return nullptr;
}

void URPGQuickbarComponent::OnRep_Slots()
{
	FRPGQuickBarSlotsChangedMessage Message;
	Message.Owner = GetOwner();
	Message.Slots = Slots;
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(FRPGGameplayTags::Get().Message_QuickBar_SlotsChanged, Message);
}

void URPGQuickbarComponent::OnRep_ActiveSlotIndex()
{
	FRPGQuickBarActiveIndexChangedMessage Message;
	Message.Owner = GetOwner();
	Message.ActiveIndex = ActiveSlotIndex;
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(FRPGGameplayTags::Get().Message_QuickBar_ActiveIndexChanged, Message);
}
