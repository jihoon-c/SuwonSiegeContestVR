#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MainLevelIntroActor.generated.h"

class APlayerController;
class AMainEducationScenarioManagerActor;
class APawn;
class UArrowComponent;
class UCameraComponent;
class UFont;
class USceneComponent;
class UTextRenderComponent;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EMainLevelIntroPhase : uint8
{
	Idle,
	MoveToTitle,
	ShowTitle,
	FadeTitle,
	MoveToPlayer,
	Complete
};

/**
 * Place one actor in the Main level to author the opening camera path and title.
 * The three arrow components are deliberately movable in the level editor.
 */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AMainLevelIntroActor : public AActor
{
	GENERATED_BODY()

public:
	AMainLevelIntroActor();

	/** Starts the overview -> title -> player sequence. */
	UFUNCTION(BlueprintCallable, Category = "Main Intro")
	void PlayIntro();

	/** Finishes the visual intro immediately and begins the configured education scenario. */
	UFUNCTION(BlueprintCallable, Category = "Main Intro")
	void SkipIntro();

	UFUNCTION(BlueprintPure, Category = "Main Intro")
	EMainLevelIntroPhase GetIntroPhase() const { return IntroPhase; }

	/** Rebuilds the title preview at TitleAnchor without moving any authored actor or anchor. */
	UFUNCTION(CallInEditor, Category = "Main Intro|Editor Preview")
	void RefreshTitlePreviewInEditor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Intro|Anchors")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Initial wide establishing view. Move and rotate this arrow in the level editor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Intro|Anchors")
	TObjectPtr<UArrowComponent> OverviewAnchor;

	/** View held while the title is visible. Defaults to the player view, but can be authored independently. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Intro|Anchors")
	TObjectPtr<UArrowComponent> TitleAnchor;

	/** Final VR player view at the fortress gate. Move and rotate this arrow in the level editor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Intro|Anchors")
	TObjectPtr<UArrowComponent> PlayerAnchor;

	/** The Main scenario manager that receives the start request after the title fades. Empty = first manager found in this level. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Main Intro|Scenario")
	TObjectPtr<AMainEducationScenarioManagerActor> TargetEducationManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float OverviewToTitleDuration = 5.0f;

	/** Required title exposure time; default is three seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float TitleDisplayDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float TitleFadeDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float TitleToPlayerDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Text")
	FText TitleText = FText::FromString(TEXT("수원화성 VR 교육"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Text", meta = (MultiLine = "true"))
	FText SubtitleText = FText::FromString(TEXT("정조의 꿈과 과학적인 방어 체계를 탐험합니다"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Text")
	FLinearColor TitleColor = FLinearColor::White;

	/** Gmarket Sans Bold is assigned in L_Main. Leave empty to use the engine default font. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Text")
	TObjectPtr<UFont> TitleFont;

	/** Gmarket Sans Medium is assigned in L_Main. Leave empty to share Title Font. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Text")
	TObjectPtr<UFont> SubtitleFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Size", meta = (ClampMin = "1.0", DeprecatedProperty, DeprecationMessage = "Use Title Font Size and Title Widget World Scale."))
	float TitleWorldSize = 30.0f;

	/** UMG glyph size. Runtime composite fonts such as GmarketSans render through this path. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Size", meta = (ClampMin = "1", ClampMax = "200"))
	int32 TitleFontSize = 72;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Size", meta = (ClampMin = "1", ClampMax = "200"))
	int32 SubtitleFontSize = 34;

	/** World scale of the UMG title planes. Increase this if the title is too small in the headset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Size", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float TitleWidgetWorldScale = 0.12f;

	/** Camera-local position of the title; X is forward, Y is sideways, Z is vertical. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Position")
	FVector TitleOffset = FVector(200.0f, 0.0f, 25.0f);

	/** Camera-local position of the subtitle; X is forward, Y is sideways, Z is vertical. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Title|Position")
	FVector SubtitleOffset = FVector(200.0f, 0.0f, -20.0f);

	/** Shows the title at TitleAnchor in the editor viewport. It is never forced visible during gameplay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Editor Preview")
	bool bShowEditorTitlePreview = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Input")
	bool bLockPlayerInputDuringIntro = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Intro|Playback")
	bool bPlayOnBeginPlay = true;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void BeginIntroWhenReady();
	void MovePawnToAnchor(const UArrowComponent* TargetAnchor, float Alpha);
	void ShowTitle(float Opacity);
	void HideTitle();
	void RefreshEditorTitlePreview();
	void FinishIntro();
	void SetInputLocked(bool bLocked);
	AMainEducationScenarioManagerActor* ResolveEducationManager() const;

	UPROPERTY(VisibleAnywhere, Category = "Main Intro|Title")
	TObjectPtr<UTextRenderComponent> TitleTextRender;

	UPROPERTY(VisibleAnywhere, Category = "Main Intro|Title")
	TObjectPtr<UTextRenderComponent> SubtitleTextRender;

	/** Actual runtime title renderer. This deliberately bypasses DefaultTextMaterialOpaque. */
	UPROPERTY(VisibleAnywhere, Category = "Main Intro|Title")
	TObjectPtr<UWidgetComponent> TitleWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "Main Intro|Title")
	TObjectPtr<UWidgetComponent> SubtitleWidgetComponent;

	TWeakObjectPtr<APawn> IntroPawn;
	TWeakObjectPtr<APlayerController> IntroPlayerController;
	FTransform MovementStartTransform;
	float PhaseElapsed = 0.0f;
	EMainLevelIntroPhase IntroPhase = EMainLevelIntroPhase::Idle;
	bool bInputLocked = false;
};
