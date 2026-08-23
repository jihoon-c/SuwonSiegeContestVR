import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_HWACHA_DRAG_VOLLEY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_HWACHA_DRAG_VOLLEY FAIL: {message}")


hwacha_bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
hologram = unreal.load_asset("/GF_Singijeon/Gameplay/Materials/M_HwachaHologram")
check(hwacha_bp is not None, "BP_SingijeonHwacha loads")
check(hologram is not None, "Hwacha hologram material exists")

if hwacha_bp:
    unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
    cdo = unreal.get_default_object(hwacha_bp.generated_class())
    body = cdo.get_editor_property("body_mesh")
    marker = cdo.get_editor_property("move_target_marker")
    carry = cdo.get_editor_property("two_hand_carry")
    left_grab = cdo.get_editor_property("left_handle_grab_point")
    right_grab = cdo.get_editor_property("right_handle_grab_point")
    left_highlight = cdo.get_editor_property("left_handle_highlight")
    right_highlight = cdo.get_editor_property("right_handle_highlight")
    check(left_grab.component_has_tag("VRGrab"), "Left handle has the VRGrab tag")
    check(right_grab.component_has_tag("VRGrab"), "Right handle has the VRGrab tag")
    check(left_highlight.component_has_tag("VRGrab"),
          "Visible left cylinder is a direct VRGrab target")
    check(right_highlight.component_has_tag("VRGrab"),
          "Visible right cylinder is a direct VRGrab target")
    check(carry.get_editor_property("allow_single_hand_carry"),
          "Either hand can pull the Hwacha")
    check(carry.get_editor_property("follow_hand_without_lag"),
          "Hwacha follows the grabbed hand without locomotion lag")
    check(not carry.get_editor_property("sweep_movement"),
          "Floor collision cannot reject the direct hand-follow delta")
    check(marker.get_editor_property("static_mesh") == body.get_editor_property("static_mesh"),
          "Target marker uses the full Hwacha shape")
    check(marker.get_material(0) == hologram,
          "Target marker uses the hologram material")
    location = marker.get_editor_property("relative_location")
    check(abs(location.x - 250.0) < 0.1 and abs(location.z) < 0.1,
          "Hologram target is 250 cm ahead at the Hwacha base height")
    check(abs(cdo.get_editor_property("volley_duration") - 10.0) < 0.01,
          "Volley duration is ten seconds")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
placed = next(
    (actor for actor in actors if actor.get_actor_label() == "BP_SingijeonHwacha_Playable"),
    None,
)
check(placed is not None, "Playable Hwacha remains placed in LV_Singijeon")

if errors:
    raise RuntimeError("Hwacha drag/volley verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_HWACHA_DRAG_VOLLEY VERIFY SUCCESS")
