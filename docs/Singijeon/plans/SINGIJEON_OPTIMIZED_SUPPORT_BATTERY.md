# 신기전 동시 발사 최적화 화차 편대

> 상태: 부채꼴 발사 개선 완료 (2026-08-26)

## 작업 목표

- `BP_SingijeonHwacha_Playable`의 실제 발사 시작과 동시에 추가 화차 편대가 발사한다.
- 추가 편대는 시나리오 판정이나 그랩 기능을 중복하지 않는 시각 연출 전용이다.
- VR에서 수백 개의 화살 액터/물리/개별 Tick을 만들지 않는다.
- 기존 레벨 액터의 Transform은 변경하지 않고 새 편대 액터만 배치한다.

## 영향 범위

- `GF_Singijeon`에 최적화 편대 액터 C++ 클래스 추가
- 신기전 자동화 테스트 추가
- `/Game/Maps/LV_Singijeon`에 개별 지원 화차 액터 4개 추가
- 배치 및 검증용 Unreal Python 스크립트 추가

## 개별 배치 개선

- 편대 Actor 하나에 화차 4대를 넣지 않고 Actor 하나가 화차 1대와 6 x 11 화살만 가진다.
- 레벨에는 지원 화차 Actor 4개를 각각 배치해 위치/회전/스케일을 독립 수정할 수 있게 한다.
- 화살 랙 기준 Transform은 우측/상단으로 소폭 보정하고 `Make Edit Widget`으로 뷰포트에서 직접 조절한다.

## 부채꼴 발사 개선

- Playable 화차와 개별 지원 화차 모두 화살촉 기준 방향에 좌우/상하 각도 분산을 적용한다.
- 기본 반각은 좌우 14도, 상하 6도이며 각 Actor의 Details에서 수정할 수 있다.
- 발사체 Actor가 전달받은 분산 방향을 실제 속도와 메시 화살촉 회전에 사용하도록 정렬한다.

## 구현 구조

- 화차 본체: ISM 1개
- 장전 화살: ISM 1개
- 비행 화살: ISM 1개
- 동기화: `ASingijeonHwachaActor::OnHwachaStateChanged`의 `Fired` 상태 구독
- 갱신: 각 화차 Actor는 발사 중에만 Tick하고 비행 Transform을 30 Hz로 묶음 갱신
- 제외: 화살별 Actor, ProjectileMovement, Collision, Niagara, 시나리오 Interactor

## 완료 조건

- Playable이 `Fired`가 되면 편대도 즉시 10초 발사를 시작한다.
- 편대 화차와 장전 화살이 에디터에서도 보인다.
- 각 발사마다 장전 인스턴스가 하나 줄고 비행 인스턴스가 생긴다.
- 비행 종료 후 인스턴스가 제거되어 다시 나타나지 않는다.
- 레벨 내 기존 액터 Transform에 변경이 없다.
- C++ 빌드와 자동화 테스트, 레벨 배치 검증이 통과한다.

## 롤백

- 레벨의 `Singijeon_OptimizedSupportBattery` 액터를 삭제한다.
- `SingijeonHwachaBatteryActor` 소스와 관련 테스트/스크립트를 삭제한다.
