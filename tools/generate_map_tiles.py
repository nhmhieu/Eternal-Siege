#!/usr/bin/env python3
"""Generate original, deterministic fantasy map tiles with Pillow.

Run with: python tools/generate_map_tiles.py
Pillow is a development-only dependency; the game has no Python dependency.
"""

from __future__ import annotations

import math
import random
import re
from pathlib import Path

try:
    from PIL import Image, ImageDraw
except ImportError as error:
    raise SystemExit(
        "Pillow is required only to regenerate tiles: pip install Pillow"
    ) from error


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "assets/images/map"
SEED = 0xE7E24A9


def tile_size() -> int:
    source = (ROOT / "include/Constants.h").read_text(encoding="utf-8")
    match = re.search(r"TILE_SIZE\s*=\s*([0-9]+(?:\.[0-9]*)?)f", source)
    if not match:
        raise RuntimeError("Could not read TILE_SIZE from include/Constants.h")
    return int(float(match.group(1)))


def clamp(value: float) -> int:
    return max(0, min(255, int(round(value))))


def painted_base(size: int, base: tuple[int, int, int], variant: int,
                 amplitude: float) -> Image.Image:
    image = Image.new("RGB", (size, size), base)
    pixels = image.load()
    phase = variant * 0.83
    denominator = max(1, size - 1)
    for y in range(size):
        for x in range(size):
            wave = (
                math.sin(2 * math.pi * x / denominator + phase) * 0.55 +
                math.cos(2 * math.pi * y / denominator - phase) * 0.45 +
                math.sin(2 * math.pi * (x + y) / denominator + phase) * 0.22
            )
            delta = wave * amplitude
            pixels[x, y] = tuple(clamp(channel + delta) for channel in base)
    return image


def grass(size: int, variant: int) -> Image.Image:
    image = painted_base(size, (62 + variant * 2, 91 + variant, 55),
                         variant, 5.0)
    draw = ImageDraw.Draw(image)
    rng = random.Random(SEED + 101 * variant)
    for _ in range(13):
        x = rng.randint(5, size - 6)
        y = rng.randint(5, size - 6)
        color = rng.choice(((78, 112, 64), (48, 77, 47), (92, 119, 69)))
        draw.line((x, y + 2, x + rng.choice((-1, 0, 1)), y - 2),
                  fill=color, width=1)
    for _ in range(3):
        x = rng.randint(7, size - 8)
        y = rng.randint(7, size - 8)
        draw.point((x, y), fill=(155, 143, 83))
    return image


def path(size: int, variant: int) -> Image.Image:
    image = painted_base(size, (119 + variant * 2, 91 + variant, 60),
                         variant + 3, 5.5)
    draw = ImageDraw.Draw(image)
    rng = random.Random(SEED + 211 * variant)
    for _ in range(11):
        x = rng.randint(5, size - 6)
        y = rng.randint(5, size - 6)
        radius = rng.choice((1, 1, 2))
        color = rng.choice(((93, 73, 52), (145, 113, 73), (106, 83, 57)))
        draw.ellipse((x - radius, y - radius, x + radius, y + radius),
                     fill=color)
    return image


def wall(size: int, variant: int) -> Image.Image:
    image = painted_base(size, (62 + variant, 68 + variant, 61),
                         variant + 7, 4.5)
    draw = ImageDraw.Draw(image)
    rng = random.Random(SEED + 307 * variant)
    stone_fill = ((80, 84, 73), (73, 79, 70), (88, 88, 72))
    rows = ((4, 17), (18, 32), (33, size - 5))
    for row_index, (top, bottom) in enumerate(rows):
        offset = -8 if (row_index + variant) % 2 else 1
        x = offset
        while x < size:
            width = rng.randint(14, 20)
            left = max(1, x + 1)
            right = min(size - 2, x + width)
            if right > left:
                fill = rng.choice(stone_fill)
                draw.rounded_rectangle(
                    (left, top, right, bottom), radius=3,
                    fill=fill, outline=(48, 53, 49), width=1)
                if rng.random() < 0.65:
                    draw.line((left + 3, top + 2, right - 3, top + 2),
                              fill=(102, 103, 82), width=1)
            x += width
    for _ in range(6):
        x = rng.randint(4, size - 5)
        y = rng.randint(4, size - 5)
        draw.ellipse((x - 2, y - 1, x + 2, y + 1), fill=(59, 91, 54))
    return image


def generate() -> None:
    size = tile_size()
    OUTPUT.mkdir(parents=True, exist_ok=True)
    generators = (("Grass", grass), ("Path", path), ("Wall", wall))
    for prefix, generator in generators:
        for variant in (1, 2):
            image = generator(size, variant - 1)
            image.save(OUTPUT / f"{prefix}Tile{variant:02d}.png")


if __name__ == "__main__":
    generate()
