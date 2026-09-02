#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "P3GameInstance.generated.h"

UCLASS()
class CH3_P3_API UP3GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UP3GameInstance();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 TotalScore;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 CurrentLevelIndex;
	UFUNCTION(BlueprintCallable, Category = "GameData")
	void AddToScore(int32 Amount);
};
