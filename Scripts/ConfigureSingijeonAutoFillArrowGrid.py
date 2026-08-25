import unreal


HWACHA_BP_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonHwacha"
ARROW_BP_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonArrow"
ARROW_MESH_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb"
ARROW_MATERIAL_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime"
HWACHA_MESH_PATH = "/GF_Singijeon/Asset/Hwacha/hwacha/StaticMeshes/hwacha"

hwacha_bp = unreal.load_asset(HWACHA_BP_PATH)
arrow_bp = unreal.load_asset(ARROW_BP_PATH)
arrow_mesh_asset = unreal.load_asset(ARROW_MESH_PATH)
arrow_material_asset = unreal.load_asset(ARROW_MATERIAL_PATH)
hwacha_mesh_asset = unreal.load_asset(HWACHA_MESH_PATH)
if (not hwacha_bp or not arrow_bp or not arrow_mesh_asset or
        not arrow_material_asset or not hwacha_mesh_asset):
    raise RuntimeError(
        "Hwacha Blueprint, Arrow Blueprint, Hwacha Static Mesh, or Arrow Static Mesh is missing"
    )

hwacha_cdo = unreal.get_default_object(hwacha_bp.generated_class())
hwacha_cdo.get_editor_property("body_mesh").set_static_mesh(hwacha_mesh_asset)
hwacha_cdo.set_editor_property("auto_fill_on_first_load", True)
hwacha_cdo.set_editor_property("auto_fill_rows", 6)
hwacha_cdo.set_editor_property("auto_fill_columns", 11)
hwacha_cdo.set_editor_property("auto_fill_column_spacing", 8.0)
hwacha_cdo.set_editor_property("auto_fill_row_spacing", 8.0)
hwacha_cdo.set_editor_property("auto_fill_arrow_mesh", arrow_mesh_asset)
hwacha_cdo.set_editor_property("auto_fill_arrow_material", arrow_material_asset)
hwacha_cdo.set_editor_property("minimum_loaded_ammunition", 66)

instance_component = hwacha_cdo.get_editor_property("auto_loaded_arrow_instances")
instance_component.set_editor_property("absolute_location", False)
instance_component.set_editor_property("absolute_rotation", False)
instance_component.set_editor_property("absolute_scale", False)
instance_component.set_editor_property("relative_location", unreal.Vector())
instance_component.set_editor_property("relative_rotation", unreal.Rotator())
instance_component.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))
instance_component.set_static_mesh(arrow_mesh_asset)
for material_index in range(max(1, len(arrow_mesh_asset.get_editor_property("static_materials")))):
    instance_component.set_material(material_index, arrow_material_asset)

arrow_cdo = unreal.get_default_object(arrow_bp.generated_class())
arrow_cdo.set_editor_property("projectile_material_override", arrow_material_asset)
projectile_mesh = arrow_cdo.get_editor_property("projectile_mesh")
previous_mesh = projectile_mesh.get_editor_property("static_mesh")
projectile_mesh.set_static_mesh(arrow_mesh_asset)
for material_index in range(max(1, len(arrow_mesh_asset.get_editor_property("static_materials")))):
    arrow_mesh_asset.set_material(material_index, arrow_material_asset)
    projectile_mesh.set_material(material_index, arrow_material_asset)
if previous_mesh and "/Engine/BasicShapes/Cylinder" in previous_mesh.get_path_name():
    projectile_mesh.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))
    projectile_mesh.set_editor_property("relative_rotation", unreal.Rotator())

for blueprint in (hwacha_bp, arrow_bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
hwacha_actor = next(
    (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonHwacha_Playable"),
    None,
)
if hwacha_actor:
    hwacha_actor.set_editor_property("auto_fill_rows", 6)
    hwacha_actor.set_editor_property("auto_fill_columns", 11)
    hwacha_actor.set_editor_property("minimum_loaded_ammunition", 66)
    hwacha_actor.set_editor_property("auto_fill_arrow_mesh", arrow_mesh_asset)
    hwacha_actor.set_editor_property("auto_fill_arrow_material", arrow_material_asset)
    for component in hwacha_actor.get_components_by_class(unreal.InstancedStaticMeshComponent):
        if component.get_name() == "AutoLoadedArrowInstances":
            component.set_editor_property("absolute_location", False)
            component.set_editor_property("absolute_rotation", False)
            component.set_editor_property("absolute_scale", False)
            component.set_editor_property("relative_location", unreal.Vector())
            component.set_editor_property("relative_rotation", unreal.Rotator())
            component.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))
            component.set_static_mesh(arrow_mesh_asset)
            for material_index in range(max(1, len(arrow_mesh_asset.get_editor_property("static_materials")))):
                component.set_material(material_index, arrow_material_asset)
for actor in actors:
    if isinstance(actor, unreal.SingijeonProjectileActor):
        placed_projectile_mesh = actor.get_editor_property("projectile_mesh")
        for material_index in range(max(1, len(arrow_mesh_asset.get_editor_property("static_materials")))):
            placed_projectile_mesh.set_material(material_index, arrow_material_asset)
unreal.EditorAssetLibrary.save_loaded_asset(arrow_mesh_asset, only_if_is_dirty=False)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

unreal.log("SINGIJEON_AUTO_FILL_ARROW_GRID CONFIGURE SUCCESS")
