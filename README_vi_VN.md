# sm64ex
Bản fork từ [sm64-port/sm64-port](https://github.com/sm64-port/sm64-port) với nhiều chức năng bổ sung. 

Hoan nghênh đóng góp dưới hình thức thông báo lỗi hoặc code, xin lưu ý **không đăng lên bất cứ thông tin có bản quyền nào của ROM gốc**.

Chạy `./extract_assets.py --clean && make clean` hay `make distclean` để xóa nội dung từ nguồn ROM gốc.

Xin đóng góp **đầu tiên** vào [nhánh nightly](https://github.com/sm64pc/sm64ex/tree/nightly/). Các chức năng mới sẽ được sáp nhập vào nhánh chính sau khi kiểm định đầy đủ.

## Chức năng mới

 * Menu để hiểu chỉnh nhiều công năng, vd hiệu chỉnh lại nút bấm.
 * (Không bắt buộc) Tải thông tin ngoài (hiện chỉ hỗ trợ texture và soundbanks), hỗ trợ các gói texture tùy chọn.
 * (Không bắt buộc) Chế độ chuột và camera analog (sử dụng [Puppycam](https://github.com/FazanaJ/puppycam)).
 * (Không bắt buộc) Hỗ trợ render với OpenGL1.3 cho các dòng máy tính cũ, và kèm luôn cả bộ GL2.1, D3D11 và D3D12 từ Emill's [n64-fast3d-engine](https://github.com/Emill/n64-fast3d-engine/).
 * Lựa chọn để tắt đi chức năng vẽ khoảng cách (drawing distances).
 * (Không bắt buộc) sửa model và texture (vd texture của khói).
 * Cho phép cắt đoạn giới thiệu của Peach và Lakitu với cờ `--skip-intro`
 * Menu hiệu chỉnh Cheat trong Options (bật lên với cờ `--cheats` hoặc ấn nút L ba lần trong menu tạm dừng).
 * Hỗ trợ tập tin lưu game cho cả little-endian và big-endian (có nghĩa bọn có thể dùng chung một save file cho bất cứ bản port hoặc emulator nào). Thêm vào đó định dạng text cho tập tin lưu cũng được hỗ trợ.

Thay đổi gần đây trong nhánh Nightly dời tập tin lưu game và hiệu chỉnh tới đường dẫn `%HOMEPATH%\AppData\Roaming\sm64pc` trên Windows và `$HOME/.local/share/sm64pc` trên Linux.
Đường dẫn có thể thay đổi với cờ `--savepath`.
Vd:
* `--savepath .` sẽ đọc các tập lưu game của đường dẫn hiện hành (lưu ý đường dẫn có thể không trùng với thư mục exe nhưng đa số là có trùng)
* `--savepath '!'` sẽ đọc các tập lưu game của đường dẫn có chứa các tập tin chạy game


## Xây dựng
Hướng dẫn cụ thể tại [wiki](https://github.com/sm64pc/sm64ex/wiki).

**Xin lưu ý, bạn cần phải có MXE trước khi biên dịch cho Windows trên Linux và WSL. Xem hướng dẫn chi tiết trong Wiki**
