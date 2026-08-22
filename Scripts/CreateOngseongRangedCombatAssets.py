import unreal


FEATURE_FOLDER = "/GF_OngseongCrossbow/Blueprints"
LEVEL_PATH = "/GF_OngseongCrossbow/Maps/LV_Ongseong"
MODULE = "/Script/GF_OngseongCrossbow."


def get_or_create_blueprint(name, parent_name):
    path = f"{FEATURE_FOLDER}/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if existing:
        return existing
    parent = unreal.load_class(None, MODULE + parent_name)
    if not parent:
        raise RuntimeError(f"{parent_name} is not loaded. Build and restart the editor first.")
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, FEATURE_FOLDER, unreal.Blueprint, factory)
    if not asset:
        raise RuntimeError(f"Failed to create {name}")
    return asset


def add_tag(actor, tag):
    tags = list(actor.tags)
    tag_name = unreal.Name(tag)
    if tag_name not in tags:
        tags.append(tag_name)
        actor.tags = tags


def main():
    unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([FEATURE_FOLDER], True)
    bolt_bp = get_or_create_blueprint("BP_OngseongBolt", "OngseongBoltProjectileActor")
    crossbow_bp = get_or_create_blueprint("BP_OngseongCrossbow", "OngseongCrossbowActor")
    for blueprint in (bolt_bp, crossbow_bp):
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)

    crossbow_default = unreal.get_default_object(crossbow_bp.generated_class())
    crossbow_default.set_editor_property("bolt_class", bolt_bp.generated_class())
    unreal.BlueprintEditorLibrary.compile_blueprint(crossbow_bp)
    for blueprint in (bolt_bp, crossbow_bp):
        unreal.EditorAssetLibrary.save_loaded_asset(blueprint)

    unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    by_label = {actor.get_actor_label(): actor for actor in actors}
    wave = by_label.get("Ongseong_WaveManager")
    cannon = by_label.get("Chongtong_Gameplay") or next(
        (actor for actor in actors if "ChongtongCannon" in actor.get_class().get_name()), None
    )
    if not wave or not cannon:
        raise RuntimeError("LV_Ongseong is missing Ongseong_WaveManager or the Chongtong cannon")

    add_tag(cannon, "Ongseong.ArcherTarget")

    pool = by_label.get("Ongseong_RangedProjectilePool")
    if not pool:
        pool_class = unreal.load_class(None, "/Script/SuwonSiegeContestVR.ActorPool")
        pool = unreal.EditorLevelLibrary.spawn_actor_from_class(pool_class, wave.get_actor_location())
        pool.set_actor_label("Ongseong_RangedProjectilePool")
    pool.set_editor_property("pooled_actor_class", bolt_bp.generated_class())
    pool.set_editor_property("initial_pool_size", 24)
    pool.set_editor_property("allow_pool_expansion", False)
    add_tag(pool, "Ongseong.ArrowPool")

    crossbow = by_label.get("Ongseong_PlayerCrossbow")
    if not crossbow:
        location = cannon.get_actor_location() + cannon.get_actor_right_vector() * 350.0
        crossbow = unreal.EditorLevelLibrary.spawn_actor_from_class(
            crossbow_bp.generated_class(), location, cannon.get_actor_rotation()
        )
        if not crossbow:
            raise RuntimeError("Failed to place BP_OngseongCrossbow in LV_Ongseong")
        crossbow.set_actor_label("Ongseong_PlayerCrossbow")
    crossbow.set_editor_property("bolt_pool", pool)

    wave.set_editor_property("archer_primary_target", cannon)
    wave.set_editor_property("archer_projectile_pool", pool)

    for blueprint in (bolt_bp, crossbow_bp):
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("ONGSEONG_RANGED_COMBAT_ASSETS_CREATED_AND_LINKED")


if __name__ == "__main__":
    main()
