# Asset attribution

The repository does not currently record the author, source, or license for the files below. These fields must be resolved before submission; no attribution has been invented.

## Font.ttf

- Asset: `fonts/Font.ttf`
- Author/source: TO BE CONFIRMED BEFORE SUBMISSION
- License: TO BE CONFIRMED BEFORE SUBMISSION
- Modified: Unknown
- Used for: UI, story, banners and HUD

## Character images

This entry applies to `images/Damian.png`, `images/Evangeline.png`, `images/Junior.png`, and `images/Lucas.png`.

- Asset: Character PNG files listed above
- Author/source: TO BE CONFIRMED BEFORE SUBMISSION
- License: TO BE CONFIRMED BEFORE SUBMISSION
- Modified: Source performs a non-destructive visible-alpha crop and proportional render scaling; asset files are unchanged
- Used for: Four Ally visuals

## PlayerMage.png

- Asset: `images/PlayerMage.png`
- Author/source: Supplied directly by the project owner for the Player replacement
- License: TO BE CONFIRMED BEFORE SUBMISSION
- Modified: File is unchanged; runtime performs a non-destructive visible-alpha crop and proportional render scaling
- Used for: Player visual (Spirit Warden)

## SpiritStaff.png

- Asset: `images/SpiritStaff.png`
- Author/source: Supplied directly by the project owner
- License: TO BE CONFIRMED BEFORE SUBMISSION
- Modified: File is unchanged; runtime performs a non-destructive visible-alpha crop and proportional render scaling
- Used for: Player Spirit Staff weapon sprite

## Missing presentation assets

Any assets listed in `docs/asset-manifest.md` that are not present in the repository still require a complete attribution entry when supplied.

## Generated sound effects

- Assets: `audio/sfx/ui_click.wav`, `spirit_bolt.wav`, `melee_hit.wav`, `bow_shot.wav`, `enemy_death.wav`, `radiant_pulse.wav`, `ally_skill.wav`, `wave_start.wav`, `boss_spawn.wav`, `victory.wav`, and `defeat.wav`
- Author/source: Generated originally by this project with `tools/generate_audio_assets.py`; no external recordings or downloaded audio were used
- License: Project-owned generated output; confirm the repository's final distribution license before submission
- Format: Mono, 44.1 kHz, PCM 16-bit WAV with deterministic synthesis, fades, and normalized peak level
- Used for: UI and gameplay event sound effects

## Generated fantasy map tiles

- Assets: `images/map/GrassTile01.png`, `GrassTile02.png`, `PathTile01.png`, `PathTile02.png`, `WallTile01.png`, and `WallTile02.png`
- Author/source: Generated originally by this project with `tools/generate_map_tiles.py`; no external artwork or downloaded textures were used
- License: Project-owned generated output; confirm the repository's final distribution license before submission
- Format: Opaque RGB PNG, generated at the `TILE_SIZE` declared in `include/Constants.h`, with a fixed seed for reproducibility
- Used for: Cosmetic rendering of the existing grass, path, spawn, deployment, and wall tile classifications
