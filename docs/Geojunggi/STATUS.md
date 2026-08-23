# Geojunggi / Nokro — current status

**Verified:** 2026-08-23  
**Overall:** Gameplay-complete prototype; final art and VR-device playtest remain.

## Implemented flow

- `LV_Nokro` contains `BP_NokroScenarioManager`; it spawns the configured crane and four repair targets.
- All 31 `DT_NokroNarration` rows have linked, interaction-gated flow and assigned audio.
- `NK_06` activates only the current damaged-wall marker, shown in yellow.
- The VR handle controls stone height; the held-machine right stick controls boom yaw.
- Both hands must hold the crane grip and press both triggers to request placement.
- Only the active target accepts a stone. Position/yaw tolerance rejects misses and resets the carried stone.
- A successful placement hides the carried stone, reveals a snapped final stone, activates the next target, updates HUD progress, and reports scenario interaction completion.
- The last stone completes the scenario and then reports completion through `ExperienceSubsystem`.
- Reset, success/failure feedback, narration queuing, HUD prompts, and completion delay are implemented.

## Art handoff

No C++ change is required for the remaining visual work.

- Replace crane component meshes/materials in `Content/Gameplay/BP_NokroCrane`.
- Replace `YellowMarker` and `PlacedStone` meshes/materials in `Content/Gameplay/BP_NokroRepairTarget`.
- Implement `On Target Activated FX` and `On Repair Completed FX` in `BP_NokroRepairTarget` with the final Niagara systems.
- Adjust the four default target offsets or place explicit repair-target actors in `LV_Nokro` when the final wall model is installed.

## Verification

- `SuwonSiegeContestVREditor Win64 Development`: passed.
- Unreal automation: 4/4 passed (`PlacementTolerance`, `CraneHeightLimits`, `SuccessFailureLoop`, `ActiveTargetSequence`).
- Asset validation: map manager, three art-ready Blueprint presets, 31 audio assignments, and all narration links passed.

## Remaining work

- Final crane, rope, wall, and stone modeling/material replacement.
- Final target-highlight and placement particle replacement.
- One headset playtest to tune grab radius, height/yaw speed, placement tolerance, and physical scale against the final models.
