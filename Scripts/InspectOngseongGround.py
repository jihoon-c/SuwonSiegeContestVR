"""Maps the collision layers under the ongseong so spawn heights can be placed on real ground."""

import unreal

LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"
FEATURE = "/Script/GF_OngseongCrossbow."


def log(msg):
    unreal.log("[Ground] " + msg)


def actors(class_path):
    cls = unreal.load_class(None, class_path)
    if not cls:
        return []
    sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    return unreal.EditorFilterLibrary.by_class(sub.get_all_level_actors(), cls)


unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


def column(x, y):
    hits = unreal.SystemLibrary.line_trace_multi(
        world, unreal.Vector(x, y, 6000.0), unreal.Vector(x, y, -3000.0),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [], unreal.DrawDebugTrace.NONE, True)
    parts = []
    if not hits:
        return parts
    for hit in hits:
        t = hit.to_tuple()
        point = t[4]
        actor = t[9]
        parts.append("{0}@{1:.0f}".format(actor.get_actor_label() if actor else "?", point.z))
    return parts


log("=== column scan (surfaces from the sky down) ===")
for y in (-4000, -3000, -2000, -1000, -400, 0, 400, 1000, 2000, 3000, 4000, 5000):
    for x in (-2000, -1000, 0, 1000, 2000):
        parts = column(float(x), float(y))
        log("({0:>5},{1:>5}) : {2}".format(x, y, ", ".join(parts) if parts else "NO COLLISION"))

log("=== key actors ===")
for path, label in ((FEATURE + "OngseongGateActor", "gate"),
                    (FEATURE + "ChongtongCannonActor", "chongtong"),
                    (FEATURE + "OngseongEnemyWaveManager", "spawner")):
    for a in actors(path):
        loc = a.get_actor_location()
        origin, extent = a.get_actor_bounds(False)
        log("{0} {1}: loc ({2:.0f},{3:.0f},{4:.0f}) bounds origin z {5:.0f} extent z {6:.0f}".format(
            label, a.get_actor_label(), loc.x, loc.y, loc.z, origin.z, extent.z))

log("=== navmesh volumes ===")
for nav in actors("/Script/NavigationSystem.NavMeshBoundsVolume"):
    origin, extent = nav.get_actor_bounds(False)
    log("{0}: origin ({1:.0f},{2:.0f},{3:.0f}) extent ({4:.0f},{5:.0f},{6:.0f})".format(
        nav.get_actor_label(), origin.x, origin.y, origin.z, extent.x, extent.y, extent.z))

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
