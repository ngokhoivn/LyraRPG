# RPG UI Structure

## Overview
Dự án sử dụng hệ thống UI Layer dựa trên Lyra, phân tách giữa "Khung Layout" (Primary Layout) và "Nội dung Layout" (HUD Layout).

## Hierarchy (Hệ thống phân cấp)

1.  **UI Policy (`B_RPG_UI_Policy`)**
    - Trỏ tới **Root Layout Class**: `W_RPG_OverallUILayout`.

2.  **Primary Layout (`W_RPG_OverallUILayout`)**
    - Kế thừa từ: `RPGPrimaryGameLayout`.
    - Vai trò: Chứa các **Stacks** (Ngăn xếp) để quản lý hiển thị theo lớp.
    - **Các Stacks chính**:
        - `GameLayer_Stack`: Chứa HUD chính.
        - `GameMenu_Stack`: Chứa Menu (Inventory, v.v.).
        - `Menu_Stack`: Chứa Main Menu.
        - `Modal_Stack`: Chứa Pop-up.

3.  **HUD Layout (`W_RPG_HUDLayout`)**
    - Kế thừa từ: `RPGHUDLayout`.
    - Vị trí: Được đẩy (Push) vào `GameLayer_Stack` của Primary Layout.
    - Vai trò: Chứa các Widget hiển thị thông tin thực tế khi chơi.
    - **UI Extension Points**:
        - Đây là nơi chứa các "lỗ hổng" (Slots) để đăng ký widget từ C++ hoặc Data Asset.
        - **`HUD.Slot.Reticle`**: Slot dành cho hồng tâm và Widget đếm ngược hồi sinh (`W_RespawnTimer`).

## Workflow đăng ký UI
1. Ability/System gọi `RegisterExtensionAsWidgetForContext`.
2. Hệ thống tìm `UIExtensionPoint` có tag tương ứng (ví dụ: `HUD.Slot.Reticle`).
3. Widget được tạo và gắn vào Extension Point đó.

> [!IMPORTANT]
> Nếu `W_RespawnTimer` không hiện, hãy đảm bảo `UIExtensionPoint` nằm trong **`W_RPG_HUDLayout`** chứ không phải trong `W_RPG_OverallUILayout`.
