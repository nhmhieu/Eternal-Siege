#!/usr/bin/env python3
"""Generate the original Eternal Siege music and sound-effect pack.

The generator is deterministic and uses only NumPy plus ffmpeg.  All output is
original procedural audio created for this student project.
"""

from __future__ import annotations

import argparse
import math
import subprocess
import tempfile
import wave
from pathlib import Path

import numpy as np


RATE = 44_100
RNG = np.random.default_rng(20260801)


def envelope(length: int, attack: float = 0.01, release: float = 0.08) -> np.ndarray:
    result = np.ones(length, dtype=np.float64)
    attack_samples = min(length, max(1, int(RATE * attack)))
    release_samples = min(length, max(1, int(RATE * release)))
    result[:attack_samples] *= np.linspace(0.0, 1.0, attack_samples)
    result[-release_samples:] *= np.linspace(1.0, 0.0, release_samples)
    return result


def oscillator(freq: float, duration: float, kind: str = "sine") -> np.ndarray:
    t = np.arange(int(RATE * duration), dtype=np.float64) / RATE
    phase = 2.0 * math.pi * freq * t
    if kind == "triangle":
        signal = (2.0 / math.pi) * np.arcsin(np.sin(phase))
    elif kind == "square":
        signal = np.sign(np.sin(phase))
    else:
        signal = np.sin(phase)
    return signal * envelope(len(signal))


def midi(note: int) -> float:
    return 440.0 * (2.0 ** ((note - 69) / 12.0))


def add_note(track: np.ndarray, start: float, duration: float, note: int,
             volume: float, kind: str = "sine") -> None:
    sample = oscillator(midi(note), duration, kind) * volume
    begin = int(start * RATE)
    end = min(len(track), begin + len(sample))
    if end > begin:
        track[begin:end] += sample[: end - begin]


def add_kick(track: np.ndarray, start: float, volume: float = 0.35) -> None:
    duration = 0.18
    t = np.arange(int(duration * RATE), dtype=np.float64) / RATE
    phase = 2.0 * math.pi * (95.0 * t - 55.0 * t * t)
    sample = np.sin(phase) * np.exp(-22.0 * t) * volume
    begin = int(start * RATE)
    end = min(len(track), begin + len(sample))
    track[begin:end] += sample[: end - begin]


def add_noise_hit(track: np.ndarray, start: float, duration: float,
                  volume: float, decay: float = 14.0) -> None:
    t = np.arange(int(duration * RATE), dtype=np.float64) / RATE
    noise = RNG.normal(0.0, 1.0, len(t))
    sample = noise * np.exp(-decay * t) * volume
    begin = int(start * RATE)
    end = min(len(track), begin + len(sample))
    track[begin:end] += sample[: end - begin]


def normalize(signal: np.ndarray, peak: float = 0.88) -> np.ndarray:
    maximum = float(np.max(np.abs(signal)))
    return signal if maximum < 1e-9 else signal * (peak / maximum)


def write_wav(path: Path, signal: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    signal = normalize(signal)
    if signal.ndim == 1:
        signal = signal[:, None]
    pcm = np.clip(signal, -1.0, 1.0)
    pcm = (pcm * 32767.0).astype("<i2")
    with wave.open(str(path), "wb") as output:
        output.setnchannels(pcm.shape[1])
        output.setsampwidth(2)
        output.setframerate(RATE)
        output.writeframes(pcm.tobytes())


def write_sfx(directory: Path) -> None:
    def save(name: str, signal: np.ndarray) -> None:
        write_wav(directory / name, signal)

    t = np.arange(int(0.11 * RATE)) / RATE
    save("ui_click.wav", np.sin(2 * math.pi * (720 + 1800 * t) * t) * np.exp(-35 * t))

    t = np.arange(int(0.34 * RATE)) / RATE
    whoosh = RNG.normal(0, 1, len(t)) * np.sin(math.pi * np.clip(t / 0.34, 0, 1))
    whoosh *= 0.45 + 0.55 * np.sin(2 * math.pi * (180 + 620 * t) * t)
    save("sword_slash.wav", whoosh)

    t = np.arange(int(0.26 * RATE)) / RATE
    twang = (np.sin(2 * math.pi * 180 * t) + 0.45 * np.sin(2 * math.pi * 360 * t))
    twang *= np.exp(-18 * t)
    snap = RNG.normal(0, 0.25, len(t)) * np.exp(-55 * t)
    save("bow_shot.wav", twang + snap)

    t = np.arange(int(0.48 * RATE)) / RATE
    magic = np.sin(2 * math.pi * (310 * t + 510 * t * t))
    magic += 0.45 * np.sin(2 * math.pi * (620 * t + 840 * t * t))
    magic *= envelope(len(t), 0.02, 0.16)
    save("magic_cast.wav", magic)

    t = np.arange(int(0.20 * RATE)) / RATE
    hit = 0.65 * RNG.normal(0, 1, len(t)) * np.exp(-28 * t)
    hit += np.sin(2 * math.pi * 72 * t) * np.exp(-20 * t)
    save("hit.wav", hit)

    t = np.arange(int(0.72 * RATE)) / RATE
    death = np.sin(2 * math.pi * (150 * t - 72 * t * t)) * np.exp(-3.5 * t)
    death += 0.25 * RNG.normal(0, 1, len(t)) * np.exp(-7 * t)
    save("monster_death.wav", death)

    wave_signal = np.zeros(int(1.0 * RATE))
    for start, note in ((0.0, 57), (0.24, 62), (0.48, 69)):
        sample = oscillator(midi(note), 0.48, "triangle") * 0.55
        begin = int(start * RATE)
        wave_signal[begin:begin + len(sample)] += sample[: len(wave_signal) - begin]
    save("wave_start.wav", wave_signal)

    upgrade = np.zeros(int(0.9 * RATE))
    for index, note in enumerate((62, 65, 69, 74)):
        add_note(upgrade, index * 0.14, 0.38, note, 0.45, "triangle")
    save("upgrade.wav", upgrade)

    t = np.arange(int(0.80 * RATE)) / RATE
    warning = np.sin(2 * math.pi * (210 * t + 245 * t * t))
    warning *= 0.3 + 0.7 * np.sin(math.pi * np.clip(t / 0.8, 0, 1))
    save("beam_warning.wav", warning)

    t = np.arange(int(0.46 * RATE)) / RATE
    zap = np.sin(2 * math.pi * (1250 * t - 1050 * t * t)) * np.exp(-5 * t)
    zap += RNG.normal(0, 0.6, len(t)) * np.exp(-12 * t)
    save("beam_fire.wav", zap)

    win = np.zeros(int(2.2 * RATE))
    for start, note in ((0.0, 60), (0.22, 64), (0.44, 67), (0.72, 72), (1.05, 76)):
        add_note(win, start, 0.8, note, 0.5, "triangle")
    save("win.wav", win)

    game_over = np.zeros(int(2.1 * RATE))
    for start, note in ((0.0, 62), (0.4, 60), (0.8, 57), (1.2, 50)):
        add_note(game_over, start, 0.85, note, 0.45, "sine")
    save("game_over.wav", game_over)


def build_music(kind: str) -> np.ndarray:
    if kind == "menu":
        bpm, bars = 72, 8
        chords = ((50, 53, 57), (46, 50, 53), (41, 45, 48), (48, 52, 55))
        melody = (62, 65, 69, 65, 58, 62, 65, 62, 57, 60, 64, 60, 60, 64, 67, 64)
    elif kind == "gameplay":
        bpm, bars = 120, 8
        chords = ((50, 53, 57), (48, 52, 55), (46, 50, 53), (48, 52, 55))
        melody = (62, 65, 69, 65, 62, 65, 70, 69, 60, 64, 67, 64, 60, 64, 69, 67)
    else:
        bpm, bars = 140, 8
        chords = ((40, 41, 47), (40, 45, 46), (38, 41, 45), (40, 41, 47))
        melody = (52, 53, 59, 53, 52, 57, 58, 57, 50, 53, 57, 53, 52, 53, 59, 60)

    beat = 60.0 / bpm
    duration = bars * 4 * beat
    mono = np.zeros(int(duration * RATE), dtype=np.float64)

    for bar in range(bars):
        chord = chords[bar % len(chords)]
        start = bar * 4 * beat
        for note in chord:
            add_note(mono, start, 4 * beat, note, 0.10 if kind == "menu" else 0.075, "sine")
        for eighth in range(8):
            note = chord[eighth % len(chord)] + 12
            add_note(mono, start + eighth * beat / 2, beat * 0.42, note,
                     0.11 if kind == "menu" else 0.14, "triangle")
        for pulse in range(4):
            add_note(mono, start + pulse * beat, beat * 0.68, chord[0] - 12,
                     0.15 if kind == "menu" else 0.23, "sine")
            if kind != "menu":
                add_kick(mono, start + pulse * beat, 0.25 if kind == "gameplay" else 0.34)
                add_noise_hit(mono, start + (pulse + 0.5) * beat, 0.08,
                              0.055 if kind == "gameplay" else 0.085, 32.0)

    step = duration / len(melody)
    for index, note in enumerate(melody):
        add_note(mono, index * step, step * 0.78, note,
                 0.13 if kind == "menu" else 0.17, "triangle")

    fade = min(int(0.025 * RATE), len(mono) // 2)
    mono[:fade] *= np.linspace(0.0, 1.0, fade)
    mono[-fade:] *= np.linspace(1.0, 0.0, fade)
    mono = normalize(mono, 0.78)
    left = mono
    right = np.roll(mono, int(0.008 * RATE)) * 0.94
    return np.column_stack((left, right))


def write_music(directory: Path) -> None:
    directory.mkdir(parents=True, exist_ok=True)
    for kind in ("menu", "gameplay", "boss"):
        with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as temp:
            wav_path = Path(temp.name)
        try:
            write_wav(wav_path, build_music(kind))
            output = directory / f"{kind}_theme.ogg"
            subprocess.run([
                "ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
                "-i", str(wav_path), "-c:a", "libvorbis", "-q:a", "5",
                str(output)
            ], check=True)
        finally:
            wav_path.unlink(missing_ok=True)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--assets", type=Path,
                        default=Path(__file__).resolve().parents[1] / "assets" / "audio")
    args = parser.parse_args()
    write_sfx(args.assets / "sfx")
    write_music(args.assets / "music")
    print(f"Generated audio pack in {args.assets}")


if __name__ == "__main__":
    main()
