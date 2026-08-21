import os
import unreal


SOURCE_DIR = os.path.join(
    unreal.Paths.project_dir(),
    "Plugins",
    "GameFeatures",
    "GF_Singijeon",
    "사적_남한산성_성벽_원본",
)
ROOT = "/GF_Singijeon/Asset/NamhansanseongWall"
TEXTURE_DIR = ROOT + "/Textures"
MATERIAL_DIR = ROOT + "/Materials"
MESH_DIR = ROOT + "/Meshes"


def import_file(filename, destination_path, destination_name="", options=None):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(SOURCE_DIR, filename))
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("destination_name", destination_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    if options:
        task.set_editor_property("options", options)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not task.get_editor_property("imported_object_paths"):
        raise RuntimeError(f"Import failed: {filename}")
    return list(task.get_editor_property("imported_object_paths"))


def load_required(path):
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Required asset is missing: {path}")
    return asset


def configure_texture(texture, kind):
    if kind == "BC":
        texture.set_editor_property("srgb", True)
    elif kind == "NM":
        texture.set_editor_property("srgb", False)
        texture.set_editor_property(
            "compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP
        )
    else:
        texture.set_editor_property("srgb", False)
        texture.set_editor_property(
            "compression_settings", unreal.TextureCompressionSettings.TC_MASKS
        )
    texture.modify()
    unreal.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False)


def make_material(set_name):
    asset_name = f"M_Stone_Barrier01{set_name}"
    asset_path = f"{MATERIAL_DIR}/{asset_name}"
    material = unreal.load_asset(asset_path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            asset_name,
            MATERIAL_DIR,
            unreal.Material,
            unreal.MaterialFactoryNew(),
        )
    if not material:
        raise RuntimeError(f"Material creation failed: {asset_path}")

    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
    base_color = load_required(f"{TEXTURE_DIR}/T_Stone_Barrier01{set_name}_BC")
    normal = load_required(f"{TEXTURE_DIR}/T_Stone_Barrier01{set_name}_NM")
    roughness = load_required(f"{TEXTURE_DIR}/T_Stone_Barrier01{set_name}_RN")

    base_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -420, -120
    )
    base_expr.set_editor_property("texture", base_color)
    normal_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -420, 100
    )
    normal_expr.set_editor_property("texture", normal)
    normal_expr.set_editor_property(
        "sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
    )
    roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -420, 320
    )
    roughness_expr.set_editor_property("texture", roughness)
    roughness_expr.set_editor_property(
        "sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_MASKS
    )

    unreal.MaterialEditingLibrary.connect_material_property(
        base_expr, "RGB", unreal.MaterialProperty.MP_BASE_COLOR
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        normal_expr, "RGB", unreal.MaterialProperty.MP_NORMAL
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_expr, "R", unreal.MaterialProperty.MP_ROUGHNESS
    )
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    return material


for folder in (ROOT, TEXTURE_DIR, MATERIAL_DIR, MESH_DIR):
    unreal.EditorAssetLibrary.make_directory(folder)

for texture_set in ("a", "b"):
    for texture_kind in ("BC", "NM", "RN"):
        asset_name = f"T_Stone_Barrier01{texture_set}_{texture_kind}"
        import_file(f"{asset_name}.png", TEXTURE_DIR, asset_name)
        configure_texture(load_required(f"{TEXTURE_DIR}/{asset_name}"), texture_kind)

material_a = make_material("a")
material_b = make_material("b")

fbx_options = unreal.FbxImportUI()
fbx_options.set_editor_property("automated_import_should_detect_type", False)
fbx_options.set_editor_property("import_mesh", True)
fbx_options.set_editor_property("import_as_skeletal", False)
fbx_options.set_editor_property("import_materials", False)
fbx_options.set_editor_property("import_textures", False)
fbx_options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
fbx_options.get_editor_property("static_mesh_import_data").set_editor_property(
    "combine_meshes", True
)

import_file("Stone_Barrier.fbx", MESH_DIR, "SM_Stone_Barrier", fbx_options)
mesh = load_required(f"{MESH_DIR}/SM_Stone_Barrier")
static_materials = list(mesh.get_editor_property("static_materials"))
if not static_materials:
    raise RuntimeError("Imported wall mesh has no material slots")

for index, static_material in enumerate(static_materials):
    slot_name = str(static_material.get_editor_property("material_slot_name")).lower()
    selected = material_b if "01b" in slot_name or slot_name.endswith("b") else material_a
    if len(static_materials) > 1 and not ("01a" in slot_name or "01b" in slot_name):
        selected = material_a if index == 0 else material_b
    mesh.set_material(index, selected)

mesh.modify()
unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
unreal.log(
    f"NAMHANSANSEONG_WALL_IMPORT SUCCESS: {mesh.get_path_name()}, "
    f"material slots={len(static_materials)}"
)
