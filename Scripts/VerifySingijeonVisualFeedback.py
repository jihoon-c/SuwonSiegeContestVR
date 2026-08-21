import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_VISUAL_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_VISUAL_VERIFY FAIL: {message}")


firepit_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonFirePit")
hwacha_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
arrow_mesh = unreal.load_asset("/GF_Singijeon/Asset/Arrow/arrowb/StaticMeshes/arrowb")
arrow_material = unreal.load_asset("/GF_Singijeon/Asset/Arrow/arrowb/Materials/M_Arrow01b")
handle_mesh = unreal.load_asset("/Engine/BasicShapes/Cylinder")
handle_material = unreal.load_asset(
    "/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/MI_GlowNT"
)
check(firepit_bp is not None, "Fire Pit Blueprint loads")
check(hwacha_bp is not None, "Hwacha Blueprint loads")

if firepit_bp:
    unreal.BlueprintEditorLibrary.compile_blueprint(firepit_bp)
    firepit_cdo = unreal.get_default_object(firepit_bp.generated_class())
    ignition_location = firepit_cdo.get_editor_property("ignition_area").get_editor_property(
        "relative_location"
    )
    check(abs(ignition_location.z - 102.0) < 0.1, "Fire Pit ignition area is at mesh top")

    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    bfl = unreal.SubobjectDataBlueprintFunctionLibrary
    fire_effects = []
    for handle in subsystem.k2_gather_subobject_data_for_blueprint(firepit_bp):
        component = bfl.get_object(bfl.get_data(handle))
        if isinstance(component, unreal.NiagaraComponent):
            fire_effects.append(component)
    check(len(fire_effects) == 1, "Fire Pit owns one Niagara fire effect")
    if fire_effects:
        effect_location = fire_effects[0].get_editor_property("relative_location")
        check(abs(effect_location.z - 155.0) < 0.1,
              "Niagara fire visual lower edge clears the bowl rim")

if hwacha_bp:
    unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
    hwacha_cdo = unreal.get_default_object(hwacha_bp.generated_class())
    instances = hwacha_cdo.get_editor_property("auto_loaded_arrow_instances")
    check(hwacha_cdo.get_editor_property("auto_fill_arrow_mesh") == arrow_mesh,
          "Auto-fill Arrow mesh is assigned")
    check(hwacha_cdo.get_editor_property("auto_fill_arrow_material") == arrow_material,
          "Auto-fill Arrow material override is assigned")
    check(instances.get_material(0) == arrow_material,
          "AutoLoadedArrowInstances resolves the Arrow material")
    check(hwacha_cdo.get_editor_property("enable_aim_guide_highlight"),
          "Aim guide highlighting is enabled")
    check(abs(hwacha_cdo.get_editor_property("aim_completion_distance") - 30.0) < 0.1,
          "Aim movement completion distance is configured")
    check(abs(hwacha_cdo.get_editor_property("aim_completion_yaw_degrees") - 10.0) < 0.1,
          "Aim rotation completion angle is configured")

    for property_name, expected_y in (
        ("left_handle_highlight", -43.0),
        ("right_handle_highlight", 43.0),
    ):
        component = hwacha_cdo.get_editor_property(property_name)
        check(component.get_editor_property("static_mesh") == handle_mesh,
              f"{property_name} has its proxy mesh")
        check(component.get_material(0) == handle_material,
              f"{property_name} has its glow material")
        location = component.get_editor_property("relative_location")
        check(abs(location.y - expected_y) < 0.1,
              f"{property_name} is aligned to its wooden handle")
        check(not component.get_editor_property("visible"),
              f"{property_name} starts hidden")
        check(component.get_editor_property("hidden_in_game"),
              f"{property_name} starts hidden before the aim interaction activates")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
firepit_actor = next(
    (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonFirePit_Playable"),
    None,
)
hwacha_actor = next(
    (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonHwacha_Playable"),
    None,
)
check(firepit_actor is not None, "LV_Singijeon contains the playable Fire Pit")
check(hwacha_actor is not None, "LV_Singijeon contains the playable Hwacha")
if firepit_actor:
    check(abs(firepit_actor.get_editor_property("ignition_area").get_editor_property(
        "relative_location").z - 102.0) < 0.1, "Placed Fire Pit resolves the raised ignition height")
    placed_effects = firepit_actor.get_components_by_class(unreal.NiagaraComponent)
    check(any(abs(effect.get_editor_property("relative_location").z - 155.0) < 0.1
              for effect in placed_effects if effect.get_name() == "FireEffect"),
          "Placed Fire Pit resolves the corrected fire visual height")
if hwacha_actor:
    check(hwacha_actor.get_editor_property("auto_fill_arrow_material") == arrow_material,
          "Placed Hwacha resolves the Arrow material")

if errors:
    raise RuntimeError("Singijeon visual verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_VISUAL_FEEDBACK VERIFY SUCCESS")
