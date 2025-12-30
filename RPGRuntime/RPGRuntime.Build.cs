// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RPGRuntime : ModuleRules
{
	public RPGRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",

			// Networking (BẮT BUỘC cho FastArray / PushModel)
			"NetCore",

			// Gameplay Ability System
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",

			// Modular Gameplay
			"ModularGameplay",
			"ModularGameplayActors",
			"GameFeatures",
			"GameplayMessageRuntime",
			"EnhancedInput",
			"AIModule",
			"CommonLoadingScreen"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",

			// Animation runtime (OK)
			"AnimGraphRuntime",

			// Optional nhưng an toàn
			"MotionWarping",
			"ContextualAnimation"
		});
	}
}
