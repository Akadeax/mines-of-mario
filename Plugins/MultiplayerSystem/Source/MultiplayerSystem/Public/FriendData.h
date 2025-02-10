#pragma once

#include "FriendData.generated.h"

USTRUCT(BlueprintType)
struct FFriendData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) FString DisplayName;
	UPROPERTY(BlueprintReadWrite) FString UserId;
};
