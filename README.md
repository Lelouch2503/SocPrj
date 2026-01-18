# SocPrj

SystemC-based SoC Peripheral Simulation Project.

## Prerequisites

Before building, please install the following dependencies:

### Required Tools
- **CMake** >= 3.20
- **Conan** >= 2.0 (Python package manager for C++ dependencies)
- **GCC/G++** or compatible C++17 compiler
- **Make** (GNU Make for Unix-like systems)

### Pre-installing Boost (Recommended)

Boost is a large dependency. To avoid long build times with Conan, it is **strongly recommended** to install Boost system-wide first:

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libboost-all-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install boost-devel
```

**Arch Linux:**
```bash
sudo pacman -S boost
```

After installing Boost, you can tell Conan to use the system version.

## Building

### Using Conan (Default)

```bash
# Full build (clean + configure + build)
make all

# Or step by step:
make conan_d    # Install Conan dependencies (Debug)
make configure  # Configure CMake
make build      # Build project
```

### Troubleshooting

**CMake: "Could not find toolchain file"**
Ensure Conan dependencies are installed first:
```bash
make conan_d
```

**CMake: "Unable to find a build program corresponding to Unix Makefiles"**
Install build-essential tools:
```bash
sudo apt-get install build-essential
```

## Running

```bash
make run
```
