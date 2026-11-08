#pragma once
#include "CoreMinimal.h"
#include "RoadCreatorPlugin/Enum/RoadType.h"
#include "SRoadPointParameter.generated.h"

USTRUCT(BlueprintType)
struct FSRoadPointParameter
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERoadType RoadType = ERoadType::Road;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width = 100.0f;
};
