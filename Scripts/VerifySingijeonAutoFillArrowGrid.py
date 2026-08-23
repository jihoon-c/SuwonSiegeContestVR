import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_AUTO_FILL_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_AUTO_FILL_VERIFY FAIL: {message}")


def vector_near_zero(value, tolerance=0.001):
    return abs(value.x) <= tolerance and abs(value.y) <= tolerance and abs(value.z) <= tolerance


hwacha_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
arrow_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonArrow")
arrow_mesh_asset = unreal.load_asset(
    "/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb"
)
arrow_material_asset = unreal.load_asset(
    "/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_SingijeonArrow_Runtime"
)
hwacha_mesh_asset = unreal.load_asset(
    "/GF_Singijeon/Asset/Hwacha/hwacha/StaticMeshes/hwacha"
)
check(hwacha_bp is not None, "BP_SingijeonHwacha loads")
check(arrow_bp is not None, "BP_SingijeonArrow loads")
check(arrow_mesh_asset is not None, "Arrow Static Mesh loads")
check(arrow_material_asset is not None, "Project-owned Arrow runtime Material loads")
check(hwacha_mesh_asset is not None, "Hwacha Static Mesh loads")
if arrow_material_asset:
    check(arrow_material_asset.get_editor_property("used_with_instanced_static_meshes"),
          "Arrow runtime Material compiles the Instanced Static Mesh shader usage")

if hwacha_bp and arrow_mesh_asset and hwacha_mesh_asset:
    unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
    cdo = unreal.get_default_object(hwacha_bp.generated_class())
    check(cdo.get_editor_property("auto_fill_on_first_load"), "Auto fill is enabled")
    check(cdo.get_editor_property("auto_fill_rows") == 6, "Auto fill has 6 rows")
    check(cdo.get_editor_property("auto_fill_columns") == 15, "Auto fill has 15 columns")
    check(cdo.get_editor_property("minimum_loaded_ammunition") == 90,
          "Hwacha requires the completed 90-arrow load")
    check(cdo.get_editor_property("auto_fill_arrow_mesh") == arrow_mesh_asset,
          "Auto-fill ISM uses the Arrow mesh")
    check(cdo.get_editor_property("auto_fill_arrow_material") == arrow_material_asset,
          "Auto-fill ISM uses the project-owned Arrow material")
    check(cdo.get_editor_property("body_mesh").get_editor_property("static_mesh") == hwacha_mesh_asset,
          "Playable Hwacha uses the moved Hwacha mesh")
    instances = cdo.get_editor_property("auto_loaded_arrow_instances")
    check(instances is not None, "Hwacha owns AutoLoadedArrowInstances")
    if instances:
        check(instances.get_editor_property("static_mesh") == arrow_mesh_asset,
              "AutoLoadedArrowInstances references the Arrow mesh")
        check(instances.get_material(0) == arrow_material_asset,
              "AutoLoadedArrowInstances uses the project-owned material override")
        check(not instances.get_editor_property("absolute_location") and
              not instances.get_editor_property("absolute_rotation") and
              not instances.get_editor_property("absolute_scale"),
              "AutoLoadedArrowInstances inherits the Hwacha transform")
        check(instances.get_attach_parent() is not None and
              instances.get_attach_parent().get_name() == "RackRoot" and
              vector_near_zero(instances.get_editor_property("relative_location")),
              "AutoLoadedArrowInstances is locally anchored to RackRoot")

if arrow_bp and arrow_mesh_asset:
    unreal.BlueprintEditorLibrary.compile_blueprint(arrow_bp)
    cdo = unreal.get_default_object(arrow_bp.generated_class())
    projectile_mesh = cdo.get_editor_property("projectile_mesh")
    check(cdo.get_editor_property("projectile_material_override") == arrow_material_asset,
          "Playable Arrow reapplies the runtime material after Grab release")
    check(projectile_mesh.get_editor_property("static_mesh") == arrow_mesh_asset,
          "Playable Arrow uses the Arrow mesh")
    check(projectile_mesh.get_material(0) == arrow_material_asset,
          "Playable Arrow uses the project-owned runtime material")

if arrow_mesh_asset and arrow_material_asset:
    check(arrow_mesh_asset.get_material(0) == arrow_material_asset,
          "Arrow Static Mesh default slot uses the project-owned runtime material")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
hwacha = next(
    (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonHwacha_Playable"),
    None,
)
check(hwacha is not None, "LV_Singijeon contains the playable Hwacha")
if hwacha:
    check(hwacha.get_editor_property("auto_fill_rows") == 6,
          "Placed Hwacha resolves 6 rows")
    check(hwacha.get_editor_property("auto_fill_columns") == 15,
          "Placed Hwacha resolves 15 columns")
    check(hwacha.get_editor_property("auto_fill_arrow_mesh") == arrow_mesh_asset,
          "Placed Hwacha resolves the Arrow mesh")
    check(hwacha.get_editor_property("auto_fill_arrow_material") == arrow_material_asset,
          "Placed Hwacha resolves the project-owned Arrow material")
    instances = next(
        (item for item in hwacha.get_components_by_class(unreal.InstancedStaticMeshComponent)
         if item.get_name() == "AutoLoadedArrowInstances"),
        None,
    )
    check(instances is not None and instances.get_material(0) == arrow_material_asset,
          "Placed Hwacha auto-fill instances retain the project-owned material override")
    if instances:
        check(not instances.get_editor_property("absolute_location") and
              not instances.get_editor_property("absolute_rotation") and
              not instances.get_editor_property("absolute_scale"),
              "Placed Hwacha instances inherit carry movement")
        check(instances.get_attach_parent() is not None and
              instances.get_attach_parent().get_name() == "RackRoot" and
              vector_near_zero(instances.get_editor_property("relative_location")),
              "Placed Hwacha instances stay locally anchored to RackRoot")

for actor in actors:
    if isinstance(actor, unreal.SingijeonProjectileActor):
        check(actor.get_editor_property("projectile_material_override") == arrow_material_asset,
              f"Placed Arrow {actor.get_actor_label()} retains its runtime material override")

if errors:
    raise RuntimeError("Singijeon auto-fill verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_AUTO_FILL_ARROW_GRID VERIFY SUCCESS")
