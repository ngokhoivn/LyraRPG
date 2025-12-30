#pragma once

#include "System/RPGGameplayTagStack.h"
#include "Templates/SubclassOf.h"
#include "RPGInventoryItemInstance.generated.h"

class URPGInventoryItemDefinition;
class URPGInventoryItemFragment;
struct FRPGInventoryList;

/**
 * URPGInventoryItemInstance
 * Represents a runtime instance of an inventory item.
 */
UCLASS(BlueprintType)
class RPGRUNTIME_API URPGInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	URPGInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool IsSupportedForNetworking() const override { return true; }

	// Stat Tag Stacks
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasStatTag(FGameplayTag Tag) const;

	// Definition Access
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TSubclassOf<URPGInventoryItemDefinition> GetItemDef() const { return ItemDef; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (DeterminesOutputType = "FragmentClass"), Category = "Inventory")
	const URPGInventoryItemFragment* FindFragmentByClass(TSubclassOf<URPGInventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return Cast<ResultClass>(FindFragmentByClass(ResultClass::StaticClass()));
	}

private:
	void SetItemDef(TSubclassOf<URPGInventoryItemDefinition> InDef);

	friend struct FRPGInventoryList;

protected:
	UPROPERTY(Replicated)
	FRPGGameplayTagStackContainer StatTags;

	UPROPERTY(Replicated)
	TSubclassOf<URPGInventoryItemDefinition> ItemDef;
};
