# RPG Plugin: Standalone & Migration Guide

This document explains the technical implementation of the RPG plugin as a hybrid system that currently leverages Lyra but is designed for future independence.

## 1. Project Architecture (Current State)
Currently, the RPG plugin operates as a **Game Feature Plugin** within the Lyra framework. To ensure functionality without "breaking" Lyra or losing RPG-specific features, several "Bridge" points have been implemented.

### Inheritance Tree
| Component | RPG Class | Lyra Base (Current) | Future Standalone Base |
| :--- | :--- | :--- | :--- |
| Asset Manager | `URPGAssetManager` | `ULyraAssetManager` | `UAssetManager` |
| Player Controller | `ARPGPlayerController` | `APlayerController` | `APlayerController` |
| Character | `ARPGCharacter` | `ALyraCharacter` (likely) | `ACharacter` |
| Hero Component | `URPGHeroComponent` | `UPawnComponent` | `UPawnComponent` |

---

## 2. Key Bridge Implementations (Critical for Standalone Development)

### A. Input System Compatibility
In `RPGHeroComponent.cpp`, the plugin expects `URPGInputComponent`. However, Lyra often forces `ULyraInputComponent`.

**Implementation Reference:** `URPGHeroComponent::InitializePlayerInput`
```cpp
// [BRIDGE LOGIC] Handles both RPG and Lyra Input Components
URPGInputComponent* RPGIC = Cast<URPGInputComponent>(PlayerInputComponent);
UEnhancedInputComponent* EnhancedIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);

if (RPGIC) {
    // Standard RPG binding
    RPGIC->BindNativeAction(...);
} else if (EnhancedIC) {
    // Fallback binding for any Enhanced Input system (including Lyra)
    // Uses raw BindAction to bypass class mismatch
    EnhancedIC->BindAction(IA, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
}
```
**Independence Step:** Once Lyra is removed, you can strictly use `URPGInputComponent` and simplify this logic.

### B. Asset Manager Registration
The `URPGAssetManager` must register its own Primary Assets to avoid "Invalid Primary Asset Id" warnings.

**Implementation Reference:** `URPGAssetManager::StartInitialLoading`
```cpp
// [BRIDGE LOGIC] Explicitly scan RPG-specific types
PrimaryAssetTypesToScan.Add(FPrimaryAssetTypeInfo(TEXT("RPGExperienceDefinition"), ...));
PrimaryAssetTypesToScan.Add(FPrimaryAssetTypeInfo(TEXT("RPGExperienceActionSet"), ...));
```

---

## 3. Configuration Setup
These settings in your `.ini` files are the "glue" that tells Unreal to use your RPG logic over Lyra defaults.

### DefaultEngine.ini
- `AssetManagerClassName=/Script/RPGRuntime.RPGAssetManager`: Forces Unreal to use your custom manager.

### DefaultGame.ini
- `RPGGameDataPath="/Game/RPG/Data/DA_RPGGameData"`: Defines where RPG-specific global data lives.

---

## 4. Future Independence Checklist
To completely detach from Lyra, follow these steps:

1. **Build.cs Update**: Remove `"LyraGame"` from `PublicDependencyModuleNames`.
2. **Inheritance Cleanup**: Change `URPGAssetManager` to inherit from `UAssetManager`.
3. **Interface Review**: Replace any `ILyra...` interfaces with native C++ or RPG-specific interfaces.
4. **Input Config**: Ensure your `URPGInputComponent` handles all initialization that Lyra currently "steals."
5. **Asset Paths**: Verify that all hardcoded paths (if any) in `RPGAssetManager` or `RPGHeroComponent` point to your plugin folder, not `/Game/Lyra/...`.

---
> [!IMPORTANT]
> Keep your **Issue Tracking** up to date. Every time you fix a Lyra-related conflict, document it here so you know what needs to be "un-bridged" later.
