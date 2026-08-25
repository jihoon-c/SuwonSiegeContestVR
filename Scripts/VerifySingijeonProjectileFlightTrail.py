import math
import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_FLIGHT_TRAIL VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_FLIGHT_TRAIL VERIFY FAIL: {message}")


blueprint = unreal.load_asset("/GF_Singijeon/Gameplay/BP_SingijeonArrow")
check(blueprint is not None, "BP_SingijeonArrow loads")
if blueprint:
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    cdo = unreal.get_default_object(blueprint.generated_class())
    trail = cdo.get_editor_property("flight_trail_effect")
    location = cdo.get_editor_property("flight_trail_relative_location")
    rotation = cdo.get_editor_property("flight_trail_relative_rotation")
    scale = cdo.get_editor_property("flight_trail_relative_scale")

    check(trail is not None, "Projectile owns the Niagara flight-trail component")
    root_component = cdo.get_editor_property("root_component")
    check(trail is not None and trail.get_attach_parent() == root_component,
          "Flight trail is attached to ProjectileMesh")
    check(trail is not None and not trail.is_active(),
          "Flight trail is inactive before launch")
    check(cdo.get_editor_property("flight_trail_system") is None,
          "Flight Trail System is an optional editor slot")
    check(math.isclose(location.x, 0.0) and math.isclose(location.y, 0.0) and
          math.isclose(location.z, 0.0), "Trail relative location is editable")
    check(math.isclose(rotation.pitch, 0.0) and math.isclose(rotation.yaw, 0.0) and
          math.isclose(rotation.roll, 0.0), "Trail relative rotation is editable")
    check(math.isclose(scale.x, 1.0) and math.isclose(scale.y, 1.0) and
          math.isclose(scale.z, 1.0), "Trail relative scale is editable")

if errors:
    raise RuntimeError("Singijeon projectile flight-trail verification failed: " +
                       "; ".join(errors))
unreal.log("SINGIJEON_PROJECTILE_FLIGHT_TRAIL VERIFY SUCCESS")
