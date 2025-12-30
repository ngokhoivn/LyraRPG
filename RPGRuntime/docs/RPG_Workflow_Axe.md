# Quy trình Triển khai Hệ thống Vũ khí RPG (Axe Workflow)

Tài liệu này hướng dẫn chi tiết quy trình triển khai trong Unreal Editor để đưa các lớp C++ vào vận hành và thiết lập vũ khí Rìu (Axe).

> [!NOTE]
> Để xem danh sách các Class C++ Framework hỗ trợ, vui lòng tham khảo [RPG_Standalone_Framework.md](file:///f:/UnrealProject/LyraRPG/Lyra/Plugins/GameFeatures/RPG/Source/RPGRuntime/docs/RPG_Standalone_Framework.md).

---

## Quy trình triển khai trong Unreal Editor (Axe Implementation)

### 1. Cấu hình Game Feature Data Asset
Mở file **RPG_Core** (Game Feature Data Asset). Tại mục **Actions**, thêm **Add Components**:
- **Component 1 (Quản lý Trang bị):**
    - Actor Class: `LyraCharacter` (hoặc Actor gốc nhân vật của bạn)
    - Component Class: `RPGEquipmentManagerComponent`
    - Cấu hình: Bật cả **Server** và **Client**.
- **Component 2 (Quản lý Kho đồ):**
    - Actor Class: `Controller`
    - Component Class: `RPGInventoryManagerComponent`
    - Cấu hình: Bật cả **Server** và **Client**.

![image.png](attachment:30255d41-b1d7-4b08-b2b0-e9d552f10e35:image.png)

### 2. Cấu hình Asset Manager
Trong **Project Settings**, đảm bảo Asset Manager có thể quét các Action Set mới:
- **Primary Asset Type:** `RPGExperienceActionSet`
- **Asset Base Class:** `RPGExperienceActionSet`
- **Directory:** `/RPG/Experiences/`

![image.png](attachment:1c917beb-09b1-420f-af1e-4e341bd975c7:image.png)

### 3. Tạo Action Set cho Quickbar
Tại thư mục `/RPG/Experiences/`, tạo Data Asset kế thừa từ `RPGExperienceActionSet`. Tên: **LAS_RPG_SharedComponents**.
- **Action - Add Component:**
    - Actor Class: `Controller`
    - Component Class: `RPGQuickbarComponent`
    - Cấu hình: Bật cả **Server** và **Client**.
- **GameFeaturesToEnable:** Thêm `RPG`.

![image.png](attachment:c8d260e5-a37a-4d6e-a4a8-2b6a9a4feb94:image.png)

### 4. Gán Action Set vào Experience
Mở **B_RPG_Experience** (Experience Definition).
- Trong mục **Action Sets**, thêm **LAS_RPG_SharedComponents** vào danh sách.

![image.png](attachment:816a79d8-62e2-4147-b5a6-68164545b585:image.png)

### 5. Node Kiểm tra Trạng thái Sẵn sàng
Sử dụng node C++ **Wait For Inventory Ready** đã được tích hợp sẵn:
- Class: `RPGAsyncAction_WaitForInventoryReady`
- Node: `Wait For Inventory Ready` -> Pin: `On Ready`.

### 6. Logic Kho đồ trong Blueprint Nhân vật (Base Class)
Mở Class cha của nhân vật (ví dụ: **B_Hero_Default**):
- **Biến `InitialInventoryItems`**: Kiểu `RPGInventoryItemDefinition` (Class Reference), chuyển thành Array.
- **Hàm `AddInitialInventory`**: 
    - Duyệt `InitialInventoryItems`.
    - Gọi `AddItemDefinition` trên `RPGInventoryManager`.
    - Gọi `AddItemToSlot` trên `RPGQuickbar`.
- **Hàm `ClearInventory`**: Dọn dẹp các Slot và Instance của item.

### 7. Áp dụng cho Axe (Rìu)
Tại Blueprint nhân vật cụ thể (ví dụ: **B_Hero_Ulag**):
- Gán **ID_Axe** vào `InitialInventoryItems`.
- Tại **Event Possessed**:
    1. Gọi **Wait For Experience Ready**.
    2. Nối vào node **Wait For Inventory Ready**.
    3. Sau khi sẵn sàng, gọi hàm **AddInitialInventory**.

---
*Tài liệu được cập nhật dựa trên tiến độ triển khai thực tế - 2025.*
