// Copyright Epic Games, Inc. All Rights Reserved.

#include "Team7DTGameMode.h"
#include "Team7DTPlayerController.h"

ATeam7DTGameMode::ATeam7DTGameMode()
{
	PlayerControllerClass = ATeam7DTPlayerController::StaticClass();
}
