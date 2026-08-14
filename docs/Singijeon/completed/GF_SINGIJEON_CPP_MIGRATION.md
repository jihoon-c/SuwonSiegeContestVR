# GF_Singijeon C++ 기능 통합

## 작업

별도 `SingijeonInteraction` 런타임 플러그인의 기능을 기존 Game Feature Plugin `GF_Singijeon`으로 통합했다.

## 구현 내용

- `GF_Singijeon`에 `GF_Singijeon` Runtime C++ 모듈 추가
- 화살 장전, 도화선 점화, 순차 발사, 기본 투사체, 횃불, 양손 운반 코드 이전
- 공개 심볼 매크로를 `GF_SINGIJEON_API`로 변경
- 기존 `/Script/SingijeonInteraction` 및 초기 `/Script/SuwonSiegeContestVR` 클래스 참조를 위한 Core Redirect 추가
- 상호작용 문서를 `docs/Singijeon/specs/VR_INTERACTION.md`로 이동
- 중복 `SingijeonInteraction` 플러그인 제거

## 주요 결정 사항

신기전 화차 상호작용은 다른 체험에서 재사용하는 공용 전투 기반이 아니라 신기전 고유 기능이므로 `GF_Singijeon`이 소유한다. 범용 Damage, Health, Faction 및 Projectile 기반은 계속 Shared Gameplay의 책임으로 남긴다.

## 검증

- UnrealHeaderTool 및 C++ Editor 빌드
- 기존 모듈명 잔여 참조 검색
- Game Feature descriptor와 `GameFeatureData` 유지 확인

## 후속 작업

- Blueprint/XR Grab 연결
- 테스트 레벨 플레이 검증
- Shared Damage 및 ExperienceSubsystem 연동
