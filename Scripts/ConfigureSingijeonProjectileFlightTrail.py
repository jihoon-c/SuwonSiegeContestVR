import unreal


BP_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonArrow"


blueprint = unreal.load_asset(BP_PATH)
if not blueprint:
    raise RuntimeError("BP_SingijeonArrow is missing")

cdo = unreal.get_default_object(blueprint.generated_class())
cdo.set_editor_property("flight_trail_relative_location", unreal.Vector(0.0, 0.0, 0.0))
cdo.set_editor_property("flight_trail_relative_rotation", unreal.Rotator(0.0, 0.0, 0.0))
cdo.set_editor_property("flight_trail_relative_scale", unreal.Vector(1.0, 1.0, 1.0))

unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)
unreal.log("SINGIJEON_PROJECTILE_FLIGHT_TRAIL CONFIGURE SUCCESS")
