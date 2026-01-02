# RPG Standalone Input Flow & Lyra Compatibility

## Overview
This document explains the input binding architecture used in the RPG plugin. It highlights the distinction between the current **Lyra Integration Phase** and the future **Pure Standalone Phase**.

## Current Architecture (Lyra Integration)
In the current project state, the RPG plugin is running on top of the Lyra Starter Game. This creates some friction because Lyra has its own initialization logic and input component classes.

### The "Smart Fallback" Logic
We discovered that even when using `ARPGPlayerController`, Unreal might initialize a `LyraInputComponent` for the pawn instead of our custom `URPGInputComponent`. 

To handle this without breaking the game, we implemented a **Smart Fallback** in `RPGHeroComponent.cpp`:
- **Step 1**: Try to cast the Input Component to `URPGInputComponent`. If successful, use the optimized native binding functions.
- **Step 2 (Fallback)**: If the cast fails (e.g., it's a `LyraInputComponent`), we cast to the base `UEnhancedInputComponent`.
- **Step 3**: We manually iterate through the `InputConfig` and bind actions using the payload-based `BindAction` template.

> [!IMPORTANT]
> This fallback is a **compatibility layer**. It ensures that abilities work regardless of which Input Component class Lyra decides to spawn.

## Standalone Transition
When the RPG plugin is moved to a clean, standalone project (without Lyra), the following will happen:

1.  **Cleaner Code**: The `else if (EnhancedIC)` fallback block in `RPGHeroComponent.cpp` can be removed.
2.  **Strict Typing**: We can strictly enforce `URPGInputComponent` across the entire project.
3.  **Performance**: Using the direct `URPGInputComponent` bindings is slightly more efficient as it doesn't require manual loops during initialization.

## Troubleshooting: "Bound 0 Ability Actions"
If the logs show `Bound 0 Ability Actions`, it indicates a **Data Asset configuration issue** rather than a code bug.

**Checklist in the Editor:**
1.  Open your `URPGInputConfig` asset (e.g., `InputData_RPG`).
2.  Look at the `Ability Input Actions` array.
3.  Ensure you have added entries for your abilities (e.g., Attack, Heal).
4.  Ensure each entry has a valid **Input Action** (UAsset) and a valid **Input Tag** (Gameplay Tag).
5.  Verify that your **Input Mapping Context (IMC)** maps the physical keys (LMB, RMB) to the same **Input Actions** defined in the config.

---
*Generated for the RPG Runtime Standalone Migration Phase.*
