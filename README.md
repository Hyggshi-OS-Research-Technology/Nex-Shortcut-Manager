# ⚡ Nex Shortcut Manager (NSM) (C++ + Qt6)

Ứng dụng quản lý và tạo lối tắt (**Nex Shortcut Manager — NSM**) chuyên nghiệp dành cho Linux, xây dựng bằng **C++17** và **Qt6**.

---

## 🐧 Các phiên bản Ubuntu & Linux hỗ trợ

| Hệ điều hành | Trạng thái | Yêu cầu Qt | Ghi chú |
| :--- | :---: | :---: | :--- |
| **Ubuntu 24.04 LTS** (Noble Numbat) | 🟢 Hỗ trợ đầy đủ | Qt 6.5+ | Có sẵn trong kho chính thức |
| **Ubuntu 23.10 / 23.04** | 🟢 Hỗ trợ đầy đủ | Qt 6.4+ | Có sẵn trong kho chính thức |
| **Ubuntu 22.04 LTS** (Jammy Jellyfish) | 🟢 Hỗ trợ đầy đủ | Qt 6.2+ | Cài qua `qt6-base-dev` |
| **Ubuntu 20.04 LTS** (Focal Fossa) | 🟡 Hỗ trợ (PPA) | Qt 6.x | Cần cài Qt6 qua PPA hoặc Qt Online Installer |
| **Debian 12** (Bookworm) | 🟢 Hỗ trợ đầy đủ | Qt 6.4+ | Hoàn toàn tương thích |
| **Linux Mint 21 / 22** | 🟢 Hỗ trợ đầy đủ | Qt 6.x | Dựa trên Ubuntu 22.04 / 24.04 |
| **Pop!_OS 22.04 / 24.04** | 🟢 Hỗ trợ đầy đủ | Qt 6.x | Hỗ trợ GNOME & COSMIC Desktop |
| **Zorin OS 16 / 17** | 🟢 Hỗ trợ đầy đủ | Qt 6.x | Hoàn toàn tương thích |
| **Fedora 38+ / Arch Linux** | 🟢 Hỗ trợ đầy đủ | Qt 6.x | Build trực tiếp qua CMake |

---

## ✨ Tính năng chính

| Icon | Tính năng | Chi tiết |
| :---: | :--- | :--- |
| 📦 | **Tự quét ứng dụng** | Tự động quét toàn bộ `/usr/share/applications` và `~/.local/share/applications` |
| 🖥️ | **Add to Desktop** | Thêm shortcut ra màn hình Desktop (`~/Desktop`), tự phân quyền thực thi (`chmod +x`) và đặt metadata `trusted` (GIO/GNOME/KDE) |
| 🗑️ | **Remove shortcut** | Xoá shortcut trực tiếp khỏi Desktop an toàn |
| 🔄 | **Refresh desktop** | Làm mới danh sách và cập nhật Desktop Environment tự động |
| ✏️ | **Đổi tên shortcut** | Hỗ trợ đổi tên nhanh hoặc đổi tên đầy đủ trong file `.desktop` |
| 🖼️ | **Đổi icon trực quan** | Bộ chọn Icon (Icon Picker Dialog) dạng lưới với thanh tìm kiếm theo tên và duyệt file ảnh tùy chọn |
| 📁 | **Shortcut tới Folder / File** | Tạo nhanh lối tắt mở thư mục hoặc file bất kỳ qua `xdg-open` |
| ⚙️ | **Chỉnh sửa toàn diện** | Tuỳ chỉnh `Name`, `Exec`, `Icon`, `Categories`, `Comment`, `Path` (Working Directory), `Terminal` mode |
| ⭐ | **Ghim ứng dụng (Pin)** | Ghim các app thường dùng lên danh sách yêu thích, lưu qua `QSettings` |
| 🔧 | **Tạo custom shortcut** | Trình tạo shortcut tự do bằng giao diện đồ hoạ thân thiện |
| 🛡️ | **Kiểm tra hợp lệ (.desktop validator)** | Tự động kiểm tra cú pháp và các trường bắt buộc theo chuẩn Freedesktop Desktop Entry Specification |
| 📥 / 📤 | **Import / Export JSON** | Sao lưu danh sách shortcut ra file JSON và khôi phục nhanh chóng |

---

## 🛠️ Cài đặt & Sử dụng

### Cách 1: Cài đặt nhanh bằng gói `.deb` (Khuyên dùng)
Tải file `shortcut-manager_1.0.0_amd64.deb` từ mục Releases trên GitHub và chạy:
```bash
sudo dpkg -i shortcut-manager_1.0.0_amd64.deb
sudo apt-get install -f # Tự động sửa nếu thiếu gói phụ thuộc
```

Sau khi cài đặt, bạn có thể mở ứng dụng trực tiếp từ App Menu hoặc gõ lệnh:
```bash
ShortcutManager
```

---

### Cách 2: Tự biên dịch từ mã nguồn (Build from source)

#### 1. Cài đặt các gói phụ thuộc:
- **Trên Ubuntu 24.04 / 22.04 & Debian 12:**
  ```bash
  sudo apt update
  sudo apt install -y build-essential cmake qt6-base-dev qt6-base-dev-tools libgl1-mesa-dev
  ```

- **Trên Fedora:**
  ```bash
  sudo dnf install -y gcc-c++ cmake qt6-qtbase-devel
  ```

- **Trên Arch Linux / Manjaro:**
  ```bash
  sudo pacman -S --needed base-devel cmake qt6-base
  ```

#### 2. Biên dịch dự án:
```bash
cd "Shortcut Manager"
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

#### 3. Chạy ứng dụng:
```bash
./ShortcutManager
```

#### 4. Đóng gói `.deb` cục bộ:
```bash
cpack -G DEB
```

---

## 📁 Cấu trúc thư mục mã nguồn

```
Shortcut Manager/
├── CMakeLists.txt              # CMake configuration & CPack packaging
├── README.md                   # Tài liệu hướng dẫn
├── .github/
│   └── workflows/
│       ├── build.yml           # GitHub Actions build & test
│       └── deb.yml             # GitHub Actions đóng gói file .deb & Release
├── resources/
│   ├── resources.qrc           # Qt Resource file
│   └── shortcut-manager.desktop# Desktop Entry cho ứng dụng
└── src/
    ├── main.cpp                # App entry point
    ├── mainwindow.h / .cpp     # Giao diện chính, Sidebar, Table, Context Menu & Actions
    ├── desktopentry.h / .cpp   # Model đọc/ghi/validate chuẩn .desktop
    ├── desktopscanner.h / .cpp # Bộ quét thư mục ứng dụng Linux
    ├── shortcutmanager.h / .cpp# Xử lý Desktop file, permissions, pinned apps
    ├── applistmodel.h / .cpp   # QAbstractTableModel cho danh sách ứng dụng
    ├── editdialog.h / .cpp     # Dialog chỉnh sửa thông tin shortcut
    ├── customshortcutdialog.h / .cpp # Dialog tạo shortcut tuỳ chỉnh
    ├── iconpickerdialog.h / .cpp     # Dialog duyệt và chọn icon
    └── importexportmanager.h / .cpp  # Xuất / nhập dữ liệu JSON
```
