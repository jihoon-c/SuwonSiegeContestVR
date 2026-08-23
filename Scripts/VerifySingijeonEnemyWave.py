import unreal


errors = []


def check(condition, message):
    if condition:
        unreal.log(f"SINGIJEON_ENEMY_WAVE VERIFY PASS: {message}")
    else:
        errors.append(message)
        unreal.log_error(f"SINGIJEON_ENEMY_WAVE VERIFY FAIL: {message}")


wave_class = unreal.load_class(None, "/Script/GF_Singijeon.SingijeonEnemyWaveActor")
check(wave_class is not None, "Enemy Wave C++ class loads")

unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/LV_Singijeon")
actors = list(unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors())
waves = [actor for actor in actors
         if isinstance(actor, unreal.SingijeonEnemyWaveActor)]
check(len(waves) == 1, "Level contains exactly one Enemy Wave")
if waves:
    wave = waves[0]
    check(wave.get_editor_property("enemy_count") == 45,
          "Wave contains 45 logical enemies")
    check(wave.get_editor_property("max_interactive_enemies") == 3,
          "Only three enemies use full Actors")
    check(abs(wave.get_editor_property("proxy_update_interval") - 0.1) < 0.001,
          "Proxy transforms update at the VR-safe 10 Hz rate")
    check(wave.get_editor_property("proxy_start_cull_distance") == 5000 and
          wave.get_editor_property("proxy_end_cull_distance") == 12000,
          "Proxy culling is constrained for VR")
    check(wave.get_editor_property("platoon_count") == 3,
          "Wave is split into three platoons")
    check(wave.get_editor_property("target_actor") is not None,
          "Wave targets the placed Hwacha")
    legacy_proxy_component = wave.get_editor_property("proxy_instances")
    check(legacy_proxy_component is not None and
          legacy_proxy_component.get_editor_property("static_mesh") is None and
          legacy_proxy_component.get_instance_count() == 0,
          "Legacy circular target proxy is removed")
    check(wave.get_editor_property("proxy_skeletal_mesh") is not None,
          "Background force uses a full skeletal character mesh")
    provider = wave.get_editor_property("proxy_animation_provider")
    check(provider is not None,
          "Background characters use a GPU animation provider")
    if provider:
        check(len(provider.get_editor_property("sequences")) == 8,
              "GPU animation is shared across eight run phases")
    check(abs(wave.get_editor_property("lateral_jitter") - 52.0) < 0.001 and
          abs(wave.get_editor_property("longitudinal_jitter") - 68.0) < 0.001 and
          abs(wave.get_editor_property("yaw_jitter_degrees") - 11.0) < 0.001,
          "Formation position and facing are visibly randomized")
    check(wave.get_editor_property("start_when_hwacha_loaded"),
          "First Hwacha load starts the charge")
    check(not wave.get_editor_property("start_on_begin_play"),
          "Wave waits for gameplay instead of racing narration")
    check(wave.get_editor_property("limit_approach_by_hwacha_procedure"),
          "Hwacha procedure limits enemy approach distance")
    limits = (
        wave.get_editor_property("loaded_approach_limit"),
        wave.get_editor_property("aimed_approach_limit"),
        wave.get_editor_property("igniting_approach_limit"),
        wave.get_editor_property("firing_approach_limit"),
    )
    check(all(abs(actual - expected) < 0.001 for actual, expected in zip(
        limits, (0.35, 0.60, 0.82, 0.95))),
        "Procedure gates are configured at 35, 60, 82, and 95 percent")

if errors:
    raise RuntimeError("Singijeon Enemy Wave verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_ENEMY_WAVE VERIFY SUCCESS")

