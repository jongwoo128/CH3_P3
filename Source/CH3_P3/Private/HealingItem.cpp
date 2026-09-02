#include "HealingItem.h"
#include "P3Character.h"

AHealingItem::AHealingItem() {
	HealAmount = 20.0f;
	ItemType = "Healing";
}

void AHealingItem::ActivateItem(AActor* Activator) {

	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player")) {
		
		if (AP3Character* PlayerCharacter = Cast<AP3Character>(Activator)) {
			PlayerCharacter->AddHealth(HealAmount);
		}
		DestroyItem();
	}
}