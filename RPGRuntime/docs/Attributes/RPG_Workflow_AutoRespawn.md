# Workflow: Hệ thống Tự động Hồi sinh (Auto Respawn)

Tài liệu này hướng dẫn chi tiết cách thiết lập hệ thống hồi sinh tự động cho nhân vật, sử dụng lớp C++ `RPGGA_AutoRespawn` và tích hợp với Lyra Experience.

## 1. Nguyên lý hoạt động
Hệ thống hồi sinh hoạt động như một **Background Service** chạy suốt vòng đời của người chơi (Player). 

> [!IMPORTANT]
> Để hệ thống này hoạt động bền vững, Ability phải được cấp cho **PlayerState** thay vì Pawn. Điều này đảm bảo khi Pawn chết và bị hủy, Ability vẫn tồn tại để điều khiển quá trình hồi sinh.

### Thành phần tham gia:
- **`RPGGA_AutoRespawn` (C++)**: Class xử lý logic đếm ngược, lắng nghe sự kiện chết và yêu cầu hồi sinh từ GameMode.
- **`GA_AutoRespawn` (Blueprint)**: Cấu hình thời gian chờ (`RespawnDelayDuration`) và lớp giao diện hiển thị (`RespawnWidgetClass`).
- **`W_RespawnTimer` (Widget)**: Giao diện hiển thị đếm ngược (copy từ ShooterCore).
- **`UI Extension Subsystem`**: Tự động chèn UI vào HUD khi bắt đầu quá trình hồi sinh.
- **`Gameplay Message Subsystem`**: Truyền đạt thời gian đếm ngược từ C++ tới UI.

---

## 2. Các bước thiết lập chi tiết

### Bước 1: Cấu hình Blueprint Ability
1. Tạo một Blueprint dựa trên class **`RPGGA_AutoRespawn`** (đặt tên là `GA_AutoRespawn`).
2. Trong tab **Details**, cấu hình:
   - **Respawn Delay Duration**: Ví dụ `5.0`.
   - **Respawn Widget Class**: Chọn `W_RespawnTimer`.
   - **Instancing Policy**: `Instanced Per Actor`.
   - **Net Execution Policy**: `Local Predicted`.

### Bước 1.5: Tạo Ability Set cho PlayerState
1. Chuột phải trong Content Browser -> **Miscellaneous** -> **Data Asset**.
2. Chọn class **`RPGAbilitySet`** (được tạo từ code C++).
3. Đặt tên là **`AbilitySet_RPG_PlayerState`**.
4. Mở Data Asset:
   - Tại mục **Granted Gameplay Abilities**, thêm một phần tử.
   - Chọn **Ability**: `GA_AutoRespawn`.
   - **Input Tag**: Để trống (vì đây là Ability chạy ngầm).

### Bước 2: Cấp kỹ năng qua Experience (QUAN TRỌNG)
Để Ability không bị mất khi nhân vật chết:
1. Mở Data Asset Experience của bạn (ví dụ: `B_RPG_Experience`).
2. Tại mục **Actions**, bạn cần sử dụng action loại **`Add Abilities (RPG)`** (Class này sẽ được tạo ở bước tiếp theo để hỗ trợ `URPGAbilitySet`).
3. Chọn **Actor Class** là `RPGPlayerState`.
4. Thêm `AbilitySet_RPG_PlayerState` vào danh sách **Ability Sets** của action này.

### Bước 2.5: Lưu ý về Standalone (Custom Action)
Trong quá trình chuyển đổi sang Standalone, action `Add Abilities` mặc định của Lyra có thể không tìm thấy `URPGAbilitySet`. Do đó, chúng ta triển khai:
- **`URPGGameFeatureAction_AddAbilities`**: Một class custom kế thừa từ `URPGGameFeatureAction_WorldActionBase`.
- Action này cho phép gán trực tiếp các asset `RPGAbilitySet` vào PlayerState ngay khi Experience được tải.

### Bước 3: Cấu hình HUD Extension Point
Để UI Hồi sinh có thể hiển thị:
1. Mở Blueprint chứa HUD Layout của bạn (ví dụ: **`W_RPG_HUDLayout`**).
   - Xem thêm về cấu hình HUD tại: [RPG UI Structure](f:/UnrealProject\LyraRPG\Lyra\Plugins\GameFeatures\RPG\Source\RPGRuntime\docs\UI\RPG_UI_Structure.md).
2. Thêm hoặc tìm một widget **`UI Extension Point`**.
3. Trong phần **Extension Point Tag**, gán tag: `HUD.Slot.Reticle`.
4. Đây là "khe cắm" mà code C++ sẽ sử dụng để hiển thị `W_RespawnTimer`.

---

## 3. Quy trình thực thi (Engine Logic)

1. **Khi vào Game**: `GA_AutoRespawn` được cấp cho PlayerState và tự động kích hoạt (nếu để `ActivationPolicy = OnSpawn`).
2. **Khi nhân vật Chết**:
   - `RPGGA_AutoRespawn` nhận tín hiệu từ `HealthComponent->OnDeathStarted`.
   - C++ gọi `RegisterRespawnUI()` để chèn widget vào HUD.
   - C++ Broadcast Message `Message.Respawn.Duration` kèm thời gian delay.
3. **Trong khi chờ**:
   - `W_RespawnTimer` nhận Message và bắt đầu đếm ngược visual.
   - Một Timer trong C++ đếm ngược logic.
4. **Khi hết thời gian**:
   - C++ yêu cầu GameMode gọi `RequestPlayerRestartNextFrame`.
   - C++ Broadcast Message `Message.Respawn.Completed` để UI dọn dẹp (nếu cần).
   - Pawn cũ bị hủy để tránh xác chết tồn đồn (Ghost character).

---

## 4. Checklist Kiểm tra lỗi (Troubleshooting)

- [ ] **UI không hiện?**: Kiểm tra HUD đã có `UIExtensionPoint` với tag `HUD.Slot.Reticle` chưa. Lưu ý nó phải nằm trong **`W_RPG_HUDLayout`** thay vì các lớp layout bên ngoài. (Xem [Hướng dẫn sửa lỗi UI](f:/UnrealProject\LyraRPG\Lyra\Plugins\GameFeatures\RPG\Source\RPGRuntime\docs\UI\fix_RespawnUIVisibility.md)).
- [ ] **Ability biến mất sau khi chết?**: Bạn đang cấp Ability cho Pawn thay vì PlayerState qua Experience Action.
- [ ] **Nhân vật không hồi sinh?**: Kiểm tra class GameMode có kế thừa từ `ARPGGameMode` và hỗ trợ `RequestPlayerRestartNextFrame` không.
- [ ] **Máy khách (Client) không thấy UI?**: Đảm bảo `NetExecutionPolicy` là `LocalPredicted` và hàm đăng ký UI được gọi trong scope `IsLocallyControlled()`.

---

> [!TIP]
> Bạn có thể mở rộng `GA_AutoRespawn` trong Blueprint để thêm hiệu ứng màn hình mờ dần (Fade out) hoặc âm thanh kịch tính khi sắp hồi sinh.
