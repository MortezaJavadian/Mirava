# Mirava - Video Course Progress Tracker

Mirava is a command-line tool that helps you watch video courses. It scans your directory to find videos and shows you how much of each video you've watched and how long you have until you finish the course.

## Installation

### Pre-built Binaries (Recommended)

#### Ubuntu/Debian (.deb packages)
```bash
# For AMD64 systems:
wget https://github.com/MortezaJavadian/Mirava/releases/latest/download/mirava_1.0.0_amd64.deb
sudo dpkg -i mirava_1.0.0_amd64.deb
sudo apt-get install -f

# For ARM64 systems:
wget https://github.com/MortezaJavadian/Mirava/releases/latest/download/mirava_1.0.0_arm64.deb
sudo dpkg -i mirava_1.0.0_arm64.deb
sudo apt-get install -f
```

#### Other Linux Distributions
```bash
# Download pre-built binary for your architecture:
wget https://github.com/MortezaJavadian/Mirava/releases/latest/download/mirava-linux-amd64
chmod +x mirava-linux-amd64
sudo mv mirava-linux-amd64 /usr/local/bin/mirava

# For ARM64:
wget https://github.com/MortezaJavadian/Mirava/releases/latest/download/mirava-linux-arm64
chmod +x mirava-linux-arm64
sudo mv mirava-linux-arm64 /usr/local/bin/mirava
```

#### Windows
```cmd
# Download from GitHub releases or use PowerShell:
curl -L https://github.com/MortezaJavadian/Mirava/releases/latest/download/mirava-windows-amd64.exe -o mirava.exe
```

#### macOS
```bash
# For Intel Macs:
wget https://github.com/MortezaJavadian/Mirava/releases/latest/download/mirava-macos-x86_64
chmod +x mirava-macos-x86_64
sudo mv mirava-macos-x86_64 /usr/local/bin/mirava

# For Apple Silicon Macs:
wget https://github.com/MortezaJavadian/Mirava/releases/latest/download/mirava-macos-arm64
chmod +x mirava-macos-arm64
sudo mv mirava-macos-arm64 /usr/local/bin/mirava
```

### Manual Installation (Build from Source)

1. **Install Dependencies:**

   **Ubuntu/Debian:**
   ```bash
   sudo apt-get update
   sudo apt-get install libavformat-dev libavutil-dev libjansson-dev build-essential
   ```

   **Fedora/RHEL:**
   ```bash
   sudo dnf install ffmpeg-devel jansson-devel gcc
   ```

   **Arch Linux:**
   ```bash
   sudo pacman -S ffmpeg jansson gcc
   ```

2. **Build and Install:**
   ```bash
   git clone https://github.com/MortezaJavadian/Mirava.git
   cd Mirava
   make
   sudo cp mirava /usr/local/bin/
   ```

## Usage

### Basic Commands

#### List Videos and Sync Progress
```bash
mirava
```

#### Set Video Progress
```bash
mirava set <video_number> <progress>
```

#### Mark Video as Complete
```bash
mirava mark <video_number> [video_number...]
```

#### Show Help
```bash
mirava help
```

### Examples

```bash
# List all videos and show progress
mirava

# Set video 3 to 50% watched
mirava set 3 50%

# Set video 5 to 1 hour 20 minutes 10 seconds watched
mirava set 5 1:20:10

# Mark video 8 as completely watched
mirava mark 8

# Mark multiple videos (3, 5, and 7) as completely watched  
mirava mark 3 5 7
```

## Output Format

```
--- Course: Python Programming ---
 1. intro/welcome.mp4                           [00:05:23] [✓]
 2. basics/variables.mp4                        [00:12:45] [75%]
 3. basics/functions.mp4                        [00:18:30] 
 4. advanced/classes.mp4                        [00:25:15] [25%]
---------------------------------------------------------------------
Total Duration: 1:01:53  |  Overall Progress: 50%
```

## Contributing

Feel free to submit issues, feature requests, or pull requests to improve Mirava.

## License

This project is open source. Please check the license file for details.