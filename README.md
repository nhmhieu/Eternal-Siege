# Eternal Siege

Eternal Siege là game phòng thủ thời gian thực viết bằng C++17 và SFML 3.
Người chơi cùng bốn đồng minh phải vượt qua bốn đợt quái theo từng nhóm nhỏ;
các wave sau có Elite và đợt cuối có boss.

## Yêu cầu

- CMake 3.20 trở lên
- Trình biên dịch hỗ trợ C++17
- SFML 3 (Graphics, Window, System)

Ví dụ với MSYS2 UCRT64 trên Windows:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" `
  -DCMAKE_PREFIX_PATH=C:\msys64\ucrt64
cmake --build build -j 4
ctest --test-dir build --output-on-failure
.\build\my_game.exe
```

CMake tự chép thư mục `assets/` và các DLL runtime mà SFML khai báo vào thư
mục chứa executable sau khi build.

## Cách chơi

1. Có thể nhấn phím hoặc chuột để bỏ qua intro.
2. Chọn **BAT DAU** tại menu.
3. Chọn đúng bốn ô hợp lệ trên bản đồ, sau đó chọn **BAT DAU**.
4. Trong trận:
   - `W A S D`: di chuyển.
   - Chuột trái: bắn theo hướng con trỏ.
   - `P`: tạm dừng/tiếp tục.
   - Giữa các wave, dùng `1`, `2`, `3` để nâng sát thương, sinh lực hoặc tốc độ
     đánh; nhấn `Backspace` để hoàn tác lần mua gần nhất; nhấn `Enter` để bắt
     đầu wave kế tiếp.

Người chơi thua khi hết máu và thắng sau khi tiêu diệt boss ở wave 4.

## Cấu trúc

- `src/`, `include/`: mã nguồn và header.
- `assets/`: font và hình nhân vật.
- `tests/`: kiểm thử state machine, map, wave và boss.
- `docs/architecture.md`: sơ đồ lớp và sơ đồ trình tự.
- `build/`: kết quả build cục bộ, không đưa vào Git.

Thiết kế dùng các lớp trừu tượng `State`, `Entity`, `Weapon`; các lớp con triển
khai hành vi riêng qua hàm `virtual`. Quyền sở hữu entity, weapon và projectile
được quản lý bằng `std::unique_ptr`.
