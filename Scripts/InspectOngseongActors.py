"""Dumps the level's collision-bearing actors so the missing walkable ground can be identified."""

import unreal

LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"


def log(msg):
    unreal.log("[Actors] " + msg)


unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = sub.get_all_level_actors()
log("total actors: {0}".format(len(all_actors)))

for actor in all_actors:
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not comps:
        continue
    origin, extent = actor.get_actor_bounds(False)
    if extent.x < 500.0 and extent.y < 500.0:
        continue  # skip small props
    details = []
    for comp in comps[:3]:
        mesh = comp.static_mesh
        collision = comp.get_collision_enabled()
        details.append("{0}/{1}".format(mesh.get_name() if mesh else "none", collision))
    log("{0:<34} class {1:<26} origin ({2:.0f},{3:.0f},{4:.0f}) extent ({5:.0f},{6:.0f},{7:.0f}) :: {8}".format(
        actor.get_actor_label(), actor.get_class().get_name(),
        origin.x, origin.y, origin.z, extent.x, extent.y, extent.z,
        ", ".join(details)))

log("=== landscapes ===")
for actor in all_actors:
    if "Landscape" in actor.get_class().get_name():
        origin, extent = actor.get_actor_bounds(False)
        log("{0} ({1}) origin ({2:.0f},{3:.0f},{4:.0f}) extent ({5:.0f},{6:.0f},{7:.0f})".format(
            actor.get_actor_label(), actor.get_class().get_name(),
            origin.x, origin.y, origin.z, extent.x, extent.y, extent.z))

log("=== labels of every actor ===")
labels = sorted(a.get_actor_label() for a in all_actors)
log(", ".join(labels))

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
