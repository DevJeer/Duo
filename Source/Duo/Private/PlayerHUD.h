// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebugHUD.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class APlayerHUD : public ADebugHUD
{
	GENERATED_BODY()
	
protected:
	// Draw the HUD.
	virtual void DrawHUD() override;
};
