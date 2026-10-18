/* STREAMING_CHUNK:Initializing application state variables... */
let isPlaying = false;
let lastTrackIndex = -1;
let previousVolume = 100;

/* STREAMING_CHUNK:Defining time string formatting helper... */
function formatTime(seconds) {
    if (isNaN(seconds) || seconds < 0) return "00:00";
    let mins = Math.floor(seconds / 60);
    let secs = Math.floor(seconds % 60);
    return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
}

/* STREAMING_CHUNK:Defining status polling handler... */
async function fetchStatus() {
    try {
        let res = await fetch('/api/status');
        if (!res.ok) throw new Error("Server response error");
        
        let data = await res.json();

        // Update Status Indicator
        const dot = document.getElementById('connection-dot');
        if (dot) dot.className = "w-2.5 h-2.5 rounded-full bg-emerald-500 animate-pulse";

        // Update Text Information
        document.getElementById('title').innerText = data.title || "Unknown Title";
        document.getElementById('artist').innerText = data.artist || "Unknown Artist";
        document.getElementById('playlist-counter').innerText = `${data.current} / ${data.total} Tracks`;
        
        // Update Timers and Seek fill
        document.getElementById('cur-time').innerText = formatTime(data.cur_time);
        document.getElementById('tot-time').innerText = formatTime(data.tot_time);

        let percent = data.tot_time > 0 ? (data.cur_time / data.tot_time) * 100 : 0;
        document.getElementById('progress-fill').style.width = percent + '%';

        // Sync Volume Slider if not actively focused
        let volumeSlider = document.getElementById('volume-slider');
        if (volumeSlider && document.activeElement !== volumeSlider) {
            volumeSlider.value = data.volume;
            document.getElementById('vol-value').innerText = `${Math.round(data.volume)}%`;
            updateVolumeIcon(data.volume);
        }

        // Sync Play/Pause Button States
        isPlaying = data.playing === 1;
        const playIcon = document.getElementById('play-icon');
        const pauseIcon = document.getElementById('pause-icon');
        if (playIcon && pauseIcon) {
            if (isPlaying) {
                playIcon.classList.add('hidden');
                pauseIcon.classList.remove('hidden');
            } else {
                playIcon.classList.remove('hidden');
                pauseIcon.classList.add('hidden');
            }
        }

        // Trigger Cover Image Refresh on Track Change
        if (data.current - 1 !== lastTrackIndex) {
            lastTrackIndex = data.current - 1;
            
            let img = document.getElementById('cover-img');
            let fallback = document.getElementById('fallback-icon');
            
            if (img) img.style.display = 'block';
            if (fallback) fallback.style.display = 'none';
            
            // Force cache-busting refresh for new track cover art
            if (img) img.src = `/api/cover?t=${new Date().getTime()}`;
        }

    } catch (err) {
        // Dim connection status on error
        const dot = document.getElementById('connection-dot');
        if (dot) dot.className = "w-2.5 h-2.5 rounded-full bg-red-500";
    }
}

/* STREAMING_CHUNK:Defining playback control functions... */
async function sendCommand(action) {
    try {
        await fetch(`/api/${action}`);
        fetchStatus();
    } catch (e) {
        console.error(`Failed to send command ${action}:`, e);
    }
}

function togglePlay() {
    sendCommand(isPlaying ? 'pause' : 'resume');
}

async function setVolume(val) {
    document.getElementById('vol-value').innerText = `${Math.round(val)}%`;
    updateVolumeIcon(val);
    await fetch(`/api/volume?level=${val}`);
}

function toggleMute() {
    let volumeSlider = document.getElementById('volume-slider');
    if (volumeSlider) {
        if (parseInt(volumeSlider.value) > 0) {
            previousVolume = volumeSlider.value;
            setVolume(0);
        } else {
            setVolume(previousVolume || 100);
        }
    }
}

function updateVolumeIcon(val) {
    const high = document.getElementById('vol-icon-high');
    const mute = document.getElementById('vol-icon-mute');
    if (high && mute) {
        if (val <= 0) {
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

/* STREAMING_CHUNK:Attaching DOM event listeners and keyboard shortcuts... */
document.addEventListener('DOMContentLoaded', () => {
    const shortcutsBtn = document.getElementById('shortcuts-btn');
    if (shortcutsBtn) {
        shortcutsBtn.addEventListener('click', () => toggleModal(true));
    }

    // Keyboard Shortcuts Handler
    document.addEventListener('keydown', (e) => {
        if (['INPUT', 'TEXTAREA'].includes(document.activeElement.tagName)) return;

        switch (e.code) {
            case 'Space':
                e.preventDefault();
                togglePlay();
                break;
            case 'ArrowLeft':
                e.preventDefault();
                sendCommand('prev');
                break;
            case 'ArrowRight':
                e.preventDefault();
                sendCommand('next');
                break;
            case 'ArrowUp':
                e.preventDefault();
                let currentVolUp = Math.min(100, parseInt(document.getElementById('volume-slider').value || 100) + 5);
                setVolume(currentVolUp);
                break;
            case 'ArrowDown':
                e.preventDefault();
                let currentVolDown = Math.max(0, parseInt(document.getElementById('volume-slider').value || 0) - 5);
                setVolume(currentVolDown);
                break;
            case 'KeyM':
                e.preventDefault();
                toggleMute();
                break;
        }
    });

    // Initialize polling loop
    setInterval(fetchStatus, 1000);
    fetchStatus();
});
