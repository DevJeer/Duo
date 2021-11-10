// Fill out your copyright notice in the Description page of Project Settings.


#include "Countdown.h"
#include "Components/TextRenderComponent.h"

// Sets default values
ACountdown::ACountdown()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CountdownText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CountdownNumber"));
	CountdownText->SetHorizontalAlignment(EHTA_Center);
	CountdownText->SetWorldSize(150.0f);
	RootComponent = CountdownText;
	CountdownTime = 3;
	CountdownTimeFloat = 4.0f;
}

// Called when the game starts or when spawned
void ACountdown::BeginPlay()
{
	Super::BeginPlay();
	
	// At beginning, set time text.
	//UpdateTimeDisplay();
	UpdateTimeDisplayByFloat();
	//GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ACountdown::AdvanceTimer, 1.0f, true);
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ACountdown::HandleTimeChange, 0.1f, true);
}

// Called every frame
void ACountdown::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACountdown::UpdateTimeDisplay()
{
	CountdownText->SetText(FText::FromString(FString::FromInt(FMath::Max(CountdownTime, 0))));
}

void ACountdown::UpdateTimeDisplayByFloat()
{
	CountdownText->SetText(FText::FromString(FString::SanitizeFloat(FMath::Max(CountdownTimeFloat, .0f))));
}

void ACountdown::AdvanceTimer()
{
	--CountdownTime;
	UpdateTimeDisplay();
	if (CountdownTime < 1)
	{
		// We're done counting down, so stop running the timer.
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		CountdownHasFinished();
	}
}

void ACountdown::HandleTimeChange()
{
	
	CountdownTimeFloat -= 0.1f;
	CountdownTimeFloat = (int)(CountdownTimeFloat * 10) / 10.0f;
	UpdateTimeDisplayByFloat();
	if (CountdownTimeFloat < 0.0f)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		CountdownHasFinished();
	}
}

void ACountdown::CountdownHasFinished_Implementation()
{
	// Change to a special readout.
	CountdownText->SetText(FText::FromString(TEXT("GO!")));

	// Reset. Enable loop.
	CountdownTime = 4;
	CountdownTimeFloat = 4.0f;
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ACountdown::HandleTimeChange, 0.1f, true);
}

