# Hướng dẫn Thiết lập Material Graph cho Health Bar (Time-Based Lerp)

Tài liệu này hướng dẫn cách xây dựng logic Material Graph để tận dụng cơ chế nội suy theo thời gian từ C++, đảm bảo thanh máu trượt mượt mà và đồng bộ với con số.

---

## 💎 1. Các tham số Material (Scalar Parameters)

Bạn cần tạo các tham số sau trong Material của mình (Lưu ý: Tên phải chính xác tuyệt đối như C++):

| Tên Tham Số | Giá trị mặc định | Ý nghĩa |
| :--- | :--- | :--- |
| `Health_Start` | 1.0 | Phần trăm máu tại thời điểm bắt đầu hiệu ứng. |
| `Health_Target` | 1.0 | Phần trăm máu đích (mới). |
| `Health_AnimStartTime` | 0.0 | Thời điểm bắt đầu chạy hiệu ứng (Timestamp từ Engine). |
| `Health_AnimDuration` | 0.35 | Tổng thời gian trượt (giây). |

---

## 🛠️ 2. Xây dựng Logic Graph (Node-by-Node)

Bạn hãy kết nối các node theo trình tự sau để tính toán giá trị Máu hiển thị:

### Bước 1: Tính Alpha dựa trên thời gian thực
1.  **Time Node**: Lấy giá trị thời gian thực của thế giới.
2.  **Subtract**: `Time` - `Health_AnimStartTime` (Tính xem đã trôi qua bao nhiêu giây).
3.  **Divide**: Kết quả trên chia cho `Health_AnimDuration` (Chuyển sang dải 0-1).
4.  **Saturate**: Khóa giá trị kết quả trong khoảng [0, 1] để tránh vượt quá giới hạn.

### Bước 2: Làm mượt Alpha (Smooth Transitions)
1.  Dẫn kết quả từ node **Saturate** vào node **SmoothStep**.
2.  Thiết lập: `Min = 0.0`, `Max = 1.0`.
3.  *Kết quả*: Alpha sẽ trôi mượt theo kiểu Ease-in/Ease-out (chuẩn Lyra).

### Bước 3: Tính toán giá trị Máu cuối cùng (Interpolated Value)
1.  **Lerp**: 
    - Input **A**: `Health_Start`
    - Input **B**: `Health_Target`
    - Input **Alpha**: Kết quả từ node **SmoothStep**.
2.  *Kết quả*: Đây chính là `% máu hiển thị` chuẩn xác tại khung hình hiện tại.

---

## 🎨 3. Áp dụng vào Visuals

Dẫn kết quả từ bước 3 vào logic hiển thị của bạn:
-   **Horizontal Fill**: So sánh: `CustomUV.x < Results`.
-   **Radial Image**: Dẫn vào tham số `Percent` của node `RadialGradientExponential`.
-   **Masking**: Multiply với Texture Mask để hiện/ẩn phần máu.

---

## ✅ 4. Checklist Debug

Nếu thanh máu vẫn không trượt mượt, hãy kiểm tra:
1.  [ ] **Tên Parameter**: Đã trùng khớp với C++ chưa? (C++ dùng: `Health_Start`, `Health_Target`, `Health_AnimStartTime`, `Health_AnimDuration`).
2.  [ ] **Node Time**: Phải dùng node `Time` chuẩn của Material Expression.
3.  [ ] **Node Saturate**: Rất quan trọng để tránh giá trị máu bị "vọt" qua 100% hoặc âm.
4.  [ ] **C++ Sync**: Trong C++, tham số `AnimDuration` cho Damage là `0.35`. Trong Widget Editor, hãy chỉnh **Maximum Interpolation Duration** của `CommonNumericTextBlock` cũng là `0.35`.

---
*Tài liệu này thuộc hệ thống RPG Plugin - Logic UI Hiệu suất cao.*
