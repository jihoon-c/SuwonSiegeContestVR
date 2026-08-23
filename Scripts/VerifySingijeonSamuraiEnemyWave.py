import math
import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"
MESH_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/SKM_Low_Poly_Samurai_VR"
RUN_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/MF_Rifle_Jog_Fwd_Samurai"
PROVIDER_PATH = "/GF_Singijeon/Gameplay/Enemy/Samurai/DA_SingijeonSamuraiRifleRun_GPU"
IK_ASSETS = (
    "/GF_Singijeon/Gameplay/Enemy/Samurai/IK_Manny_Rifle_Source",
    "/GF_Singijeon/Gameplay/Enemy/Samurai/IK_LowPolySamurai_Target",
    "/GF_Singijeon/Gameplay/Enemy/Samurai/RTG_MannyRifle_To_LowPolySamurai",
)


def require(condition, message):
    if not condition:
        raise RuntimeError(message)
    unreal.log(f"SAMURAI_VERIFY PASS: {message}")


mesh = unreal.load_asset(MESH_PATH)
run = unreal.load_asset(RUN_PATH)
provider = unreal.load_asset(PROVIDER_PATH)
require(mesh is not None, "optimized Samurai mesh exists")
require(run is not None, "retargeted Rifle Jog exists")
require(provider is not None, "GPU run provider exists")

for asset_path in IK_ASSETS:
    require(unreal.EditorAssetLibrary.does_asset_exist(asset_path), f"retarget asset exists: {asset_path}")

mesh_editor = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
require(mesh_editor.get_lod_count(mesh) == 3, "optimized Samurai mesh has three LODs")
require(run.get_editor_property("skeleton") == mesh.get_editor_property("skeleton"),
        "Rifle Jog uses the Samurai skeleton")
require(provider.get_editor_property("skinned_asset") == mesh,
        "GPU provider uses the optimized Samurai mesh")

sequences = list(provider.get_editor_property("sequences"))
require(len(sequences) == 12, "GPU provider has twelve animation variants")
rates = [float(item.get_editor_property("play_rate")) for item in sequences]
positions = [float(item.get_editor_property("position")) for item in sequences]
require(all(item.get_editor_property("sequence") == run for item in sequences),
        "all GPU variants use the retargeted Rifle Jog")
require(math.isclose(min(rates), 0.86, abs_tol=0.001) and
        math.isclose(max(rates), 1.14, abs_tol=0.001),
        "GPU play rates span 0.86 to 1.14")
require(len({round(value, 3) for value in rates}) == 12 and
        len({round(value, 3) for value in positions}) == 12,
        "GPU variants have unique speed and start phase")

world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
require(world is not None, "LV_Singijeon loads")
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
waves = [actor for actor in actor_subsystem.get_all_level_actors()
         if isinstance(actor, unreal.SingijeonEnemyWaveActor)]
require(len(waves) == 1, "LV_Singijeon has exactly one enemy Wave")
wave = waves[0]
require(wave.get_editor_property("proxy_skeletal_mesh") == mesh,
        "Wave uses the optimized Samurai mesh")
require(wave.get_editor_property("proxy_animation_provider") == provider,
        "Wave uses the Samurai GPU provider")
require(wave.get_editor_property("foreground_run_animation") == run,
        "foreground enemies use the retargeted Rifle Jog")
require(wave.get_editor_property("proxy_min_lod") == 1,
        "Wave enforces LOD1 or lower detail")
require(wave.get_editor_property("enemy_count") == 45 and
        wave.get_editor_property("max_interactive_enemies") == 3,
        "Wave retains 45 enemies with only three full Actors")
require(math.isclose(wave.get_editor_property("min_run_animation_rate"), 0.86, abs_tol=0.001) and
        math.isclose(wave.get_editor_property("max_run_animation_rate"), 1.14, abs_tol=0.001),
        "foreground play-rate range is configured")

unreal.log("SAMURAI_VERIFY SUCCESS")
