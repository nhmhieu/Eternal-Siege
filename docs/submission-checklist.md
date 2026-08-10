# Danh Sách Kiểm Tra Đóng Gói & Nộp Bài (Submission Checklist)

> Document tổng hợp danh mục nộp bài đồ án **Eternal Siege** theo chuẩn Rubric.

---

## 1. Danh Mục Các Thành Phần Bắt Buộc

- [x] **Source code C++ đầy đủ & Biên dịch thành công**:
  - Mã nguồn trong các thư mục `include/`, `src/`, `assets/`.
  - Cấu hình biên dịch bằng `CMakeLists.txt`.
  - Kết quả biên dịch Release mode và chạy thành công trên môi trường Windows.
- [x] **Báo cáo kỹ thuật chi tiết (`docs/project-report.md`)**:
  - Giới thiệu game, mục tiêu, luật chơi.
  - Bảng phân tích chức năng hệ thống.
  - Sơ đồ lớp (Class Diagram) & Sơ đồ trình tự (Sequence Diagram).
  - Cấu trúc thư mục & phân tích 4 nguyên lý Hướng đối tượng (OOP).
  - Kết quả kiểm thử thủ công (Manual Testing) & Đánh giá thủ công (Visual QA).
  - Bảng phân công thành viên 5 người chính xác.
  - Hướng phát triển & Phụ lục tham khảo.
- [x] **Kịch bản Video thuyết minh (`docs/video-script.md`)**:
  - Kịch bản 3–5 phút giới thiệu thành viên, gameplay, Kingdom va chạm F3, nâng cấp/undo, Boss battle và tổng kết OOP.
- [x] **Báo cáo PDF nộp bài**:
  - Được biên dịch/chuyển đổi từ `docs/project-report.md` thành định dạng PDF sạch đẹp, phông chữ Tiếng Việt không lỗi.

---

## 2. Kiểm Tra Chặn Nộp Bài (Submission Blockers)

- [!IMPORTANT]
  **SUBMISSION BLOCKER - THÀNH VIÊN NHÓM (GROUP SIZE BLOCKER)**:
  - **Trạng thái**: Nhóm có **5 thành viên** (Hiếu, Thuận, Tuấn, Ngữ, Vy).
  - **Yêu cầu Rubric**: Cho phép 3–4 sinh viên.
  - **Hành động bắt buộc**: Phải xin ý kiến xác nhận/chấp thuận từ Giảng viên trước khi tạo archive ZIP cuối cùng. Không tự ý xóa tên thành viên hoặc bịa thông tin.

- [!NOTE]
  **VIDEO MP4 NỘP BÀI (FINAL VIDEO MP4 BLOCKER)**:
  - **Trạng thái**: Kịch bản video `docs/video-script.md` đã hoàn tất.
  - **Hành động bắt buộc**: Cần quay video màn hình thực tế (3–5 phút) kèm giọng thuyết minh theo đúng kịch bản trước khi nộp.

---

## 3. Cấu Trúc Đóng Gói (Package Structure)

Khi đạt đủ tất cả các điều kiện trên, gói nộp bài zip chỉ chứa:
```text
Eternal-Siege-Submission/
├── Báo_Cáo_Đồ_Án_Eternal_Siege.pdf
├── Demo_Video_Eternal_Siege.mp4 (hoặc file link)
├── CMakeLists.txt
├── README.md
├── BUILD_WINDOWS.bat
├── RUN_GAME.bat
├── include/
├── src/
├── assets/
└── docs/
```

**Các thư mục / file TUYỆT ĐỐI KHÔNG đưa vào gói zip**:
- `.git/`, `.vs/`, `.opencode/`
- `build*` (toàn bộ các thư mục build)
- `tmp/`, `*.obj`, `*.exe`, `*.log`, `build_output.txt`
