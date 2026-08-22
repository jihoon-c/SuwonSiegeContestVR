import unreal


FOLDER = "/GF_OngseongCrossbow/Blueprints"
LEVEL_PATH = "/GF_OngseongCrossbow/Maps/LV_Ongseong"


def main():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.scan_paths_synchronous(["/GF_OngseongCrossbow"], True)
    redirectors = [
        asset.package_name
        for asset in registry.get_assets_by_path("/GF_OngseongCrossbow", recursive=True)
        if str(asset.asset_class_path.asset_name) == "ObjectRedirector"
    ]
    if redirectors:
        raise RuntimeError(f"Unfixed Ongseong redirectors: {redirectors}")
    bolt = unreal.EditorAssetLibrary.load_asset(f"{FOLDER}/BP_OngseongBolt")
    crossbow = unreal.EditorAssetLibrary.load_asset(f"{FOLDER}/BP_OngseongCrossbow")
    if not bolt or not crossbow:
        raise RuntimeError("Crossbow or bolt Blueprint is missing")
    crossbow_default = unreal.get_default_object(crossbow.generated_class())
    if crossbow_default.get_editor_property("bolt_class") != bolt.generated_class():
        raise RuntimeError("BP_OngseongCrossbow.BoltClass is not BP_OngseongBolt")

    unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    by_label = {actor.get_actor_label(): actor for actor in actors}
    wave = by_label.get("Ongseong_WaveManager")
    cannon = by_label.get("Chongtong_Gameplay") or next(
        (actor for actor in actors if "ChongtongCannon" in actor.get_class().get_name()), None
    )
    pool = by_label.get("Ongseong_RangedProjectilePool")
    placed_crossbow = by_label.get("Ongseong_PlayerCrossbow")
    if not all((wave, cannon, pool, placed_crossbow)):
        raise RuntimeError("Ranged combat actors are not fully placed")
    if unreal.Name("Ongseong.ArcherTarget") not in cannon.tags:
        raise RuntimeError("Cannon is missing the archer target tag")
    if wave.get_editor_property("archer_primary_target") != cannon:
        raise RuntimeError("Wave manager archer target is not the cannon")
    if wave.get_editor_property("archer_projectile_pool") != pool:
        raise RuntimeError("Wave manager archer projectile pool is not linked")
    if placed_crossbow.get_editor_property("bolt_pool") != pool:
        raise RuntimeError("Placed crossbow projectile pool is not linked")
    unreal.log("ONGSEONG_RANGED_COMBAT_ASSETS_VERIFIED")


if __name__ == "__main__":
    main()
