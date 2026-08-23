# Scenario 인터랙션 가이드와 VR 자막 가시성 개선 계획

**상태: 완료 (2026-08-22)**

## 목적

현재 Scenario에서 수행해야 하는 액터 위에 `Grab`, `Drag`, `Trigger`, `Observe`, `Combat` 등 상황별 안내를 표시하고, 카메라 앞 World Space 자막이 월드 메시 뒤에 가려지는 문제를 해결한다.

## 현재 상태

- Scenario는 `InteractionType + TargetID`로 현재 행동과 대상 액터를 식별할 수 있다.
- 대상 액터 대부분은 동일한 `TargetID`를 가진 `ScenarioInteractableComponent`를 보유한다.
- 공통 인터랙션 가이드 UI는 없고 일부 Feature만 메시 하이라이트를 자체 제공한다.
- 자막은 HMD 전방 180cm의 World Space Widget이라 그 사이에 메시가 있으면 Depth Test로 가려진다.

## 구현 범위

- Scenario Interaction에 에디터 설정 가능한 가이드 동작 종류와 문구 재정의 추가
- Scenario Manager가 현재 Target Actor를 찾아 액터 위에 공통 World Widget 표시
- 타입별 기본 입력 안내와 Feature별 `Custom` 단계의 명시적 Grab/Drag/Trigger 설정
- 자막을 HMD 가까이 옮기고 Screen Space로 전환하여 월드 메시의 가림 제거

## 변경 예정 파일

- Core Scenario 타입, Manager, 공통 Guide Component/Widget
- `VRPlayerPawn` 자막 HUD 설정
- 공심돈·신기전 Scenario 구성 스크립트와 Data Asset
- Core 자동화 테스트와 관련 문서

## 구현 단계

1. 가이드 작성 필드와 기본 표시 문구를 정의한다.
2. Manager에 현재 Target Actor 탐색·표시·완료 시 숨김 기능을 추가한다.
3. 공심돈·신기전의 실제 플레이 Interaction에 가이드 종류를 지정한다.
4. 자막 렌더링 공간과 거리를 수정한다.
5. 에셋 검증, C++ 빌드, 자동화와 Blueprint 컴파일을 수행한다.

## 다른 Feature에 미치는 영향

공통 Scenario Manager를 사용하는 모든 Feature가 가이드 기능을 자동으로 사용할 수 있다. 특정 Feature 클래스에 대한 Core 역의존은 추가하지 않으며 `docs/ARCHITECTURE.md`는 수정하지 않는다.

## 검증 방법

- 가이드 타입별 기본 문구와 표시 가능 여부 자동화 테스트
- 공심돈·신기전 DA의 플레이 Interaction 가이드 설정 검증
- Manager가 TargetID가 일치하는 Actor를 해석하는지 런타임 테스트
- 자막 Widget이 HMD 가까운 Screen Space 설정인지 C++/Blueprint 기본값 확인
- Editor 빌드와 관련 자동화 테스트 실행
