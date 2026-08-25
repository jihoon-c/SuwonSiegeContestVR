import unreal


BLUEPRINT_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonBox"
EXPECTED_COUNT = 42
errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_BOX_VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_BOX_VERIFY FAIL: {message}")


def vector_nearly_equal(left, right, tolerance):
    return (
        abs(left.x - right.x) <= tolerance
        and abs(left.y - right.y) <= tolerance
        and abs(left.z - right.z) <= tolerance
    )


blueprint = unreal.load_asset(BLUEPRINT_PATH)
check(blueprint is not None, "BP_SingijeonBox loads")

actor = None
if blueprint:
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        blueprint.generated_class(), unreal.Vector(0.0, 0.0, -100000.0), unreal.Rotator())
    check(actor is not None, "Temporary verification actor spawns")

if actor:
    components = actor.get_components_by_class(unreal.ActorComponent)
    box = next(
        (component for component in components
         if isinstance(component, unreal.StaticMeshComponent)
         and not isinstance(component, unreal.InstancedStaticMeshComponent)
         and component.get_name() == "StaticMesh"),
        None,
    )
    instances = next(
        (component for component in components
         if isinstance(component, unreal.InstancedStaticMeshComponent)
         and component.get_name() == "InstancedStaticMesh"),
        None,
    )
    check(box is not None, "Existing crate StaticMesh component remains")
    if box:
        check(
            vector_nearly_equal(
                box.get_editor_property("relative_location"),
                unreal.Vector(-269.749206, 112.0, 0.0),
                0.01),
            "Crate component location was not changed",
        )
        check(
            vector_nearly_equal(
                box.get_editor_property("relative_scale3d"),
                unreal.Vector(4.5, 2.0, 2.0),
                0.001),
            "Crate component scale was not changed",
        )
    check(instances is not None, "Existing InstancedStaticMesh component remains")
    if instances:
        check(instances.get_instance_count() == EXPECTED_COUNT,
              "ISM contains 7 x 6 = 42 arrows")
        transforms = [
            instances.get_instance_transform(index, False)
            for index in range(instances.get_instance_count())
        ]
        locations = [transform.translation for transform in transforms]
        if locations:
            check(min(value.y for value in locations) >= -16.01 and
                  max(value.y for value in locations) <= 44.01,
                  "Arrow columns stay inside the crate width")
            check(min(value.z for value in locations) >= -0.01 and
                  max(value.z for value in locations) <= 40.01,
                  "Arrow layers fill the crate height without crossing the rim")
        check(not instances.get_editor_property("cast_shadow"),
              "Dense arrow instances disable shadows for VR")
        check(instances.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION,
              "Dense arrow instances disable collision")

    unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(actor)

if errors:
    raise RuntimeError("BP_SingijeonBox verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_BOX_VERIFY SUCCESS")
