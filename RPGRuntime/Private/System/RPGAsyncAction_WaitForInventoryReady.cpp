// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/RPGAsyncAction_WaitForInventoryReady.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Inventory/RPGInventoryManagerComponent.h"
#include "Inventory/RPGQuickbarComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGAsyncAction_WaitForInventoryReady)

URPGAsyncAction_WaitForInventoryReady::URPGAsyncAction_WaitForInventoryReady(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

URPGAsyncAction_WaitForInventoryReady* URPGAsyncAction_WaitForInventoryReady::WaitForInventoryReady(APawn* Pawn)
{
	URPGAsyncAction_WaitForInventoryReady* Action = NewObject<URPGAsyncAction_WaitForInventoryReady>();
	Action->BoundPawn = Pawn;
	return Action;
}

void URPGAsyncAction_WaitForInventoryReady::Activate()
{
	if (!BoundPawn.IsValid())
	{
		SetReadyToDestroy();
		return;
	}

	Step();
}

void URPGAsyncAction_WaitForInventoryReady::Step()
{
	APawn* Pawn = BoundPawn.Get();
	if (!Pawn)
	{
		SetReadyToDestroy();
		return;
	}

	AController* Controller = Pawn->GetController();
	if (Controller)
	{
		URPGInventoryManagerComponent* InventoryComponent = Controller->FindComponentByClass<URPGInventoryManagerComponent>();
		URPGQuickbarComponent* QuickbarComponent = Controller->FindComponentByClass<URPGQuickbarComponent>();

		if (InventoryComponent && QuickbarComponent)
		{
			OnReady.Broadcast();
			SetReadyToDestroy();
			return;
		}
	}

	// Not ready yet, poll again in 0.1s
	if (UWorld* World = Pawn->GetWorld())
	{
		World->GetTimerManager().SetTimer(TimerHandle, this, &URPGAsyncAction_WaitForInventoryReady::Step, 0.1f, false);
	}
	else
	{
		SetReadyToDestroy();
	}
}
