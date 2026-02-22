# Linux Utilities & Shells in C

A collection of low-level Linux utilities and progressively featured command-line shells written in C. This project explores Unix internals, system calls, and process management from the ground up.

---

## Project Components

### 1. Basic Utilities

Implemented using raw system calls — `open()`, `read()`, `write()`, `getcwd()`, `rename()`, and `unlink()` — rather than higher-level library abstractions.

| Utility | Description | Key System Calls |
|---------|-------------|-----------------|
| `pwd`   | Prints the current working directory | `getcwd()` |
| `echo`  | Writes arguments to stdout with spacing | `write()` |
| `cp`    | Copies a source file to a destination using a 1 KB buffer | `open()`, `read()`, `write()` |
| `mv`    | Moves/renames a file; falls back to copy+delete if `rename()` fails across filesystems | `rename()`, `unlink()` |

---

### 2. The Shell Series

A progression of shell implementations, each layer adding new capabilities.

#### 🔹 FemtoShell
The most minimal shell — a basic REPL loop supporting only `echo` and `exit`. No child processes, no external commands. A proof-of-concept for the shell execution model.

**Supported built-ins:** `echo`, `exit`

---

#### 🔹 PicoShell
A fully functional interactive shell. Parses user input dynamically, runs external programs as child processes, and handles several built-in commands.

**Supported built-ins:** `echo`, `pwd`, `cd`, `exit`

**Key features:**
- Uses `fork()` + `execvp()` + `waitpid()` to spawn and reap child processes
- Tokenizes input with `strtok()` and builds `argv` arrays dynamically using `realloc()` and `strdup()`
- Tracks and returns the last command's exit status
- Flushes `stdout` before forking to prevent buffer duplication in the child

---

#### 🔹 NanoShell
Extends PicoShell with support for shell variables and environment management.

**Supported built-ins:** `echo`, `pwd`, `cd`, `export`, `exit`

**Key features:**
- **Shell variables** — assign variables with `name=value` syntax (no spaces); stored in a dynamic `ShellVar` array using `realloc`
- **Variable substitution** — expands `$VAR` tokens in any argument before execution, checking shell variables first, then the environment via `getenv()`
- **`export`** — promotes a shell variable into the process environment with `setenv()`, making it available to child processes
- **65 KB input buffer** — supports long and complex command lines

---

#### 🔹 MicroShell
The most complete shell in the series. Builds on NanoShell with full I/O redirection support.

**Supported built-ins:** `echo`, `pwd`, `cd`, `export`, `exit`

**Key features:**
- Everything in NanoShell, plus:
- **I/O redirection** — supports `<` (stdin), `>` (stdout), and `2>` (stderr) operators
- Redirection is applied in child processes via `dup2()` before `execvp()`, so the command's file descriptors are transparently replaced
- Redirection operators are stripped from the argument list before execution, so `argv` passed to the command stays clean
- Built-in commands validate redirect targets (e.g., checks that `>` files can be opened) without executing the redirection itself

---

## Learning Goals

This project was built to develop hands-on familiarity with:

- **File I/O** — manual buffer management and file descriptor lifecycle
- **Process management** — how a shell spawns, executes, and reaps child processes
- **Memory management** — dynamic allocation with `realloc` and `strdup`
- **Shell internals** — variable storage, substitution, and environment propagation
- **I/O redirection** — using `dup2()` to rewire standard file descriptors in child processes
- **System calls** — direct interaction with the Linux kernel, bypassing libc wrappers where possible
- **Error handling** — checking return values of every system call and exiting with meaningful codes

---

## How to Compile & Run

Each component has its own source file. Compile and run any utility with:

```bash
gcc microshell.c -o microshell
./microshell
```

Replace `microshell.c` with the desired source file (e.g., `pwd.c`, `cp.c`, `nanoshell.c`).

---

## 🗂 Project Structure

```
.
├── pwd.c           # Print working directory
├── echo.c          # Echo arguments to stdout
├── cp.c            # File copy utility (1 KB buffered)
├── mv.c            # File move/rename utility
├── femtoshell.c    # Minimal shell: echo + exit only
├── picoshell.c     # Functional shell: fork/exec, built-ins, dynamic parsing
├── nanoshell.c     # Shell variables, $VAR substitution, export
└── microshell.c    # I/O redirection (<, >, 2>) via dup2
```

---
