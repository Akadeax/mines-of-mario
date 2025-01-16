// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MarioGameInstance.generated.h"

UCLASS()
class MINESOFMARIO_API UMarioGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loading Screen")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	virtual void BeginLoadingScreen(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "Loading Screen")
	virtual void EndLoadingScreen(UWorld* InLoadedWorld);
};
