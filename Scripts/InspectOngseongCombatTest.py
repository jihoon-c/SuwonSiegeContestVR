"""Prints the distances that decide ram approach time and chongtong engagement in the test level."""

import unreal

LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"
FEATURE = "/Script/GF_OngseongCrossbow."


def log(msg):
    unreal.log("[Inspect] " + msg)


def actors(class_path):
    cls = unreal.load_class(None, class_path)
    if not cls:
        return []
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    return unreal.EditorFilterLibrary.by_class(subsystem.get_all_level_actors(), cls)


unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)

gates = actors(FEATURE + "OngseongGateActor")
waves = actors(FEATURE + "OngseongEnemyWaveManager")
scenarios = actors(FEATURE + "OngseongDefenseScenarioManager")
cannons = actors(FEATURE + "ChongtongCannonActor")

gate = gates[0] if gates else None
if not gate:
    log("NO GATE FOUND")
else:
    gate_loc = gate.get_actor_location()
    log("gate {0} at {1}".format(gate.get_actor_label(), gate_loc))

    for wave in waves:
        loc = wave.get_actor_location()
        log("enemy spawn {0} at {1}, distance to gate {2:.0f} cm".format(
            wave.get_actor_label(), loc, unreal.Vector.distance(loc, gate_loc)))

    for scenario in scenarios:
        ram_point = scenario.get_editor_property("ram_spawn_point")
        if ram_point:
            loc = ram_point.get_actor_location()
            log("ram spawn {0} at {1}, distance to gate {2:.0f} cm".format(
                ram_point.get_actor_label(), loc, unreal.Vector.distance(loc, gate_loc)))
        else:
            log("ram spawn point is NOT set; the ram starts at the enemy spawn")
        retreat = scenario.get_editor_property("retreat_point")
        if retreat:
            rloc = retreat.get_actor_location()
            log("retreat point {0} at {1}, distance to gate {2:.0f} cm".format(
                retreat.get_actor_label(), rloc, unreal.Vector.distance(rloc, gate_loc)))
        else:
            log("retreat point: not set")

    for cannon in cannons:
        loc = cannon.get_actor_location()
        log("chongtong {0} at {1}, distance to gate {2:.0f} cm, fire range {3}".format(
            cannon.get_actor_label(), loc, unreal.Vector.distance(loc, gate_loc),
            cannon.get_editor_property("fire_range")))
        for wave in waves:
            log("    distance to {0}: {1:.0f} cm".format(
                wave.get_actor_label(), unreal.Vector.distance(loc, wave.get_actor_location())))

for pool in actors("/Script/SuwonSiegeContestVR.ActorPool"):
    log("pool {0} at {1}, class {2}, size {3}".format(
        pool.get_actor_label(), pool.get_actor_location(),
        pool.get_editor_property("pooled_actor_class"), pool.get_editor_property("initial_pool_size")))

for start in actors("/Script/Engine.PlayerStart"):
    log("player start {0} at {1}".format(start.get_actor_label(), start.get_actor_location()))

navs = actors("/Script/NavigationSystem.NavMeshBoundsVolume")
for nav in navs:
    origin, extent = nav.get_actor_bounds(False)
    log("navmesh volume at {0} extent {1}".format(origin, extent))
log("NavMeshBoundsVolume count: {0}".format(len(navs)))

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
