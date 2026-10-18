let isPlaying = false;
let lastTrackIndex = -1;
let previousVolume = 100;
let playlistTracks = [];

let isSeeking = false;
let totalDuration = 0;

function formatTime(seconds) {
    if (isNaN(seconds) || seconds < 0) return "00:00";
    let mins = Math.floor(seconds / 60);
    let secs = Math.floor(seconds % 60);
    return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
}

async function fetchPlaylist() {
    try {
        let res = await fetch('/api/playlist');
        if (!res.ok) return;
        playlistTracks = await res.json();
        renderPlaylist();
    } catch (e) {
        console.warn("Playlist endpoint not available.");
    }
}

function renderPlaylist() {
    const listContainer = document.getElementById('playlist-list');
    const mobileContainer = document.getElementById('mobile-playlist-list');
    if (!playlistTracks.length) return;

    const htmlContent = playlistTracks.map((track, idx) => {
        const isActive = idx === lastTrackIndex;
        return `
            <button onclick="playTrack(${idx})" 
                class="w-full text-left p-2.5 rounded-xl border transition flex items-center justify-between group ${
                    isActive 
                    ? 'bg-indigo-600/20 border-indigo-500/50 text-indigo-200 shadow-md' 
                    : 'bg-slate-900/40 hover:bg-slate-800/80 border-slate-800/60 text-slate-300 hover:text-white'
                }">
                <div class="flex items-center space-x-3 overflow-hidden mr-2">
                    <span class="text-xs font-mono shrink-0 w-5 text-right ${isActive ? 'text-indigo-400 font-bold' : 'text-slate-500'}">
                        ${isActive ? '▶' : idx + 1}
                    </span>
                    <div class="truncate">
                        <p class="text-xs font-semibold truncate leading-tight">${track.title || track.filename || `Track ${idx + 1}`}</p>
                        <p class="text-[10px] text-slate-400 truncate">${track.artist || 'Unknown Artist'}</p>
                    </div>
                </div>
                ${isActive ? '<span class="w-2 h-2 rounded-full bg-indigo-400 animate-pulse shrink-0"></span>' : ''}
            </button>
        `;
    }).join('');

    if (listContainer) listContainer.innerHTML = htmlContent;
    if (mobileContainer) mobileContainer.innerHTML = htmlContent;
}

async function playTrack(index) {
    try {
        await fetch(`/api/play?index=${index}`);
        toggleMobileQueue(false);
        fetchStatus();
    } catch (e) {
        console.error(`Failed to play track index ${index}:`, e);
    }
}

async function fetchStatus() {
    try {
        let res = await fetch('/api/status');
        if (!res.ok) throw new Error("Server response error");

        let data = await res.json();

        // Update connection status
        const dot = document.getElementById('connection-dot');
        if (dot) dot.className = "w-2 h-2 rounded-full bg-emerald-500 animate-pulse";

        // Update song info
        document.getElementById('title').innerText = data.title || "Unknown Title";
        document.getElementById('artist').innerText = data.artist || "Unknown Artist";
        document.getElementById('playlist-counter').innerText = `${data.current} / ${data.total} Tracks`;

        totalDuration = data.tot_time || 0;
        document.getElementById('tot-time').innerText = formatTime(totalDuration);

        // Update seek slider position if user is not currently dragging it
        if (!isSeeking) {
            document.getElementById('cur-time').innerText = formatTime(data.cur_time);
            let seekSlider = document.getElementById('seek-slider');
            if (seekSlider && totalDuration > 0) {
                let percent = (data.cur_time / totalDuration) * 100;
                seekSlider.value = percent;
            }
        }

        // Sync volume slider
        let volumeSlider = document.getElementById('volume-slider');
        if (volumeSlider && document.activeElement !== volumeSlider) {
            volumeSlider.value = data.volume;
            document.getElementById('vol-value').innerText = `${Math.round(data.volume)}%`;
            updateVolumeIcon(data.volume);
        }

        // Playback state
        isPlaying = data.playing === 1;
        const playIcon = document.getElementById('play-icon');
        const pauseIcon = document.getElementById('pause-icon');
        if (playIcon && pauseIcon) {
            playIcon.classList.toggle('hidden', isPlaying);
            pauseIcon.classList.toggle('hidden', !isPlaying);
        }

        // Cover & queue index refresh
        const currentIdx = data.current - 1;
        if (currentIdx !== lastTrackIndex) {
            lastTrackIndex = currentIdx;
            let img = document.getElementById('cover-img');
            let fallback = document.getElementById('fallback-icon');
            if (img) img.style.display = 'block';
            if (fallback) fallback.style.display = 'none';
            if (img) img.src = `/api/cover?t=${new Date().getTime()}`;
            renderPlaylist();
        }

    } catch (err) {
        const dot = document.getElementById('connection-dot');
        if (dot) dot.className = "w-2 h-2 rounded-full bg-red-500";
    }
}

// Handler while dragging scrubber
function onSeekInput(percent) {
    isSeeking = true;
    if (totalDuration > 0) {
        let currentSecs = (percent / 100) * totalDuration;
        document.getElementById('cur-time').innerText = formatTime(currentSecs);
    }
}

// Handler when scrubber released
async function onSeekChange(percent) {
    if (totalDuration > 0) {
        let targetSecs = (percent / 100) * totalDuration;
        try {
            await fetch(`/api/seek?pos=${targetSecs.toFixed(2)}`);
        } catch (e) {
            console.error("Seek request failed:", e);
        }
    }
    isSeeking = false;
    fetchStatus();
}

async function sendCommand(endpoint) {
    try {
        await fetch(`/api/${endpoint}`);
        fetchStatus();
    } catch (e) {
        console.error(`Command ${endpoint} failed:`, e);
    }
}

function togglePlay() {
    sendCommand(isPlaying ? 'pause' : 'resume');
}

function setVolume(val) {
    document.getElementById('vol-value').innerText = `${Math.round(val)}%`;
    updateVolumeIcon(val);
    fetch(`/api/volume?level=${val}`);
}

function toggleMute() {
    let volumeSlider = document.getElementById('volume-slider');
    if (!volumeSlider) return;

    if (volumeSlider.value > 0) {
        previousVolume = volumeSlider.value;
        setVolume(0);
        volumeSlider.value = 0;
    } else {
        setVolume(previousVolume);
        volumeSlider.value = previousVolume;
    }
}

function updateVolumeIcon(val) {
    const high = document.getElementById('vol-icon-high');
    const mute = document.getElementById('vol-icon-mute');
    if (high && mute) {
        if (val == 0) {
            high.classList.add('hidden');
            mute.classList.remove('hidden');
        } else {
            high.classList.remove('hidden');
            mute.classList.add('hidden');
        }
    }
}

function toggleModal(show) {
    const modal = document.getElementById('shortcuts-modal');
    if (modal) {
        modal.classList.toggle('hidden', !show);
        modal.classList.toggle('flex', show);
    }
}

function toggleMobileQueue(show) {
    const modal = document.getElementById('mobile-queue-modal');
    if (modal) modal.classList.toggle('hidden', !show);
}

document.addEventListener('keydown', (e) => {
    if (['INPUT', 'TEXTAREA'].includes(document.activeElement.tagName)) return;

    switch (e.key) {
        case ' ':
            e.preventDefault();
            togglePlay();
            break;
        case 'ArrowRight':
            e.preventDefault();
            sendCommand('next');
            break;
        case 'ArrowLeft':
            e.preventDefault();
            sendCommand('prev');
            break;
        case 'ArrowUp':
            e.preventDefault();
            {
                let slider = document.getElementById('volume-slider');
                let newVol = Math.min(100, parseInt(slider.value) + 5);
                slider.value = newVol;
                setVolume(newVol);
            }
            break;
        case 'ArrowDown':
            e.preventDefault();
            {
                let slider = document.getElementById('volume-slider');
                let newVol = Math.max(0, parseInt(slider.value) - 5);
                slider.value = newVol;
                setVolume(newVol);
            }
            break;
        case 'm':
        case 'M':
            toggleMute();
            break;
    }
});

document.addEventListener('DOMContentLoaded', () => {
    const shortcutsBtn = document.getElementById('shortcuts-btn');
    if (shortcutsBtn) {
        shortcutsBtn.addEventListener('click', () => toggleModal(true));
    }

    setInterval(fetchStatus, 1000);
    fetchStatus();
    fetchPlaylist();
});