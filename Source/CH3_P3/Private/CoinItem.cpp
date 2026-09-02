#include "CoinItem.h"
#include "Engine/World.h"
#include "P3GameState.h"

ACoinItem::ACoinItem() {
	PointValue = 0;
	ItemType = "DefaultCoin";
}

void ACoinItem::ActivateItem(AActor* Activator) {
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player")) {

		if (UWorld* World = GetWorld()) {
			if (AP3GameState* GameState = World->GetGameState<AP3GameState>()) {
				GameState->AddScore(PointValue);
				GameState->OnCoinCollected();
			}
		}
		
		DestroyItem();
	}
}

