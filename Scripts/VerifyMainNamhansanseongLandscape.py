import unreal


MAIN_MAP = "/Game/Maps/Main/L_Main"
EXPECTED_MATERIAL = "/Game/Namhansanseong/Materials/Landscape/MI_Landscape"


world = unreal.EditorLoadingAndSavingUtils.load_map(MAIN_MAP)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
landscapes = [
    actor
    for actor in actors
    if actor.get_class().get_name() in ("Landscape", "LandscapeStreamingProxy")
]
managers = [
    actor for actor in actors if actor.get_class().get_name() == "MainEducationScenarioManagerActor"
]
starts = [actor for actor in actors if isinstance(actor, unreal.PlayerStart)]
triggers = [actor for actor in actors if isinstance(actor, unreal.ExperienceTravelTriggerActor)]
imported_paths = [
    actor.get_path_name() for actor in actors if "L_NamhansanseongLandscape" in actor.get_path_name()
]
materials = {
    str(actor.get_editor_property("landscape_material"))
    for actor in landscapes
    if actor.get_editor_property("landscape_material")
}

errors = []
if len(actors) != 129:
    errors.append(f"expected 129 loaded actors, found {len(actors)}")
if len(imported_paths) != 123:
    errors.append(f"expected 123 actors in Landscape sublevel, found {len(imported_paths)}")
if len(landscapes) != 122:
    errors.append(f"expected 122 Landscape actors, found {len(landscapes)}")
if any("L_Main.L_Main:PersistentLevel.Landscape" in actor.get_path_name() for actor in landscapes):
    errors.append("old flat persistent Landscape still exists")
if len(managers) != 1:
    errors.append(f"expected one education manager, found {len(managers)}")
if len(starts) != 1:
    errors.append(f"expected one PlayerStart, found {len(starts)}")
if len(triggers) != 2:
    errors.append(f"expected two travel triggers, found {len(triggers)}")
if not any(EXPECTED_MATERIAL in material for material in materials):
    errors.append("MI_Landscape is not assigned")

if starts:
    start_location = starts[0].get_actor_location()
    if abs(start_location.x + 23078.779835) > 1.0 or abs(start_location.y - 8513.437204) > 1.0:
        errors.append(f"PlayerStart is not aligned to the Demo terrain anchor: {start_location}")
else:
    start_location = unreal.Vector()

unreal.log(
    f"MAIN_LANDSCAPE_VERIFY actors={len(actors)} imported={len(imported_paths)} "
    f"landscapes={len(landscapes)} managers={len(managers)} starts={len(starts)} "
    f"triggers={len(triggers)} start={start_location} materials={sorted(materials)}"
)

if errors:
    for error in errors:
        unreal.log_error(f"MAIN_LANDSCAPE_VERIFY FAIL: {error}")
    raise RuntimeError("Main Namhansanseong Landscape verification failed")

unreal.log("MAIN_LANDSCAPE_VERIFY SUCCESS")
