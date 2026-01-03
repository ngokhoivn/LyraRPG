// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "RPGHealthBar.generated.h"

class UImage;
class UCommonNumericTextBlock;
class URPGHealthComponent;
class UMaterialInstanceDynamic;
class UWidgetAnimation;

/**
 * RPG Health Bar
 * - Giữ nguyên material Lyra
 * - Nội suy mượt bằng C++ (Tick thông minh)
 */
UCLASS()
class RPGRUNTIME_API URPGHealthBar : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	URPGHealthBar(const FObjectInitializer& ObjectInitializer);

protected:
	//~UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	/* =========================
	 *  Health Update
	 * ========================= */
	void UpdateHealthBar(float OldValueNorm, float NewValueNorm, bool bSnap = false);
	void UpdateVisuals(float VisualValueNorm);
	void InitializeBarVisuals();

	UFUNCTION()
	void HandleHealthChanged(URPGHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

protected:
	/* =========================
	 *  Materials
	 * ========================= */
	void SetDynamicMaterials();
	void ResetAnimatedState();

	UMaterialInstanceDynamic* CreateAndAssignMID(UImage* TargetImage, UMaterialInstanceDynamic* CurrentMID);

protected:
	/* =========================
	 *  Widgets
	 * ========================= */
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> BarFill = nullptr;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> BarBorder = nullptr;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> BarGlow = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> HealthNumber = nullptr;

protected:
	/* =========================
	 *  State
	 * ========================= */
	UPROPERTY(Transient)
	TObjectPtr<URPGHealthComponent> CurrentHealthComponent = nullptr;

	float CurrentVisualHealthNorm = 1.0f;
	float TargetHealthNorm = 1.0f;
	bool bIsAnimating = false;

	UPROPERTY(EditDefaultsOnly, Category = "Health Bar")
	float VisualInterpSpeed = 6.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Health Bar")
	float AnimationDelay = 0.5f;

	float CurrentAnimationWaitTimer = 0.0f;
	float CurrentDamageOrHealingState = -1.0f; // -1.0 for damage, 1.0 for healing

protected:
	/* =========================
	 *  Materials (MID)
	 * ========================= */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BarFillMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BarBorderMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BarGlowMID = nullptr;

protected:
	/* =========================
	 *  Material Parameters (Lyra)
	 * ========================= */
	static const FName Param_HealthCurrent;
	static const FName Param_HealthUpdated;
	static const FName Param_DamageOrHealing;
};
