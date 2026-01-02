# Phân tích và Giải pháp cho RPGGA_AutoRespawn

Tài liệu này giải thích lý do tại sao `W_RespawnTimer` không hiển thị và hướng dẫn cách sửa lỗi thông qua việc sử dụng **Experience Action**.

## � Câu hỏi: Có nên cho vào Action của Experience thay vì PawnData?

**ĐÚNG CHÍNH XÁC.** Đây là mấu chốt của vấn đề.

### Tại sao PawnData không phù hợp?
- Khi nhân vật chết, Actor (Pawn) sẽ bị Unpossess và sau đó bị hủy (Destroy). 
- Nếu `GA_AutoRespawn` được cấp qua `PawnData`, nó sẽ nằm trong `AbilitySystemComponent` của Pawn. Khi Pawn bị hủy, Ability này cũng biến mất ngay lập tức.
- Kết quả là UI không kịp hiện, hoặc hiện lên rồi biến mất ngay, và timer gọi lệnh Respawn cũng không bao giờ chạy xong.

### Tại sao Experience Action là giải pháp đúng?
- Experience Action (thường là `AddAbilities`) sẽ cấp Ability trực tiếp vào **PlayerState**.
- PlayerState tồn tại xuyên suốt trận đấu, không bị hủy khi nhân vật chết và hồi sinh.
- Điều này đảm bảo `GA_AutoRespawn` luôn chạy ngầm, sẵn sàng lắng nghe sự kiện chết của bất kỳ Pawn nào mà Player đó đang điều khiển.

---

## 🛠️ Giải pháp thực hiện

### 1. Di chuyển Ability sang Experience
1. Tìm Data Asset **`B_RPG_Experience`** (hoặc Experience bạn đang dùng).
2. Trong phần **Actions**, tìm hoặc thêm một Action loại **`Add Abilities`**.
3. Thêm `GA_AutoRespawn` vào danh sách **Abilities** của Action này.
4. Xóa `GA_AutoRespawn` khỏi `AbilitySet` của `HeroPawnData` (nếu trước đó bạn đã đặt ở đó).

### 2. Xác minh UI Extension Point
- Mở Blueprint HUD của bạn (ví dụ `W_RPG_HUD`).
- Đảm bảo có một widget **`UI Extension Point`**.
- Trong phần Details của nó, phần **Extension Tag** phải được set là: `HUD.Slot.Reticle`.
- Đây là nơi `W_RespawnTimer` sẽ được "nhét" vào khi code C++ gọi `RegisterExtensionAsWidgetForContext`.

### 3. Kiểm tra Class Widget
- Đảm bảo trong `GA_AutoRespawn`, biến `RespawnWidgetClass` đã được chọn là `W_RespawnTimer`.

---

## ✅ Kết quả mong chờ
Sau khi chuyển sang Experience Action:
1. Bạn nhấn Play.
2. Nhân vật chết.
3. `GA_AutoRespawn` (đang chạy trên PlayerState) nhận tín hiệu.
4. Nó đăng ký `W_RespawnTimer` vào HUD.
5. Widget hiện lên và bắt đầu đếm ngược vì nó nhận được Gameplay Message từ C++.
6. Sau 5 giây, nhân vật hồi sinh và Widget tự động biến mất khi Ability kết thúc hoặc bị remove (tùy logic).

> [!IMPORTANT]
> Trong Lyra, các khả năng mang tính chất "hệ thống" hoặc "quản lý" (như Respawn, Spectating, Scoreboard) luôn nên nằm ở **PlayerState** thông qua Experience Actions.
