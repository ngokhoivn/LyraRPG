Bài 16: Quy trình Thiết lập Kiến trúc UI (Lyra-Style Standalone)

## **❓ Vì sao Lyra không gắn UI vào HUD truyền thống?**

Trong Lyra, UI được thiết kế theo triết lý **"UI không thuộc về Gameplay"**:

- ✅ **UI không thuộc về Pawn** - Khi Pawn chết/đổi, UI vẫn tồn tại
- ✅ **UI không thuộc về HUD** - HUD chỉ là một Layer trong cấu trúc
- ✅ **UI được quản lý theo "Experience"** - Có thể thay đổi toàn bộ UI khi chuyển chế độ chơi
- ✅ **UI có thể Hot-load** - Thêm/xóa UI mà không cần restart game

**Điều này cho phép:**

- Thay đổi toàn bộ UI theo Game Mode / Experience
- Hot-load UI qua Game Feature Plugin
- Một Player có thể có Layout khác Player khác (split-screen)
- UI vận hành độc lập với gameplay state

---

## **🔄 Kiến trúc UI phi tập trung**

Khác với cách làm truy thống (gắn widget vào HUD), chúng ta sử dụng hệ thống **"Đẩy" (Push) widget** vào các **tầng (Layers)** thông qua một trình quản lý tập trung.

---

## **🧱 Phân biệt Layer và Widget trong Lyra**

| **Layer** | **Vai trò** | **Ví dụ** |
| --- | --- | --- |
| **Game** | `UI.Layer.Game` - Dành cho HUD (Máu, Mana, MiniMap) | `W_RPG_HealthBar` |
| **GameMenu** | `UI.Layer.GameMenu` - Menu tương tác khi chơi (Inventory, Skills) | `W_RPG_Inventory` |
| **Menu** | `UI.Layer.Menu` - Menu lớn hệ thống (Settings, Main Menu) | `W_RPG_Settings` |
| **Modal** | `UI.Layer.Modal` - Lớp cao nhất (Thông báo, Pop-up xác nhận) | `W_RPG_Confirmation` |
| **Widget** | Thứ thực sự được render trên màn hình | `W_RPG_HUDLayout`, `W_RPG_HealthBar` |
| **Layout** | Widget chứa các widget con | `W_RPG_HUDLayout` |
| **Component** | Widget thành phần nhỏ | `W_RPG_HealthBar`, `W_RPG_ManaBar` |

**Quy tắc quan trọng:**

- Layer chỉ là **vị trí logic** trong stack
- Widget được **push vào Layer**
- Một Layer có thể chứa nhiều Widget (stack)
- Layout là Widget đặc biệt **chỉ chứa** widget con

---

## **⚙️ Các thành phần cốt lõi (C++)**

| **Class** | **Vai trò** | **Quan trọng cần hiểu** |
| --- | --- | --- |
| **RPGPrimaryGameLayout** | Widget gốc chứa các Layer (Game, Menu, Overlay). | Quản lý các Stack (Container), cho phép Push/Pop widget. |
| **RPGUIPolicy** | Bộ não điều phối, quyết định Layout nào được dùng cho Player nào. | Tạo và quản lý Primary layout cho mỗi player. |
| **RPGUIManagerSubsystem** | Điểm truy cập toàn cục để thay đổi UI từ bất kỳ đâu. | Quản lý vòng đời của Policy và cung cấp truy cập static. |
| **RPGActivatableWidget** | Lớp cha cho Widget có thể đóng/mở và quản lý Input. | Chịu trách nhiệm Input Focus, quản lý Input Mode (Game/Menu). |
| **RPGUserWidget** | Lớp cơ bản cho tất cả widget UI. | Có thể lắng nghe Gameplay Message, cập nhật linh hoạt. |

### **🎮 RPGActivatableWidget - Vai trò thực sự**

cpp

```
// Chịu trách nhiệm:
1. Nhận / nhả Input Focus tự động
2. Quyết định Input Mode (Game / UI / Both)
3. Tự động push/pop khỏi Layer Stack
4. Quản lý trạng thái kích hoạt (Activated/Deactivated)

// KHÔNG BAO GIỜ:
1. Chứa gameplay logic
2. Truy cập trực tiếp vào Pawn/PlayerController
3. Bind Attribute trực tiếp
```

---

## **🔔 Luồng dữ liệu 1 chiều trong Lyra**

**Nguyên tắc vàng:** Widget chỉ là **subscriber**, không phải **controller**

---

## **🎨 Quy trình triển khai 4 Bước (Kiến trúc chuẩn)**

### **Bước 1: Tạo Root Layout (Bộ khung chứa tầng)**

1. **Tạo Widget**: Tạo một Widget Blueprint mới tên là **`W_RPG_OverallUILayout`**.
2. **Kế thừa**: Vào **Class Settings**, đổi **Parent Class** thành **`RPGPrimaryGameLayout`**.
3. **Thiết kế (Designer)**:
    - Thêm một **Canvas Panel**.
    - Kéo các widget **Common Activatable Widget Stack** vào Canvas.
    - **QUAN TRỌNG**: Đặt tên cho các Stack chính xác như sau để C++ tự động nhận diện:
        - `GameLayer_Stack`
        - `GameMenu_Stack`
        - `Menu_Stack`
        - `Modal_Stack`
4. **Đăng ký Layer**:
    - Nhờ logic trong RPGPrimaryGameLayout (C++), các Layer sẽ được tự động đăng ký khi Widget được khởi tạo. Bạn không cần viết thêm logic RegisterLayer trong Blueprint nữa như trong dự án Lyra, nếu muốn thêm các Layer tùy chỉnh khác ta sẽ thêm trong class C++.
    
### **Bước 2: Cấu hình UI Policy**

1. **Tạo Blueprint**: Tạo một Blueprint Class kế thừa từ **`RPGUIPolicy`**, đặt tên là **`B_RPG_UIPolicy`**.
2. **Thiết lập Layout**: Trong **Class Defaults**, tại mục **Layout Class**, chọn **`W_RPG_OverallUILayout`**.
3. **Kích hoạt System (Tự động)**: Để hệ thống tự động tạo UI khi game bắt đầu, bạn cần khai báo class Policy này trong file cấu hình của project.
    - Mở file **`Config/DefaultGame.ini`**.
    - Thêm (hoặc sửa) đoạn code sau:
    ```ini
    [/Script/RPGRuntime.RPGUIManagerSubsystem]
    DefaultPolicyClass=/RPG/UI/B_RPG_UIPolicy.B_RPG_UIPolicy_C
    ```
    - **Lưu ý**: Đảm bảo đường dẫn Asset trỏ đúng vào nơi bạn lưu `B_RPG_UIPolicy`.

### **Bước 3: Tạo Layout Nội dung (Ví dụ: HUD)**

1. **Tạo Widget**: Đặt tên là **`W_RPG_HUDLayout`**.
2. **Kế thừa**: Đổi **Parent Class** thành **`RPGHUDLayout`**.
3. **Thiết kế**: Kéo các Component nhỏ như `W_RPG_HealthBar` vào đây. Đây là nơi bạn sắp xếp vị trí các thanh máu, bản đồ trên màn hình.

### **Bước 4: Đẩy (Push) UI vào màn hình**

Để hiển thị HUD, chúng ta không dùng `Add to Viewport`, mà "đẩy" nó vào Layer:

**Cách 1: Qua Gameplay Feature Action (Tự động)**
1. Mở **Experience Definition** (ví dụ: `B_RPG_Experience`) hoặc **Action Set** (ví dụ: `LAS_RPG_SharedInput`).
2. Trong mục **Actions**, thêm một Action **`Add Widgets`**.
3. Cấu hình phần **Layout**:
    - **Layout Class**: `W_RPG_HUDLayout`
    - **Layer ID**: `UI.Layer.Game` (HUD sẽ nằm ở tầng Game)

**Cách 2: Qua Code/Blueprint (Thủ công)**
Sử dụng function **`Push Widget to Layer Stack`** từ `RPGPrimaryGameLayout`.

---

## **🔍 Checklist Kiểm tra HUD không hiện**

Nếu bạn nhấn Play mà không thấy thanh máu, hãy kiểm tra:

1. **Experience Action**: Action Add Widgets đã có đúng Class và Slot Name chưa?
2. **C++ Logic**: Class `RPGGameFeatureAction_AddWidgets` đã được implement logic "Push" chưa? (Nếu chưa, HUD sẽ không bao giờ được tạo)
3. **UI Policy**: Bạn có đang dùng đúng Policy (Ví dụ: `B_RPG_UIPolicy`) trong cấu hình Game không?
4. **Visibility**: Widget con (`W_RPG_HealthBar`) có đang để Visible trong Layout không?
5. **Message System**: HealthChanged message có được gửi từ gameplay code không?

> [!TIP]
> 
> 
> **Mẹo soi Widget:**
> 
> Trong lúc đang Play, gõ lệnh console `WidgetReflector` để soi xem Widget có thực sự tồn tại trong cây thư mục UI không nhưng đang bị ẩn hoặc kích thước 0,0.
> 

---

## **❌ Sai lầm thường gặp khi học Lyra UI**

### **🚫 CÁC ĐIỀU KHÔNG NÊN LÀM**

1. **Gắn HealthBar trực tiếp vào Pawn** - Phá vỡ tính độc lập của UI
2. **Bind ProgressBar % trong Designer** - Tạo coupling trực tiếp
3. **Tick Blueprint để lerp giá trị** - Tốn performance, khó debug
4. **Truy cập trực tiếp ASC từ Widget** - Vi phạm nguyên tắc separation of concerns
5. **Để Widget quyết định gameplay state** - UI không nên ảnh hưởng gameplay

### **✅ CÁCH ĐÚNG THEO LYRA**

1. **Dùng Gameplay Message Subsystem** - Decoupling hoàn toàn
2. **Widget chỉ subscribe message** - Không biết message từ đâu
3. **Logic update nằm trong callback** - Không tick, không poll
4. **UI Policy quyết định layout** - Không hardcode
5. **Experience điều khiển UI hiển thị** - Có thể thay đổi dynamic

---

## **🧠 Tư duy cần nắm trước khi chuyển sang C++**

### **1. UI là "View" trong MVP/MVVM**

- Chỉ hiển thị, không xử lý logic
- Dữ liệu đến từ Message Bus

### **2. Mọi thứ đều có thể "Push/Pop"**

- Không cần `SetVisibility`
- Dùng `UIManager->PushWidgetToLayer()`

### **3. Input được quản lý tập trung**

- `ActivatableWidget` tự động xử lý focus
- Không cần manual `SetInputMode`

### **4. UI sống lâu hơn Pawn**

- Không bind vào Pawn events
- Dùng PlayerState/PlayerController events

### **5. Hot-reload là first-class citizen**

- UI có thể thêm/xóa khi game đang chạy
- Game Feature Plugin là bạn