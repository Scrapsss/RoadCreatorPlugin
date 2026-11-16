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
	// On détruit tout les anciens meshs placés
	for (int i = 0; i < GeneratedSplineMeshes.Num(); i++)
	{
		if (IsValid(GeneratedSplineMeshes[i]))
		{
			GeneratedSplineMeshes[i]->DestroyComponent();
		}
	}
	
	GeneratedSplineMeshes.Empty();
	
	if (!RoadMeshTemp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("Aucun static mesh précisé !"));
		return;
	}
	
	// -- Road Generation
	FVector MeshSize = (RoadMeshTemp->GetBoundingBox().Max - RoadMeshTemp->GetBoundingBox().Min);
	int NumberOfMeshes = FMath::CeilToInt(SplineComponent->GetSplineLength() / MeshSize.X);
	
	USplineMeshComponent* MeshNeighbour = nullptr;
	
	for (int i = 0; i < NumberOfMeshes; i++)
	{
		FVector StartPos = SplineComponent->GetLocationAtDistanceAlongSpline(MeshSize.X * i, ESplineCoordinateSpace::Local);
		FVector EndPos = SplineComponent->GetLocationAtDistanceAlongSpline(FMath::Min(MeshSize.X * (i + 1),SplineComponent->GetSplineLength()), ESplineCoordinateSpace::Local);
		
		FVector StartTang = SplineComponent->GetTangentAtDistanceAlongSpline(MeshSize.X * i, ESplineCoordinateSpace::Local);
		FVector EndTang = SplineComponent->GetTangentAtDistanceAlongSpline(MeshSize.X * (i + 1), ESplineCoordinateSpace::Local);
		
		StartTang = StartTang.GetClampedToMaxSize(MeshSize.X);
		EndTang = EndTang.GetClampedToMaxSize(MeshSize.X);
		
		TOptional<FSRoadPointParameter> RoadParameters = GetRoadPointParameter(MeshSize.X * i);
		
		USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(this);
		splineMesh->SetMobility(EComponentMobility::Movable);

		splineMesh->SetupAttachment(SplineComponent);
		splineMesh->SetStaticMesh(RoadMeshTemp);
		splineMesh->SetForwardAxis(ESplineMeshAxis::Y);
		
		if (IsValid(MeshNeighbour))
		{
			splineMesh->SetStartScale(MeshNeighbour->GetEndScale());
		}
		
		if (RoadParameters.IsSet())
		{
			splineMesh->SetEndScale(FVector2D(1, RoadParameters->Width));
		}
		else
		{
			splineMesh->SetEndScale(FVector2D(1, DefaultWidth));
		}
		
		
		splineMesh->SetStartRoll(FMath::DegreesToRadians(-90.0f));
		splineMesh->SetEndRoll(FMath::DegreesToRadians(-90.0f));
		splineMesh->SetStartAndEnd(StartPos, StartTang, EndPos, EndTang);
		
		
		splineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		splineMesh->SetCollisionResponseToAllChannels(ECR_Block);
		splineMesh->UpdateMesh();
		
		splineMesh->RegisterComponent();
		
		GeneratedSplineMeshes.Add(splineMesh);
		MeshNeighbour = splineMesh;
	}
}

void ARoadCreator::SetRoadPointParameters()
{
	RoadPointParameters.SetNum(SplineComponent->GetNumberOfSplinePoints());
}


TOptional<FSRoadPointParameter> ARoadCreator::GetRoadPointParameter(float DistanceAlongSpline)
{
	if (GeneratedSplineMeshes.IsEmpty())
	{
		return {};
	}
	
	FSRoadPointParameter RoadParameterToUse;
	float ClosestDistance = 0;
	for (int i = 0; i < GeneratedSplineMeshes.Num(); i++)
	{
		float PointDistance = SplineComponent->GetDistanceAlongSplineAtLocation(SplineComponent->GetSplinePointAt(i, ESplineCoordinateSpace::World).Position, ESplineCoordinateSpace::World);
		
		float Distance = DistanceAlongSpline - PointDistance;
		if (Distance < 0) continue;
		if (Distance < ClosestDistance || ClosestDistance == 0)
		{
			RoadParameterToUse = RoadPointParameters[i];
		}
	}
	return RoadParameterToUse;
}
