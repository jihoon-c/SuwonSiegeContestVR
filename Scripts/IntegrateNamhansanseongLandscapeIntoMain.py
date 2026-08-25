import unreal


SOURCE_MAP = "/Game/Namhansanseong/Maps/Demo_Namhansanseong"
MAIN_MAP = "/Game/Maps/Main/L_Main"
LANDSCAPE_MAP = "/Game/Maps/Main/L_NamhansanseongLandscape"


actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def all_actors():
    return actor_subsystem.get_all_level_actors()


def is_landscape(actor):
    return actor.get_class().get_name() in ("Landscape", "LandscapeStreamingProxy")


# Preserve the Demo's known playable terrain anchor instead of leaving Main at the
# old flat test plane's origin, which would place the player below the mountain.
unreal.EditorLoadingAndSavingUtils.load_map(SOURCE_MAP)
source_starts = [actor for actor in all_actors() if isinstance(actor, unreal.PlayerStart)]
if len(source_starts) != 1:
    raise RuntimeError(f"Expected one Demo PlayerStart, found {len(source_starts)}")
source_start_location = source_starts[0].get_actor_location()
source_start_rotation = source_starts[0].get_actor_rotation()

world = unreal.EditorLoadingAndSavingUtils.load_map(MAIN_MAP)
main_actors = all_actors()
existing_imported = [
    actor for actor in main_actors if "L_NamhansanseongLandscape" in actor.get_path_name()
]

main_landscapes = [
    actor
    for actor in main_actors
    if is_landscape(actor) and "L_Main.L_Main:PersistentLevel" in actor.get_path_name()
]
if len(main_landscapes) > 1:
    raise RuntimeError(f"Expected at most one old Main Landscape, found {len(main_landscapes)}")
if main_landscapes:
    if not actor_subsystem.destroy_actor(main_landscapes[0]):
        raise RuntimeError("Could not remove the old flat Main Landscape")

if not existing_imported:
    streaming = unreal.EditorLevelUtils.add_level_to_world(
        world,
        LANDSCAPE_MAP,
        unreal.LevelStreamingAlwaysLoaded,
    )
    if not streaming:
        raise RuntimeError("Could not add the Namhansanseong Landscape sublevel")
    unreal.log(f"MAIN_LANDSCAPE_TRANSFER streaming={streaming.get_path_name()}")

main_actors = all_actors()
main_starts = [
    actor
    for actor in main_actors
    if isinstance(actor, unreal.PlayerStart) and "L_Main.L_Main:PersistentLevel" in actor.get_path_name()
]
if len(main_starts) != 1:
    raise RuntimeError(f"Expected one Main PlayerStart, found {len(main_starts)}")

main_start = main_starts[0]
old_start_location = main_start.get_actor_location()
anchor_delta = source_start_location - old_start_location
main_start.set_actor_location(source_start_location, False, False)
main_start.set_actor_rotation(source_start_rotation, False)

travel_triggers = [
    actor
    for actor in main_actors
    if isinstance(actor, unreal.ExperienceTravelTriggerActor)
    and "L_Main.L_Main:PersistentLevel" in actor.get_path_name()
]
for trigger in travel_triggers:
    trigger.set_actor_location(trigger.get_actor_location() + anchor_delta, False, False)

if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, False):
    raise RuntimeError("Could not save L_Main after Landscape integration")

unreal.EditorLoadingAndSavingUtils.load_map(MAIN_MAP)
verified_actors = all_actors()
verified_landscapes = [actor for actor in verified_actors if is_landscape(actor)]
verified_managers = [
    actor for actor in verified_actors if actor.get_class().get_name() == "MainEducationScenarioManagerActor"
]
verified_starts = [actor for actor in verified_actors if isinstance(actor, unreal.PlayerStart)]
verified_triggers = [
    actor for actor in verified_actors if isinstance(actor, unreal.ExperienceTravelTriggerActor)
]
materials = {
    str(actor.get_editor_property("landscape_material"))
    for actor in verified_landscapes
    if actor.get_editor_property("landscape_material")
}

unreal.log(
    f"MAIN_LANDSCAPE_TRANSFER verify actors={len(verified_actors)} "
    f"landscapes={len(verified_landscapes)} managers={len(verified_managers)} "
    f"player_starts={len(verified_starts)} triggers={len(verified_triggers)} "
    f"materials={sorted(materials)}"
)

errors = []
if len(verified_landscapes) != 122:
    errors.append(f"expected 122 imported Landscape actors, found {len(verified_landscapes)}")
if len(verified_managers) != 1:
    errors.append(f"expected one Main education manager, found {len(verified_managers)}")
if len(verified_starts) != 1:
    errors.append(f"expected one PlayerStart, found {len(verified_starts)}")
if len(verified_triggers) != 2:
    errors.append(f"expected two travel triggers, found {len(verified_triggers)}")
if not any("MI_Landscape" in material for material in materials):
    errors.append("MI_Landscape reference was not preserved")
if errors:
    raise RuntimeError("Main Landscape transfer verification failed: " + "; ".join(errors))

unreal.log(
    f"MAIN_LANDSCAPE_TRANSFER anchor={source_start_location} rotation={source_start_rotation} SUCCESS"
)
