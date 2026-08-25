# 목적

옹성 시나리오의 남은 작업 중 사용자가 지시한 항목을 처리한다.

1. 테스트 레벨에서만 검증된 전투 배치를 본편 `LV_Ongseong`에 적용한다
2. 충차 접근 시간을 약 3분으로 조정한다
3. `SC_OngseongCombat` Sound Concurrency 에셋을 만들고 연결한다
4. 임시 파티클을 어울리는 에셋으로 교체한다
5. 플레이어를 성벽 위 고정으로 확정하고 구현한다
6. 인터랙션이 모두 구현되어 있는지 점검하고, 인터랙션 지점에 파티클/발광 피드백을 추가한다

**상태**: `Completed` — 결과는 `completed/2026-08-24_MAIN_LEVEL_ROLLOUT_AND_INTERACTION_FX.md`

---

# 현재 상태 (작업 착수 시점)

* 코드 수정과 `SM_Ground` 콜리전 수정은 본편·테스트 레벨 양쪽에 적용되어 있었다
* 스포너 위치·총통 방향·NavMesh 볼륨·충차 Pool/스폰 지점은 **테스트 레벨에만** 적용되어 있었다
* 충차는 8,250cm를 35cm/s로 접근해 약 4분이 걸렸다
* `ExplosionSoundConcurrency` / `CombatSoundConcurrency` 속성은 존재하지만 연결된 에셋이 없었다
* 포탄 폭발과 총통 발사가 모두 Engine 템플릿 `SimpleExplosion`을 사용했다
* 플레이어 이동 정책은 `STATUS.md` 4.6에서 미확정이었다
* 장전물(화약·쑤시개·대포알)과 그립 지점에 **집을 수 있다는 시각 신호가 전혀 없었다**

---

# 구현 범위

## Core (Shared 상호작용 계약)

* `UInteractionHighlightComponent` 신설 — Overlay Material 발광 + 루프 Niagara 조합
* `AVRPlayerPawn::SetLocomotionEnabled(bMove, bTeleport)`와 `bEnableTeleport` 추가
* `/Game/Core/VR/Interaction/M_InteractionHighlight` 및 Blue/Amber Material Instance

Core는 어떤 Game Feature도 참조하지 않는다. Feature가 Core 컴포넌트를 켜고 끄기만 한다.

## Feature (GF_OngseongCrossbow)

* 장전물 3종에 장전 프롬프트 발광 연결, **지금 필요한 하나만** 발광
* 총통 양손 그립 지점에 준비 완료 시 호박색 프롬프트
* 쇠뇌 그립 지점에 장전 완료·미파지 시 파란색 프롬프트, 발사 FX/사운드 추가
* 충차 `MoveSpeed` 35 → 42 cm/s
* 총통 발사 FX를 Muzzle Flash로, 포탄 폭발 FX를 Dirt Explosion으로 교체
* `AOngseongDefenseScenarioManager`가 시작 시 플레이어 이동/텔레포트를 잠금

## 에셋 / 레벨

* `/GF_OngseongCrossbow/Asset/Sound/SC_OngseongCombat` 생성 및 5개 Blueprint에 연결
* `LV_Ongseong`에 테스트 레벨과 동일한 배치·Pool 적용

---

# 변경 예정 파일

```text
Source/SuwonSiegeContestVR/Public/Core/VR/InteractionHighlightComponent.h      (신규)
Source/SuwonSiegeContestVR/Private/Core/VR/InteractionHighlightComponent.cpp   (신규)
Source/SuwonSiegeContestVR/Public/Core/VR/VRPlayerPawn.h
Source/SuwonSiegeContestVR/Private/Core/VR/VRPlayerPawn.cpp
Plugins/.../Public|Private/Ongseong/ChongtongLoadingItemActor.*
Plugins/.../Public|Private/Ongseong/ChongtongCannonActor.*
Plugins/.../Public|Private/Ongseong/OngseongCrossbowActor.*
Plugins/.../Public/Ongseong/OngseongRamActor.h
Plugins/.../Public|Private/Ongseong/OngseongDefenseScenarioManager.*
Plugins/.../Private/Ongseong/ChongtongProjectileActor.cpp
Plugins/.../Private/Tests/OngseongInteractionPromptTests.cpp                   (신규)
Scripts/CreateOngseongInteractionFX.py                                        (신규)
Scripts/ApplyOngseongCombatLayout.py                                          (신규)
Content/Core/VR/Interaction/*                                                 (신규 에셋)
Plugins/.../Content/Asset/Sound/SC_OngseongCombat.uasset                      (신규 에셋)
Plugins/.../Content/Maps/LV_Ongseong.umap
Plugins/.../Content/Blueprints/BP_Chongtong*.uasset, BP_OngseongCrossbow.uasset
```

---

# 구현 단계

1. Core 하이라이트 컴포넌트와 이동 잠금 계약을 추가한다
2. Feature 액터들이 프롬프트를 켜고 끄도록 연결한다
3. 충차 속도와 FX 기본값을 교체한다
4. Editor 타깃을 빌드한다
5. 에디터 Python으로 하이라이트 Material과 Sound Concurrency를 생성·연결한다
6. 에디터 Python으로 본편 레벨 배치를 적용한다
7. Game 타깃 빌드, Automation, 본편 레벨 헤드리스 구동으로 검증한다
8. 문서를 갱신한다

---

# 다른 Feature에 미치는 영향

* `UInteractionHighlightComponent`와 `SetLocomotionEnabled`는 Core에 추가되므로
  신기전·공심돈·거중기에서도 그대로 재사용할 수 있다. 기존 동작은 바뀌지 않는다
  (`bEnableTeleport` 기본값 true, 하이라이트는 명시적으로 켜야 동작).
* `GF_Singijeon`은 이미 화차 손잡이에 자체 하이라이트 메시를 두고 있다.
  이번 변경으로 그 구현을 건드리지 않으며, 이후 통일 여부는 별도 판단 사항이다.
* `LV_Ongseong` 배치 변경은 옹성 체험의 전투 구도를 바꾼다. 백업은
  `Saved/CodexBackups/2026-08-24_OngseongMainLevelRollout/LV_Ongseong.umap`에 있다.

---

# 검증 방법

* `SuwonSiegeContestVREditor` / `SuwonSiegeContestVR` Win64 Development 빌드
* `Automation RunTests SuwonSiegeContestVR` 전수 통과
* 본편 `LV_Ongseong` 헤드리스 구동에서 적 스폰·총통 교전·충차 접근 시간·클리어 확인
* 실제 HMD 확인은 이 작업 범위 밖이며 여전히 남는다
