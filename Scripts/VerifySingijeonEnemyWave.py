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
    check(wave.get_editor_property("max_interactive_enemies") == 10,
          "Only ten enemies use full Actors")
    check(wave.get_editor_property("platoon_count") == 3,
          "Wave is split into three platoons")
    check(wave.get_editor_property("target_actor") is not None,
          "Wave targets the placed Hwacha")
    check(wave.get_editor_property("proxy_mesh") is not None,
          "Wave has a lightweight proxy mesh")
    check(wave.get_editor_property("start_when_hwacha_loaded"),
          "First Hwacha load starts the charge")
    check(not wave.get_editor_property("start_on_begin_play"),
          "Wave waits for gameplay instead of racing narration")

if errors:
    raise RuntimeError("Singijeon Enemy Wave verification failed: " + "; ".join(errors))
unreal.log("SINGIJEON_ENEMY_WAVE VERIFY SUCCESS")

