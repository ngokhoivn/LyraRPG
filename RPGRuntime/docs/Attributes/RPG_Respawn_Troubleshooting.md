# Hướng dẫn Khắc phục Sự cố & Bảo trì Hệ thống Respawn (RPG)

Tài liệu này ghi lại kiến trúc, các lỗi đã biết và cách bảo trì hệ thống hồi sinh trong RPG Plugin. Đây là tài liệu sống, cần được cập nhật mỗi khi phát hiện lỗi mới hoặc thay đổi logic.

## 1. Kiến trúc Hệ thống
Hệ thống hồi sinh dựa trên **Lyra Architecture**, chia làm 2 phần:
- **Persistent Data (PlayerState)**: Chứa Ability System Component (ASC) và Attribute Set. Dữ liệu này tồn tại xuyên suốt trận đấu.
- **Transient Actor (Pawn)**: Nhân vật đại diện trên màn hình. Bị xóa khi chết và tạo mới khi hồi sinh.

> [!IMPORTANT]
> Luôn truy cập ASC thông qua `RPGPawnExtensionComponent` để đảm bảo lấy đúng ASC từ PlayerState, tránh rò rỉ hoặc sai lệch dữ liệu trên Pawn cũ.

## 2. Các điểm kiểm soát quan trọng (Checkpoints)

### A. Reset Chỉ số (Attributes)
Mọi việc reset chỉ số phải thông qua hàm tập trung:
- **Hàm**: `URPGAbilitySystemComponent::FullResetAttributes()`
- **Tác dụng**: Reset Health, Mana, Stamina về Max.
- **Vị trí gọi**: `RPGGA_AutoRespawn::ExecuteRespawn`.

### B. Logic "Bất tử" (bOutOfHealth)
- **Vấn đề**: Flag `bOutOfHealth` trong `RPGAttributeSet` ngăn chặn việc kích hoạt Event Death nhiều lần liên tiếp.
- **Giải pháp**: Phải override `PostAttributeChange` để reset `bOutOfHealth = false` ngay khi máu > 0 (lúc hồi sinh). Nếu không, nhân vật sẽ không thể chết ở mạng thứ 2.

### C. Ngăn chặn Cộng dồn (Ability Leaks)
- **Cơ chế**: Vũ khí/Trang bị cấp kỹ năng cho PlayerState.
- **Rủi ro**: Nếu Pawn chết mà không gỡ kỹ năng (Unequip), mạng sau sẽ bị gán thêm bộ kỹ năng mới -> Damage cộng dồn (20 -> 40 -> 60).
- **Kiểm tra**: Luôn đảm bảo `RPGWeaponInstance::Unequip` được gọi và tìm đúng ASC trên PlayerState để `TakeFromAbilitySystem`.

## 3. Nhật ký sửa lỗi (Bug History)

| Ngày | Lỗi | Nguyên nhân | Giải pháp |
| :--- | :--- | :--- | :--- |
| 02/01/26 | Damage cộng dồn | Unequip không tìm thấy ASC trên Pawn đã chết | Dùng `PawnExtensionComponent` để tìm ASC trên PlayerState |
| 02/01/26 | Không chết được mạng 2 | `bOutOfHealth` không được reset | Override `PostAttributeChange` để reset flag |
| 02/01/26 | HUD không cập nhật | Broadcast chỉ nằm trong PostGameplayEffect | Chuyển Broadcast UI vào `PostAttributeChange` |
| 01/01/26 | Respawn UI không hiện | PlayerController chưa sẵn sàng | Thêm Retry logic trong `RPGUIPolicy` |

## 4. Hướng dẫn Tuning (Cấu hình)
- **Gameplay Effect**: Sát thương tức thời LUÔN dùng chế độ **Instant**. Tuyệt đối không dùng `Duration` cho Meta-Attribute (Damage/Healing).
- **Gameplay Tags**: 
    - `Ability.Respawn.Duration`: Để UI bắt đầu đếm ngược.
    - `Ability.Respawn.Completed`: Để đóng UI và hồi sinh.

---
*Cập nhật lần cuối: 02/01/2026 bởi Antigravity*
