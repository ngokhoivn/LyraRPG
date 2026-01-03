// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/RPGGA_Combo.h"
#include "AbilitySystemComponent.h"
#include "System/RPGGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "System/RPGLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGA_Combo)

URPGGA_Combo::URPGGA_Combo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = ERPGAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ERPGAbilityActivationGroup::Exclusive_Blocking;
}

void URPGGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC || !ComboMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(ASC))
	{
		RPGASC->SetLooseGameplayTagCount(FRPGGameplayTags::Get().Status_Action_Combo, 1);
	}

	// 1. Start the first attack
	CurrentSectionName = TEXT("Attack1");
	bInputBuffered = false;

	// 2. Play Montage and wait for it to end
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("ComboPlayTask"),
		ComboMontage,
		1.0f,
		CurrentSectionName
	);

	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();

	// 3. Listen for Gameplay events (Damage and Transitions)
	// Transition events: Event.Combo.Transition.XXXX
	UAbilityTask_WaitGameplayEvent* WaitTransitionTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FRPGGameplayTags::Get().GameplayEvent_Combo_Transition,
		nullptr,
		false,
		false
	);
	WaitTransitionTask->EventReceived.AddDynamic(this, &ThisClass::HandleComboEvent);
	WaitTransitionTask->ReadyForActivation();

	// Damage events: Event.Combo.Damage
	UAbilityTask_WaitGameplayEvent* WaitDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FRPGGameplayTags::Get().GameplayEvent_Combo_Damage,
		nullptr,
		false,
		false
	);
	WaitDamageTask->EventReceived.AddDynamic(this, &ThisClass::HandleComboEvent);
	WaitDamageTask->ReadyForActivation();

	// 4. Bind Input 
	// In Lyra/RPG, we can override OnInputPressed if the component informs us
	// For simplicity in C++, we'll use a direct binding to the spec handle if possible, 
	// or rely on the AbilitySystemComponent's AbilityInputPressed delegate.
}

void URPGGA_Combo::HandleComboEvent(FGameplayEventData Payload)
{
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();

	// A. Handle Damage
	if (Payload.EventTag.MatchesTag(GameplayTags.GameplayEvent_Combo_Damage))
	{
		// Sát thương sẽ được xử lý ở đây (Sweep Trace hoặc Apply GE)
		// UI/VFX cũng có thể lắng nghe tag này
		UE_LOG(LogRPG, Log, TEXT("Combo: Gây sát thương tại Section %s"), *CurrentSectionName.ToString());
		return;
	}

	// B. Handle Transition
	if (Payload.EventTag.MatchesTag(GameplayTags.GameplayEvent_Combo_Transition))
	{
		// Nếu đã nhấn phím (Buffer), nhảy sang Section tiếp theo
		if (bInputBuffered)
		{
			// Lấy tên Section từ Tag (Ví dụ: Event.Combo.Transition.Attack2 -> Attack2)
			FString TagString = Payload.EventTag.ToString();
			FString SectionStr;
			if (TagString.Split(TEXT("."), nullptr, &SectionStr, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
			{
				AdvanceToNextSection(FName(*SectionStr));
			}
		}
	}
}

void URPGGA_Combo::AdvanceToNextSection(FName SectionName)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		// Nhảy Montage
		ASC->CurrentMontageJumpToSection(SectionName);
		
		// Cập nhật trạng thái
		CurrentSectionName = SectionName;
		bInputBuffered = false;
		
		UE_LOG(LogRPG, Log, TEXT("Combo: Nhảy sang Section %s"), *SectionName.ToString());
	}
}

void URPGGA_Combo::OnAbilityInputPressed()
{
	// Đánh dấu người chơi muốn đánh đòn tiếp theo
	bInputBuffered = true;
	UE_LOG(LogRPG, Log, TEXT("Combo: Đã nhận Input Buffer"));
}

void URPGGA_Combo::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	OnAbilityInputPressed();
}

void URPGGA_Combo::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (URPGAbilitySystemComponent* ASC = GetRPGAbilitySystemComponentFromActorInfo())
	{
		ASC->SetLooseGameplayTagCount(FRPGGameplayTags::Get().Status_Action_Combo, 0);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
