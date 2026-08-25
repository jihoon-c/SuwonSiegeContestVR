import unreal


MAIN_MAP = "/Game/Maps/Main/L_Main"
SHARED_LANDSCAPE_MAP = "/Game/Maps/Main/L_NamhansanseongLandscape"


def imported_actor_count():
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    return len([actor for actor in actors if "L_NamhansanseongLandscape" in actor.get_path_name()])


world = unreal.EditorLoadingAndSavingUtils.load_map(MAIN_MAP)
before = imported_actor_count()
unreal.SystemLibrary.execute_console_command(world, "Suwon.AttachNamhansanseongLandscape")
after = imported_actor_count()

unreal.log(
    f"SHARED_LANDSCAPE_REUSE_VERIFY target={MAIN_MAP} shared={SHARED_LANDSCAPE_MAP} "
    f"before={before} after={after}"
)
if before != 123 or after != 123:
    raise RuntimeError(
        "Shared Landscape command did not preserve the single streaming-level reference "
        f"(before={before}, after={after})"
    )

unreal.log("SHARED_LANDSCAPE_REUSE_VERIFY SUCCESS")
