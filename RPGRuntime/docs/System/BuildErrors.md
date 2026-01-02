# RPG Plugin Build Errors Report

## Error Log Summary
The following errors occurred after standaloning `RPGPawnData` and `RPGExperienceDefinition`:

| Error Message | Analysis |
| :--- | :--- |
| `Expected RPGHUD.h to be first header included` | include order violation in `RPGHUD.cpp`. (Fixed) |
| `'FStreamableHandle': type name mismatched` | Forward declared as `class`, should be `struct`. (Fixed) |
| `Cannot open include file: 'LoadingProcessInterface.h'` | Missing dependency in `Build.cs`. (Fixed: Added `CommonLoadingScreen`) |
| `function '...' already has a body` | Duplicate definitions of Team ID conversion helpers. (Fixed: Cleaned `.h` and `.cpp`) |
| `UpdatePlayerStartSpot: redefinition; different basic types` | Incorrect return type `void` instead of `bool` in `.cpp`. (Fixed) |
| `unresolved external symbol ... URPGTeamAgentInterface::URPGTeamAgentInterface` | Missing constructor for `UINTERFACE` class. (Fixed) |

## Root Cause Analysis
1. **Invalid API Macro**: The code used `LYRAGAME_API` which belongs to the LyraGame module. Each module must use its own generated API macro (e.g., `RPGRUNTIME_API`).
2. **Missing Module Dependencies**: We are referencing Lyra classes like `ULyraAbilitySet` but may not be including the `LyraGame` module in `RPGRuntime.Build.cs`.
3. **Incomplete Headers**: Forward declarations are useful but sometimes the UHT needs specific includes in the header if they are used as `TObjectPtr` or in mảng.

## Proposed Fixes
1. **API Macro Swap**: Replace all `LYRAGAME_API` with `RPGRUNTIME_API` in all new standalone headers.
2. **Comprehensive Standaloning**: We must redefine the following Lyra core classes within the RPG module to break the dependency:
    - [x] `RPGExperienceActionSet` (Done)
    - [x] `RPGPawnData` (Fixed)
    - [x] `RPGExperienceDefinition` (Fixed)
    - [x] `RPGAbilitySet` (Done)
    - [x] `RPGAbilityTagRelationshipMapping` (Done)
    - [x] `RPGInputConfig` (Done)
    - [x] `RPGCameraMode` (Done)
    - [x] `RPGAbilitySystemComponent` (Done)
    - [x] `RPGGameplayAbility` (Done)
    - [x] `RPGCharacter` (Done)
    - [x] `RPGCharacterMovementComponent` (Done)
    - [x] `RPGHealthComponent` (Done)
    - [x] `RPGPawnExtensionComponent` (Done)
    - [x] `RPGHealthSet` (Done)

4. **UHT Errors (Missing Type Information)**:
    - `Unable to find 'class', 'delegate', 'enum', or 'struct' with name 'ARPGPlayerController'` (In `RPGCharacter.h`)
    - `Unable to find 'class', 'delegate', 'enum', or 'struct' with name 'ARPGPlayerState'` (In `RPGCharacter.h`)
      - *Cause*: Forward declarations in header are not enough for `UFUNCTION` return types or `UPROPERTY` in some cases; UHT requires the full definition or header include.

5. **UHT Error (MinimalAPI vs Export Macro)**:
    - `MinimalAPI cannot be specified when the class is fully exported using a MODULENAME_API macro` (In `RPGCharacter.h`)
      - *Cause*: `MinimalAPI` is used when you only want to export specific functions. Using `RPGRUNTIME_API` exports the whole class, making `MinimalAPI` redundant and illegal.
      - *Resolution*: Remove `MinimalAPI` from `UCLASS()` when `RPGRUNTIME_API` is present.

7. **Batch Compilation Errors (Character/Player System)** [RESOLVED]:
    - **Missing Module Dependencies**:
        - `Cannot open include file: 'ModularCharacter.h'` -> FIXED: Added `ModularGameplayActors` to `Build.cs`.
        - `Cannot open include file: 'GameFeatures/GameFeatureAction_AddInputContextMapping.h'` -> FIXED: Fixed include path to `#include "GameFeatureAction_AddInputContextMapping.h"`.
    - **Missing Type Info / Header Includes**:
        - `'FGameplayEffectSpec': undeclared identifier` -> FIXED: Included `GameplayEffect.h` in `RPGHealthComponent.cpp` and `RPGHealthSet.cpp`.
        - `Cannot open include file: 'LyraLogChannels.h'` -> FIXED: Replaced with `RPGLogChannels.h`.
    - **Team System Helpers**:
        - `'GenericTeamIdToInteger': identifier not found` -> FIXED: Implemented standalone helpers in `RPGTeamAgentInterface.h/cpp` and moved declarations to top of header for visibility.
    - **Syntax/Naming Conflicts**:
        - `URGPawnExtensionComponent` errors -> FIXED: Renamed to `URPGPawnExtensionComponent`.
    - **Namespace Errors**:
        - `'RPGGameplayTags': is not a class or namespace name` -> FIXED.

6. **Include Errors (Missing Headers)**:
    - `AbilitySystem/Abilities/LyraGameplayAbility.h` (Used in `RPGAbilitySet.cpp`)
    - `RPGRuntimeLog.h` (Used in `RPGInputConfig.cpp`)
    - `LyraCameraComponent.h` (Used in `RPGCameraMode.cpp`)

## Analysis & Proposed Fixes
1. **API Macro Swap**: (Completed for PawnData, Experience, etc.)
2. **Comprehensive Standaloning**: (Completed for AbilitySet, TagMapping, InputConfig, CameraMode)
3. **Missing Headers Fix & Remaining Dependencies**:
    - **RPG Log**: Created `RPGLogChannels.h`. I will update `RPGInputConfig.cpp` to use `#include "System/RPGLogChannels.h"`.
    - **Remaining Lyra Dependencies**: To fully standalone, we need to decide if we should also standalone the following classes/constants that are still being referenced:
        - [x] `URPGAbilitySystemComponent` (Done)
        - [x] `URPGGameplayAbility` (Done)
        - [x] `URPGCameraComponent` (Done)
        - [ ] `URPGPlayerCameraManager` -> Should we create `URPGPlayerCameraManager`?
        - `LYRA_CAMERA_DEFAULT_FOV`, `LYRA_CAMERA_DEFAULT_PITCH_MIN/MAX` -> Should we define `RPG_CAMERA_...` constants?

4. **Temporary Fix (Enable Dependency)**: If we want to build *now* while waiting for the above to be standaloned, we must add `"LyraGame"` to `PrivateDependencyModuleNames` in `RPGRuntime.Build.cs` and fix include paths to be relative to the module root.
