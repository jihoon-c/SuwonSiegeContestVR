import unreal


HWACHA_BP_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonHwacha"
LEVEL_PATHS = ("/Game/Maps/LV_Singijeon", "/GF_Singijeon/Maps/LV_Singijeon")
FIRE_SYSTEM_PATH = "/Game/NiagaraExamples/FX_Misc/NS_Fire"
HWACHA_MESH_PATH = "/GF_Singijeon/Asset/Hwacha/hwacha/StaticMeshes/hwacha"
TORCH_MESH_PATH = "/GF_Singijeon/Asset/Torch/Burning_Wood_Torch"
FUSE_GUIDE_MESH_PATH = "/Engine/BasicShapes/Sphere"
FUSE_GUIDE_MATERIAL_PATH = "/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/MI_GlowNT"


def configure_hwacha(actor):
    body = actor.get_editor_property("body_mesh")
    body.set_mobility(unreal.ComponentMobility.MOVABLE)

    fuse = actor.get_editor_property("fuse")
    fuse.set_sphere_radius(24.0, False)
    fuse.set_editor_property("generate_overlap_events", True)
    fuse.set_editor_property("relative_location", unreal.Vector(-35.0, 0.0, 60.0))

    carry = actor.get_editor_property("two_hand_carry")
    carry.set_editor_property("allow_single_hand_carry", True)
    carry.set_editor_property("follow_hand_without_lag", True)
    carry.set_editor_property("sweep_movement", False)

    effect = actor.get_editor_property("fuse_ignition_effect")
    effect.set_editor_property("asset", fire_system)
    effect.set_editor_property("auto_activate", False)
    effect.set_editor_property("absolute_location", False)
    effect.set_editor_property("relative_location", unreal.Vector())
    effect.set_editor_property("relative_scale3d", unreal.Vector(0.16, 0.16, 0.16))

    guide = actor.get_editor_property("fuse_guide")
    guide.set_static_mesh(fuse_guide_mesh)
    guide.set_material(0, fuse_guide_material)
    guide.set_editor_property("relative_location", unreal.Vector())
    guide.set_editor_property("relative_scale3d", unreal.Vector(0.12, 0.12, 0.12))
    guide.set_editor_property("visible", False)
    guide.set_editor_property("hidden_in_game", True)

    for property_name in ("left_handle_highlight", "right_handle_highlight"):
        handle = actor.get_editor_property(property_name)
        tags = list(handle.get_editor_property("component_tags"))
        if not any(str(tag) == "VRGrab" for tag in tags):
            tags.append(unreal.Name("VRGrab"))
        handle.set_editor_property("component_tags", tags)


hwacha_bp = unreal.load_asset(HWACHA_BP_PATH)
fire_system = unreal.load_asset(FIRE_SYSTEM_PATH)
hwacha_mesh = unreal.load_asset(HWACHA_MESH_PATH)
torch_mesh = unreal.load_asset(TORCH_MESH_PATH)
fuse_guide_mesh = unreal.load_asset(FUSE_GUIDE_MESH_PATH)
fuse_guide_material = unreal.load_asset(FUSE_GUIDE_MATERIAL_PATH)
if (not hwacha_bp or not fire_system or not hwacha_mesh or not torch_mesh or
        not fuse_guide_mesh or not fuse_guide_material):
    raise RuntimeError("Hwacha Blueprint or Fuse guide assets are missing")

# NS_Fire uses a CPU Static Mesh data interface. Without CPU access it retries
# against the owning Hwacha mesh every frame and floods the VR PIE log.
hwacha_mesh.set_editor_property("allow_cpu_access", True)
unreal.EditorAssetLibrary.save_loaded_asset(hwacha_mesh, only_if_is_dirty=False)
torch_mesh.set_editor_property("allow_cpu_access", True)
unreal.EditorAssetLibrary.save_loaded_asset(torch_mesh, only_if_is_dirty=False)

unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
configure_hwacha(unreal.get_default_object(hwacha_bp.generated_class()))
unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
unreal.EditorAssetLibrary.save_loaded_asset(hwacha_bp, only_if_is_dirty=False)

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for level_path in LEVEL_PATHS:
    if not unreal.EditorAssetLibrary.does_asset_exist(level_path):
        continue
    unreal.EditorLoadingAndSavingUtils.load_map(level_path)
    for actor in actor_subsystem.get_all_level_actors():
        if isinstance(actor, unreal.SingijeonHwachaActor):
            configure_hwacha(actor)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

unreal.log("SINGIJEON_CARRY_FUSE CONFIGURE SUCCESS")
