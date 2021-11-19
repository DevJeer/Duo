// Fill out your copyright notice in the Description page of Project Settings.

#include "SplineActor.h"
#include "Components/SplineComponent.h"

// Sets default values
ASplineActor::ASplineActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_splineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	if (m_splineComponent)
	{
		SetRootComponent(m_splineComponent);
		m_splineComponent->EditorTangentColor = FLinearColor(.0f, .5f, .0f);
	}
}


void ASplineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Init
	if (m_splineComponent && m_splineMeshMap.Num() > 0)
	{
		// Lookup all pertinent values.
		FSplineMeshDetails* startMeshDetails = nullptr;
		if (m_splineMeshMap.Contains(ESplineMeshType::START))
		{
			startMeshDetails = m_splineMeshMap.Find(ESplineMeshType::START);
		}
		FSplineMeshDetails* endMeshDetails = nullptr;
		if (m_splineMeshMap.Contains(ESplineMeshType::END))
		{
			endMeshDetails = m_splineMeshMap.Find(ESplineMeshType::END);
		}
		FSplineMeshDetails* defaultMeshDetails = nullptr;
		if (m_splineMeshMap.Contains(ESplineMeshType::DEFAULT))
		{
			defaultMeshDetails = m_splineMeshMap.Find(ESplineMeshType::DEFAULT);
		}
		else
		{
			// If we don't have default mesh to work, exit.
			return;
		}

		const int32 splinePoints = m_splineComponent->GetNumberOfSplinePoints();
		for (int splineCount = 0; splineCount < (splinePoints - 1); splineCount++)
		{
			USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
			UStaticMesh* staticMesh = defaultMeshDetails->mesh;
			UMaterialInterface* material = nullptr;
			ESplineMeshAxis::Type forwardAxis = defaultMeshDetails->forwardAxis;

			// Start mesh
			if (startMeshDetails && startMeshDetails->mesh && splineCount == 0)
			{
				staticMesh = startMeshDetails->mesh;
				forwardAxis = startMeshDetails->forwardAxis;
				if (startMeshDetails->defaultMaterial)
				{
					material = startMeshDetails->defaultMaterial;
				}
			}
			else if (endMeshDetails && endMeshDetails->mesh && splineCount > 2 && splineCount == (splinePoints - 2))
			{
				// End mesh
				staticMesh = endMeshDetails->mesh;
				forwardAxis = endMeshDetails->forwardAxis;
				if (endMeshDetails->defaultMaterial)
				{
					material = endMeshDetails->defaultMaterial;
				}
			}
			else
			{
				// Middle mesh
				if (defaultMeshDetails->alternativeMaterial && splineCount > 0 && splineCount % 2 == 0)
				{
					material = defaultMeshDetails->alternativeMaterial;
				}
				else if (defaultMeshDetails->defaultMaterial)
				{
					material = defaultMeshDetails->defaultMaterial;
				}
			}
			
			// update mesh detail
			splineMesh->SetStaticMesh(staticMesh);
			splineMesh->SetForwardAxis(forwardAxis, true);
			splineMesh->SetMaterial(0, material);

			// Initialize the object.
			splineMesh->RegisterComponentWithWorld(GetWorld());
			splineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			splineMesh->SetMobility(EComponentMobility::Movable);
			splineMesh->AttachToComponent(m_splineComponent, FAttachmentTransformRules::KeepRelativeTransform);


			// Define the position of the points and tangents.
			const FVector startPoint = m_splineComponent->GetLocationAtSplinePoint(splineCount, ESplineCoordinateSpace::Local);
			const FVector startTangent = m_splineComponent->GetTangentAtSplinePoint(splineCount, ESplineCoordinateSpace::Local);
			const FVector endPoint = m_splineComponent->GetLocationAtSplinePoint(splineCount + 1, ESplineCoordinateSpace::Local);
			const FVector endTangent = m_splineComponent->GetTangentAtSplinePoint(splineCount + 1, ESplineCoordinateSpace::Local);
			
			splineMesh->SetStartAndEnd(startPoint, startTangent, endPoint, endTangent, true);

			// query physics, no collision.
			splineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

// Called when the game starts or when spawned
void ASplineActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
