"""Checks the shipping Ongseong level against the test level and against the gated-start setup.

Read-only. Prints PASS/FAIL lines and raises at the end if anything failed.

    UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="<this file>" -unattended -nosplash
"""

import json

import unreal

SOURCE_LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"
TARGET_LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong"
LANDSCAPE_LEVEL = "/Game/Maps/Main/L_NamhansanseongLandscape"
SCENARIO_BLUEPRINT = "/GF_OngseongCrossbow/Blueprints/BP_OngseongDefenseScenarioManager"
NARRATION_TABLE = "/GF_OngseongCrossbow/Data/DT_OngseongNarration"

SUBLEVEL_CLASSES = {"Landscape", "LandscapeStreamingProxy", "WorldDataLayers"}
TEST_ONLY_LABELS = {"DebugCamera_PlayerStart"}
# A GroupActor's transform is the cached centre of its members, recomputed by the editor.
# The members themselves are compared, so comparing the cached centre would be noise.
DERIVED_TRANSFORM_CLASSES = {"GroupActor", "RecastNavMesh"}

EXPECTED_NEXT_ROWS = {
    "ON_07": "ON_09", "ON_08": "None", "ON_09": "ON_12", "ON_10": "ON_11",
    "ON_11": "None", "ON_12": "ON_13", "ON_13": "None", "ON_16": "ON_17",
}

failures = []


def check(condition, message):
    unreal.log("[Verify] {0} {1}".format("PASS" if condition else "FAIL", message))
    if not condition:
        failures.append(message)


def load_level(path):
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(path)


def persistent_actors(level_path):
    map_name = level_path.split("/")[-1]
    result = []
    for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        level = actor.get_typed_outer(unreal.Level)
        owning_world = level.get_outer() if level else None
        if not owning_world or owning_world.get_name() != map_name:
            continue
        if actor.get_class().get_name() in SUBLEVEL_CLASSES:
            continue
        result.append(actor)
    return result


def layout_of(level_path):
    load_level(level_path)
    layout = {}
    for actor in persistent_actors(level_path):
        if actor.get_actor_label() in TEST_ONLY_LABELS:
            continue
        transform = actor.get_actor_transform()
        layout[(actor.get_class().get_name(), actor.get_actor_label())] = (
            [round(v, 1) for v in (transform.translation.x, transform.translation.y, transform.translation.z)],
            [round(v, 1) for v in (transform.rotation.rotator().pitch,
                                   transform.rotation.rotator().yaw,
                                   transform.rotation.rotator().roll)],
            [round(v, 3) for v in (transform.scale3d.x, transform.scale3d.y, transform.scale3d.z)],
        )
    return layout


def level_blueprint_has_playsound(level_path):
    map_name = level_path.split("/")[-1]
    blueprint = unreal.load_object(None, "{0}.{1}:PersistentLevel.{1}".format(level_path, map_name))
    if not blueprint:
        return False
    for graph in unreal.BlueprintEditorLibrary.list_graphs(blueprint):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        if not editor:
            continue
        for node in editor.list_all_nodes():
            if "PlaySound2D" in "".join(str(node.get_node_title()).split()):
                return True
    return False


# 1. Layout parity, excluding the debug-only actors.
source_layout = layout_of(SOURCE_LEVEL)
source_has_playsound = level_blueprint_has_playsound(SOURCE_LEVEL)
target_layout = layout_of(TARGET_LEVEL)

only_target = sorted(set(target_layout) - set(source_layout))
only_source = sorted(set(source_layout) - set(target_layout))
check(not only_target, "no shipping-only actors left over: " + str(only_target))
check(not only_source, "every test-level actor is present: " + str(only_source))

shared = set(source_layout) & set(target_layout)
mismatched = [key for key in shared
              if key[0] not in DERIVED_TRANSFORM_CLASSES and source_layout[key] != target_layout[key]]
check(not mismatched, "{0} shared actors match transforms (mismatched: {1})".format(
    len(shared), mismatched[:5]))

# 2. The shared landscape is streamed in, and the VR GameMode is untouched.
world = unreal.EditorLevelLibrary.get_editor_world()
streamed = [level.get_outer().get_name() for level in unreal.EditorLevelUtils.get_levels(world)]
check(LANDSCAPE_LEVEL.split("/")[-1] in streamed, "landscape sublevel attached: " + str(streamed))
game_mode = world.get_world_settings().get_editor_property("default_game_mode")
check(game_mode is None, "no debug GameMode override on the shipping level (found {0})".format(game_mode))

# 3. Gameplay wiring in the shipping level.
actors = persistent_actors(TARGET_LEVEL)
wave = next((a for a in actors if a.get_class().get_name() == "OngseongEnemyWaveManager"), None)
scenario = next((a for a in actors if a.get_class().get_name() == "BP_OngseongDefenseScenarioManager_C"), None)
check(wave is not None and scenario is not None, "wave manager and scenario manager are placed")
if wave and scenario:
    check(wave.get_editor_property("initial_spawn_point") is not None, "wave InitialSpawnPoint wired")
    check(wave.get_editor_property("soldier_respawn_point") is not None, "wave SoldierRespawnPoint wired")
    check(wave.get_editor_property("auto_start") is False, "wave auto-start is off")
    check(scenario.get_editor_property("ram_spawn_point") is not None, "scenario RamSpawnPoint wired")
    check(scenario.get_editor_property("start_after_chongtong_loaded") is True,
          "scenario waits for the chongtong to be loaded")
    check(abs(scenario.get_editor_property("assault_start_delay") - 2.0) < 0.001,
          "assault starts 2s after the briefing")

roles = sorted(str(a.get_editor_property("spawn_point_role"))
               for a in actors if isinstance(a, unreal.OngseongSpawnPointActor))
check(len(roles) == 3 and len(set(roles)) == 3, "three distinct spawn-point roles: " + str(roles))

# 4. Blueprint audio slots.
blueprint = unreal.load_asset(SCENARIO_BLUEPRINT)
defaults = unreal.get_default_object(blueprint.generated_class())
horn = defaults.get_editor_property("assault_horn_sound")
music = defaults.get_editor_property("battle_music")
check(horn is not None, "assault horn assigned: " + str(horn.get_name() if horn else None))
check(music is not None, "battle music assigned: " + str(music.get_name() if music else None))

# 5. Narration flow and preserved audio.
rows = {row["Name"]: row for row in json.loads(unreal.load_asset(NARRATION_TABLE).export_to_json_string())}
bad_chain = {name: rows[name].get("NextRow") for name, expected in EXPECTED_NEXT_ROWS.items()
             if str(rows[name].get("NextRow", "None")) != expected}
check(not bad_chain, "narration chain rewired: " + str(bad_chain))
silent = sorted(name for name, row in rows.items()
                if str(row.get("NarrationSound", "None")) in ("", "None"))
check(not silent, "every narration row kept its recorded audio (silent: {0})".format(silent))

# 6. The BGM no longer starts from a Level Blueprint.
check(not source_has_playsound, "no PlaySound2D in the LV_Ongseong_CombatTest Level Blueprint")
check(not level_blueprint_has_playsound(TARGET_LEVEL), "no PlaySound2D in the LV_Ongseong Level Blueprint")

unreal.log("[Verify] {0} check(s) failed".format(len(failures)))
if failures:
    raise RuntimeError("ONGSEONG_MAIN_LEVEL_FLOW_VERIFY FAILED: " + "; ".join(failures))
unreal.log("ONGSEONG_MAIN_LEVEL_FLOW_VERIFY SUCCESS")
