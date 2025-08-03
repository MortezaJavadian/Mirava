# Mirava - Video Course Progress Tracker

Mirava is a command-line tool that to help you watch video courses. It scans your directory for find videos and show you how much of each video you've watched and how long do you have until you finish the course.

## Usage

### Basic Commands

#### List Videos and Sync Progress
```bash
./mirava
```

#### Set Video Progress
```bash
./mirava set <video_number> <progress>
```

#### Mark Video as Complete
```bash
./mirava mark <video_number>
```

#### Show Help
```bash
./mirava help
```

### Examples

```bash
# List all videos and show progress
./mirava

# Set video 3 to 50% watched
./mirava set 3 50%

# Set video 5 to 1 hour 20 minutes 10 seconds watched
./mirava set 5 1:20:10

# Mark video 8 and 9 as completely watched
./mirava mark 8 9
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
## Developing To Improve Mirava

### Installing Dependencies

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

### Installation

1. Clone or download the project
2. Navigate to the project directory
3. Build the program:
   ```bash
   make
   ```
4. (Optional) Install globally:
   ```bash
   sudo cp mirava /usr/local/bin/
   ```

## Contributing

Feel free to submit issues, feature requests, or pull requests to improve Mirava.

## License

This project is open source. Please check the license file for details.