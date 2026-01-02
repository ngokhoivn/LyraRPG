# Cấu trúc Quản lý Gameplay Ability (GA)

Tài liệu này bản đồ hóa cách các Gameplay Ability được gán cho nhân vật thông qua các Data Asset, giúp tra cứu và sửa lỗi nhanh.

## 1. Sơ đồ Kết nối (Hierarchy)

Trong dự án này, khả năng của nhân vật được cấp phát qua 2 con đường chính:

### A. Khả năng theo Trang bị (Equipment-based)
Dùng cho các chiêu thức gắn liền với vũ khí (Axe, Bow, Sword).
`PawnData (HeroPawnData)` 
  └── `InitialInventoryItems`
        └── `InventoryItemDefinition`
              └── `EquipmentDefinition (wid_Axe)`
                    └── `AbilitySetsToGrant`
                          └── `AbilitySet`
                                └── **`GA_Test_Damage`, `GA_Test_Health`**

### B. Khả năng Hệ thống/Toàn cục (Global/Hero-based)
Dùng cho các khả năng mặc định (Chết, Nhảy, Dash, Né).
`PawnData (HeroPawnData)`
  └── `AbilitySets` (Mảng Global)
        └── `AbilitySet (AbilitySet_RPGHero)`
              └── **`GA_Death`, `GA_Jump`**

---

## 2. Danh sách Data Asset Quan trọng

| Asset Name | Loại Class | Mục đích | Kết nối tới |
| :--- | :--- | :--- | :--- |
| **`HeroPawnData`** | `RPGPawnData` | Trung tâm cấu hình cho Hero | (Gốc) |
| **`wid_Axe`** | `RPGEquipmentDefinition` | Cấu hình vũ khí Rìu | `HeroPawnData` |
| **`AbilitySet_RPGHero`** | `RPGAbilitySet` | Chứa các GA chung (như Death) | `HeroPawnData` |

---

## 3. Hướng dẫn sửa lỗi nhanh (Troubleshooting)

### Nếu mất một Ability nào đó:
1. **Kiểm tra "Trung tâm"**: Mở `HeroPawnData`.
2. **Kiểm tra nhánh Global**: Xem mảng `Ability Sets` trong `HeroPawnData` có chứa `AbilitySet_RPGHero` chưa? (Đây là nơi chứa `GA_Death`).
3. **Kiểm tra nhánh Trang bị**: Xem vũ khí đang cầm có dẫn tới `Ability Set` chứa GA đó không?
4. **Kiểm tra Granting**: Trong `Ability Set`, đảm bảo GA đã được thêm vào mục `Granted Gameplay Abilities` (không phải mục Effect hay Attribute).

### Lưu ý về GA_Death:
- `GA_Death` phải nằm trong nhánh **Global**, vì nhân vật có thể chết ngay cả khi không cầm bất kỳ vũ khí nào.
- Đảm bảo `HeroPawnData` luôn có `AbilitySet_RPGHero` trong mảng `Ability Sets`.
