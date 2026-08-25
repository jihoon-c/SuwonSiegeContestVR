import unreal


BP_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonHwacha"
LEVEL_PATH = "/Game/Maps/LV_Singijeon"
CYLINDER_PATH = "/Engine/BasicShapes/Cylinder"
WHITE_MATERIAL_PATH = "/Game/NiagaraExamples/Gallery/StaticMesh/BeachBall/M_BeachBallWhite"


def configure(hwacha):
    hwacha.set_editor_property(
        "ammo_grid_center_arrow_offset", unreal.Vector(0.0, 0.0, 20.0))
    hwacha.set_editor_property(
        "ammo_grid_center_arrow_rotation", unreal.Rotator(0.0, 180.0, 0.0))
    hwacha.set_editor_property("fuse_cord_start", unreal.Vector(15.0, 0.0, 75.0))
    hwacha.set_editor_property("fuse_cord_radius", 0.8)

    center_arrow = hwacha.get_editor_property("ammo_grid_center_arrow")
    center_arrow.set_editor_property("hidden_in_game", True)

    fuse_cord = hwacha.get_editor_property("fuse_cord")
    fuse_cord.set_static_mesh(cylinder_mesh)
    fuse_cord.set_material(0, white_material)
    fuse_cord.set_editor_property("hidden_in_game", False)
    fuse_cord.set_editor_property("cast_shadow", False)
    fuse_cord.set_editor_property("generate_overlap_events", False)


blueprint = unreal.load_asset(BP_PATH)
cylinder_mesh = unreal.load_asset(CYLINDER_PATH)
white_material = unreal.load_asset(WHITE_MATERIAL_PATH)
if not blueprint or not cylinder_mesh or not white_material:
    raise RuntimeError("Hwacha Blueprint, Cylinder, or white material is missing")

configure(unreal.get_default_object(blueprint.generated_class()))
unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)

unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
for actor in actors:
    if isinstance(actor, unreal.SingijeonHwachaActor):
        configure(actor)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SINGIJEON_AMMO_CENTER_ARROW_FUSE_CORD CONFIGURE SUCCESS")
