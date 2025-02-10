// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbySelectMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTravelStarted);

/**
 * 
 */
UCLASS()
class MINESOFMARIO_API ULobbySelectMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(
		int32 SessionNumPublicConnections = 2,
		FString SessionMatchType = FString(TEXT("Escape")),
		FString SessionLobbyPath = FString(TEXT("/Game/Lobby/Lobby"))
	);

protected:
	UFUNCTION(BlueprintCallable) void HostSession();
	
private:
	UFUNCTION() void OnCreateSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);
	void OnUserInviteAccepted(const FOnlineSessionSearchResult& InviteResult, const bool bWasSuccessful);
	
	UPROPERTY() class USessionsSubsystem* SessionsSubsystem;
	
	int32 NumPublicConnections{ 2 };
	FString MatchType{ TEXT("Escape") };
	FString LobbyPath{ TEXT("") };

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
};
