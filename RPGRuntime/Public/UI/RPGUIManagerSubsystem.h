// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "RPGUIManagerSubsystem.generated.h"

class URPGUIPolicy;

/**
 * URPGUIManagerSubsystem
 *
 * The manager subsystem for the game UI.
 */
UCLASS(Config = Game)
class RPGRUNTIME_API URPGUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	URPGUIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	const URPGUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	URPGUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

protected:
	void SwitchToPolicy(URPGUIPolicy* NewPolicy);

	void OnLocalPlayerAdded(ULocalPlayer* LocalPlayer);
	void OnLocalPlayerRemoved(ULocalPlayer* LocalPlayer);

	UPROPERTY(Config, EditAnywhere, Category = "UI")
	TSoftClassPtr<URPGUIPolicy> DefaultPolicyClass;

private:
	UPROPERTY(Transient)
	TObjectPtr<URPGUIPolicy> CurrentPolicy;
};
