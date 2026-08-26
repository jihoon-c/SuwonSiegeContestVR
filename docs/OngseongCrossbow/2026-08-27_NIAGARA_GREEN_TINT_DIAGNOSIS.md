# 나이아가라 연두색 오염 진단

## 확인된 사실 (에셋에서 직접 검증)

`ExplosionEffect` 기본값인 `/Game/NiagaraExamples/FX_Weapons/Impacts/NS_Impact_Concrete`는
Epic 샘플 콘텐츠이고, 그 emitter들이 **Light Renderer와 Decal Renderer를 함께 들고 있다.**

```text
NE_Impact.uasset              : Decal  Light  Mesh  Sprite
NE_Impact_LightDecal.uasset   : Decal  Light  Sprite
NE_Impact_Mesh.uasset         : Decal  Light  Mesh  Sprite
NE_Impact_MeshAndSprite.uasset: Decal  Light  Mesh  Sprite
NE_Impact_Sprite.uasset       : Decal  Light  Mesh  Sprite
NE_Impact_SecondarySprite     : Sprite (only)
```

이 시스템 안에서 **다른 지오메트리의 픽셀을 바꿀 수 있는 요소는 Light와 Decal 둘뿐이다.**
스프라이트/리본/메시 렌더러는 자기 파티클만 그린다. 따라서 랜드스케이프와 스켈레탈 메시가
착탄 지점 주변에서만 물드는 현상의 출처는 이 둘 중 하나다.

* **Light Renderer** — 랜드스케이프와 스켈레탈 메시 양쪽에 영향을 준다. 범위가 국소적이고
  효과가 끝나면 사라진다. 증상과 가장 잘 맞는다.
* **Decal Renderer** — 랜드스케이프에는 남지만 스켈레탈 메시에는 기본적으로 투영되지 않는다.
  `NS_Impact_Concrete`의 컴파일된 셰이더에 `LandscapeCommon.ush`와 `VirtualTextureCommon.ush`가
  포함돼 있어, 데칼이 랜드스케이프의 Runtime Virtual Texture 경로를 타고 있다.

이 프로젝트는 이 부류의 버그를 이미 한 번 겪었다. `ChongtongProjectileActor.cpp`에 남아 있는 주석:

> Do not use NS_Dirt_Explosion_Medium here: its sample post-process emitter can tint the whole
> scene lime green, including terrain and skeletal enemies.

즉 같은 샘플 팩의 다른 시스템에서 동일 증상이 이미 보고됐다.

## 머즐 플래시는 별개 원인이다

`NS_MuzzleFlash`의 emitter들에는 **Light도 Decal도 없다.**

```text
NE_MuzzleFlash_Base       : Ribbon  Sprite
NE_MuzzleFlash_Smoke      : Sprite
NE_MuzzleFlash_Sparks_Base: Sprite
NE_BulletShells           : Mesh  Sprite
```

따라서 머즐 쪽 오염은 착탄 쪽과 원인이 다르다. 남은 후보는 샘플 머티리얼 인스턴스
(`MI_Flipbook_Pyro_Muzzle`, `MI_MuzzleFlash_Sphere`, `MI_Stylized_Sparkes`,
`MI_Flipbook_Smoke_Muzzle`)의 블렌드 모드/셰이딩 설정이다. 이건 에디터에서 확인해야 한다.

## 확정 절차 (PIE 콘솔, 각각 한 줄)

착탄 오염의 범인을 한 번에 가른다.

```
showflag.Decals 0
```

이걸로 사라지면 Decal Renderer.

```
showflag.DirectLighting 0
```

이걸로 사라지면 Light Renderer.

머즐 오염은 위 두 개로 안 사라지므로, `NS_MuzzleFlash`를 열어 emitter를 하나씩 꺼 보며
어느 스프라이트 머티리얼인지 좁힌다.

## 권장 수정

샘플 콘텐츠를 전투 FX로 계속 쓰지 않는 것이 근본 대책이다.

1. `NS_Impact_Concrete`를 `GF_OngseongCrossbow/Asset/VFX/NS_ChongtongImpact`로 **복제**한다.
   (`/Game/NiagaraExamples`를 직접 수정하면 엔진 샘플을 쓰는 다른 곳이 같이 바뀐다.)
2. 복제본에서 **Light Renderer와 Decal Renderer를 삭제**한다. 스프라이트/메시만 남긴다.
3. `BP_ChongtongProjectile` Class Defaults의 `Explosion Effect`를 복제본으로 교체한다.
4. 머즐도 같은 방식으로 `NS_MuzzleFlash`를 복제해 문제 emitter를 제거하고
   `BP_ChongtongCannon`의 `Muzzle Effect`를 교체한다.

## 이번 커밋에서 되돌린 것

`ExplosionEffectScale` 기본값을 `6.0` → `2.0`으로 낮췄다.

Niagara Light Renderer의 반경은 시스템 스케일을 따라간다. 착탄 파티클이 안 보이는 문제를
고치려고 스케일을 6배로 올린 것이 **빛 반경도 6배로 키워** 원래 미묘하던 오염을
눈에 띄는 초록 물듦으로 키웠을 가능성이 높다.

파티클 크기는 위 1~3번으로 Light를 제거한 뒤에 다시 올려야 한다.
