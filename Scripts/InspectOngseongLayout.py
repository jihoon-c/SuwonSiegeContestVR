"""Prints the placed combatants and structure bounds that define the intended battle layout."""

import unreal

LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"


def log(msg):
    unreal.log("[Layout] " + msg)


unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = sub.get_all_level_actors()

log("=== placed combatants and gameplay points ===")
keys = ("BP_Ally", "BP_Enemy", "BP_Ongseong", "Crossbow", "Chongtong", "WaveManager",
        "RetreatPoint", "RamSpawn", "Ram_SpawnPoint", "GateObjective", "PlayerStart",
        "Idle", "SKM_tripo")
for actor in all_actors:
    label = actor.get_actor_label()
    if any(k.lower() in label.lower() for k in keys):
        loc = actor.get_actor_location()
        log("{0:<40} ({1:>7.0f},{2:>7.0f},{3:>6.0f})  {4}".format(
            label, loc.x, loc.y, loc.z, actor.get_class().get_name()))


def group_bounds(prefix):
    xs, ys, zs = [], [], []
    count = 0
    for actor in all_actors:
        if actor.get_actor_label().startswith(prefix):
            loc = actor.get_actor_location()
            xs.append(loc.x)
            ys.append(loc.y)
            zs.append(loc.z)
            count += 1
    if not count:
        return
    log("{0}: {1} actors, x [{2:.0f} .. {3:.0f}], y [{4:.0f} .. {5:.0f}], z [{6:.0f} .. {7:.0f}]".format(
        prefix, count, min(xs), max(xs), min(ys), max(ys), min(zs), max(zs)))


log("=== structure groups ===")
for prefix in ("Battlement_GateDefense", "StoneWall_Approach", "JihwaGate"):
    group_bounds(prefix)

log("=== ground ===")
for actor in all_actors:
    if actor.get_actor_label() in ("ground", "background"):
        origin, extent = actor.get_actor_bounds(False)
        log("{0}: x [{1:.0f} .. {2:.0f}], y [{3:.0f} .. {4:.0f}], z {5:.0f}".format(
            actor.get_actor_label(), origin.x - extent.x, origin.x + extent.x,
            origin.y - extent.y, origin.y + extent.y, origin.z))

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
