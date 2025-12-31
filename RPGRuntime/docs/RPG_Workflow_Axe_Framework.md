# RPG Workflow Axe Framework (C++ Changes)

Tài liệu này chi tiết các lớp C++ và Interface được tạo mới hoặc cập nhật để hỗ trợ quy trình triển khai vũ khí Rìu (Axe).

## 1. Interface Hiệu ứng (Feedback System)
### [NEW] `IRPGContextEffectsInterface`
- **Vị trí**: `Public/Feedback/ContextEffects/RPGContextEffectsInterface.h`
- **Mục đích**: Cung cấp hàm `AnimMotionEffect` để giao tiếp giữa Animation và Actor hiệu ứng.
- **Tiêu chuẩn**: Được refactor từ Lyra để hoạt động độc lập trong RPG Plugin.

### [UPDATE] `URPGExperienceDefinition` & `URPGExperienceActionSet`
- Thêm override `GetPrimaryAssetId()`: Đảm bảo Asset Manager nhận diện đúng loại Asset (Type) ngay cả khi class được kế thừa trong Blueprint. Điều này giúp sửa lỗi Crash "Assertion failed: AssetClass" trong Editor.

## 2. Hệ thống Nhân vật (Character System)
### [NEW] `ARPGHero_Character`
- **Vị trí**: `Public/Character/RPGHero_Character.h`
- **Kế thừa**: `ARPGCharacter`
- **Tính năng mới**:
    - `DeathMontages`: Mảng các hoạt ảnh chết để chọn ngẫu nhiên.
    - `OnDeathStarted`: Override lại logic bắt đầu chết để tích hợp AI Unregister và Ragdoll delay.
    - `AnimMotionEffect_Implementation`: Implement interface hiệu ứng bước chân.
    - `HideEquippedWeapons`: Tích hợp với Equipment Manager để dọn dẹp hiển thị vũ khí khi chết.

## 3. Hệ thống Trang bị (Equipment System)
### [UPDATE] `URPGWeaponInstance`
- Thêm hàm `SetActorsHidden(bool bHidden)`: Cho phép ẩn/hiện toàn bộ các Actor được spawn bởi vũ khí (ví dụ: Mesh của rìu).

### [UPDATE] `URPGEquipmentManagerComponent`
- Thêm hàm `SetAllWeaponsHidden(bool bHidden)`: Duyệt qua tất cả các trang bị đang được active và gọi ẩn/hiện chúng.

## 4. Chi tiết Kỹ thuật Luồng Hoạt động (Technical Breakdown)

### Qúa trình Tạo và Trang bị
- **Bước 1: `InventoryManager->AddItemDefinition`**: Nhận vào `ID_Greatsword`, tạo ra một `URPGInventoryItemInstance`. Đây là nơi lưu trữ dữ liệu động (độ bền, thuộc tính riêng).
- **Bước 2: `Quickbar->AddItemToSlot`**: Ánh xạ `Instance` vừa tạo vào một ô phím tắt (0-9).
- **Bước 3: `EquipmentManager->EquipItem`**: Khi slot được kích hoạt, nó lấy `URPGEquipmentDefinition` từ Fragment của Item và thực hiện:
    - Spawn `URPGWeaponInstance`.
    - Gắn Mesh vào Socket trên Character.
    - Cấp (Grant) các Gameplay Abilities cho nhân vật.

---
## 5. Các lỗi thường gặp và cách khắc phục (Common Issues)

### Lỗi Biên dịch sau khi làm sạch dự án (Cleanup)
Nếu bạn gặp lỗi liên quan đến `'PrimaryAssetTypesToScan': undeclared identifier` hoặc `'PrimaryAssetTypesToScan': is not a member of 'URPGAssetManager'`:

**Nguyên nhân:** 
Do sự thay đổi trong cấu trúc Engine (đặc biệt là các bản cập nhật mới) hoặc lỗi liên kết sau khi regenerate dự án, khiến compiler không nhận diện được các thành viên bảo vệ (protected) từ class cha.

**Cách khắc phục TRIỆT ĐỂ:** 
Để đảm bảo tính ổn định cao nhất và tuân thủ kiến trúc của Lyra, chúng ta sẽ chuyển toàn bộ việc quét Asset sang file cấu hình và để code C++ trống phần này.

1. **Trong C++ (`RPGAssetManager.cpp`):** Hãy comment hoặc xóa các dòng đăng ký trong `StartInitialLoading()`:
   ```cpp
   void URPGAssetManager::StartInitialLoading()
   {
       Super::StartInitialLoading();
       // Đã chuyển sang cấu hình trong DefaultGame.ini để tránh lỗi biên dịch
       /*
       this->PrimaryAssetTypesToScan.Add(...);
       */
   }
   ```

2. **Trong `DefaultGame.ini`:** Đảm bảo bạn đã có các dòng sau (đã hướng dẫn trong workflow chính):
   ```ini
   [/Script/Engine.AssetManagerSettings]
   +PrimaryAssetTypesToScan=(PrimaryAssetType="RPGExperienceDefinition",AssetBaseClass=/Script/RPGRuntime.RPGExperienceDefinition,bHasBlueprintClasses=True,bIsEditorOnly=False,bDirectoriesShowBackslashes=True)
   +PrimaryAssetTypesToScan=(PrimaryAssetType="RPGExperienceActionSet",AssetBaseClass=/Script/RPGRuntime.RPGExperienceActionSet,bHasBlueprintClasses=True,bIsEditorOnly=False,bDirectoriesShowBackslashes=True)
   ```

---
*Cập nhật: 2025 - Chuyển sang kiến trúc Scan Asset qua Config để tối ưu hóa tính ổn định.*
