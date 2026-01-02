# Quy trình Triển khai Death Ability (Khả năng Chết)

Tài liệu này hướng dẫn cách triển khai hệ thống chết cho nhân vật, từ việc kích hoạt Ability đến xử lý Animation và ẩn nhân vật, dựa trên kiến trúc GAS của RPG Plugin.

## 🔄 Luồng Logic Tổng quát

1. **Máu về 0**: `RPGAttributeSet` phát hiện Health <= 0.
2. **Kích hoạt Event**: `RPGHealthComponent` nhận tín hiệu `HandleOutOfHealth` và gửi Gameplay Event `GameplayEvent.Death`.
3. **Kích hoạt Ability**: `GA_Hero_Death` (kế thừa từ lớp C++ `URPGGA_Death`) lắng nghe event trên và bắt đầu thực thi.
4. **Xử lý Montage**: Lớp C++ tự động lấy Death Montage từ Character Class và phát thông qua `RPGAbilitySystemComponent`.
5. **Hoàn tất**: Sau khi animation kết thúc (hoặc một khoảng delay), ẩn Actor và chuyển Camera.

---

## 🛠️ Các Bước Thực Hiện

### Bước 1: Cấu hình GA_Hero_Death
1. Tạo một Blueprint `GA_Hero_Death` kế thừa từ lớp C++ **`RPGGA_Death`**.
2. **Kích hoạt tự động**: Bạn **không cần** cấu hình "Ability Triggers" thủ công. Lớp C++ `RPGGA_Death` đã được thiết lập để tự động lắng nghe tag `GameplayEvent.Death`.
3. **Cấu hình C++**:
   - `bAutoStartDeath`: Mặc định là `true` (Tự động gọi `StartDeath` trên HealthComponent).
   - `bAutoFinishDeath`: Mặc định là `true` (Tự động gọi `FinishDeath` khi Ability kết thúc).
4. **Logic bổ sung**: Mặc dù lớp C++ đã xử lý việc phát Montage ngẫu nhiên, bạn vẫn có thể ghi đè logic trong Blueprint nếu cần xử lý đặc biệt.
   - **Thứ tự ưu tiên Montage**: Lớp C++ sẽ kiểm tra mảng `DeathMontages` trong chính Ability trước, nếu trống nó mới tìm đến mảng `DeathMontages` trong Character Class. Điều này cho phép bạn tạo nhiều kiểu chết khác nhau (chết do cháy, chết do điện...) bằng cách gán animation riêng cho từng Ability Blueprint.

### Bước 2: Thiết lập Ability Set & Pawn Data
Hệ thống standalone sử dụng `RPGAbilitySet` để gán khả năng cho nhân vật.
1. Tạo/Mở: DataAsset `AbilitySet_RPGHero`.
2. **Granted Gameplay Abilities**: Thêm `GA_Hero_Death`.
3. **Kết nối trung tâm (QUAN TRỌNG)**: 
   - Mở DataAsset **`HeroPawnData`** (loại `RPGPawnData`).
   - Tìm mảng **`Ability Sets`**.
   - Thêm **`AbilitySet_RPGHero`** vào mảng này. Nếu thiếu bước này, `GA_Death` sẽ không bao giờ được cấp cho nhân vật.
4. **Input Tag**: Để trống (vì kích hoạt bằng Event).

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

### Bước 5: Kiểm tra và Tinh chỉnh (BP)
Vì chúng ta đã triển khai hầu hết logic vào C++, các bước trong Blueprint sẽ đơn giản hơn rất nhiều:

1. **Tự động hóa**: Bạn **không cần** tự gọi `Set Actor Hidden` hay `Disable Collision` trong Blueprint nữa. Lớp C++ `ARPGCharacter` đã tự thực hiện:
   - `DisableMovementAndCollision()`: Tắt va chạm Capsule và dừng di chuyển ngay khi bắt đầu chết.
   - `UninitAndDestroy()`: Tự động ẩn Actor (`SetActorHiddenInGame`) và hủy Actor (Server) sau khi Ability kết thúc.
2. **Blueprint Hook (Tùy chọn)**: Nếu bạn muốn thực hiện hiệu ứng đặc biệt (VFX, SFX) khi kết thúc quá trình chết:
   - Chuột phải trong `B_Hero_Default`, tìm event **`K2_OnDeathFinished`**.
   - Event này được gọi ngay trước khi Actor bị ẩn/hủy bởi C++.
3. **Camera**: Lớp C++ mặc định sẽ giữ Camera tại vị trí chết. Nếu muốn chuyển Camera sang chế độ quan sát (Spectator), bạn có thể xử lý trong `K2_OnDeathFinished`.

---

## 🏷️ Các Tag và Event quan trọng
| Loại | Tag | Ghi chú |
| :--- | :--- | :--- |
| **Event** | `GameplayEvent.Death` | Dùng để kích hoạt GA |
| **Status** | `Status.Death.Dying` | Trạng thái đang chết (đang chạy animation) |
| **Status** | `Status.Death.Dead` | Trạng thái đã chết hoàn toàn |

---

## ✅ Checklist Kiểm tra
- [ ] Nhân vật có ẩn đi sau khi animation kết thúc không?
- [ ] Camera có chuyển trạng thái không?

> [!TIP]
> Bạn nên lưu danh sách Death Montages trong một biến mảng (Array) tại Character Class để có thể random kiểu chết, giúp gameplay sinh động hơn.
