#!/usr/bin/env python3
"""Fast asset/configuration validation that does not require SFML."""

from __future__ import annotations

import struct
import sys
import wave
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def png_info(path: Path) -> tuple[int, int, int]:
    data = path.read_bytes()[:29]
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG")
    width, height = struct.unpack(">II", data[16:24])
    return width, height, data[25]


def check(condition: bool, message: str, errors: list[str]) -> None:
    print(("PASS" if condition else "FAIL") + ": " + message)
    if not condition:
        errors.append(message)


def main() -> int:
    errors: list[str] = []
    constants_source = (ROOT / "include/Constants.h").read_text(
        encoding="utf-8")
    tile_size_match = re.search(
        r"TILE_SIZE\s*=\s*([0-9]+(?:\.[0-9]*)?)f", constants_source)
    check(tile_size_match is not None,
          "TILE_SIZE can be read from Constants.h", errors)
    tile_size = (int(float(tile_size_match.group(1)))
                 if tile_size_match else 0)
    player_mage = ROOT / "assets/images/PlayerMage.png"
    check(player_mage.exists(), "player mage exists", errors)
    if player_mage.exists():
        width, height, color_type = png_info(player_mage)
        check((width, height) == (1024, 1536),
              "PlayerMage is 1024x1536", errors)
        check(color_type in (4, 6), "PlayerMage has alpha", errors)

    spirit_staff = ROOT / "assets/images/SpiritStaff.png"
    check(spirit_staff.exists(), "spirit staff exists", errors)
    if spirit_staff.exists():
        width, height, color_type = png_info(spirit_staff)
        check((width, height) == (1024, 1536),
              "SpiritStaff is 1024x1536", errors)
        check(color_type in (4, 6), "SpiritStaff has alpha", errors)

    character_names = ["Damian", "Evangeline", "Junior", "Lucas"]
    monster_names = ["NormalMonster", "EliteMonster", "Boss"]

    for name in character_names:
        path = ROOT / "assets" / "images" / "characters" / f"{name}.png"
        check(path.exists(), f"character exists: {name}", errors)
        if path.exists():
            width, height, color_type = png_info(path)
            check((width, height) == (1024, 1024),
                  f"{name} is 1024x1024", errors)
            check(color_type in (4, 6), f"{name} has alpha", errors)

    for name in monster_names:
        path = ROOT / "assets" / "images" / "monsters" / f"{name}.png"
        check(path.exists(), f"monster exists: {name}", errors)
        if path.exists():
            width, height, color_type = png_info(path)
            check((width, height) == (1024, 1024),
                  f"{name} is 1024x1024", errors)
            check(color_type in (4, 6), f"{name} has alpha", errors)

    background = ROOT / "assets/images/ui/menu_background.png"
    logo = ROOT / "assets/images/ui/logo.png"
    check(background.exists() and png_info(background)[:2] == (1280, 720),
          "menu background is 1280x720", errors)
    check(logo.exists() and png_info(logo)[2] in (4, 6),
          "logo has transparency", errors)
    icon = ROOT / "assets/images/ui/window_icon.png"
    check(icon.exists() and png_info(icon)[:2] == (256, 256) and
          png_info(icon)[2] in (4, 6),
          "window icon is 256x256 with transparency", errors)
    map_tiles = (
        "GrassTile01", "GrassTile02", "PathTile01", "PathTile02",
        "WallTile01", "WallTile02")
    for name in map_tiles:
        tile = ROOT / "assets/images/map" / f"{name}.png"
        readable = tile.exists() and tile.stat().st_size > 0
        check(readable, f"map tile exists and is non-empty: {name}", errors)
        if readable:
            try:
                width, height, _ = png_info(tile)
                check((width, height) == (tile_size, tile_size),
                      f"map tile is {tile_size}x{tile_size}: {name}", errors)
            except (OSError, ValueError, struct.error):
                check(False, f"map tile is readable PNG: {name}", errors)

    map_source = (ROOT / "src/Map.cpp").read_text(encoding="utf-8-sig")
    required_map_keys = (
        "MapGrass01", "MapGrass02", "MapPath01", "MapPath02",
        "MapWall01", "MapWall02")
    for key in required_map_keys:
        check(key in map_source, f"map texture key is registered: {key}",
              errors)
    check("C:/Project GAME" not in map_source and
          "C:\\Project GAME" not in map_source,
          "map renderer contains no absolute project path", errors)

    music = ["menu_theme.ogg", "gameplay_theme.ogg", "boss_theme.ogg"]
    sounds = [
        "ui_click.wav", "spirit_bolt.wav", "melee_hit.wav", "bow_shot.wav",
        "enemy_death.wav", "radiant_pulse.wav", "ally_skill.wav",
        "wave_start.wav", "boss_spawn.wav", "victory.wav", "defeat.wav",
        "beam_warning.wav", "beam_fire.wav", "upgrade.wav"
    ]
    for name in music:
        path = ROOT / "assets/audio/music" / name
        check(path.exists() and path.read_bytes()[:4] == b"OggS",
              f"valid OGG header: {name}", errors)
    for name in sounds:
        path = ROOT / "assets/audio/sfx" / name
        valid = False
        if path.exists():
            try:
                with wave.open(str(path), "rb") as audio:
                    valid = (audio.getnframes() > 0 and
                             audio.getframerate() == 44_100 and
                             audio.getnchannels() == 1 and
                             audio.getsampwidth() == 2)
            except (wave.Error, EOFError):
                pass
        check(valid, f"valid mono 44.1 kHz PCM16 WAV: {name}", errors)

    ally_source = (ROOT / "src/Ally.cpp").read_text(encoding="utf-8")
    ally_rows = {
        "Damian": "{450.f, 28, 1.75f, 250.f, WeaponType::Bow, 1.f}",
        "Evangeline": "{400.f, 24, 2.00f, 115.f, WeaponType::Sword, 0.75f}",
        "Junior": "{324.f, 17, 1.05f, 220.f, WeaponType::Wand, 1.f}",
        "Lucas": "{360.f, 38, 2.70f, 135.f, WeaponType::Sword, 1.f}",
    }
    for name, row in ally_rows.items():
        check(row in ally_source, f"current ally config contains {name}", errors)

    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    audio_component = re.search(
        r"find_package\s*\(\s*SFML\b.*?\bAudio\b", cmake, re.DOTALL)
    check("SFML::Audio" in cmake and audio_component is not None,
          "CMake links the SFML Audio component", errors)

    print(f"\nValidation result: {len(errors)} error(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
