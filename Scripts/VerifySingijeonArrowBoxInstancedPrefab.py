import unreal


ROOT = "/GF_Singijeon/Gameplay/Props"
PREFAB_PATH = ROOT + "/BP_SingijeonArrowBox_Instanced"
ARROW_MESH_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb"
ARROW_MATERIAL_PATH = "/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime"
errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_ARROW_BOX_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_ARROW_BOX_VERIFY FAIL: {message}")


prefab = unreal.load_asset(PREFAB_PATH)
arrow_mesh = unreal.load_asset(ARROW_MESH_PATH)
arrow_material = unreal.load_asset(ARROW_MATERIAL_PATH)
check(prefab is not None, "Arrow box prefab loads")
check(arrow_mesh is not None, "Source arrow mesh loads")
check(arrow_material is not None, "Runtime arrow material loads")

if prefab:
    unreal.BlueprintEditorLibrary.compile_blueprint(prefab)
    preview_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        prefab.generated_class(), unreal.Vector(0.0, 0.0, -100000.0), unreal.Rotator())
    components = preview_actor.get_components_by_class(unreal.ActorComponent) if preview_actor else []
    tray_names = {"ArrowBoxBottom", "ArrowBoxLeftWall", "ArrowBoxRightWall",
                  "ArrowBoxFrontWall", "ArrowBoxBackWall"}
    found_names = {component.get_name() for component in components if component}
    check(tray_names.issubset(found_names), "Prefab contains the five wooden tray boards")
    instances = next((component for component in components
                      if isinstance(component, unreal.InstancedStaticMeshComponent) and
                      component.get_name() == "ArrowInstances"), None)
    check(instances is not None, "Prefab owns ArrowInstances ISM component")
    if instances:
        check(instances.get_editor_property("static_mesh") == arrow_mesh,
              "ISM uses the Singijeon arrow mesh")
        check(instances.get_material(0) == arrow_material,
              "ISM uses the runtime instancing-safe arrow material")
        check(instances.get_instance_count() == 90,
              "ISM contains 6 x 15 = 90 arrows")
        check(not instances.get_editor_property("cast_shadow"),
              "Arrow instances disable individual shadows for VR cost control")
        check(instances.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION,
              "Arrow display instances have no collision")
    if preview_actor:
        unreal.EditorLevelLibrary.destroy_actor(preview_actor)

if errors:
    raise RuntimeError("Arrow box prefab verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_ARROW_BOX_VERIFY SUCCESS")
