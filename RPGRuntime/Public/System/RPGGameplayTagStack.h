// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "RPGGameplayTagStack.generated.h"

struct FRPGGameplayTagStackContainer;

/**
 * Represents a single stack of a gameplay tag
 */
USTRUCT(BlueprintType)
struct FRPGGameplayTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FRPGGameplayTagStack()
	{}

	FRPGGameplayTagStack(FGameplayTag InTag, int32 InStackCount)
		: Tag(InTag)
		, StackCount(InStackCount)
	{}

	FString GetDebugString() const;

private:
	friend FRPGGameplayTagStackContainer;

	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	int32 StackCount = 0;
};

/**
 * Container for gameplay tag stacks
 */
USTRUCT(BlueprintType)
struct FRPGGameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FRPGGameplayTagStackContainer()
	{}

public:
	// Adds a specified number of stacks to the tag (creating a new stack if it doesn't exist)
	void AddStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (removing the stack if it reaches 0)
	void RemoveStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	int32 GetStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	bool ContainsTag(FGameplayTag Tag) const;

	//~FFastArraySerializer contract
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FRPGGameplayTagStack, FRPGGameplayTagStackContainer>(Stacks, DeltaParms, *this);
	}
	//~End of FFastArraySerializer contract

private:
	UPROPERTY()
	TArray<FRPGGameplayTagStack> Stacks;
};

template<>
struct TStructOpsTypeTraits<FRPGGameplayTagStackContainer> : public TStructOpsTypeTraitsBase2<FRPGGameplayTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
