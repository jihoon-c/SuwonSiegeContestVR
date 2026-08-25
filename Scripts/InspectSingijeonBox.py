import unreal


ASSET_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonBox"

blueprint = unreal.load_asset(ASSET_PATH)
if not blueprint:
    raise RuntimeError(f"Missing Blueprint: {ASSET_PATH}")

unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
    blueprint.generated_class(), unreal.Vector(0.0, 0.0, -100000.0), unreal.Rotator())
if not actor:
    raise RuntimeError("Could not spawn BP_SingijeonBox for inspection")

try:
    for component in actor.get_components_by_class(unreal.ActorComponent):
        line = f"SINGIJEON_BOX_COMPONENT name={component.get_name()} class={component.get_class().get_name()}"
        if isinstance(component, unreal.SceneComponent):
            line += (
                f" location={component.get_editor_property('relative_location')}"
                f" rotation={component.get_editor_property('relative_rotation')}"
                f" scale={component.get_editor_property('relative_scale3d')}"
                f" parent={component.get_attach_parent().get_name() if component.get_attach_parent() else 'None'}"
            )
        if isinstance(component, unreal.StaticMeshComponent):
            mesh = component.get_editor_property("static_mesh")
            line += f" mesh={mesh.get_path_name() if mesh else 'None'}"
            if mesh:
                bounds = mesh.get_bounds()
                line += f" mesh_origin={bounds.origin} mesh_extent={bounds.box_extent}"
        if isinstance(component, unreal.InstancedStaticMeshComponent):
            line += f" instances={component.get_instance_count()}"
            if component.get_instance_count() > 0:
                locations = []
                for index in range(component.get_instance_count()):
                    transform = component.get_instance_transform(index, False)
                    locations.append(transform.translation)
                min_v = unreal.Vector(
                    min(value.x for value in locations),
                    min(value.y for value in locations),
                    min(value.z for value in locations),
                )
                max_v = unreal.Vector(
                    max(value.x for value in locations),
                    max(value.y for value in locations),
                    max(value.z for value in locations),
                )
                line += f" instance_min={min_v} instance_max={max_v}"
                for index in range(min(7, component.get_instance_count())):
                    line += f" i{index}={component.get_instance_transform(index, False)}"
        unreal.log(line)
finally:
    unreal.EditorLevelLibrary.destroy_actor(actor)

unreal.log("SINGIJEON_BOX_INSPECT SUCCESS")
