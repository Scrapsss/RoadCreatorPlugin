#pragma once

#include "CoreMinimal.h"
#include "RoadType.generated.h"

UENUM(BlueprintType)
enum class ERoadType : uint8
{
	Road,
	Grass,
	Gravel,
	Dirt,
	Snow
};
