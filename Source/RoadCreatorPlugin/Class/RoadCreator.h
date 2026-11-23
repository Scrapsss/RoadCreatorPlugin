// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "RoadCreatorPlugin/Enum/RoadType.h"
#include "RoadCreatorPlugin/Struct/SDataSplineMesh.h"
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
	
	UFUNCTION(BlueprintCallable, Category = "RoadCreator")
	virtual void GenerateRoad();
	
	
	UFUNCTION(CallInEditor, Category = "RoadCreator")
	virtual void OldGenerateRoad();
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadCreator")
	bool LiveGeneration = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* SplineComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadCreator")
	TMap<ERoadType, TObjectPtr<UStaticMesh>> RoadMeshes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadCreator")
	float DefaultWidth = 1.0f;
	
	UPROPERTY()
	TArray<USplineMeshComponent*> GeneratedSplineMeshes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadPointParameters")
	float WidthForAll = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoadPointParameters")
	TArray<FSRoadPointParameter> RoadPointParameters;
	
	UFUNCTION(BlueprintCallable, Category = "RoadPointParameters")
	void SetRoadPointParameters();
	
	UFUNCTION()
	int GetRoadPointParameter(float DistanceAlongSpline);
	
	UFUNCTION()
	UStaticMesh* GetRoadStaticMesh(ERoadType RoadType);
	
	UFUNCTION()
	bool IsSplineMeshDifferent(USplineMeshComponent* FirstSplineMesh, FSDataSplineMesh SecondSplineMesh);
	
	UFUNCTION(CallInEditor, Category = "RoadPointParameters")
	void SetAllPointsWidth();
	
	UFUNCTION(CallInEditor, Category = "RoadPointParameters")
	void AddScale();
	
	UFUNCTION(CallInEditor, Category = "RoadPointParameters")
	void SubtractScale();
	
	UFUNCTION(CallInEditor, Category = "RoadPointParameters")
	void ModifyScale(float value, bool WantToSet);
	
};