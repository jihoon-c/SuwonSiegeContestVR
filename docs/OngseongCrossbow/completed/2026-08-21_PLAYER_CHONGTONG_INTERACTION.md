# 플레이어 총통 상호작용 구현

## 결과

- 장전 순서: 화약 → 쑤시개 왕복 3회 → 대포알
- 장전물: Engine 기본 Cylinder/Sphere 메시를 사용하는 교체 가능한 Blueprint Actor
- 그랩: 화약/쑤시개/대포알 Blueprint에 기존 Core `BP_GrabComponent`를 `GrabPoint`로 조립
- 신호: 단계별 TextRender + 색상 PointLight + 성공 Niagara/사운드
- 준비 완료: VR Pawn 카메라를 총통 뒤 `PlayerCameraAnchor`에 고정하고 이동 잠금
- 조준: 양손 그립이 유지되는 동안 손 사이 방향으로 포신 yaw/pitch 회전
- 발사: 양손 트리거가 동시에 눌렸을 때만 발사
- 포탄: 중력 곡사, 지면/적 충돌 폭발, 직접 피해와 범위 피해
- 반복: 발사 후 장전 초기화, 5발 발사 시 `OnExperienceCompleted`

## 확장 지점

`AChongtongLoadingItemActor`는 장전 종류·표현·재생성만 제공한다. 실제 손 그랩은 Blueprint child가
`/Game/XRFramework/Blueprints/BP_GrabComponent`를 구성하여 재사용하므로 별도 총통용 그랩 C++ 클래스가 없다.
총통의 양손 조준은 일반 소품 그랩과 동작이 달라 Feature 전용 `UChongtongAimGripComponent`가 담당한다.
Blueprint child에서 최종 메시를 교체할 수 있다.
총통과 포탄의 NiagaraSystem/SoundBase 속성은 데이터 교체만으로 최종 FX·사운드를 적용한다.
장전 횟수, 완료 발수, 반경, 피해량, 조준 각도는 모두 에디터 속성으로 노출한다.

## 검증

- UnrealBuildTool `SuwonSiegeContestVREditor Win64 Development` 전체 컴파일/링크: 성공
- `SuwonSiegeContestVR.Ongseong.Chongtong.LoadingSequence` 자동화 테스트 추가
- 기존 `BP_ChongtongCannon` parent: `/Script/GF_OngseongCrossbow.ChongtongCannonActor`
- 신규 `BP_ChongtongProjectile` 생성 및 `BP_ChongtongCannon.ProjectileClass` 연결/컴파일/저장
- 신규 `BP_ChongtongPowder`, `BP_ChongtongRammer`, `BP_ChongtongCannonball` 생성
- 각 장전 Blueprint에 Core `BP_GrabComponent` 조립 및 `BP_ChongtongCannon` 클래스 슬롯 연결
- `LV_Ongseong` 배치 인스턴스: `BP_ChongtongCannon_C_1`

## Core 영향 범위

- 기존 Core `BP_GrabComponent` 에셋은 수정하지 않는다.
- 총통 전용 클래스가 Core를 참조하며, Core가 총통 Feature를 참조하지 않는다.
- Core C++ 변경은 `VRPlayerPawn.h/.cpp`의 기능 중립적인 상호작용 계약에 한정한다:
  그립/트리거 입력 분리, `TryGrab`/`TryRelease`를 가진 SceneComponent 탐색, 양손 그랩 허용,
  잡은 컴포넌트로 트리거 전달, 조종 시점 앵커 고정.
- 따라서 런타임 모듈 순환 의존성은 없지만, 다른 작업이 같은 `VRPlayerPawn` 두 파일을 수정하면 소스 병합 충돌 가능성이 있다.
