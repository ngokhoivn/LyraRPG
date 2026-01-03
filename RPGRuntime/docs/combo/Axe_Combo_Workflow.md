# 🪓 Axe Combo Workflow (Editor Guide)

Tài liệu này hướng dẫn chi tiết các bước thiết lập trong Unreal Editor để kích hoạt hệ thống Axe Combo sau khi code C++ đã sẵn sàng.

---

## Bước 1: Thiết lập Animation Montage (`AM_Axe_Combo`)

Đây là bước quan trọng nhất vì nó điều khiển nhịp độ của combo.

1.  **Tạo Montage**: Chuột phải vào Animation đòn đánh (ví dụ: `Axe_Attack_1`) -> **Create > Create AnimMontage**. Đặt tên là `AM_Axe_Combo`.
2.  **Chia Section**: Trong cửa sổ Montage, mở tab **Montage Sections**:
    - Thêm Section: `Attack1`, `Attack2`, `Attack3`. 
    - **Quan trọng**: Xóa các đường nối tự động giữa các Section (Click chuột phải vào Section, chọn **Remove Link**). Chúng ta muốn C++ điều khiển việc chuyển đòn, không muốn nó tự động phát hết chuỗi.
3.  **Đặt Notify (Transitions)**: Tại cửa sổ Timeline:
    - Tìm giai đoạn "Action Window" (khi đòn đánh đã vung xong và nhân vật đang hồi tư thế).
    - Chuột phải -> **Add Notify > RPGAnimNotify_SendGameplayEvent**.
    - **Tag**: `Event.Combo.Transition.Attack2` (Báo hiệu nếu nhấn phím sẽ nhảy sang Attack2).
    - Tương tự cho Attack2 nhảy sang Attack3.
4.  **Đặt Notify (Damage)**:
    - Tại thời điểm vũ khí chạm mục tiêu: **Add Notify > RPGAnimNotify_SendGameplayEvent**.
    - **Tag**: `Event.Combo.Damage`.

---

## Bước 2: Tạo Gameplay Effect tiêu tốn Stamina (`GE_Axe_Combo_Cost`)

1.  Tạo GE mới tên là `GE_Axe_Combo_Cost`.
2.  **Duration Policy**: `Instant`.
3.  **Modifiers**:
    - **Attribute**: `RPGAttributeSet.Stamina`.
    - **Op**: `Add`.
    - **Magnitude**: `-15.0` (Ví dụ: mỗi đòn tốn 15 thể lực).

---

## Bước 3: Tạo Gameplay Ability Axe Combo (`GA_Axe_Combo`)

1.  Chuột phải -> **Gameplay Ability Blueprint**. 
2.  Chọn lớp cha là **`RPGGA_Combo`** (Lớp C++ tôi sẽ tạo).
3.  Đặt tên là `GA_Axe_Combo`.
4.  Trong **Class Defaults**:
    - **Combo Montage**: Chọn `AM_Axe_Combo`.
    - **Cost Gameplay Effect**: Chọn `GE_Axe_Combo_Cost`.
    - **Ability Tags**: Thêm Tag `Ability.Action.Attack`.
    - **Activation Policy**: `OnInputTriggered`.

---

## Bước 4: Đăng ký vào Ability Set

1.  Mở Asset **`AbilitySet_RPGHero`**.
2.  Thêm phần tử mới vào **Granted Gameplay Abilities**:
    - **Ability**: `GA_Axe_Combo`.
    - **Input Tag**: `InputTag.Weapon.Attack` (Thường là Chuột trái).

---

## ✅ Checklist Kiểm tra
- [ ] Montage đã bỏ liên kết (Remove Link) giữa các Section chưa?
- [ ] Notify Transition đã mang Tag của đòn *kế tiếp* chưa?
- [ ] GE Cost đã có giá trị âm chưa?
- [ ] Bấm chuột trái 3 lần liên tục có ra đủ 3 đòn không?
- [ ] Nếu bấm 1 lần và đứng im, nhân vật có tự động kết thúc đòn và về Idle không?
