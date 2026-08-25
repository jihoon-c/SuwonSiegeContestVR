"""Reports why a moving enemy can sit on the first frame of its run animation.

Read-only. Run inside the Unreal Editor (Output Log -> Cmd -> "py <path>") or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecCmds="py <this file>;Quit" -unattended -nosplash

A pose that never advances while the state machine is in Run has a short list of causes:
  * the sequence carries a single key (imported as a pose, not a take)
  * the sequence is not looping, so it plays once and holds
  * RateScale is 0
  * the state graph uses a Sequence Evaluator (explicit time) instead of a Sequence Player
  * the mesh only ticks its pose when rendered, or the anim instance is not updating

This prints the evidence for each so the fix is not a guess.
"""

import unreal

FEATURE = "/GF_OngseongCrossbow"
SEQUENCES = (
    FEATURE + "/Asset/Character/Enemy/AS_MeleeRun",
    FEATURE + "/Asset/Character/Enemy/AS_RangeRun",
    FEATURE + "/Asset/Character/Enemy/AS_EnemyIdle",
    FEATURE + "/Asset/Character/Enemy/AS_Shooting",
)
ANIM_BLUEPRINTS = (
    FEATURE + "/Animation/ABP_EnemyMelee",
    FEATURE + "/Animation/ABP_EnemyArcher",
)
CHARACTERS = (
    FEATURE + "/Blueprints/BP_EnemySword",
    FEATURE + "/Blueprints/BP_EnemyArcher",
)

asset_lib = unreal.EditorAssetLibrary


def log(message):
    unreal.log("[AnimDiag] " + message)


def prop(target, name, default="<no such property>"):
    try:
        return target.get_editor_property(name)
    except Exception:
        return default


def report_sequences():
    for path in SEQUENCES:
        sequence = asset_lib.load_asset(path)
        if not sequence:
            log("MISSING " + path)
            continue
        name = path.rsplit("/", 1)[1]
        length = prop(sequence, "sequence_length")
        keys = prop(sequence, "number_of_sampled_keys")
        frames = prop(sequence, "number_of_sampled_frames")
        rate = prop(sequence, "rate_scale")
        loop = prop(sequence, "loop")
        root_motion = prop(sequence, "enable_root_motion")
        log("{0}: length={1} keys={2} frames={3} rateScale={4} loop={5} rootMotion={6}".format(
            name, length, keys, frames, rate, loop, root_motion))
        # A single-key take is a static pose no matter what the state machine does.
        try:
            if float(length) <= 0.05 or int(keys) <= 1:
                log("    ^ SUSPECT: this asset holds a single pose, not a run cycle")
        except Exception:
            pass
        try:
            if float(rate) == 0.0:
                log("    ^ SUSPECT: RateScale is 0, the pose cannot advance")
        except Exception:
            pass
        if loop is False:
            log("    ^ SUSPECT: not looping; it plays once and holds its last frame")


def report_anim_graph_nodes(path):
    """Sequence Evaluators show one explicit frame; Sequence Players advance with time."""
    blueprint = asset_lib.load_asset(path)
    if not blueprint:
        log("MISSING " + path)
        return
    name = path.rsplit("/", 1)[1]
    found_any = False
    try:
        graphs = unreal.BlueprintEditorLibrary.get_all_graphs(blueprint)
    except Exception as error:
        log("{0}: cannot enumerate graphs from Python ({1}).".format(name, error))
        log("    Open the ABP and check the Run state by hand: it must contain a")
        log("    'Play <sequence>' node (Sequence Player) with Loop Animation = true,")
        log("    not a 'Sequence Evaluator'.")
        return
    for graph in graphs:
        graph_name = str(graph.get_name())
        for node in unreal.BlueprintEditorLibrary.get_all_nodes_in_graph(graph):
            node_name = str(node.get_class().get_name())
            if "SequenceEvaluator" in node_name or "SequencePlayer" in node_name:
                found_any = True
                log("{0} :: {1}: {2}".format(name, graph_name, node_name))
                if "SequenceEvaluator" in node_name:
                    log("    ^ SUSPECT: an evaluator holds one explicit time; use a Sequence Player")
    if not found_any:
        log("{0}: no sequence player/evaluator nodes reachable from Python.".format(name))


def report_meshes():
    for path in CHARACTERS:
        blueprint = asset_lib.load_asset(path)
        if not blueprint:
            log("MISSING " + path)
            continue
        name = path.rsplit("/", 1)[1]
        cdo = unreal.get_default_object(blueprint.generated_class())
        mesh = None
        for component in cdo.get_components_by_class(unreal.SkeletalMeshComponent):
            mesh = component
            break
        if not mesh:
            log(name + ": no SkeletalMeshComponent on the CDO")
            continue
        log("{0}: animMode={1} animClass={2} visibilityTick={3} paused={4} rate={5}".format(
            name,
            prop(mesh, "animation_mode"),
            prop(mesh, "anim_class"),
            prop(mesh, "visibility_based_anim_tick_option"),
            prop(mesh, "pause_anims", "<runtime only>"),
            prop(mesh, "global_anim_rate_scale")))
        option = str(prop(mesh, "visibility_based_anim_tick_option"))
        if "ONLY_TICK_POSE_WHEN_RENDERED" in option.upper():
            log("    ^ SUSPECT: pose only ticks when rendered; pooled/off-screen actors freeze")


def main():
    log("--- animation sequences ---")
    report_sequences()
    log("--- anim blueprint graphs ---")
    for path in ANIM_BLUEPRINTS:
        report_anim_graph_nodes(path)
    log("--- character meshes ---")
    report_meshes()
    log("done")


main()

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
