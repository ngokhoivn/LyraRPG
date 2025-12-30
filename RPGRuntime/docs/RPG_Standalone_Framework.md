# Danh sách Class C++ Standalone (RPG Framework)

Tài liệu này tổng hợp toàn bộ các thành phần lõi đã được refactor sang bản Standalone cho Plugin RPG, gỡ bỏ sự phụ thuộc vào LyraGame.

---

## 1. Hệ thống Lõi & Khởi tạo
- **`URPGAssetManager`**: Quản lý nạp Asset thông minh (Async/Sync), tích hợp Startup Jobs.
- **`RPGGameplayTags`**: Hệ thống Tag trung tâm cho toàn bộ Plugin.
- **`RPGLogChannels`**: Các kênh Log riêng biệt (`LogRPG`, `LogRPGExperience`).

## 2. Hệ thống Experience & Game Framework
- **`URPGExperienceDefinition` & `URPGExperienceActionSet`**: Data Asset định nghĩa kịch bản màn chơi.
- **`URPGExperienceManagerComponent`**: Linh kiện điều phối việc nạp Experience.
- **`URPGExperienceManager`**: Tracking reference count cho Game Feature Plugins.
- **`ARPGGameMode`**: Khởi chạy Experience và quản lý spawn.
- **`ARPGGameState`**: Chứa Experience Manager và Global ASC.
- **`ARPGWorldSettings`**: Cho phép gán Experience trực tiếp vào Level.
- **`FRPGVerbMessage`**: Struct tin nhắn phục vụ Multicast thông báo.

## 3. Hệ thống Nhân vật & GAS
- **`URPGPawnExtensionComponent`**: Điều phối vòng đời khởi tạo Pawn (Init State).
- **`URPGAbilitySystemComponent`**: Thành phần cốt lõi của GAS Standalone.
- **`URPGPawnData`**: Định nghĩa dữ liệu gốc cho Pawn (ASC, Input, Camera).
- **`URPGHeroComponent`**: Xử lý Input Mapping và Camera cho nhân vật.
- **`URPGHealthComponent`**: Quản lý máu và trạng thái sinh mệnh.

## 4. Hệ thống Kho đồ & Trang bị (Modular)
- **`URPGInventoryManagerComponent` & `URPGQuickbarComponent`**.
- **`URPGEquipmentManagerComponent` & `URPGEquipmentInstance`**.

---
*Tài liệu được cập nhật dựa trên tiến độ Standalone Framework - 2025.*
