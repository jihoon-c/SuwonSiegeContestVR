# 목적

**상태: Completed (2026-08-12)**

프로젝트 전용 VR Player Pawn과 Data Table 기반 나레이션·자막 진행 시스템을 구현한다. 나레이션이 끝나면 자막을 숨기고, 데이터에 따라 다음 대사로 자동 진행하거나 외부 신호를 기다리며, 임의 이벤트와 VR 위젯 표시 요청을 처리할 수 있어야 한다.

# 현재 상태

- 템플릿 `BP_XRPawn`만 존재하며 프로젝트 전용 Pawn이 없다.
- 공통 자막 Widget, 나레이션 데이터, 진행 관리 컴포넌트가 없다.
- Main 문서에는 NPC 대사 데이터 형식과 자막 표시 방식이 미결정으로 기록되어 있다.

# 구현 범위

- Core C++ VR Pawn 기반 클래스와 Blueprint 자산
- 시야 앞 World Space 자막 HUD 및 이벤트 위젯 영역
- `FTableRowBase` 기반 나레이션 행 구조와 빈 Data Table 자산
- 음성 종료 기반 자막 해제, 자동 진행, 수동 진행 대기, 시퀀스 종료
- 행 종료 이벤트 배열과 후속 Widget Class 요청
- Blueprint에서 구독 가능한 진행 이벤트

# 변경 예정 파일

- `Source/SuwonSiegeContestVR/Core/VR/*`
- `Source/SuwonSiegeContestVR/Core/Narration/*`
- `Source/SuwonSiegeContestVR/SuwonSiegeContestVR.Build.cs`
- `Content/Core/VR/Pawn/BP_VRPlayerPawn.uasset`
- `Content/Core/Experience/Definitions/DT_Narration.uasset`
- `docs/Main/*`
- `docs/ARCHITECTURE.md`
- `docs/DIRECTORY_STRUCTURE.md`

# 구현 단계

1. 나레이션 행 구조와 진행 컴포넌트 구현
2. 네이티브 자막 Widget 구현
3. VR Pawn 컴포넌트 계층 및 HUD 연결
4. Blueprint Pawn과 Data Table 자산 생성
5. C++ 및 Blueprint 로드 검증
6. 문서 완료 상태 반영

# 다른 Feature에 미치는 영향

Core가 특정 Game Feature를 참조하지 않는다. 각 Feature는 나레이션 행의 이벤트 이름을 구독하거나 진행 컴포넌트 API를 호출하는 방향으로만 연동한다.

# 검증 방법

- Unreal Editor C++ 빌드 및 UHT 통과
- 생성 Blueprint와 Data Table의 부모/행 구조 확인
- 전체 Blueprint CompileAllBlueprints 오류 확인
- 이전 템플릿 Pawn 자산이 변경되지 않았는지 확인
