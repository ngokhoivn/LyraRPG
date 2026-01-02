// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/RPGGameplayAbility.h"
#include "System/RPGLogChannels.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Camera/RPGCameraMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGameplayAbility)

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}

URPGGameplayAbility::URPGGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = ERPGAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ERPGAbilityActivationGroup::Independent;

	bLogCancelation = false;

	ActiveCameraMode = nullptr;
}

URPGAbilitySystemComponent* URPGGameplayAbility::GetRPGAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<URPGAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

APlayerController* URPGGameplayAbility::GetPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<APlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

APlayerState* URPGGameplayAbility::GetPlayerStateFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<APlayerState>(CurrentActorInfo->OwnerActor.Get()) : nullptr);
}

void URPGGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	// Removed GameplayMessageSubsystem dependency for now.
	// You can implement RPG-specific message broadcasting here later.
	for (FGameplayTag Reason : FailedReason)
	{
		UE_LOG(LogRPG, Warning, TEXT("Ability %s failed to activate: %s"), *GetName(), *Reason.ToString());
	}
}

bool URPGGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	URPGAbilitySystemComponent* RPGASC = CastChecked<URPGAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (RPGASC->IsActivationGroupBlocked(ActivationGroup))
	{
		return false;
	}

	return true;
}

void URPGGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	// The ability can not block canceling if it's replaceable.
	if (!bCanBeCanceled && (ActivationGroup == ERPGAbilityActivationGroup::Exclusive_Replaceable))
	{
		UE_LOG(LogRPG, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void URPGGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void URPGGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void URPGGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URPGGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearCameraMode();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool URPGGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void URPGGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

FGameplayEffectContextHandle URPGGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	return Super::MakeEffectContext(Handle, ActorInfo);
}

void URPGGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);
}

bool URPGGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}

	const URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	if (RPGASC)
	{
		RPGASC->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
	}

	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;
		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

void URPGGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	OnPawnAvatarSet();
}

void URPGGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}

void URPGGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	if (ActorInfo && !Spec.IsActive() && (ActivationPolicy == ERPGAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bIsLocallyControlled = ActorInfo->IsLocallyControlled();
			const bool bIsAuthority = ActorInfo->IsNetAuthority();

			const bool bClientShouldActivate = bIsLocallyControlled && bIsLocalExecution;
			const bool bServerShouldActivate = bIsAuthority && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

bool URPGGameplayAbility::CanChangeActivationGroup(ERPGAbilityActivationGroup NewGroup) const
{
	if (!IsInstantiated() || !IsActive())
	{
		return false;
	}

	if (ActivationGroup == NewGroup)
	{
		return true;
	}

	URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponentFromActorInfo();
	check(RPGASC);

	if ((ActivationGroup != ERPGAbilityActivationGroup::Exclusive_Blocking) && RPGASC->IsActivationGroupBlocked(NewGroup))
	{
		return false;
	}

	if ((NewGroup == ERPGAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
	{
		return false;
	}

	return true;
}

bool URPGGameplayAbility::ChangeActivationGroup(ERPGAbilityActivationGroup NewGroup)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);

	if (!CanChangeActivationGroup(NewGroup))
	{
		return false;
	}

	if (ActivationGroup != NewGroup)
	{
		URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponentFromActorInfo();
		check(RPGASC);

		RPGASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
		RPGASC->AddAbilityToActivationGroup(NewGroup, this);

		ActivationGroup = NewGroup;
	}

	return true;
}

void URPGGameplayAbility::SetCameraMode(TSubclassOf<URPGCameraMode> CameraMode)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(SetCameraMode, );
	// Implementation needs RPGHeroComponent or similar.
	ActiveCameraMode = CameraMode;
}

void URPGGameplayAbility::ClearCameraMode()
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ClearCameraMode, );
	if (ActiveCameraMode)
	{
		ActiveCameraMode = nullptr;
	}
}
