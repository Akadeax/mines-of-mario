// Fill out your copyright notice in the Description page of Project Settings.


#include "MarioGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "MoviePlayer.h"

void UMarioGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMarioGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMarioGameInstance::EndLoadingScreen);
}

void UMarioGameInstance::BeginLoadingScreen(const FString& MapName)
{
	if (IsRunningDedicatedServer()) return;
	
	const auto LoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass, TEXT("LoadingScreen"));
	const auto LoadingSWidgetPtr = LoadingWidget->TakeWidget();
	
	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.WidgetLoadingScreen = LoadingSWidgetPtr;
	LoadingScreen.bAllowInEarlyStartup = false;
	LoadingScreen.PlaybackType = MT_Normal;
	LoadingScreen.bAllowEngineTick = false;
	LoadingScreen.bWaitForManualStop = false;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreen.MinimumLoadingScreenDisplayTime = 0.f;
 
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

void UMarioGameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{
}
