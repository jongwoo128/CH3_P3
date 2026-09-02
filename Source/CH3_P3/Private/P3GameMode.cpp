#include "P3GameMode.h"
#include "P3Character.h"
#include "P3PlayerController.h"
#include "P3GameState.h"

AP3GameMode::AP3GameMode() {
	DefaultPawnClass = AP3Character::StaticClass();
	PlayerControllerClass = AP3PlayerController::StaticClass();
	GameStateClass = AP3GameState::StaticClass();
}