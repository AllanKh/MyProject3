#pragma once
#include "CoreMinimal.h"
#include "EMatchColor.generated.h"

UENUM(BlueprintType)
enum class EMatchColor : uint8
{
    None,
    Red,
    Green,
    Blue,
    Yellow
};
