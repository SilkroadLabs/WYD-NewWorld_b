# WYD New World

This repository contains the full source code for the **WYD New World** project. The codebase includes both client and server components used to build and run the game. Source files are organized in the following folders:

- `Source Cliente/` – client source code and Visual Studio project
- `Source Server/` – server source code and Visual Studio project
- `Release Cliente/` – prebuilt client binaries
- `Release Server/` – prebuilt server binaries
- `Arte/` – assets and database scripts

## Build instructions

1. Install **Visual Studio 2022** (or a compatible version) with C++ development tools.
2. To build the client, open `Source Cliente/Source Cliente/TMProject.sln` in Visual Studio and build the **Release** configuration.
3. To build the server, open `Source Server/Source Server/Server.sln` in Visual Studio and build the **Release** configuration.
4. The resulting executables will be placed in the corresponding `Build` or `Release` folders inside each project.

The project is released under the terms of the GNU General Public License version 3. See `LICENSE` for details.
