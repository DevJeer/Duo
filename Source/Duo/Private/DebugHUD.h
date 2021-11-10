// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DebugHUD.generated.h"

/**
 * 
 */
UCLASS()
class ADebugHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	ADebugHUD();

	// Add a FText to the HUD.
	void AddText(const TCHAR* title, const FText& value);

	// Add a float to the HUD.
	void AddFloat(const TCHAR* title, float value);

	// Add a int to the HUD.
	void AddInt(const TCHAR* title, int32 value);

	// Add a bool to the HUD.
	void AddBool(const TCHAR* title, bool value);

	// Add a vector to the HUD.
	void AddVector(const TCHAR* title, const FVector& value);

	// Add a vector2d to the HUD.
	void AddVector2D(const TCHAR* title, const FVector2D& value);

	// Add a rotator to the HUD.
	void AddRotator(const TCHAR* title, const FRotator& value);

	// Draw HUD
	virtual void DrawHUD() override;

	float m_horizontalOffset = 150.0f;

private:
	// Convert a TCHAR to FText.
	FText CStringToText(const TCHAR* textStr);
	// Convert a bool to FText.
	FText BoolToText(bool value);
	// Render infos onto the HUD by a row.
	void RenderInfo(const TCHAR* title, const FText& value, const FLinearColor& color = FLinearColor::White);

	float m_width = 0.0f;
	float m_height = 0.0f;
	float m_lineHeight = 25.0f;

	// Font
	UPROPERTY(Transient)
		UFont* m_font = nullptr;
};
