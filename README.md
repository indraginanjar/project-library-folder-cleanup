# Project Library Folder Cleanup

A cross-platform C++ application for recursively identifying and removing library folders (node_modules, venv, target, etc.) from a base directory.

## Motivation

This project was born out of a persistent problem: constantly running out of disk space. As a developer working on many different projects simultaneously, each project pulls its own set of external libraries into local folders like `node_modules`, `venv`, `target`, and others. Over time, these library folders accumulate and consume massive amounts of disk space—often tens of gigabytes—even for projects that haven't been touched in months.

The problem compounds when:
- Working with Node.js projects (each `node_modules` can be 200MB-1GB+)
- Managing Python virtual environments (multiple `venv` folders across projects)
- Building Rust/Java projects (large `target` and `.gradle` directories)
- Switching between different technology stacks regularly

This tool provides a quick way to identify and clean up these library folders across all your projects, reclaiming valuable disk space without manually hunting through directories.

## Features

- **Cross-platform**: Works on Windows and Linux
- **Dual interface**: GUI (Qt6) and CLI modes
- **Safe deletion**: Dry-run mode, confirmation prompts, system directory protection
- **Configurable**: Customize which folder names to search for
- **Progress reporting**: Real-time progress updates during operations

## Building

### Prerequisites

#### All Platforms
- **CMake** 3.16 or higher
- **C++17 compatible compiler**:
  - GCC 7+ (Linux)
  - Clang 5+ (Linux/macOS)
  - MSVC 2017+ (Windows)
- **Qt6** (required for GUI mode)
  - Qt6 Core
  - Qt6 Widgets
  - Qt6 Gui

#### Linux

Install dependencies on Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake qt6-base-dev
```

Install dependencies on Fedora/RHEL:
```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel
```

#### Windows

1. Install Visual Studio 2017 or later with C++ development tools
2. Install CMake from https://cmake.org/download/
3. Install Qt6 from https://www.qt.io/download-qt-installer
   - **IMPORTANT**: Add Qt6 bin directories to environment variable PATH
   - Example paths to add:
     - `C:\Qt\6.10.2\mingw_64\bin`
     - `C:\Qt\Tools\mingw1310_64\bin`
   - Or set `CMAKE_PREFIX_PATH` to Qt6 installation directory

**How to Add to PATH (Windows):**
1. Open System Properties → Advanced → Environment Variables
2. In System Variables, select "Path" and click Edit
3. Click New and add:
   - `C:\Qt\6.10.2\mingw_64\bin` (adjust to your Qt version)
   - `C:\Qt\Tools\mingw1310_64\bin` (adjust to your MinGW version)
4. Click OK to save
5. Restart terminal/command prompt to apply changes

### Building on Linux

```bash
# Clone or navigate to project directory
cd library-folder-cleanup

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Executables will be in build directory:
# - plf_cleanup_cli
# - plf_cleanup_gui
```

**Optional: Install system-wide**
```bash
sudo make install
```

### Building on Windows

#### Using Visual Studio Developer Command Prompt

```cmd
REM Navigate to project directory
cd library-folder-cleanup

REM Create build directory
mkdir build
cd build

REM Configure with CMake (adjust Qt path as needed)
cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64 ..

REM Build
cmake --build . --config Release

REM Executables will be in build\Release:
REM - plf_cleanup_cli.exe
REM - plf_cleanup_gui.exe
```

#### Using MinGW

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/mingw_64 ..

# Build
mingw32-make -j4
```

### Build Options

CMake build options can be configured during the cmake configuration step:

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `BUILD_GUI` | ON/OFF | ON | Build the GUI application (requires Qt6) |
| `BUILD_CLI` | ON/OFF | ON | Build the CLI application |
| `BUILD_TESTS` | ON/OFF | ON | Build test suites (unit, property, integration) |
| `CMAKE_BUILD_TYPE` | Debug/Release | Debug | Build configuration (Linux/MinGW) |

**Examples:**

Build CLI only (no Qt6 required):
```bash
cmake -DBUILD_GUI=OFF ..
```

Build release version:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

Build without tests:
```bash
cmake -DBUILD_TESTS=OFF ..
```

### Troubleshooting Build Issues

**Qt6 not found:**
```bash
# Set CMAKE_PREFIX_PATH to Qt6 installation
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 ..
```

**Compiler not found (Windows):**
- Run cmake from Visual Studio Developer Command Prompt
- Or add compiler to PATH

**Permission errors (Linux):**
```bash
# Ensure build directory is writable
chmod -R u+w build/
```

## Usage

### Prerequisites for Running the Application (Windows)

**IMPORTANT**: Before running the application, ensure Qt directories are registered in the PATH environment variable:

1. **Check current PATH:**
   ```cmd
   echo %PATH%
   ```
   Verify the output contains:
   - `C:\Qt\6.10.2\mingw_64\bin`
   - `C:\Qt\Tools\mingw1310_64\bin`

2. **If not present, add to PATH:**
   - Open System Properties → Advanced → Environment Variables
   - Edit "Path" variable in System Variables
   - Add both directories above
   - Restart terminal/command prompt

3. **Verify installation:**
   ```cmd
   where Qt6Core.dll
   ```
   Should display the path to Qt6Core.dll

**Alternative (Without Modifying PATH):**
Use the provided `run_gui.bat` script which automatically includes the PATH.

### CLI Mode

The CLI application provides a command-line interface for automated cleanup tasks and scripting.

#### Basic Syntax

```bash
./plf_cleanup_cli [options] <base_directory>
```

#### Command-Line Flags

| Flag | Description | Example |
|------|-------------|---------|
| `--dry-run` | Scan and identify folders without deleting them. Useful for previewing what would be removed. | `--dry-run` |
| `--force` | Skip confirmation prompt before deletion. Use with caution! | `--force` |
| `--verbose` | Enable detailed output including all operations and errors. | `--verbose` |
| `--folders <list>` | Comma-separated list of custom folder names to search for. Overrides default configuration. | `--folders node_modules,venv,target` |
| `--help` | Display help message with all available options. | `--help` |

#### Examples

Dry-run scan to preview what would be deleted:
```bash
./plf_cleanup_cli --dry-run /home/user/projects
```

Delete with custom folder list:
```bash
./plf_cleanup_cli --folders node_modules,venv /home/user/projects
```

Automated deletion without confirmation (for scripts):
```bash
./plf_cleanup_cli --force --verbose /home/user/projects
```

Verbose output for debugging:
```bash
./plf_cleanup_cli --verbose --dry-run /home/user/projects
```

#### Exit Codes

- `0` - Success (operation completed without fatal errors)
- `1` - Invalid arguments or fatal error
- `2` - Operation cancelled by user

#### Output

- **Standard Output**: Progress messages, scan results, deletion summary
- **Standard Error**: Error messages and warnings
- **Log File**: Detailed operation log saved to `cleanup.log` in application directory

### GUI Mode

The GUI application provides an intuitive graphical interface for interactive cleanup operations.

#### Starting the GUI

```bash
./plf_cleanup_gui
```

#### GUI Workflow

1. **Select Base Directory**
   - Click the "Browse" button to open a directory picker
   - Or manually enter the path in the text field
   - The application validates the path before allowing operations

2. **Configure Target Folders** (Optional)
   - Click "Configure Folders" to customize which folder names to search for
   - Add or remove folder names from the list
   - Changes are saved to the configuration file

3. **Scan for Library Folders**
   - Click "Scan" to begin recursive directory traversal
   - Progress bar shows scan progress
   - Results appear in the table with folder paths and sizes

4. **Review Results**
   - Browse the list of found library folders
   - Check/uncheck folders to select which ones to delete
   - View total size that will be reclaimed

5. **Delete Selected Folders**
   - Click "Delete Selected" to begin deletion
   - Confirmation dialog appears for safety
   - Progress bar shows deletion progress
   - Summary dialog displays results when complete

#### GUI Features

- **Real-time Progress**: Progress bars and status messages during operations
- **Selective Deletion**: Choose which folders to delete using checkboxes
- **Size Display**: See individual and total folder sizes
- **Error Reporting**: Clear error messages for any issues encountered
- **Cancellation**: Cancel long-running operations at any time

## Configuration

The application can be configured to search for custom folder names through multiple methods.

### Default Target Folders

The application includes these default library folder names:
- `node_modules` - Node.js dependencies
- `venv` - Python virtual environments
- `.venv` - Python virtual environments (hidden)
- `target` - Rust/Maven build output
- `.gradle` - Gradle cache
- `build` - Generic build output
- `dist` - Distribution/build output
- `__pycache__` - Python bytecode cache

### Configuration Methods

#### 1. Configuration File

Create a `config.json` file in the application directory to persist custom folder names.

**File Location:**
- Linux: Same directory as the executable
- Windows: Same directory as the executable

**Format:**

```json
{
    "version": "1.0",
    "target_folders": [
        "node_modules",
        "venv",
        ".venv",
        "target",
        ".gradle",
        "build",
        "dist",
        "__pycache__",
        "vendor",
        "bower_components"
    ]
}
```

**Field Descriptions:**
- `version` (string): Configuration format version (currently "1.0")
- `target_folders` (array of strings): List of folder names to search for

**Validation Rules:**
- Folder names must not contain path separators (`/` or `\`)
- Folder names must not contain invalid filesystem characters
- Empty folder names are rejected
- Duplicate names are automatically removed

**Loading Priority:**
1. Configuration file (if exists)
2. Default configuration (if no file or file is invalid)

#### 2. CLI Flags

Override configuration for a single run using the `--folders` flag:

```bash
./plf_cleanup_cli --folders node_modules,venv,target /path/to/scan
```

This overrides both the configuration file and defaults for this execution only.

#### 3. GUI Configuration Dialog

In GUI mode, click "Configure Folders" to:
- View current folder list
- Add new folder names
- Remove existing folder names
- Save changes to `config.json`

Changes made in the GUI are immediately saved and persist across sessions.

## Testing

The project includes comprehensive test suites using Catch2 framework with RapidCheck for property-based testing.

### Running All Tests

```bash
cd build
ctest
```

Or with verbose output:
```bash
ctest --verbose
```

### Running Individual Test Suites

```bash
# Unit tests - specific examples and edge cases
./unit_tests

# Property-based tests - universal correctness properties
./property_tests

# Integration tests - end-to-end workflows
./integration_tests
```

### Test Categories

- **Unit Tests**: Validate specific functionality, edge cases, and platform-specific behavior
- **Property Tests**: Validate universal properties across randomized inputs (100+ iterations)
- **Integration Tests**: Validate complete CLI and GUI workflows

## Safety Features

The application includes multiple safety mechanisms to prevent accidental data loss:

1. **System Directory Protection**: Refuses to operate on system directories (/, C:\, /usr, /System, /Windows)
2. **Dry-Run Mode**: Preview what would be deleted without actually deleting
3. **Confirmation Prompts**: Requires explicit confirmation before deletion (unless --force flag used)
4. **Selective Deletion**: GUI allows choosing specific folders to delete
5. **Symlink Safety**: Does not follow symbolic links outside the base directory
6. **Error Isolation**: Continues processing if individual folders fail to delete
7. **Comprehensive Logging**: All operations logged to file for audit trail

## Logging

The application creates a log file in the application directory:

**Log File Location:**
- Linux: `./cleanup.log`
- Windows: `cleanup.log` in executable directory

**Log Contents:**
- Timestamp for each operation
- All scanned directories
- Found library folders
- Deletion operations
- Errors and warnings
- Operation summaries

**Log Levels:**
- DEBUG: Detailed operation information (verbose mode only)
- INFO: Normal operation messages
- WARNING: Non-fatal issues
- ERROR: Fatal errors and failures

## Troubleshooting

### Common Issues

**"Permission denied" errors:**
- Run with elevated privileges (sudo on Linux, Administrator on Windows)
- Check folder permissions
- Close applications that may have files open in target folders

**"System directory" error:**
- The application refuses to operate on system directories for safety
- Choose a user directory instead (e.g., /home/user, C:\Users\username)

**GUI doesn't start:**
- **IMPORTANT**: Ensure Qt6 libraries are registered in the PATH environment variable
  - Check if `C:\Qt\6.10.2\mingw_64\bin` and `C:\Qt\Tools\mingw1310_64\bin` are in PATH
  - Restart terminal/command prompt after adding to PATH
- Ensure Qt6 libraries are installed and in PATH
- Check that Qt6 plugins are accessible
- Try running from command line to see error messages

**Folders not found:**
- Verify folder names in configuration match exactly (case-sensitive on Linux)
- Check that base directory path is correct
- Use --verbose flag in CLI to see detailed scan information

**Deletion fails:**
- Some folders may be locked by running processes
- Close IDEs, terminals, or applications using those folders
- Check the log file for specific error messages

## Performance

The application is designed for efficiency:

- **Memory Usage**: Stays under 500MB even for very large directory trees
- **Parallel Operations**: Deletion operations run in parallel where safe
- **Size Caching**: Directory sizes calculated once and cached
- **Efficient Traversal**: Uses optimized filesystem APIs
- **Progress Updates**: Regular updates without impacting performance

## License

MIT License
