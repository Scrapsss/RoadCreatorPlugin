// Fill out your copyright notice in the Description page of Project Settings.


#include "RoadCreator.h"

#include "Components/SplineMeshComponent.h"


// Sets default values
ARoadCreator::ARoadCreator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>("SplineMesh");
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
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("Generate Road"));
	}
	
	float SplinePointNumber = SplineComponent->GetNumberOfSplinePoints();
	
	for (int i = 0; i < SplinePointNumber - 1; i++)
	{
		
		
		USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>();
		splineMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	
	
}

