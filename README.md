# C Local Music Player 🎵

<a name="link">Requirements & running</a>

​A *custom*, *high-performance*, *lightweight* local music player</br>
written from scratch in C and built for Linux for the time being.</br>
Designed with a modular architecture and powered by miniaudio.</br>
Now it uses a web-app GUI

It features:</br>
* a recursive directory scanner,</br>
* an in-memory dynamic playlist, </br>
* real-time auto-advance,</br>

![Screenshot of working my_player](https://github.com/Desire4444/My_player/blob/main/screenshots/playing.png)

## Project Overview & Features

​This player was built as a from-scratch systems programming project to handle 
local music collection management and multi-threaded audio playback
without heavy desktop framework bloat.

​**Recursive Directory Scanner**:</br>
Automatically crawls through a specified root directory and all its subfolders to discover local audio tracks.</br>
Now able to select directory from GUI
![Screenshot of working my_player](https://github.com/Desire4444/My_player/blob/main/screenshots/directory.png)

**Dynamic In-Memory Playlist**:</br>
Efficiently allocates and resizes dynamic memory arrays to store and manage track lists.
​Robust Audio Playback Engine: Integrates miniaudio for clean cross-platform audio decoding and hardware routing.

## Architecture & Tech Stack

​The codebase is split into clean, decoupled functional modules:</br>
​Core C & POSIX: Written in standard C `(gcc)`, utilizing system-level headers (`<dirent.h>`, `<sys/stat.h>`, `<poll.h>`).
​Build System: Managed via a clean, dependency-tracked Makefile.
​Audio Layer: Powered by **miniaudio** (single-header audio library) handling thread creation (`-lpthread`), math operations (`-lm`), and dynamic loading (`-ldl`).</br>
​**UI** using mangoose It is able to use dynamic web UI featuring flex
![Screenshot of working my_player](https://github.com/Desire4444/My_player/blob/main/screenshots/que.png)

​**End-of-Track Auto-Advance**:</br>
Seamlessly transitions to the next track in the playlist when a song finishes naturally.

## Prerequisites / System Dependencies

​To compile and run this player on a **Linux environment**, ensure you have the following installed:</br>
​Compiler & Build Tools: `gcc`, `browswer` and `make`</br>
​Audio Subsystem: An active Linux audio server (`PipeWire`, `PulseAudio`, or `ALSA`)</br>
​Core Development Libraries: Standard C library, `pthread`, `m`, and `dl` (typically pre-installed on all Linux distros).

## Installation & Building Instructions
```
sudo pacman -Syu gcc make
make clear
make
```
The repository already contains two music files for testing out if it plays on your setup in `music_test`

### Running the program
**Tesing if it outputs sound**
`./music_player music_test/` 
</br>
**paste the url in your broswer for the GUI**
`http://localhost:8080`

</br>
**Add your own path**
`./music_player /path/to/your/music/folder/` | **Inclusion of directory is now optional**

## Features simple shortcuts
![Screenshot of working my_player](https://github.com/Desire4444/My_player/blob/main/screenshots/shortcuts.png)

### Your welcome to help out as well if you want to
