# Main Level Intro Plan

## Goal

Implement an editable Main-level opening: a full-view establishing shot, a lerped move to the gate/player view, a three-second title with fade-out, then the existing instructor narration.

## Work

1. Add a Main runtime intro actor with movable Overview, Title, and Player arrow anchors.
2. Add a narrowly scoped scenario auto-start deferral hook so only Main can wait for its intro.
3. Place and configure the intro actor in `L_Main`; have it begin the Main scenario after title fade-out.
4. Verify compilation and static map configuration.
