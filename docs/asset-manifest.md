# Eternal Siege asset manifest

All paths are relative to the repository/package root. Missing files are optional at runtime: the presentation loader reports each missing path once and keeps the previous primitive rendering or silence.

## Assets currently present

| Asset | File | Size | Format / transparency | Source use | Status |
| --- | --- | ---: | --- | --- | --- |
| Main font | `assets/fonts/Font.ttf` | 159,108 bytes | TrueType | All states, HUD and banners | Present; load tested |
| Ash | `assets/images/Ash.png` | 1800x1000 | PNG, alpha | Player render | Present; load tested; visible-alpha crop applied |
| Damian | `assets/images/Damian.png` | 2000x1200 | PNG, alpha | Ally render | Present; load tested; visible-alpha crop applied |
| Evangeline | `assets/images/Evangeline.png` | 2000x1200 | PNG, alpha | Ally render | Present; load tested; visible-alpha crop applied |
| Junior | `assets/images/Junior.png` | 387x350 | PNG, alpha | Ally render | Present; load tested; visible-alpha crop applied |
| Lucas | `assets/images/Lucas.png` | 400x504 | PNG, alpha | Ally render | Present; load tested; visible-alpha crop applied |

The author, original source and license of every present file are not recorded in the repository and must be confirmed before submission.

## Integration-ready assets still missing

| Filename | Suggested size | Format | Transparent | Use | Status |
| --- | ---: | --- | :---: | --- | --- |
| `assets/images/ui/logo.png` | 800x300 | PNG | Yes | Intro and menu logo | Missing; text fallback |
| `assets/images/ui/window_icon.png` | 256x256 | PNG | Yes | Window icon | Missing; default OS icon |
| `assets/images/intro/eternal_gate.png` | 1280x720 | PNG | Optional | Intro art | Missing; story text remains complete |
| `assets/images/intro/heroes.png` | 1280x720 | PNG | Yes | Intro heroes art | Missing; story text remains complete |
| `assets/images/monsters/normal.png` | 128x128 | PNG | Yes | Normal monster | Missing; red rectangle fallback |
| `assets/images/monsters/elite.png` | 160x160 | PNG | Yes | Elite monster | Missing; existing elite shape fallback |
| `assets/images/monsters/boss.png` | 256x256 | PNG | Yes | Abyssal Lord | Missing; existing large boss shape/aura fallback |
| `assets/images/map/grass.png` | 48x48 | PNG | No | Ground tile | Missing; old ground color |
| `assets/images/map/path.png` | 48x48 | PNG | No | Walkable path | Missing; old path color |
| `assets/images/map/wall.png` | 48x48 | PNG | No | Blocked wall | Missing; old wall color |
| `assets/images/map/spawn.png` | 48x48 | PNG | Optional | Monster spawn | Missing; old spawn color |
| `assets/images/map/gate.png` | 48x48 | PNG | Optional | Deployment/objective marker | Missing; old deploy color |
| `assets/audio/music/menu_theme.ogg` | 1-3 min loop | OGG | N/A | Intro/menu music | Missing; silence |
| `assets/audio/music/battle_theme.ogg` | 1-3 min loop | OGG | N/A | Waves 1-3 | Missing; silence |
| `assets/audio/music/boss_theme.ogg` | 1-3 min loop | OGG | N/A | Boss phase | Missing; battle track continues if available |
| `assets/audio/sfx/menu_select.wav` | under 2 s | WAV | N/A | Start click | Missing; silence |
| `assets/audio/sfx/wave_start.wav` | under 3 s | WAV | N/A | Wave start | Missing; silence |
| `assets/audio/sfx/boss_spawn.wav` | under 4 s | WAV | N/A | Boss entrance | Missing; silence |
| `assets/audio/sfx/victory.wav` | under 6 s | WAV | N/A | Win | Missing; silence |
| `assets/audio/sfx/game_over.wav` | under 6 s | WAV | N/A | Game Over | Missing; silence |
| `assets/audio/sfx/skill_cast.wav` | under 2 s | WAV | N/A | Ally skill | Missing; hook deferred to avoid combat coupling |
| `assets/audio/sfx/arrow.wav` | under 1 s | WAV | N/A | Arrow | Missing; hook deferred to avoid repeat spam |
| `assets/audio/sfx/sword_hit.wav` | under 1 s | WAV | N/A | Sword hit | Missing; hook deferred to avoid combat coupling |
| `assets/audio/sfx/monster_death.wav` | under 2 s | WAV | N/A | Monster death | Missing; hook deferred to avoid ownership changes |
