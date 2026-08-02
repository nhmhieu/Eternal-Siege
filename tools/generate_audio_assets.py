#!/usr/bin/env python3
"""Generate Eternal Siege's original mono 44.1 kHz PCM SFX assets."""

from __future__ import annotations

import math
import random
import struct
import wave
from pathlib import Path


RATE = 44_100
OUTPUT = Path(__file__).resolve().parents[1] / "assets/audio/sfx"
RNG = random.Random(0xE7E2A1)


def tone(duration: float, start_hz: float, end_hz: float | None = None,
         volume: float = 1.0, phase: float = 0.0) -> list[float]:
    count = int(duration * RATE)
    end_hz = start_hz if end_hz is None else end_hz
    result: list[float] = []
    angle = phase
    for index in range(count):
        progress = index / max(1, count - 1)
        frequency = start_hz + (end_hz - start_hz) * progress
        angle += 2.0 * math.pi * frequency / RATE
        result.append(math.sin(angle) * volume)
    return result


def noise(duration: float, volume: float = 1.0) -> list[float]:
    return [RNG.uniform(-volume, volume)
            for _ in range(int(duration * RATE))]


def mix(*tracks: tuple[list[float], float]) -> list[float]:
    length = max((len(samples) + int(offset * RATE)
                  for samples, offset in tracks), default=0)
    output = [0.0] * length
    for samples, offset in tracks:
        start = int(offset * RATE)
        for index, sample in enumerate(samples):
            output[start + index] += sample
    return output


def shape(samples: list[float], attack: float = 0.008,
          release: float = 0.04, decay: float = 0.0) -> list[float]:
    attack_samples = max(1, int(attack * RATE))
    release_samples = max(1, int(release * RATE))
    length = len(samples)
    output: list[float] = []
    for index, sample in enumerate(samples):
        fade_in = min(1.0, index / attack_samples)
        fade_out = min(1.0, (length - 1 - index) / release_samples)
        exponential = math.exp(-decay * index / RATE)
        output.append(sample * fade_in * fade_out * exponential)
    return output


def low_noise(duration: float, volume: float, smoothing: float) -> list[float]:
    current = 0.0
    output: list[float] = []
    for value in noise(duration, volume):
        current += (value - current) * smoothing
        output.append(current)
    return output


def save(name: str, samples: list[float]) -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    peak = max((abs(sample) for sample in samples), default=1.0)
    gain = 0.72 / max(peak, 1e-9)
    pcm = bytearray()
    for sample in samples:
        value = max(-1.0, min(1.0, sample * gain))
        pcm.extend(struct.pack("<h", int(value * 32767)))
    with wave.open(str(OUTPUT / name), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(RATE)
        wav.writeframes(pcm)


def generate() -> None:
    save("ui_click.wav", shape(mix(
        (tone(0.075, 920, 1320, 0.8), 0.0),
        (tone(0.045, 1840, 1380, 0.3), 0.012)), 0.002, 0.025, 9.0))

    save("spirit_bolt.wav", shape(mix(
        (tone(0.34, 520, 1320, 0.75), 0.0),
        (tone(0.28, 1040, 1810, 0.32), 0.025),
        (low_noise(0.24, 0.23, 0.06), 0.0)), 0.008, 0.075, 2.8))

    save("melee_hit.wav", shape(mix(
        (tone(0.18, 155, 82, 0.72), 0.0),
        (low_noise(0.13, 0.9, 0.19), 0.0)), 0.001, 0.055, 8.0))

    save("bow_shot.wav", shape(mix(
        (tone(0.22, 410, 130, 0.55), 0.0),
        (tone(0.10, 1280, 350, 0.45), 0.0),
        (low_noise(0.18, 0.35, 0.12), 0.015)), 0.002, 0.05, 6.0))

    save("enemy_death.wav", shape(mix(
        (tone(0.62, 280, 72, 0.65), 0.0),
        (low_noise(0.48, 0.5, 0.055), 0.04)), 0.006, 0.15, 2.3))

    radiant = mix(
        (tone(0.75, 392, 784, 0.42), 0.0),
        (tone(0.68, 494, 988, 0.34), 0.08),
        (tone(0.56, 659, 1318, 0.28), 0.16))
    save("radiant_pulse.wav", shape(radiant, 0.025, 0.18, 0.75))

    save("ally_skill.wav", shape(mix(
        (tone(0.72, 230, 920, 0.55), 0.0),
        (tone(0.48, 690, 1510, 0.38), 0.08),
        (low_noise(0.42, 0.24, 0.05), 0.0)), 0.012, 0.13, 1.2))

    save("wave_start.wav", shape(mix(
        (tone(0.22, 523, 659, 0.62), 0.0),
        (tone(0.30, 784, 1047, 0.72), 0.24)), 0.008, 0.08, 1.4))

    save("boss_spawn.wav", shape(mix(
        (tone(1.05, 62, 43, 0.8), 0.0),
        (tone(0.40, 118, 54, 0.62), 0.28),
        (low_noise(0.75, 0.65, 0.025), 0.18)), 0.015, 0.22, 1.15))

    victory = mix(
        (tone(0.34, 523, 659, 0.42), 0.0),
        (tone(0.38, 659, 784, 0.46), 0.28),
        (tone(0.78, 784, 1047, 0.58), 0.58),
        (tone(0.70, 1047, 1318, 0.28), 0.64))
    save("victory.wav", shape(victory, 0.012, 0.24, 0.35))

    defeat = mix(
        (tone(0.48, 392, 294, 0.48), 0.0),
        (tone(0.58, 294, 196, 0.52), 0.34),
        (tone(0.78, 196, 98, 0.58), 0.76),
        (low_noise(0.58, 0.22, 0.035), 0.76))
    save("defeat.wav", shape(defeat, 0.015, 0.28, 0.5))


if __name__ == "__main__":
    generate()
