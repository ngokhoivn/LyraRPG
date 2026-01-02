// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/RPGGameFeatureAction_AddAbilities.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilitySet.h"
#include "AbilitySystem/RPGGameplayAbility.h"
#include "System/RPGLogChannels.h"
#include "Player/RPGPlayerState.h"
#include "Character/RPGPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGameFeatureAction_AddAbilities)

#define LOCTEXT_NAMESPACE "RPGGameFeatures"

//////////////////////////////////////////////////////////////////////
// URPGGameFeatureAction_AddAbilities

void URPGGameFeatureAction_AddAbilities::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);

	if (!ensureAlways(ActiveData.ActiveExtensions.IsEmpty()) ||
		!ensureAlways(ActiveData.ComponentRequests.IsEmpty()))
	{
		ActiveData.ActiveExtensions.Empty();
		ActiveData.ComponentRequests.Empty();
	}

	Super::OnGameFeatureActivating(Context);
}

void URPGGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FPerContextData* ActiveData = ContextData.Find(Context);

	if (ensure(ActiveData))
	{
		for (auto It = ActiveData->ActiveExtensions.CreateIterator(); It; ++It)
		{
			RemoveActorAbilities(It.Key(), *ActiveData);
		}

		ActiveData->ComponentRequests.Empty();
		ContextData.Remove(Context);
	}
}

#if WITH_EDITOR
EDataValidationResult URPGGameFeatureAction_AddAbilities::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FRPGGameFeatureAbilitiesEntry& Entry : AbilitiesList)
	{
		if (Entry.ActorClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("EntryHasNullActor", "Null ActorClass at index {0} in AbilitiesList"), FText::AsNumber(EntryIndex)));
		}

		if (Entry.GrantedAbilities.IsEmpty() && Entry.GrantedAttributes.IsEmpty() && Entry.GrantedAbilitySets.IsEmpty())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("EntryHasNoAddOns", "Index {0} in AbilitiesList will do nothing (no granted abilities, attributes, or ability sets)"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}

	return Result;
}
#endif

void URPGGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			int32 EntryIndex = 0;
			for (const FRPGGameFeatureAbilitiesEntry& Entry : AbilitiesList)
			{
				if (!Entry.ActorClass.IsNull())
				{
					UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
						this, &URPGGameFeatureAction_AddAbilities::HandleActorExtension, EntryIndex, ChangeContext);
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);

					ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
				}
				EntryIndex++;
			}
		}
	}
}

void URPGGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext)
{
	URPGGameFeatureAction_AddAbilities::FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (ActiveData && AbilitiesList.IsValidIndex(EntryIndex))
	{
		const FRPGGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
		if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
		{
			RemoveActorAbilities(Actor, *ActiveData);
		}
		else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == ARPGPlayerState::NAME_RPGAbilityReady))
		{
			AddActorAbilities(Actor, Entry, *ActiveData);
		}
	}
}

void URPGGameFeatureAction_AddAbilities::AddActorAbilities(AActor* Actor, const FRPGGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
{
	check(Actor);
	if (!Actor->HasAuthority())
	{
		return;
	}

	// early out if Actor already has ability extensions applied
	if (ActiveData.ActiveExtensions.Find(Actor) != nullptr)
	{
		return;
	}

	URPGAbilitySystemComponent* RPGASC = Actor->FindComponentByClass<URPGAbilitySystemComponent>();
	if (!RPGASC)
	{
		if (URPGPawnExtensionComponent* PawnExtComp = Actor->FindComponentByClass<URPGPawnExtensionComponent>())
		{
			RPGASC = PawnExtComp->GetRPGAbilitySystemComponent();
		}
	}

	if (RPGASC)
	{
		UE_LOG(LogRPG, Log, TEXT("URPGGameFeatureAction_AddAbilities::AddActorAbilities: Granting abilities to %s"), *GetNameSafe(Actor));
		FActorExtensions AddedExtensions;
		AddedExtensions.Abilities.Reserve(AbilitiesEntry.GrantedAbilities.Num());
		AddedExtensions.Attributes.Reserve(AbilitiesEntry.GrantedAttributes.Num());
		AddedExtensions.AbilitySetHandles.Reserve(AbilitiesEntry.GrantedAbilitySets.Num());

		for (const FRPGAbilityGrant& Ability : AbilitiesEntry.GrantedAbilities)
		{
			if (!Ability.AbilityType.IsNull())
			{
				FGameplayAbilitySpec NewAbilitySpec(Ability.AbilityType.LoadSynchronous(), Ability.AbilityLevel);
				FGameplayAbilitySpecHandle AbilityHandle = RPGASC->GiveAbility(NewAbilitySpec);

				AddedExtensions.Abilities.Add(AbilityHandle);
			}
		}

		for (const FRPGAttributeSetGrant& Attributes : AbilitiesEntry.GrantedAttributes)
		{
			if (!Attributes.AttributeSetType.IsNull())
			{
				TSubclassOf<UAttributeSet> SetType = Attributes.AttributeSetType.LoadSynchronous();
				if (SetType)
				{
					UAttributeSet* NewSet = NewObject<UAttributeSet>(RPGASC->GetOwner(), SetType);
					if (!Attributes.InitializationData.IsNull())
					{
						UDataTable* InitData = Attributes.InitializationData.LoadSynchronous();
						if (InitData)
						{
							NewSet->InitFromMetaDataTable(InitData);
						}
					}

					AddedExtensions.Attributes.Add(NewSet);
					RPGASC->AddAttributeSetSubobject(NewSet);
				}
			}
		}

		for (const TSoftObjectPtr<const URPGAbilitySet>& SetPtr : AbilitiesEntry.GrantedAbilitySets)
		{
			if (const URPGAbilitySet* Set = SetPtr.LoadSynchronous())
			{
				Set->GiveToAbilitySystem(RPGASC, &AddedExtensions.AbilitySetHandles.AddDefaulted_GetRef(), Actor);
			}
		}

		ActiveData.ActiveExtensions.Add(Actor, AddedExtensions);
	}
}

void URPGGameFeatureAction_AddAbilities::RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData)
{
	if (URPGGameFeatureAction_AddAbilities::FActorExtensions* ActorExtensions = ActiveData.ActiveExtensions.Find(Actor))
	{
		URPGAbilitySystemComponent* RPGASC = Actor->FindComponentByClass<URPGAbilitySystemComponent>();
		if (!RPGASC)
		{
			if (URPGPawnExtensionComponent* PawnExtComp = Actor->FindComponentByClass<URPGPawnExtensionComponent>())
			{
				RPGASC = PawnExtComp->GetRPGAbilitySystemComponent();
			}
		}

		if (RPGASC)
		{
			for (UAttributeSet* AttribSetInstance : ActorExtensions->Attributes)
			{
				RPGASC->RemoveSpawnedAttribute(AttribSetInstance);
			}

			for (FGameplayAbilitySpecHandle AbilityHandle : ActorExtensions->Abilities)
			{
				RPGASC->ClearAbility(AbilityHandle);
			}

			for (FRPGAbilitySet_GrantedHandles& SetHandle : ActorExtensions->AbilitySetHandles)
			{
				SetHandle.TakeFromAbilitySystem(RPGASC);
			}
		}

		ActiveData.ActiveExtensions.Remove(Actor);
	}
}

#undef LOCTEXT_NAMESPACE
