# 🦌 Moose Hunting

**Moose Hunting** is a small project (game) about hunting moose.
The project is built using **CMake** and can be compiled on **Windows, Linux, and macOS**.

---

# Requirements

Before building the project, make sure the following tools are installed:

- **CMake 3.20 or higher**
- **A C++ compiler**
  - **GCC / Clang** for Linux and macOS
  - **MSVC or MinGW** for Windows

- **Git** (to clone the repository)

---

# Getting Started

## 1. Clone the Repository

```bash
git clone <repository-url>
cd Moose-hunting
```

---

# Configure the Project

Choose the instructions depending on your operating system and preferred build tool.

---

# Windows

## Option A — MinGW (GCC)

If you have **MinGW-w64** installed and added to your `PATH`:

```bash
cmake -G "MinGW Makefiles" -S . -B build
```

---

## Option B — Visual Studio

If **Visual Studio** (with C++ tools) is installed:

```bash
cmake -S . -B build
```

CMake will automatically select a generator such as:

```
Visual Studio 17 2022
```

You can then:

### Open the project in Visual Studio

```
build/Moose-hunting.sln
```

### Or build from the command line

```bash
cmake --build build --config Release
```

---

## Option C — Ninja

If **Ninja** is installed:

```bash
cmake -G "Ninja" -S . -B build
```

---

# Linux / macOS

On Unix-like systems the default generator is usually **Unix Makefiles**.

Make sure a C++ compiler is installed:

- `g++`
- `clang`

On macOS you may need to install **Xcode Command Line Tools**:

```bash
xcode-select --install
```

### Configure the project

```bash
cmake -S . -B build
```

---

## Optional Generators

### Makefiles

```bash
cmake -G "Unix Makefiles" -S . -B build
```

### Ninja

```bash
cmake -G "Ninja" -S . -B build
```

### Xcode (macOS)

```bash
cmake -G "Xcode" -S . -B build
```

---

# Build the Project

After configuration, build the project:

```bash
cmake --build build
```

For multi-configuration generators (such as Visual Studio or Xcode):

```bash
cmake --build build --config Release
```

or

```bash
cmake --build build --config Debug
```

---

# ▶️ Run the Executable

After a successful build, the executable will be located inside the **build** directory.

### Windows

```bash
cd build
.\MooseHunting.exe
```

### Linux / macOS

```bash
cd build
./MooseHunting
```

---

# Quick Build & Run

_(Git Bash on Windows with MinGW)_

```bash
cmake -G "MinGW Makefiles" -S . -B build && cmake --build build && cd build && ./MooseHunting
```

---

# Notes

- Make sure **CMake and your compiler are available in PATH**.
- If you modify the source code, you only need to repeat the **build and run** steps.
- For easier development, consider using an IDE with built-in CMake support.

### Recommended IDEs

- **Visual Studio**
- **CLion**
- **VS Code** with **CMake Tools**
- **Qt Creator**

---

# Project Structure

```
Moose-hunting
│
├── external
├── include
├── src
│   └── main.cpp
│
├── .gitignore
├── CMakeLists.txt
│
└── README.md
```
