"""Run after opening the target map in Unreal Editor, then save that map."""

import unreal


world = unreal.EditorLevelLibrary.get_editor_world()
if world is None:
    raise RuntimeError("Open the target level before attaching the shared Landscape")

unreal.SystemLibrary.execute_console_command(world, "Suwon.AttachNamhansanseongLandscape")
unreal.log("NAMHANSANSEONG_LANDSCAPE_REUSE command dispatched")

