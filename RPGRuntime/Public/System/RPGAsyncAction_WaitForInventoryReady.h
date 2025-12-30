// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "RPGAsyncAction_WaitForInventoryReady.generated.h"

class APawn;
class URPGInventoryManagerComponent;
class URPGQuickbarComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRPGInventoryReadyDelegate);

/**
 * URPGAsyncAction_WaitForInventoryReady
 *
 *	Async action to wait for the inventory and quickbar components to be ready on a pawn's controller.
 */
UCLASS()
class RPGRUNTIME_API URPGAsyncAction_WaitForInventoryReady : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	URPGAsyncAction_WaitForInventoryReady(const FObjectInitializer& ObjectInitializer);

	/**
	 * Waits for the inventory and quickbar components to be ready on the pawn's controller.
	 */
	UFUNCTION(BlueprintCallable, Category = "RPG|Inventory", meta = (BlueprintInternalUseOnly = "true"))
	static URPGAsyncAction_WaitForInventoryReady* WaitForInventoryReady(APawn* Pawn);

	virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FRPGInventoryReadyDelegate OnReady;

private:
	void Step();

	UPROPERTY()
	TWeakObjectPtr<APawn> BoundPawn;

	FTimerHandle TimerHandle;
};
