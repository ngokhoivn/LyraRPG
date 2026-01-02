// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameMode/RPGGameFeatureAction_WorldActionBase.h"
#include "AbilitySystem/RPGAbilitySet.h"
#include "AbilitySystem/RPGGameplayAbility.h"
#include "RPGGameFeatureAction_AddAbilities.generated.h"
class UAttributeSet;
class UDataTable;
class URPGAbilitySystemComponent;

/**
 * FRPGAbilityGrant
 *
 *  Structured used by the AddAbilities action to grant individual abilities.
 */
USTRUCT(BlueprintType)
struct FRPGAbilityGrant
{
	GENERATED_BODY()

	// Ability to grant.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Abilities)
	TSoftClassPtr<URPGGameplayAbility> AbilityType;

	// Level to grant the ability at.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Abilities)
	int32 AbilityLevel = 1;
};

/**
 * FRPGAttributeSetGrant
 *
 *  Structured used by the AddAbilities action to grant attribute sets.
 */
USTRUCT(BlueprintType)
struct FRPGAttributeSetGrant
{
	GENERATED_BODY()

	// Attribute set to grant.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attributes)
	TSoftClassPtr<UAttributeSet> AttributeSetType;

	// Data table to initialize the attribute set from.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attributes)
	TSoftObjectPtr<UDataTable> InitializationData;
};

/**
 * FRPGGameFeatureAbilitiesEntry
 *
 *  Entry used in the AddAbilities action to configure what and where to grant.
 */
USTRUCT(BlueprintType)
struct FRPGGameFeatureAbilitiesEntry
{
	GENERATED_BODY()

	// The base class of the actor to add the abilities to.
	UPROPERTY(EditAnywhere, Category=Abilities)
	TSoftClassPtr<AActor> ActorClass;

	// List of abilities to grant.
	UPROPERTY(EditAnywhere, Category=Abilities)
	TArray<FRPGAbilityGrant> GrantedAbilities;

	// List of attribute sets to grant.
	UPROPERTY(EditAnywhere, Category=Attributes)
	TArray<FRPGAttributeSetGrant> GrantedAttributes;

	// List of ability sets to grant.
	UPROPERTY(EditAnywhere, Category=Abilities)
	TArray<TSoftObjectPtr<const URPGAbilitySet>> GrantedAbilitySets;
};

/**
 * GameFeatureAction that adds gameplay abilities, attribute sets, and ability sets to actors.
 * Standalone version of Lyra's GameFeatureAction_AddAbilities.
 */
UCLASS(meta = (DisplayName = "Add Abilities (RPG)"))
class RPGRUNTIME_API URPGGameFeatureAction_AddAbilities final : public URPGGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UGameFeatureAction interface

private:
	//~ Begin URPGGameFeatureAction_WorldActionBase interface
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End URPGGameFeatureAction_WorldActionBase interface

	struct FActorExtensions
	{
		TArray<FGameplayAbilitySpecHandle> Abilities;
		TArray<TObjectPtr<UAttributeSet>> Attributes;
		TArray<FRPGAbilitySet_GrantedHandles> AbilitySetHandles;
	};

	struct FPerContextData
	{
		TMap<AActor*, FActorExtensions> ActiveExtensions;
		TArray<TSharedPtr<struct FComponentRequestHandle>> ComponentRequests;
	};

	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext);
	void AddActorAbilities(AActor* Actor, const FRPGGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);
	void RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData);

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	UPROPERTY(EditAnywhere, Category="Abilities", meta=(TitleProperty="ActorClass"))
	TArray<FRPGGameFeatureAbilitiesEntry> AbilitiesList;
};
