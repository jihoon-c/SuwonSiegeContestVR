# Main Level Intro Completed

## Implemented flow

`AMainLevelIntroActor` in `L_Main` runs this sequence at game start:

```text
OverviewAnchor
  → lerp (Overview To Title Duration)
TitleAnchor
  → title visible for 3 seconds → fade-out
PlayerAnchor
  → AMainEducationScenarioManagerActor.StartEducationAfterIntro()
  → existing instructor narration / Main scenario
```

`TitleAnchor` and `PlayerAnchor` start at the same final gate view, matching the requested default. Move `TitleAnchor` independently if a separate title composition is desired.

## Editor controls

Select `MainLevelIntro_CastleToGate` in `L_Main`.

- Move/rotate the blue `OverviewAnchor`, yellow `TitleAnchor`, and green `PlayerAnchor` arrows directly in the viewport.
- Adjust the four timing properties under `Main Intro > Timing`.
- Edit title/subtitle text, color, scale, and camera-relative offsets under `Main Intro > Title`.
- Assign a different manager only through `Target Education Manager`; the Main manager's `Wait For Intro Sequence` must remain enabled.

## Authoring caveat

The current shared Namhansanseong sublevel contains terrain only. The overview camera path is ready, but a literal full castle view requires castle/gate mesh actors to be present at the chosen anchor compositions.

## Validation

- Build `SuwonSiegeContestVREditor`.
- Run `Scripts/ConfigureMainLevelIntro.py` with Unreal Editor Python.
- Run `Scripts/VerifyMainLevelIntro.py` with Unreal Editor Python.
