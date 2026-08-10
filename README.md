# Eternal Siege

## 1. Giới thiệu
- Eternal Siege là trò chơi phòng thủ 2D thời gian thực phát triển bằng C++17 và SFML 3.
- Người chơi điều khiển Spirit Mage đi qua Asterfall Kingdom (Royal Capital / The Last Bastion), tiến vào Ruined Catacombs, lựa chọn và bố trí đội hình 4 Ally đồng minh, vượt qua 4 wave quái vật dồn dập trên bản đồ tạo ngẫu nhiên theo thuật toán Drunkard Walk và đánh bại Boss cuối để bảo vệ vương quốc.

## 2. Luồng game
Intro -> Menu -> Kingdom -> Level Select -> Setup -> Gameplay -> Win / Game Over.

## 3. Điều khiển

### Asterfall Kingdom Hub (Fixed Handcrafted Map)
- Location: **Asterfall Kingdom (The Last Bastion)**
- `W`, `A`, `S`, `D`: Di chuyển nhân vật (230 px/s, bám đất mượt mà).
- `E`: Tương tác (Mở cổng thành, di chuyển vào Ruined Catacombs với vùng cảm ứng 120px).
- `V`: Bật / Tắt chỉ đường tới Objective (Genshin-style Navigation: đường đi vàng, beacon điểm đến, mũi tên chỉ hướng ngoài màn hình).
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
- Combat map được sinh ngẫu nhiên thuật toán **Drunkard Walk** (carving đường đi và cỏ, kiểm tra connected component và tự động chọn 4 vị trí xuất hiện quái hợp lệ).
- `W`, `A`, `S`, `D`: Di chuyển Player.
- Nhấp chuột trái (LMB Tap/Release): Bắn Spirit Bolt.
- Giữ chuột trái (LMB Hold/Release): Nạp năng lượng và bắn Heavy Spirit Bolt.
- `Shift` hoặc Chuột phải (RMB): Dash lướt nhanh né đòn.
- `Q`: Radiant Pulse (Hồi 18% HP tối đa cho các Ally hợp lệ trong phạm vi).
- `P` / `Escape`: Tạm dừng / Tiếp tục game (Pause / Resume).
- `R`: Bật / Tắt bảng Hướng dẫn (Tutorial).
- `M`: Bật / Tắt âm thanh (Mute).

### Intermission (Nâng cấp giữa các Wave)
- `1`, `2`, `3`: Mua các gói nâng cấp chỉ số.
- `Backspace`: Hoàn tác (Undo) giao dịch nâng cấp gần nhất và hoàn Gold.
- `Enter`: Bắt đầu wave tiếp theo.

## 4. Nhân vật và Gameplay
- **Player (Spirit Mage)**: Trang bị Spirit Staff, tấn công bằng Spirit Bolt và Heavy Spirit Bolt, hỗ trợ đồng đội bằng Radiant Pulse.
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
  - Phase Enrage (dưới ~25% HP): Tăng tốc độ di chuyển và sát thương.

## 5. Công nghệ
- **Ngôn ngữ**: C++17.
- **Thư viện đồ họa & âm thanh**: SFML 3 (Audio, Graphics, Window, System).
- **Build System**: CMake (bản 3.21 trở lên).

## 6. Build và Chạy Game

### Build bằng CMake
```powershell
cmake -S . -B build
cmake --build build --parallel 4
```

### Chạy Executable
```powershell
.\build\my_game.exe
```

Có thể sử dụng file script `BUILD_WINDOWS.bat` để tự động build nhanh trên môi trường Windows.

## 7. Cấu trúc Project

```text
Eternal-Siege/
├── assets/             # Hình ảnh, âm thanh, font chữ và tilemaps
├── docs/               # Tài liệu báo cáo và kiến trúc hệ thống
├── include/            # Các file header (.h) định nghĩa lớp và giao diện
├── src/                # Mã nguồn thực thi (.cpp)
├── BUILD_WINDOWS.bat   # Script build nhanh trên Windows
├── RUN_GAME.bat        # Script chạy trò chơi
├── CMakeLists.txt      # File cấu hình build CMake
├── README.md           # Tài liệu hướng dẫn project
└── .gitignore          # File cấu hình bỏ qua git
```

## 8. Kiến trúc OOP

Project áp dụng chặt chẽ 4 nguyên lý cốt lõi của Lập trình Hướng đối tượng:

1. **Tính Đóng gói (Encapsulation)**:
   - Các thuộc tính trạng thái, HP, vị trí, cooldown của `Player`, `Ally`, `Monster`, `Boss` được che giấu trong phạm vi `private`/`protected`. Accessor/Mutator được cung cấp an toàn qua các hàm member.
2. **Tính Kế thừa (Inheritance)**:
   - Hệ thống trạng thái game kế thừa từ lớp cơ sở `State` (`KingdomState`, `SetupState`, `GameplayState`, `MenuState`, v.v.).
   - Nhân vật trong trận chiến kế thừa từ `Entity` (`Player`, `Ally`, `Monster`, `Elite`, `Boss`).
   - Vũ khí kế thừa từ `Weapon` (`Sword`, `Bow`, `Wand`, `SpiritStaff`).
3. **Tính Đa hình (Polymorphism)**:
   - `StateMachine` quản lý và cập nhật `State` hiện tại thông qua con trỏ polymorph `std::unique_ptr<State>`.
   - `CombatManager` cập nhật và render danh sách `Entity` và `Weapon` thông qua các phương thức ảo (`update`, `draw`, `attack`).
4. **Tính Trừu tượng (Abstraction)**:
   - Giao diện `State`, `Entity`, `Weapon` ẩn đi các chi tiết tính toán va chạm hay render bên dưới, giúp các manager (`WaveManager`, `UpgradeManager`, `CombatManager`) tương tác ở mức độ trừu tượng cao.

## 9. Kiểm thử thủ công (Manual Testing)

Đã thực hiện kiểm thử thủ công toàn bộ các luồng chức năng của trò chơi:
- **Menu & Chuyển State**: Chuyển đổi mượt mà từ Intro -> Menu -> Kingdom -> Level Select -> Setup -> Gameplay -> Win / Game Over.
- **Asterfall Kingdom Hub**: Di chuyển WASD mượt màng, bám đất, camera follow mượt không lag/dead-zone, chỉ đường `V` (Genshin-style), banner địa danh `ASTERFALL KINGDOM`, va chạm công trình/NPC, chuyển đổi thời tiết F5 và ngày đêm F4.
- **Procedural Map**: Kiểm tra Drunkard Walk sinh địa hình ngẫu nhiên liên thông, tự động phân bố 4 vị trí xuất hiện quái hợp lệ.
- **Setup Team**: Kiểm tra chọn thẻ hero, đặt 4 Ally vào vị trí hợp lệ, thu hồi chuột phải, reset đội hình bằng `R` và xác nhận chỉ cho bắt đầu khi đủ 4 Ally.
- **Combat & Skills**: Kiểm tra Spirit Bolt (LMB tap), Heavy Spirit Bolt (LMB hold), Dash (`Shift`/RMB), Radiant Pulse (`Q` hồi máu Ally trong phạm vi).
- **Wave System & Intermission**: Kiểm tra thứ tự xuất hiện 4 wave, tính năng mua nâng cấp `1`/`2`/`3`, hoàn tác nâng cấp `Backspace` và qua wave `Enter`.
- **Boss Battle**: Kiểm tra chuyển giao Phase 2 (Beam Laser ~50% HP), trạng thái Enrage (~25% HP), luồng Win khi hạ Boss và Game Over khi Player hết HP.

## 10. Asset Attribution
Thông tin chi tiết về tác giả và nguồn gốc các tài nguyên (hình ảnh, âm thanh, font chữ) được lưu trữ tại file `assets/ATTRIBUTION.md`.
