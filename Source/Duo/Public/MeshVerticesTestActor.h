// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshVerticesTestActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class DUO_API AMeshVerticesTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeshVerticesTestActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	TArray<FVector> MeshData(const UStaticMeshComponent* StaticMeshComponent);

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* smComp = nullptr;
};
