# Quy trình Triển khai Death Ability (Khả năng Chết)

Tài liệu này hướng dẫn cách triển khai hệ thống chết cho nhân vật, từ việc kích hoạt Ability đến xử lý Animation và ẩn nhân vật, dựa trên kiến trúc GAS của RPG Plugin.

## 🔄 Luồng Logic Tổng quát

1. **Máu về 0**: `RPGAttributeSet` phát hiện Health <= 0.
2. **Kích hoạt Event**: `RPGHealthComponent` nhận tín hiệu `HandleOutOfHealth` và gửi Gameplay Event `GameplayEvent.Death`.
3. **Kích hoạt Ability**: `GA_Hero_Death` lắng nghe event trên và bắt đầu thực thi.
4. **Xử lý Montage**: Lấy Death Montage từ Class nhân vật và phát thông qua `RPGAbilitySystemComponent`.
5. **Hoàn tất**: Sau khi animation kết thúc (hoặc một khoảng delay), ẩn Actor và chuyển Camera.

---

## 🛠️ Các Bước Thực Hiện

### Bước 1: Cấu hình GA_Hero_Death
1. Tìm/Tạo Ability: `GA_Hero_Death` (Cha: `RPGGameplayAbility`).
2. **Ability Triggers**:
   - **Tag**: `GameplayEvent.Death`
   - **Trigger Source**: `Gameplay Event`
3. **Logic trong BP**:
   - `WaitGameplayEvent` (nếu cần dữ liệu sát thương) hoặc trực tiếp `PlayMontageAndWait`.
   - Chọn Montage từ biến được định nghĩa trong Character Class.

### Bước 2: Thiết lập Ability Set
Hệ thống standalone sử dụng `RPGAbilitySet` để gán khả năng cho nhân vật.
1. Tạo/Mở: `AbilitySet_RPGHero`.
2. **Granted Abilities**: Thêm `GA_Hero_Death`.
3. **Input Tag**: Để trống (vì kích hoạt bằng Event).

### Bước 3: Animation & Montage
Để nhân vật chết mượt mà, cần cấu hình Montage đúng cách:
1. **Retargeting**: Đảm bảo Animation Death đã được retarget sang Skeleton của nhân vật (ví dụ: Yin).
2. **Montage Config**:
   - **Slot**: Sử dụng `FullBody` (đảm bảo ABP đã có node Slot tương ứng).
   - **Root Motion**: Bật `EnableRootMotion` và `Force Root Lock` để nhân vật không bị trượt hoặc nhảy về vị trí cũ sau khi chết.

### Bước 4: Cập nhật Animation Blueprint (ABP)
Mở `ABP_Player_Base` (hoặc ABP tương ứng):
1. Thêm node **Slot 'DefaultSlot.FullBody'** vào luồng logic chính (thường sau node *Inertialization* hoặc trước *Output Pose*).
2. Việc này cho phép Montage ghi đè toàn bộ chuyển động của nhân vật.

### Bước 5: Logic Death trong Character Blueprint
Thay vì dùng Ragdoll (phức tạp), chúng ta sử dụng logic ẩn nhân vật:
1. Mở `B_Hero_Default`.
2. **Event OnDeathStarted**: Lắng nghe từ `HealthComponent`.
3. **Logic Ẩn**:
   - Sử dụng `PlayAnimMontage` (nếu không chạy qua GAS) hoặc lắng nghe kết thúc từ GA.
   - Thêm `Delay` = `Duration - 0.2s`.
   - Gọi `Set Actor Hidden In Game` = `true`.
   - Vô hiệu hóa va chạm: `Set Actor Enable Collision` = `false`.

---

## 🏷️ Các Tag và Event quan trọng
| Loại | Tag | Ghi chú |
| :--- | :--- | :--- |
| **Event** | `GameplayEvent.Death` | Dùng để kích hoạt GA |
| **Status** | `Status.Death.Dying` | Trạng thái đang chết (đang chạy animation) |
| **Status** | `Status.Death.Dead` | Trạng thái đã chết hoàn toàn |

---

## ✅ Checklist Kiểm tra
- [ ] Nhân vật có phát đúng Montage khi hết máu không?
- [ ] Montage có bị ngắt quãng giữa chừng không (kiểm tra Slot name)?
- [ ] Nhân vật có ẩn đi sau khi animation kết thúc không?
- [ ] Camera có chuyển trạng thái không?

> [!TIP]
> Bạn nên lưu danh sách Death Montages trong một biến mảng (Array) tại Character Class để có thể random kiểu chết, giúp gameplay sinh động hơn.
