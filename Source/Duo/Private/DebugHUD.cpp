// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugHUD.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "UObject/ConstructorHelpers.h"

ADebugHUD::ADebugHUD()
{
	// Find font to use.
	static ConstructorHelpers::FObjectFinder<UFont> s_font(TEXT("/Game/Duo/Fonts/Font_CONSOLA"));

	if (s_font.Object)
	{
		m_font = s_font.Object;
		// Set font size.
		m_font->LegacyFontSize = 15;
		// Font style: Bold / Bold Italics / Light/ ItalicS / Regular
		m_font->LegacyFontName = TEXT("BoldItalics");
	}
}

void ADebugHUD::AddText(const TCHAR* title, const FText& value)
{
	RenderInfo(title, value);
}

void ADebugHUD::AddFloat(const TCHAR* title, float value)
{
	RenderInfo(title, FText::AsNumber(value));
}

void ADebugHUD::AddInt(const TCHAR* title, int32 value)
{
	RenderInfo(title, FText::AsNumber(value));
}

void ADebugHUD::AddBool(const TCHAR* title, bool value)
{
	RenderInfo(title, BoolToText(value), (value == true) ? FLinearColor::Green : FLinearColor::Red);
}

void ADebugHUD::AddVector(const TCHAR* title, const FVector& value)
{
	RenderInfo(title, FText::FromString(value.ToString()));
}

void ADebugHUD::AddVector2D(const TCHAR* title, const FVector2D& value)
{
	RenderInfo(title, FText::FromString(value.ToString()));
}

void ADebugHUD::AddRotator(const TCHAR* title, const FRotator& value)
{
	RenderInfo(title, FText::FromString(value.ToString()));
}

void ADebugHUD::DrawHUD()
{
	m_width = m_height = 20.0f;
}

FText ADebugHUD::CStringToText(const TCHAR* textStr)
{
	return FText::FromString(textStr);
}

FText ADebugHUD::BoolToText(bool value)
{
	return FText::FromString((value == true) ? TEXT("true") : TEXT("false"));
}

void ADebugHUD::RenderInfo(const TCHAR* title, const FText& value, const FLinearColor& color)
{
	// Item0
	FCanvasTextItem item0(FVector2D(m_width, m_height), CStringToText(title), m_font, color);
	item0.EnableShadow(FLinearColor(0.0f, 0.0f, 0.0f));
	Canvas->DrawItem(item0);

	// Item1
	FCanvasTextItem item1(FVector2D(m_width + m_horizontalOffset, m_height), value, m_font, color);
	item1.EnableShadow(FLinearColor(0.0f, 0.0f, 0.0f));
	Canvas->DrawItem(item1);

	m_height += m_lineHeight;
}