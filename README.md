# gridflux - A Minimalist Tiling Assistant

gridflux is a lightweight and minimalist tiling assistant designed to arrange and manage windows efficiently. It supports automatic window tiling, workspace organization.



https://github.com/user-attachments/assets/045ea8e5-f5c9-42e8-80fd-e89794850010



## Features

- **Automatic Window Tiling**: Dynamically arranges windows in a tiled layout.
- **Workspace Management**: Supports multiple workspaces for better productivity.
- **Lightweight**: Built with GTK, Libwnck, and XCB to ensure minimal resource usage.
- **Customizable Layouts**: Supports horizontal and vertical splits.

## Supported Platforms

- **Linux Only** (Tested on Debian, Ubuntu, Fedora, Arch Linux, and Alpine Linux)

## Installation

### 1. Install Dependencies

Before installing gridflux, ensure you have the required dependencies.

Run the following command based on your Linux distribution:

- **Debian/Ubuntu**:
  ```sh
  sudo apt update && sudo apt install -y gcc make pkg-config libgtk-3-dev libwnck-3-dev libxcb1-dev
  ```
- **Fedora**:
  ```sh
  sudo dnf install -y gcc make pkg-config gtk3-devel libwnck3-devel libxcb-devel
  ```
- **Alpine Linux**:
  ```sh
  sudo apk add --no-cache gcc make pkgconf gtk+3.0-dev libwnck-dev libxcb-dev
  ```

### 2. Build and Install gridflux

```sh
make
sudo make install
```

### 3. Enable gridflux as a Systemd User Service

```sh
systemctl --user enable gridflux.service
systemctl --user start gridflux.service
```

gridflux will now run automatically upon user login.

## Uninstallation

To remove gridflux from your system, run:

```sh
make uninstall
```

This will stop and disable the service, remove the systemd unit, and delete the binary.

## Usage

Once installed, gridflux will automatically arrange windows in a tiled layout. The second opened window in a workspace will have a **10-pixel height gap** to enhance visibility, while subsequent windows follow normal tiling behavior.

## Contributing

Feel free to contribute by submitting issues, pull requests, or feature suggestions.

## License

gridflux is released under the **GPLv3** license.

---

For more information and updates, visit the project repository.

**Author:** Ardi Nugraha  
**Contact:** [0x4rd1@gmail.com]
