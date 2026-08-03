# Eternal Siege

Eternal Siege là game phòng thủ thời gian thực 2D viết bằng C++17 và SFML 3.0.2. Người chơi điều khiển pháp sư hỗ trợ Spirit Warden, phối hợp với bốn đồng minh để vượt qua bốn wave gồm Normal, Elite và Boss ba phase.

## Yêu cầu và build

- CMake 3.20 trở lên.
- Trình biên dịch hỗ trợ C++17.
- SFML 3.0.2: Audio, Graphics, Window và System.

Ví dụ với MSYS2 UCRT64 trên Windows:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
cmake -S . -B build -DBUILD_TESTING=ON -G "MinGW Makefiles" `
  -DCMAKE_PREFIX_PATH=C:\msys64\ucrt64
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
.\build\my_game.exe
```

CMake tự chép `assets/` và các DLL runtime do SFML khai báo vào thư mục chứa executable. Có thể dùng `BUILD_WINDOWS.bat` để build nhanh trên cấu hình Windows đã hỗ trợ.

## Điều khiển và luồng game

- Intro → Menu → Setup (chọn đúng 4 vị trí Ally) → Gameplay.
- `W A S D`: di chuyển PlayerMage.
- Chuột trái: bắn Spirit Bolt theo hướng con trỏ.
- `Q`: Radiant Pulse, hồi 18% max HP cho Ally sống, bị thương và trong bán kính.
- `H`: mở/đóng hướng dẫn trong trận.
- `M`: bật/tắt âm thanh.
- `P`: tạm dừng/tiếp tục.
- Giữa các wave: `1`, `2`, `3` mua nâng cấp; `Backspace` hoàn tác giao dịch gần nhất; `Enter` bắt đầu wave kế tiếp.
- Màn Victory/Defeat: `Enter` chơi lại, `Escape` về menu; cũng hỗ trợ chuột.

Player sử dụng Spirit Staff/Spirit Bolt màu xanh–vàng. Junior dùng Wand/Magic tím, Damian dùng Bow, Evangeline và Lucas dùng Sword. Wave 4 kết thúc bằng Boss ba phase: Beam mở ở ngưỡng 50% HP và Enraged mở khi vượt ngưỡng 25% HP. Tiêu diệt Boss dẫn tới Victory; Player hết HP dẫn tới Defeat.

## Cấu trúc

- `src/`, `include/`: mã nguồn và header.
- `assets/`: hình ảnh, tile map, font, nhạc và SFX; nguồn gốc được ghi tại `assets/ATTRIBUTION.md`.
- `tests/`: 12 executable test được đăng ký với CTest.
- `tools/validate_project.py`: kiểm tra asset, cấu hình và tính di động của project.
- `docs/architecture.md`: kiến trúc và ownership.
- `docs/project-report.md`: bản thảo báo cáo đồ án.

Thiết kế sử dụng các lớp trừu tượng `State`, `Entity`, `Weapon`; ownership entity, weapon và projectile được quản lý bằng `std::unique_ptr`. Collision/pathfinding dựa trên dữ liệu logic, độc lập với sprite và hiệu ứng.


