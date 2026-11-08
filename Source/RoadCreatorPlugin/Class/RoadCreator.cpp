// Fill out your copyright notice in the Description page of Project Settings.


#include "RoadCreator.h"

#include "Components/SplineMeshComponent.h"


// Sets default values
ARoadCreator::ARoadCreator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>("SplineMesh");
	SetRootComponent(SplineComponent);
}

// Called when the game starts or when spawned
void ARoadCreator::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARoadCreator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARoadCreator::GenerateRoad()
{
	if (!RoadMeshTemp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("Aucun StaticMesh n'a été donné"));
		return;
	}
	
	// On détruit tout les anciens meshs placés
	for (int i = 0; i < GeneratedSplineMeshes.Num(); i++)
	{
		if (IsValid(GeneratedSplineMeshes[i]))
		{
			GeneratedSplineMeshes[i]->DestroyComponent();
		}
	}
	GeneratedSplineMeshes.Empty();
	
	
	FVector MeshSize = (RoadMeshTemp->GetBoundingBox().Max - RoadMeshTemp->GetBoundingBox().Min);
	int NumberOfMeshes = FMath::CeilToInt(SplineComponent->GetSplineLength() / MeshSize.X);
	
	for (int i = 0; i < NumberOfMeshes; i++)
	{
		FVector StartPos = SplineComponent->GetLocationAtDistanceAlongSpline(MeshSize.X * i, ESplineCoordinateSpace::Local);
		FVector EndPos = SplineComponent->GetLocationAtDistanceAlongSpline(FMath::Min(MeshSize.X * (i + 1),SplineComponent->GetSplineLength()), ESplineCoordinateSpace::Local);
		
		FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(MeshSize.X * i, ESplineCoordinateSpace::Local);
		FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(MeshSize.X * (i + 1), ESplineCoordinateSpace::Local);
		
		StartTang = StartTang.GetClampedToMaxSize(MeshSize.X);
		EndTang = EndTang.GetClampedToMaxSize(MeshSize.X);
		
		USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(this);
		splineMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		splineMesh->SetStaticMesh(RoadMeshTemp);
		splineMesh->RegisterComponent();
		splineMesh->SetStartAndEnd(StartPos, StartTang, EndPos, EndTang);
		
		GeneratedSplineMeshes.Add(splineMesh);
	}
}

void ARoadCreator::LiveGenerateRoad()
{
	if (!LiveGeneration) return;
	
	if (CurrentFrameGap % FrameGap == 0)
	{
		GenerateRoad();
		CurrentFrameGap = 0;
	}
	CurrentFrameGap++;
}

void ARoadCreator::SetRoadPointParameters()
{
	RoadPointParameters.SetNum(SplineComponent->GetNumberOfSplinePoints());
}

