"""Creates the interaction-highlight material and the Ongseong combat sound concurrency.

Run inside the Unreal Editor (Output Log -> Cmd -> "py <path>") or headlessly:

    UnrealEditor-Cmd.exe <uproject> -ExecCmds="py <this file>;Quit" -unattended -nosplash

Produces:
  /Game/Core/VR/Interaction/M_InteractionHighlight        rim glow used as an overlay material
  /Game/Core/VR/Interaction/MI_InteractionHighlight_Blue  grab prompt colour (UInteractionHighlightComponent default)
  /Game/Core/VR/Interaction/MI_InteractionHighlight_Amber  hold-with-both-hands colour
  /GF_OngseongCrossbow/Asset/Sound/SC_OngseongCombat      caps simultaneous combat sounds

and links the concurrency into the chongtong, shell and crossbow Blueprints.
Idempotent: existing assets are reconfigured, never duplicated.
"""

import unreal

CORE_INTERACTION_FOLDER = "/Game/Core/VR/Interaction"
HIGHLIGHT_MATERIAL = CORE_INTERACTION_FOLDER + "/M_InteractionHighlight"
HIGHLIGHT_BLUE = CORE_INTERACTION_FOLDER + "/MI_InteractionHighlight_Blue"
HIGHLIGHT_AMBER = CORE_INTERACTION_FOLDER + "/MI_InteractionHighlight_Amber"

SOUND_FOLDER = "/GF_OngseongCrossbow/Asset/Sound"
COMBAT_CONCURRENCY = SOUND_FOLDER + "/SC_OngseongCombat"

FEATURE_BLUEPRINTS = "/GF_OngseongCrossbow/Blueprints"
CONCURRENCY_TARGETS = (
    (FEATURE_BLUEPRINTS + "/BP_ChongtongProjectile", "explosion_sound_concurrency"),
    (FEATURE_BLUEPRINTS + "/BP_ChongtongCannon", "combat_sound_concurrency"),
    (FEATURE_BLUEPRINTS + "/BP_PlayableChongtong", "combat_sound_concurrency"),
    (FEATURE_BLUEPRINTS + "/BP_AllyChongtong", "combat_sound_concurrency"),
    (FEATURE_BLUEPRINTS + "/BP_OngseongCrossbow", "fire_sound_concurrency"),
)

# 15 living enemies plus four chongtongs can stack a lot of one-shots on a standalone headset.
MAX_CONCURRENT_COMBAT_SOUNDS = 8

GRAB_BLUE = unreal.LinearColor(0.05, 0.45, 1.0, 1.0)
HOLD_AMBER = unreal.LinearColor(1.0, 0.55, 0.05, 1.0)
DEFAULT_INTENSITY = 4.0

asset_lib = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
material_lib = unreal.MaterialEditingLibrary
PROBLEMS = []


def log(message):
    unreal.log("[OngseongFX] " + message)


def warn(message):
    PROBLEMS.append(message)
    unreal.log_warning("[OngseongFX] " + message)


def create_asset(path, asset_class, factory):
    if asset_lib.does_asset_exist(path):
        return asset_lib.load_asset(path)
    folder, name = path.rsplit("/", 1)
    created = asset_tools.create_asset(name, folder, asset_class, factory)
    if not created:
        raise RuntimeError("Failed to create " + path)
    log("Created " + path)
    return created


def build_highlight_material():
    """Fresnel rim glow. Additive so the object keeps its own shading and nothing sorts wrongly."""
    material = create_asset(HIGHLIGHT_MATERIAL, unreal.Material, unreal.MaterialFactoryNew())
    if material.get_editor_property("shading_model") == unreal.MaterialShadingModel.MSM_UNLIT and \
            material_lib.get_num_material_expressions(material) > 0:
        log("Highlight material already built")
        return material

    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_SURFACE)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("two_sided", True)

    color = material_lib.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -900, -150)
    color.set_editor_property("parameter_name", "HighlightColor")
    color.set_editor_property("default_value", GRAB_BLUE)

    intensity = material_lib.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -900, 50)
    intensity.set_editor_property("parameter_name", "Intensity")
    intensity.set_editor_property("default_value", DEFAULT_INTENSITY)

    fresnel = material_lib.create_material_expression(material, unreal.MaterialExpressionFresnel, -900, 250)
    fresnel.set_editor_property("exponent", 2.5)
    fresnel.set_editor_property("base_reflect_fraction", 0.15)

    color_times_intensity = material_lib.create_material_expression(material, unreal.MaterialExpressionMultiply, -550, -50)
    material_lib.connect_material_expressions(color, "", color_times_intensity, "A")
    material_lib.connect_material_expressions(intensity, "", color_times_intensity, "B")

    rim = material_lib.create_material_expression(material, unreal.MaterialExpressionMultiply, -250, 0)
    material_lib.connect_material_expressions(color_times_intensity, "", rim, "A")
    material_lib.connect_material_expressions(fresnel, "", rim, "B")

    material_lib.connect_material_property(rim, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    material_lib.recompile_material(material)
    asset_lib.save_loaded_asset(material)
    log("Built " + HIGHLIGHT_MATERIAL)
    return material


def build_highlight_instance(path, parent, color):
    instance = create_asset(path, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    material_lib.set_material_instance_parent(instance, parent)
    material_lib.set_material_instance_vector_parameter_value(instance, "HighlightColor", color)
    material_lib.set_material_instance_scalar_parameter_value(instance, "Intensity", DEFAULT_INTENSITY)
    asset_lib.save_loaded_asset(instance)
    log("Configured " + path)
    return instance


def build_combat_concurrency():
    concurrency = create_asset(COMBAT_CONCURRENCY, unreal.SoundConcurrency, unreal.SoundConcurrencyFactory())
    settings = concurrency.get_editor_property("concurrency")
    settings.set_editor_property("max_count", MAX_CONCURRENT_COMBAT_SOUNDS)
    settings.set_editor_property("limit_to_owner", False)
    settings.set_editor_property("resolution_rule", unreal.MaxConcurrentResolutionRule.STOP_FARTHEST_THEN_OLDEST)
    settings.set_editor_property("volume_scale", 0.85)
    concurrency.set_editor_property("concurrency", settings)
    asset_lib.save_loaded_asset(concurrency)
    log("Configured {0} (max {1} voices)".format(COMBAT_CONCURRENCY, MAX_CONCURRENT_COMBAT_SOUNDS))
    return concurrency


def link_concurrency(concurrency):
    for path, property_name in CONCURRENCY_TARGETS:
        blueprint = asset_lib.load_asset(path)
        if not blueprint:
            warn("Missing Blueprint " + path)
            continue
        default_object = unreal.get_default_object(blueprint.generated_class())
        try:
            default_object.set_editor_property(property_name, concurrency)
        except Exception as error:
            warn("Could not set {0} on {1}: {2}".format(property_name, path, error))
            continue
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        asset_lib.save_loaded_asset(blueprint)
        log("Linked {0} -> {1}".format(path, property_name))


def main():
    material = build_highlight_material()
    build_highlight_instance(HIGHLIGHT_BLUE, material, GRAB_BLUE)
    build_highlight_instance(HIGHLIGHT_AMBER, material, HOLD_AMBER)
    link_concurrency(build_combat_concurrency())
    if PROBLEMS:
        unreal.log_warning("[OngseongFX] Finished with {0} item(s) needing attention:".format(len(PROBLEMS)))
        for problem in PROBLEMS:
            unreal.log_warning("[OngseongFX]   - " + problem)
    else:
        log("Done.")


main()

if "-unattended" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
