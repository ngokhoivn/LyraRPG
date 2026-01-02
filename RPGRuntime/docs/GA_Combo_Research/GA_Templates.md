# Professional GA Templates: Atomic, Stateful, & Combo

Bộ tài liệu này cung cấp 3 loại Template nền tảng cho mọi Gameplay Ability trong dự án, đảm bảo tính bao đóng, dễ mở rộng và tuân thủ chuẩn GAS chuyên nghiệp.

---

## 🏗️ Phân loại Ability (Category Mapping)

| Loại Ability | Đặc điểm | Ví dụ | Template khuyến nghị |
| :--- | :--- | :--- | :--- |
| **Atomic** | Đơn thỏa, kết thúc ngay | Fireball, Heal, Dash | `GA_Atomic_Template` |
| **Stateful** | Duy trì trạng thái dài | Sprint, Aim, Guard | `GA_Stateful_Template` |
| **Combo** | State Machine phức tạp | Melee Combos | `GA_Combo_Template` |

---

## 🧱 1. GA_Atomic_Template (One-shot Abilities)
/** 
 * Atomic Ability Template:
 * - One-shot (Kích hoạt 1 lần -> Hết).
 * - Không giữ trạng thái (No state).
 * - Không dùng cho chuỗi đòn đánh (No combo chain).
 * - Tốt cho: Fireball, Heal, Dash, Emote.
 */

### Header (`GA_Atomic.h`)
```cpp
UCLASS()
class UGA_Atomic_Template : public UCoreGameplayAbility {
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    UAnimMontage* ActiveMontage;

    // Delegate để Blueprint quyết định bước tiếp theo
    UFUNCTION(BlueprintImplementableEvent)
    void OnAbilityMontageFinished();

    virtual void ActivateAbility(...) override;
};
```

### Source (`GA_Atomic.cpp`)
```cpp
void UGA_Atomic_Template::ActivateAbility(...) {
    if (ActiveMontage) {
        // Sử dụng PlayMontageAndWait để theo dõi vòng đời hoạt ảnh
        UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ActiveMontage);
        Task->OnCompleted.AddDynamic(this, &UGA_Atomic_Template::Internal_OnMontageFinished);
        Task->ReadyForActivation();
    } else {
        // Thực thi logic ngay nếu không có Montage
        ExecuteAbilityLogic();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UGA_Atomic_Template::Internal_OnMontageFinished() {
    // Gọi Event cho Blueprint (Dành cho Logic mở rộng)
    OnAbilityMontageFinished();
    
    // Mặc định kết thúc Ability an toàn sau Montage
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

---

## 🔁 2. GA_Stateful_Template (Continuous Abilities)
/**
 * Stateful Ability Template:
 * - Duy trì trạng thái dựa trên Input (Sprint, Aim).
 * - KHÔNG nên lạm dụng để Apply Damage trực tiếp (Sát thương nên từ Atomic GA).
 * - Tuyệt đối Cleanup tag trước khi End.
 */

### Key Logic (Stateful):
- **Không auto EndAbility** sau Montage.
- Lắng nghe `WaitInputRelease` hoặc `WaitGameplayTagRemove` để tự hủy trạng thái.

```cpp
void UGA_Stateful_Template::ActivateAbility(...) {
    // 1. Gán State Tag duy trì
    UpdateStateTag(State_Action_Aiming, true);

    // 2. Lắng nghe tín hiệu nhả phím
    UAbilityTask_WaitInputRelease* InputTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
    InputTask->OnRelease.AddDynamic(this, &UGA_Stateful_Template::EndingProcess);
    InputTask->ReadyForActivation();
}

void UGA_Stateful_Template::EndingProcess(float TimeHeld) {
    // Dọn dẹp Tag trước khi kết thúc
    UpdateStateTag(State_Action_Aiming, false);
    EndAbility(...);
}
```

---

## ⚔️ 3. GA_Combo_Template (State Machine)
*Cơ chế "Pure Tag-State" điều khiển bởi Animation Event.*

### Key Logic (Combo):
- **Event-Driven**: Đợi tín hiệu `Event.Combo.Transition` từ Animation Montage.
- **Input Buffer**: Ghi nhận phím bấm trong "Action Window" để quyết định Jumping đòn tiếp theo.
- **State Switch**: Cập nhật `State.Action.Combo.X` liên tục theo nhịp đánh.

*Chi tiết mã nguồn xem tại: [Ideal_Combo_Ability.md](file:///c:/Users/ngokh/Downloads/Crunch_Test-master/docs/GA_Combo_Research/Ideal_Combo_Ability.md)*

---

## 🛠️ Nguyên tắc "Vàng" khi sử dụng Template
1. **Dùng đúng loại**: Đừng cố biến một Atomic GA thành Combo GA bằng cách thêm `if-else`. Hãy đổi Template.
2. **Cleanup tuyệt đối**: Mọi Stateful Ability phải đảm bảo xóa toàn bộ "Loose Tags" trong chu trình kết thúc.
3. **Logic decoupled**: Con số sát thương luôn nằm trong Gameplay Effect, Template chỉ lo phần "Vòng đời" (Lifecycle).
