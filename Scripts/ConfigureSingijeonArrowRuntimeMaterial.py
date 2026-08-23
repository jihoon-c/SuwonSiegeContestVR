import unreal


MATERIAL_DIR = "/GF_Singijeon/Asset/Arrow/arrowb/Materials"
MATERIAL_NAME = "M_SingijeonArrow_Runtime"
MATERIAL_PATH = f"{MATERIAL_DIR}/{MATERIAL_NAME}"
ARROW_MESH_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb"
BASE_COLOR_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/Textures/arrowb_texture_0"
METALLIC_ROUGHNESS_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/Textures/arrowb_texture_1"
NORMAL_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/Textures/arrowb_texture_2"


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
    raise RuntimeError("Could not create the project-owned Singijeon arrow material")

# Auto-loaded rack arrows render through UInstancedStaticMeshComponent. Without
# this usage flag Unreal does not compile the ISM vertex-factory shader permutation,
# even when the component's material reference itself is valid.
material.set_editor_property("used_with_instanced_static_meshes", True)

base_color = unreal.load_asset(BASE_COLOR_PATH)
metallic_roughness = unreal.load_asset(METALLIC_ROUGHNESS_PATH)
normal = unreal.load_asset(NORMAL_PATH)
arrow_mesh = unreal.load_asset(ARROW_MESH_PATH)
if not base_color or not metallic_roughness or not normal or not arrow_mesh:
    raise RuntimeError("Arrow textures or Static Mesh are missing")

unreal.MaterialEditingLibrary.delete_all_material_expressions(material)


def texture_sample(texture, x, y):
    expression = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, x, y
    )
    expression.set_editor_property("texture", texture)
    return expression


base_sample = texture_sample(base_color, -700, -150)
packed_sample = texture_sample(metallic_roughness, -700, 100)
normal_sample = texture_sample(normal, -700, 350)
normal_sample.set_editor_property(
    "sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
)

unreal.MaterialEditingLibrary.connect_material_property(
    base_sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR
)
unreal.MaterialEditingLibrary.connect_material_property(
    packed_sample, "G", unreal.MaterialProperty.MP_ROUGHNESS
)
unreal.MaterialEditingLibrary.connect_material_property(
    packed_sample, "B", unreal.MaterialProperty.MP_METALLIC
)
unreal.MaterialEditingLibrary.connect_material_property(
    normal_sample, "RGB", unreal.MaterialProperty.MP_NORMAL
)
unreal.MaterialEditingLibrary.recompile_material(material)

for index in range(max(1, len(arrow_mesh.get_editor_property("static_materials")))):
    arrow_mesh.set_material(index, material)

unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
unreal.EditorAssetLibrary.save_loaded_asset(arrow_mesh, only_if_is_dirty=False)
unreal.log("SINGIJEON_ARROW_RUNTIME_MATERIAL CONFIGURE SUCCESS")
