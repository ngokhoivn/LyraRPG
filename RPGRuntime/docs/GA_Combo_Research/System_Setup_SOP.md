# Pure System Setup SOP (Standard Operating Procedure)

Hướng dẫn này giúp kĩ sư thiết lập toàn bộ hệ thống Combo trong 10 phút, tập trung hoàn toàn vào các bước thực thi tinh gọn.

---

## Bước 1: Khai báo "Ngôn ngữ chung" (Gameplay Tags)
Chỉ cần 3 nhánh Tag chính (theo chuẩn Lyra):
1.  `State.Action.Combo`: Trạng thái tổng quát.
2.  `Event.Combo.Transition`: Tín hiệu chuyển đòn.
3.  `Event.Combo.Damage`: Tín hiệu gây sát thương.

---

## Bước 2: Cấu hình Animation Montage (The Trigger)
Đây là nơi bạn "lái" logic.
1.  **Sections**: Đặt tên Section đơn giản: `Attack1`, `Attack2`, ...
2.  **Notifies**:
    - Sử dụng `AN_SendComboEvent` có sẵn thuộc tính `FGameplayTag EventTag` (để tránh hardcode trong code).
    - Đặt `Transition` tag ở cuối "Action Window" (khi đòn đánh xong nhưng vẫn cho phép bấm tiếp).
    - Đặt `Damage` tag ở đỉnh của cú vung kiếm.

---

## Bước 3: Thiết lập Blueprint (The Integration)
Tạo Blueprint kế thừa từ `UGA_Combo` (Pure Class) và:
1.  Kéo Montage vào biến `ComboMontage`.
2.  Cấu hình `DamageMap`:
    - `State.Action.Combo.Light1` -> `GE_LightDamage`
    - `State.Action.Combo.Light2` -> `GE_MediumDamage`
    - ...

---

## Bước 4: Kiểm tra chu trình (The Loop)
1.  **Nhấn phím**: Ability bắt đầu, đòn 1 phát.
2.  **Nhấn phím lần 2 (Buffering)**: Logic ghi nhận "Player muốn đánh tiếp".
3.  **Anim Notify báo (Transition)**: Logic kiểm tra Buffer -> Có Buffer -> Nhảy Section `Attack2` -> Cập nhật State.
4.  **Kết thúc**: Nếu không có phím bấm tiếp theo, Ability tự động dọn dẹp Tag và kết thúc an toàn.

---

## Quy tắc "Vàng" để giữ hệ thống sạch:
- **Nguyên tắc 1**: Tuyệt đối không viết logic gây sát thương theo thời gian (Tick) trong C++. Hãy dùng Gameplay Effect.
- **Nguyên tắc 2**: Animation Montage chỉ được chứa Notifies, không được chứa logic chuyển đổi trực tiếp.
- **Nguyên tắc 3**: Mọi sự thay đổi về nhịp độ (Timing) Combo chỉ được chỉnh sửa trong Montage, không được sửa trong Code.
- **Nguyên tắc 4**: **GA_Combo không bao giờ tự quyết định đòn kế tiếp.** Mọi chuyển đổi phải đến từ Event do Animation phát ra. Điều này ngăn chặn việc coder vô tình viết logic rẽ nhánh "cứng" trong Ability.

---
**Kết quả**: Bạn có một hệ thống Action mượt mà, dễ bảo trì và cực kỳ dễ hiểu cho bất kỳ thành viên mới nào tham gia dự án. 
