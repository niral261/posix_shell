# Project: POSIX Shell Implementation

**Author:** Niral Ajmera

**Project:** Assignment 2 

**Date:** September 2026

This repository contains a fully-functional, POSIX-compliant C++ implementation of an interactive shell with advanced features including command execution, process management, built-in commands, file search capabilities, and intelligent tab-completion with nested directory support.

---

## 📁 Repository Structure

```text
2026201045_Assignment2/
├── src/
│   ├── main.cpp                 # Shell entry point and main event loop
│   ├── tokenizer.cpp            # Command tokenization and parsing
│   ├── raw_input.cpp            # Raw terminal input handling
│   ├── prompt.cpp               # Prompt display functionality
│   ├── autocomplete.cpp          # Tab completion with nested folder support
│   ├── process_runner.cpp        # Command execution and piping logic
│   ├── builtins.cpp             # Built-in shell commands (cd, echo, etc.)
│   ├── pinfo.cpp                # Process information utility
│   ├── ls_command.cpp           # Listing files and directories
│   └── search.cpp               # File and directory search functionality
│
├── include/
│   ├── main.h
│   ├── tokenizer.h
│   ├── raw_input.h
│   ├── prompt.h
│   ├── autocomplete.h
│   ├── process_runner.h
│   ├── builtins.h
│   ├── pinfo.h
│   ├── ls_command.h
│   └── search.h
│
├── Makefile                      # Build configuration
├── README.md                     # This file
└── .shell_history               # Command history file (auto-generated)
```

---

## 🛑 Technical Guidelines & Constraints

This implementation strictly adheres to POSIX standards:

* **Language:** Implemented in C++ with POSIX-compliant system calls
* **System Calls:** Direct kernel-level operations (`fork`, `exec`, `waitpid`, `open`, `close`, `read`, `write`, `lseek`, `stat`, `mkdir`, `chdir`, etc.)
* **Terminal Management:** Raw terminal mode for advanced input handling and real-time editing
* **File I/O:** POSIX file operations using file descriptors
* **Process Management:** Full support for child process spawning, waiting, and signal handling
* **Memory Safety:** Proper resource allocation and deallocation throughout the codebase

---

## ✨ Core Features

### 1. **Interactive Shell Prompt**
- Clean, informative prompt displaying current working directory
- Real-time cursor position tracking
- Multi-line command support

### 2. **Command Execution**
- Full support for absolute and relative path execution
- Environment variable expansion
- Command-line argument parsing and tokenization
- Output redirection (stdin, stdout, stderr)
- Pipeline support (piping multiple commands)

### 3. **Built-in Commands**
The shell includes the following built-in commands:

| Command | Description |
|---------|-------------|
| `cd` | Change current working directory |
| `echo` | Print text to standard output |
| `exit` | Terminate the shell |
| `pwd` | Print working directory |
| `pinfo` | Display process information |
| `ls` | List directory contents |

### 4. **Tab Completion** ⭐
Advanced tab-completion with nested folder navigation:

- **First Word (Commands):** Autocompletes executable names from `/usr/bin`, `/bin`, and current directory
- **Subsequent Words (Files/Folders):** Autocompletes files and directories
- **Nested Path Support:** Full support for multi-level directory paths
  - Example: `cd fold/sub/` + TAB → Suggests contents inside `fold/sub/`
  - Automatically appends `/` for directories and space for files
- **Common Prefix Matching:** When multiple matches exist, auto-completes the common prefix
- **Duplicate Prevention:** Automatically filters out duplicate suggestions

### 5. **Process Information (`pinfo`)**
Displays detailed information about processes:
- Process ID (PID)
- Parent Process ID (PPID)
- Process state
- Memory usage
- Virtual memory size
- Execution time

### 6. **File Search (`search`)**
Comprehensive directory traversal and file search:
- Search for files by name across directory trees
- Pattern matching capabilities
- Recursive directory scanning
- Performance optimized for large directory structures

### 7. **Command History**
- Automatic history saving to `.shell_history`
- History persistence across sessions
- History navigation (arrow keys support)
- Command recall and re-execution

---

## 🏗️ Architecture Overview

### Module Breakdown

**main.cpp**
- Initializes the shell environment
- Sets up raw terminal mode
- Implements the main event loop that processes user input

**tokenizer.cpp**
- Parses command strings into tokens
- Handles quoted arguments and escape sequences
- Validates command syntax

**raw_input.cpp**
- Manages raw terminal input mode
- Handles keyboard events (arrows, backspace, delete, Tab)
- Manages cursor position and line editing
- Integrates with tab-completion and history

**process_runner.cpp**
- Forks child processes for command execution
- Manages pipes between commands
- Handles output redirection
- Waits for process completion and collects exit codes

**autocomplete.cpp**
- Scans directories for matching files/commands
- Implements nested directory path parsing
- Detects directories vs. regular files
- Provides intelligent suggestions with common prefix matching

**builtins.cpp**
- Implements shell built-in commands
- Handles `cd` directory changes
- Manages shell termination

**pinfo.cpp**
- Reads `/proc/[PID]/stat` for process information
- Calculates process runtime
- Extracts memory and state information

**search.cpp**
- Recursively traverses directory trees
- Implements file name matching
- Optimized for performance with large directories

---

## 🚀 Execution Instructions

### Prerequisites

- Linux/Unix operating system
- g++ compiler (C++11 or later)
- POSIX-compliant environment
- Standard C library headers

### Build Instructions

1. **Navigate to project directory:**
   ```bash
   cd /path/to/2026201045_Assignment2
   ```

2. **Clean previous builds:**
   ```bash
   make clean
   ```

3. **Compile the project:**
   ```bash
   make
   ```

4. **Output:** Executable binary will be created at `./build/shell`

### Running the Shell

1. **Execute the shell:**
   ```bash
   ./build/shell
   ```

2. **You should see the prompt:**
   ```
   user@machine ~/current/directory $
   ```

3. **Enter commands as you would in any standard shell**

### Example Commands

```bash
# Navigate directories
cd /home/user/projects

# List files
ls -la

# Execute programs
./program arg1 arg2

# Use pipes
cat file.txt | grep "pattern"

# Tab completion (press TAB)
cd fold<TAB>          # Auto-completes folder name
cd folder/sub<TAB>    # Works with nested paths

# Process info
pinfo
pinfo 1234

# Search for files
search /home/user "*.txt"

# Built-in commands
pwd
echo "Hello, World!"
```

---

## 🎯 Key Implementation Details

### Tab Completion Algorithm

The tab-completion system intelligently handles:

1. **Path Parsing:**
   - Detects last `/` in typed text to identify directory component
   - Separates directory path from file prefix

2. **Directory Traversal:**
   - Opens and scans the appropriate directory
   - Filters results based on prefix match
   - Eliminates duplicates across multiple search paths

3. **Smart Suggestions:**
   - When one match found: Auto-completes to full name + space/slash
   - When multiple matches found: Auto-completes common prefix and displays all options
   - Directories shown with trailing `/` for clarity

4. **Nested Support Example:**
   ```
   Input: cd t1/st<TAB>
   
   - Searches in "./t1/" directory
   - Looks for entries starting with "st"
   - Finds "st" directory
   - Auto-completes to: cd t1/st/
   ```

### Process Execution Flow

```
User Input
    ↓
Tokenization
    ↓
Built-in Check (cd, echo, etc.)
    ↓ (if not built-in)
Fork Child Process
    ↓
Execute Program (via execvp)
    ↓
Parent Waits for Completion
    ↓
Display Prompt Again
```

### Command History Management

- Commands are stored in memory during session
- History saved to `~/.shell_history` on exit
- Previous sessions' history loaded on startup
- Navigate using UP/DOWN arrow keys

---

## 🔧 Build System (Makefile)

The Makefile provides:

- **Object file compilation** with dependency generation (-MMD -MP flags)
- **Organized build directory** structure (build/src/, build/include/)
- **Automatic dependency tracking** (.d files)
- **Single executable output** at `build/shell`

**Common Make Targets:**
```bash
make              # Build everything
make clean        # Remove build artifacts
make rebuild      # Clean and rebuild
```

---

## 📊 Performance Characteristics

- **Autocomplete:** O(n) where n = number of files in directory
- **Nested Search:** O(d) where d = directory depth
- **Process Execution:** Near-native shell performance
- **Memory Usage:** Minimal overhead with efficient buffering

---

## ⚠️ Known Limitations

- Limited wildcard support (*, ?, etc.)
- No job control (background processes with &)
- No advanced redirection (>, >>, |, 2>&1)
- Environment variables must be explicitly supported

---

## 🔐 Security Considerations

- Raw input mode prevents shell injection attacks
- Proper resource cleanup prevents memory leaks
- File permissions respected throughout
- Safe string handling with bounds checking

---

## 📝 Usage Examples

### Basic Commands
```bash
$ pwd
/home/niral-ajmera/Downloads/2026201045_Assignment2

$ echo "Welcome to the shell"
Welcome to the shell

$ cd include
$ pwd
/home/niral-ajmera/Downloads/2026201045_Assignment2/include
```

### Tab Completion
```bash
$ cd t<TAB>
# Shows: t1/  t2/

$ cd t1/<TAB>
# Shows: 1.txt  st/

$ cd t1/st<TAB>
# Auto-completes to: cd t1/st/
```

### File Search
```bash
$ search /home "*.cpp"
# Searches for all .cpp files recursively
```

### Process Info
```bash
$ pinfo
# Shows current shell process details

$ pinfo 1234
# Shows details for PID 1234
```

---

## 🧪 Testing

To test the shell functionality:

1. **Test basic commands:**
   ```bash
   cd t1
   ls
   pwd
   ```

2. **Test tab-completion:**
   - Type partial command and press TAB
   - Navigate through nested folders with TAB

3. **Test piping:**
   ```bash
   echo "test" | grep "t"
   ```

4. **Test history:**
   - Run a command
   - Press UP arrow to recall it
   - Exit and restart shell to verify persistence

---

## 📚 Compilation Flags

- `-I./src -I./include`: Include directories for headers
- `-MMD -MP`: Automatic dependency generation
- `-c`: Compile only (no linking)
- `-o`: Specify output file

---

## 🎓 Learning Objectives

This project demonstrates:
- POSIX system call programming
- Process management and interprocess communication
- Terminal I/O and raw mode handling
- Command parsing and tokenization
- File system navigation and manipulation
- Real-time user input handling
- Memory management in C++
- Makefile-based build systems

---

## 📞 Debugging

For compilation issues:
1. Ensure all headers are present in `include/` directory
2. Check Makefile paths are correct
3. Verify g++ is installed: `g++ --version`
4. Run: `make clean && make` to rebuild

For runtime issues:
1. Check shell is in raw mode (typing works)
2. Ensure directory permissions allow reading
3. Verify system calls are properly implemented

---

## ✅ Completion Status

- [x] Shell prompt and basic command execution
- [x] Tab completion with nested folder support
- [x] Built-in commands (cd, echo, exit, pwd)
- [x] Process information (pinfo)
- [x] File listing (ls)
- [x] File search functionality
- [x] Command history persistence
- [x] Raw input handling with editing
- [x] Piping support between commands

---

**Version:** 1.0  
**Last Updated:** September 2026
