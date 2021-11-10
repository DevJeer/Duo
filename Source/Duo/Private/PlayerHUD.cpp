// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "Components/CapsuleComponent.h"
#include "FlyModeCharacter.h"
#include "Kismet/GameplayStatics.h"

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();


	/**************************FlyModeCharacterDebugHUD******************************/
	AFlyModeCharacter* flyModeCharacter = Cast<AFlyModeCharacter>(GetOwningPawn());
	if (flyModeCharacter)
	{
		AddText(TEXT("FlyMode"), FText::FromString(FString(TEXT(""))));
		AddVector(TEXT("Forward dir"), flyModeCharacter->m_forwardDir);
		AddVector(TEXT("Right dir"), flyModeCharacter->m_rightDir);
		AddVector(TEXT("Up dir"), flyModeCharacter->m_upDir);
		//AddFloat(TEXT("ScrollWheel"), flyModeCharacter->scrollWheelInputValue);
		//AddVector(TEXT("Anchor point dir"), flyModeCharacter->zoomAnchorDir);
	}
}