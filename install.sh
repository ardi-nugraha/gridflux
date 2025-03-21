#!/bin/bash

# Detect user and computer name
USER_NAME=$(whoami)
COMPUTER_NAME=$(hostname)
INSTALL_DIR="/usr/local/bin"
SERVICE_FILE="/etc/systemd/system/gridflux.service"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
print_status() {
  echo -e "${GREEN}[*]${NC} $1"
}

print_warning() {
  echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
  echo -e "${RED}[x]${NC} $1"
}

print_info() {
  echo -e "${BLUE}[i]${NC} $1"
}

install_dependencies() {
  print_status "Detecting distribution and installing dependencies..."
  local dependencies="libx11-dev cmake gcc make pkg-config libgtk-3-dev libwnck-3-dev libxcb1-dev"

  if [ -f /etc/os-release ]; then
    . /etc/os-release
    case "$ID" in
    ubuntu | debian)
      print_info "Detected Debian-based distribution."
      sudo apt update
      for dep in $dependencies; do
        if dpkg-query -l "$dep" &>/dev/null; then
          print_info "$dep is already installed."
        else
          print_status "Installing $dep..."
          sudo apt install -y "$dep"
        fi
      done
      ;;
    rhel | fedora | centos | almalinux | rocky)
      print_info "Detected RHEL-based distribution."
      sudo dnf check-update || sudo yum check-update
      sudo dnf install -y gcc make pkg-config gtk3-devel libwnck3-devel libxcb-devel
      ;;
    arch | manjaro)
      print_info "Detected Arch-based distribution."
      sudo pacman -Syu --noconfirm
      sudo pacman -Sy --noconfirm gcc make pkg-config gtk3 libwnck3 xcb
      ;;
    *)
      print_error "Unsupported distribution: $ID"
      exit 1
      ;;
    esac
  else
    print_error "Unable to detect distribution. Ensure dependencies are installed manually."
    exit 1
  fi
}

build_and_install() {
  set -e

  SESSION=$(echo "$XDG_CURRENT_DESKTOP" | awk '{print toupper($0)}')

  case "$SESSION" in
  *KDE*) SESSION_MACRO="-DSESSION_KDE" ;;
  *GNOME*) SESSION_MACRO="-DSESSION_GNOME" ;;
  *XFCE*) SESSION_MACRO="-DSESSION_XFCE" ;;
  *LXQT*) SESSION_MACRO="-DSESSION_LXQT" ;;
  *MATE*) SESSION_MACRO="-DSESSION_MATE" ;;
  *) SESSION_MACRO="" ;;
  esac

  print_status "Detected session: $SESSION"
  print_status "Using compilation flag: $SESSION_MACRO"

  print_status "Building the project..."
  make clean
  make SESSION_MACRO="$SESSION_MACRO"

  print_status "Stopping existing gridflux service..."
  sudo systemctl stop gridflux.service 2>/dev/null || true

  print_status "Checking for existing gridflux process..."
  pkill gridflux 2>/dev/null || true

  print_info "Waiting for processes to stop..."
  sleep 2

  print_status "Installing gridflux to $INSTALL_DIR..."
  sudo install -m 755 gridflux "$INSTALL_DIR/" || {
    print_error "Failed to copy gridflux to $INSTALL_DIR"
    exit 1
  }

  print_status "Binary installation complete"
}

create_systemd_service() {
  print_status "Creating systemd service for gridflux..."
  if [ -f "$SERVICE_FILE" ]; then
    print_warning "Service already exists. Removing old service..."
    sudo systemctl stop gridflux.service 2>/dev/null || true
    sudo systemctl disable gridflux.service 2>/dev/null || true
    sudo rm "$SERVICE_FILE"
  fi

  # Get current display and xauthority
  CURRENT_DISPLAY="${DISPLAY:-:1}"
  CURRENT_XAUTHORITY="${XAUTHORITY:-$HOME/.Xauthority}"

  sudo bash -c "cat > $SERVICE_FILE" <<EOL
[Unit]
Description=Tilling Assistant 
After=graphical-session.target display-manager.service
PartOf=graphical-session.target

[Service]
Type=simple
ExecStart=$INSTALL_DIR/gridflux
User=$USER_NAME
Environment=DISPLAY=$CURRENT_DISPLAY
Environment=XAUTHORITY=$CURRENT_XAUTHORITY
Environment=XDG_SESSION_TYPE=x11
Environment=DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u)/bus
StandardOutput=journal
StandardError=journal
Restart=always
RestartSec=10

[Install]
WantedBy=default.target
EOL

  sudo systemctl daemon-reload
  sudo systemctl enable gridflux.service
  print_status "Service enabled successfully"

  # Check DISPLAY before starting
  print_info "Current DISPLAY value: $CURRENT_DISPLAY"
  print_info "Current XAUTHORITY: $CURRENT_XAUTHORITY"

  sudo systemctl start gridflux.service
  print_status "Service started successfully"

  # Check service status
  sleep 2
  systemctl status gridflux.service

  print_info "You can check logs with: journalctl -u gridflux.service -f"
}

main() {
  print_status "Starting installation of gridflux..."
  print_info "Installing as user: $USER_NAME"
  print_info "Computer name: $COMPUTER_NAME"

  install_dependencies || {
    print_error "Failed to install dependencies"
    exit 1
  }

  build_and_install || {
    print_error "Failed to build and install"
    exit 1
  }

  create_systemd_service || {
    print_error "Failed to create systemd service"
    exit 1
  }

  print_status "Installation complete! gridflux is set to run at login."
}

# Check if running as root
if [ "$EUID" -eq 0 ]; then
  print_error "Please do not run as root or with sudo"
  exit 1
fi

main
