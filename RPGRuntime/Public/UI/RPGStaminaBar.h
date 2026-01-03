// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "RPGStaminaBar.generated.h"

class UImage;
class UCommonNumericTextBlock;
class URPGHealthComponent;
class UMaterialInstanceDynamic;

/**
 * RPG Stamina Bar
 * Specialized widget for stamina display following Lyra patterns.
 */
UCLASS()
class RPGRUNTIME_API URPGStaminaBar : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	URPGStaminaBar(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	void UpdateStaminaBar(float OldValueNorm, float NewValueNorm, bool bSnap = false);
	void UpdateVisuals(float VisualValueNorm);
	void InitializeBarVisuals();

	UFUNCTION()
	void HandleStaminaChanged(URPGHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandleMaxStaminaChanged(URPGHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

protected:
	void SetDynamicMaterials();
	UMaterialInstanceDynamic* CreateAndAssignMID(UImage* TargetImage, UMaterialInstanceDynamic* CurrentMID);

protected:
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> BarFill = nullptr;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonNumericTextBlock> StaminaNumber = nullptr;

protected:
	UPROPERTY(Transient)
	TObjectPtr<URPGHealthComponent> CurrentHealthComponent = nullptr;

	float CurrentVisualStaminaNorm = 1.0f;
	float TargetStaminaNorm = 1.0f;
	bool bIsAnimating = false;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina Bar")
	float VisualInterpSpeed = 15.0f; // Stamina usually interps faster than health

protected:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BarFillMID = nullptr;

	static const FName Param_StaminaCurrent;
	static const FName Param_StaminaUpdated;
};
