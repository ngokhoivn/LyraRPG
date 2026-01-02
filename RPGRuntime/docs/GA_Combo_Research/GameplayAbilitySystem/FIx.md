# Tiến trình Sửa lỗi: Input Không Áp dụng Thay đổi Máu

## 🛑 Vấn đề Hiện tại
Người chơi nhấn Chuột trái (LMB) và Chuột phải (RMB) nhưng không thấy máu tăng hoặc giảm trên nhân vật, mặc dù Input Component có vẻ đã nhận tín hiệu.

## 🔍 Phân tích Nguyên nhân
1. **Input Binding**: Hệ thống Input trong `RPGHeroComponent.cpp` sử dụng cơ chế "Smart Fallback" để tương thích với Lyra. Các hàm `Input_AbilityInputTagPressed` đã được gọi, nhưng có thể chưa có Ability nào được gán cho các Tag tương ứng.
2. **Thiếu Ability Test**: Dự án hiện chưa có một Gameplay Ability (GA) đơn giản để thực hiện việc cộng/trừ máu trực tiếp nhằm kiểm tra luồng xử lý Attribute.
3. **Attribute Mapping**: `RPGAttributeSet` sử dụng các Meta-attributes `Damage` và `Healing` để xử lý máu. Cần một GA áp dụng các Effect vào các Attribute này.

## 🛠️ Giải pháp Đề xuất: GA_Template_Test
Tôi sẽ triển khai một class test chuyên dụng mang tên `URPGGA_ModifyHealth`.

### 1. Cấu trúc GA Test (Chuẩn C++)
- **Class**: `URPGGA_ModifyHealth` (Kế thừa từ `URPGGameplayAbility`).
- **Thông số**: 
    - `Value`: Lượng giá trị thay đổi.
    - `bIsHealing`: Xác định tác động vào `Damage` (giảm máu) hay `Healing` (tăng máu).

### 2. Luồng thực hiện (Workflow)
- [ ] **Bước 1**: Viết code C++ cho `URPGGA_ModifyHealth`.
- [ ] **Bước 2**: Compile và tạo 2 Blueprint:
    - `GA_Test_Damage`: Gán `Value = 10`, `bIsHealing = false`.
    - `GA_Test_Heal`: Gán `Value = 10`, `bIsHealing = true`.
- [ ] **Bước 3**: Cấu hình `URPGInputConfig`:
    - Map `InputTag.Ability.Attack` -> LMB.
    - Map `InputTag.Ability.Heal` -> RMB.
- [ ] **Bước 4**: Cấu hình `URPGAbilitySet`:
    - Gán `GA_Test_Damage` cho Tag `InputTag.Ability.Attack`.
    - Gán `GA_Test_Heal` cho Tag `InputTag.Ability.Heal`.

## ✅ Trạng thái Hiện tại
- [x] Nghiên cứu luồng Input (`RPGHeroComponent`, `RPGPlayerController`).
- [x] Nghiên cứu cấu trúc Attribute (`RPGAttributeSet`).
- [x] Triển khai Code C++ cho GA Test (`URPGGA_ModifyHealth`).
- [x] **Bản vá khẩn cấp (Emergency Patch)**: Đã sửa GA để tác động trực tiếp vào `Health` nhằm bỏ qua logic GE.
- [x] **Sửa lỗi Compile**: Thêm `#include "System/RPGLogChannels.h"` để sửa lỗi `'LogRPG': undeclared identifier`.
- [/] Đang phân tích Log để tìm điểm nghẽn tiếp theo.

---

## 🔍 7. Hướng dẫn Phân tích Log (Luồng Input -> GAS)

Để tìm chính xác nơi bị "nghẽn", bạn hãy nhập từ khóa **`LogRPG`** vào ô Filter của Output Log và quan sát các dòng sau:

### Kịch bản 1: Nghẽn tại HeroComponent (Input không vào ASC)
- **Log cần tìm**: `Input_AbilityInputTagPressed: Tag: [Tên Tag]`
- **Nếu KHÔNG thấy**: Input chưa map đúng vào `HeroComponent`. Kiểm tra `RPG Pawn Data` và `Input Mapping Context (IMC)`.

### Kịch bản 2: Nghẽn tại Mapping (ASC không tìm thấy Ability)
- **Log cần tìm**: `AbilityInputTagPressed: Matching tag [Tên Tag] with ability [Tên GA]`
- **Nếu KHÔNG thấy**: `AbilitySystemComponent` không tìm thấy GA nào khớp với Tag bạn vừa nhấn.
- **Khắc phục**: Kiểm tra lại `RPGAbilitySet` (Axe). Tag `InputTag.SelfDamage` phải trùng khớp giữa `InputConfig` và `AbilitySet`.

### Kịch bản 3: Nghẽn tại Activation Policy
- **Log cần tìm**: `ProcessAbilityInput: Found Ability [Tên GA] for Input`
- **Nếu thấy Log này nhưng KHÔNG thấy log của GA**: Ability đã được tìm thấy nhưng không thể Activate.
- **Khắc phục**: Mở Blueprint `GA_Test_Damage`, kiểm tra **Activation Policy** phải là **`On Input Triggered`**. 

### Kịch bản 4: GA đã chạy nhưng không đổi máu (Lỗi Logic GA)
- **Log cần tìm**: `RPGGA_ModifyHealth: Applied [Số] to Health. New Health: [Số]`
- **Nếu thấy dòng này**: Code C++ đã chạy và đã gọi hàm sửa máu. 
- **Nếu máu vẫn 100**: Có thể UI không cập nhật hoặc có một logic khác (như God Mode hoặc Cheat) đang ghi đè lại máu ngay lập tức.

---
*Mẹo: Hãy gõ lệnh `showdebug abilitysystem` và kiểm tra cột bên trái để xem GA có đang xuất hiện trong danh sách "Activatable Abilities" không.*

## 🔍 Phân tích Lỗ hổng "Nghẽn" (Clog) - Tại sao bấm phím có Log nhưng máu không đổi?

Dựa trên Log và mã nguồn, đây là các điểm nghi vấn gây nghẽn:

### 1. Sai lệch Luồng xử lý Attribute (Nghi phạm chính)
- **Vấn đề**: Class `URPGGA_ModifyHealth` hiện tại đang dùng `ApplyModToAttribute` tác động vào `Damage` và `Healing`.
- **Tại sao nghẽn**: Trong C++, logic chuyển từ `Damage` sang `Health` nằm ở hàm **`PostGameplayEffectExecute`**. Hàm này **CHỈ CHẠY** khi có một Gameplay Effect (GE) được thực thi. Việc dùng `ApplyModToAttribute` chỉ đơn thuần cộng thêm số vào biến `Damage` mà không kích hoạt sự kiện tính toán lại máu.
- **Giải pháp**: Tôi sẽ sửa lại GA để tác động trực tiếp vào attribute `Health` (để test) hoặc hướng dẫn dùng Gameplay Effect.

### 2. Sự khớp (Mapping) giữa Input Tag và Ability Set
- **Vấn đề**: Bạn đề cập đã thêm `GA_Test_damage` với tag `InputTag.SelfDamage`. Tuy nhiên log hiện: `InputTag.SelfHeal`.
- **Tại sao nghẽn**: Nếu bạn nhấn phím và log ra `SelfHeal`, nhưng trong `Ability Set` chỉ có mapping cho `SelfDamage`, thì `AbilitySystemComponent` sẽ không tìm thấy Ability nào để kích hoạt.
- **Kiểm tra**: Hãy đảm bảo Tag trong `Input Config` (gán cho phím) và Tag trong `Ability Set` (gán cho GA) phải **GIỐNG HỆT NHAU**.

### 3. Trạng thái Ability Activation (Policy)
- **Vấn đề**: Nếu GA chưa được set `Activation Policy` là `On Input Triggered`.
- **Dấu hiệu**: Log hiện `Input_AbilityInputTagPressed` nhưng không có log của riêng Ability (vì nó không Activate).

### 4. Vấn đề Replication (Dành cho Server/Client)
- **Vấn đề**: Máu chỉ trừ ở Server nhưng UI (Client) không cập nhật. 
- **Kiểm tra**: Dùng lệnh `showdebug abilitysystem` để xem con số thực tế trên Server.
---

## 🏗️ 8. Cấu trúc Liên kết (Architecture Chain)

Theo thông tin từ dự án, luồng liên kết các Asset như sau:
1. **Ability**: `GA_Test_Damage` (URPPGA_ModifyHealth).
2. **Ability Set**: `AbilitySet_Axe` (Chứa GA và map với Tag `InputTag.SelfDamage`).
3. **Equipment Definition**: `WID_Axe` (Kết nối tới `AbilitySet_Axe`).
4. **Pawn Data**: `HeroPawnData` (Chứa `WID_Axe`).

### 🧐 Nghi vấn mới: Lỗi khi cấp phát (Granting) Ability từ Trang bị
Nếu bạn thấy log Input nhưng máu không đổi, tỉ lệ cao là **Ability chưa được cấp phát (Give/Grant) vào nhân vật**.

- **Vấn đề**: Khi GA nằm trong Equipment (Trang bị), nó chỉ được cấp phát khi Trang bị đó được **Equip** vào nhân vật.
- **Kiểm tra**:
    - Nhân vật của bạn đã thực sự cầm chiếc Rìu (Axe) trên tay chưa?
    - Nếu chưa cầm, GA sẽ chưa tồn tại trong hệ thống, dẫn đến việc bấm phím không có tác dụng.
    - Dùng lệnh `showdebug abilitysystem` để kiểm tra danh sách Ability đang có.

---
---

## 🛠️ 9. Kiểm chứng Logic C++ (Code-Verified)

Tôi đã kiểm tra mã nguồn C++ và xác nhận luồng logic như sau:
1. **Granting**: `RPGWeaponInstance::Equip` gọi `AbilitySet->GiveToAbilitySystem`.
2. **Tagging**: Code C++ tự động thêm `InputTag` vào `DynamicSpecSourceTags` của Ability.
3. **Matching**: `RPGAbilitySystemComponent` tìm kiếm Ability dựa trên `DynamicSpecSourceTags`.

### 🚨 Điểm mấu chốt để tìm lỗi (Critical Checkpoint)

Bạn hãy tìm dòng log này trong Output Log:
> **`AbilityInputTagPressed: Matching tag [Tên Tag] with ability [Tên GA]`**

- **Nếu THẤY dòng này**: Nghĩa là Ability **ĐÃ** được cấp phát và hệ thống **ĐÃ** tìm thấy nó. Nếu máu vẫn không trừ -> Lỗi tại **Activation Policy** (phải là `On Input Triggered`) hoặc GA bị block.
- **Nếu KHÔNG THẤY dòng này (Dù có log `Input_AbilityInputTagPressed`)**: Nghĩa là Ability **CHƯA** được cấp phát thành công vào nhân vật.

**Các bước xử lý khi không thấy "Matching tag":**
1. **Kiểm tra Quyền (Authority)**: Ability chỉ được cấp phát ở Server. Hãy chắc chắn bạn đang test với `Net Mode: Play As Listen Server` hoặc `Dedicated Server`.
2. **Kiểm tra Asset WID_Axe**: 
   - Mở `WID_Axe` (Equipment Definition).
   - Kiểm tra mảng `Ability Sets To Grant` đã chứa `AbilitySet_Axe` chưa.
3. **Kiểm tra Asset AbilitySet_Axe**:
   - Đảm bảo dòng chứa `GA_Test_Damage` có cột **Input Tag** được điền chính xác là `InputTag.SelfDamage`.
4. **Kiểm tra việc Equip**: 
   - Rìu có thực sự xuất hiện trên tay nhân vật không? Nếu không thấy Rìu, code `Equip()` chưa chạy -> GA chưa được cấp.

---
