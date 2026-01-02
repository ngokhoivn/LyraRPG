# Hướng dẫn Xử lý lỗi Hiển thị UI & Quản lý Material (MID)

Tài liệu này ghi lại quá trình xử lý lỗi không tìm thấy thanh máu (Health Bar) khi Play và giải thích logic Core UI liên quan đến Material Instance Dynamic.

---

## 🛑 1. Checklist Xử lý lỗi Visibility (Thanh máu không hiện)

Khi nhấn Play mà không thấy UI, hãy kiểm tra theo thứ tự ưu tiên sau:

### A. Lỗi Asset & References (Thể xác)
- **Vấn đề**: Copy Widget từ Lyra nhưng không copy các Material Instance (MI).
- **Dấu hiệu**: Widget tồn tại (soi bằng Widget Reflector thấy) nhưng trong suốt hoặc hiện ô vuông lỗi.
- **Khắc phục**: 
    - Đảm bảo các MI nằm đúng thư mục `Content/UI/HUD/Art`.
    - Mở Widget, kiểm tra các biến Material, nếu rỗng thì chọn lại MI tương ứng.

### B. Lỗi C++ Add Widgets Action (Linh hồn)
- **Vấn đề**: Class `RPGGameFeatureAction_AddWidgets` mặc định của Lyra/Plugin chỉ là placeholder (khung trống).
- **Khắc phục**: Đã triển khai code thực tế để tìm `RPGPrimaryGameLayout` và gọi `PushContentToLayer_ForPlayer`.
- **Lưu ý quan trọng**: Action này của chúng ta nhắm vào class **`ARPGHUD`**. Nếu Map/Experience dùng `ALyraHUD`, Action sẽ không chạy.

### C. Lỗi Cấu hình INI & Policy (Hệ thần kinh)
- **Vấn đề**: Quên cấu hình `DefaultGame.ini`.
- **Khắc phục**: Đảm bảo có đoạn sau trong `DefaultGame.ini`:
    ```ini
    [/Script/RPGRuntime.RPGUIManagerSubsystem]
    DefaultPolicyClass=/RPG/UI/B_RPG_UIPolicy.B_RPG_UIPolicy_C
    ```

### D. Lỗi Layer Naming (Địa chỉ)
- **Vấn đề**: Tên các Widget Stack trong `W_RPG_OverallUILayout` không khớp với C++.
- **Khắc phục**: Đặt tên chính xác là `GameLayer_Stack`, `GameMenu_Stack`, `Menu_Stack`, `Modal_Stack`.

---

## 🎨 2. Logic Macro "Create and Assign MID"

Macro này là "trái tim" của việc hiển thị thanh máu động trong Lyra.

### Mục đích:
Tạo ra một **Material Instance Dynamic (MID)** từ một Material cha để có thể thay đổi các tham số (ví dụ: % máu) bằng code mà không ảnh hưởng đến các thanh máu của đối tượng khác.

### Luồng Logic:
1. **Kiểm tra Parent**: Nếu Material cha rỗng -> Báo lỗi.
2. **Kiểm tra MID hiện tại**:
    - Nếu đã tồn tại (Valid) và đúng loại -> Dùng tiếp (Tối ưu hiệu năng).
    - Nếu chưa có -> Tiến hành tạo mới bằng hàm `Create Dynamic Material Instance`.
3. **Gán (Assign)**: Lưu kết quả vào biến MID để sử dụng cho các lần cập nhật sau.

### Chuyển đổi sang C++ (Dành cho việc Standalone hóa):
Thay vì dùng Macro Blueprint cồng kềnh, ta sử dụng hàm sau trong class cha của Widget:

```cpp
/**
 * Tạo MID nếu chưa có và gán vào biến lưu trữ.
 * @param InMID Biến lưu trữ (Reference)
 * @param InParent Material cha
 */
void URPGUserWidget::CreateAndAssignMID(UMaterialInstanceDynamic*& InMID, UMaterialInterface* InParent)
{
    if (!InParent) return;

    // Chỉ tạo mới khi MID chưa tồn tại hoặc cha bị thay đổi
    if (!InMID || InMID->GetParent() != InParent)
    {
        InMID = UMaterialInstanceDynamic::Create(InParent, this);
    }
}
```

---

## ✅ 3. Checklist Gỡ lỗi Định kỳ (Tóm tắt)

Sử dụng checklist này mỗi khi UI không hiển thị như mong muốn:

- [ ] **Bước 1: Cấu hình INI** - `DefaultGame.ini` đã trỏ đúng và có hậu tố `_C` chưa?
    - Mở file **`Config/DefaultGame.ini`**.
    - Thêm (hoặc sửa) đoạn code sau:
    ```ini
    [/Script/RPGRuntime.RPGUIManagerSubsystem]
    DefaultPolicyClass=/RPG/UI/B_RPG_UIPolicy.B_RPG_UIPolicy_C
    ```
- [ ] **Bước 2: Cấu hình Experience/ActionSet** - Kiểm tra trong **Experience Definition** (ví dụ: `B_RPG_Experience`) hoặc **Action Set** (ví dụ: `LAS_RPG_SharedInput`) đã thêm Action **`Add Widgets`** chưa?
- [ ] **Bước 3: Layer Name** - Các Stack trong Blueprint Layout có đặt tên đúng `_Stack` chưa?
- [ ] **Bước 4: HUD Class** - Map/Experience có đang dùng class `ARPGHUD` không? (Tuyệt đối không dùng `ALyraHUD` nếu muốn dùng Action của RPG Plugin).
- [ ] **Bước 5: Material Assets** - Các Image trong Widget đã được gán Material Instance hợp lệ chưa?
- [ ] **Bước 6: Compilation** - Đã Compile C++ và Blueprint thành công chưa?

---

## �️ 4. Cách tìm và đọc Output Log lỗi UI

Output Log là công cụ quan trọng nhất để biết hệ thống UI có đang vận hành đúng luồng không.

### Cách mở Output Log:
1. Trong Unreal Editor, đi tới menu: **Window -> Output Log**.
2. Hoặc sử dụng phím tắt (nếu có cấu hình).

### Cách lọc và tìm lỗi:
1. **Sử dụng Thanh Tìm kiếm (Search Bar)**: Gõ từ khóa **`LogRPG`** để chỉ xem các thông tin liên quan đến Plugin của chúng ta.
2. **Kiểm tra luồng khởi tạo thành công (Màu Trắng/Xanh)**:
    - `RPGUIManagerSubsystem: Loading Default Policy [B_RPG_UIPolicy_C]`
    - `RPGUIPolicy: Creating Root Layout [W_RPG_OverallUILayout_C]`
    - `Registered UI Layer [UI.Layer.Game] with widget [GameLayer_Stack]`
    - `URPGGameFeatureAction_AddWidgets::AddWidgets: Pushing layout [W_RPG_HUDLayout_C]`
3. **Dấu hiệu lỗi (Màu Vàng/Đỏ)**:
    - `RPGUIManagerSubsystem: DefaultPolicyClass is NULL!`
    - `RPGUIPolicy: LayoutClass is NULL!` -> Kiểm tra lại Class Defaults của `B_RPG_UIPolicy`.
    ### 7. Missing RPGAttributeSet / Health Drops to 0 at Startup
    **Issue:** Character has no attributes in `showdebug abilitysystem` or health starts at 100 then immediately drops to 0.

    **Possible Causes:**
    *   **Missing PawnData in PlayerState:** The `PawnData` was not correctly propagated from the GameMode to the PlayerState, causing the initialization state machine to stall.
    *   **Incomplete Initialization Flow:** The character never reached `InitState.DataInitialized`, preventing `InitializeAbilitySystem` from being called.

    **Resolution Steps:**
    1.  **Check `RPGGameMode::HandleStartingNewPlayer_Implementation`**: Ensure it calls `SetPawnData` on the `RPGPlayerState`.
    2.  **Verify `RPGPlayerState` Replication**: Ensure `OnRep_PawnData` notifies the character via `RPGPawnExtensionComponent::HandlePlayerStateReplicated`.
    3.  **Check `DA_RPGGameData`**: Ensure default attribute values are not 0.
    - `URPGGameFeatureAction_AddWidgets::AddWidgets: HUD has no owning player controller` -> Race condition, HUD chưa sẵn sàng.

> [!IMPORTANT]  
> Nếu bạn không thấy bất kỳ dòng nào có chữ `LogRPG` trong Output Log sau khi Play, nghĩa là **Subsystem chưa được khởi tạo**, hãy kiểm tra lại xem Plugin RPG đã được Enable và Code đã được Compile chưa.

---

## 🛠️ 5. Công cụ Gỡ lỗi Trực quan (Visual Debugging)

Khi gặp lỗi UI, hãy sử dụng các lệnh sau trong Console (`~`):

1. **`WidgetReflector`**: Mở bảng soi Widget. Giúp kiểm tra Widget có đang chiếm chỗ trên màn hình không (Dù nó đang tàng hình).
2. **`cat Config/DefaultGame.ini`** (Nếu dùng Terminal): Kiểm tra nhanh cấu hình Policy.
3. **Output Log**: Tìm kiếm từ khóa `LogRPG` để xem luồng nạp UI:
    - `RPGUIManagerSubsystem: Loading Default Policy...`
    - `RPGUIPolicy: Creating Root Layout...`
    - `URPGGameFeatureAction_AddWidgets::AddWidgets: Pushing layout...`

---

## 🛠️ 6. Kiểm tra luồng Input - Ability - Attribute (Mới)

Nếu UI đã hiển thị nhưng thanh máu không thay đổi khi nhấn chuột:

- [ ] **Kiểm tra Input Tag**: Sử dụng lệnh console `showdebug abilitysystem`. Khi nhấn chuột, tag tương ứng (VD: `InputTag.Ability.Attack`) có sáng lên hoặc hiện số đếm không?
- [ ] **Kiểm tra Ability Granting**: Đảm bảo GA đã được thêm vào `URPGAbilitySet` và `AbilitySet` đó đã được gán cho `PawnData` của nhân vật.
- [ ] **Kiểm tra Attribute Set**:
    - Máu (Health) trong C++ được tính toán trong `RPGAttributeSet::PostGameplayEffectExecute`.
    - Phải áp dụng Gameplay Effect vào attribute `Damage` (để trừ máu) hoặc `Healing` (để cộng máu).
- [ ] **Output Log**: Tìm kiếm `InitializePlayerInput: Bound X Ability Actions`. Nếu X = 0, bạn chưa cấu hình `Data Asset` cho Input.

---

## 🎨 8. Xử lý lỗi Thanh máu không chạy (Animation & Material)

Nếu chỉ số máu (số) nhảy nhưng thanh bar đứng yên hoặc không có hiệu ứng "tụt dần", lý do thường nằm ở Material Parameters.

### A. Các tham số Material quan trọng:
Thanh máu của Lyra sử dụng các Scalar Parameter sau để điều khiển hiển thị:
- **`Health_Current`**: % máu hiện tại (0-100).
- **`Health_Updated`**: % máu mục tiêu sau khi đổi (0-100).
- **`Animate_Damage`**: Kích hoạt hiệu ứng nháy đỏ khi mất máu (0 hoặc 1).
- **`Animate_Heal`**: Kích hoạt hiệu ứng nháy xanh khi hồi máu (0 hoặc 1).

### B. Logic nháy hiệu ứng (Flash Effect):
Để thanh máu có cảm giác "sống", C++ phải gán `Animate_Damage` lên 1.0 khi máu giảm, sau đó reset về 0.0. 

### C. Cách Fix trong C++:
Chúng ta cần cập nhật hàm `UpdateHealthBar` để nhận diện hướng thay đổi (giảm hay tăng) và cập nhật Parameter tương ứng:

```cpp
// Nếu máu giảm (Damage)
MID->SetScalarParameterValue(FName("Animate_Damage"), 1.0f);
// Nếu máu tăng (Heal)
MID->SetScalarParameterValue(FName("Animate_Heal"), 1.0f);
```

> [!TIP]
> Material của Lyra thường xử lý việc mờ dần (fade out) của hiệu ứng nháy dựa trên thời gian nội bộ của nó, nên đôi khi chỉ cần set lên 1.0 là đủ.

### D. Lỗi Range (0-1 vs 0-100):
- **Triệu chứng**: Số nhảy đúng (ví dụ từ 100 xuống 70) nhưng thanh bar luôn đầy hoặc luôn rỗng.
- **Nguyên nhân**: C++ gửi giá trị `100.0` vào Material, trong khi Material chỉ hiểu giá trị từ `0.0` đến `1.0`. Bất kỳ giá trị nào > 1.0 sẽ được coi là "Đầy 100%".
- **Khắc phục**: Luôn dùng giá trị **Normalized (0.0 - 1.0)** khi gọi `SetScalarParameterValue` cho thanh Progress Bar.

### E. Lỗi Thanh máu "giật cục" (Giật khựng):
- **Vấn đề**: Thanh máu nhảy ngay lập tức thay vì trượt mượt mà.
- **Nguyên nhân**: Gọi `ResetAnimatedState()` quá sớm trong C++ làm triệt tiêu hiệu ứng nội suy (Lerp) của Shader. Shader cần các tham số cũ và mới được giữ nguyên để tính toán độ trễ.
- **Khắc phục**: 
    - Bỏ gọi Reset trong hàm `UpdateHealthBar`. 
    - Để Shader tự xử lý việc giảm dần hiệu ứng thông qua thời gian.
    - Đảm bảo `Health_Current` (Giá trị cũ) và `Health_Updated` (Giá trị mới) được gán đúng cách để Shader có điểm bắt đầu và điểm kết thúc để nội suy.

### F. Điều chỉnh tốc độ trượt (CatchUp Speed):
- **Vấn đề**: Thanh máu trượt quá nhanh hoặc quá chậm.
- **Khắc phục**: 
    - Sử dụng biến `CatchUpSpeed` (hoặc `InterpSpeed`) trong C++.
    - Giá trị càng thấp (VD: 0.5), thanh máu bắt kịp càng chậm (tạo cảm giác "nặng" và mượt hơn).
    - Đảm bảo trong Shader Material có sử dụng tham số `InterpSpeed` để nhân với DeltaTime hoặc Alpha.

---

🛠️ Thiết lập Material Graph (Time-based Lerp)
Bạn hãy tạo/sửa Material của thanh máu với logic như sau:

1. Khai báo 4 Scalar Parameters:

Health_Start: Giá trị máu cũ.
Health_Target: Giá trị máu mới.
Health_AnimStartTime: Thời điểm bắt đầu (Timestamp).
Health_AnimDuration: Thời gian chạy hiệu ứng.
2. Node Logic (Copy-Paste tư duy):

Bước 1 (Tính Alpha): Time (Engine Node) ➡️ Subtract (Health_AnimStartTime) ➡️ Divide (Health_AnimDuration) ➡️ Saturate (để khóa Alpha trong 0-1).
Bước 2 (Làm mượt - Optional): Dẫn Alpha qua node SmoothStep (Input 0=0, 1=1) để có hiệu ứng trượt mượt mà (Ease-in, Ease-out).
Bước 3 (Tính kết quả): Lerp (A = Health_Start, B = Health_Target, Alpha = Kết quả từ bước 2).
3. Kết nối:

Kết quả của node Lerp cuối cùng chính là giá trị để bạn đưa vào logic che (Mask) hoặc cắt (Crop) thanh máu của mình.
💡 Mẹo nhỏ cho Designer:
Trong Widget Editor, hãy chọn HealthNumber (CommonNumericTextBlock) và chỉnh:

Maximum Interpolation Duration: 0.35 (Khớp với con số damage trong C++).
Bây giờ khi nhận damage, C++ sẽ gán các tham số vào Material và Shader sẽ tự xử lý hiệu ứng nội suy thông qua thời gian.


