"""Traces line of sight from each chongtong to candidate enemy positions in the test level.

Mirrors UCombatTargetingComponent::HasLineOfSightTo: ECC_Visibility, complex trace,
from the cannon actor origin to the candidate bounds centre.
"""

import unreal

LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"
FEATURE = "/Script/GF_OngseongCrossbow."
ENEMY_EYE_Z = 90.0


def log(msg):
    unreal.log("[LOS] " + msg)


def actors(class_path):
    cls = unreal.load_class(None, class_path)
    if not cls:
        return []
    sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    return unreal.EditorFilterLibrary.by_class(sub.get_all_level_actors(), cls)


unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


def trace(start, end, ignore):
    hit = unreal.SystemLibrary.line_trace_single(
        world, start, end, unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, ignore,
        unreal.DrawDebugTrace.NONE, True)
    return hit


def ground_z(x, y):
    hit = trace(unreal.Vector(x, y, 5000.0), unreal.Vector(x, y, -5000.0), [])
    if hit:
        return hit.to_tuple()[4].z  # impact point
    return None


gates = actors(FEATURE + "OngseongGateActor")
gate = gates[0] if gates else None
gate_loc = gate.get_actor_location() if gate else unreal.Vector(150.0, 100.0, 0.0)
log("gate at ({0:.0f}, {1:.0f}, {2:.0f})".format(gate_loc.x, gate_loc.y, gate_loc.z))

# Candidate enemy positions: current spawn, then a corridor outside the gate (-Y) and inside (+Y).
candidates = [("current spawn", 0.0, 0.0)]
for y in (-500, -1000, -1500, -2000, -2500, -3000, -4000):
    candidates.append(("outside y={0}".format(y), 150.0, float(y)))
for y in (400, 800, 1200, 1600, 2000):
    candidates.append(("inside y={0}".format(y), 150.0, float(y)))

for x, y in ((-1200.0, -1000.0), (1200.0, -1000.0), (-1200.0, 600.0), (1200.0, 600.0)):
    candidates.append(("flank ({0:.0f},{1:.0f})".format(x, y), x, y))

for name, x, y in candidates:
    gz = ground_z(x, y)
    log("candidate {0:<22} ground z = {1}".format(name, "none" if gz is None else "{0:.0f}".format(gz)))

for cannon in actors(FEATURE + "ChongtongCannonActor"):
    loc = cannon.get_actor_location()
    log("--- {0} at ({1:.0f}, {2:.0f}, {3:.0f}) ---".format(cannon.get_actor_label(), loc.x, loc.y, loc.z))
    for name, x, y in candidates:
        gz = ground_z(x, y)
        target_z = (gz if gz is not None else 0.0) + ENEMY_EYE_Z
        target = unreal.Vector(x, y, target_z)
        hit = trace(loc, target, [cannon])
        if not hit:
            log("    {0:<22} VISIBLE  (dist {1:.0f})".format(name, unreal.Vector.distance(loc, target)))
        else:
            t = hit.to_tuple()
            blocker = t[9]  # hit actor
            comp = t[10]  # hit component
            point = t[4]
            log("    {0:<22} blocked by {1} / {2} at ({3:.0f}, {4:.0f}, {5:.0f})".format(
                name,
                blocker.get_actor_label() if blocker else "?",
                comp.get_name() if comp else "?",
                point.x, point.y, point.z))

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
