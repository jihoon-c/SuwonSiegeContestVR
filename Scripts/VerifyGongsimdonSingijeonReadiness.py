from pathlib import Path
import runpy


SCRIPT_DIR = Path(__file__).resolve().parent
VERIFIERS = [
    "VerifyGongsimdonActionInteractions.py",
    "VerifyGongsimdonEnemyGroup.py",
    "VerifySingijeonScenarioFlow.py",
    "VerifySingijeonSingleHandTargetMove.py",
    "VerifySingijeonVisualFeedback.py",
    "VerifySingijeonCarryFuse.py",
    "VerifySingijeonHwachaDragVolley.py",
    "VerifySingijeonFirePitAndAmmoVisualFixes.py",
    "VerifySingijeonEnemyWave.py",
]

for verifier in VERIFIERS:
    runpy.run_path(str(SCRIPT_DIR / verifier), run_name=f"__verify_{verifier}__")

print("GONGSIMDON_SINGIJEON_READINESS VERIFY SUCCESS")
