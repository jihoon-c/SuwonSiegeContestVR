import unreal


ROOT = "/GF_Singijeon/Asset/NamhansanseongWall/Optimized"
SOURCE_MESH_PATH = "/GF_Singijeon/Asset/NamhansanseongWall/Meshes/SM_Stone_Barrier"
errors = []


def check(condition, message):
    if condition:
        unreal.log(f"OPTIMIZED_STONE_BARRIER_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"OPTIMIZED_STONE_BARRIER_VERIFY FAIL: {message}")


source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
low_texture = unreal.load_asset(ROOT + "/Textures/T_Stone_Barrier01a_BC_Low")
low_material = unreal.load_asset(ROOT + "/Materials/M_Stone_Barrier_Low")
prefab = unreal.load_asset(ROOT + "/Prefabs/BP_Stone_Barrier_Optimized")
check(source_mesh is not None, "Source wall mesh loads")
check(low_texture is not None, "1K wall Base Color texture loads")
check(low_material is not None, "Fully Rough optimized wall material loads")
check(prefab is not None, "Optimized wall prefab loads")

if low_texture:
    check(low_texture.get_editor_property("max_texture_size") == 1024,
          "Optimized texture is capped at 1024 pixels")
if low_material:
    check(low_material.get_editor_property("fully_rough"),
          "Optimized material is Fully Rough")

if prefab:
    unreal.BlueprintEditorLibrary.compile_blueprint(prefab)
    subobjects = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    subobject_bfl = unreal.SubobjectDataBlueprintFunctionLibrary
    mesh_component = next(
        (component for handle in subobjects.k2_gather_subobject_data_for_blueprint(prefab)
         for component in [subobject_bfl.get_object(subobject_bfl.get_data(handle))]
         if isinstance(component, unreal.StaticMeshComponent) and
         component.get_editor_property("static_mesh") == source_mesh), None
    )
    check(mesh_component is not None, "Prefab owns the optimized wall mesh component")
    if mesh_component:
        check(mesh_component.get_editor_property("static_mesh") == source_mesh,
              "Prefab uses SM_Stone_Barrier geometry")
        check(mesh_component.get_material(0) == low_material,
              "Prefab slot 0 uses optimized material")
        check(mesh_component.get_material(1) == low_material,
              "Prefab slot 1 uses the same optimized material")
        check(mesh_component.get_editor_property("ld_max_draw_distance") == 3500.0,
              "Prefab enables 35m distance culling")

if errors:
    raise RuntimeError("Optimized stone barrier verification failed: " + "; ".join(errors))
unreal.log("OPTIMIZED_STONE_BARRIER_VERIFY SUCCESS")
