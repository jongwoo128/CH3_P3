#include "P3GameInstance.h"

UP3GameInstance::UP3GameInstance()
{
	TotalScore = 0;
	CurrentLevelIndex = 0;
}

void UP3GameInstance::AddToScore(int32 Amount) {
	TotalScore += Amount;
	UE_LOG(LogTemp, Warning, TEXT("Total Score Updated: %d"), TotalScore);
}