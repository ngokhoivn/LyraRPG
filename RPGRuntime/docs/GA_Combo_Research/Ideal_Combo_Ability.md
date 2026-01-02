# The Ideal GA_Combo: Pure Implementation

Tài liệu này trình bày mã nguồn "lý tưởng" cho lớp `GA_Combo`. Chúng tôi loại bỏ mọi logic rườm rà, chỉ giữ lại khung xương Event-Driven mạnh mẽ nhất.

---

## 1. Header (`GA_Combo.h`)
Sự tinh gọn bắt đầu từ việc chỉ sử dụng các biến thành viên thiết yếu.

```cpp
UCLASS()
class UGA_Combo : public UCoreGameplayAbility {
protected:
    UPROPERTY(EditDefaultsOnly)
    UAnimMontage* ComboMontage;

    // Liên kết giữa Trạng thái và Sát thương
    UPROPERTY(EditDefaultsOnly)
    TMap<FGameplayTag, TSubclassOf<UGameplayEffect>> DamageMap;

    FGameplayTag CurrentStateTag;

    // Các hàm xử lý Event cốt lõi
    void OnAbilityInputPressed();
    void OnComboTransitionEvent(FGameplayTag EventTag);
};
```

---

## 2. Logic thực thi (`GA_Combo.cpp`)

### A. Khởi chạy (The Trigger)
Sử dụng `PlayMontageAndWait` và bắt đầu lắng nghe Input ngay lập tức.

```cpp
void UGA_Combo::ActivateAbility(...) {
    // 1. Play Montage đòn đầu tiên
    PlayMontageAndWait("Attack1");

    // 2. Gán State ban đầu
    UpdateStateTag(Tag_Attack1);

    // 3. Lắng nghe Input & Event chuyển đòn
    WaitInputPress();
    WaitGameplayEvent("Event.Combo.Transition");
}
```

### B. Chuyển State (The Heart)
Đây là nơi bộ não hoạt động. Nó không tự ý quyết định đòn tiếp theo, nó chỉ phản hồi lại tín hiệu từ Animation.

```cpp
void UGA_Combo::OnComboTransitionEvent(FGameplayTag EventTag) {
    // Nguyên tắc: GA_Combo không bao giờ tự quyết định đòn kế tiếp.
    // Mọi chuyển đổi phải đến từ Event do Animation phát ra (EventTag).
    
    // Nếu người chơi đã bấm phím (Input Buffering)
    if (bInputBufferPressed) {
        // Trích xuất Section tiếp theo TRỰC TIẾP từ thông tin của EventTag
        FName NextSection = GetNextSectionFromTag(EventTag);
        
        // Nhảy sang đòn tiếp theo
        MontageJumpToSection(NextSection);
        
        // Cập nhật State Tag mới (ví dụ: State.Action.Combo.Light2)
        UpdateStateTag(GetStateTagFromSection(NextSection));
        
        // Reset buffer và đợi tiếp
        ResetInputBuffer();
    }
}
```

---

## 3. Tại sao đây là "Ideal"?

1.  **Zero If-Else**: Logic chuyển đòn hoàn toàn dựa trên sự kết hợp giữa Event từ Animation và trạng thái Input của Player.
2.  **Animation-Led Logic**: Code không "đoán" nhịp đòn, Animator là người cầm lái hoàn toàn.
3.  **State Synchronization**: Tag Trạng thái được cập nhật đồng thời với Animation, giúp cho hệ thống SFX/VFX có thể bám sát hoàn hảo.
4.  **Encapsulation**: Ability không cần quan tâm Montage dài bao nhiêu hay có bao nhiêu Section. Nó chỉ quan tâm đến các điểm "Transition Points" do Animator đặt ra.

---
**Key Takeaway**: Hãy biến `GA_Combo` thành một trọng tài (Referee) công bằng: Nếu Animation báo "Mở cửa" và Player báo "Muốn đi", thì Trọng tài cho phép "Bước tiếp". 
