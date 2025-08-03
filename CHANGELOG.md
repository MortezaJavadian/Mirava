# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-08-04

### Added
- Initial release of Mirava Video Course Progress Tracker
- Smart course detection that searches up directory tree for `.mirava_data.json`
- Support for multiple video formats (mp4, avi, mkv, mov, wmv, flv, webm, m4v)
- Progress tracking with multiple formats:
  - Percentage (e.g., `50%`)
  - Time format (e.g., `1:20:10`)
  - Raw seconds
- Multiple video marking support: `mirava mark 1 3 5`
- Course hierarchy support - works from any subdirectory
- JSON-based progress persistence
- Relative path display from course root
- Cross-platform support:
  - Linux (AMD64, ARM64) - .deb packages and standalone binaries
  - Windows (AMD64) - standalone executable
  - macOS (Intel x86_64, Apple Silicon ARM64) - standalone binaries
- Man page for Unix systems
- Comprehensive help system

### Features
- **Smart Detection**: Automatically finds video files and course root
- **Progress Tracking**: Track exactly where you left off in each video
- **Multi-format Support**: Works with all common video formats
- **Cross-platform**: Native binaries for Linux, Windows, and macOS
- **Clean Interface**: Simple command-line interface with clear output
- **Persistent Storage**: Never lose your progress

### Commands
- `mirava` - List videos and sync progress
- `mirava set <num> <progress>` - Set progress for specific video
- `mirava mark <num> [num...]` - Mark one or more videos as complete
- `mirava help` - Show usage information

## [Unreleased]

### Planned
- Configuration file support
- Integration with popular video players
- Export/import functionality
- Statistics and analytics
- Course completion certificates
