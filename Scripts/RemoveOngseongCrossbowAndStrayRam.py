"""Removes the retired crossbow content and the stray ram from the Ongseong levels.

The crossbow was superseded by the chongtong, so its Blueprint, its placed instance and its
dedicated bolt pool go away. The ram placed inside the fortress is not the scenario objective -
the objective ram comes from Ram_ActorPool - and allied chongtongs wasted shots on it.

Run inside the Unreal Editor (Output Log -> Cmd -> "py <path>") or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecCmds="py <this file>;Quit" -unattended -nosplash

Idempotent: reports "already gone" when there is nothing left to remove.
Requires the editor target to be built after AOngseongCrossbowActor was deleted, so the
placed instances resolve as unknown-class actors and can be removed by label.
"""

import unreal

LEVELS = (
    "/GF_OngseongCrossbow/Maps/LV_Ongseong",
    "/GF_OngseongCrossbow/Maps/LV_Ongseong_CombatTest",
)
FEATURE_MODULE = "/Script/GF_OngseongCrossbow."
CORE_MODULE = "/Script/SuwonSiegeContestVR."
CROSSBOW_BLUEPRINT = "/GF_OngseongCrossbow/Blueprints/BP_OngseongCrossbow"

# Actor labels that belong to the retired crossbow, plus the ram that is not the objective.
CROSSBOW_LABELS = ("Ongseong_PlayerCrossbow", "BP_OngseongCrossbow", "Ongseong_BoltPool", "Bolt_ActorPool")
STRAY_RAM_LABEL = "BP_OngseongRam"

asset_lib = unreal.EditorAssetLibrary
PROBLEMS = []


def log(message):
    unreal.log("[OngseongCleanup] " + message)


def warn(message):
    PROBLEMS.append(message)
    unreal.log_warning("[OngseongCleanup] " + message)


def actor_subsystem():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def destroy(actor, reason):
    label = actor.get_actor_label()
    location = actor.get_actor_location()
    actor_subsystem().destroy_actor(actor)
    log("removed {0} at ({1:.0f},{2:.0f},{3:.0f}) - {4}".format(
        label, location.x, location.y, location.z, reason))


def remove_crossbow_actors(actors):
    """The class is gone from C++, so match on label and on the crossbow bolt pool's contents."""
    removed = 0
    for actor in list(actors):
        label = actor.get_actor_label()
        if any(label.startswith(prefix) for prefix in CROSSBOW_LABELS):
            destroy(actor, "retired crossbow")
            removed += 1
    return removed


def remove_crossbow_bolt_pool(actors):
    """Only the crossbow used a bolt pool; the archers keep their own arrow pool."""
    pool_class = unreal.load_class(None, CORE_MODULE + "ActorPool")
    if not pool_class:
        return 0
    removed = 0
    for pool in unreal.EditorFilterLibrary.by_class(actors, pool_class):
        pooled = pool.get_editor_property("pooled_actor_class")
        name = str(pooled.get_name()).lower() if pooled else ""
        if "bolt" in name and pool.get_actor_label() != "Ongseong_RangedProjectilePool":
            destroy(pool, "crossbow bolt pool")
            removed += 1
    return removed


def remove_stray_rams(actors):
    """Keeps nothing: the scenario acquires its ram from Ram_ActorPool at runtime."""
    ram_class = unreal.load_class(None, FEATURE_MODULE + "OngseongRamActor")
    if not ram_class:
        warn("OngseongRamActor class is not loaded")
        return 0
    removed = 0
    for ram in unreal.EditorFilterLibrary.by_class(actors, ram_class):
        # Pool-owned rams are spawned at runtime, so anything found in the saved level is placed.
        destroy(ram, "not the scenario objective; the objective ram comes from Ram_ActorPool")
        removed += 1
    return removed


def clean_level(level):
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem.load_level(level):
        warn("Could not load " + level)
        return
    actors = actor_subsystem().get_all_level_actors()
    removed = remove_crossbow_actors(actors)
    removed += remove_crossbow_bolt_pool(actors)
    removed += remove_stray_rams(actors)
    # Always re-save. Actors whose native class was deleted are dropped silently at load, so the
    # package still names the retired Blueprint until it is written back out.
    level_subsystem.save_current_level()
    if not asset_lib.save_asset(level, False):
        warn("save_asset({0}) failed".format(level))
    log("{0}: removed {1} actor(s), saved".format(level, removed))


def delete_crossbow_blueprint():
    if not asset_lib.does_asset_exist(CROSSBOW_BLUEPRINT):
        log("Blueprint already gone: " + CROSSBOW_BLUEPRINT)
        return
    referencers = asset_lib.find_package_referencers_for_asset(CROSSBOW_BLUEPRINT, False)
    if referencers:
        warn("{0} is still referenced by {1}; delete it manually after clearing them".format(
            CROSSBOW_BLUEPRINT, referencers))
        return
    if asset_lib.delete_asset(CROSSBOW_BLUEPRINT):
        log("deleted " + CROSSBOW_BLUEPRINT)
    else:
        warn("delete_asset failed for " + CROSSBOW_BLUEPRINT)


def main():
    for level in LEVELS:
        clean_level(level)
    delete_crossbow_blueprint()
    if PROBLEMS:
        unreal.log_warning("[OngseongCleanup] Finished with {0} item(s) needing attention:".format(len(PROBLEMS)))
        for problem in PROBLEMS:
            unreal.log_warning("[OngseongCleanup]   - " + problem)
    else:
        log("Done.")


main()

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
