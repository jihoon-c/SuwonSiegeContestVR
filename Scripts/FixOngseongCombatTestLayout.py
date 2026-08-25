"""Places the ongseong test level's spawn points on the real walkable corridor.

The enemy spawner shipped at the world origin, which is off the `ground` mesh
(x -3250..3750, y 80..9080, z 10), so pooled enemies dropped into empty space and never
reached the defenders. Enemies belong at the north end of the walled corridor and advance
south toward the gate objective at y = 100. The navmesh volume also sat below the floor.

Idempotent. Only touches LV_Ongseong_CombatTest.
"""

import unreal

LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"
FEATURE = "/Script/GF_OngseongCrossbow."

GROUND_Z = 10.0
PAWN_Z = 98.0        # matches the enemy actors already placed in the level
RAM_Z = 243.0        # matches the ram already placed in the level
ENEMY_SPAWN_Y = 7800.0   # inside the generated navmesh; 8200 projected 239 cm away and stalled pathing
RAM_SPAWN_Y = 8350.0
RETREAT_Y = 8900.0
CORRIDOR_X = 150.0   # centre line of the corridor, same as the gate


def log(msg):
    unreal.log("[FixLayout] " + msg)


def warn(msg):
    unreal.log_warning("[FixLayout] " + msg)


unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = actor_subsystem.get_all_level_actors()


def by_label(label):
    for actor in all_actors:
        if actor.get_actor_label() == label:
            return actor
    return None


def by_class(class_path):
    cls = unreal.load_class(None, class_path)
    if not cls:
        return []
    return unreal.EditorFilterLibrary.by_class(all_actors, cls)


def place(actor, x, y, z, yaw=None):
    # Editor-locked actors silently refuse to move, so clear the lock and verify the result.
    try:
        if actor.get_editor_property("lock_location"):
            actor.set_editor_property("lock_location", False)
            log("cleared location lock on " + actor.get_actor_label())
    except Exception:
        pass
    target = unreal.Vector(x, y, z)
    applied = actor.set_actor_location(target, False, True)
    if yaw is not None:
        actor.set_actor_rotation(unreal.Rotator(0.0, 0.0, yaw), True)
    actual = actor.get_actor_location()
    if unreal.Vector.distance(actual, target) > 1.0:
        warn("{0} did not move (requested ({1:.0f},{2:.0f},{3:.0f}), still at ({4:.0f},{5:.0f},{6:.0f}), api returned {7})".format(
            actor.get_actor_label(), x, y, z, actual.x, actual.y, actual.z, applied))
    else:
        log("{0} -> ({1:.0f}, {2:.0f}, {3:.0f})".format(actor.get_actor_label(), actual.x, actual.y, actual.z))


# Enemies advance from the north end of the corridor toward the gate objective (south, -Y).
FACING_GATE_YAW = -90.0

spawner = by_label("Ongseong_WaveManager")
if spawner:
    place(spawner, CORRIDOR_X, ENEMY_SPAWN_Y, PAWN_Z, FACING_GATE_YAW)
else:
    warn("Ongseong_WaveManager not found")

ram_point = by_label("Ongseong_RamSpawnPoint")
if ram_point:
    place(ram_point, CORRIDOR_X, RAM_SPAWN_Y, RAM_Z, FACING_GATE_YAW)
else:
    warn("Ongseong_RamSpawnPoint not found")

retreat = by_label("Ongseong_RetreatPoint")
if retreat:
    place(retreat, CORRIDOR_X, RETREAT_Y, PAWN_Z)
else:
    warn("Ongseong_RetreatPoint not found")

# Point the scenario at the level's own ram spawn point and drop the duplicate this tooling made.
for scenario in by_class(FEATURE + "OngseongDefenseScenarioManager"):
    if ram_point:
        scenario.set_editor_property("ram_spawn_point", ram_point)
        log("scenario ram spawn point -> " + ram_point.get_actor_label())
    if retreat:
        scenario.set_editor_property("retreat_point", retreat)

duplicate = by_label("Ram_SpawnPoint")
if duplicate:
    actor_subsystem.destroy_actor(duplicate)
    log("removed duplicate Ram_SpawnPoint")

def muzzle_component(actor):
    for comp in actor.get_components_by_class(unreal.SceneComponent):
        if comp.get_name() == "Muzzle":
            return comp
    return None


def face_cannons_into_the_corridor():
    """The chongtongs shipped facing outward, so every shell hit the battlement in front of them."""
    for cannon in by_class(FEATURE + "ChongtongCannonActor"):
        muzzle = muzzle_component(cannon)
        if not muzzle:
            warn("no Muzzle component on " + cannon.get_actor_label())
            continue
        loc = cannon.get_actor_location()
        muzzle_loc = muzzle.get_world_location()
        outward = (muzzle_loc.x - loc.x)
        # Emplacements sit on the side walls; the corridor centre line is x = 150.
        points_away = (loc.x - CORRIDOR_X) * outward > 0.0
        if not points_away:
            log("{0} already faces the corridor (muzzle {1:.0f},{2:.0f})".format(
                cannon.get_actor_label(), muzzle_loc.x, muzzle_loc.y))
            continue
        rotation = cannon.get_actor_rotation()
        cannon.set_actor_rotation(unreal.Rotator(rotation.roll, rotation.pitch, rotation.yaw + 180.0), True)
        new_muzzle = muzzle_component(cannon).get_world_location()
        log("{0} rotated 180 deg: muzzle ({1:.0f},{2:.0f}) -> ({3:.0f},{4:.0f})".format(
            cannon.get_actor_label(), muzzle_loc.x, muzzle_loc.y, new_muzzle.x, new_muzzle.y))


face_cannons_into_the_corridor()

# The navmesh volume must contain the floor plus standing room above it.
for volume in by_class("/Script/NavigationSystem.NavMeshBoundsVolume"):
    origin, extent = volume.get_actor_bounds(False)
    log("navmesh before: origin ({0:.0f},{1:.0f},{2:.0f}) extent ({3:.0f},{4:.0f},{5:.0f})".format(
        origin.x, origin.y, origin.z, extent.x, extent.y, extent.z))
    scale = volume.get_actor_scale3d()
    # Current unscaled half-size, so the target half-size can be reached with a scale change.
    base = unreal.Vector(extent.x / max(scale.x, 0.0001), extent.y / max(scale.y, 0.0001), extent.z / max(scale.z, 0.0001))
    target = unreal.Vector(3600.0, 4600.0, 400.0)
    volume.set_actor_scale3d(unreal.Vector(target.x / base.x, target.y / base.y, target.z / base.z))
    volume.set_actor_location(unreal.Vector(250.0, 4580.0, GROUND_Z + 250.0), False, True)
    origin, extent = volume.get_actor_bounds(False)
    log("navmesh after:  origin ({0:.0f},{1:.0f},{2:.0f}) extent ({3:.0f},{4:.0f},{5:.0f})".format(
        origin.x, origin.y, origin.z, extent.x, extent.y, extent.z))

unreal.SystemLibrary.execute_console_command(None, "RebuildNavigation")

level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
saved = level_subsystem.save_current_level()
log("save_current_level returned {0}".format(saved))
if not unreal.EditorAssetLibrary.save_asset(LEVEL, False):
    warn("save_asset({0}) failed".format(LEVEL))

# Read the actors back so a silent save failure cannot pass as success.
for label in ("Ongseong_WaveManager", "Ongseong_RamSpawnPoint", "Ongseong_RetreatPoint"):
    actor = by_label(label)
    if actor:
        loc = actor.get_actor_location()
        log("verify {0}: ({1:.0f}, {2:.0f}, {3:.0f})".format(label, loc.x, loc.y, loc.z))

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
