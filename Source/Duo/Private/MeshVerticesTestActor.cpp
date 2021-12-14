// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshVerticesTestActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Rendering/PositionVertexBuffer.h"
#include "StaticMeshResources.h"
#include "DrawDebugHelpers.h"

DECLARE_LOG_CATEGORY_CLASS(MeshVerticesTest, Log, All)

// Sets default values
AMeshVerticesTestActor::AMeshVerticesTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	smComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	smComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AMeshVerticesTestActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMeshVerticesTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<FVector> points = MeshData(smComp);
	for (int i = 0; i < points.Num(); i++)
	{
		DrawDebugPoint(this->GetWorld(), points[i], 10.0f, FColor::Green);
	}
}

TArray<FVector> AMeshVerticesTestActor::MeshData(const UStaticMeshComponent* StaticMeshComponent)
{
	TArray<FVector> vertices = TArray<FVector>();

	if(!IsValidLowLevel())
		return vertices;
	if(!StaticMeshComponent)
		return vertices;
	if(!StaticMeshComponent->GetStaticMesh())
		return vertices;
	if(!StaticMeshComponent->GetStaticMesh()->GetRenderData())
		return vertices;
	if (StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources.Num() > 0)
	{
		FPositionVertexBuffer* vertexBuffer = &StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer;
		if (vertexBuffer)
		{
			const int32 vertexCount = vertexBuffer->GetNumVertices();
			for (int32 index = 0; index < vertexCount; index++)
			{
				const FVector worldSpaceVertexLocation = GetActorLocation() + GetTransform().TransformVector(vertexBuffer->VertexPosition(index));
				vertices.Add(worldSpaceVertexLocation);
			}
		}
	}
	return vertices;
}

