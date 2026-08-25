"""Makes the enemy locomotion animations loop.

Diagnosis (Scripts/DiagnoseEnemyLocomotionAnimation.py): every imported enemy sequence has
bLoop = false. That matters twice over:

  * UAnimationGraphSchema::SpawnNodeFromAsset calls CopySettingsFromAnimationAsset when an
    animation is dragged into an AnimGraph, and UAnimGraphNode_SequencePlayer copies the
    asset's bLoop into the node's Loop Animation. So the Run state was built with looping OFF.
  * A non-looping run cycle plays once (AS_MeleeRun is 0.73 s) and then holds its final frame.
    On a seamless cycle the final frame looks like the first one, which is exactly the reported
    "stuck in the first pose while running".

This fixes the asset flag AND the already-created Sequence Player nodes, because changing the
asset does not retroactively update nodes that were created earlier.

Run inside the Unreal Editor (Output Log -> Cmd -> "py <path>") or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecCmds="py <this file>;Quit" -unattended -nosplash
"""

import unreal

FEATURE = "/GF_OngseongCrossbow"

# Cycles that must loop. AS_Shooting is a one-shot attack and is deliberately left alone.
LOOPING_SEQUENCES = (
    FEATURE + "/Asset/Character/Enemy/AS_MeleeRun",
    FEATURE + "/Asset/Character/Enemy/AS_RangeRun",
    FEATURE + "/Asset/Character/Enemy/AS_EnemyIdle",
    FEATURE + "/Asset/Character/Instructor/AS_AllyIdle",
)
ONE_SHOT_SEQUENCES = (
    FEATURE + "/Asset/Character/Enemy/AS_Shooting",
)
ANIM_BLUEPRINTS = (
    FEATURE + "/Animation/ABP_EnemyMelee",
    FEATURE + "/Animation/ABP_EnemyArcher",
    FEATURE + "/Animation/ABP_Ally",
)

asset_lib = unreal.EditorAssetLibrary
PROBLEMS = []


def log(message):
    unreal.log("[AnimLoopFix] " + message)


def warn(message):
    PROBLEMS.append(message)
    unreal.log_warning("[AnimLoopFix] " + message)


def fix_sequences():
    for path in LOOPING_SEQUENCES:
        sequence = asset_lib.load_asset(path)
        if not sequence:
            warn("missing " + path)
            continue
        name = path.rsplit("/", 1)[1]
        if sequence.get_editor_property("loop"):
            log(name + ": already loops")
            continue
        sequence.set_editor_property("loop", True)
        asset_lib.save_loaded_asset(sequence)
        log(name + ": loop = true")
    for path in ONE_SHOT_SEQUENCES:
        sequence = asset_lib.load_asset(path)
        if sequence:
            log("{0}: left one-shot (loop={1})".format(
                path.rsplit("/", 1)[1], sequence.get_editor_property("loop")))


def graphs_of(blueprint):
    """AnimBlueprint graphs are not exposed by a single accessor; try every container."""
    found = []
    for property_name in ("function_graphs", "ubergraph_pages", "delegate_signature_graphs", "macro_graphs"):
        try:
            graphs = blueprint.get_editor_property(property_name)
        except Exception:
            continue
        if graphs:
            found.extend(graphs)
    return found


def collect_nodes(graph, depth=0):
    """State machines nest their states in sub-graphs, so walk down through them."""
    nodes = []
    try:
        graph_nodes = graph.get_editor_property("nodes")
    except Exception:
        return nodes
    for node in graph_nodes or []:
        nodes.append(node)
        if depth < 6:
            for sub_property in ("bound_graph", "sub_graph", "graphs", "sub_graphs"):
                try:
                    child = node.get_editor_property(sub_property)
                except Exception:
                    continue
                if not child:
                    continue
                children = child if isinstance(child, (list, tuple)) else [child]
                for sub_graph in children:
                    nodes.extend(collect_nodes(sub_graph, depth + 1))
    return nodes


def fix_anim_blueprints():
    for path in ANIM_BLUEPRINTS:
        blueprint = asset_lib.load_asset(path)
        if not blueprint:
            warn("missing " + path)
            continue
        name = path.rsplit("/", 1)[1]
        graphs = graphs_of(blueprint)
        if not graphs:
            warn(name + ": no graphs reachable from Python; set Loop Animation by hand")
            continue

        players = 0
        changed = 0
        for graph in graphs:
            for node in collect_nodes(graph):
                class_name = str(node.get_class().get_name())
                if "SequencePlayer" not in class_name and "SequenceEvaluator" not in class_name:
                    continue
                players += 1
                if "SequenceEvaluator" in class_name:
                    warn("{0}: a Sequence Evaluator holds one explicit frame; replace it with a player".format(name))
                    continue
                try:
                    anim_node = node.get_editor_property("node")
                    sequence = anim_node.get_editor_property("sequence")
                    if sequence and str(sequence.get_name()) in ("AS_Shooting",):
                        continue
                    if anim_node.get_editor_property("loop_animation"):
                        continue
                    anim_node.set_editor_property("loop_animation", True)
                    node.set_editor_property("node", anim_node)
                    changed += 1
                    log("{0}: {1} -> Loop Animation = true".format(name, sequence.get_name() if sequence else "?"))
                except Exception as error:
                    warn("{0}: could not set Loop Animation ({1}); do it by hand".format(name, error))
        if changed:
            unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
            asset_lib.save_loaded_asset(blueprint)
        log("{0}: {1} asset player node(s), {2} changed".format(name, players, changed))


def main():
    fix_sequences()
    fix_anim_blueprints()
    if PROBLEMS:
        unreal.log_warning("[AnimLoopFix] Finished with {0} item(s) needing attention:".format(len(PROBLEMS)))
        for problem in PROBLEMS:
            unreal.log_warning("[AnimLoopFix]   - " + problem)
    else:
        log("Done.")


main()

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
