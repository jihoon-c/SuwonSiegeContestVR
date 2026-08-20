# Gameplay 문서

Shared Gameplay 계층의 구현 상태와 후속 작업을 관리하는 문서 모음이다.

- `plans/`: 아직 완료되지 않은 구현·통합·검증 작업
- `completed/`: 실제로 완료되고 검증된 Shared Gameplay 작업
- `STATUS.md`: 현재 코드 기준의 단일 현황

## 범위

Shared Gameplay는 Core와 Game Feature 사이의 재사용 계층이다.

```text
GF_OngseongCrossbow / GF_Gongsimdon / GF_Singijeon
                         ↓
Shared Gameplay: Character / Combat / AI / Pooling
                         ↓
Core: VR / Scenario / Experience
```

체험 고유의 Wave, 총통, 충차, 성문, 탐지 연출, Behavior Tree/StateTree Asset은 각 Game Feature가 소유한다.
Shared Gameplay는 특정 Feature를 직접 참조하지 않는다.
