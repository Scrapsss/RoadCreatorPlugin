// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "RoadCreatorPlugin/Enum/RoadType.h"
#include "RoadCreator.generated.h"

UCLASS()
class ROADCREATORPLUGIN_API ARoadCreator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoadCreator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	virtual void GenerateRoad();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadCreator")
	float TestSize = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* SplineComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadCreator")
	TMap<ERoadType, TObjectPtr<UStaticMesh>> RoadMeshes; 
};
