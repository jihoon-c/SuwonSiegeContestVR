"""Dumps the actor placement of LV_Ongseong and LV_Ongseong_CombatTest for comparison.

Read-only. Writes JSON to ONGSEONG_PARITY_OUT (default: Saved/OngseongParity.json).

    UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="<this file>" -unattended -nosplash
"""

import json
import os

import unreal

LEVELS = [
    "/GF_OngseongCrossbow/Maps/LV_Ongseong",
    "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest",
]
OUT = os.environ.get("ONGSEONG_PARITY_OUT", os.path.join(unreal.Paths.project_saved_dir(), "OngseongParity.json"))

# Properties worth capturing per class keyword. Missing ones are skipped silently.
INTERESTING = [
    "b_auto_start", "b_maintain_population", "max_concurrent_enemies", "swordsman_slots",
    "archer_slots", "respawn_delay", "b_use_defense_time_limit", "defense_duration",
    "b_return_to_main_on_success", "b_auto_retry_on_failure", "b_lock_player_to_battlement",
    "spawn_point_role", "pooled_actor_class", "pool_size", "b_allow_growth",
    "objective_target", "enemy_pool", "archer_enemy_pool", "archer_projectile_pool",
    "initial_spawn_point", "soldier_respawn_point", "archer_primary_target",
    "gate_actor", "wave_manager", "ram_pool", "ram_spawn_point", "retreat_point",
    "b_play_introduction", "archer_range", "gate_target", "b_player_operable",
]


def prop(actor, name):
    try:
        value = actor.get_editor_property(name)
    except Exception:
        return None
    if isinstance(value, (bool, int, float, str)):
        return value
    if value is None:
        return None
    if isinstance(value, unreal.Object):
        try:
            return value.get_name()
        except Exception:
            return str(value)
    return str(value)


def dump_level(level_path):
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(level_path)
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    entries = []
    for actor in subsystem.get_all_level_actors():
        loc = actor.get_actor_location()
        rot = actor.get_actor_rotation()
        scale = actor.get_actor_scale3d()
        entry = {
            "label": actor.get_actor_label(),
            "level": actor.get_outer().get_name() if actor.get_outer() else "?",
            "class": actor.get_class().get_path_name(),
            "loc": [round(loc.x, 1), round(loc.y, 1), round(loc.z, 1)],
            "rot": [round(rot.pitch, 2), round(rot.yaw, 2), round(rot.roll, 2)],
            "scale": [round(scale.x, 3), round(scale.y, 3), round(scale.z, 3)],
        }
        props = {}
        for name in INTERESTING:
            value = prop(actor, name)
            if value is not None:
                props[name] = value
        if props:
            entry["props"] = props
        entries.append(entry)
    entries.sort(key=lambda e: (e["class"], e["label"]))

    world = unreal.EditorLevelLibrary.get_editor_world()
    settings = world.get_world_settings()
    streaming = []
    try:
        for level in unreal.EditorLevelUtils.get_levels(world):
            outer = level.get_outer()
            streaming.append(outer.get_path_name() if outer else level.get_name())
    except Exception as error:
        streaming.append("ERROR: " + str(error))
    return {
        "level": level_path,
        "actor_count": len(entries),
        "streaming_levels": streaming,
        "game_mode_override": prop(settings, "default_game_mode"),
        "actors": entries,
    }


result = {}
for level in LEVELS:
    result[level] = dump_level(level)

with open(OUT, "w", encoding="utf-8") as handle:
    json.dump(result, handle, ensure_ascii=False, indent=1)
unreal.log("ONGSEONG_PARITY_DUMP SUCCESS -> " + OUT)
