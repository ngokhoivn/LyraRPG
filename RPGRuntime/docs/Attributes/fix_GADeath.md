# Hướng dẫn Kiểm tra Lỗi Kích hoạt GA_Death

Tài liệu này cung cấp các bước để kiểm tra tại sao `GA_Death` không kích hoạt khi nhân vật hết máu. Để hiểu rõ cách các Ability được kết nối trong dự án, hãy xem [Cấu trúc GA](file:///f:/UnrealProject/LyraRPG/Lyra/Plugins/GameFeatures/RPG/Source/RPGRuntime/docs/Attributes/RPG_Ability_Structure.md).

## 1. Kết quả Fix lỗi thành công
✅ **NGUYÊN NHÂN**: `AbilitySet_RPGHero` chứa `GA_Death` chưa được nối vào `HeroPawnData`.
✅ **GIẢI PHÁP**: Đã thêm `AbilitySet_RPGHero` vào mảng **Ability Sets** của `HeroPawnData`.

Dựa trên log phân tích:
- `RPGHealthComponent: Sending GameplayEvent.Death` chạy OK.
- `RPGGA_Death: ActivateAbility started` chạy OK sau khi nối Asset.

## 2. Hướng dẫn Kiểm tra Chi tiết (Checklist)

### A. Lỗi Grant Ability (Log event có, Log GA không có)
- [ ] `GA_Hero_Death` đã được thêm vào **Ability Set** chưa?
- [ ] **Ability Set** (`AbilitySet_RPGHero`) đã được gán vào **HeroPawnData** chưa? (Xem sơ đồ cấu hình trong file Structure bên trên).
- [ ] Character Blueprint có đang sử dụng đúng **HeroPawnData** đó không?

### B. Lỗi Animation (Cả 2 Log đều có nhưng nhân vật đứng yên)
- [ ] Montage đã gán vào mảng `Death Montages` (trong Ability hoặc trong Character) chưa?
- [ ] **Slot name** trong Montage có phải là `FullBody` không?
- [ ] Anim Blueprint đã có node `Slot 'FullBody'` chưa?
- [ ] Kiểm tra xem Montage có bị **Interrupt** bởi một Ability khác không? (Death nên có Activation Group là `Exclusive_Blocking`).

### C. Lỗi Event Tag (Log event không có)
- [ ] Kiểm tra xem có hiệu ứng GameplayEffect nào đang "hồi máu" ngay lập tức khiến máu không bao giờ chạm mốc 0 không?
- [ ] Kiểm tra `HandleOutOfHealth` trong `RPGHealthComponent` có bị ghi đè (override) sai trong Blueprint không?

---
*Ghi chú: Luôn ưu tiên kiểm tra Output Log trước để khoanh vùng lỗi.*
