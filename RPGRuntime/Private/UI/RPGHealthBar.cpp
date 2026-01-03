// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RPGHealthBar.h"
#include "Character/RPGHealthComponent.h"

#include "Components/Image.h"
#include "CommonNumericTextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Animation/WidgetAnimation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGHealthBar)

/* =========================
 *  Lyra Material Params
 * ========================= */
const FName URPGHealthBar::Param_HealthCurrent(TEXT("Health_Current"));
const FName URPGHealthBar::Param_HealthUpdated(TEXT("Health_Updated"));
const FName URPGHealthBar::Param_DamageOrHealing(TEXT("DamageOrHealing"));

URPGHealthBar::URPGHealthBar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bHasScriptImplementedTick = true; // Cho phép Tick thủ công
}

void URPGHealthBar::NativeConstruct()
{
	Super::NativeConstruct();

	SetDynamicMaterials();
	ResetAnimatedState();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::HandlePossessedPawnChanged);
		HandlePossessedPawnChanged(nullptr, PC->GetPawn());
	}

	InitializeBarVisuals();
	bIsAnimating = false; // Mặc định không chạy nội suy
}

void URPGHealthBar::NativeDestruct()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.RemoveAll(this);
	}

	if (CurrentHealthComponent)
	{
		CurrentHealthComponent->OnHealthChangedDelegate.RemoveAll(this);
		CurrentHealthComponent = nullptr;
	}

	Super::NativeDestruct();
}

void URPGHealthBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsAnimating)
	{
		return;
	}

	if (CurrentAnimationWaitTimer > 0.0f)
	{
		CurrentAnimationWaitTimer -= InDeltaTime;
		// Even during delay, we call UpdateVisuals to ensure the main bar snaps 
		// if it hasn't already (though UpdateHealthBar calls it once).
		UpdateVisuals(CurrentVisualHealthNorm);
		return;
	}

	CurrentVisualHealthNorm = FMath::FInterpTo(
		CurrentVisualHealthNorm,
		TargetHealthNorm,
		InDeltaTime,
		VisualInterpSpeed
	);

	UpdateVisuals(CurrentVisualHealthNorm);

	if (FMath::IsNearlyEqual(CurrentVisualHealthNorm, TargetHealthNorm, 0.001f))
	{
		CurrentVisualHealthNorm = TargetHealthNorm;
		UpdateVisuals(CurrentVisualHealthNorm);

		bIsAnimating = false;
	}
}

/* =========================
 *  Health Logic
 * ========================= */

void URPGHealthBar::UpdateHealthBar(float OldValueNorm, float NewValueNorm, bool bSnap)
{
	TargetHealthNorm = NewValueNorm;

	if (bSnap)
	{
		CurrentVisualHealthNorm = NewValueNorm;
		UpdateVisuals(NewValueNorm);
		bIsAnimating = false;
		return;
	}

	bIsAnimating = true;
	CurrentAnimationWaitTimer = AnimationDelay;

	// "Ghost Bar" Logic: 
	// - Health_Current (Visual): Should temporarily stay at the old value during delay
	// - Health_Updated (Target): Represents the new real health immediately
	
	// Initial update to snap the main bar (Health_Updated)
	UpdateVisuals(CurrentVisualHealthNorm);

	// Detect Damage vs Healing
	if (NewValueNorm < OldValueNorm)
	{
		CurrentDamageOrHealingState = -1.0f; // Damage
	}
	else if (NewValueNorm > OldValueNorm)
	{
		CurrentDamageOrHealingState = 1.0f; // Healing
	}

	UpdateVisuals(CurrentVisualHealthNorm);

	// Death
	if (NewValueNorm <= 0.0f)
	{
	}
}

void URPGHealthBar::UpdateVisuals(float VisualValueNorm)
{
	// Material "Ingredients" mapping:
	// - Health_Current: The sliding/visual value (CurrentVisualHealthNorm)
	// - Health_Updated: The actual/target value (TargetHealthNorm)
	// - DamageOrHealing: -1.0 for Damage (Red), 1.0 for Healing (Green/Blue)
	
	for (UMaterialInstanceDynamic* MID : { BarFillMID.Get(), BarBorderMID.Get(), BarGlowMID.Get() })
	{
		if (MID)
		{
			MID->SetScalarParameterValue(Param_HealthCurrent, VisualValueNorm);
			MID->SetScalarParameterValue(Param_HealthUpdated, TargetHealthNorm);
			MID->SetScalarParameterValue(Param_DamageOrHealing, CurrentDamageOrHealingState);
		}
	}

	if (BarGlow)
	{
		bool bShowGlow = false;
		
		if (CurrentDamageOrHealingState < 0.0f) // Damage Mode
		{
			// Show if visual (old) is still sliding down to target
			bShowGlow = (VisualValueNorm > TargetHealthNorm + 0.001f);
		}
		else // Healing Mode
		{
			// Show if visual (old) is still sliding up to target
			bShowGlow = (VisualValueNorm < TargetHealthNorm - 0.001f);
		}

		BarGlow->SetVisibility(bShowGlow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (HealthNumber)
	{
		// Numeric display follows the interpolated sliding value for visual consistency
		HealthNumber->SetCurrentValue(VisualValueNorm * 100.0f);
	}
}

void URPGHealthBar::InitializeBarVisuals()
{
	const float HealthNorm = CurrentHealthComponent
		? CurrentHealthComponent->GetHealthNormalized()
		: 1.0f;

	CurrentVisualHealthNorm = HealthNorm;
	TargetHealthNorm = HealthNorm;

	UpdateVisuals(HealthNorm);
}

/* =========================
 *  Materials
 * ========================= */

UMaterialInstanceDynamic* URPGHealthBar::CreateAndAssignMID(
	UImage* TargetImage,
	UMaterialInstanceDynamic* CurrentMID)
{
	if (!TargetImage)
	{
		return nullptr;
	}

	if (CurrentMID)
	{
		return CurrentMID;
	}

	return TargetImage->GetDynamicMaterial();
}

void URPGHealthBar::SetDynamicMaterials()
{
	BarFillMID = CreateAndAssignMID(BarFill, BarFillMID);
	BarBorderMID = CreateAndAssignMID(BarBorder, BarBorderMID);
	BarGlowMID = CreateAndAssignMID(BarGlow, BarGlowMID);
}

void URPGHealthBar::ResetAnimatedState()
{
	// No longer driving manual Lyra damage params to avoid material mismatch
}

/* =========================
 *  Pawn / Health Binding
 * ========================= */

void URPGHealthBar::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (CurrentHealthComponent)
	{
		CurrentHealthComponent->OnHealthChangedDelegate.RemoveAll(this);
		CurrentHealthComponent = nullptr;
	}

	if (NewPawn)
	{
		if (URPGHealthComponent* HealthComp = URPGHealthComponent::FindHealthComponent(NewPawn))
		{
			CurrentHealthComponent = HealthComp;
			CurrentHealthComponent->OnHealthChangedDelegate.AddDynamic(
				this,
				&ThisClass::HandleHealthChanged
			);

			const float HealthNorm = CurrentHealthComponent->GetHealthNormalized();
			UpdateHealthBar(HealthNorm, HealthNorm, true);
		}
	}
}

void URPGHealthBar::HandleHealthChanged(
	URPGHealthComponent* HealthComponent,
	float OldValue,
	float NewValue,
	AActor* Instigator)
{
	if (!HealthComponent)
	{
		return;
	}

	const float MaxHealth = HealthComponent->GetMaxHealth();
	if (MaxHealth > 0.0f)
	{
		UpdateHealthBar(OldValue / MaxHealth, NewValue / MaxHealth);
	}
}
