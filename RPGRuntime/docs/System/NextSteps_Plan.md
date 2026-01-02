# Kế hoạch Hành động Tiếp theo (RPG Next Steps Plan)

Dựa trên việc kiểm tra mã nguồn hiện tại, các hệ thống C++ cốt lõi (Inventory, Quickbar, Equipment, Animation Tags) đã có nền tảng vững chắc. Tuy nhiên, để hoàn thiện một vòng lặp Gameplay RPG đầy đủ, chúng ta cần thực thực hiện các bước sau:

---

## 1. Tích hợp Gameplay Ability System (GAS) - "Trái tim" của Combat
Hiện tại chúng ta mới chỉ cầm được vũ khí, nhưng chưa có logic để tấn công hoặc quản lý máu/mạnh.
- [ ] **Tạo RPGAttributeSet**: Định nghĩa các chỉ số như `Health`, `Mana`, `Strength`, `Stamina`.
- [ ] **Tạo GameplayAbilities cơ bản**:
    - `GA_Axe_Attack`: Logic tấn công cho Rìu (trừ Stamina, gây sát thương).
    - `GA_Hero_Dodge`: Kỹ năng né tránh.
- [ ] **Liên kết Ability vào Weapon**: Khi trang bị Rìu, tự động cấp (Grant) `GA_Axe_Attack` cho nhân vật.

## 2. Hệ thống Giao diện (UI) - "Cửa sổ" tương tác
Tất cả logic Inventory hiện tại đang chạy ngầm trong C++.
- [ ] **Inventory Widget**: Hiển thị danh sách vật phẩm trong túi đồ.
- [ ] **Quickbar Widget**: Hiển thị 3-5 slot phím tắt, làm nổi bật slot đang chọn.
- [ ] **HUD Indicators**: Thanh máu (Health Bar) và thanh năng lượng (Mana/Stamina).

## 3. Hệ thống Tương tác (Interaction) - "Nguồn" vật phẩm
- [ ] **RPGInteractable Interface**: Interface C++ để nhân vật biết vật thể nào có thể tương tác.
- [ ] **B_Item_Pickup**: Blueprint Actor trong thế giới (ví dụ một chiếc Rìu nằm trên đất) mà người chơi có thể nhấn phím `E` để nhặt vào Inventory.

## 4. Hoàn thiện Blueprint & Experience (Action Sets)
Đưa các component C++ vào vận hành thực tế thông qua hệ thống Lyra.
- [ ] **LAS_RPG_SharedComponents**: Tạo Action Set để tự động thêm `InventoryManager`, `QuickbarComponent` vào Project.
- [ ] **Cấu hình Hero Data**: Gán các vật phẩm khởi đầu (Initial Inventory) cho nhân vật trong Blueprint.

---

## Lộ trình Đề xuất (Roadmap)

### Giai đoạn A: "Cơ bắp & Chỉ số" (GAS)
Tập trung vào AttributeSet và Ability tấn công cơ bản để nhân vật thực sự có thể "chiến đấu".

### Giai đoạn B: "Thế giới & Nhặt đồ" (Interaction)
Xây dựng logic nhặt vũ khí từ đất vào túi đồ thay vì gán sẵn bằng code.

### Giai đoạn C: "Ngắm nhìn & Cảm nhận" (UI & VFX)
Hoàn thiện giao diện và các hiệu ứng hình ảnh (VFX) khi chém Rìu trúng mục tiêu.

---

**Bạn có muốn bắt đầu với Giai đoạn A (Xây dựng hệ thống chỉ số và kỹ năng tấn công) ngay không?**
