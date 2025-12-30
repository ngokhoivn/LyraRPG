#include "Inventory/RPGInventoryItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGInventoryItemDefinition)

//////////////////////////////////////////////////////////////////////
// URPGInventoryItemDefinition

URPGInventoryItemDefinition::URPGInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const URPGInventoryItemFragment* URPGInventoryItemDefinition::FindFragmentByClass(TSubclassOf<URPGInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (URPGInventoryItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////
// URPGInventoryFunctionLibrary

const URPGInventoryItemFragment* URPGInventoryFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<URPGInventoryItemDefinition> ItemDef, TSubclassOf<URPGInventoryItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<URPGInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}
