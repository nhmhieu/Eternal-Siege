# Asset attribution

This document records the provenance and attribution state for all external and generated visual and audio assets in the **Eternal Siege** repository.

## Font.ttf

- Asset: `fonts/Font.ttf`
- Author/source: Original source was not recorded in the repository.
- License: Not recorded in the repository.
- Modified: File used as provided for UI, story frames, title banners, and HUD rendering.
- Used for: UI, story frames, title banners, and HUD rendering.

## Character images

This entry applies to `images/Damian.png`, `images/Evangeline.png`, `images/Junior.png`, and `images/Lucas.png`.

- Asset: Character PNG files listed above
- Author/source: Original source was not recorded in the repository.
- License: Not recorded in the repository.
- Modified: Source performs a non-destructive visible-alpha crop and proportional render scaling; asset files are unchanged.
- Used for: Four Ally visuals.

## PlayerMage.png

- Asset: `images/PlayerMage.png`
- Author/source: Supplied directly by the project team for the Player character visual.
- License: Project-supplied asset.
- Modified: File is unchanged; runtime performs a non-destructive visible-alpha crop and proportional render scaling.
- Used for: Player visual (Spirit Warden).

## SpiritStaff.png

- Asset: `images/SpiritStaff.png`
- Author/source: Supplied directly by the project team.
- License: Project-supplied asset.
- Modified: File is unchanged; runtime performs a non-destructive visible-alpha crop and proportional render scaling.
- Used for: Player Spirit Staff weapon sprite.

## Generated sound effects

- Assets: `audio/sfx/ui_click.wav`, `spirit_bolt.wav`, `melee_hit.wav`, `bow_shot.wav`, `enemy_death.wav`, `radiant_pulse.wav`, `ally_skill.wav`, `wave_start.wav`, `boss_spawn.wav`, `victory.wav`, and `defeat.wav`
- Author/source: Generated during development by project-owned synthesis scripts; no external recordings or downloaded audio were used.
- License: Project-owned generated output.
- Format: Mono, 44.1 kHz, PCM 16-bit WAV with deterministic synthesis, fades, and normalized peak level.
- Used for: UI and gameplay event sound effects.

## Generated fantasy map tiles

- Assets: `images/map/GrassTile01.png`, `GrassTile02.png`, `PathTile01.png`, `PathTile02.png`, `WallTile01.png`, and `WallTile02.png`
- Author/source: Generated during development by project-owned generation scripts; no external artwork or downloaded textures were used.
- License: Project-owned generated output.
- Format: Opaque RGB PNG, generated with a fixed seed for reproducibility.
- Used for: Cosmetic rendering of grass, path, spawn, deployment, and wall tile classifications.
