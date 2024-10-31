# System Monitor

A modern system monitoring application built with Dear ImGui and SDL2, providing real-time visualization of system metrics including CPU usage, memory, processes, and network activity.

## Features

- Real-time system monitoring
- CPU usage graphs and statistics
- Memory and swap usage visualization
- Process management with filtering
- Network interface monitoring
- Modern, customizable UI with Dear ImGui

## Requirements

- C++ compiler with C++17 support
- SDL2 library
- OpenGL 3.0+ support
- Make build system

### System Dependencies

#### Linux
```bash
sudo apt-get install libsdl2-dev

```

## Building

1. Clone the repository
```bash
git clone https://github.com/Raymond9734/system-monitor.git
```

2. Build the project
```bash
make
```

The executable will be created as `monitor` in the project root directory.

## Running

After building, simply run:
```bash
./monitor
```

## Project Structure

- `main.cpp` - Application entry point and ImGui setup
- `render.cpp` - UI rendering and visualization logic
- `mem.cpp` - Memory and process monitoring
- `network.cpp` - Network interface monitoring
- `system.cpp` - System information gathering
- `imgui/` - Dear ImGui library files

## Configuration

The application's window layout and preferences are stored in `imgui.ini`. You can modify this file to persist your preferred window arrangements.

## Development

The project uses Dear ImGui version 1.80 WIP for the UI framework. Key configuration files:

- `.vscode/launch.json` - Debug configuration
- `Makefile` - Build configuration
- `header.h` - Main header with data structures and function declarations

## License

This project uses the following open-source components:
- Dear ImGui - MIT License
- SDL2 - zlib License
- STB Libraries - MIT License or Public Domain

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Troubleshooting

If you encounter rendering issues:
- Ensure OpenGL 3.0+ is supported on your system
- Check SDL2 installation and version
- Verify graphics drivers are up to date

For build issues:
- Ensure all dependencies are installed
- Check compiler version supports C++17
- Verify SDL2 paths in Makefile match your system
