C Local Music Player 🎵
​A custom, high-performance, lightweight local music player written from scratch in C for Linux. Designed with a modular architecture and powered by miniaudio, it features a recursive directory scanner, an in-memory dynamic playlist, real-time auto-advance, and a sleek Terminal User Interface (TUI) dashboard powered by ANSI escape codes.
​1. Project Overview & Features
​This player was built as a from-scratch systems programming project to handle local music collection management and multi-threaded audio playback without heavy desktop framework bloat.
​Recursive Directory Scanner: Automatically crawls through a specified root directory and all its subfolders to discover local audio tracks.
​Dynamic In-Memory Playlist: Efficiently allocates and resizes dynamic memory arrays to store and manage track lists.
​Robust Audio Playback Engine: Integrates miniaudio for clean cross-platform audio decoding and hardware routing.
​End-of-Track Auto-Advance: Seamlessly transitions to the next track in the playlist when a song finishes naturally.
​ANSI TUI Dashboard: Provides a real-time, non-scrolling visual dashboard showing player status, current track index, and active filename.
​Non-Blocking Control Loop: Listens for user commands instantaneously using system polling (poll()) while audio streams in the background.
​2. Architecture & Tech Stack
​The codebase is split into clean, decoupled functional modules:
​Core C & POSIX: Written in standard C (gcc), utilizing system-level headers (<dirent.h>, <sys/stat.h>, <poll.h>).
​Build System: Managed via a clean, dependency-tracked Makefile.
​Audio Layer: Powered by miniaudio (single-header audio library) handling thread creation (-lpthread), math operations (-lm), and dynamic loading (-ldl).
​UI Layer: Pure ANSI escape sequences for terminal cursor and screen management.
​3. Prerequisites / System Dependencies
​To compile and run this player on a Linux environment, ensure you have the following installed:
​Compiler & Build Tools: gcc and make
​Audio Subsystem: An active Linux audio server (PipeWire, PulseAudio, or ALSA)
​Core Development Libraries: Standard C library, pthread, m, and dl (typically pre-installed on all Linux distros).
​4. Installation & Building Instructions