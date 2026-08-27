"""Authors the assets behind the gated Ongseong start: the war-horn cue, the reordered
instructor narration and the Blueprint defaults that expose the horn and the battle music.

Run inside the Unreal Editor (Output Log -> Cmd -> "py <path>") or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="<this file>" -unattended -nosplash

Idempotent.
"""

import json

import unreal

SOUND_FOLDER = "/GF_OngseongCrossbow/Asset/Sound"
HORN_WAVE = SOUND_FOLDER + "/WarHorn"
HORN_CUE = SOUND_FOLDER + "/SC_WarHorn"
BATTLE_MUSIC_CUE = "/GF_OngseongCrossbow/Asset/Sound/BGM/SC_BGM"
SCENARIO_BLUEPRINT = "/GF_OngseongCrossbow/Blueprints/BP_OngseongDefenseScenarioManager"
NARRATION_TABLE = "/GF_OngseongCrossbow/Data/DT_OngseongNarration"

# The main level now teaches the loading drill before the assault, so the instructor chain is:
#   entry      ON_01..ON_07 -> ON_09 -> ON_12 -> ON_13   (then the loading events take over)
#   loading    PowderLoaded ON_14, RammingCompleted ON_15, ReadyToAim ON_16 -> ON_17
#   briefing   TrainingCompleted ON_08  ("지금부터 실제 상황을 가정한 수비 훈련을 시작하겠습니다")
#   assault    WaveStarted ON_10 -> ON_11
#   clear      DefenseSucceeded ON_22
NEXT_ROWS = {
    "ON_01": "ON_02",
    "ON_02": "ON_03",
    "ON_03": "ON_04",
    "ON_04": "ON_05",
    "ON_05": "ON_06",
    "ON_06": "ON_07",
    "ON_07": "ON_09",
    "ON_08": None,
    "ON_09": "ON_12",
    "ON_10": "ON_11",
    "ON_11": None,
    "ON_12": "ON_13",
    "ON_13": None,
    "ON_16": "ON_17",
}


def log(message):
    unreal.log("[AssaultStart] " + str(message))


def save(asset):
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + asset.get_path_name())


def resolve_horn_sound():
    """
    Prefers an authored SC_WarHorn Sound Cue and falls back to the raw WarHorn wave.

    A Sound Cue cannot be authored from Python: USoundCue::AllNodes is protected, so a
    script-built cue would open with an empty graph and lose its node on the next editor save.
    The scenario property is a USoundBase, so drop an SC_WarHorn next to the wave and re-run
    this script to promote it.
    """
    cue = unreal.load_asset(HORN_CUE)
    if cue:
        log("using authored SC_WarHorn")
        return cue

    wave = unreal.load_asset(HORN_WAVE)
    if not wave:
        raise RuntimeError("WarHorn sound is missing at " + HORN_WAVE)
    log("SC_WarHorn not authored yet; defaulting the horn to the WarHorn wave")
    return wave


def rewire_narration():
    table = unreal.load_asset(NARRATION_TABLE)
    if not table:
        raise RuntimeError("DT_OngseongNarration is missing")

    # Round-tripping through JSON keeps the imported NarrationSound and subtitle of every row;
    # only the flow columns are rewritten.
    rows = json.loads(table.export_to_json_string())
    by_name = {row["Name"]: row for row in rows}
    missing = sorted(set(NEXT_ROWS) - set(by_name))
    if missing:
        raise RuntimeError("Narration rows are missing: " + ", ".join(missing))

    changed = []
    for row_name, next_row in NEXT_ROWS.items():
        row = by_name[row_name]
        wanted_next = next_row or "None"
        if str(row.get("NextRow", "None")) == wanted_next:
            continue
        changed.append("{0}: {1} -> {2}".format(row_name, row.get("NextRow", "None"), wanted_next))
        row["NextRow"] = wanted_next
        row["AdvanceMode"] = "Auto" if next_row else "Stop"
        row["AdvanceDelay"] = 0.15 if next_row else 0.0

    if not changed:
        log("narration chain already correct")
        return
    if not table.fill_from_json_string(json.dumps(rows, ensure_ascii=False)):
        raise RuntimeError("Could not write the rewired narration rows back")
    save(table)
    for line in changed:
        log("narration " + line)


def configure_scenario_blueprint():
    blueprint = unreal.load_asset(SCENARIO_BLUEPRINT)
    if not blueprint:
        raise RuntimeError("BP_OngseongDefenseScenarioManager is missing")

    horn = resolve_horn_sound()
    music = unreal.load_asset(BATTLE_MUSIC_CUE)
    if not music:
        raise RuntimeError("SC_BGM is missing at " + BATTLE_MUSIC_CUE)

    defaults = unreal.get_default_object(blueprint.generated_class())
    defaults.set_editor_property("assault_horn_sound", horn)
    defaults.set_editor_property("battle_music", music)
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    save(blueprint)
    log("BP defaults: horn={0}, music={1}".format(horn.get_name(), music.get_name()))


rewire_narration()
configure_scenario_blueprint()
log("ONGSEONG_ASSAULT_START_ASSETS SUCCESS")
