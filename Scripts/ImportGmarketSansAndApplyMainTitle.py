"""Import Gmarket Sans fonts and assign them to the placed Main intro without moving it."""

import os

import unreal


FONT_SOURCE_DIRECTORY = os.path.join(unreal.Paths.project_dir(), "GmarketSansTTF")
FONT_DESTINATION = "/Game/UI/Fonts"
FONT_FACE_ASSET_NAME = "GmarketSansBold"
FONT_ASSET_NAME = "GmarketSansBold_Font"
FONT_FILENAME = "GmarketSansTTFBold.ttf"
MAIN_LEVEL_PATH = "/Game/Maps/Main/L_Main"


def import_or_load_font(filename):
    font_path = f"{FONT_DESTINATION}/{FONT_ASSET_NAME}"
    font = unreal.load_asset(font_path)
    if not font:
        font_face_path = f"{FONT_DESTINATION}/{FONT_FACE_ASSET_NAME}"
        if not unreal.load_asset(font_face_path):
            source_path = os.path.join(FONT_SOURCE_DIRECTORY, filename)
            if not os.path.isfile(source_path):
                raise RuntimeError(f"Missing font file: {source_path}")
            task = unreal.AssetImportTask()
            task.set_editor_property("filename", source_path)
            task.set_editor_property("destination_path", FONT_DESTINATION)
            task.set_editor_property("destination_name", FONT_FACE_ASSET_NAME)
            task.set_editor_property("automated", True)
            task.set_editor_property("replace_existing", False)
            task.set_editor_property("save", True)
            unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        unreal.SystemLibrary.execute_console_command(
            unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),
            "Suwon.CreateGmarketSansTitleFont",
        )
        font = unreal.load_asset(font_path)
    if not isinstance(font, unreal.Font):
        raise RuntimeError(f"Runtime Font import failed: {font_path}")
    return font


unreal.EditorAssetLibrary.make_directory(FONT_DESTINATION)
title_font = import_or_load_font(FONT_FILENAME)

world = unreal.EditorLoadingAndSavingUtils.load_map(MAIN_LEVEL_PATH)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
intros = [actor for actor in actors if isinstance(actor, unreal.MainLevelIntroActor)]
if len(intros) != 1:
    raise RuntimeError(f"Expected exactly one MainLevelIntroActor, got {len(intros)}")

intro = intros[0]
location_before = intro.get_actor_location()
rotation_before = intro.get_actor_rotation()
scale_before = intro.get_actor_scale3d()
intro.set_editor_property("title_font", title_font)
intro.set_editor_property("subtitle_font", title_font)
intro.set_editor_property("show_editor_title_preview", True)

location_after = intro.get_actor_location()
rotation_after = intro.get_actor_rotation()
scale_after = intro.get_actor_scale3d()
rotation_delta = unreal.MathLibrary.normalized_delta_rotator(rotation_after, rotation_before)
if (unreal.MathLibrary.vector_distance(location_before, location_after) > 0.01 or
        unreal.MathLibrary.vector_distance(scale_before, scale_after) > 0.0001 or
        abs(rotation_delta.pitch) > 0.01 or abs(rotation_delta.yaw) > 0.01 or
        abs(rotation_delta.roll) > 0.01):
    raise RuntimeError("Refusing to save: Main intro Actor transform changed")

if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True):
    raise RuntimeError("Could not save imported fonts and L_Main")

unreal.log(
    "GMARKET_SANS_MAIN_TITLE SUCCESS: Gmarket Sans Bold assigned to title and subtitle; "
    f"intro_location={location_after}"
)
