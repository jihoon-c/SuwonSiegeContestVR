import unreal


TARGET_MAP = "/GF_Singijeon/Maps/LV_Singijeon"
EXPECTED_GAMEPLAY_TOKENS = (
    "ScenarioManager",
    "SingijeonHwacha",
    "SingijeonTorch",
    "SingijeonFirePit",
    "SingijeonEnemyWave",
    "PlayerStart",
)


world = unreal.EditorLoadingAndSavingUtils.load_map(TARGET_MAP)
if world is None:
    raise RuntimeError(f"Could not load target map: {TARGET_MAP}")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
landscapes = [actor for actor in actors if actor.get_class().get_name() == "Landscape"]
if len(landscapes) != 1:
    raise RuntimeError(f"Expected one Landscape, got {len(landscapes)}")

landscape = landscapes[0]
origin, extent = landscape.get_actor_bounds(False)
material = landscape.get_editor_property("landscape_material")
max_lod = landscape.get_editor_property("max_lod_level")
tags = [str(tag) for tag in landscape.tags]
unreal.log(
    f"LANDSCAPE_TRANSFER_VERIFY landscape={landscape.get_actor_label()} "
    f"location={landscape.get_actor_location()} scale={landscape.get_actor_scale3d()} "
    f"bounds_origin={origin} bounds_extent={extent} material={material} "
    f"max_lod={max_lod} tags={tags}"
)

if extent.z <= 100.0:
    raise RuntimeError(f"Landscape is still flat: extent.z={extent.z}")
if material is None or "MI_Landscape" not in material.get_path_name():
    raise RuntimeError(f"Unexpected Landscape material: {material}")
if "NamhansanseongTerrain" not in tags:
    raise RuntimeError(f"Landscape transfer tag missing: {tags}")

missing = []
for token in EXPECTED_GAMEPLAY_TOKENS:
    matches = [actor for actor in actors if token in actor.get_class().get_name() or token in actor.get_name()]
    if not matches:
        missing.append(token)
    else:
        unreal.log(
            f"LANDSCAPE_TRANSFER_VERIFY gameplay={token} "
            f"count={len(matches)} sample={matches[0].get_name()} location={matches[0].get_actor_location()}"
        )

if missing:
    raise RuntimeError(f"Missing gameplay actors: {missing}")

unreal.log("LANDSCAPE_TRANSFER_VERIFY SUCCESS")
