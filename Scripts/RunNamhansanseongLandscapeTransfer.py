import unreal


world = unreal.EditorLevelLibrary.get_editor_world()
if world is None:
    raise RuntimeError("Editor world is unavailable")

unreal.SystemLibrary.execute_console_command(world, "Suwon.TransferNamhansanseongLandscape")
unreal.log("NAMHANSANSEONG_LANDSCAPE_TRANSFER command dispatched")
unreal.SystemLibrary.quit_editor()
