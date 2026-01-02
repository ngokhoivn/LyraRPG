# Hướng dẫn sửa lỗi Sát thương cộng dồn (Damage Stacking)

### 1. Phân tích nguyên nhân
Trong hệ thống GAS (Gameplay Ability System), các thuộc tính như **Damage** (Sát thương) và **Healing** (Hồi phục) được gọi là **Meta-Attributes**. 

- **Đặc điểm**: Meta-Attributes là các thuộc tính tạm thời, được dùng làm "trạm trung chuyển" để tính toán trước khi trừ vào Health (Máu).
- **Vấn đề của bạn**: Bạn đang để Gameplay Effect (GE) ở chế độ **Duration** (Thời hạn).
- **Tại sao nó cộng dồn?**: 
    - Khi dùng `Duration`, GAS sẽ thêm một **Modifier** (hệ số điều chỉnh) vào thuộc tính Damage.
    - Lần nhấn thứ nhất: Damage = 0 (Base) + 20 (Modifier 1) = 20.
    - Lần nhấn thứ hai: Damage = 0 (Base) + 20 (Modifier 1) + 20 (Modifier 2) = 40.
    - Hàm `SetDamage(0.0f)` trong C++ chỉ xóa giá trị Base, không thể xóa được các Modifier đang còn hạn (Duration).

### 2. Cách khắc phục (Fix)
Bạn không cần sửa code C++, mà cần sửa cấu hình trong **Data Asset của Gameplay Effect**:

1. Mở Gameplay Effect mà bạn dùng để gây sát thương (thường là cái được gán vào `EffectToApply` trong GA).
2. Tìm mục **Duration Policy**.
3. Đổi từ `Duration` (hoặc `Infinite`) sang **`Instant`**.

### 3. Tại sao nên dùng Instant?
- Với `Instant`, giá trị sát thương sẽ được áp dụng ngay lập tức, tính toán vào Máu và biến mất ngay sau đó. 
- Nó không để lại "Modifier" nào trên nhân vật, giúp lần gây sát thương sau luôn bắt đầu từ số 0.

### 4. Kiểm tra lại C++ (Safeguard)
Trong [RPGAttributeSet.cpp](file:///f:/UnrealProject/LyraRPG/Lyra/Plugins/GameFeatures/RPG/Source/RPGRuntime/Private/AbilitySystem/Attributes/RPGAttributeSet.cpp), logic xử lý đã chuẩn:
```cpp
if (Data.EvaluatedData.Attribute == GetDamageAttribute())
{
    const float LocalDamageDone = GetDamage();
    SetDamage(0.0f); // Reset base value
    // ... trừ vào Health ...
}
```
Lỗi stack hoàn toàn là do cấu hình GE là `Duration`. Hãy chuyển nó về `Instant` và mọi thứ sẽ hoạt động chính xác (20, 20, 20...).
