// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EscapeRoomLoadSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MINESOFMARIO_API UEscapeRoomLoadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	bool HostIsNavigator = false;
};
