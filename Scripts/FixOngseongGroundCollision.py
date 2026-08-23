"""Gives the ongseong floor real collision.

SM_Ground ships with two box collision primitives of zero height, so nothing ever lands on it:
pooled enemies, placed enemies and ally soldiers all fall out of the world. Switching the mesh to
complex-as-simple collision makes the floor geometry itself collide, which is what a flat static
floor needs.
"""

import unreal

GROUND_MESH = "/GF_OngseongCrossbow/Asset/Prop/Ground/SM_Ground"
LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest"


def log(msg):
    unreal.log("[GroundFix] " + msg)


mesh = unreal.EditorAssetLibrary.load_asset(GROUND_MESH)
if not mesh:
    raise RuntimeError("missing " + GROUND_MESH)

body = mesh.get_editor_property("body_setup")
log("collision_trace_flag before: {0}".format(body.get_editor_property("collision_trace_flag")))
body.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
log("collision_trace_flag after:  {0}".format(body.get_editor_property("collision_trace_flag")))
unreal.EditorAssetLibrary.save_asset(GROUND_MESH, False)
log("saved " + GROUND_MESH)

# Navigation has to be rebuilt now that a floor exists.
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()

log("=== floor probes ===")
for x, y in ((150.0, 8200.0), (150.0, 4000.0), (150.0, 1000.0), (1700.0, 3550.0)):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(x, y, 1500.0), unreal.Vector(x, y, -1000.0),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [], unreal.DrawDebugTrace.NONE, True)
    if hit:
        t = hit.to_tuple()
        log("({0:>6.0f},{1:>6.0f}) hits {2} at z {3:.0f}".format(x, y, t[9].get_actor_label() if t[9] else "?", t[4].z))
    else:
        log("({0:>6.0f},{1:>6.0f}) STILL NO FLOOR".format(x, y))

unreal.SystemLibrary.execute_console_command(None, "RebuildNavigation")
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
log("rebuilt navigation and saved " + LEVEL)

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
