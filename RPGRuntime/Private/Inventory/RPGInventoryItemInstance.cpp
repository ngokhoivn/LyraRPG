#include "Inventory/RPGInventoryItemInstance.h"
#include "Inventory/RPGInventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGInventoryItemInstance)

URPGInventoryItemInstance::URPGInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URPGInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemDef);
}

void URPGInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void URPGInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 URPGInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool URPGInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void URPGInventoryItemInstance::SetItemDef(TSubclassOf<URPGInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
}

const URPGInventoryItemFragment* URPGInventoryItemInstance::FindFragmentByClass(TSubclassOf<URPGInventoryItemFragment> FragmentClass) const
{
	if (ItemDef && FragmentClass)
	{
		return GetDefault<URPGInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}
