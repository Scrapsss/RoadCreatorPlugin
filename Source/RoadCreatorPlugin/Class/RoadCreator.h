// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "RoadCreatorPlugin/Enum/RoadType.h"
#include "RoadCreatorPlugin/Struct/SRoadPointParameter.h"
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
	
	UFUNCTION(CallInEditor, Category = "RoadCreator")
	virtual void GenerateRoad();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* SplineComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadCreator")
	TMap<ERoadType, TObjectPtr<UStaticMesh>> RoadMeshes; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadCreator")
	UStaticMesh* RoadMeshTemp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadCreator")
	FRotator RoadRotation;
	
	UPROPERTY()
	TArray<USplineMeshComponent*> GeneratedSplineMeshes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadPointParameters")
	TArray<FSRoadPointParameter> RoadPointParameters;
	
	UFUNCTION(BlueprintCallable, Category = "RoadPointParameters")
	void SetRoadPointParameters();
};