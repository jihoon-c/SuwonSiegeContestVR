import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SCENARIO_GUIDE_SUBTITLE_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SCENARIO_GUIDE_SUBTITLE_VERIFY FAIL: {message}")


pawn_blueprint = unreal.load_asset("/Game/Core/VR/Pawn/BP_VRPlayerPawn")
check(pawn_blueprint is not None, "BP_VRPlayerPawn loads")
if pawn_blueprint and pawn_blueprint.generated_class():
    pawn_cdo = unreal.get_default_object(pawn_blueprint.generated_class())
    offset = pawn_cdo.get_editor_property("subtitle_hud_offset")
    subtitle_hud = pawn_cdo.get_editor_property("subtitle_hud")
    check(abs(offset.x - 85.0) < 0.01 and abs(offset.z + 28.0) < 0.01,
          "Subtitle is authored 85 cm in front of the HMD")
    check(subtitle_hud is not None, "SubtitleHUD component exists")
    if subtitle_hud:
        check(subtitle_hud.get_editor_property("space") == unreal.WidgetSpace.WORLD,
              "SubtitleHUD uses OpenXR-compatible World Space")
        check(subtitle_hud.get_editor_property("translucency_sort_priority") == 10000,
              "SubtitleHUD has the highest transparent sort priority")

manager_class = unreal.load_class(None, "/Script/SuwonSiegeContestVR.ScenarioManagerActor")
check(manager_class is not None, "ScenarioManagerActor class loads")
if manager_class:
    manager_cdo = unreal.get_default_object(manager_class)
    guide = manager_cdo.get_editor_property("interaction_guide")
    check(guide is not None, "Scenario Manager owns the common InteractionGuide")
    if guide:
        check(guide.get_editor_property("enable_interaction_guides"),
              "Interaction guides are enabled by default")
        check(guide.get_editor_property("guide_widget_class") is not None,
              "Interaction guide has a default native Widget class")
        check(guide.get_editor_property("scale_with_view_distance"),
              "Interaction guides scale with HMD distance")
        check(abs(guide.get_editor_property("scale_reference_distance") - 300.0) < 0.01,
              "Interaction guide distance scaling uses a 3 m reference")
        check(abs(guide.get_editor_property("maximum_world_scale") - 0.65) < 0.01,
              "Interaction guide distance scaling has a safe upper bound")

if errors:
    raise RuntimeError("Scenario guide/subtitle verification failed: " + "; ".join(errors))
unreal.log("SCENARIO_INTERACTION_GUIDES_AND_SUBTITLE VERIFY SUCCESS")
