import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_AUTO_FILL_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_AUTO_FILL_VERIFY FAIL: {message}")


hwacha_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
arrow_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonArrow")
arrow_mesh_asset = unreal.load_asset(
    "/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb"
)
hwacha_mesh_asset = unreal.load_asset(
    "/GF_Singijeon/Asset/Hwacha/hwacha/StaticMeshes/hwacha"
)
check(hwacha_bp is not None, "BP_SingijeonHwacha loads")
check(arrow_bp is not None, "BP_SingijeonArrow loads")
check(arrow_mesh_asset is not None, "Arrow Static Mesh loads")
check(hwacha_mesh_asset is not None, "Hwacha Static Mesh loads")

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
    check(cdo.get_editor_property("body_mesh").get_editor_property("static_mesh") == hwacha_mesh_asset,
          "Playable Hwacha uses the moved Hwacha mesh")
    instances = cdo.get_editor_property("auto_loaded_arrow_instances")
    check(instances is not None, "Hwacha owns AutoLoadedArrowInstances")
    if instances:
        check(instances.get_editor_property("static_mesh") == arrow_mesh_asset,
              "AutoLoadedArrowInstances references the Arrow mesh")

if arrow_bp and arrow_mesh_asset:
    unreal.BlueprintEditorLibrary.compile_blueprint(arrow_bp)
    cdo = unreal.get_default_object(arrow_bp.generated_class())
    projectile_mesh = cdo.get_editor_property("projectile_mesh")
    check(projectile_mesh.get_editor_property("static_mesh") == arrow_mesh_asset,
          "Playable Arrow uses the Arrow mesh")

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

if errors:
    raise RuntimeError("Singijeon auto-fill verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_AUTO_FILL_ARROW_GRID VERIFY SUCCESS")
