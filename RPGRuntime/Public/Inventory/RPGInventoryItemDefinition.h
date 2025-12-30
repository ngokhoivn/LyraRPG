#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"
#include "Equipment/RPGEquipmentDefinition.h"
#include "RPGInventoryItemDefinition.generated.h"

class URPGInventoryItemInstance;

//////////////////////////////////////////////////////////////////////

/**
 * URPGInventoryItemFragment
 * Base class for modular data fragments that can be attached to an item definition.
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class RPGRUNTIME_API URPGInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(URPGInventoryItemInstance* Instance) const {}
};

//////////////////////////////////////////////////////////////////////

/**
 * URPGInventoryFragment_EquippableItem
 * Fragment that links an inventory item to an equipment definition (weapons, armor, etc.)
 */
UCLASS()
class RPGRUNTIME_API URPGInventoryFragment_EquippableItem : public URPGInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<URPGEquipmentDefinition> EquipmentDefinition;
};

//////////////////////////////////////////////////////////////////////

/**
 * URPGInventoryItemDefinition
 * Data asset that defines what an item is using a collection of fragments.
 */
UCLASS(Blueprintable, Const, Abstract)
class RPGRUNTIME_API URPGInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	URPGInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", Instanced)
	TArray<TObjectPtr<URPGInventoryItemFragment>> Fragments;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory", meta = (DetermineOutputType = "FragmentClass"))
	const URPGInventoryItemFragment* FindFragmentByClass(TSubclassOf<URPGInventoryItemFragment> FragmentClass) const;
};

//////////////////////////////////////////////////////////////////////

/**
 * URPGInventoryFunctionLibrary
 */
UCLASS()
class RPGRUNTIME_API URPGInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory", meta = (DeterminesOutputType = "FragmentClass"))
	static const URPGInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<URPGInventoryItemDefinition> ItemDef, TSubclassOf<URPGInventoryItemFragment> FragmentClass);
};
