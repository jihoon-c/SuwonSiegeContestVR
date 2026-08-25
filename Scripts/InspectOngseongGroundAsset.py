"""Checks whether the ongseong floor mesh actually has collision geometry."""

import unreal

LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"


def log(msg):
    unreal.log("[GroundAsset] " + msg)


unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

for actor in sub.get_all_level_actors():
    if actor.get_actor_label() not in ("ground", "background"):
        continue
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = comp.static_mesh
        log("actor {0} component {1}".format(actor.get_actor_label(), comp.get_name()))
        log("    collision enabled : {0}".format(comp.get_collision_enabled()))
        log("    collision profile : {0}".format(comp.get_collision_profile_name()))
        log("    blocks pawn       : {0}".format(comp.get_collision_response_to_channel(unreal.CollisionChannel.ECC_PAWN)))
        log("    blocks visibility : {0}".format(comp.get_collision_response_to_channel(unreal.CollisionChannel.ECC_VISIBILITY)))
        if not mesh:
            log("    mesh              : NONE")
            continue
        log("    mesh              : {0}".format(mesh.get_path_name()))
        body = mesh.get_editor_property("body_setup")
        if body:
            log("    collision complexity: {0}".format(body.get_editor_property("collision_trace_flag")))
            agg = body.get_editor_property("agg_geom")
            log("    simple primitives  : boxes {0}, spheres {1}, convex {2}".format(
                len(agg.get_editor_property("box_elems")),
                len(agg.get_editor_property("sphere_elems")),
                len(agg.get_editor_property("convex_elems"))))
        else:
            log("    body setup         : NONE (no collision at all)")

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
