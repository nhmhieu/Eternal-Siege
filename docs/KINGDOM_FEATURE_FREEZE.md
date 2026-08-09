# Kingdom Feature Freeze

Kingdom feature development is frozen after the four-stage presentation and systems pass. Its final supported scope is:

1. Flattened-base layered 2.5D map with extracted foreground occluders.
2. Shared surface, collision, and finite height-level data.
3. Foot-based locomotion driven by actual displacement.
4. Stable render-item Y-sort and architectural occlusion.
5. Water-mask-clipped animated water rendering.
6. Continuous day/night logic, world lighting, emissive light zones, and UI isolation.
7. Restrained environmental motion near visible world features.
8. Deterministic NPC phase schedules.
9. Distance-falloff spatial ambience architecture using repository-owned audio only.
10. Clear, light-rain, and mist weather modes.
11. Day/weather color grading. Parallax is omitted because the source world is flattened and shifting extracted duplicates would produce ghosting.
12. Cached assets, frame-boundary transitions, and post-transition clock reset.

Future Kingdom changes are limited to bug/crash fixes, performance, collision correction, balance/visibility, accessibility, and compiler/SFML compatibility. Do not add quests, crafting, expanded inventory, hunger/thirst, mounts, companions, regions, NPC types, Kingdom combat systems, multiplayer, or another feature phase.

The source artwork remains flattened. Foreground pixels are deterministically copied from that source and therefore cannot reveal hidden artwork beneath structures; this is an accepted presentation constraint, not a runtime fallback.
