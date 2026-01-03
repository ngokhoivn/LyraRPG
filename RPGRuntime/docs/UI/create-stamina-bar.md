---

## 🔍 Kiến trúc Kết nối (C++ & Editor)
Trước khi bắt đầu, bạn cần hiểu cách Stamina Bar "tìm thấy" nhân vật:
1. **C++ Layer**: Cả Health và Stamina đều được quản lý bởi **`URPGHealthComponent`** trên nhân vật (`ARPGCharacter`). Widget `RPGStaminaBar` trong C++ đã được lập trình để tự động tìm linh kiện này khi bạn cầm nhân vật.
2. **UI Layer**: Health Bar hiện đang nằm trong Widget **`W_RPG_HUDLayout`**. Đây cũng chính là nơi bạn sẽ kéo Stamina Bar vào.
3. **Trigger**: HUD được hiển thị thông qua **Experience Action** (`Add Widgets`) trong Experience Definition của bạn (ví dụ: `B_RPG_Experience`).

---

## Bước 1: Tạo Widget Blueprint (Cách nhanh nhất)
1. **Copy W_RPG_HealthBar** (Tìm trong Content Browser).
2. Đặt tên là `W_StaminaBar`.
3. Mở `W_StaminaBar`, vào **Class Settings**, đổi **Parent Class** thành **`RPGStaminaBar`**.
4. **Đổi tên Component**:
    - Tìm `HealthNumber` (Common Numeric Text Block) và đổi tên thành **`StaminaNumber`**.
    - Đảm bảo `BarFill` (Image) vẫn giữ nguyên tên.
5. **Dọn dẹp**: Xóa `BarGlow`, `BarBorder`, `EliminatedText` và các Widget thừa khác.
6. **Gán Material**: Chọn `BarFill`, trong mục **Appearance > Brush > Image**, chọn Material Instance **`MI_UI_StaminaBar_Fill`**.
7. **Dọn dẹp Animation**:
    - Mở tab **Animations**.
    - Bạn sẽ thấy các cảnh báo (Warnings) đỏ cho `OnDamaged`, `OnHealed`, v.v.
    - **Tốt nhất**: Xóa các Animation này đi vì Stamina không cần hiệu ứng nháy như Health.

## Bước 2: Thêm vào HUD Layout
1. Tìm và mở Widget **`W_RPG_HUDLayout`**.
2. Kéo `W_StaminaBar` từ Content Browser vào Canvas Panel (hoặc Vertical Box chứa Health Bar).
3. Căn chỉnh vị trí ngay dưới thanh Health Bar.

## Bước 3: Thiết lập Hệ thống Stamina Tự động (Không cần nhấn phím)

Tôi đã lập trình C++ để nhân vật tự động bật/tắt các Tag sau:
- **`Status.Movement.Moving`**: Tự bật khi bạn di chuyển.
- **`Status.Movement.Idle`**: Tự bật khi bạn đứng yên.

Dựa vào đó, chúng ta sẽ tạo 2 Gameplay Effect (GE) chạy ngầm:

### 1. GE_Stamina_Regeneration (Hồi thể lực khi đứng yên)
1. Tạo một GE mới tên là `GE_Stamina_Regen`.
2. **Duration Policy**: `Infinite`.
3. **Period**: `0.1` (Hồi mỗi 0.1 giây để thanh chạy mượt).
4. **Modifiers**: `RPGAttributeSet.Stamina`, Op: `Add`, Magnitude: `1.0` (Tùy chỉnh tốc độ hồi).
5. **GE Requirement (Ongoing Tag Requirements)**:
    - **Target Does NOT Have Tag**: `Status.Movement.Moving`.
    - *(Nghĩa là: Chỉ hồi khi KHÔNG di chuyển)*.

### 2. GE_Stamina_Decay (Tiêu hao thể lực khi di chuyển)
1. Tạo một GE mới tên là `GE_Stamina_Decay`.
2. **Duration Policy**: `Infinite`.
3. **Period**: `0.1`.
4. **Modifiers**: `RPGAttributeSet.Stamina`, Op: `Add`, Magnitude: `-2.0` (Tùy chỉnh tốc độ tốn).
5. **GE Requirement (Ongoing Tag Requirements)**:
    - **Target HAS Tag**: `Status.Movement.Moving`.
    - *(Nghĩa là: Chỉ trừ khi ĐANG di chuyển)*.

### 3. Kích hoạt hệ thống (Sử dụng GA Thụ động)
Để 2 GE này luôn chạy tự động dựa trên trạng thái nhân vật, tôi đã tạo lớp C++ **`RPGGA_PassiveStamina`**.

1. Tạo một **Gameplay Ability Blueprint** mới, chọn lớp cha là **`RPGGA_PassiveStamina`**. Đặt tên là `GA_PassiveStamina`.
2. Trong tab **Class Defaults** của `GA_PassiveStamina`:
    - **Activation Policy**: Chọn `On Spawn` (Để nó tự chạy khi bạn sinh ra).
    - **Passive Effects**: Thêm 2 phần tử vào mảng này:
        - `GE_Stamina_Regen`
        - `GE_Stamina_Decay`
3. **Đăng ký vào Ability Set**:
    - Mở Asset **`AbilitySet_RPGHero`**.
    - Thêm `GA_PassiveStamina` vào mảng **Granted Gameplay Abilities**.
    - (Không cần gán phím tắt vì đây là GA thụ động).

## Bước 4: Chạy thử trong Game
1. Nhấn **Play**.
2. **Đứng yên**: Thanh Stamina sẽ tự động hồi.
3. **Di chuyển**: Thanh Stamina sẽ tự động tụt xuống mượt mà.

---

> [!TIP]
Nếu thanh Stamina không hiện số, hãy kiểm tra xem bạn đã gán StaminaNumber là biến (Is Variable = True) trong Widget Blueprint chưa.
>