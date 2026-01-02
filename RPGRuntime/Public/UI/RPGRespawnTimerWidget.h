// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UI/RPGUserWidget.h"
#include "GameplayTagContainer.h"
#include "RPGRespawnTimerWidget.generated.h"

struct FRPGRespawnTimerDurationMessage;
struct FRPGVerbMessage;
struct FGameplayMessageListenerHandle;
class UCommonTextBlock;

/**
 * URPGRespawnTimerWidget
 *
 * C++ base class for the respawn timer widget.
 */
UCLASS(Abstract, Blueprintable)
class RPGRUNTIME_API URPGRespawnTimerWidget : public URPGUserWidget
{
	GENERATED_BODY()

public:
	URPGRespawnTimerWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Handlers for gameplay messages
	void OnRespawnDurationMessage(FGameplayTag Channel, const FRPGRespawnTimerDurationMessage& Payload);
	void OnRespawnCompletedMessage(FGameplayTag Channel, const FRPGVerbMessage& Payload);

	/** Update the visual state of the widget */
	UFUNCTION(BlueprintImplementableEvent, Category = Respawn)
	void OnRespawnStarted(float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = Respawn)
	void OnRespawnFinished();

private:
	void UpdateCountdown();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Time;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> FillGlow;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Intro;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Outro;

	FTimerHandle TimerHandle_UpdateCountdown;
	
	float TargetRespawnTime = 0.0f;
	float TotalRespawnDuration = 0.0f;
};
