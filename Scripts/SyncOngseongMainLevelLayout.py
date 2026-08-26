"""Brings the shipping LV_Ongseong layout in line with the verified LV_Ongseong_CombatTest.

What it copies:
  * the shared Namhansanseong landscape sublevel
  * the transform of every actor that exists in both levels, matched on class + label
  * actors that only the test level has (spawn points, the player start, the extra mountain)
  * the removal of actors that only the shipping level still has (old ground plane, stray enemies)
  * the wave manager and scenario spawn-point wiring

What it deliberately does NOT copy, because the shipping level is the VR build:
  * the GameMode override (BP_OngseongCombatTestGameMode is the non-VR debug mode)
  * DebugCamera_PlayerStart

It also switches the shipping level onto the loading-gated start and takes the BGM out of the
Level Blueprint, where it started at BeginPlay instead of with the assault.

    UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="<this file>" -unattended -nosplash

Idempotent. A copy of the level is written to Saved/CodexBackups before anything is changed.
"""

import os
import shutil

import unreal

SOURCE_LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"
TARGET_LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong"
LANDSCAPE_LEVEL = "/Game/Maps/Main/L_NamhansanseongLandscape"
BACKUP_DIR = os.path.join(unreal.Paths.project_saved_dir(), "CodexBackups", "2026-08-26_OngseongMainLevelParity")
TARGET_UMAP = os.path.join(
    unreal.Paths.project_plugins_dir(),
    "GameFeatures/GF_OngseongCrossbow/Content/Maps/LV_Ongseong.umap")

# Actors owned by the streaming landscape, never by either persistent level.
SUBLEVEL_CLASSES = {"Landscape", "LandscapeStreamingProxy", "WorldDataLayers"}
# Debug-only actors that stay in the test level.
TEST_ONLY_LABELS = {"DebugCamera_PlayerStart"}
# A group's transform is derived from its members, which are moved individually.
DERIVED_TRANSFORM_CLASSES = {"GroupActor", "RecastNavMesh"}


def log(message):
    unreal.log("[LayoutSync] " + str(message))


def actor_subsystem():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def load_level(path):
    if not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(path):
        raise RuntimeError("Could not load " + path)


def persistent_actors(level_path):
    """Only the actors stored in this map's own package; streaming sublevels are excluded."""
    map_name = level_path.split("/")[-1]
    result = []
    for actor in actor_subsystem().get_all_level_actors():
        # Resolve through the owning ULevel, not the package: newly spawned actors and
        # one-file-per-actor levels do not keep the map name in their outermost package.
        level = actor.get_typed_outer(unreal.Level)
        owning_world = level.get_outer() if level else None
        if not owning_world or owning_world.get_name() != map_name:
            continue
        if actor.get_class().get_name() in SUBLEVEL_CLASSES:
            continue
        result.append(actor)
    return result


def key_of(actor):
    return (actor.get_class().get_name(), actor.get_actor_label())


def transform_of(actor):
    return actor.get_actor_transform()


def snapshot_source():
    load_level(SOURCE_LEVEL)
    snapshot = {}
    for actor in persistent_actors(SOURCE_LEVEL):
        if actor.get_actor_label() in TEST_ONLY_LABELS:
            continue
        entry = {
            "class": actor.get_class(),
            "class_name": actor.get_class().get_name(),
            "label": actor.get_actor_label(),
            "transform": transform_of(actor),
            "folder": actor.get_folder_path(),
        }
        if isinstance(actor, unreal.StaticMeshActor):
            component = actor.static_mesh_component
            entry["static_mesh"] = component.get_editor_property("static_mesh")
            entry["materials"] = list(component.get_editor_property("override_materials"))
            entry["mobility"] = component.get_editor_property("mobility")
        role = None
        try:
            role = actor.get_editor_property("spawn_point_role")
        except Exception:
            pass
        if role is not None:
            entry["spawn_point_role"] = role
        snapshot[key_of(actor)] = entry
    log("source snapshot: {0} actors".format(len(snapshot)))
    return snapshot


def backup_target():
    os.makedirs(BACKUP_DIR, exist_ok=True)
    if os.path.isfile(TARGET_UMAP):
        shutil.copy2(TARGET_UMAP, os.path.join(BACKUP_DIR, "LV_Ongseong.umap"))
        log("backed up LV_Ongseong.umap to " + BACKUP_DIR)


def attach_landscape():
    world = unreal.EditorLevelLibrary.get_editor_world()
    for level in unreal.EditorLevelUtils.get_levels(world):
        if level.get_outermost().get_name() == LANDSCAPE_LEVEL:
            log("landscape sublevel already attached")
            return
    streaming = unreal.EditorLevelUtils.add_level_to_world(
        world, LANDSCAPE_LEVEL, unreal.LevelStreamingAlwaysLoaded)
    if not streaming:
        raise RuntimeError("Could not attach " + LANDSCAPE_LEVEL)
    log("attached landscape sublevel " + LANDSCAPE_LEVEL)


def spawn_missing(entry):
    actor = actor_subsystem().spawn_actor_from_class(
        entry["class"], entry["transform"].translation, entry["transform"].rotation.rotator())
    if not actor:
        raise RuntimeError("Could not spawn " + entry["label"])
    actor.set_actor_label(entry["label"])
    actor.set_actor_transform(entry["transform"], False, True)
    folder = entry.get("folder")
    if folder and str(folder) not in ("", "None"):
        actor.set_folder_path(folder)
    if "static_mesh" in entry and isinstance(actor, unreal.StaticMeshActor):
        component = actor.static_mesh_component
        component.set_editor_property("mobility", entry["mobility"])
        component.set_editor_property("static_mesh", entry["static_mesh"])
        component.set_editor_property("override_materials", entry["materials"])
    if "spawn_point_role" in entry:
        actor.set_editor_property("spawn_point_role", entry["spawn_point_role"])
    return actor


def apply_layout(snapshot):
    load_level(TARGET_LEVEL)
    backup_target()

    existing = {}
    for actor in persistent_actors(TARGET_LEVEL):
        existing[key_of(actor)] = actor

    moved = 0
    for key, entry in snapshot.items():
        actor = existing.get(key)
        if not actor:
            continue
        if entry["class_name"] in DERIVED_TRANSFORM_CLASSES:
            continue
        if not actor.get_actor_transform().is_near_equal(entry["transform"], 0.01, 0.01, 0.01):
            actor.set_actor_transform(entry["transform"], False, True)
            moved += 1
    log("aligned {0} transforms".format(moved))

    removed = []
    for key, actor in existing.items():
        if key in snapshot:
            continue
        removed.append(actor.get_actor_label())
        actor_subsystem().destroy_actor(actor)
    log("removed {0}: {1}".format(len(removed), ", ".join(sorted(removed)) or "none"))

    added = []
    for key, entry in snapshot.items():
        if key in existing:
            continue
        spawn_missing(entry)
        added.append(entry["label"])
    log("added {0}: {1}".format(len(added), ", ".join(sorted(added)) or "none"))


def find_one(class_name):
    for actor in persistent_actors(TARGET_LEVEL):
        if actor.get_class().get_name() == class_name:
            return actor
    return None


def spawn_point_with_role(role_name):
    wanted = getattr(unreal.OngseongSpawnPointRole, role_name)
    for actor in persistent_actors(TARGET_LEVEL):
        if not isinstance(actor, unreal.OngseongSpawnPointActor):
            continue
        if actor.get_editor_property("spawn_point_role") == wanted:
            return actor
    return None


def wire_gameplay():
    wave = find_one("OngseongEnemyWaveManager")
    scenario = find_one("BP_OngseongDefenseScenarioManager_C")
    if not wave or not scenario:
        raise RuntimeError("The wave manager or the defense scenario manager is missing")

    initial = spawn_point_with_role("ENEMY_INITIAL")
    respawn = spawn_point_with_role("SOLDIER_RESPAWN")
    ram = spawn_point_with_role("RAM")
    if not (initial and respawn and ram):
        raise RuntimeError("One of the three spawn-point roles is missing")

    wave.set_editor_property("initial_spawn_point", initial)
    wave.set_editor_property("soldier_respawn_point", respawn)
    # The assault is started by the scenario once the trainee has loaded the cannon.
    wave.set_editor_property("auto_start", False)
    scenario.set_editor_property("ram_spawn_point", ram)
    scenario.set_editor_property("start_after_chongtong_loaded", True)
    log("wired spawn points, wave auto-start off, loading-gated start on")


def strip_level_blueprint_bgm(level_path):
    """The BGM used to start at BeginPlay from the Level Blueprint. The scenario owns it now."""
    load_level(level_path)
    map_name = level_path.split("/")[-1]
    blueprint = unreal.load_object(
        None, "{0}.{1}:PersistentLevel.{1}".format(level_path, map_name))
    if not blueprint:
        log("no level script blueprint in " + level_path)
        return

    removed = 0
    for graph in unreal.BlueprintEditorLibrary.list_graphs(blueprint):
        editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
        if not editor:
            continue
        for node in list(editor.list_all_nodes()):
            title = "".join(str(node.get_node_title()).split())
            if "PlaySound2D" in title:
                editor.remove_nodes([node])
                removed += 1
    if removed:
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        log("removed {0} PlaySound2D node(s) from the {1} Level Blueprint".format(removed, map_name))
    else:
        log("no PlaySound2D node in the {0} Level Blueprint".format(map_name))
    save_level(level_path)


def save_level(level_path):
    """
    Saves that one map package. Saving "the current level" is wrong here: attaching a sublevel
    makes the sublevel current, and the shared landscape must never be re-saved by this script.
    """
    if not unreal.EditorAssetLibrary.save_asset(level_path, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + level_path)


snapshot = snapshot_source()
apply_layout(snapshot)
wire_gameplay()
attach_landscape()
save_level(TARGET_LEVEL)
strip_level_blueprint_bgm(TARGET_LEVEL)
strip_level_blueprint_bgm(SOURCE_LEVEL)
log("ONGSEONG_MAIN_LEVEL_LAYOUT_SYNC SUCCESS")
