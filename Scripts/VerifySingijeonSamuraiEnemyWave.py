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
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"SAMURAI_VERIFY PASS: retarget authoring asset exists: {asset_path}")
    else:
        unreal.log_warning(
            f"SAMURAI_VERIFY: optional retarget authoring asset is not registered: {asset_path}")

mesh_editor = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
require(mesh_editor.get_lod_count(mesh) == 3, "optimized Samurai mesh has three LODs")
for lod_index in range(mesh_editor.get_lod_count(mesh)):
    settings = mesh_editor.get_lod_build_settings(mesh, lod_index)
    require(settings.get_editor_property("optimize_for_instancing"),
            f"Samurai mesh LOD{lod_index} is optimized for instancing")
require(run.get_editor_property("skeleton") == mesh.get_editor_property("skeleton"),
        "Rifle Jog uses the Samurai skeleton")
require(run.get_play_length() > 0.1, "Rifle Jog has a non-zero play length")
animation_model = run.get_editor_property("data_model_interface")
require(animation_model is not None, "Rifle Jog has an animation data model")
require(animation_model.get_num_bone_tracks() > 1,
        "Rifle Jog contains animated bone tracks")
require(provider.get_editor_property("skinned_asset") == mesh,
        "GPU provider uses the optimized Samurai mesh")

for material_slot in list(mesh.get_editor_property("materials")):
    material_interface = material_slot.get_editor_property("material_interface")
    require(material_interface is not None, "Samurai material slot is assigned")
    overrides = material_interface.get_editor_property("base_property_overrides")
    instanced_skinned_mesh_usage = 1 << 26
    require((int(overrides.get_editor_property("override_usage_flags")) &
             instanced_skinned_mesh_usage) != 0,
            f"material {material_interface.get_path_name()} overrides instanced skinned usage")
    require((int(overrides.get_editor_property("usage_flags")) &
             instanced_skinned_mesh_usage) != 0,
            f"material {material_interface.get_path_name()} enables instanced skinned usage")

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
require(not wave.get_editor_property("use_gpu_instanced_crowd"),
        "Wave uses the reliable pose-sharing skeletal renderer by default")
require(wave.get_editor_property("shared_pose_leader_count") == 8,
        "Wave limits animation evaluation to eight shared pose leaders")
require(wave.get_editor_property("enemy_count") == 45 and
        wave.get_editor_property("max_interactive_enemies") == 3,
        "Wave retains 45 enemies with only three full Actors")
require(math.isclose(wave.get_editor_property("min_run_animation_rate"), 0.86, abs_tol=0.001) and
        math.isclose(wave.get_editor_property("max_run_animation_rate"), 1.14, abs_tol=0.001),
        "foreground play-rate range is configured")

unreal.log("SAMURAI_VERIFY SUCCESS")
