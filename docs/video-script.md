# Kịch Bản Video Demonstration Eternal Siege (3–5 Phút)

> **Mục tiêu**: Video thuyết minh giới thiệu tổng quan đề tài, luật chơi, hệ thống Kingdom & va chạm chân thực, chiến đấu 4 wave, tính năng nâng cấp/hoàn tác, Boss chiến và tổng kết 4 nguyên lý OOP.

---

## Phân Đoạn & Nội Dung Thuyết Minh

### Phân đoạn 1: Giới thiệu Đề tài & Thành viên (0:00 - 0:35)
- **Hình ảnh**: Màn hình Intro hiển thị logo Eternal Siege và thông tin nhóm.
- **Lời thoại (Voiceover)**:
  > "Xin chào Thầy và các bạn! Đây là video demo đồ án Eternal Siege - trò chơi Real-Time Action RPG / Tower Defense 2D được nhóm phát triển bằng ngôn ngữ C++17 và thư viện SFML 3. Nhóm bao gồm 5 thành viên: Nguyễn Huỳnh Minh Hiếu - Trưởng nhóm phụ trách kiến trúc và core gameplay; Nghiêm Đình Thuận phụ trách cơ chế chiến đấu; Phan Huỳnh Minh Tuấn phụ trách giao diện UI; Nguyễn Huỳnh Quốc Ngữ phụ trách đồ họa và TextureManager; và Phạm Hoàng Xuân Vy phụ trách bản đồ sinh ngẫu nhiên."

### Phân đoạn 2: Main Menu & Điều khiển (0:35 - 1:05)
- **Hình ảnh**: Thao tác phím Escape để vào Menu, click "Bắt đầu", di chuyển nhân vật trong Kingdom.
- **Lời thoại (Voiceover)**:
  > "Tại Main Menu, người chơi có thể chọn 'Bắt đầu' để bước vào Vương quốc (Kingdom). Nhân vật di chuyển bằng phím WASD. Chúng ta có thể tương tác với Cổng Thành bằng phím E. Các phím tiện ích F4 để đổi thời tiết/ngày đêm, F6 để kiểm tra FPS và lượng cache texture."

### Phân đoạn 3: Kiểm thử Va chạm & Chiều sâu Kingdom (F3 Overlay) (1:05 - 2:00)
- **Hình ảnh**: Bật F3 Navigation Overlay (`F3`), di chuyển Player qua cổng thành, đi quanh gốc cây, fountain, vách đá catacomb và đi qua cầu.
- **Lời thoại (Voiceover)**:
  > "Bấm F3 để bật hệ thống F3 Collision Debug. Như Thầy thấy, va chạm của Player được tính chính xác dựa trên Foot Collider ở hai bàn chân (màu vàng), hoàn toàn độc lập với kích thước sprite. Player chỉ có thể di chuyển trên các vùng hợp lệ (Walkable regions màu xanh lá), bị chặn bởi thân cây, cột cổng, tường đá và bờ sông. Đặc biệt khi di chuyển qua cầu, Player nằm đúng giữa lớp lan can xa (Bridge Far) và lan can gần (Bridge Near), tạo cảm giác chiều sâu tự nhiên mà không bị nhảy Y-sort."

### Phân đoạn 4: Setup Đội hình & Màn Chiến đấu (2:00 - 3:00)
- **Hình ảnh**: Tiến vào Ruined Catacombs, chọn 4 vị trí Ally (Damian, Evangeline, Junior, Lucas) trên bản đồ tile, bấm R để bắt đầu Wave 1.
- **Lời thoại (Voiceover)**:
  > "Khi bước qua Cửa Cực vào Ruined Catacombs, trò chơi chuyển sang Setup State. Người chơi bố trí 4 đồng minh với các vai trò Đỡ đòn, Sát thương và Hỗ trợ. Khi trận đấu bắt đầu, các Ally tự động tìm quái vật gần nhất để tấn công. Player sử dụng Gậy Phép ngắm theo chuột, giữ chuột trái để tụ lực bắn đạn nặng Heavy Spirit Bolt, và bấm Q để tung Radiant Pulse hồi máu cho đồng đội."

### Phân đoạn 5: Nâng cấp, Hoàn tác & Thách thức Boss (3:00 - 4:15)
- **Hình ảnh**: Màn hình Intermission giữa các Wave, bấm 1/2/3 để nâng cấp, Backspace để Undo, đợt quái Wave 4 xuất hiện Boss tung Laser Beam và nộ chiến (Enrage Phase).
- **Lời thoại (Voiceover)**:
  > "Sau mỗi Wave, người chơi vào khoảng nghỉ Intermission để mua nâng cấp sát thương hoặc lượng máu bằng Gold. Nếu mua nhầm, phím Backspace cho phép Hoàn tác (Undo) giao dịch ngay lập tức. Ở Wave 4, Boss xuất hiện với các kỹ năng nguy hiểm như bắn Laser Beam quét rộng và nộ chiến tăng tốc độ đánh khi máu giảm thấp."

### Phân đoạn 6: Chiến thắng & Tổng kết OOP (4:15 - 4:45)
- **Hình ảnh**: Tiêu diệt Boss, màn hình Victory hiện ra với nút Chơi lại và Về menu.
- **Lời thoại (Voiceover)**:
  > "Tiêu diệt Boss mang lại Chiến thắng (Victory). Kiến trúc mã nguồn áp dụng triệt để 4 nguyên lý OOP: Đóng gói chỉ số private trong Entity/State; Kế thừa từ các lớp cơ sở State, Entity, Weapon; Đa hình trong việc update và render tập hợp đối tượng; và Trừu tượng hóa các module Map, WaveManager, SoundManager. Cảm ơn Thầy và các bạn đã theo dõi!"

---

## Danh Sách Thiết Bị & Thiết Lập Quay Video
- **Độ phân giải khuyến nghị**: 1920x1080 hoặc 1280x720, 60fps.
- **Công cụ quay**: OBS Studio / Xbox Game Bar.
- **Định dạng file xuất**: `MP4` H.264 / AAC Audio.
