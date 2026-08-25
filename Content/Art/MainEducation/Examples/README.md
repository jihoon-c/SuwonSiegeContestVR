# Main Education Example Images

AI로 생성한 편집·배치 확인용 예시 이미지다. 최종 고증 이미지가 준비되면
`DA_Scenario_MainEducation > 01 Editor Flow > Stage > Steps > Content > Image`에서 교체한다.

| PNG / Texture | 기본 사용 위치 |
|---|---|
| `T_MainEdu_Overview_Example` | 수원화성 전체 구조, 최종 정리 |
| `T_MainEdu_WallDefense_Example` | 성벽 방어와 화약무기 |
| `T_MainEdu_GongsimdonCutaway_Example` | 공심돈 실제 모습/단면 |
| `T_MainEdu_OngseongPlan_Example` | 옹성 평면/비교 |
| `T_MainEdu_PulleyComparison_Example` | 녹로·거중기 구조와 비교 |

## 제작 방식

- 도구: Codex built-in `image_gen`
- 용도 분류: `scientific-educational`
- 공통 조건: 16:9 VR 교육 화면, 이미지 안 텍스트/로고/워터마크 없음, UI Callout 여백 확보

## 최종 Prompt Set

공통 Prompt:

```text
Use case: scientific-educational
Asset type: Unreal VR educational presentation image, 16:9 landscape
Style: polished museum educational illustration with realistic materials and diagram clarity
Constraints: historically plausible late-Joseon Korean setting; clean room for Unreal UI callouts;
no text, labels, arrows, logos, or watermark
```

개별 Primary Request:

1. 수원화성의 연속 성벽, 주요 성문, 관찰 시설과 성곽 전체 체계를 보여주는 사선 항공 조감도
2. 성 밖의 접근군과 성벽 위 조총·화포·신기전 참고 장비를 구분해 보여주는 성벽 방어 장면
3. 내부 층, 계단, 관찰구, 공격구, 성벽 연결을 드러낸 공심돈 건축 단면
4. 성문 바깥을 감싸는 반원형 옹성과 제한된 접근 경로를 보여주는 직교 조감 평면
5. 성벽 물자 이동용 녹로와 축성용 다중 도르래 거중기를 나란히 보여주는 비교 이미지

## 최종 적용 전 주의

- 교육용 예시이므로 문화재 전문가의 구조·복식·무기·기계 고증을 거쳐야 한다.
- 설명 문구와 화살표는 이미지에 합성하지 않고 Unreal Widget에서 표시한다.
- 새 이미지로 교체해도 Content ID와 Scenario Step ID는 변경하지 않는다.
