import unreal


HWACHA_BP_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonHwacha"
MATERIAL_DIR = "/GF_Singijeon/Gameplay/Materials"
MATERIAL_NAME = "M_HwachaHologram"
MATERIAL_PATH = f"{MATERIAL_DIR}/{MATERIAL_NAME}"


asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
material = unreal.load_asset(MATERIAL_PATH)
if not material:
    material = asset_tools.create_asset(
        MATERIAL_NAME,
        MATERIAL_DIR,
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
if not material:
    raise RuntimeError("Could not create the Hwacha hologram material")

material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
material.set_editor_property("two_sided", True)
unreal.MaterialEditingLibrary.delete_all_material_expressions(material)

emissive = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant3Vector, -300, -100
)
emissive.set_editor_property("constant", unreal.LinearColor(0.0, 0.65, 1.0, 1.0))
opacity = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -300, 100
)
opacity.set_editor_property("r", 0.28)
unreal.MaterialEditingLibrary.connect_material_property(
    emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
)
unreal.MaterialEditingLibrary.connect_material_property(
    opacity, "", unreal.MaterialProperty.MP_OPACITY
)
unreal.MaterialEditingLibrary.recompile_material(material)

hwacha_bp = unreal.load_asset(HWACHA_BP_PATH)
if not hwacha_bp:
    raise RuntimeError("BP_SingijeonHwacha is missing")

unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
cdo = unreal.get_default_object(hwacha_bp.generated_class())
body = cdo.get_editor_property("body_mesh")
marker = cdo.get_editor_property("move_target_marker")
carry = cdo.get_editor_property("two_hand_carry")
hwacha_mesh = body.get_editor_property("static_mesh")
if not hwacha_mesh:
    raise RuntimeError("BP_SingijeonHwacha BodyMesh has no Static Mesh")

cdo.set_editor_property("show_move_target_marker", True)
cdo.set_editor_property("move_target_acceptance_radius", 55.0)
cdo.set_editor_property("volley_duration", 10.0)
carry.set_editor_property("allow_single_hand_carry", True)
carry.set_editor_property("follow_hand_without_lag", True)

marker.set_static_mesh(hwacha_mesh)
for material_index in range(max(1, len(hwacha_mesh.get_editor_property("static_materials")))):
    marker.set_material(material_index, material)
marker.set_editor_property("relative_location", unreal.Vector(250.0, 0.0, 0.0))
marker.set_editor_property("relative_rotation", unreal.Rotator())
marker.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))
marker.set_editor_property("visible", False)
marker.set_editor_property("hidden_in_game", True)

unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
unreal.EditorAssetLibrary.save_loaded_asset(hwacha_bp, only_if_is_dirty=False)
unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("SINGIJEON_HWACHA_DRAG_VOLLEY CONFIGURE SUCCESS")
