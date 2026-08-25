# Singijeon Enemy Wave VR Visibility Fix

## Cause

The wave, Hwacha, arrow, torch, and fire pit were stored around `(-44,000, -44,000)` while `PlayerStart` remained near the Singijeon play-space origin. The VR player therefore never reached the wave; this was a level-placement issue, not a skeletal-mesh or OpenXR rendering failure.

## Applied fix

`FixSingijeonGameplayPlacementAfterLandscape.py` restores the authored local anchors and projects each actor onto the streamed Namhansanseong Landscape collision surface. The wave is at `(1110, 205, 334.7)` and targets the Hwacha at `(395, 58, 363.2)`.

The transient `SkeletalMeshActor` background path was replaced with the same `EnemySoldierActor` rendering path used by the foreground targets. All 45 actors have collision, movement tick, actor tick, shadows, and navigation contribution disabled where not needed.

The Wave now defaults to the project's standard `SKM_Manny_Simple` mesh. The Samurai-only run animation and GPU animation provider are cleared from the placed Wave because they target a different skeleton.

## Changed files

- `Plugins/GameFeatures/GF_Singijeon/Content/Maps/LV_Singijeon.umap`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Public/Enemy/SingijeonEnemyWaveActor.h`
- `Plugins/GameFeatures/GF_Singijeon/Source/GF_Singijeon/Private/Enemy/SingijeonEnemyWaveActor.cpp`
- `Scripts/FixSingijeonGameplayPlacementAfterLandscape.py`
- `Scripts/InspectSingijeonEnemyVisibility.py`

## Verification

`InspectSingijeonEnemyVisibility.py` after a fresh map reload confirmed:

- 45 enemies are prepared.
- 45 VR-safe `EnemySoldierActor` instances are created; they use the same actor render path as the previously visible foreground targets.
- Their mesh components are visible, not hidden in game, and spawn within 141 cm of the wave anchor.
- `SuwonSiegeContestVREditor Win64 Development` build succeeded.
