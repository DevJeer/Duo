// Fill out your copyright notice in the Description page of Project Settings.


#include "TestWidget.h"
#include "UMG/Public/Components/Button.h"
#include "UMG/Public/Components/TextBlock.h"

bool UTestWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	m_btnAddOne = Cast<UButton>(GetWidgetFromName("Button_AddOne"));
	m_tbNumber = Cast<UTextBlock>(GetWidgetFromName("TextBlock_Number"));
	m_btnPos = FVector2D(.0f, .0f);
	m_btnAddOneTransform = m_btnAddOne->RenderTransform;
	m_btnAddOneTransform.Translation = m_btnPos;
	m_btnAddOne->SetRenderTransform(m_btnAddOneTransform);
	return true;
}

void UTestWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (m_btnAddOne)
	{
		m_btnAddOne->OnClicked.AddDynamic(this, &UTestWidget::OnAddOneClicked);
	}
	m_number = 0;
}

void UTestWidget::OnAddOneClicked()
{
	if (m_tbNumber)
	{
		m_number++;
		m_btnPos += FVector2D(10.0f, 5.0f);
		m_btnAddOneTransform.Translation = m_btnPos;
		m_tbNumber->SetText(FText::AsNumber(m_number));
		m_btnAddOne->SetRenderTransform(m_btnAddOneTransform);
	}
}
