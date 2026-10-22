# MagSH
> **Disclaimer:** This is my first major programming project, created strictly for educational purposes. It is a learning exercise in C systems programming, not a production-ready shell. Please view it as a work in progress.
> **Note:** This shell is confirmed to work correctly on BSD and Linux systems only; compatibility with other operating systems has not been tested.
## About the Project
**MagSH** is a custom, Unix-like command-line shell written in C from scratch. This project was built to deeply understand how operating systems work under the hood: process management, file descriptors, system calls, and signal handling.
## Features
- **Interactive prompt** with colored output and current directory display.

- **Built-in commands:** `cd`, `exit`.

- **External command execution** using `fork()` and `execvp()` system calls.

- **Pipes (`|`):** chaining commands by passing stdout to stdin.

- **Output redirection (`>`):** saving command output to files.

- **Signal handling:** graceful handling of `Ctrl+C` (SIGINT), `Ctrl+Z` (SIGTSTP), and `Ctrl+\` (SIGQUIT) to prevent the shell from crashing.

- **Cross-platform compatibility:** designed to work seamlessly on Linux and BSD systems (FreeBSD, macOS) using standard POSIX APIs.

- **OS compatibility:** Works correctly on BSD and Linux systems only; compatibility with other systems has not been tested.

## Build & Run
You will need a C compiler (like `gcc` or `clang`) installed on your system.


```bash
# Compile the project
gcc magsh.c -o magsh

# Run the shell
./magsh
