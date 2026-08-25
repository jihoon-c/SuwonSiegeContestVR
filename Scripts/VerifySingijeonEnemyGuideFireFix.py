import os
import runpy

import unreal


SCRIPT_DIR = os.path.join(unreal.Paths.project_dir(), "Scripts")
for script_name in (
    "VerifySingijeonEnemyWave.py",
    "VerifySingijeonCarryFuse.py",
    "VerifyScenarioInteractionGuidesAndSubtitle.py",
):
    unreal.log(f"SINGIJEON_FIX VERIFY RUN: {script_name}")
    runpy.run_path(os.path.join(SCRIPT_DIR, script_name), run_name="__main__")

unreal.log("SINGIJEON_ENEMY_GUIDE_FIRE_FIX VERIFY SUCCESS")
