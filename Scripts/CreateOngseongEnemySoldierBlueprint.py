import unreal


SHARED_ENEMY_PATH = "/Game/Gameplay/Characters/BP_EnemySoldier"
FEATURE_FOLDER = "/GF_OngseongCrossbow/Blueprints"
FEATURE_ENEMY_NAME = "BP_OngseongEnemySoldier"
FEATURE_ENEMY_PATH = f"{FEATURE_FOLDER}/{FEATURE_ENEMY_NAME}"
MAP_PATH = "/GF_OngseongCrossbow/Maps/LV_Ongseong"
POOL_LABEL = "Enemy_ActorPool"


def create_or_load_feature_enemy(parent_class):
    existing = unreal.EditorAssetLibrary.load_asset(FEATURE_ENEMY_PATH)
    if existing:
        return existing

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    return unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        FEATURE_ENEMY_NAME,
        FEATURE_FOLDER,
        unreal.Blueprint,
        factory,
    )


def main():
    shared_enemy = unreal.EditorAssetLibrary.load_asset(SHARED_ENEMY_PATH)
    if not shared_enemy or not shared_enemy.generated_class():
        raise RuntimeError("Shared BP_EnemySoldier is missing or invalid")

    feature_enemy = create_or_load_feature_enemy(shared_enemy.generated_class())
    if not feature_enemy:
        raise RuntimeError("Failed to create BP_OngseongEnemySoldier")

    unreal.BlueprintEditorLibrary.compile_blueprint(feature_enemy)
    unreal.EditorAssetLibrary.save_loaded_asset(feature_enemy)

    unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    enemy_pool = next(
        (actor for actor in actors if actor.get_actor_label() == POOL_LABEL),
        None,
    )
    if not enemy_pool:
        raise RuntimeError(f"{MAP_PATH} does not contain {POOL_LABEL}")

    enemy_pool.set_editor_property("pooled_actor_class", feature_enemy.generated_class())
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.save_current_level():
        raise RuntimeError(f"Failed to save {MAP_PATH}")

    if enemy_pool.get_editor_property("pooled_actor_class") != feature_enemy.generated_class():
        raise RuntimeError("Enemy_ActorPool did not retain BP_OngseongEnemySoldier")

    unreal.log("ONGSEONG_ENEMY_SOLDIER_CREATED_AND_LINKED")


if __name__ == "__main__":
    main()
