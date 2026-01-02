# Sửa lỗi: W_RespawnTimer Không Hiển Thị

## Mô tả lỗi
Widget `W_RespawnTimer` (được gán trong `GA_AutoRespawn`) không xuất hiện trên màn hình khi nhân vật chính chết, mặc dù logic hồi sinh (respawn) vẫn hoạt động bình thường.

## Nguyên nhân gốc rễ & Giải pháp

### 1. Vị trí cấp Ability (Đã giải quyết)
- **Vấn đề**: Cấp `GA_AutoRespawn` cho **Pawn** (thông qua `PawnData`) khiến Ability bị hủy khi nhân vật chết, đúng vào thời điểm UI cần hiển thị.
- **Giải pháp**: Cấp Ability cho **PlayerState**. PlayerState tồn tại xuyên suốt quá trình hồi sinh.
- **Triển khai**: Sử dụng action tùy chỉnh `URPGGameFeatureAction_AddAbilities` trong Experience Definition để nhắm mục tiêu vào `RPGPlayerState`.

### 2. Kiểm tra UI Extension Point (Vị trí đặt Slot)
- **Vấn đề**: Slot `HUD.Slot.Reticle` bị đặt sai chỗ (đặt ở lớp Khung bên ngoài thay vì lớp HUD bên trong).
- **Yêu cầu**: `UIExtensionPointWidget` với tag `HUD.Slot.Reticle` **PHẢI** nằm trong **`W_RPG_HUDLayout`**.
- **Chi tiết cấu trúc**: Xem tại [RPG UI Structure](f:/UnrealProject/LyraRPG/Lyra/Plugins/GameFeatures/RPG/Source/RPGRuntime/docs/UI/RPG_UI_Structure.md).
- **Hành động**: Đảm bảo `W_RPG_OverallUILayout` chỉ chứa các Stack, còn `W_RPG_HUDLayout` (nằm trong GameLayer_Stack) mới là nơi chứa Slot.

### 3. Lỗi lệch kiểu cấu trúc Message (Struct Type Mismatch)
- **Vấn đề**: Log báo lỗi `Struct type mismatch`. Code C++ gửi tin nhắn kiểu `RPGRespawnTimerDurationMessage`, nhưng Widget `W_RespawnTimer` (copy từ Lyra) lại đang đợi kiểu `LyraInteractionDurationMessage`.
- **Hành động**: 
    1. Mở Blueprint **`W_RespawnTimer`**.
    2. Tìm node **Register Gameplay Message Listener**.
    3. Ở phần **Payload Type**, đổi từ `LyraInteractionDurationMessage` sang **`RPGRespawnTimerDurationMessage`**.
    4. Cập nhật lại các node nối vào Payload để lấy giá trị `RespawnDelay`.

### 3. Liên kết Class Widget
- **Yêu cầu**: Blueprint `GA_AutoRespawn` phải được gán class `W_RespawnTimer` vào thuộc tính `RespawnWidgetClass`.
- **Hành động**: Mở `GA_AutoRespawn` và kiểm tra trình chọn class.

### 4. Kích hoạt ở phía Client (Client-Side)
- **Vấn đề**: Nếu Ability chỉ được kích hoạt trên Server, hàm `RegisterRespawnUI` (chỉ chạy ở Client) sẽ không hoạt động.
- **Xác minh**: Đã thêm `UE_LOG` vào `RPGGA_AutoRespawn::ActivateAbility` để xác nhận nó có chạy trên client đang điều khiển nhân vật hay không.

### 5. Thời điểm gửi tin nhắn (Message Timing)
- **Vấn đề**: Tin nhắn `Message.Respawn.Duration` có thể được phát đi trước khi widget hoàn tất việc khởi tạo và lắng nghe.
- **Giám sát**: Đã thêm các bản log chẩn đoán vào `RPGGA_AutoRespawn.cpp` để theo dõi thời điểm phát tin nhắn so với thời điểm đăng ký UI.

## Log chẩn đoán (C++)
Đã thêm vết (trace) trong `RPGGA_AutoRespawn.cpp`:
- `URPGGA_AutoRespawn::ActivateAbility`: Theo dõi nếu được điều khiển cục bộ (locally controlled).
- `URPGGA_AutoRespawn::RegisterRespawnUI`: Theo dõi việc đăng ký thành công và cảnh báo nếu Widget Class bị trống.
- `URPGGA_AutoRespawn::OnDeathStarted`: Theo dõi việc phát tin nhắn và thời gian chờ.

## Kiểm tra phía Blueprint (QUAN TRỌNG)
Nếu log C++ báo thành công nhưng vẫn chưa thấy UI, hãy kiểm tra:

### 1. Slot (UI Extension Point) trong HUD
- Mở Blueprint layout HUD chính (ví dụ: `W_RPG_HUD`).
- Tìm widget **UI Extension Point**.
- Đảm bảo thuộc tính **Extension Point Tag** của nó là `HUD.Slot.Reticle`.
- Kiểm tra xem widget này có bị ẩn (`Visibility`) hay nằm ngoài màn hình không.

### 2. Widget W_RespawnTimer
- Đảm bảo widget này lắng nghe tin nhắn với tag mới: `Ability.Respawn.Duration`.
- Kiểm tra logic `OnMessageReceived` trong Widget xem nó có thực sự bật Visibility của chính nó lên không.

## Cách xác minh qua Log mới
Sau khi compile code mới, hãy tìm dòng này trong Output Log:
- `RPGUIPolicy: Creating Root Layout [B_RPG_HUD_C] for Player [LyraLocalPlayer_1]`
  -> Dòng này xác nhận HUD đã được tạo thành công sau khi retry.
- `Registering widget W_RespawnTimer_C to slot HUD.Slot.Reticle with context LyraLocalPlayer_1`
  -> Dòng này xác nhận UI đã được đăng ký đúng context người chơi.
