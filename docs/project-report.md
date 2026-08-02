# Báo cáo đồ án Eternal Siege

> Đây là bản thảo kỹ thuật dựa trên mã nguồn hiện tại. Trước khi xuất PDF để
> nộp, nhóm phải bổ sung thông tin thành viên, phân công, ảnh chụp màn hình và
> kết quả playtest thủ công. Không nên nộp nguyên bản có placeholder.

## 1. Giới thiệu đề tài

### 1.1. Ý tưởng

Eternal Siege là trò chơi phòng thủ thời gian thực 2D. Người chơi điều khiển
một pháp sư hỗ trợ dùng Spirit Staff và bố trí bốn đồng minh trước khi trận đấu bắt đầu. Đội hình phải
vượt qua bốn wave gồm Normal, Elite và Boss.

### 1.2. Mục tiêu và luật chơi

- Người chơi di chuyển bằng WASD và bắn bằng chuột trái.
- Phím Q kích hoạt Radiant Pulse để hồi máu cho đồng minh hợp lệ; P tạm dừng,
  H mở hướng dẫn và M bật/tắt âm thanh.
- Đồng minh tự chọn Monster sống ở gần để tấn công.
- Batch sau chỉ xuất hiện khi batch hiện tại đã bị tiêu diệt.
- Sau mỗi wave, người chơi có thể dùng gold để nâng cấp.
- Player hết HP dẫn đến Game Over.
- Tiêu diệt Boss cuối Wave 4 dẫn đến chiến thắng.

## 2. Phân tích hệ thống

### 2.1. Yêu cầu chức năng

| Chức năng | Đầu vào | Xử lý | Kết quả |
|---|---|---|---|
| Menu | Click Bắt đầu/Thoát | `MenuState` kiểm tra vùng nút | Sang Setup hoặc đóng game |
| Bố trí Ally | Click các tile hợp lệ | `Map` thêm/bỏ vị trí | Bốn Ally được tạo khi bắt đầu |
| Điều khiển Player | WASD, chuột trái, Q | Movement collision, Spirit Staff/Spirit Bolt và Radiant Pulse | Player di chuyển, bắn và hỗ trợ Ally |
| Wave | `deltaTime`, số quái sống | `WaveManager` chạy batch | Normal/Elite/Boss xuất hiện |
| Combat | Hitbox và team | `CombatManager` áp damage | HP giảm, entity chết logic |
| Nâng cấp | Phím 1/2/3 | `UpgradeManager` trừ gold | Tăng damage/HP/fire rate |
| Hoàn tác | Backspace | Lấy giao dịch cuối | Hoàn chỉ số và gold |
| Kết thúc | HP Player hoặc Wave 4 | `GameplayState` kiểm tra | Game Over hoặc Win |

### 2.2. Mô hình lớp

Sơ đồ lớp và giải thích ownership nằm trong
[`architecture.md`](architecture.md).

## 3. Thiết kế hệ thống

### 3.1. UML

- Sơ đồ lớp: xem `docs/architecture.md`.
- Sơ đồ trình tự người chơi bắn Monster: xem `docs/architecture.md`.

### 3.2. Thiết kế giao diện

- Intro: giới thiệu nhóm và chuyển sang menu.
- Menu: tên game, nút Bắt đầu và Thoát.
- Setup: bản đồ tile và vùng chọn bốn Ally.
- Gameplay: bản đồ bên trái; HUD bên phải hiển thị HP, gold, wave, số quái,
  nâng cấp và hướng dẫn phím.
- Win/Game Over: thông báo kết quả, nút Chơi lại và Về menu.

> Cần chèn ảnh chụp thực tế của năm nhóm màn hình trên trước khi nộp.

## 4. Cài đặt

### 4.1. Cấu trúc thư mục

- `include/`: khai báo lớp.
- `src/`: cài đặt game.
- `assets/`: font và texture nhân vật.
- `tests/`: test logic và state machine.
- `docs/`: tài liệu kiến trúc và báo cáo.
- `build/`: kết quả sinh bởi CMake, không đưa vào Git.

### 4.2. Các module chính

- State: `IntroState`, `MenuState`, `SetupState`, `GameplayState`,
  `WinState`, `GameOverState`.
- Entity: `Player`, `Ally`, `Monster`, `Elite`, `Boss`.
- Combat: `Weapon`, `Bow`, `Sword`, `Wand`, `SpiritStaff`, `Projectiles`,
  `MagicBolt`, `SpiritBolt`, `CombatManager`.
- Game rules: `WaveManager`, `UpgradeManager`.
- Support/Boss: `RadiantPulse`, Ally Skills, `BossHealthBar`,
  `BossEnrageNotice`, Boss Beam và `GameplayTransitionGate`.
- An toàn gameplay: `Map`, `EntityCollision`, `EntityLifecycle`.
- Giao diện/tài nguyên: `HUD`, `Effects`, `TextureManager`, `AssetLocator`,
  `AudioManager`.

### 4.3. Vận dụng OOP

- Đóng gói: HP, cooldown, target và dữ liệu wave nằm trong lớp quản lý.
- Kế thừa: state kế thừa `State`; nhân vật kế thừa `Entity`; Elite/Boss kế
  thừa `Monster`; Bow/Sword kế thừa `Weapon`.
- Đa hình: game gọi `update`, `draw` và `triggerAction` qua kiểu cơ sở.
- Trừu tượng hóa: `State`, `Entity` và `Weapon` quy định giao diện chung.
- RAII: owner dùng `std::unique_ptr`, không gọi `delete` thủ công.

## 5. Kiểm thử và đánh giá

### 5.1. Kiểm thử tự động

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build -j 4
ctest --test-dir build --output-on-failure
```

Release candidate đăng ký 12 bộ test: `state_machine_tests`, `core_logic_tests`,
`ally_combat_tests`, `combat_regression_tests`, `ally_skill_tests`,
`presentation_tests`, `gameplay_balance_tests`, `support_presentation_tests`,
`tutorial_tests`, `intermission_tests`, `boss_enrage_tests` và
`full_game_integration_tests`. Các test bao phủ state transition, map/collision,
projectile safety, Ally Skill, balance, UI logic, pause, upgrade/undo, Boss
Beam/Enrage, reward và restart.

### 5.2. Kiểm thử thủ công cần ghi vào báo cáo

- Menu Bắt đầu/Thoát.
- Chọn và bỏ chọn Ally trong Setup.
- Di chuyển/bắn, pause/resume.
- Chuyển batch, intermission, mua và undo nâng cấp.
- Game Over, Boss, Win, Chơi lại và Về menu.

> Cần điền ngày kiểm thử, người kiểm thử, kết quả và ảnh minh họa thực tế.

## 6. Phân công công việc nhóm

| Thành viên | Mã số sinh viên | Vai trò | Công việc cụ thể |
|---|---|---|---|
| [Điền tên] | [Điền MSSV] | [Vai trò] | [Công việc] |
| [Điền tên] | [Điền MSSV] | [Vai trò] | [Công việc] |
| [Điền tên] | [Điền MSSV] | [Vai trò] | [Công việc] |
| [Điền nếu nhóm có 4 người] | [Điền MSSV] | [Vai trò] | [Công việc] |

## 7. Hướng phát triển tương lai

- Bổ sung thêm map hoặc kiểu Monster mà không thay đổi giao diện `Entity`.
- Tách cấu hình gameplay ra file dữ liệu để dễ cân bằng.
- Hoàn thiện attribution/license cho mọi asset trước khi phân phối công khai.
- Bổ sung automation cho luồng giao diện khi có test harness cửa sổ phù hợp.

## 8. Phụ lục và tài liệu tham khảo

- Mã nguồn C++17 trong repository Eternal Siege.
- SFML 3 documentation.
- CMake documentation.
- Tài liệu yêu cầu đồ án lớp 25C08.
