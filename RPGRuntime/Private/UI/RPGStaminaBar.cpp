// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RPGStaminaBar.h"
#include "Character/RPGHealthComponent.h"
#include "AbilitySystem/Attributes/RPGAttributeSet.h"

#include "Components/Image.h"
#include "CommonNumericTextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGStaminaBar)

const FName URPGStaminaBar::Param_StaminaCurrent(TEXT("Health_Current")); 
const FName URPGStaminaBar::Param_StaminaUpdated(TEXT("Health_Updated")); // New target value for ghost bar logic

URPGStaminaBar::URPGStaminaBar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bHasScriptImplementedTick = true;
}

void URPGStaminaBar::NativeConstruct()
{
	Super::NativeConstruct();

	SetDynamicMaterials();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::HandlePossessedPawnChanged);
		HandlePossessedPawnChanged(nullptr, PC->GetPawn());
	}

	InitializeBarVisuals();
}

void URPGStaminaBar::NativeDestruct()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.RemoveAll(this);
	}

	if (CurrentHealthComponent)
	{
		CurrentHealthComponent->OnStaminaChangedDelegate.RemoveAll(this);
		CurrentHealthComponent->OnMaxStaminaChangedDelegate.RemoveAll(this);
		CurrentHealthComponent = nullptr;
	}

	Super::NativeDestruct();
}

void URPGStaminaBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsAnimating)
	{
		return;
	}

	CurrentVisualStaminaNorm = FMath::FInterpTo(
		CurrentVisualStaminaNorm,
		TargetStaminaNorm,
		InDeltaTime,
		VisualInterpSpeed
	);

	UpdateVisuals(CurrentVisualStaminaNorm);

	if (FMath::IsNearlyEqual(CurrentVisualStaminaNorm, TargetStaminaNorm, 0.001f))
	{
		CurrentVisualStaminaNorm = TargetStaminaNorm;
		UpdateVisuals(CurrentVisualStaminaNorm);
		bIsAnimating = false;
	}
}

void URPGStaminaBar::UpdateStaminaBar(float OldValueNorm, float NewValueNorm, bool bSnap)
{
	TargetStaminaNorm = NewValueNorm;

	if (bSnap)
	{
		CurrentVisualStaminaNorm = NewValueNorm;
		UpdateVisuals(NewValueNorm);
		bIsAnimating = false;
		return;
	}

	bIsAnimating = true;
}

void URPGStaminaBar::UpdateVisuals(float VisualValueNorm)
{
	if (BarFillMID)
	{
		BarFillMID->SetScalarParameterValue(Param_StaminaCurrent, VisualValueNorm);
		BarFillMID->SetScalarParameterValue(Param_StaminaUpdated, TargetStaminaNorm);
	}

	if (StaminaNumber)
	{
		StaminaNumber->SetCurrentValue(VisualValueNorm * 100.0f);
	}
}

void URPGStaminaBar::InitializeBarVisuals()
{
	if (CurrentHealthComponent)
	{
		const float Stamina = CurrentHealthComponent->GetStamina();
		const float MaxStamina = CurrentHealthComponent->GetMaxStamina();
		const float StaminaNorm = (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 1.0f;

		CurrentVisualStaminaNorm = StaminaNorm;
		TargetStaminaNorm = StaminaNorm;
		UpdateVisuals(StaminaNorm);
	}
}

void URPGStaminaBar::HandleStaminaChanged(URPGHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	if (HealthComponent)
	{
		const float MaxStamina = HealthComponent->GetMaxStamina();
		if (MaxStamina > 0.0f)
		{
			UpdateStaminaBar(OldValue / MaxStamina, NewValue / MaxStamina);
		}
	}
}

void URPGStaminaBar::HandleMaxStaminaChanged(URPGHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	if (HealthComponent)
	{
		const float Stamina = HealthComponent->GetStamina();
		if (NewValue > 0.0f)
		{
			UpdateStaminaBar(Stamina / OldValue, Stamina / NewValue, true);
		}
	}
}

void URPGStaminaBar::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (CurrentHealthComponent)
	{
		CurrentHealthComponent->OnStaminaChangedDelegate.RemoveAll(this);
		CurrentHealthComponent->OnMaxStaminaChangedDelegate.RemoveAll(this);
		CurrentHealthComponent = nullptr;
	}

	if (NewPawn)
	{
		if (URPGHealthComponent* HealthComp = URPGHealthComponent::FindHealthComponent(NewPawn))
		{
			CurrentHealthComponent = HealthComp;
			CurrentHealthComponent->OnStaminaChangedDelegate.AddDynamic(this, &ThisClass::HandleStaminaChanged);
			CurrentHealthComponent->OnMaxStaminaChangedDelegate.AddDynamic(this, &ThisClass::HandleMaxStaminaChanged);

			InitializeBarVisuals();
		}
	}
}

void URPGStaminaBar::SetDynamicMaterials()
{
	if (BarFill && !BarFillMID)
	{
		BarFillMID = BarFill->GetDynamicMaterial();
	}
}

UMaterialInstanceDynamic* URPGStaminaBar::CreateAndAssignMID(UImage* TargetImage, UMaterialInstanceDynamic* CurrentMID)
{
	if (!TargetImage) return nullptr;
	if (CurrentMID) return CurrentMID;
	return TargetImage->GetDynamicMaterial();
}
