#pragma once
#include "CoreMinimal.h"
#include "SDataSplineMesh.generated.h"

USTRUCT(BlueprintType)
struct FSDataSplineMesh
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector StartPos;
	
	UPROPERTY()
	FVector EndPos;
	
	UPROPERTY()
	FVector StartTang;
	
	UPROPERTY()
	FVector EndTang;
	
	UPROPERTY()
	UStaticMesh* StaticMesh;
	
	UPROPERTY()
	FVector2D StartScale;
	
	UPROPERTY()
	FVector2D EndScale;
	
};
