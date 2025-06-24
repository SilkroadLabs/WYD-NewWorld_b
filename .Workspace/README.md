# MMORPG Game Server and Client

This project contains a C++ implementation of a classic MMORPG game server and client.

## Structure
- `server/` - C++ game server
- `client/` - C++ game client

## Building
This project uses CMake for cross-platform builds (Windows/Linux).

### Build Instructions
1. Install CMake and a C++ compiler (Visual Studio on Windows, GCC/Clang on Linux).
2. Run the following commands:

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Running
- Run the server executable from the `build/server/` directory.
- Run the client executable from the `build/client/` directory.
