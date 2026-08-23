"""Creates the non-VR Ongseong combat test GameMode, its test level and the pooling setup.

Run inside the Unreal Editor (Output Log -> Cmd -> "py <path>") or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecCmds="py <this file>;Quit" -unattended -nosplash

Requires the editor target to be built first so the Core debug classes are loaded.
The script is idempotent and never modifies LV_Ongseong itself.
"""

import unreal

FEATURE_BLUEPRINTS = "/GF_OngseongCrossbow/Blueprints"
DEBUG_FOLDER = FEATURE_BLUEPRINTS + "/Debug"
SOURCE_LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong"
TEST_LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"
TEST_GAMEMODE = DEBUG_FOLDER + "/BP_OngseongCombatTestGameMode"
RAM_BLUEPRINT = FEATURE_BLUEPRINTS + "/BP_OngseongRam"

CORE_MODULE = "/Script/SuwonSiegeContestVR."
FEATURE_MODULE = "/Script/GF_OngseongCrossbow."

# Concurrency budget from docs/OngseongCrossbow/plans/2026-08-24_POOLED_INSTANCES_AND_FX.md
SWORDSMAN_POOL_SIZE = 9
ARCHER_POOL_SIZE = 8
ARROW_POOL_SIZE = 24
RAM_POOL_SIZE = 2

asset_lib = unreal.EditorAssetLibrary
PROBLEMS = []


def log(message):
    unreal.log("[OngseongCombatTest] " + message)


def warn(message):
    PROBLEMS.append(message)
    unreal.log_warning("[OngseongCombatTest] " + message)


def load_blueprint_class(path):
    try:
        generated = asset_lib.load_blueprint_class(path)
        if generated:
            return generated
    except Exception:
        pass
    return unreal.load_class(None, "{0}.{1}_C".format(path, path.rsplit("/", 1)[1]))


def set_prop(target, names, value):
    """UE Python strips the leading 'b' from bool properties; try both spellings."""
    for name in names:
        try:
            target.set_editor_property(name, value)
            return True
        except Exception:
            continue
    warn("Could not set {0} on {1}".format(names[0], target.get_name()))
    return False


def create_blueprint(path, parent_path):
    if asset_lib.does_asset_exist(path):
        log("Blueprint already exists: " + path)
        return
    parent = unreal.load_class(None, parent_path)
    if not parent:
        raise RuntimeError(parent_path + " is not loaded. Build the editor target and restart the editor first.")
    folder, name = path.rsplit("/", 1)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.Blueprint, factory)
    if not blueprint:
        raise RuntimeError("Failed to create " + path)
    asset_lib.save_asset(path)
    log("Created blueprint: " + path)


def duplicate_level():
    if asset_lib.does_asset_exist(TEST_LEVEL):
        log("Test level already exists: " + TEST_LEVEL)
        return
    if not asset_lib.does_asset_exist(SOURCE_LEVEL):
        raise RuntimeError("Missing source level: " + SOURCE_LEVEL)
    if not asset_lib.duplicate_asset(SOURCE_LEVEL, TEST_LEVEL):
        raise RuntimeError("Failed to duplicate " + SOURCE_LEVEL)
    asset_lib.save_asset(TEST_LEVEL)
    log("Duplicated level: " + TEST_LEVEL)


def actor_subsystem():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def get_actors_of_class(class_path):
    actor_class = unreal.load_class(None, class_path)
    if not actor_class:
        return []
    return unreal.EditorFilterLibrary.by_class(actor_subsystem().get_all_level_actors(), actor_class)


def find_pool(tag=None, pooled_class_name=None):
    """Pools are identified by their gameplay tag first, then by the class they pool."""
    pools = get_actors_of_class(CORE_MODULE + "ActorPool")
    if tag:
        for pool in pools:
            if pool.actor_has_tag(tag):
                return pool
    if pooled_class_name:
        for pool in pools:
            pooled_class = pool.get_editor_property("pooled_actor_class")
            if pooled_class and pooled_class_name.lower() in str(pooled_class.get_name()).lower():
                return pool
    return None


def set_world_gamemode():
    gamemode_class = load_blueprint_class(TEST_GAMEMODE)
    if not gamemode_class:
        warn("Could not load the test GameMode class. Set it manually in World Settings.")
        return
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    settings = None
    try:
        settings = world.get_world_settings()
    except Exception:
        found = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WorldSettings)
        settings = found[0] if found else None
    if not settings:
        warn("Could not reach the World Settings actor. Set the GameMode override manually.")
        return
    settings.set_editor_property("default_game_mode", gamemode_class)
    log("GameMode override set on " + TEST_LEVEL)


def configure_pools():
    swordsman_pool = find_pool(tag="Ongseong.SwordsmanPool", pooled_class_name="Sword")
    archer_pool = find_pool(tag="Ongseong.ArcherPool", pooled_class_name="Archer")
    arrow_pool = find_pool(tag="Ongseong.ArrowPool", pooled_class_name="Arrow")
    for pool, size, label in (
        (swordsman_pool, SWORDSMAN_POOL_SIZE, "swordsman"),
        (archer_pool, ARCHER_POOL_SIZE, "archer"),
        (arrow_pool, ARROW_POOL_SIZE, "arrow"),
    ):
        if pool:
            set_prop(pool, ["initial_pool_size"], size)
            set_prop(pool, ["allow_pool_expansion", "b_allow_pool_expansion"], False)
            log("Sized {0} pool ({1}) to {2}".format(label, pool.get_actor_label(), size))
        else:
            warn("Could not find the {0} pool. Size it manually.".format(label))


def configure_ram_pool():
    ram_pool = find_pool(tag="Ongseong.RamPool", pooled_class_name="Ram")
    if not ram_pool:
        pool_class = unreal.load_class(None, CORE_MODULE + "ActorPool")
        ram_pool = actor_subsystem().spawn_actor_from_class(pool_class, unreal.Vector(0.0, 0.0, 0.0))
        ram_pool.set_actor_label("Ram_ActorPool")
        ram_pool.tags = ["Ongseong.RamPool"]
        log("Spawned Ram_ActorPool")
    ram_class = load_blueprint_class(RAM_BLUEPRINT)
    if ram_class:
        set_prop(ram_pool, ["pooled_actor_class"], ram_class)
    else:
        warn("Could not load " + RAM_BLUEPRINT + ". Assign the pooled class manually.")
    set_prop(ram_pool, ["initial_pool_size"], RAM_POOL_SIZE)
    set_prop(ram_pool, ["allow_pool_expansion", "b_allow_pool_expansion"], False)
    return ram_pool


# The ram must start far enough away for its slow approach to be visible and shootable.
RAM_APPROACH_DISTANCE = 3000.0


def ensure_ram_spawn_point(scenario):
    """Places the ram outside the ongseong, on the same side the enemies retreat to."""
    existing = scenario.get_editor_property("ram_spawn_point")
    if existing:
        log("Ram spawn point already set: " + existing.get_actor_label())
        return
    gates = get_actors_of_class(FEATURE_MODULE + "OngseongGateActor")
    retreat = scenario.get_editor_property("retreat_point")
    if not gates or not retreat:
        warn("Need a gate and a retreat point to place the ram spawn. Set RamSpawnPoint manually.")
        return
    gate_location = gates[0].get_actor_location()
    direction = unreal.Vector(
        retreat.get_actor_location().x - gate_location.x,
        retreat.get_actor_location().y - gate_location.y,
        0.0)
    length = direction.length()
    if length < 1.0:
        warn("The retreat point sits on the gate. Set RamSpawnPoint manually.")
        return
    location = unreal.Vector(
        gate_location.x + direction.x / length * RAM_APPROACH_DISTANCE,
        gate_location.y + direction.y / length * RAM_APPROACH_DISTANCE,
        gate_location.z)
    point = actor_subsystem().spawn_actor_from_class(unreal.TargetPoint, location)
    point.set_actor_label("Ram_SpawnPoint")
    set_prop(scenario, ["ram_spawn_point"], point)
    log("Placed Ram_SpawnPoint {0} cm from the gate".format(int(RAM_APPROACH_DISTANCE)))


def configure_scenarios(ram_pool):
    scenarios = get_actors_of_class(FEATURE_MODULE + "OngseongDefenseScenarioManager")
    if not scenarios:
        warn("No OngseongDefenseScenarioManager found in the level.")
    for scenario in scenarios:
        # The test level must never hand control back to the Main experience.
        set_prop(scenario, ["return_to_main_on_success", "b_return_to_main_on_success"], False)
        if ram_pool:
            set_prop(scenario, ["ram_pool"], ram_pool)
        ensure_ram_spawn_point(scenario)
        log("Configured " + scenario.get_actor_label())


def ensure_player_start():
    """The observer camera spawns at the PlayerStart. Without one it appears at the world origin."""
    starts = get_actors_of_class("/Script/Engine.PlayerStart")
    if starts:
        log("PlayerStart already present: " + starts[0].get_actor_label())
        return
    cannons = get_actors_of_class(FEATURE_MODULE + "ChongtongCannonActor")
    gates = get_actors_of_class(FEATURE_MODULE + "OngseongGateActor")
    if not cannons:
        warn("No chongtong found; place a PlayerStart for the observer camera manually.")
        return
    location = cannons[0].get_actor_location()
    location = unreal.Vector(location.x, location.y, location.z + 250.0)
    rotation = unreal.Rotator(0.0, 0.0, 0.0)
    if gates:
        rotation = unreal.MathLibrary.find_look_at_rotation(location, gates[0].get_actor_location())
    start = actor_subsystem().spawn_actor_from_class(unreal.PlayerStart, location, rotation)
    start.set_actor_label("DebugCamera_PlayerStart")
    log("Spawned DebugCamera_PlayerStart near " + cannons[0].get_actor_label())


def configure_test_level():
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_subsystem.load_level(TEST_LEVEL)
    set_world_gamemode()
    configure_scenarios(configure_ram_pool())
    configure_pools()
    ensure_player_start()
    level_subsystem.save_current_level()
    log("Saved " + TEST_LEVEL)


def main():
    create_blueprint(TEST_GAMEMODE, CORE_MODULE + "DebugFreeCameraGameMode")
    duplicate_level()
    configure_test_level()
    if PROBLEMS:
        unreal.log_warning("[OngseongCombatTest] Finished with {0} item(s) needing manual attention:".format(len(PROBLEMS)))
        for problem in PROBLEMS:
            unreal.log_warning("[OngseongCombatTest]   - " + problem)
    else:
        log("Done. Open LV_Ongseong_CombatTest and press Play (not VR Preview).")


main()

# Headless runs pass -unattended; quit so the commandlet does not hang after the work is done.
if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
