import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_CARRY_FUSE PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_CARRY_FUSE FAIL: {message}")


bp = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
fire_system = unreal.load_asset("/Game/NiagaraExamples/FX_Misc/NS_Fire")
guide_mesh = unreal.load_asset("/Engine/BasicShapes/Sphere")
guide_material = unreal.load_asset(
    "/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/MI_GlowNT")
check(bp is not None, "BP_SingijeonHwacha loads")
if bp:
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())
    fuse = cdo.get_editor_property("fuse")
    carry = cdo.get_editor_property("two_hand_carry")
    effect = cdo.get_editor_property("fuse_ignition_effect")
    guide = cdo.get_editor_property("fuse_guide")
    body = cdo.get_editor_property("body_mesh")
    left_highlight = cdo.get_editor_property("left_handle_highlight")
    right_highlight = cdo.get_editor_property("right_handle_highlight")
    check(body.get_editor_property("mobility") == unreal.ComponentMobility.MOVABLE,
          "Hwacha root remains movable while a handle is held")
    check(fuse.get_unscaled_sphere_radius() >= 24.0,
          "Fuse overlap radius is suitable for a held VR torch")
    check(fuse.get_editor_property("generate_overlap_events"),
          "Fuse generates overlap events")
    check(carry.get_editor_property("allow_single_hand_carry"),
          "Either hand can carry the Hwacha")
    check(not carry.get_editor_property("sweep_movement"),
          "Floor contact cannot block direct hand-follow movement")
    check(effect is not None, "Hwacha owns FuseIgnitionEffect")
    check(effect and effect.get_editor_property("asset") == fire_system,
          "FuseIgnitionEffect uses NS_Fire")
    check(effect and not effect.get_editor_property("auto_activate"),
          "Fuse effect only runs during ignition progress")
    check(guide is not None and guide.get_editor_property("static_mesh") == guide_mesh,
          "Fuse owns a visible sphere guide")
    check(guide and guide.get_material(0) == guide_material,
          "Fuse sphere uses the glow guide material")
    fuse_location = fuse.get_editor_property("relative_location")
    check(abs(fuse_location.x + 35.0) < 0.1 and
          abs(fuse_location.y) < 0.1 and abs(fuse_location.z - 60.0) < 0.1,
          "Fuse contact is fixed at the rear-center guide location")
    check(left_highlight.component_has_tag("VRGrab") and
          right_highlight.component_has_tag("VRGrab"),
          "The two visible handle cylinders are direct VRGrab targets")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
placed = next((actor for actor in actors if isinstance(actor, unreal.SingijeonHwachaActor)), None)
check(placed is not None, "Playable Hwacha remains placed in LV_Singijeon")
if placed:
    check(placed.get_editor_property("body_mesh").get_editor_property(
        "mobility") == unreal.ComponentMobility.MOVABLE,
          "Placed Hwacha root is movable")
    check(not placed.get_editor_property("two_hand_carry").get_editor_property("sweep_movement"),
          "Placed Hwacha uses non-swept hand following")
    check(placed.get_editor_property("fuse").get_unscaled_sphere_radius() >= 24.0,
          "Placed Hwacha has the widened Fuse overlap")
    check(placed.get_editor_property("fuse_guide") is not None,
          "Placed Hwacha contains the Fuse location guide")

if errors:
    raise RuntimeError("Singijeon carry/fuse verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_CARRY_FUSE VERIFY SUCCESS")
