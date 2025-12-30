// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/RPGCameraMode.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"
#include "Camera/RPGCameraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGCameraMode)

//////////////////////////////////////////////////////////////////////////
// FRPGCameraModeView
//////////////////////////////////////////////////////////////////////////
FRPGCameraModeView::FRPGCameraModeView()
	: Location(ForceInit)
	, Rotation(ForceInit)
	, ControlRotation(ForceInit)
	, FieldOfView(80.0f)
{
}

void FRPGCameraModeView::Blend(const FRPGCameraModeView& Other, float OtherWeight)
{
	if (OtherWeight <= 0.0f)
	{
		return;
	}
	else if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}


//////////////////////////////////////////////////////////////////////////
// URPGCameraMode
//////////////////////////////////////////////////////////////////////////
URPGCameraMode::URPGCameraMode()
{
	FieldOfView = 80.0f;
	ViewPitchMin = -89.0f;
	ViewPitchMax = 89.0f;

	BlendTime = 0.5f;
	BlendFunction = ERPGCameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;
}

URPGCameraComponent* URPGCameraMode::GetRPGCameraComponent() const
{
	return CastChecked<URPGCameraComponent>(GetOuter());
}

UWorld* URPGCameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* URPGCameraMode::GetTargetActor() const
{
	const URPGCameraComponent* RPGCameraComponent = GetRPGCameraComponent();
	return RPGCameraComponent->GetTargetActor();
}

FVector URPGCameraMode::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

			return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
		}

		return TargetPawn->GetPawnViewLocation();
	}

	return TargetActor->GetActorLocation();
}

FRotator URPGCameraMode::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	return TargetActor->GetActorRotation();
}

void URPGCameraMode::UpdateCameraMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void URPGCameraMode::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void URPGCameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
	const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

	switch (BlendFunction)
	{
	case ERPGCameraModeBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case ERPGCameraModeBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case ERPGCameraModeBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case ERPGCameraModeBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	default:
		break;
	}
}

void URPGCameraMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

	switch (BlendFunction)
	{
	case ERPGCameraModeBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case ERPGCameraModeBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case ERPGCameraModeBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case ERPGCameraModeBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	default:
		break;
	}
}

void URPGCameraMode::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("      RPGCameraMode: %s (%f)"), *GetName(), BlendWeight));
}


//////////////////////////////////////////////////////////////////////////
// URPGCameraModeStack
//////////////////////////////////////////////////////////////////////////
URPGCameraModeStack::URPGCameraModeStack()
{
	bIsActive = true;
}

void URPGCameraModeStack::ActivateStack()
{
	if (!bIsActive)
	{
		bIsActive = true;
		for (URPGCameraMode* CameraMode : CameraModeStack)
		{
			if (CameraMode) CameraMode->OnActivation();
		}
	}
}

void URPGCameraModeStack::DeactivateStack()
{
	if (bIsActive)
	{
		bIsActive = false;
		for (URPGCameraMode* CameraMode : CameraModeStack)
		{
			if (CameraMode) CameraMode->OnDeactivation();
		}
	}
}

void URPGCameraModeStack::PushCameraMode(TSubclassOf<URPGCameraMode> CameraModeClass)
{
	if (!CameraModeClass) return;

	URPGCameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);

	int32 StackSize = CameraModeStack.Num();

	if ((StackSize > 0) && (CameraModeStack[0] == CameraMode)) return;

	int32 ExistingStackIndex = INDEX_NONE;
	float ExistingStackContribution = 1.0f;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		if (CameraModeStack[StackIndex] == CameraMode)
		{
			ExistingStackIndex = StackIndex;
			ExistingStackContribution *= CameraMode->GetBlendWeight();
			break;
		}
		else
		{
			ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
		}
	}

	if (ExistingStackIndex != INDEX_NONE)
	{
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}
	else
	{
		ExistingStackContribution = 0.0f;
	}

	const bool bShouldBlend = ((CameraMode->GetBlendTime() > 0.0f) && (StackSize > 0));
	const float BlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

	CameraMode->SetBlendWeight(BlendWeight);
	CameraModeStack.Insert(CameraMode, 0);
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	if (ExistingStackIndex == INDEX_NONE)
	{
		CameraMode->OnActivation();
	}
}

bool URPGCameraModeStack::EvaluateStack(float DeltaTime, FRPGCameraModeView& OutCameraModeView)
{
	if (!bIsActive) return false;
	UpdateStack(DeltaTime);
	BlendStack(OutCameraModeView);
	return true;
}

URPGCameraMode* URPGCameraModeStack::GetCameraModeInstance(TSubclassOf<URPGCameraMode> CameraModeClass)
{
	check(CameraModeClass);
	for (URPGCameraMode* CameraMode : CameraModeInstances)
	{
		if (CameraMode && CameraMode->GetClass() == CameraModeClass) return CameraMode;
	}

	URPGCameraMode* NewCameraMode = NewObject<URPGCameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);
	CameraModeInstances.Add(NewCameraMode);
	return NewCameraMode;
}

void URPGCameraModeStack::UpdateStack(float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0) return;

	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		URPGCameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);
		CameraMode->UpdateCameraMode(DeltaTime);

		if (CameraMode->GetBlendWeight() >= 1.0f)
		{
			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);
			break;
		}
	}

	if (RemoveCount > 0)
	{
		for (int32 StackIndex = RemoveIndex; StackIndex < StackSize; ++StackIndex)
		{
			URPGCameraMode* CameraMode = CameraModeStack[StackIndex];
			if (CameraMode) CameraMode->OnDeactivation();
		}
		CameraModeStack.RemoveAt(RemoveIndex, RemoveCount);
	}
}

void URPGCameraModeStack::BlendStack(FRPGCameraModeView& OutCameraModeView) const
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0) return;

	const URPGCameraMode* CameraMode = CameraModeStack[StackSize - 1];
	check(CameraMode);

	OutCameraModeView = CameraMode->GetCameraModeView();

	for (int32 StackIndex = (StackSize - 2); StackIndex >= 0; --StackIndex)
	{
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);
		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
	}
}

void URPGCameraModeStack::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("   --- RPG Camera Modes (Begin) ---")));
	for (const URPGCameraMode* CameraMode : CameraModeStack)
	{
		if (CameraMode) CameraMode->DrawDebug(Canvas);
	}
	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   --- RPG Camera Modes (End) ---")));
}

void URPGCameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	if (CameraModeStack.Num() == 0)
	{
		OutWeightOfTopLayer = 1.0f;
		OutTagOfTopLayer = FGameplayTag();
	}
	else
	{
		URPGCameraMode* TopEntry = CameraModeStack.Last();
		check(TopEntry);
		OutWeightOfTopLayer = TopEntry->GetBlendWeight();
		OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
	}
}
