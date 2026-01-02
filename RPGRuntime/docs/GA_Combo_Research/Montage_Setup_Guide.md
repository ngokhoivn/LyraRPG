# Animation Montage Setup Guide: Pure Combo Integration

Tài liệu này hướng dẫn Animator cách thiết lập Montage để làm "nguồn phát tín hiệu" cho hệ thống Pure Tag-State Combo.

---

## 1. Cấu trúc Montage Sections

Để GA_Combo có thể điều hướng chính xác, các Section phải được đặt tên rõ ràng và liên tục.

- **Quy ước**: `Attack1`, `Attack2`, `Attack3`, ...
- **Lưu ý**: Đảm bảo các đòn đánh được nối tiếp nhau thông qua bảng **Montage Sections** (không để tự động phát hết chuỗi nếu không có input).

---

## 2. Thiết lập Notifies (The Signal Pipeline)

Chúng ta sử dụng một loại Notify duy nhất `AN_SendComboEvent` nhưng với các **Gameplay Tags** khác nhau để điều khiển logic.

### A. Transition Notify (Tín hiệu chuyển đòn)
- **Vị trí**: Đặt ở cuối "Action Window". Đây là thời điểm mà hoạt ảnh vung kiếm đã kết thúc và nhân vật đang ở trạng thái chuẩn bị (Follow-through).
- **Cấu hình Tag**: `Event.Combo.Transition.Attack2` (Lưu ý: Tag này báo hiệu đòn *tiếp theo*).
- **Mục đích**: Báo cho GA biết: "Nếu Player đã nhấn phím, hãy nhảy sang Section Attack2 ngay bây giờ".

### B. Damage Notify (Tín hiệu gây sát thương)
- **Vị trí**: Đặt tại "Impact Point". Thời điểm mà vũ khí thực sự chạm vào mục tiêu.
- **Cấu hình Tag**: `Event.Combo.Damage`.
- **Mục đích**: Kích hoạt việc kiểm tra va chạm (Sweep Trace) và áp dụng Gameplay Effect.

---

## 3. Visualizing the Action Window

Một đòn đánh hiệu quả nên được chia làm 3 giai đoạn trên Timeline:

1.  **Anticipation (Bắt đầu đòn)**: Không nhận input, nhân vật đang lấy đà.
2.  **Impact (Gây sát thương)**: Đặt `Event.Combo.Damage` tại đây.
3.  **Action Window (Cửa sổ chuyển tiếp)**: Đặt `Event.Combo.Transition` tại đây. Đây là lúc mượt mà nhất để chuyển sang đòn tiếp theo.

---

## 4. Checklist cho Animator

- [ ] Các Section đánh đúng tên quy ước (`AttackX`).
- [ ] Notify `Transition` mang Tag đúng của đòn *kế tiếp*.
- [ ] Notify `Damage` được đặt chính xác tại thời điểm va chạm.
- [ ] Cấu hình **Blend Out Time** thấp (khoảng 0.1s - 0.2s) để đòn đánh cảm giác nhanh và nhạy.

---

## 5. Tại sao Animator là người cầm lái?
Với hệ thống này, Coder không can thiệp vào "cảm giác" của đòn đánh. Nếu bạn muốn đòn combo nhanh hơn, bạn chỉ cần di chuyển Notify `Transition` về phía trước trên Timeline. Hệ thống logic sẽ tự động thích nghi mà không cần sửa code.

---
**Triết lý**: "Animator defines the rhythm, Logic ensures the state."
