// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilityTagRelationshipMapping.h"
#include "AbilitySystem/RPGGameplayAbility.h"
#include "System/RPGLogChannels.h"
#include "AttributeSet.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGAbilitySystemComponent)

UE_DEFINE_GAMEPLAY_TAG(TAG_RPG_AbilityInputBlocked, "RPG.AbilityInputBlocked");

URPGAbilitySystemComponent::URPGAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void URPGAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Removed LyraGlobalAbilitySystem dependency for now.
	// If needed, we can implement an RPGGlobalAbilitySystem.

	Super::EndPlay(EndPlayReason);
}

void URPGAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (bHasNewPawnAvatar)
	{
		// Removed LyraAnimInstance and LyraGlobalAbilitySystem dependencies for now.
		// These will need to be replaced with RPG equivalents when ready.

		TryActivateAbilitiesOnSpawn();
	}
}

void URPGAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const URPGGameplayAbility* RPGAbilityCDO = Cast<URPGGameplayAbility>(AbilitySpec.Ability))
		{
			RPGAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}

void URPGAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		// Cancel all the spawned instances.
		TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : Instances)
		{
			URPGGameplayAbility* RPGAbilityInstance = CastChecked<URPGGameplayAbility>(AbilityInstance);

			if (ShouldCancelFunc(RPGAbilityInstance, AbilitySpec.Handle))
			{
				if (RPGAbilityInstance->CanBeCanceled())
				{
					RPGAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), RPGAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
				}
			}
		}
	}
}

void URPGAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const URPGGameplayAbility* RPGAbility, FGameplayAbilitySpecHandle Handle)
	{
		const ERPGAbilityActivationPolicy ActivationPolicy = RPGAbility->GetActivationPolicy();
		return ((ActivationPolicy == ERPGAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == ERPGAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void URPGAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	if (Spec.IsActive())
	{
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void URPGAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
}

void URPGAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				UE_LOG(LogRPG, Log, TEXT("AbilityInputTagPressed: Matching tag %s with ability %s"), *InputTag.ToString(), *GetNameSafe(AbilitySpec.Ability));
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void URPGAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void URPGAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_RPG_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	//
	// Process all abilities that activate when the input is held.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const URPGGameplayAbility* RPGAbilityCDO = Cast<URPGGameplayAbility>(AbilitySpec->Ability);
				if (RPGAbilityCDO && RPGAbilityCDO->GetActivationPolicy() == ERPGAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				UE_LOG(LogRPG, Log, TEXT("ProcessAbilityInput: Found Ability %s for Input"), *GetNameSafe(AbilitySpec->Ability));
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const URPGGameplayAbility* RPGAbilityCDO = Cast<URPGGameplayAbility>(AbilitySpec->Ability);

					if (RPGAbilityCDO && RPGAbilityCDO->GetActivationPolicy() == ERPGAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	// Try to activate all the abilities that are from presses and holds.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void URPGAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void URPGAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(RPGAbility->GetActivationGroup(), RPGAbility);
	}
}

void URPGAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	HandleAbilityFailed(Ability, FailureReason);
}

void URPGAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(RPGAbility->GetActivationGroup(), RPGAbility);
	}
}

void URPGAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);
}

void URPGAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);
}

void URPGAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

void URPGAbilitySystemComponent::SetTagRelationshipMapping(URPGAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void URPGAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void URPGAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	UE_LOG(LogRPG, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetNameSafe(Ability), *FailureReason.ToString());

	if (const URPGGameplayAbility* RPGAbility = Cast<const URPGGameplayAbility>(Ability))
	{
		RPGAbility->OnAbilityFailedToActivate(FailureReason);
	}	
}

bool URPGAbilitySystemComponent::IsActivationGroupBlocked(ERPGAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case ERPGAbilityActivationGroup::Independent:
		bBlocked = false;
		break;

	case ERPGAbilityActivationGroup::Exclusive_Replaceable:
	case ERPGAbilityActivationGroup::Exclusive_Blocking:
		bBlocked = (ActivationGroupCounts[(uint8)ERPGAbilityActivationGroup::Exclusive_Blocking] > 0);
		break;

	default:
		break;
	}

	return bBlocked;
}

void URPGAbilitySystemComponent::AddAbilityToActivationGroup(ERPGAbilityActivationGroup Group, URPGGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case ERPGAbilityActivationGroup::Independent:
		break;

	case ERPGAbilityActivationGroup::Exclusive_Replaceable:
	case ERPGAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(ERPGAbilityActivationGroup::Exclusive_Replaceable, Ability, bReplicateCancelAbility);
		break;

	default:
		break;
	}
}

void URPGAbilitySystemComponent::RemoveAbilityFromActivationGroup(ERPGAbilityActivationGroup Group, URPGGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void URPGAbilitySystemComponent::CancelActivationGroupAbilities(ERPGAbilityActivationGroup Group, URPGGameplayAbility* IgnoreAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const URPGGameplayAbility* RPGAbility, FGameplayAbilitySpecHandle Handle)
	{
		return ((RPGAbility->GetActivationGroup() == Group) && (RPGAbility != IgnoreAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void URPGAbilitySystemComponent::AddDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	// This relies on RPGAssetManager and RPGGameData which are not yet standaloned.
}

void URPGAbilitySystemComponent::RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	// This relies on RPGAssetManager and RPGGameData which are not yet standaloned.
}

void URPGAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

const UAttributeSet* URPGAbilitySystemComponent::GetOrCreateAttributeSet(const TSubclassOf<UAttributeSet>& AttributeSetClass)
{
	check(AttributeSetClass);

	for (UAttributeSet* Set : GetSpawnedAttributes())
	{
		if (Set && Set->IsA(AttributeSetClass))
		{
			return Set;
		}
	}

	UAttributeSet* AttributeSet = NewObject<UAttributeSet>(GetOwner(), AttributeSetClass);
	AddAttributeSetSubobject(AttributeSet);
	return AttributeSet;
}
