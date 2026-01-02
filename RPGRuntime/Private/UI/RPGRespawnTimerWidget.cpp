// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RPGRespawnTimerWidget.h"
#include "CommonTextBlock.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/RPGRespawnMessage.h"
#include "Messages/RPGVerbMessage.h"
#include "System/RPGGameplayTags.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGRespawnTimerWidget)

URPGRespawnTimerWidget::URPGRespawnTimerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URPGRespawnTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start hidden
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen for respawn messages
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	
	MessageSubsystem.RegisterListener(RPGGameplayTags::Message_Respawn_Duration, this, &ThisClass::OnRespawnDurationMessage);
	MessageSubsystem.RegisterListener(RPGGameplayTags::Message_Respawn_Completed, this, &ThisClass::OnRespawnCompletedMessage);
}

void URPGRespawnTimerWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_UpdateCountdown);
	
	Super::NativeDestruct();
}

void URPGRespawnTimerWidget::OnRespawnDurationMessage(FGameplayTag Channel, const FRPGRespawnTimerDurationMessage& Payload)
{
	// Only care if it's for us
	if (Cast<APlayerState>(Payload.PlayerState.Get()) != GetOwningPlayerState())
	{
		return;
	}

	TargetRespawnTime = GetWorld()->GetTimeSeconds() + Payload.RespawnDelay;
	TotalRespawnDuration = Payload.RespawnDelay;

	// Show widget and play intro
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (Intro)
	{
		PlayAnimation(Intro);
	}

	// Start local countdown update
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_UpdateCountdown, this, &ThisClass::UpdateCountdown, 0.1f, true);
	
	// Initial update
	UpdateCountdown();

	// Notify Blueprint
	OnRespawnStarted(Payload.RespawnDelay);
}

void URPGRespawnTimerWidget::OnRespawnCompletedMessage(FGameplayTag Channel, const FRPGVerbMessage& Payload)
{
	// Only care if it's for us
	if (Cast<APlayerState>(Payload.Instigator.Get()) != GetOwningPlayerState())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_UpdateCountdown);
	
	// Play outro
	if (Outro)
	{
		PlayAnimation(Outro);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	// Notify Blueprint
	OnRespawnFinished();
}

void URPGRespawnTimerWidget::UpdateCountdown()
{
	const float TimeSeconds = GetWorld()->GetTimeSeconds();
	const float TimeRemaining = FMath::Max(0.0f, TargetRespawnTime - TimeSeconds);
	
	if (Time)
	{
		Time->SetText(FText::AsNumber(FMath::CeilToInt(TimeRemaining)));
	}

	if (FillGlow && TotalRespawnDuration > 0.0f)
	{
		const float Percent = FMath::Clamp(1.0f - (TimeRemaining / TotalRespawnDuration), 0.0f, 1.0f);
		FillGlow->GetDynamicMaterial()->SetScalarParameterValue(TEXT("AnimateArcFill"), Percent);
	}

	if (TimeRemaining <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_UpdateCountdown);
	}
}
