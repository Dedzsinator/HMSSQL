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

THIRD_PARTY_DIR="third_party"
DOWNLOAD_DIR="temp_downloads"

# URLs for third-party libraries
ARGPARSE_URL="https://github.com/p-ranav/argparse/archive/refs/tags/v2.9.zip"
FMT_URL="https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.zip"
LIBFORT_URL="https://github.com/seleznevae/libfort/archive/refs/tags/v0.4.2.zip"
LIBPG_QUERY_URL="https://github.com/pganalyze/libpg_query/archive/refs/tags/13-2.1.2.zip"
LINENOISE_URL="https://github.com/antirez/linenoise/archive/refs/heads/master.zip"
MURMUR3_URL="https://raw.githubusercontent.com/PeterScott/murmur3/master/MurmurHash3.h"
MURMUR3_CPP_URL="https://raw.githubusercontent.com/PeterScott/murmur3/master/MurmurHash3.cpp" 
SPDLOG_URL="https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.zip"
UTF8PROC_URL="https://github.com/JuliaStrings/utf8proc/archive/refs/tags/v2.8.0.zip"
HTTPLIB_URL="https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.11.3.zip"
JSON_URL="https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.zip"

download_and_extract() {
  local url=$1
  local target_dir=$2
  local extracted_dir=$3
  local target_name=${4:-${extracted_dir}}
  
  echo "Downloading from $url..."
  mkdir -p "$DOWNLOAD_DIR"
  local download_file="$DOWNLOAD_DIR/$(basename "$url")"
  
  if [[ "$url" == *".zip" ]]; then
    # Download and extract ZIP files
    curl -L "$url" -o "$download_file"
    mkdir -p "$target_dir"
    unzip -q "$download_file" -d "$DOWNLOAD_DIR"
    
    if [ -d "$target_dir/$target_name" ]; then
      rm -rf "$target_dir/$target_name"
    fi
    
    mv "$DOWNLOAD_DIR/$extracted_dir" "$target_dir/$target_name"
  else
    # Download individual files
    mkdir -p "$target_dir/$target_name"
    curl -L "$url" -o "$target_dir/$target_name/$(basename "$url")"
  fi
}

setup_third_party() {
  echo "Setting up third-party libraries..."
  
  mkdir -p "$THIRD_PARTY_DIR"
  
  # Download and extract argparse
  download_and_extract "$ARGPARSE_URL" "$THIRD_PARTY_DIR" "argparse-2.9" "argparse"
  
  # Download and extract fmt
  download_and_extract "$FMT_URL" "$THIRD_PARTY_DIR" "fmt-9.1.0" "fmt"
  
  # Download and extract libfort
  download_and_extract "$LIBFORT_URL" "$THIRD_PARTY_DIR" "libfort-0.4.2" "libfort"
  
  # Download and extract libpg_query
  download_and_extract "$LIBPG_QUERY_URL" "$THIRD_PARTY_DIR" "libpg_query-13-2.1.2" "libpg_query"
  
  # Download and extract linenoise
  download_and_extract "$LINENOISE_URL" "$THIRD_PARTY_DIR" "linenoise-master" "linenoise"
  
  # Download murmur3 files
  mkdir -p "$THIRD_PARTY_DIR/murmur3"
  curl -L "$MURMUR3_URL" -o "$THIRD_PARTY_DIR/murmur3/MurmurHash3.h"
  curl -L "$MURMUR3_CPP_URL" -o "$THIRD_PARTY_DIR/murmur3/MurmurHash3.cpp"
  
  # Download and extract spdlog
  download_and_extract "$SPDLOG_URL" "$THIRD_PARTY_DIR" "spdlog-1.11.0" "spdlog"
  
  # Download and extract utf8proc
  download_and_extract "$UTF8PROC_URL" "$THIRD_PARTY_DIR" "utf8proc-2.8.0" "utf8proc"
  
  # Download and extract httplib
  download_and_extract "$HTTPLIB_URL" "$THIRD_PARTY_DIR" "cpp-httplib-0.11.3" "httplib"
  
  # Download and extract json
  download_and_extract "$JSON_URL" "$THIRD_PARTY_DIR" "json-3.11.2" "json"
  
  # Clean up downloads
  rm -rf "$DOWNLOAD_DIR"
  
  echo "Third-party libraries have been successfully downloaded and extracted."
}

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

  # Setup third-party libraries
  setup_third_party

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
  powershell.exe -Command "choco install -y git cmake curl unzip --installargs 'ADD_CMAKE_TO_PATH=System'"
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
      cmake \
      doxygen \
      git \
      pkg-config \
      zlib1g-dev \
      libelf-dev \
      libdwarf-dev \
      curl \
      unzip
}

install_fedora() {
  # Update dnf.
  dnf -y update
  # Install packages.
  dnf -y install \
      @development-tools \
      cmake \
      doxygen \
      git \
      pkgconf-pkg-config \
      zlib-devel \
      elfutils-libelf-devel \
      libdwarf \
      curl \
      unzip
}

install_arch() {
  # Update pacman.
  pacman -Syu --noconfirm
  # Install packages.
  pacman -S --noconfirm \
      base-devel \
      cmake \
      doxygen \
      git \
      pkgconf \
      zlib \
      elfutils \
      libdwarf \
      curl \
      unzip
}

main "$@"