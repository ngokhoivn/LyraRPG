# Pure Core Architecture: Essential GAS Components

Kiến trúc này loại bỏ toàn bộ các lớp trung gian không cần thiết, tập trung vào 3 thành phần cốt lõi của một hệ thống Combo chuyên nghiệp.

---

## 1. Cấu trúc Thư mục chuẩn (Minimalist)

```text
Source/YourGame/
├── GAS/
│   ├── CoreAbilitySystemComponent.h/cpp  <- Quản lý Level/Ability Specs.
│   ├── CoreGameplayAbility.h/cpp       <- Lớp cha (Helper gán State Tags).
│   └── GA_Combo.h/cpp                   <- Logic State Machine (Duy nhất).
└── Notifies/
    ├── AN_ComboEvent.h/cpp              <- Notify gửi tín hiệu State/Damage.
```

---

## 2. Các lớp nòng cốt (The "Essential" Classes)

### A. Core Ability System Component
Chỉ giữ lại các hàm trần trụi:
- **`GiveAbility`**: Cấp quyền sử dụng combo.
- **`HandleInput`**: Điều hướng tín hiệu từ phím bấm tới Ability.

### B. Core Gameplay Ability (The Skeleton)
Lớp này phải có helper xử lý **Loose Tags**. Đây là chìa khóa để hệ thống "Pure" hoạt động.
- **`SetStateTag(FGameplayTag Tag, bool bAdd)`**: Thông báo cho thế giới biết nhân vật đang làm gì mà không cần hỏi Animation.

### C. GA_Combo (The Brain)
Thay vì dùng legacy loop, `GA_Combo` mới sử dụng cấu trúc **Event-Driven**:
1. Phát Montage.
2. Đứng chờ `WaitGameplayEvent`.
3. Khi nhận Event -> Kiểm tra Input Buffer -> Chuyển Section & Cập nhật State Tag.

---

## 3. Gameplay Tags: Ngôn ngữ duy nhất
Hệ thống Pure yêu cầu phân cấp Tag nghiêm ngặt (theo chuẩn Lyra):
- `State.Action.Combo` (Đang đánh)
- `State.Action.Combo.Light1` (Đang ở đòn 1)
- `State.Action.Combo.Light2` (Đang ở đòn 2)
- `Event.Combo.Transition` (Tín hiệu chuyển đòn)
- `Event.Combo.Damage` (Tín hiệu gây sát thương)

---
**Tại sao phải làm vậy?**
Vì khi bạn tách biệt hoàn toàn Logic khỏi Animation, bạn có thể thay đổi toàn bộ Montage (ví dụ đổi từ kiếm sang thương) mà **không cần sửa một dòng code nào**. Logic chỉ quan tâm "Khi nhận được tín hiệu Next, nếu có Input thì nhảy đòn tiếp theo".
