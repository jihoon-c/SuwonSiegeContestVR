# Singijeon Enemy Wave VR Visibility Fix Plan

## Goal

Restore the Singijeon gameplay actors to the player play-space and place them on the imported Namhansanseong terrain so the enemy wave is visible in VR.

## Steps

1. Inspect the placed wave, Hwacha, and player-start coordinates in `LV_Singijeon`. Completed.
2. Restore the authored gameplay anchors and project them to Landscape collision height. Completed.
3. Re-run the runtime representation audit and confirm the 45 VR-safe enemy actors are visible and near the wave anchor. Completed.
