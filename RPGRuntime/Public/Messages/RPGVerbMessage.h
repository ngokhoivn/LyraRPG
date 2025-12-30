// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "RPGVerbMessage.generated.h"

/**
 * FRPGVerbMessage
 * 
 * A message structure primarily used for server-to-client notifications like eliminations.
 */
USTRUCT(BlueprintType)
struct FRPGVerbMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = Message)
	FGameplayTag Verb;

	UPROPERTY(BlueprintReadWrite, Category = Message)
	TObjectPtr<UObject> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = Message)
	TObjectPtr<UObject> Target = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = Message)
	TArray<TObjectPtr<UObject>> ExtraObjects;

	UPROPERTY(BlueprintReadWrite, Category = Message)
	FGameplayTagContainer ContextTags;

	UPROPERTY(BlueprintReadWrite, Category = Message)
	double Magnitude = 0.0;

	FString ToString() const;
};
