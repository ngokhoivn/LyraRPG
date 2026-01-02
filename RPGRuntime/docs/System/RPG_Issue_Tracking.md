# RPG Issue Tracking & Resolution

Tài liệu này dùng để theo dõi các lỗi phát sinh trong quá trình chuyển đổi Standalone RPG và các bước khắc phục cụ thể.

## 1. Lỗi Cấu hình Asset Manager (Mức ưu tiên: CAO NHẤT)

### Hiện tượng
`LogRPG: Error: URPGAssetManager is not set as the AssetManagerClassName in DefaultEngine.ini. Fallback to base class functionality.`

### Giải pháp (3 bước quan trọng - CẬP NHẬT)

**Bước 1: Sửa lỗi Crash tại `InitializeNativeTags` (ĐÃ THỰC HIỆN TRONG CODE)**
*   **Nguyên nhân**: Hàm `InitializeNativeTags()` bị gọi quá sớm trong `StartupModule` của plugin khi Engine chưa sẵn sàng. Ngoài ra, hàm này đã được `RPGAssetManager` gọi lại một lần nữa (dư thừa).
*   **Fix**: Tôi đã xóa lời gọi hàm này trong `RPGRuntimeModule.cpp`. Bây giờ chỉ có Asset Manager chịu trách nhiệm khởi tạo Tags vào đúng thời điểm an toàn.

**Bước 2: Cấu hình chuẩn `RPG.uplugin`**
Bạn cần đảm bảo cấu hình như sau để Module nạp đúng lúc cho Asset Manager:
```json
"Modules": [
    {
        "Name": "RPGRuntime",
        "Type": "Runtime",
        "LoadingPhase": "PreDefault"
    }
],
"ExplicitlyLoaded": false,
"BuiltInInitialFeatureState": "Active"
```

**Bước 3: Phát sinh lại Project Files và Build**
1. Đóng sạch Editor/IDE.
2. Chuột phải vào `.uproject` -> **Generate Visual Studio project files**.
3. **Build** lại Solution.

> [!TIP]
> Việc xóa `InitializeNativeTags()` khỏi `StartupModule` giải quyết lỗi crash ở dòng 10 trong `RPGGameplayTags.cpp`. Bây giờ hệ thống sẽ đợi đến khi Asset Manager khởi động mới nạp Tags.

---

## 2. Thiếu Experience Asset mặc định (Mức ưu tiên: CAO)

### Hiện tượng
`LogRPGExperience: Error: EXPERIENCE: Failed to load Experience definition RPGExperienceDefinition:B_RPGDefaultExperience from path .`

### Phân tích
GameMode đang cố gắng fallback về một asset tên là `B_RPGDefaultExperience` nhưng asset này chưa tồn tại trong folder Content của bạn.

### Giải pháp
1.  Trong Unreal Editor, chuột phải vào folder `/Content/RPG/Experiences/` (hoặc folder tương tự bạn đã cấu hình trong Asset Manager).
2.  Chọn **Miscellaneous -> Data Asset**.
3.  Chọn Class là **RPGExperienceDefinition**.
4.  Đặt tên chính xác là: `B_RPGDefaultExperience`.
5.  Mở asset đó ra, cấu hình ít nhất:
    *   **Default Pawn Data**: Chọn một Pawn Data hợp lệ (ví dụ: `DA_Hero_Axe`).

---

## 3. Lỗi thiếu Animation Blueprints (Mức ưu tiên: TRUNG BÌNH)

### Hiện tượng
`LogStreaming: Warning: LoadPackage: SkipPackage: /Game/RPG/Animations/ABP_Player_Axe ... does not exist on disk`
`LogUObjectGlobals: Error: CDO Constructor (RPGAxeInstance): Failed to find /Game/RPG/Animations/ABP_Player_Axe.ABP_Player_Axe_C`

### Phân tích
Trong code C++ của các Weapon Instance (`RPGAxeInstance`, `RPGGreatswordInstance`), có các đường dẫn cứng (hardcoded paths) trỏ đến các Animation Blueprint. Các file này hiện không có trên ổ đĩa tại vị trí đó.

### Giải pháp
Bạn cần tạo (hoặc copy từ Lyra/Project cũ) các Animation Blueprint vào đúng đường dẫn sau trong folder Content của project:
*   `/Game/RPG/Animations/ABP_Player_Axe`
*   `/Game/RPG/Animations/ABP_Player_Unarmed`
*   `/Game/RPG/Animations/ABP_Player_Greatsword`

**Các file C++ tham chiếu cứng (Hardcoded) cần lưu ý:**
*   `RPGAxeInstance.cpp`: Trỏ đến `ABP_Player_Axe` và `ABP_Player_Unarmed`.
*   `RPGGreatswordInstance.cpp`: Trỏ đến `ABP_Player_Greatsword` và `ABP_Player_Unarmed`.

Nếu bạn muốn đổi đường dẫn hoặc sử dụng Asset khác, hãy cập nhật lại ở hàm Constructor trong các file `.cpp` nêu trên.

---

## 4. Reset và Theo dõi (Communication)

Tôi đã tạo tài liệu này để chúng ta theo dõi tiến độ sửa lỗi. Khi bạn hoàn thành một bước (ví dụ: sửa xong .ini), hãy nhắn cho tôi để tôi kiểm tra lại log.

---

## Trạng thái Hiện tại
- [ ] Cấu hình `AssetManagerClassName` trong `DefaultEngine.ini`.
- [ ] Tạo Data Asset `B_RPGDefaultExperience`.
- [ ] Khắc phục các file Animation Blueprint bị thiếu (hoặc sửa lại đường dẫn trong C++).
- [ ] Cấu hình `RPGGameDataPath` trong `DefaultGame.ini` và tạo asset tương ứng.

---

## 5. Lỗi Crash: Cast of nullptr to RPGGameData (NEW)

### Hiện tượng
`Fatal error: [...] Cast of nullptr to RPGGameData failed`

### Phân tích
`URPGAssetManager` được yêu cầu load dữ liệu game cơ bản (`RPGGameData`) ngay khi khởi động, nhưng nó không tìm thấy đường dẫn asset này trong file cấu hình, dẫn đến việc Cast một đối tượng rỗng (nullptr).

### Giải pháp (Cập nhật quan trọng nhất)

**1. Sửa file `Config/DefaultGame.ini` (KHÔNG phải DefaultEngine.ini)**:
Vì class `URPGAssetManager` được khai báo với `Config = Game`, nên các thuộc tính của nó phải nằm trong `DefaultGame.ini` để Engine có thể đọc được.
* Di chuyển đoạn sau từ `DefaultEngine.ini` sang `DefaultGame.ini`:
```ini
[/Script/RPGRuntime.RPGAssetManager]
RPGGameDataPath="/RPG/System/DA_RPGGameData.DA_RPGGameData"
```

**2. Xác nhận đường dẫn Asset**:
Đường dẫn `"/RPG/System/DA_RPGGameData.DA_RPGGameData"` tương ứng với:
* Folder trong Editor: `/Content/RPG/System/` (đối với plugin RPG).
* Tên file asset: `DA_RPGGameData`.
* Đảm bảo trong Editor, asset này đã được tạo với Class là `RPGGameData`.

> [!IMPORTANT]
> **HÀNH ĐỘNG BẮT BUỘC ĐỂ KHỞI ĐỘNG EDITOR:**
> 1. **Bật lại plugin `GameFeatures`**: Trong file `.uproject`, bạn **PHẢI** để `"Enabled": true` cho `GameFeatures`. Toàn bộ plugin RPG phụ thuộc vào nó.
> 2. **Cấu hình `RPG.uplugin`**: Đặt `"ExplicitlyLoaded": false`. Nếu để là `true`, nạp module sẽ bị trì hoãn và Asset Manager sẽ không tìm thấy class cần thiết.
> 3. **Rebuild C++**: Tôi đã sửa code để không còn bị crash cứng (Fatal Error) khi thiếu GameData, giúp bạn có thể vào được Editor.

---

## 7. Lỗi: Unexpected Input Component class (ĐÃ XỬ LÝ - RESILIENT FIX)

### Hiện tượng
Log xác nhận class thực tế là `LyraInputComponent`, gây Cast lỗi sang `URPGInputComponent`.

### Giải pháp (Đã thực hiện)
*   **Resilient Binding**: Cập nhật `RPGHeroComponent.cpp` để hỗ trợ binding cho bất kỳ class nào kế thừa từ `UEnhancedInputComponent` (bao gồm cả `LyraInputComponent`).
*   **Fallback Logic**: Nếu không phải `URPGInputComponent`, code sẽ tự động dùng hàm `BindAction` chuẩn của Engine để kết nối các phím từ `InputConfig`.

---

## 8. Lỗi: Missing Native Input Actions (ĐANG CHỜ FIX ASSET)

### Hiện tượng
`Can't find NativeInputAction for InputTag [...] on InputConfig [InputData_RPG]`
Thiếu mapping cho `InputTag.Look.Stick`, `InputTag.Crouch`, `InputTag.AutoRun`.

### Giải pháp (Người dùng cần thực hiện)
1. Mở asset `InputData_RPG` (trong folder `RPG/Input/`).
2. Trong phần **Native Input Actions**, thêm các entry cho các tag còn thiếu.
3. Map các tag đó với các `InputAction` asset tương ứng (có thể dùng lại IA của Lyra như `IA_Look_Stick`).

---

## Trạng thái Hiện tại
- [x] Cấu hình `AssetManagerClassName`.
- [x] Tạo Data Asset `B_RPGDefaultExperience`.
- [x] Khắc phục các file Animation Blueprint.
- [x] Cấu hình `RPGGameDataPath`.
- [x] Sửa lỗi code Input Component (Resilient Fix).
- [/] Đang chờ người dùng cập nhật Asset `InputData_RPG`.

---

## 9. Lỗi: Ngược trục Y chuột (Pitch) (ĐÃ SỬA)

### Hiện tượng
Khi di chuyển chuột lên trên, camera lại hướng xuống dưới (Inverted Y-axis).

### Giải pháp
Sửa code trong `RPGHeroComponent.cpp` tại hàm `Input_LookMouse` và `Input_LookStick`:
* Nhân giá trị `Value.Y` với `-1.0f` trước khi truyền vào `AddControllerPitchInput`.
* Điều này giúp đồng bộ hóa hướng di chuyển chuột với hướng nhìn của camera theo thói quen phổ biến.
