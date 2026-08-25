import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"GUIDE_ANCHOR VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"GUIDE_ANCHOR VERIFY FAIL: {message}")


unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
interactors = []
preview_count = 0
for actor in actors:
    actor_interactors = actor.get_components_by_class(unreal.ScenarioInteractableComponent)
    interactors.extend(actor_interactors)
    if actor_interactors:
        arrows = actor.get_components_by_class(unreal.ArrowComponent)
        preview_count += min(len(actor_interactors), len(arrows))
        for marker in arrows:
            check(marker.get_editor_property("hidden_in_game"),
                  "Editor guide anchor is hidden during gameplay")

check(len(interactors) > 0, "LV_Singijeon contains Scenario Interactors")
check(preview_count == len(interactors),
      "Every Scenario Interactor owns an editor guide-anchor preview")
check(all(interactor.get_editor_property("guide_anchor_offset").z <= 10.0
          for interactor in interactors),
      "Default guide anchors are reduced to 10cm above actor bounds")

if errors:
    raise RuntimeError("Scenario guide editor-anchor verification failed: " +
                       "; ".join(errors))
unreal.log("SCENARIO_GUIDE_EDITOR_ANCHOR VERIFY SUCCESS")
