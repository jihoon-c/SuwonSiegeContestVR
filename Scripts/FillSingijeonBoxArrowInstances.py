import unreal


BLUEPRINT_PATH = "/GF_Singijeon/Gameplay/BP_SingijeonBox"
INSTANCE_COMPONENT_NAME = "InstancedStaticMesh"
COLUMNS = 7
LAYERS = 6
COLUMN_START_Y = -16.0
COLUMN_SPACING = 10.0
LAYER_SPACING = 8.0

blueprint = unreal.load_asset(BLUEPRINT_PATH)
if not blueprint:
    raise RuntimeError(f"Missing Blueprint: {BLUEPRINT_PATH}")

subobjects = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
subobject_bfl = unreal.SubobjectDataBlueprintFunctionLibrary
instances = None
for handle in subobjects.k2_gather_subobject_data_for_blueprint(blueprint):
    component = subobject_bfl.get_object(subobject_bfl.get_data(handle))
    if component:
        unreal.log(
            f"SINGIJEON_BOX_FILL TEMPLATE name={component.get_name()} "
            f"class={component.get_class().get_name()}"
        )
    if (
        isinstance(component, unreal.InstancedStaticMeshComponent)
        and component.get_name().startswith(INSTANCE_COMPONENT_NAME)
    ):
        instances = component
        break

if not instances:
    raise RuntimeError(f"Missing ISM component: {INSTANCE_COMPONENT_NAME}")
if not instances.get_editor_property("static_mesh"):
    raise RuntimeError("BP_SingijeonBox ISM has no Static Mesh")

# The authored crate interior is approximately X=225, Y=80, Z=60 cm.
# The arrow is approximately X=200, Y=12, Z=9 cm, so a dense 7 x 6 stack
# fills the usable interior while keeping arrowheads and feathers inside the walls.
instances.clear_instances()
for layer in range(LAYERS):
    for column in range(COLUMNS):
        # Tiny deterministic X offsets break the perfectly flat end silhouette
        # without rotating the 200 cm arrows into the crate walls.
        x_offset = ((column + layer * 2) % 3 - 1) * 1.25
        location = unreal.Vector(
            x_offset,
            COLUMN_START_Y + column * COLUMN_SPACING,
            layer * LAYER_SPACING,
        )
        instances.add_instance(unreal.Transform(location=location), False)

instances.set_editor_property("cast_shadow", False)
instances.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
instances.set_editor_property("generate_overlap_events", False)

unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False):
    raise RuntimeError(f"Could not save Blueprint: {BLUEPRINT_PATH}")

unreal.log(
    "SINGIJEON_BOX_FILL SUCCESS: "
    f"{BLUEPRINT_PATH} instances={COLUMNS * LAYERS} layout={COLUMNS}x{LAYERS}"
)
