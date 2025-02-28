#!/bin/bash

## =================================================================
## HMSSQL PACKAGE INSTALLATION
##
## This script will install all the packages that are needed to
## build and run the DBMS.
##
## Supported environments:
##  * Ubuntu 22.04 (x86-64)
##  * Fedora (x86-64)
##  * Arch Linux (x86-64)
##  * macOS 13 Ventura (x86-64 or ARM)
## =================================================================

main() {
  set -o errexit

  if [ "$1" == "-y" ] 
  then 
      install
  else
      echo "PACKAGES WILL BE INSTALLED. THIS MAY BREAK YOUR EXISTING TOOLCHAIN."
      echo "YOU ACCEPT ALL RESPONSIBILITY BY PROCEEDING."
      read -p "Proceed? [y/N] : " yn
  
      case $yn in
          Y|y) install;;
          *) ;;
      esac
  fi

  echo "Script complete."
}

install() {
  set -x
  UNAME=$(uname | tr "[:lower:]" "[:upper:]" )

  case $UNAME in
    LINUX)
        install_linux ;;
    Darwin)
        give_up ;;
    WIN*)
        install_win ;;
    *)
        give_up ;;
  esac
}

give_up() {
  set +x
  echo "Unsupported distribution '$UNAME'"
  echo "Please contact our support team for additional help."
  echo "Be sure to include the contents of this message."
  echo "Platform: $(uname -a)"
  echo
  echo
  exit 1
}


### test it!!!!!
install_win() {
  echo "Setting up Windows environment for HMSSQL..."
  
  # Check if running in WSL
  if grep -q Microsoft /proc/version; then
    echo "Detected WSL environment."
    echo "Installing dependencies within WSL..."
    install_ubuntu
    return
  fi

  # Check if PowerShell is available
  if ! command -v powershell.exe &> /dev/null; then
    echo "PowerShell not found. Please run this script within a PowerShell environment."
    give_up
  fi

  # Check if Chocolatey is installed
  echo "Checking for Chocolatey package manager..."
  if ! powershell.exe -Command "if (!(Get-Command choco.exe -ErrorAction SilentlyContinue)) { exit 1 }"; then
    echo "Installing Chocolatey package manager..."
    powershell.exe -Command "Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"
  fi

  echo "Installing required packages with Chocolatey..."
  powershell.exe -Command "choco install -y git cmake --installargs 'ADD_CMAKE_TO_PATH=System'"
  powershell.exe -Command "choco install -y visualstudio2022-workload-vctools"
  powershell.exe -Command "choco install -y llvm"
  
  echo "Setting up environment variables..."
  # Add MSVC and LLVM to the PATH if needed
  powershell.exe -Command "[Environment]::SetEnvironmentVariable('Path', [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';C:\Program Files\LLVM\bin', 'Machine')"
  
  echo "Windows environment setup complete."
  echo "Please restart your shell or terminal to ensure all changes take effect."
}

install_linux() {
  if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
  else
    give_up
  fi

  case $OS in
    ubuntu)
      install_ubuntu ;;
    fedora)
      install_fedora ;;
    arch)
      install_arch ;;
    *)
      give_up ;;
  esac
}

install_ubuntu() {
  # Update apt-get.
  apt-get -y update
  # Install packages.
  apt-get -y install \
      build-essential \
      clang-14 \
      clang-format-14 \
      clang-tidy-14 \
      cmake \
      doxygen \
      git \
      pkg-config \
      zlib1g-dev \
      libelf-dev \
      libdwarf-dev
}

install_fedora() {
  # Update dnf.
  dnf -y update
  # Install packages.
  dnf -y install \
      @development-tools \
      clang \
      clang-tools-extra \
      cmake \
      doxygen \
      git \
      pkgconf-pkg-config \
      zlib-devel \
      elfutils-libelf-devel \
      libdwarf
}

install_arch() {
  # Update pacman.
  pacman -Syu --noconfirm
  # Install packages.
  pacman -S --noconfirm \
      base-devel \
      clang \
      clang-tools-extra \
      cmake \
      doxygen \
      git \
      pkgconf \
      zlib \
      elfutils \
      libdwarf
}

main "$@"