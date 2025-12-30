// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/RPGGameplayTagStack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGameplayTagStack)

FString FRPGGameplayTagStack::GetDebugString() const
{
	return FString::Printf(TEXT("%s x%d"), *Tag.ToString(), StackCount);
}

void FRPGGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		return;
	}

	if (StackCount > 0)
	{
		for (FRPGGameplayTagStack& Stack : Stacks)
		{
			if (Stack.Tag == Tag)
			{
				const int32 NewCount = Stack.StackCount + StackCount;
				Stack.StackCount = NewCount;
				MarkItemDirty(Stack);
				return;
			}
		}

		FRPGGameplayTagStack& NewStack = Stacks.Emplace_GetRef(Tag, StackCount);
		MarkItemDirty(NewStack);
	}
}

void FRPGGameplayTagStackContainer::RemoveStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		return;
	}

	if (StackCount > 0)
	{
		for (auto It = Stacks.CreateIterator(); It; ++It)
		{
			FRPGGameplayTagStack& Stack = *It;
			if (Stack.Tag == Tag)
			{
				if (Stack.StackCount <= StackCount)
				{
					It.RemoveCurrent();
					MarkArrayDirty();
				}
				else
				{
					const int32 NewCount = Stack.StackCount - StackCount;
					Stack.StackCount = NewCount;
					MarkItemDirty(Stack);
				}
				return;
			}
		}
	}
}

int32 FRPGGameplayTagStackContainer::GetStackCount(FGameplayTag Tag) const
{
	for (const FRPGGameplayTagStack& Stack : Stacks)
	{
		if (Stack.Tag == Tag)
		{
			return Stack.StackCount;
		}
	}
	return 0;
}

bool FRPGGameplayTagStackContainer::ContainsTag(FGameplayTag Tag) const
{
	for (const FRPGGameplayTagStack& Stack : Stacks)
	{
		if (Stack.Tag == Tag)
		{
			return true;
		}
	}
	return false;
}
