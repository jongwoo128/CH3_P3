#include "P3GameState.h"
#include "P3GameInstance.h"
#include "P3Character.h"
#include "P3PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Blueprint/UserWidget.h"

AP3GameState::AP3GameState() {
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;// 전체 초기화
	LevelDuration = 30.0f;
	CurrentLevelIndex = 0;
	MaxLevels = 3;
	CurrentWaveIndex = 0;
	WavesPerLevel = 3;
}

void AP3GameState::BeginPlay() {
	Super::BeginPlay();

	FString CurrentMapName = GetWorld()->GetMapName(); // 현재 로드되어 있는 맵(레벨)의 이름을 문자열로 가져옴.
	if (CurrentMapName.Contains(TEXT("MenuLevel"))) { // 가져온 맵 이름 안에 "MenuLevel"이라는 문자열이 포함되어 있는지 검사
		return;   // 메인 메뉴 레벨이라고 확인되면 아래의 웨이브/타이머가 시작하지 않도록 MenuLevel 한정 함수를 종료하도록 함.
	}	// 같은 BeginPlay()이어도 레벨이 넘어가는 시점에서 새로운 Actor가 자동 생성되는 개념이기에 MenuLevel -> BasicLevel로 넘어가면서 새로 생성된
		// BasicLevel 이후의 Actor들은 이후의 함수들이 자연스럽게 실행됨.

	StartLevel();

	GetWorldTimerManager().SetTimer( // 현재의 월드의 타이머 매니저를 통해 반복 타이머 하나 등록
		HUDUpdateTimerHandle, // 타이머를 식별하거나 제어할 때 쓸 핸들
		this, // 타이머가 실행될 때 호출할 함수가 속한 대상 오브젝트 (GameState 자기 자신)
		&AP3GameState::UpdateHUD, // 타이머 시간이 될 때마다 호출할 함수. (HUD갱신을 위함.) 전역 함수가 아닌 멤버 함수는 혼자서 호출이 불가함으로 멤버 함수의 주소를 통해 가져오기 위함의 &이다.
		0.1f, // 타이머 반복 간격(0.1초 마다)
		true // true면 한번만이 아닌 계속 반복.
	);
}

int32 AP3GameState::GetScore() const {
	return Score;
}

void AP3GameState::AddScore(int32 Amount) {

	if (UGameInstance* GameInstance = GetGameInstance()) {
		UP3GameInstance* P3GameInstance = Cast<UP3GameInstance>(GameInstance);
		if (P3GameInstance) {
			P3GameInstance->AddToScore(Amount);
		}
	}
}

void AP3GameState::StartLevel() {

	if (AP3PlayerController* PlayerController = Cast<AP3PlayerController>(GetWorld()->GetFirstPlayerController())) {
		PlayerController->ShowGameHUD();
	}
	if (UGameInstance* GameInstance = GetGameInstance()) {
		UP3GameInstance* P3GameInstance = Cast<UP3GameInstance>(GameInstance);
		if (P3GameInstance) {
			CurrentLevelIndex = P3GameInstance->CurrentLevelIndex;
		}
	}

	ShowLevelBanner();

	CurrentWaveIndex = 0;
	StartWave(CurrentWaveIndex);
}

void AP3GameState::StartWave(int32 WaveIndex) {
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	const int32 ConfigIndex = CurrentLevelIndex * WavesPerLevel + WaveIndex;
	if (!WaveConfigs.IsValidIndex(ConfigIndex)) {
		UE_LOG(LogTemp, Warning, TEXT("WaveConfigs에 해당 인덱스(%d) 설정이 없습니다. 레벨을 종료합니다."), ConfigIndex);
		EndLevel();
		return;
	}
	const FWaveConfig& Config = WaveConfigs[ConfigIndex];

	const FString WaveStartMsg = FString::Printf(TEXT("Wave %d 시작!"), WaveIndex + 1);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *WaveStartMsg);
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, WaveStartMsg);
	}

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	for (int32 i = 0; i < Config.ItemSpawnCount; i++) {
		if (FoundVolumes.Num() > 0) {
			ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
			if (SpawnVolume) {
				AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
				if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass())) {
					SpawnedCoinCount++;
				}
			}
		}
	}

	GetWorldTimerManager().SetTimer(
		LevelTimerHandle,
		this,
		&AP3GameState::OnWaveTimeUp,
		Config.WaveDuration,
		false
	);
}

void AP3GameState::OnWaveTimeUp() {
	AdvanceWave();
}

void AP3GameState::OnCoinCollected() {
	CollectedCoinCount++;
	UE_LOG(LogTemp, Warning, TEXT("Coin Collected: %d / %d"),
		CollectedCoinCount,
		SpawnedCoinCount);
	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount) {
		AdvanceWave();
	}
}

void AP3GameState::AdvanceWave() {
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);

	CurrentWaveIndex++;
	if (CurrentWaveIndex >= WavesPerLevel) {
		EndLevel();
	}
	else {
		StartWave(CurrentWaveIndex);
		ShowWaveBanner();
	}
}

void AP3GameState::EndLevel() {
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);
	
	if (UGameInstance* GameInstance = GetGameInstance()) {
		UP3GameInstance* P3GameInstance = Cast<UP3GameInstance>(GameInstance);
		if (P3GameInstance) {
			AddScore(Score);
			CurrentLevelIndex++;
			P3GameInstance->CurrentLevelIndex = CurrentLevelIndex;
		}
	}

	if (CurrentLevelIndex >= MaxLevels) {
		OnGameOver();
		return;
	}
	
	if (LevelMapNames.IsValidIndex(CurrentLevelIndex)) {
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	}
	else {
		OnGameOver();
	}
}

/*
void AP3GameState::OnGameOver() {
	if (AP3PlayerController* PlayerController = Cast<AP3PlayerController>(GetWorld()->GetFirstPlayerController())) {
		if (AP3PlayerController* P3PlayerCtroller = Cast<AP3PlayerController>(PlayerController)) {
			P3PlayerCtroller->SetPause(true);
			P3PlayerCtroller->ShowMainMenu(true);
		}
	}
}
*/
void AP3GameState::OnGameOver() {
	if (bIsGameOver) {
		return;
	}
	bIsGameOver = true;

	GetWorldTimerManager().ClearTimer(LevelTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveBannerTimerHandle);
	GetWorldTimerManager().ClearTimer(LevelBannerTimerHandle);
	GetWorldTimerManager().ClearTimer(HUDUpdateTimerHandle);

	if (AP3PlayerController* P3PlayerController = Cast<AP3PlayerController>(GetWorld()->GetFirstPlayerController())) {
		P3PlayerController->SetPause(true);
		P3PlayerController->ShowMainMenu(true);
	}
}

void AP3GameState::UpdateHUD() {
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController()) {
		if (AP3PlayerController* P3PlayerController = Cast<AP3PlayerController>(PlayerController)) {
			if (UUserWidget* HUDWidget = P3PlayerController->GetHUDWidget()) {

				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time")))) {
					float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
				}
				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score")))) {
					if (UGameInstance* GameInstance = GetGameInstance()) {
						UP3GameInstance* P3GameInstance = Cast<UP3GameInstance>(GameInstance);
						if (P3GameInstance) {
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), P3GameInstance->TotalScore)));
						}
					}
				}
				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level")))) {
					LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
				}
				if (UTextBlock* WaveText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Wave")))) {
					WaveText->SetText(FText::FromString(FString::Printf(TEXT("Wave: %d / %d"), CurrentWaveIndex + 1, WavesPerLevel)));
				}

				if (AP3Character* P3Character = Cast<AP3Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))) {
					float CurrentHealth = P3Character->GetHealth();
					float MaxHealth = P3Character->GetMaxHealth();

					if (UTextBlock* HealthText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Health")))) {
						HealthText->SetText(FText::FromString(
							FString::Printf(TEXT("HP : %.0f / %.0f"), CurrentHealth, MaxHealth)));
					}

					if (UProgressBar* HealthBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("HealthBar")))) {
						float Percent = FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f);
						HealthBar->SetPercent(Percent);

						FLinearColor BarColor;
						if (Percent > 0.5f)       BarColor = FLinearColor(0.133f, 0.773f, 0.369f); // #22C55E
						else if (Percent > 0.25f) BarColor = FLinearColor(0.961f, 0.651f, 0.137f); // #F5A623
						else                      BarColor = FLinearColor(0.898f, 0.282f, 0.302f); // #E5484D

						HealthBar->SetFillColorAndOpacity(BarColor);
					}
				}
			}
		}
	}
}

void AP3GameState::ShowLevelBanner()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController()) {
		if (AP3PlayerController* P3PlayerController = Cast<AP3PlayerController>(PlayerController)) {
			if (UUserWidget* HUDWidget = P3PlayerController->GetHUDWidget()) {

				if (UTextBlock* LevelBannerText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("LevelBannerText")))) {
					LevelBannerText->SetText(FText::FromString(
						FString::Printf(TEXT("Level %d"), CurrentLevelIndex + 1)));
				}

				if (UBorder* LevelBanner = Cast<UBorder>(HUDWidget->GetWidgetFromName(TEXT("LevelBanner")))) {
					LevelBanner->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
	}

	GetWorldTimerManager().ClearTimer(LevelBannerTimerHandle);
	GetWorldTimerManager().SetTimer(LevelBannerTimerHandle, this, &AP3GameState::HideLevelBanner, 1.8f, false);
}

void AP3GameState::HideLevelBanner()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController()) {
		if (AP3PlayerController* P3PlayerController = Cast<AP3PlayerController>(PlayerController)) {
			if (UUserWidget* HUDWidget = P3PlayerController->GetHUDWidget()) {
				if (UBorder* LevelBanner = Cast<UBorder>(HUDWidget->GetWidgetFromName(TEXT("LevelBanner")))) {
					LevelBanner->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}
}

void AP3GameState::ShowWaveBanner()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController()) {
		if (AP3PlayerController* P3PlayerController = Cast<AP3PlayerController>(PlayerController)) {
			if (UUserWidget* HUDWidget = P3PlayerController->GetHUDWidget()) {

				if (UTextBlock* WaveBannerText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("WaveBannerText")))) {
					WaveBannerText->SetText(FText::FromString(
						FString::Printf(TEXT("Wave %d"), CurrentWaveIndex + 1)));
				}

				if (UBorder* WaveBanner = Cast<UBorder>(HUDWidget->GetWidgetFromName(TEXT("WaveBanner")))) {
					WaveBanner->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
	}

	GetWorldTimerManager().ClearTimer(WaveBannerTimerHandle);
	GetWorldTimerManager().SetTimer(WaveBannerTimerHandle, this, &AP3GameState::HideWaveBanner, 1.8f, false);
}

void AP3GameState::HideWaveBanner()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController()) {
		if (AP3PlayerController* P3PlayerController = Cast<AP3PlayerController>(PlayerController)) {
			if (UUserWidget* HUDWidget = P3PlayerController->GetHUDWidget()) {
				if (UBorder* WaveBanner = Cast<UBorder>(HUDWidget->GetWidgetFromName(TEXT("WaveBanner")))) {
					WaveBanner->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}
}