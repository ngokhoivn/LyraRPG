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

---
*Cập nhật: 2025 - Tích hợp hệ thống Death & Footsteps chuyên biệt.*
