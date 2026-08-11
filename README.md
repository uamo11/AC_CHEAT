# AC_CHEAT - Memory Manipulation & Reverse Engineering Study

> [!WARNING]
> **FOR EDUCATIONAL AND RESEARCH PURPOSES ONLY**
> This repository is a proof-of-concept for educational research in reverse engineering, memory manipulation, and OpenGL function hooking. It is intended for computer science students and cybersecurity enthusiasts to study memory layout and application flow.
> 
> The authors do not condone, support, or encourage the use of this code to gain unfair advantages in multiplayer games or violate any Terms of Service. Do not use this software on public servers.

## Overview
This project demonstrates how to interact with the memory space of a running Win32 application (AssaultCube 1.3.0.2). It explores concepts such as:
- **Dynamic Link Library (DLL) Injection**
- **Win32 API Interception** (Hooking `wglSwapBuffers`, `WndProc`)
- **Memory Reading/Writing** (Direct pointer manipulation)
- **Overlay Rendering** (Using ImGui over an existing OpenGL context)

## Technical Details
The codebase shows how to locate data structures in memory using static offsets, how to calculate distances between 3D vectors in a virtual world (for mathematical study), and how to safely execute code in the context of another process without causing race conditions.

## Building from Source
Pre-compiled binaries are intentionally **NOT** provided. To study the code in action, you must compile it yourself using MinGW (GCC).

1. Ensure `g++.exe` is in your PATH.
2. Run `build.bat` to compile the DLL and the injector.
3. The resulting binaries will be placed in the `bin/` directory.

## License
MIT License. See the code for details. Use responsibly and at your own risk.
