
#include "P3PlayerController.h"
#include "P3GameState.h"
#include "P3GameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/TextBlock.h"


AP3PlayerController::AP3PlayerController()
	: InputMappingContext(nullptr),
	MoveAction(nullptr),
	JumpAction(nullptr),
	LookAction(nullptr),
	SprintAction(nullptr),
	HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr)
{}

void AP3PlayerController::BeginPlay() {
	Super::BeginPlay();

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer()) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) {
			if (InputMappingContext) {
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("MenuLevel")) {
		ShowMainMenu(false);
	}
}

UUserWidget* AP3PlayerController::GetHUDWidget() const {
	return HUDWidgetInstance;
}

void AP3PlayerController::ShowMainMenu(bool bIsRestart) {
	if (HUDWidgetInstance) {
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance) {
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (MainMenuWidgetClass) {
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetClass) {
			MainMenuWidgetInstance->AddToViewport();

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly()); // 뒤에 화면이 마우스 커서에 따라 시점이 움직이는 것을 막고 UI창에 한해서만 향할 수 있도록 해줌.
		}
	}

	if (UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText")))) {
		if (bIsRestart) {
			ButtonText->SetText(FText::FromString(TEXT("Restart")));
		}
		else {
			ButtonText->SetText(FText::FromString(TEXT("Start")));
		}
	}

	if (UWidget* MainMenuButton = MainMenuWidgetInstance->GetWidgetFromName(TEXT("MainMenuButton"))) {
		MainMenuButton->SetVisibility(bIsRestart ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (UWidget* QuitButton1 = MainMenuWidgetInstance->GetWidgetFromName(TEXT("QuitButton1"))) {
		QuitButton1->SetVisibility(bIsRestart ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (UWidget* QuitButton2 = MainMenuWidgetInstance->GetWidgetFromName(TEXT("QuitButton2"))) {
		QuitButton2->SetVisibility(bIsRestart ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (bIsRestart) {
		UFunction* PlayAnimFunc = MainMenuWidgetInstance->FindFunction(FName("PlayGameOverAnim"));
		if (PlayAnimFunc) {
			MainMenuWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
		}

		if (UTextBlock* TotalScoreText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName("TotalScoreText"))) {
			if (UP3GameInstance* P3GameInstance = Cast<UP3GameInstance>(UGameplayStatics::GetGameInstance(this))) {
				TotalScoreText->SetText(FText::FromString(
					FString::Printf(TEXT("Total Score: %d"), P3GameInstance->TotalScore)));
			}
		}
	}
}

void AP3PlayerController::ShowGameHUD() {
	if (HUDWidgetInstance) {
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance) {
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (HUDWidgetClass) {
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance) {
			HUDWidgetInstance->AddToViewport();

			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly()); // 뒤에 화면이 마우스 커서에 따라 시점이 움직이는 것을 막고 UI창에 한해서만 향할 수 있도록 해줌.
		}
	}

	AP3GameState* P3GameState = GetWorld() ? GetWorld()->GetGameState<AP3GameState>() : nullptr;
	if (P3GameState) {
		P3GameState->UpdateHUD();
	}
}

void AP3PlayerController::StartGame() {
	if (UP3GameInstance* P3GameInstance = Cast<UP3GameInstance>(UGameplayStatics::GetGameInstance(this))) {
		P3GameInstance->CurrentLevelIndex = 0;
		P3GameInstance->TotalScore = 0;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
	SetPause(false);
}

void AP3PlayerController::QuitGame() {
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AP3PlayerController::GoToMainMenu()
{
	if (UP3GameInstance* P3GameInstance = Cast<UP3GameInstance>(UGameplayStatics::GetGameInstance(this))) {
		P3GameInstance->CurrentLevelIndex = 0;
		P3GameInstance->TotalScore = 0;
	}
	SetPause(false);
	UGameplayStatics::OpenLevel(GetWorld(), FName("MenuLevel"));
}