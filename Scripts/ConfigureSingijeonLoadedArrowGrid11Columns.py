import unreal


ROWS = 6
COLUMNS = 11
CAPACITY = ROWS * COLUMNS
HWACHA_BP_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonHwacha"
LEVEL_PATH = "/Game/Maps/LV_Singijeon"


hwacha_bp = unreal.load_asset(HWACHA_BP_PATH)
if not hwacha_bp:
    raise RuntimeError("BP_SingijeonHwacha is missing")

hwacha_cdo = unreal.get_default_object(hwacha_bp.generated_class())
hwacha_cdo.set_editor_property("auto_fill_rows", ROWS)
hwacha_cdo.set_editor_property("auto_fill_columns", COLUMNS)
hwacha_cdo.set_editor_property("minimum_loaded_ammunition", CAPACITY)
unreal.BlueprintEditorLibrary.compile_blueprint(hwacha_bp)
unreal.EditorAssetLibrary.save_loaded_asset(hwacha_bp, only_if_is_dirty=False)

unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
hwacha = next(
    (actor for actor in actors if isinstance(actor, unreal.SingijeonHwachaActor)),
    None,
)
if not hwacha:
    raise RuntimeError("LV_Singijeon does not contain a playable Hwacha")

hwacha.set_editor_property("auto_fill_rows", ROWS)
hwacha.set_editor_property("auto_fill_columns", COLUMNS)
hwacha.set_editor_property("minimum_loaded_ammunition", CAPACITY)

# Native-component attachment changes can preserve an old world-space offset on an
# already placed Blueprint actor.  Normalize the placed ISM so loaded arrows stay
# rigidly anchored to RackRoot while the Hwacha is carried.
arrow_instances = next(
    (
        component
        for component in hwacha.get_components_by_class(unreal.SceneComponent)
        if component.get_name() == "AutoLoadedArrowInstances"
    ),
    None,
)
if not arrow_instances:
    raise RuntimeError("Placed Hwacha is missing AutoLoadedArrowInstances")

arrow_instances.set_editor_property("absolute_location", False)
arrow_instances.set_editor_property("absolute_rotation", False)
arrow_instances.set_editor_property("absolute_scale", False)
arrow_instances.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 0.0))
arrow_instances.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, 0.0))
arrow_instances.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

unreal.log(
    f"SINGIJEON_LOADED_ARROW_GRID CONFIGURE SUCCESS: "
    f"rows={ROWS} columns={COLUMNS} capacity={CAPACITY}"
)
