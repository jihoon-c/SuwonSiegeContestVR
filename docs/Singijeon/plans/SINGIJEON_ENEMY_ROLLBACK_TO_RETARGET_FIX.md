# 목적

적군을 `SINGIJEON_SAMURAI_RETARGETER_PREVIEW_FIX` 완료 직후의 단일 Wave 동작으로 복원한다.

상태: 완료

# 현재 상태

- Samurai Root/Hips Scale, IK Goal, Retargeter, 달리기 애니메이션은 정상화되어 있다.
- 이후 생존자 후퇴, 다중 Wave 예산 분배, 측면 Wave 2개가 추가되었다.
- 행군 사운드는 최근 별도 요구사항으로 추가되었다.

# 구현 범위

- 중앙 `Singijeon_EnemyWave`는 현재 Transform을 그대로 유지한다.
- 이후 추가된 좌/우 측면 Wave만 제거한다.
- 후퇴 상태와 다중 Wave 자동 예산 분배를 제거한다.
- 당시 값인 10Hz 갱신, Pose Leader 8개, Volley 사상률 100%, Scale 0.9를 복원한다.
- Samurai/IK/Retargeter/Run Animation과 행군 사운드는 유지한다.

# 변경 예정 파일

- `SingijeonEnemyWaveActor.h/.cpp`
- `SingijeonEnemyWaveTests.cpp`
- Enemy Wave 설정/검증 스크립트
- `LV_Singijeon`의 Enemy Wave 배치 데이터

# 구현 단계

1. 후퇴 및 다중 Wave 런타임 코드를 제거한다.
2. 자동화 테스트와 설정 스크립트를 단일 Wave 기준으로 복원한다.
3. 중앙 Wave Transform을 보존하며 측면 Wave만 제거하고 당시 속성을 적용한다.
4. C++ 빌드, 자동화 테스트, 레벨 데이터 검증을 수행한다.

# 다른 Feature에 미치는 영향

`GF_Singijeon` 적군 Wave에만 한정하며 Core, 타 Feature, 기존 레벨 Actor Transform은 변경하지 않는다.

# 검증 방법

- UnrealBuildTool 빌드
- Enemy Wave 자동화 테스트
- 레벨 내 Wave 수와 중앙 Wave Transform 전후 비교
- Samurai 메시/Run Animation/행군 사운드 참조 확인

# 완료 결과

- 중앙 Wave Transform과 다른 기존 Actor Transform을 유지했다.
- 좌/우 측면 Wave 2개를 제거했다.
- 후퇴 및 다중 Wave 자동 예산 분배를 제거했다.
- 단일 Wave/45명/Scale 0.9/10Hz/Pose Leader 8/Volley 100% 상태로 복원했다.
- C++ 빌드와 Unreal Python 레벨/에셋 검증을 통과했다.
- 새 플러그인 DLL로 Enemy Wave 자동화 테스트 2개를 모두 통과했다.
