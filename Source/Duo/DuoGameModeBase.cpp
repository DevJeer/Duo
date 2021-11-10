// Copyright Epic Games, Inc. All Rights Reserved.


#include "DuoGameModeBase.h"
#include "Private/PlayerHUD.h"

ADuoGameModeBase::ADuoGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup game mode default value.
	HUDClass = APlayerHUD::StaticClass();
	static ConstructorHelpers::FObjectFinder<UClass> flymodeCharacter(TEXT("/Game/Duo/Blueprints/Player/BP_FlyModeCharacter.BP_FlyModeCharacter_C"));
	if (flymodeCharacter.Object)
	{
		DefaultPawnClass = flymodeCharacter.Object;
	}
}
