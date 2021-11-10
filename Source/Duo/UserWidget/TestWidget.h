// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestWidget.generated.h"

/**
 * 
 */
UCLASS()
class DUO_API UTestWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;
	virtual void NativeConstruct();

	UFUNCTION()
	void OnAddOneClicked();

protected:
	class UButton* m_btnAddOne;
	class UTextBlock* m_tbNumber;
	int32 m_number;

private:
	FVector2D m_btnPos;

	FWidgetTransform m_btnAddOneTransform;
};
