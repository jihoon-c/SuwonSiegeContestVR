import unreal


PAWN_PATH = "/Game/Core/VR/Pawn/BP_VRPlayerPawn"

pawn_blueprint = unreal.load_asset(PAWN_PATH)
if not pawn_blueprint or not pawn_blueprint.generated_class():
    raise RuntimeError("BP_VRPlayerPawn could not be loaded")

pawn_cdo = unreal.get_default_object(pawn_blueprint.generated_class())
pawn_cdo.set_editor_property("subtitle_hud_offset", unreal.Vector(85.0, 0.0, -28.0))
subtitle_hud = pawn_cdo.get_editor_property("subtitle_hud")
if not subtitle_hud:
    raise RuntimeError("BP_VRPlayerPawn SubtitleHUD component is missing")
subtitle_hud.set_widget_space(unreal.WidgetSpace.WORLD)
subtitle_hud.set_editor_property("relative_location", unreal.Vector(85.0, 0.0, -28.0))
subtitle_hud.set_editor_property("relative_scale3d", unreal.Vector(0.05, 0.05, 0.05))
subtitle_hud.set_editor_property("translucency_sort_priority", 10000)

unreal.BlueprintEditorLibrary.compile_blueprint(pawn_blueprint)
if not unreal.EditorAssetLibrary.save_loaded_asset(
        pawn_blueprint, only_if_is_dirty=False):
    raise RuntimeError("BP_VRPlayerPawn could not be saved")

unreal.log("SCENARIO_INTERACTION_GUIDES_AND_SUBTITLE SETUP SUCCESS")
