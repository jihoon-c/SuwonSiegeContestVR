import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_SINGLE_HAND_MOVE PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_SINGLE_HAND_MOVE FAIL: {message}")


hwacha_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
hologram_material = unreal.load_asset(
    "/GF_Singijeon/Gameplay/Materials/M_HwachaHologram"
)
check(hwacha_bp is not None, "BP_SingijeonHwacha loads")
check(hologram_material is not None, "Hwacha hologram material loads")

if hwacha_bp:
    unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
    cdo = unreal.get_default_object(hwacha_bp.generated_class())
    carry = cdo.get_editor_property("two_hand_carry")
    body = cdo.get_editor_property("body_mesh")
    marker = cdo.get_editor_property("move_target_marker")
    check(carry.get_editor_property("allow_single_hand_carry"),
          "Single-hand carry is enabled for left or right grip")
    check(cdo.get_editor_property("enable_aim_guide_highlight"),
          "Handle grab guides remain enabled")
    check(cdo.get_editor_property("show_move_target_marker"),
          "Destination marker is enabled")
    check(abs(cdo.get_editor_property("move_target_acceptance_radius") - 55.0) < 0.1,
          "Destination acceptance radius is 55 cm")
    check(marker.get_editor_property("static_mesh") == body.get_editor_property("static_mesh"),
          "Destination marker uses the full Hwacha shape")
    check(marker.get_material(0) == hologram_material,
          "Destination marker uses the Hwacha hologram material")
    location = marker.get_editor_property("relative_location")
    check(abs(location.x - 250.0) < 0.1 and abs(location.z) < 0.1,
          "Destination hologram is 250 cm ahead at the Hwacha base height")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
placed = next(
    (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonHwacha_Playable"),
    None,
)
check(placed is not None, "LV_Singijeon contains the playable Hwacha")
if placed:
    check(placed.get_editor_property("show_move_target_marker"),
          "Placed Hwacha inherits the destination marker")
    check(placed.get_editor_property("two_hand_carry").get_editor_property(
        "allow_single_hand_carry"), "Placed Hwacha inherits single-hand carry")

if errors:
    raise RuntimeError("Single-hand target move verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_SINGLE_HAND_TARGET_MOVE VERIFY SUCCESS")
