import math
import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_AUTHORING_VISUAL VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_AUTHORING_VISUAL VERIFY FAIL: {message}")


blueprint = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonHwacha")
white_material = unreal.load_asset(
    "/Game/NiagaraExamples/Gallery/StaticMesh/BeachBall/M_BeachBallWhite")
check(blueprint is not None, "BP_SingijeonHwacha loads")
if blueprint:
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    cdo = unreal.get_default_object(blueprint.generated_class())
    center_arrow = cdo.get_editor_property("ammo_grid_center_arrow")
    grid_preview = cdo.get_editor_property("ammo_grid_editor_preview")
    fuse_cord = cdo.get_editor_property("fuse_cord")
    check(center_arrow is not None, "Hwacha owns the ammo-grid center ArrowComponent")
    check(grid_preview is not None, "Hwacha owns the editor-only loaded-arrow mesh preview")
    check(grid_preview is not None and grid_preview.get_editor_property("hidden_in_game"),
          "Loaded-arrow editor preview is hidden in game")
    check(grid_preview is not None and not grid_preview.get_editor_property("cast_shadow"),
          "Loaded-arrow editor preview does not cast shadows")
    check(center_arrow is not None and center_arrow.get_editor_property("hidden_in_game"),
          "Ammo-grid center arrow is editor-visible but hidden in game")
    check(fuse_cord is not None and fuse_cord.get_editor_property("static_mesh") is not None,
          "Hwacha owns the thin fuse cord mesh")
    check(fuse_cord is not None and fuse_cord.get_material(0) == white_material,
          "Fuse cord uses the white material")
    check(fuse_cord is not None and not fuse_cord.get_editor_property("cast_shadow"),
          "Fuse cord does not cast a VR-costly shadow")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
hwacha = next((actor for actor in actors
               if isinstance(actor, unreal.SingijeonHwachaActor)), None)
check(hwacha is not None, "LV_Singijeon contains the playable Hwacha")
if hwacha:
    center_arrow = hwacha.get_editor_property("ammo_grid_center_arrow")
    grid_preview = hwacha.get_editor_property("ammo_grid_editor_preview")
    slot = hwacha.get_editor_property("default_ammo_slot")
    rows = hwacha.get_editor_property("auto_fill_rows")
    columns = hwacha.get_editor_property("auto_fill_columns")
    expected = slot.get_editor_property("relative_location")
    expected += hwacha.get_editor_property("auto_fill_grid_offset")
    expected += unreal.Vector(
        0.0,
        0.5 * max(0, columns - 1) * hwacha.get_editor_property("auto_fill_column_spacing"),
        0.5 * max(0, rows - 1) * hwacha.get_editor_property("auto_fill_row_spacing"),
    )
    expected += hwacha.get_editor_property("ammo_grid_center_arrow_offset")
    actual = center_arrow.get_editor_property("relative_location")
    check(math.isclose(actual.x, expected.x, abs_tol=0.1) and
          math.isclose(actual.y, expected.y, abs_tol=0.1) and
          math.isclose(actual.z, expected.z, abs_tol=0.1),
          "Editor arrow is centered over the configured loaded-arrow grid")
    check(grid_preview is not None and grid_preview.get_instance_count() == rows * columns,
          "Editor preview shows every configured loaded arrow before PIE")
    check(grid_preview is not None and
          grid_preview.get_editor_property("static_mesh") ==
          hwacha.get_editor_property("auto_fill_arrow_mesh"),
          "Editor preview uses the same mesh as runtime auto-fill")
    fuse_cord = hwacha.get_editor_property("fuse_cord")
    scale = fuse_cord.get_editor_property("relative_scale3d")
    check(scale.x < 0.05 and scale.y < 0.05 and scale.z > 0.1,
          "Fuse cord is a long, thread-thin cylinder")

if errors:
    raise RuntimeError("Singijeon authoring visual verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_AMMO_CENTER_ARROW_FUSE_CORD VERIFY SUCCESS")
