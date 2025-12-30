# RPG Architecture & Design Guide

Tài liệu này trình bày chi tiết về kiến trúc hệ thống, mục đích của các thành phần và quy trình vận hành trong dự án RPG dựa trên nền tảng Lyra Framework.

---

## Phần 1: Các Thành phần Cốt lõi (Components) & Cấu hình
Mục tiêu: Đảm bảo tính tách biệt trách nhiệm (Separation of Concerns) và quản lý dữ liệu linh hoạt.

| Bước | Thành phần | Vị trí | Mục đích & Lý do Kiến trúc |
| :--- | :--- | :--- | :--- |
| **1** | **EquipmentManager** | Character | **Quản lý Trang bị thực tế**: Gắn trực tiếp vào nhân vật để xử lý việc hiển thị (Skeletal Mesh vũ khí) và các hiệu ứng game (Ability System, Modifier) khi vật phẩm được cầm trên tay. |
| **2** | **InventoryManager** | Controller | **Quản lý "Túi đồ" Logic**: Đặt ở Controller giúp kho đồ tồn tại xuyên suốt vòng đời của người chơi ngay cả khi nhân vật chết và respawn. Tách biệt logic sở hữu khỏi hình thể. |
| **3** | **QuickbarComponent** | Controller | **Quản lý Thanh phím tắt**: Lớp quản lý cao cấp điều phối giữa Inventory (lấy item) và Equipment (lệnh trang bị). Ánh xạ vật phẩm vào các slot (0, 1, 2...) để người chơi chọn nhanh. |
| **4** | **Action Set** | Experience | **Tính Modular**: Nhóm các component "dùng chung" (LAS_RPG_SharedComponents) vào Experience giúp dễ dàng bật/tắt tính năng cho các chế độ game khác nhau và đảm bảo thứ tự tải an toàn. Sử dụng `RPGExperienceActionSet`. |
| **5** | **Asset Manager** | Project Settings | **Quét và Tải dữ liệu**: Khai báo để Lyra Framework nhận diện các loại asset tùy chỉnh (như `RPGExperienceActionSet`). Giúp tránh lỗi null khi nạp game. |

---

## Phần 2: Xử lý Bất đồng bộ (Async Handling)
Mục tiêu: Giải quyết vấn đề "Race Condition" trong môi trường mạng và nạp dữ liệu song song.

### Macro: `WaitForInventoryReadiness`
*   **Vấn đề**: Khi sự kiện `Possessed` xảy ra, không có gì đảm bảo các Component (Inventory, Quickbar) đã được khởi tạo xong bởi Action Set.
*   **Giải pháp**: Tạo một vòng lặp "chờ đợi tích cực" (Active Waiting) kiểm tra sự tồn tại của Controller và các Component liên quan.
*   **Lợi ích**: Ngăn chặn lỗi Null Reference và đảm bảo logic `AddInitialInventory` chỉ chạy khi môi trường đã hoàn toàn sẵn sàng.

---

## Phần 3: Logic Blueprint trong Nhân vật (Character)
Mục tiêu: Thiết kế hướng dữ liệu (Data-Driven) và quản lý vòng đời bộ nhớ.

### 1. Dữ liệu Khởi tạo (`InitialInventoryItems`)
*   Sử dụng mảng Class Reference của `ItemDefinition`.
*   Giúp Designer dễ dàng thay đổi trang bị ban đầu cho từng loại nhân vật (Warrior, Mage, Rogue) mà không cần can thiệp vào code.

### 2. Quy trình `AddInitialInventory` (Luồng 3 bước chuẩn)
1.  **Sở hữu (Inventory)**: Khởi tạo Item Instance từ Definition và đưa vào `InventoryManager`.
2.  **Gán Slot (Quickbar)**: Liên kết Instance đó vào một slot cụ thể trên thanh phím tắt.
3.  **Trang bị (Equipment)**: Kích hoạt slot, ra lệnh cho `EquipmentManager` thực hiện hành động cầm vũ khí và nạp Animation.

### 3. Giải phóng Bộ nhớ (`ClearInventory`)
*   **Mục đích**: Hủy tất cả Item Instance khi nhân vật chết hoặc reset.
*   **Lý do**: Tránh rò rỉ bộ nhớ (Memory Leak) – một vấn đề cực kỳ nghiêm trọng trong các game chơi lâu hoặc môi trường server.

---

## Phần 4: Vòng đời Vận hành (Lifecycle)
Mục tiêu: Xác định thời điểm "vàng" cho các thao tác.

*   **Sự kiện Possessed**: Là thời điểm an toàn nhất vì nhân vật đã có Controller điều khiển, Mesh và Socket đã sẵn sàng để gắn vũ khí.
*   **Wait For Experience Ready**: Kết hợp với bước kiểm tra Component để tạo ra chuỗi logic hoàn chỉnh: 
    `Experience Ready -> Components Ready -> Character Ready -> Add Items`.

---

## Tổng kết Tư duy Thiết kế (Design Philosophy)

1.  **Tách biệt Trách nhiệm**: Inventory biết bạn có gì, Quickbar biết bạn muốn dùng gì, Equipment biết cách gắn nó lên người.
2.  **Data-Driven**: Sử dụng Data Asset tối đa để tăng khả năng mở rộng.
3.  **An toàn Bất đồng bộ**: Luôn dùng các cơ chế Wait/Delay để đảm bảo tài nguyên sẵn sàng.
4.  **Quản lý Vòng đời**: Luôn có cặp hàm Khởi tạo (`Add`) và Hủy (`Clear`) để đảm bảo tính ổn định.
