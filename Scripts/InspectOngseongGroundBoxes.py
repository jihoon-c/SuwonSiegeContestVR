"""Prints the ground mesh's collision boxes in world space and probes the floor."""

import unreal

LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"


def log(msg):
    unreal.log("[Boxes] " + msg)


unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()

for actor in sub.get_all_level_actors():
    if actor.get_actor_label() != "ground":
        continue
    loc = actor.get_actor_location()
    scale = actor.get_actor_scale3d()
    log("ground actor at ({0:.0f},{1:.0f},{2:.0f}) scale ({3:.2f},{4:.2f},{5:.2f})".format(
        loc.x, loc.y, loc.z, scale.x, scale.y, scale.z))
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = comp.static_mesh
        body = mesh.get_editor_property("body_setup") if mesh else None
        if not body:
            continue
        agg = body.get_editor_property("agg_geom")
        for box in agg.get_editor_property("box_elems"):
            centre = box.get_editor_property("center")
            log("  local box centre ({0:.0f},{1:.0f},{2:.0f}) size ({3:.0f} x {4:.0f} x {5:.0f})".format(
                centre.x, centre.y, centre.z,
                box.get_editor_property("x"), box.get_editor_property("y"), box.get_editor_property("z")))
            log("    world centre approx ({0:.0f},{1:.0f},{2:.0f}) world size ({3:.0f} x {4:.0f} x {5:.0f})".format(
                loc.x + centre.x * scale.x, loc.y + centre.y * scale.y, loc.z + centre.z * scale.z,
                box.get_editor_property("x") * scale.x,
                box.get_editor_property("y") * scale.y,
                box.get_editor_property("z") * scale.z))

log("=== downward probes from z=1500 ===")
for x, y in ((150.0, 8200.0), (150.0, 4000.0), (150.0, 1000.0), (150.0, 200.0), (1700.0, 3550.0), (3240.0, 3510.0)):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(x, y, 1500.0), unreal.Vector(x, y, -1000.0),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [], unreal.DrawDebugTrace.NONE, True)
    if hit:
        t = hit.to_tuple()
        log("({0:>6.0f},{1:>6.0f}) hits {2} at z {3:.0f}".format(x, y, t[9].get_actor_label() if t[9] else "?", t[4].z))
    else:
        log("({0:>6.0f},{1:>6.0f}) NO FLOOR".format(x, y))

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
