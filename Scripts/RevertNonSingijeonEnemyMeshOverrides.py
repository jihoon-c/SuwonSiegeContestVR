"""Restore the shared/Ongseong enemy Blueprints changed during the incorrect scope update."""

import unreal


BLUEPRINT_PATHS = (
    "/Game/Gameplay/Characters/BP_EnemySoldier",
    "/GF_OngseongCrossbow/Blueprints/BP_OngseongEnemySoldier",
)
ORIGINAL_MESH_PATH = "/Game/NiagaraExamples/Gallery/SkeletalMesh/Mannequins/Meshes/SKM_Manny_Simple"


original_mesh = unreal.load_asset(ORIGINAL_MESH_PATH)
if not isinstance(original_mesh, unreal.SkeletalMesh):
    raise RuntimeError(f"Missing original enemy mesh: {ORIGINAL_MESH_PATH}")

for blueprint_path in BLUEPRINT_PATHS:
    blueprint = unreal.load_asset(blueprint_path)
    if not blueprint or not blueprint.generated_class():
        raise RuntimeError(f"Missing Blueprint: {blueprint_path}")
    cdo = unreal.get_default_object(blueprint.generated_class())
    component = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
    if not component:
        raise RuntimeError(f"No SkeletalMeshComponent on {blueprint_path}")

    relative_location = component.get_editor_property("relative_location")
    relative_rotation = component.get_editor_property("relative_rotation")
    component.set_editor_property("skeletal_mesh", original_mesh)
    component.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))
    if (component.get_editor_property("relative_location") != relative_location or
            component.get_editor_property("relative_rotation") != relative_rotation):
        raise RuntimeError(f"Refusing to save: mesh transform changed on {blueprint_path}")
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {blueprint_path}")

unreal.log("NON_SINGIJEON_ENEMY_REVERT SUCCESS: shared and Ongseong mesh overrides restored")
