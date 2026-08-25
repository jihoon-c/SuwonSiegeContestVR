"""Reports the state of every Ongseong interaction so the audit is evidence, not memory.

Run inside the Unreal Editor (Output Log -> Cmd -> "py <path>") or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecCmds="py <this file>;Quit" -unattended -nosplash

Read-only. Prints one line per interaction point: what drives it, and whether it has a
grab component, a visual prompt and a feedback effect.
"""

import unreal

FEATURE_BLUEPRINTS = "/GF_OngseongCrossbow/Blueprints"
LEVEL = "/GF_OngseongCrossbow/Maps/LV_Ongseong"

asset_lib = unreal.EditorAssetLibrary


def log(message):
    unreal.log("[OngseongAudit] " + message)


def load_blueprint(path):
    blueprint = asset_lib.load_asset(path)
    if not blueprint:
        log("MISSING Blueprint " + path)
    return blueprint


def default_object(path):
    blueprint = load_blueprint(path)
    if not blueprint:
        return None
    return unreal.get_default_object(blueprint.generated_class())


def component_names(path):
    """Blueprint-added components live in the construction script, not on the CDO."""
    blueprint = load_blueprint(path)
    if not blueprint:
        return []
    names = []
    cdo = unreal.get_default_object(blueprint.generated_class())
    if cdo:
        names.extend(component.get_name() for component in cdo.get_components_by_class(unreal.SceneComponent))
    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    library = unreal.SubobjectDataBlueprintFunctionLibrary
    for handle in subsystem.k2_gather_subobject_data_for_blueprint(blueprint):
        data = library.get_data(handle)
        variable_name = str(library.get_variable_name(data))
        if variable_name and variable_name not in names:
            names.append(variable_name)
    return names


def describe(value):
    if value is None:
        return "none"
    try:
        return value.get_name()
    except Exception:
        return str(value)


def report_loading_items():
    for name in ("BP_ChongtongPowder", "BP_ChongtongRammer", "BP_ChongtongCannonball"):
        path = FEATURE_BLUEPRINTS + "/" + name
        cdo = default_object(path)
        if not cdo:
            continue
        names = component_names(path)
        log("{0}: type={1} grab={2} prompt={3} components={4}".format(
            name,
            cdo.get_editor_property("item_type"),
            "GrabPoint" in names,
            "LoadingPrompt" in names,
            names))


def report_cannons():
    for name in ("BP_PlayableChongtong", "BP_ChongtongCannon", "BP_AllyChongtong"):
        path = FEATURE_BLUEPRINTS + "/" + name
        cdo = default_object(path)
        if not cdo:
            continue
        names = component_names(path)
        log("{0}: aimGrip={1} aimPrompt={2} spawnProps={3} shots={4} rams={5}".format(
            name,
            "AimGrip" in names,
            "AimPrompt" in names,
            cdo.get_editor_property("spawn_placeholder_props"),
            cdo.get_editor_property("required_shots_to_complete"),
            cdo.get_editor_property("required_rammer_strokes")))
        log("    items: powder={0} rammer={1} cannonball={2}".format(
            describe(cdo.get_editor_property("powder_item_class")),
            describe(cdo.get_editor_property("rammer_item_class")),
            describe(cdo.get_editor_property("cannonball_item_class"))))
        log("    fx: muzzle={0} load={1} sound={2} concurrency={3}".format(
            describe(cdo.get_editor_property("muzzle_effect")),
            describe(cdo.get_editor_property("load_success_effect")),
            describe(cdo.get_editor_property("fire_sound")),
            describe(cdo.get_editor_property("combat_sound_concurrency"))))


def report_crossbow():
    path = FEATURE_BLUEPRINTS + "/BP_OngseongCrossbow"
    cdo = default_object(path)
    if not cdo:
        return
    names = component_names(path)
    log("BP_OngseongCrossbow: grip={0} prompt={1} maxAmmo={2} reload={3}s".format(
        "Grip" in names,
        "GripPrompt" in names,
        cdo.get_editor_property("max_ammo"),
        cdo.get_editor_property("reload_duration")))
    log("    fx: fire={0} sound={1} concurrency={2}".format(
        describe(cdo.get_editor_property("fire_effect")),
        describe(cdo.get_editor_property("fire_sound")),
        describe(cdo.get_editor_property("fire_sound_concurrency"))))


def report_projectile():
    cdo = default_object(FEATURE_BLUEPRINTS + "/BP_ChongtongProjectile")
    if not cdo:
        return
    log("BP_ChongtongProjectile: explosion={0} sound={1} concurrency={2}".format(
        describe(cdo.get_editor_property("explosion_effect")),
        describe(cdo.get_editor_property("explosion_sound")),
        describe(cdo.get_editor_property("explosion_sound_concurrency"))))


def report_level_instances():
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(LEVEL)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    for class_path, label in (
        ("/Script/GF_OngseongCrossbow.OngseongCrossbowActor", "crossbow"),
        ("/Script/GF_OngseongCrossbow.ChongtongCannonActor", "chongtong"),
    ):
        actor_class = unreal.load_class(None, class_path)
        if not actor_class:
            continue
        for actor in unreal.EditorFilterLibrary.by_class(actors, actor_class):
            location = actor.get_actor_location()
            log("level {0}: {1} at ({2:.0f},{3:.0f},{4:.0f})".format(
                label, actor.get_actor_label(), location.x, location.y, location.z))


def main():
    log("--- loading items (grab -> insert) ---")
    report_loading_items()
    log("--- chongtong (aim + fire) ---")
    report_cannons()
    log("--- crossbow ---")
    report_crossbow()
    log("--- shell ---")
    report_projectile()
    log("--- placed instances ---")
    report_level_instances()
    log("done")


main()

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
