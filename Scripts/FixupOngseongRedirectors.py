import unreal


ROOT = "/GF_OngseongCrossbow"


def main():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.scan_paths_synchronous([ROOT], True)
    redirectors = []
    seen_paths = set()
    for asset in registry.get_assets_by_path(ROOT, recursive=True):
        if str(asset.asset_class_path.asset_name) != "ObjectRedirector":
            continue
        if str(asset.asset_name) != "BP_OngseongEnemy":
            continue
        object_path = f"{asset.package_name}.{asset.asset_name}"
        if object_path in seen_paths:
            continue
        seen_paths.add(object_path)
        redirector = asset.get_asset()
        if redirector:
            redirectors.append(redirector)

    if redirectors:
        referencers = unreal.EditorAssetLibrary.find_package_referencers_for_asset(
            f"{ROOT}/Blueprints/BP_OngseongEnemy", True
        )
        if referencers:
            raise RuntimeError(f"Redirector still has referencers: {referencers}")
        if not unreal.EditorAssetLibrary.delete_asset(
            f"{ROOT}/Blueprints/BP_OngseongEnemy"
        ):
            raise RuntimeError("Failed to delete unreferenced Ongseong redirector")
    unreal.log(f"ONGSEONG_REDIRECTORS_FIXED={len(redirectors)}")


if __name__ == "__main__":
    main()
