import unreal


failures = []


def check(condition, message):
    if condition:
        unreal.log(f"[PASS] {message}")
    else:
        unreal.log_error(f"[FAIL] {message}")
        failures.append(message)


world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
check(world is not None, "LV_Singijeon loads")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = list(actor_subsystem.get_all_level_actors())
batteries = [
    actor for actor in actors
    if isinstance(actor, unreal.SingijeonHwachaBatteryActor)
]
check(len(batteries) == 4, "Four independently placeable support Hwacha actors are placed")

if batteries:
    playable = batteries[0].get_editor_property("source_hwacha")
    check(
        playable is not None
        and abs(playable.get_editor_property("volley_horizontal_spread_half_angle") - 14.0) < 0.01
        and abs(playable.get_editor_property("volley_vertical_spread_half_angle") - 6.0) < 0.01,
        "Playable Hwacha uses the editable 14 x 6 degree fan spread",
    )
    expected_labels = {
        f"Singijeon_OptimizedSupportHwacha_{index:02d}"
        for index in range(1, 5)
    }
    check(
        {battery.get_actor_label() for battery in batteries} == expected_labels,
        "Support Hwacha actors use stable individual labels",
    )
    check(
        len({tuple(transform_values) for transform_values in (
            (
                battery.get_actor_location().x,
                battery.get_actor_location().y,
                battery.get_actor_location().z,
            )
            for battery in batteries
        )}) == 4,
        "Each support Hwacha has an independent actor Transform",
    )
    check(
        all(
            battery.get_editor_property("source_hwacha") is not None
            and battery.get_editor_property("source_hwacha").get_actor_label()
            == "BP_SingijeonHwacha_Playable"
            for battery in batteries
        ),
        "Every support Hwacha references the playable Hwacha",
    )
    check(
        all(battery.get_cart_count() == 1 for battery in batteries),
        "Each support actor renders exactly one cart",
    )
    check(
        all(battery.get_loaded_arrow_count() == 66 for battery in batteries),
        "Each support actor renders one 6 x 11 arrow set",
    )
    check(
        all(abs(battery.get_editor_property("volley_horizontal_spread_half_angle") - 14.0) < 0.01
            and abs(battery.get_editor_property("volley_vertical_spread_half_angle") - 6.0) < 0.01
            for battery in batteries),
        "Every support Hwacha uses the editable 14 x 6 degree fan spread",
    )
    check(sum(battery.get_loaded_arrow_count() for battery in batteries) == 264,
          "Four actors render 264 loaded arrows in total")
    check(
        all(
            abs(battery.get_editor_property("rack_transform").translation.y - 10.0) < 0.01
            and abs(battery.get_editor_property("rack_transform").translation.z - 80.0) < 0.01
            for battery in batteries
        ),
        "Every arrow rack is shifted slightly right and upward",
    )
    check(
        all(len(battery.get_components_by_class(unreal.ArrowComponent)) == 1
            for battery in batteries),
        "Every support actor exposes an editor arrow-rack guide",
    )
    check(all(battery.get_flying_arrow_count() == 0 for battery in batteries),
          "No flying ISM instances exist before launch")

    for battery in batteries:
        instance_components = list(
            battery.get_components_by_class(unreal.InstancedStaticMeshComponent)
        )
        check(len(instance_components) == 3,
              f"{battery.get_actor_label()} uses exactly three ISM components")
        check(
            all(component.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION
                for component in instance_components),
            f"{battery.get_actor_label()} instances have collision disabled",
        )
        check(
            all(not component.get_editor_property("generate_overlap_events")
                for component in instance_components),
            f"{battery.get_actor_label()} instances have overlap disabled",
        )

if failures:
    raise RuntimeError("; ".join(failures))

unreal.log("SINGIJEON_SUPPORT_BATTERY VERIFY SUCCESS")
