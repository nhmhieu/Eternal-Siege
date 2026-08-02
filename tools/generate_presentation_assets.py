#!/usr/bin/env python3
"""Generate deterministic map tiles and a window icon for Eternal Siege."""

from __future__ import annotations

import random
from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
RNG = random.Random(20260801)
SIZE = 48


def noisy_base(color: tuple[int, int, int], variation: int) -> Image.Image:
    image = Image.new("RGB", (SIZE, SIZE), color)
    pixels = image.load()
    for y in range(SIZE):
        for x in range(SIZE):
            delta = RNG.randint(-variation, variation)
            pixels[x, y] = tuple(max(0, min(255, channel + delta)) for channel in color)
    return image


def grass() -> Image.Image:
    image = noisy_base((43, 91, 55), 9)
    draw = ImageDraw.Draw(image)
    for _ in range(35):
        x, y = RNG.randrange(SIZE), RNG.randrange(SIZE)
        shade = RNG.choice([(76, 127, 70), (31, 73, 43), (101, 139, 69)])
        draw.line((x, y, x + RNG.choice((-1, 1)), y - RNG.randint(2, 4)), fill=shade)
    return image


def path() -> Image.Image:
    image = noisy_base((103, 84, 58), 8)
    draw = ImageDraw.Draw(image)
    for y in range(0, SIZE, 12):
        offset = 6 if (y // 12) % 2 else 0
        draw.line((0, y, SIZE, y), fill=(61, 52, 43), width=1)
        for x in range(-offset, SIZE, 16):
            draw.line((x, y, x, min(SIZE, y + 12)), fill=(68, 56, 44), width=1)
    return image


def wall() -> Image.Image:
    image = noisy_base((67, 72, 80), 7)
    draw = ImageDraw.Draw(image)
    for y in range(0, SIZE, 12):
        offset = 8 if (y // 12) % 2 else 0
        draw.line((0, y, SIZE, y), fill=(31, 34, 40), width=2)
        for x in range(-offset, SIZE, 16):
            draw.line((x, y, x, min(SIZE, y + 12)), fill=(38, 41, 48), width=2)
    draw.line((1, 1, SIZE - 2, 1), fill=(115, 119, 126), width=1)
    return image


def rune(base: Image.Image, color: tuple[int, int, int], gate: bool) -> Image.Image:
    image = base.copy()
    draw = ImageDraw.Draw(image, "RGBA")
    center = SIZE // 2
    draw.ellipse((7, 7, 41, 41), outline=(*color, 190), width=3)
    draw.ellipse((13, 13, 35, 35), outline=(*color, 100), width=2)
    if gate:
        draw.line((center, 10, center, 38), fill=(*color, 230), width=3)
        draw.line((14, 25, center, 10, 34, 25), fill=(*color, 210), width=3)
    else:
        points = [(center, 9), (29, 20), (40, center), (29, 29),
                  (center, 40), (19, 29), (8, center), (19, 20)]
        draw.line(points + [points[0]], fill=(*color, 220), width=2)
    return image


def make_icon() -> None:
    source = Image.open(ROOT / "assets/images/ui/logo.png").convert("RGBA")
    alpha_box = source.getchannel("A").getbbox()
    if alpha_box:
        source = source.crop(alpha_box)
    source.thumbnail((220, 220), Image.Resampling.LANCZOS)
    icon = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    icon.alpha_composite(source, ((256 - source.width) // 2,
                                  (256 - source.height) // 2))
    icon.save(ROOT / "assets/images/ui/window_icon.png")


def main() -> None:
    tile_dir = ROOT / "assets/images/map"
    tile_dir.mkdir(parents=True, exist_ok=True)
    grass_tile = grass()
    path_tile = path()
    grass_tile.save(tile_dir / "grass.png")
    path_tile.save(tile_dir / "path.png")
    wall().save(tile_dir / "wall.png")
    rune(noisy_base((83, 42, 52), 7), (242, 80, 104), False).save(tile_dir / "spawn.png")
    rune(grass_tile, (70, 205, 235), True).save(tile_dir / "gate.png")
    make_icon()
    print(f"Generated map tiles and icon under {ROOT / 'assets/images'}")


if __name__ == "__main__":
    main()
