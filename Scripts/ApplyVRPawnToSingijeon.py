import unreal


GAME_MODE_PATH = "/Game/XRFramework/Blueprints/BP_XRGameMode"
EXPECTED_PAWN_PATH = "/Game/Core/VR/Pawn/BP_VRPlayerPawn"
LEVEL_PATH = "/Game/Maps/LV_Singijeon"

game_mode_blueprint = unreal.load_asset(GAME_MODE_PATH)
expected_pawn_blueprint = unreal.load_asset(EXPECTED_PAWN_PATH)
world = unreal.load_asset(LEVEL_PATH)

if not game_mode_blueprint or not expected_pawn_blueprint or not world:
    raise RuntimeError("Required GameMode, Pawn, or Singijeon level asset could not be loaded")

game_mode_class = game_mode_blueprint.generated_class()
expected_pawn_class = expected_pawn_blueprint.generated_class()
game_mode_cdo = unreal.get_default_object(game_mode_class)
configured_pawn_class = game_mode_cdo.get_editor_property("default_pawn_class")

if configured_pawn_class != expected_pawn_class:
    raise RuntimeError(
        f"{GAME_MODE_PATH} currently uses {configured_pawn_class}, expected {expected_pawn_class}. "
        "Refusing to overwrite the existing modified GameMode asset."
    )

world_settings = world.get_world_settings()
world_settings.set_editor_property("default_game_mode", game_mode_class)
unreal.EditorAssetLibrary.save_loaded_asset(world, only_if_is_dirty=True)

saved_world = unreal.load_asset(LEVEL_PATH)
saved_game_mode = saved_world.get_world_settings().get_editor_property("default_game_mode")
if saved_game_mode != game_mode_class:
    raise RuntimeError(f"Failed to persist GameMode override on {LEVEL_PATH}")

unreal.log(f"Configured {LEVEL_PATH} GameMode={game_mode_class}")
unreal.log(f"Verified DefaultPawnClass={configured_pawn_class}")
