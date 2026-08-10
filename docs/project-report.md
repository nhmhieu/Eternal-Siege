# Báo Cáo Đồ Án Eternal Siege

## 1. Giới thiệu đề tài

### 1.1. Ý tưởng trò chơi
**Eternal Siege** là trò chơi phòng thủ tháp / nhập vai hành động 2D thời gian thực (Real-Time Action RPG / Tower Defense) phát triển bằng ngôn ngữ C++17 và thư viện đồ họa SFML 3. Người chơi hóa thân thành một pháp sư bảo vệ vương quốc, di chuyển tự do trong khu vực vương quốc (**Kingdom**) để nhận nhiệm vụ, mở cổng thành và tiến vào khu vực chiến đấu (**Ruined Catacombs**). Trong màn chiến đấu, người chơi điều khiển nhân vật sử dụng Gậy Phép (Spirit Staff), bắn đạn linh hồn (Spirit Bolt / Heavy Spirit Bolt) và dùng kỹ năng hồi máu (Radiant Pulse) để hỗ trợ 4 đồng minh (**Damian**, **Evangeline**, **Junior**, **Lucas**) đẩy lùi 4 đợt tấn công của quái vật.

### 1.2. Mục tiêu và luật chơi
- **Di chuyển & Thao tác**:
  - Trong Kingdom: Di chuyển bằng phím `WASD`, tương tác với Cổng Thành hoặc Cửa Cực bằng phím `E`, bật/tắt hiển thị va chạm F3 (`F3 Navigation Overlay`), đổi buổi trong ngày (`F4`), đổi thời tiết (`F5`), hiển thị hiệu năng (`F6`).
  - Trong màn chiến đấu: Di chuyển bằng `WASD`, ngắm và bắn bằng Chuột trái, giữ Chuột trái để tích lực bắn đạn nặng (`Heavy Spirit Bolt`), phím `Shift` hoặc Chuột phải (`RMB`) để lướt né đạn (`Dash`), phím `Q` để tung kỹ năng hồi máu (`Radiant Pulse` hồi máu cho các đồng minh Ally trong phạm vi). Tạm dừng bằng `P`, xem hướng dẫn bằng `R`, bật/tắt âm thanh bằng `M`.
- **Đồng minh & Đội hình**: Trước khi bắt đầu chiến đấu (Setup State), người chơi bố trí 4 vị trí đồng minh trên bản đồ. Các đồng minh tự động tìm kiếm quái vật gần nhất trong tầm đánh để tấn công.
- **Tiến trình Wave**: Màn chiến đấu gồm 4 Wave quái vật (Normal Monster, Elite Monster, Boss). Mỗi đợt có nhiều batch quái vật, batch tiếp theo chỉ xuất hiện khi batch hiện tại bị tiêu diệt. Sau mỗi wave có khoảng thời gian nghỉ (**Intermission**), người chơi dùng Gold tích lũy để nâng cấp chỉ số (Sát thương, HP, Tốc độ đánh) hoặc hoàn tác (**Undo**).
- **Điều kiện Thắng / Thua**:
  - **Thua (Game Over)**: HP của Player giảm về 0.
  - **Thắng (Victory)**: Tiêu diệt Boss ở cuối Wave 4.

---

## 2. Phân tích chức năng hệ thống

### 2.1. Bảng phân tích chức năng

| Chức năng | Đầu vào (Input) | Xử lý (Processing) | Kết quả (Output) |
|---|---|---|---|
| **Chuyển State / Menu** | Mouse click trên nút Menu / phím Escape | `StateMachine` chuyển giữa `IntroState`, `MenuState`, `KingdomState`, `SetupState`, `GameplayState`, `WinState`, `GameOverState` | Màn hình tương ứng được render |
| **Di chuyển Kingdom & Va chạm** | Phím `WASD` | `KingdomMap::resolveMovementWithActors` kiểm tra vùng đi được (walkable regions), va chạm footprint của cổng, gốc cây, công trình, bờ sông, lan can cầu và chân NPC | Player di chuyển mượt mà, trượt dọc vật cản, tự chuyển Idle khi bị chặn hoàn toàn |
| **Bố trí Ally (Setup)** | Click chọn Hero Card, click tile trên map, phím `R` | `AllyPlacementModel` xác thực vị trí tile, gán slot cho từng `AllyType` | 4 Ally được khởi tạo đúng vị trí khi bắt đầu trận |
| **Tấn công & Tích lực** | Click / Giữ Chuột trái | `Player` tính vector ngắm, bắn `SpiritBolt` hoặc nạp năng lượng sinh `HeavySpiritBolt` kèm hiệu ứng màn hình | Đạn bay về phía mục tiêu, gây sát thương lên quái vật |
| **Kỹ năng Hồi máu (Radiant Pulse)** | Phím `Q` | `RadiantPulse` kiểm tra cooldown và tầm ảnh hưởng, hồi HP cho các Ally trong vùng | Vòng sóng ánh sáng lan tỏa, HP các Ally thân thiện tăng |
| **Xử lý Chiến đấu & Sát thương** | Vị trí đạn & hitbox nhân vật | `CombatManager` kiểm tra va chạm hitbox giữa đạn/đòn đánh và Entity khác Team | Giảm HP target, kích hoạt hurt flash, xóa entity khi HP <= 0 |
| **Wave Quái vật & Boss** | `deltaTime`, số quái sống | `WaveManager` quản lý timer spawn batch, kích hoạt Boss Enrage và kỹ năng Boss Beam | Quái xuất hiện đúng đợt, hiển thị thanh HP Boss và thông báo nộ chiến |
| **Nâng cấp & Hoàn tác** | Phím `1`, `2`, `3` / `Backspace` | `UpgradeManager` trừ Gold, tăng chỉ số nhân vật; lưu lịch sử giao dịch để hoàn tác | Chỉ số chiến đấu tăng, hiển thị thông báo trên HUD |

---



## 4. Cài đặt hệ thống & Bốn nguyên lý OOP

### 4.1. Vận dụng 4 nguyên lý Hướng đối tượng (OOP)
1. **Tính Đóng gói (Encapsulation)**:
   - Tất cả thuộc tính quan trọng như máu (`health`), chỉ số tấn công, tọa độ, danh sách collider đều được đặt ở phạm vi `private`/`protected`.
   - Quyền truy cập và chỉnh sửa được thực hiện qua các getter/setter hợp lệ (`getPosition()`, `getFootCollider()`, `takeDamage()`, `canStandAt()`).
2. **Tính Kế thừa (Inheritance)**:
   - Lớp `State` làm lớp cơ sở cho tất cả trạng thái game (`KingdomState`, `GameplayState`, `SetupState`, ...).
   - Lớp `Entity` làm lớp cơ sở cho toàn bộ đối tượng động (`Player`, `Ally`, `Monster`, `Elite`, `Boss`).
   - Lớp `Weapon` làm lớp cơ sở cho các vũ khí (`Bow`, `Sword`, `Wand`, `SpiritStaff`).
3. **Tính Đa hình (Polymorphic Behavior)**:
   - Các hàm virtual như `update()`, `draw()`, `takeDamage()`, `handleInput()` được ghi đè (override) ở các lớp con. Lớp quản lý `StateMachine` và `CombatManager` làm việc hoàn toàn qua con trỏ/tham chiếu lớp cơ sở.
4. **Tính Trừu tượng (Abstraction)**:
   - Các module cung cấp giao diện đơn giản, che giấu độ phức tạp bên trong. Ví dụ: `KingdomMap::resolveMovementWithActors()` giấu toàn bộ thuật toán kiểm tra 9 probe điểm và va chạm đa giác cầu; `WaveManager` giấu chi tiết spawn ngẫu nhiên và timer batch.

---

## 5. Kiểm thử và Báo cáo Đánh giá

### 5.1. Kết quả kiểm thử thủ công (Manual Functional Verification & Visual QA)
Đã thực hiện kiểm thử thủ công toàn bộ các tính năng của trò chơi cho bản phát hành chính thức:
- **Intro & Presentation**: Màn hình Intro hiển thị đúng 5 frame story theo tiến trình, Frame 1 hiển thị credit `Created by Ngo0Group` với khung chứa tối sang trọng và hiệu ứng xuất hiện mượt mà.
- **Main Menu**: Menu phản hồi chính xác phím Escape, click nút Start / Exit, âm thanh SFX ui_click.
- **Vương quốc Asterfall (Kingdom Hub)**: Nhân vật di chuyển WASD mượt mà kèm hiệu ứng trượt góc tường (corner sliding), bám đất với bóng chân đôi (dual-layer grounding shadow); chỉ đường `V` định hướng đúng Cổng thành và Cửa Cực; phím `E` tương tác mở cổng; phím `F3`-`F6` hỗ trợ debug.
- **Bố trí Đội hình (Setup State)**: Bố trí đủ 4 Ally (Damian, Evangeline, Junior, Lucas) trên bản đồ tile; phím `R` reset đội hình; chuột phải thu hồi Ally; xác nhận bắt đầu trận.
- **Procedural Combat Map**: Sinh địa hình ngẫu nhiên liên thông bằng thuật toán Drunkard Walk; tự động phân bổ 4 vị trí xuất hiện quái hợp lệ.
- **Chiến đấu & Kỹ năng**: `Spirit Bolt` (LMB tap), `Heavy Spirit Bolt` (LMB hold), `Dash` (`Shift`/RMB), `Radiant Pulse` (`Q` hồi máu cho các Ally trong vùng).
- **Wave & Intermission**: Quản lý 4 wave dồn dập, khoảng nghỉ Intermission cho phép mua nâng cấp (`1`/`2`/`3`), hoàn tác (`Backspace`), xem hướng dẫn (`R`), và sang wave tiếp (`Enter`).
- **Trận đấu Boss & Kết thúc**: Boss chuyển Phase 2 Beam Laser ở 50% HP, kích hoạt Enrage ở 25% HP; màn hình Victory và GameOver chuyển đổi đúng luồng.

### 5.2. Kết quả Đánh giá Thủ công (Manual Visual QA)
- **Cổng thành**: Đi qua chính giữa cổng khi mở thành công; hai cột đá và tường thành chặn chính xác khi đóng hoặc khi đâm vào cột.
- **Cây cối & Công trình**: Thân cây/gốc cây chặn chân Player; tán cây che đè lên Player khi đi phía sau. Công trình (Tavern, Guildhall, Cottage, Fountain, Catacomb) chặn đúng footprint móng.
- **Cầu & Nước**: Player chỉ di chuyển trên mặt cầu (Bridge Deck). Đi sau lan can xa (`BridgeFar`) và đi trước lan can gần (`BridgeNear`) tạo độ sâu tự nhiên, chân không lơ lửng, shadow nằm đúng mặt cầu. Nước hai bên chặn tuyệt đối.
- **Phản hồi di chuyển**: Khi ép vào tường/vật cản nhưng tiếp tục giữ phím `WASD`, Player trượt dọc cạnh vật thể mượt mà hoặc chuyển về trạng thái Idle, không xảy ra hiện tượng chạy tại chỗ.

---

## 6. Phân công công việc nhóm

| Thành viên | Mã số sinh viên | Vai trò và nhiệm vụ cụ thể |
|---|---:|---|
| **Nguyễn Huỳnh Minh Hiếu** | 25127331 | **Trưởng nhóm**; phân tích và tổ chức kiến trúc; thiết kế HUD và các màn hình/trạng thái giao diện; phát triển và tích hợp core gameplay như combat, ally, wave, upgrade và state flow; merge/review các module; xử lý collision, cleanup và lỗi tích hợp; tích hợp và hoàn thiện khu vực Kingdom; xây dựng/duy trì test; kiểm tra build; tổng hợp tài liệu và báo cáo. |
| **Nghiêm Đình Thuận** | 25127152 | Phát triển gameplay màn chiến đấu; tham gia xử lý các cơ chế Player, quái vật và tương tác chiến đấu theo phần code thực tế của thành viên. |
| **Phan Huỳnh Minh Tuấn** | 25127168 | Thiết kế và triển khai UI; menu và  tích hợp phần hiển thị vào game. |
| **Nguyễn Huỳnh Quốc Ngữ** | 25127103 | Phụ trách đồ họa và quản lý texture; xây dựng/hoàn thiện `TextureManager`; chuẩn bị và tích hợp texture vào các đối tượng trong game. |
| **Phạm Hoàng Xuân Vy** | 25127173 | Thiết kế map màn chiến đấu; cài đặt sinh map bằng thuật toán Drunkard Walk; tham gia kiểm tra bố cục và khả năng di chuyển trên map trong trận. |



---

## 7. Hướng phát triển tương lai
1. Bổ sung thêm các chương bản đồ mới và hệ thống nhân vật / quái vật đa dạng nhờ kiến trúc hướng đối tượng mở rộng dễ dàng.
2. Tách toàn bộ chỉ số cân bằng sang file cấu hình JSON/YAML external để tinh chỉnh không cần biên dịch lại.
3. Hỗ trợ chơi đa người chơi (Co-op Multiplayer) thông qua SFML Network.

---

## 8. Phụ lục và tài liệu tham khảo
- Mã nguồn C++17 trong repository Eternal Siege.
- Thư viện SFML 3 (Simple and Fast Multimedia Library).
- Công cụ biên dịch CMake 3.21+ và GCC / Clang / MSVC.
