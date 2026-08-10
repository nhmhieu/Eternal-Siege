# Eternal Siege

## 1. Giới thiệu
- Eternal Siege là trò chơi hành động - phòng thủ 2D thời gian thực phát triển bằng C++17 và SFML 3.
- Người chơi vào vai Spirit Warden, đi qua Asterfall Kingdom (The Last Bastion), tiến vào Ruined Catacombs, lựa chọn và bố trí đội hình 4 Ally đồng minh, vượt qua 4 wave quái vật dồn dập trên bản đồ tạo ngẫu nhiên theo thuật toán Drunkard Walk và đánh bại Abyssal Lord để bảo vệ vương quốc.

## 2. Luồng game
Intro -> Menu -> Asterfall Kingdom -> Level Select -> Setup -> Gameplay -> Win / Game Over.

## 3. Điều khiển

### Asterfall Kingdom Hub (Fixed Handcrafted Map)
- Location: **Asterfall Kingdom (The Last Bastion)**
- `W`, `A`, `S`, `D`: Di chuyển nhân vật (230 px/s, bám đất mượt mà).
- `E`: Tương tác (Mở cổng thành, di chuyển vào Ruined Catacombs với vùng cảm ứng 120px).
- `V`: Bật / Tắt hệ thống chỉ đường mục tiêu (đường chỉ dẫn vàng, beacon điểm đến và mũi tên hướng khi mục tiêu nằm ngoài màn hình).
- `Escape`: Quay lại Main Menu.
- `F3`: Bật/tắt Navigation & Collision Debug Overlay.
- `F4`: Chuyển đổi giai đoạn thời gian (Dawn / Day / Sunset / Night).
- `F5`: Chuyển đổi thời tiết (Clear / LightRain / Mist).
- `F6`: Bật/tắt Thông tin hiệu năng & cache (FPS, frame time, texture cache hit/miss).

### Level Select
- `A` / `D` hoặc Phím Mũi tên: Chọn Dungeon (Ruined Catacombs).
- `Enter`: Xác nhận chọn Dungeon.
- `Escape`: Quay lại Kingdom Hub.

### Setup (Bố trí đội hình)
- Click chuột trái chọn Thẻ Hero để xem chỉ số và kỹ năng.
- Click chuột trái vào ô hợp lệ trên bản đồ để đặt hoặc di chuyển Ally.
- Click chuột phải vào Ally đã đặt để thu hồi.
- `R`: Reset lại đội hình (thu hồi toàn bộ Ally).
- `Escape`: Quay lại Level Select.
- Phải đặt đủ 4 Ally mới có thể bắt đầu trận đấu.

### Gameplay (Procedural Combat Map)
- Combat map được sinh ngẫu nhiên theo thuật toán **Drunkard Walk** (carving đường đi và cỏ, kiểm tra connected component và tự động chọn 4 vị trí xuất hiện quái hợp lệ).
- `W`, `A`, `S`, `D`: Di chuyển Player.
- Nhấp chuột trái (LMB Tap/Release): Bắn Spirit Bolt.
- Giữ chuột trái (LMB Hold/Release): Nạp năng lượng và bắn Heavy Spirit Bolt.
- `Shift` hoặc Chuột phải (RMB): Dash lướt nhanh né đòn.
- `Q`: Radiant Pulse (Hồi 18% HP tối đa cho các Ally hợp lệ trong phạm vi, không hồi Player).
- `P` / `Escape`: Tạm dừng / Tiếp tục game (Pause / Resume).
- `R`: Bật / Tắt bảng Hướng dẫn (Tutorial).
- `M`: Bật / Tắt âm thanh (Mute).

### Intermission (Nâng cấp giữa các Wave)
- `1`, `2`, `3`: Mua các gói nâng cấp chỉ số.
- `Backspace`: Hoàn tác (Undo) giao dịch nâng cấp gần nhất và hoàn Gold.
- `Enter`: Bắt đầu wave tiếp theo.
- `R`: Bật / Tắt bảng hướng dẫn (Tutorial).

## 4. Nhân vật và Gameplay
- **Player (Spirit Warden)**: Trang bị Spirit Staff, tấn công bằng Spirit Bolt và Heavy Spirit Bolt, hỗ trợ đồng đội bằng Radiant Pulse.
- **4 Ally đồng minh**:
  - **Damian**: Dùng Bow + Explosive Arrow (bắn xa, gây sát thương diện rộng).
  - **Evangeline**: Dùng Sword + Shield Wall (cận chiến, chống chịu chính).
  - **Junior**: Dùng Wand + Arcane Burst (pháp sư hỗ trợ phép thuật).
  - **Lucas**: Dùng Sword + Whirlwind (đấu sĩ cận chiến diện rộng).
- **Wave Plan (4 Wave)**:
  - Wave 1: 1 Quái Normal.
  - Wave 2: 2 Normal -> 2 Normal.
  - Wave 3: 2 Normal -> 2 Normal -> 2 Elite.
  - Wave 4: 2 Normal -> 2 Elite -> 2 Elite -> 1 Boss.
  - Batch tiếp theo chỉ kích hoạt khi toàn bộ quái vật batch hiện tại đã bị tiêu diệt.
- **Boss Final (Ruined Catacombs Lord)**:
  - Phase 1: Tấn công cận chiến và tung kỹ năng thường.
  - Phase 2 (dưới ~50% HP): Kích hoạt Beam Laser càn quét diện rộng.
  - Phase Enrage (dưới ~25% HP): Tăng tốc độ di chuyển và tăng khả năng chống chịu.

## 5. Công nghệ
- **Ngôn ngữ**: C++17.
- **Thư viện đồ họa & âm thanh**: SFML 3 (Audio, Graphics, Window, System).
- **Build System**: CMake (bản 3.21 trở lên).

## 6. Build và Chạy Game

### Cách 1 - Khuyến nghị (Windows + MSYS2 UCRT64)
1. Cài đặt môi trường MSYS2 UCRT64 với SFML 3 và CMake.
2. Chạy file script build tự động:
```cmd
BUILD_WINDOWS.bat
```
3. Sau khi build thành công, khởi chạy trò chơi bằng script:
```cmd
RUN_GAME.bat
```

### Cách 2 - Build thủ công (PowerShell / CMD)
Sử dụng CMake và MinGW Makefiles từ cửa sổ lệnh:
```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\msys64\ucrt64
cmake --build build --parallel 4
.\build\my_game.exe
```

## 7. Cấu trúc Project

```text
Eternal-Siege/
├── assets/             # Hình ảnh, âm thanh và font chữ
├── include/            # Các file header (.h) định nghĩa lớp và giao diện
├── src/                # Mã nguồn C++ thực thi (.cpp)
├── BUILD_WINDOWS.bat   # Script build nhanh trên Windows (MSYS2 UCRT64)
├── RUN_GAME.bat        # Script chạy trò chơi
├── CMakeLists.txt      # File cấu hình build CMake
├── README.md           # Tài liệu hướng dẫn project
└── .gitignore          # File cấu hình bỏ qua git
```

## 8. Kiến trúc OOP

Project áp dụng 4 nguyên lý cốt lõi của Lập trình Hướng đối tượng:

1. **Tính Đóng gói (Encapsulation)**:
   - Trạng thái nội bộ như HP, vị trí, cooldown, target và weapon được quản lý trong các lớp tương ứng (phạm vi `private`/`protected`); lớp bên ngoài thao tác thông qua các hàm thành viên phù hợp.
2. **Tính Kế thừa (Inheritance)**:
   - Lớp cơ sở `State` cho các trạng thái (`IntroState`, `MenuState`, `KingdomState`, `LevelSelectState`, `SetupState`, `GameplayState`, `WinState`, `GameOverState`).
   - Lớp cơ sở `Entity` cho các đối tượng động (`Player`, `Ally`, `Monster`, `Elite`, `Boss`).
   - Lớp cơ sở `Weapon` cho các vũ khí (`Sword`, `Bow`, `Wand`, `SpiritStaff`).
3. **Tính Đa hình (Polymorphism)**:
   - `StateMachine` quản lý và cập nhật `State` hiện tại thông qua con trỏ polymorph `std::unique_ptr<State>`.
   - Các lớp `State` con ghi đè các hàm sự kiện và vòng đời `handleEvent()`, `update()`, `render()`.
   - `Player`, `Ally` và `Monster` kế thừa `Entity` và triển khai các hành vi ảo như `update()`, `draw()`, `takeDamage()`.
   - Các lớp `Sword`, `Bow`, `Wand` và `SpiritStaff` kế thừa `Weapon` và triển khai các hàm thuần ảo như `getHitbox()`, `isHitting()`, `getAttackRange()` và `triggerAction()`.
4. **Tính Trừu tượng (Abstraction)**:
   - Các giao diện `State`, `Entity`, `Weapon` làm lớp cơ sở giúp các module làm việc ở mức độ trừu tượng cao.
   - Các bộ quản lý `CombatManager`, `WaveManager`, `UpgradeManager` và `KingdomMap` che giấu logic xử lý cụ thể khỏi trạng thái chính.

## 9. Kiểm thử thủ công (Manual Testing)

Đã thực hiện kiểm thử thủ công toàn bộ các luồng chức năng của trò chơi:
- **Menu & Chuyển State**: Chuyển đổi mượt mà từ Intro -> Menu -> Kingdom -> Level Select -> Setup -> Gameplay -> Win / Game Over.
- **Asterfall Kingdom Hub**: Di chuyển WASD mượt mà, bám đất, camera follow mượt không lag/dead-zone, chỉ đường mục tiêu `V`, banner địa danh `ASTERFALL KINGDOM`, va chạm công trình/NPC, chuyển đổi thời tiết F5 và ngày đêm F4.
- **Procedural Map**: Kiểm tra Drunkard Walk sinh địa hình ngẫu nhiên liên thông, tự động phân bố 4 vị trí xuất hiện quái hợp lệ.
- **Setup Team**: Kiểm tra chọn thẻ hero, đặt 4 Ally vào vị trí hợp lệ, thu hồi chuột phải, reset đội hình bằng `R` và xác nhận chỉ cho bắt đầu khi đủ 4 Ally.
- **Combat & Skills**: Kiểm tra Spirit Bolt (LMB tap), Heavy Spirit Bolt (LMB hold), Dash (`Shift`/RMB), Radiant Pulse (`Q` hồi máu Ally trong phạm vi).
- **Wave System & Intermission**: Kiểm tra thứ tự xuất hiện 4 wave, tính năng mua nâng cấp `1`/`2`/`3`, hoàn tác nâng cấp `Backspace`, xem hướng dẫn `R` và qua wave `Enter`.
- **Boss Battle**: Kiểm tra chuyển giao Phase 2 (Beam Laser ~50% HP), trạng thái Enrage (~25% HP), luồng Win khi hạ Boss và Game Over khi Player hết HP.
