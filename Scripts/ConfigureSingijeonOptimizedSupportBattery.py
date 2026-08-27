import unreal


LEVEL_PATH = "/Game/Maps/LV_Singijeon"
PLAYABLE_LABEL = "BP_SingijeonHwacha_Playable"
BATTERY_LABEL_PREFIX = "Singijeon_OptimizedSupportHwacha_"
SUPPORT_OFFSETS = (
    unreal.Vector(0.0, -250.0, 0.0),
    unreal.Vector(0.0, 250.0, 0.0),
    unreal.Vector(-300.0, -250.0, 0.0),
    unreal.Vector(-300.0, 250.0, 0.0),
)


def transform_values(actor):
    location = actor.get_actor_location()
    rotation = actor.get_actor_rotation()
    scale = actor.get_actor_scale3d()
    return (
        location.x, location.y, location.z,
        rotation.pitch, rotation.yaw, rotation.roll,
        scale.x, scale.y, scale.z,
    )


world = unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
if not world:
    raise RuntimeError("LV_Singijeon could not be loaded")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = list(actor_subsystem.get_all_level_actors())
playable = next(
    (actor for actor in actors if actor.get_actor_label() == PLAYABLE_LABEL), None
)
if not playable or not isinstance(playable, unreal.SingijeonHwachaActor):
    raise RuntimeError("Playable Singijeon Hwacha was not found")

# The preservation snapshot makes this script fail before saving if it moves any
# previously placed actor. Only support-Hwacha actors being replaced are exempt.
preserved_transforms = {
    actor.get_path_name(): transform_values(actor)
    for actor in actors
    if not isinstance(actor, unreal.SingijeonHwachaBatteryActor)
}

batteries = [
    actor for actor in actors
    if isinstance(actor, unreal.SingijeonHwachaBatteryActor)
]
for battery in batteries:
    actor_subsystem.destroy_actor(battery)

playable_transform = playable.get_actor_transform()
new_batteries = []
for index, local_offset in enumerate(SUPPORT_OFFSETS, start=1):
    battery = actor_subsystem.spawn_actor_from_class(
        unreal.SingijeonHwachaBatteryActor,
        playable_transform.transform_location(local_offset),
        playable.get_actor_rotation(),
    )
    if not battery:
        raise RuntimeError(f"Support Hwacha {index:02d} could not be spawned")
    battery.set_actor_label(f"{BATTERY_LABEL_PREFIX}{index:02d}")
    battery.set_actor_scale3d(playable.get_actor_scale3d())
    battery.set_editor_property("source_hwacha", playable)
    battery.set_editor_property("arrow_rows", 6)
    battery.set_editor_property("arrow_columns", 11)
    # Backward-compatible migration: the previously built class stored four
    # local carts in this array. New builds always create one identity cart.
    if hasattr(battery, "cart_transforms"):
        battery.set_editor_property("cart_transforms", [unreal.Transform()])
    battery.set_editor_property(
        "rack_transform",
        unreal.Transform(
            location=unreal.Vector(25.0, 10.0, 80.0),
            rotation=unreal.Rotator(25.0, 0.0, 0.0),
        ),
    )
    battery.set_editor_property("volley_duration", 10.0)
    battery.set_editor_property("volley_horizontal_spread_half_angle", 14.0)
    battery.set_editor_property("volley_vertical_spread_half_angle", 6.0)
    battery.set_editor_property("visual_update_interval", 1.0 / 30.0)
    battery.set_editor_property("sound_every_nth_arrow", 16)
    new_batteries.append(battery)

current_by_path = {
    actor.get_path_name(): actor
    for actor in actor_subsystem.get_all_level_actors()
}
for actor_path, expected_transform in preserved_transforms.items():
    actor = current_by_path.get(actor_path)
    if not actor or transform_values(actor) != expected_transform:
        raise RuntimeError(f"Existing actor Transform changed unexpectedly: {actor_path}")

for battery in new_batteries:
    if battery.get_cart_count() != 1:
        raise RuntimeError(
            f"Expected 1 cart in {battery.get_actor_label()}, got {battery.get_cart_count()}"
        )
    if battery.get_loaded_arrow_count() != 66:
        raise RuntimeError(
            f"Expected 66 arrows in {battery.get_actor_label()}, "
            f"got {battery.get_loaded_arrow_count()}"
        )

if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True):
    raise RuntimeError("LV_Singijeon could not be saved")

unreal.log(
    "SINGIJEON_SUPPORT_BATTERY CONFIGURE SUCCESS "
    f"actors={len(new_batteries)} carts={sum(x.get_cart_count() for x in new_batteries)} "
    f"loaded_arrows={sum(x.get_loaded_arrow_count() for x in new_batteries)} "
    f"source={playable.get_actor_label()}"
)
