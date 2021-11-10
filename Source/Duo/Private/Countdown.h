// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Countdown.generated.h"

UCLASS()
class ACountdown : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACountdown();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	int32 CountdownTime;

	UPROPERTY(EditAnywhere)
	float CountdownTimeFloat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTextRenderComponent* CountdownText;

	void UpdateTimeDisplay();

	void UpdateTimeDisplayByFloat();

	void AdvanceTimer();

	void HandleTimeChange();

	UFUNCTION(BlueprintNativeEvent)
	void CountdownHasFinished();
	
	virtual void CountdownHasFinished_Implementation();

	FTimerHandle CountdownTimerHandle;
};
