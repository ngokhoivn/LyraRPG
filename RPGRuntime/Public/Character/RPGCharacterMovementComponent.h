// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "NativeGameplayTags.h"
#include "RPGCharacterMovementComponent.generated.h"

class UObject;
struct FFrame;

RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RPG_Movement_Stopped);

/**
 * FRPGCharacterGroundInfo
 *
 *	Information about the ground under the character.  It only gets updated as needed.
 */
USTRUCT(BlueprintType)
struct FRPGCharacterGroundInfo
{
	GENERATED_BODY()

	FRPGCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};


/**
 * URPGCharacterMovementComponent
 *
 *	The base character movement component class used by this project.
 *  Standalone version of LyraCharacterMovementComponent.
 */
UCLASS(Config = Game)
class RPGRUNTIME_API URPGCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	URPGCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void SimulateMovement(float DeltaTime) override;

	virtual bool CanAttemptJump() const override;

	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "RPG|CharacterMovement")
	const FRPGCharacterGroundInfo& GetGroundInfo();

	void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

protected:

	virtual void InitializeComponent() override;

protected:

	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FRPGCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
};
