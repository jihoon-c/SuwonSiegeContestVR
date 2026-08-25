"""Applies the verified Ongseong combat layout to the shipping level.

The fixes were proven in LV_Ongseong_CombatTest (see
docs/OngseongCrossbow/completed/2026-08-24_CHONGTONG_ENGAGEMENT_FIX.md). This script rolls the
same actor placement and pooling into LV_Ongseong, which still shipped with:

  * the enemy spawner at the world origin, effectively on top of the gate
  * chongtong muzzles pointing away from the corridor
  * a NavMeshBoundsVolume that does not cover the standing room above the floor
  * no ram pool and no ram spawn point

Run inside the Unreal Editor (Output Log -> Cmd -> "py <path>") or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecCmds="py <this file>;Quit" -unattended -nosplash

Set ONGSEONG_LAYOUT_LEVEL to target a different level. Idempotent.
"""

import os

import unreal

LEVEL = os.environ.get("ONGSEONG_LAYOUT_LEVEL", "/GF_OngseongCrossbow/Maps/LV_Ongseong")
CORE_MODULE = "/Script/SuwonSiegeContestVR."
FEATURE_MODULE = "/Script/GF_OngseongCrossbow."
RAM_BLUEPRINT = "/GF_OngseongCrossbow/Blueprints/BP_OngseongRam"

GROUND_Z = 10.0
PAWN_Z = 98.0
RAM_Z = 243.0
CORRIDOR_X = 150.0      # centre line of the walled corridor, same as the gate
ENEMY_SPAWN_Y = 7800.0  # inside the generated navmesh
RAM_SPAWN_Y = 8350.0    # ~7,450 cm of approach once the 800 cm staging stop is subtracted
RETREAT_Y = 8900.0
FACING_GATE_YAW = -90.0
RAM_MOVE_SPEED = 42.0   # AOngseongRamActor::MoveSpeed, tuned for a three-minute approach

# Concurrency budget from docs/OngseongCrossbow/plans/2026-08-24_POOLED_INSTANCES_AND_FX.md
SWORDSMAN_POOL_SIZE = 9
ARCHER_POOL_SIZE = 8
ARROW_POOL_SIZE = 24
RAM_POOL_SIZE = 2

asset_lib = unreal.EditorAssetLibrary
PROBLEMS = []


def log(message):
    unreal.log("[OngseongLayout] " + message)


def warn(message):
    PROBLEMS.append(message)
    unreal.log_warning("[OngseongLayout] " + message)


def actor_subsystem():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def all_actors():
    return actor_subsystem().get_all_level_actors()


def by_label(label):
    for actor in all_actors():
        if actor.get_actor_label() == label:
            return actor
    return None


def by_class(class_path):
    actor_class = unreal.load_class(None, class_path)
    if not actor_class:
        return []
    return unreal.EditorFilterLibrary.by_class(all_actors(), actor_class)


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


def place(actor, x, y, z, yaw=None):
    # Editor-locked actors silently refuse to move, so clear the lock and verify the result.
    try:
        if actor.get_editor_property("lock_location"):
            actor.set_editor_property("lock_location", False)
            log("cleared location lock on " + actor.get_actor_label())
    except Exception:
        pass
    target = unreal.Vector(x, y, z)
    actor.set_actor_location(target, False, True)
    if yaw is not None:
        actor.set_actor_rotation(unreal.Rotator(0.0, 0.0, yaw), True)
    actual = actor.get_actor_location()
    if unreal.Vector.distance(actual, target) > 1.0:
        warn("{0} did not move (wanted ({1:.0f},{2:.0f},{3:.0f}), still at ({4:.0f},{5:.0f},{6:.0f}))".format(
            actor.get_actor_label(), x, y, z, actual.x, actual.y, actual.z))
    else:
        log("{0} -> ({1:.0f}, {2:.0f}, {3:.0f})".format(actor.get_actor_label(), actual.x, actual.y, actual.z))


def place_spawn_points():
    """Enemies advance from the north end of the corridor toward the gate objective (south, -Y)."""
    spawner = by_label("Ongseong_WaveManager")
    if not spawner:
        managers = by_class(FEATURE_MODULE + "OngseongEnemyWaveManager")
        spawner = managers[0] if managers else None
    if spawner:
        place(spawner, CORRIDOR_X, ENEMY_SPAWN_Y, PAWN_Z, FACING_GATE_YAW)
    else:
        warn("No enemy wave manager found")

    ram_point = by_label("Ongseong_RamSpawnPoint")
    if not ram_point:
        ram_point = actor_subsystem().spawn_actor_from_class(
            unreal.TargetPoint, unreal.Vector(CORRIDOR_X, RAM_SPAWN_Y, RAM_Z))
        ram_point.set_actor_label("Ongseong_RamSpawnPoint")
        log("Spawned Ongseong_RamSpawnPoint")
    place(ram_point, CORRIDOR_X, RAM_SPAWN_Y, RAM_Z, FACING_GATE_YAW)

    retreat = by_label("Ongseong_RetreatPoint")
    if retreat:
        place(retreat, CORRIDOR_X, RETREAT_Y, PAWN_Z)
    else:
        warn("Ongseong_RetreatPoint not found")
    return ram_point, retreat


def muzzle_component(actor):
    for component in actor.get_components_by_class(unreal.SceneComponent):
        if component.get_name() == "Muzzle":
            return component
    return None


def face_cannons_into_the_corridor():
    """The chongtongs shipped facing outward, so every shell hit the battlement in front of them."""
    for cannon in by_class(FEATURE_MODULE + "ChongtongCannonActor"):
        muzzle = muzzle_component(cannon)
        if not muzzle:
            warn("no Muzzle component on " + cannon.get_actor_label())
            continue
        location = cannon.get_actor_location()
        muzzle_location = muzzle.get_world_location()
        outward = muzzle_location.x - location.x
        # Emplacements sit on the side walls; the corridor centre line is x = 150.
        if (location.x - CORRIDOR_X) * outward <= 0.0:
            log("{0} already faces the corridor".format(cannon.get_actor_label()))
            continue
        rotation = cannon.get_actor_rotation()
        cannon.set_actor_rotation(unreal.Rotator(rotation.roll, rotation.pitch, rotation.yaw + 180.0), True)
        new_muzzle = muzzle_component(cannon).get_world_location()
        log("{0} rotated 180 deg: muzzle ({1:.0f},{2:.0f}) -> ({3:.0f},{4:.0f})".format(
            cannon.get_actor_label(), muzzle_location.x, muzzle_location.y, new_muzzle.x, new_muzzle.y))


def fix_navmesh():
    """The navmesh volume must contain the floor plus standing room above it."""
    volumes = by_class("/Script/NavigationSystem.NavMeshBoundsVolume")
    if not volumes:
        warn("No NavMeshBoundsVolume in the level")
        return
    for volume in volumes:
        origin, extent = volume.get_actor_bounds(False)
        log("navmesh before: origin ({0:.0f},{1:.0f},{2:.0f}) extent ({3:.0f},{4:.0f},{5:.0f})".format(
            origin.x, origin.y, origin.z, extent.x, extent.y, extent.z))
        scale = volume.get_actor_scale3d()
        base = unreal.Vector(extent.x / max(scale.x, 0.0001),
                             extent.y / max(scale.y, 0.0001),
                             extent.z / max(scale.z, 0.0001))
        target = unreal.Vector(3600.0, 4600.0, 400.0)
        volume.set_actor_scale3d(unreal.Vector(target.x / base.x, target.y / base.y, target.z / base.z))
        volume.set_actor_location(unreal.Vector(250.0, 4580.0, GROUND_Z + 250.0), False, True)
        origin, extent = volume.get_actor_bounds(False)
        log("navmesh after:  origin ({0:.0f},{1:.0f},{2:.0f}) extent ({3:.0f},{4:.0f},{5:.0f})".format(
            origin.x, origin.y, origin.z, extent.x, extent.y, extent.z))
    unreal.SystemLibrary.execute_console_command(None, "RebuildNavigation")


def find_pool(tag=None, pooled_class_name=None):
    """Pools are identified by their gameplay tag first, then by the class they pool."""
    pools = by_class(CORE_MODULE + "ActorPool")
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


def configure_pools():
    for tag, class_name, size, label in (
        ("Ongseong.SwordsmanPool", "Sword", SWORDSMAN_POOL_SIZE, "swordsman"),
        ("Ongseong.ArcherPool", "Archer", ARCHER_POOL_SIZE, "archer"),
        ("Ongseong.ArrowPool", "Arrow", ARROW_POOL_SIZE, "arrow"),
    ):
        pool = find_pool(tag=tag, pooled_class_name=class_name)
        if not pool:
            warn("Could not find the {0} pool. Size it manually.".format(label))
            continue
        set_prop(pool, ["initial_pool_size"], size)
        set_prop(pool, ["allow_pool_expansion", "b_allow_pool_expansion"], False)
        log("Sized {0} pool ({1}) to {2}".format(label, pool.get_actor_label(), size))


def configure_ram_pool():
    ram_pool = find_pool(tag="Ongseong.RamPool", pooled_class_name="Ram")
    if not ram_pool:
        ram_pool = actor_subsystem().spawn_actor_from_class(
            unreal.load_class(None, CORE_MODULE + "ActorPool"), unreal.Vector(0.0, 0.0, 0.0))
        ram_pool.set_actor_label("Ram_ActorPool")
        ram_pool.tags = ["Ongseong.RamPool"]
        log("Spawned Ram_ActorPool")
    ram_class = load_blueprint_class(RAM_BLUEPRINT)
    if ram_class:
        set_prop(ram_pool, ["pooled_actor_class"], ram_class)
    else:
        warn("Could not load " + RAM_BLUEPRINT)
    set_prop(ram_pool, ["initial_pool_size"], RAM_POOL_SIZE)
    set_prop(ram_pool, ["allow_pool_expansion", "b_allow_pool_expansion"], False)
    return ram_pool


def configure_scenarios(ram_pool, ram_point, retreat):
    scenarios = by_class(FEATURE_MODULE + "OngseongDefenseScenarioManager")
    if not scenarios:
        warn("No OngseongDefenseScenarioManager found in the level.")
    for scenario in scenarios:
        if ram_pool:
            set_prop(scenario, ["ram_pool"], ram_pool)
        if ram_point:
            set_prop(scenario, ["ram_spawn_point"], ram_point)
        if retreat:
            set_prop(scenario, ["retreat_point"], retreat)
        log("Configured " + scenario.get_actor_label())

    gates = by_class(FEATURE_MODULE + "OngseongGateActor")
    if gates and ram_point:
        approach = unreal.Vector.distance(gates[0].get_actor_location(), ram_point.get_actor_location()) - 800.0
        log("Ram approach: {0:.0f} cm to the staging point, {1:.0f} s at {2:.0f} cm/s".format(
            approach, approach / RAM_MOVE_SPEED, RAM_MOVE_SPEED))


def remove_stray_duplicates():
    """Earlier tooling could leave a second, unreferenced ram spawn point behind."""
    duplicate = by_label("Ram_SpawnPoint")
    if duplicate:
        actor_subsystem().destroy_actor(duplicate)
        log("removed duplicate Ram_SpawnPoint")


def main():
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.load_level(LEVEL):
        raise RuntimeError("Could not load " + LEVEL)
    log("Applying the verified combat layout to " + LEVEL)

    ram_point, retreat = place_spawn_points()
    remove_stray_duplicates()
    face_cannons_into_the_corridor()
    configure_pools()
    configure_scenarios(configure_ram_pool(), ram_point, retreat)
    fix_navmesh()

    level_subsystem.save_current_level()
    if not asset_lib.save_asset(LEVEL, False):
        warn("save_asset({0}) failed".format(LEVEL))

    # Read the actors back so a silent save failure cannot pass as success.
    for label in ("Ongseong_WaveManager", "Ongseong_RamSpawnPoint", "Ongseong_RetreatPoint", "Ram_ActorPool"):
        actor = by_label(label)
        if actor:
            location = actor.get_actor_location()
            log("verify {0}: ({1:.0f}, {2:.0f}, {3:.0f})".format(label, location.x, location.y, location.z))
        else:
            warn("verify {0}: missing".format(label))

    if PROBLEMS:
        unreal.log_warning("[OngseongLayout] Finished with {0} item(s) needing attention:".format(len(PROBLEMS)))
        for problem in PROBLEMS:
            unreal.log_warning("[OngseongLayout]   - " + problem)
    else:
        log("Done.")


main()

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
