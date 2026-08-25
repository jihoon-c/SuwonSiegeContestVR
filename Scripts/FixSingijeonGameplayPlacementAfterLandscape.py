import math
import unreal


LEVEL_PATH = "/GF_Singijeon/Maps/LV_Singijeon"

# These are the authored Singijeon play-space anchors before the Namhansanseong
# terrain import.  The Z coordinate is projected onto the imported Landscape.
ANCHORS = {
    "BP_SingijeonHwacha_Playable": (unreal.Vector(395.0, 58.0, 0.0), 5.0, unreal.Rotator(0.0, 180.0, 0.0)),
    "BP_SingijeonTorch_Playable": (unreal.Vector(-50.0, -112.0, 0.0), 110.0, unreal.Rotator()),
    "BP_SingijeonArrow_Playable": (unreal.Vector(71.0, 235.0, 0.0), 110.0, unreal.Rotator()),
    "BP_SingijeonFirePit_Playable": (unreal.Vector(120.0, -180.0, 0.0), 0.0, unreal.Rotator()),
    "Singijeon_EnemyWave": (unreal.Vector(1110.0, 205.0, 0.0), 0.0, unreal.Rotator()),
}


world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError("LV_Singijeon could not be loaded")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = {actor.get_actor_label(): actor for actor in actor_subsystem.get_all_level_actors()}


def landscape_height(point):
    hit = unreal.SystemLibrary.line_trace_single(
        world,
        point + unreal.Vector(0.0, 0.0, 50000.0),
        point - unreal.Vector(0.0, 0.0, 50000.0),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,
        False,
        [],
        unreal.DrawDebugTrace.NONE,
        True,
    ).to_dict()
    if not hit["blocking_hit"] or not isinstance(hit["hit_actor"], unreal.LandscapeProxy):
        raise RuntimeError(f"Landscape ground was not found for {point}: {hit}")
    return hit["impact_point"].z


for label, (anchor, z_offset, rotation) in ANCHORS.items():
    actor = actors.get(label)
    if not actor:
        raise RuntimeError(f"Required Singijeon actor is missing: {label}")
    ground_z = landscape_height(anchor)
    target = unreal.Vector(anchor.x, anchor.y, ground_z + z_offset)
    actor.set_actor_location(target, False, False)
    actor.set_actor_rotation(rotation, False)
    unreal.log(f"SINGIJEON_PLACEMENT {label}={target}")

wave = actors["Singijeon_EnemyWave"]
hwacha = actors["BP_SingijeonHwacha_Playable"]
wave.set_editor_property("target_actor", hwacha)
wave.set_editor_property(
    "proxy_skeletal_mesh",
    unreal.load_asset("/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Meshes/SKM_Manny_Simple"),
)
# The prior rifle-jog animation targets the Samurai skeleton.  Leave the
# default Manny mesh in its authored pose until a Manny-compatible run asset
# is explicitly assigned in the Enemy Wave details panel.
wave.set_editor_property("foreground_run_animation", None)
wave.set_editor_property("proxy_animation_provider", None)

if not unreal.EditorLevelLibrary.save_current_level():
    raise RuntimeError("LV_Singijeon could not be saved after placement recovery")
unreal.log("SINGIJEON_GAMEPLAY_PLACEMENT_FIX SUCCESS")
