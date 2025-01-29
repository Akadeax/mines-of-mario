// Fill out your copyright notice in the Description page of Project Settings.

#include "ServerBrowser.h"

#include "SessionsSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UServerBrowser::MenuSetup(int32 SessionNumPublicConnections, FString SessionMatchType, FString SessionLobbyPath)
{
	NumPublicConnections = SessionNumPublicConnections;
	MatchType = SessionMatchType;
	LobbyPath = FString::Printf(TEXT("%s?listen"), *SessionLobbyPath);

	const UGameInstance* GameInstance{ GetGameInstance() };
	check(GameInstance);
	
	SessionsSubsystem = GameInstance->GetSubsystem<USessionsSubsystem>();
	check(SessionsSubsystem);
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	
	const UWorld* World{ GetWorld() };
	check(World);

	APlayerController* PlayerController{ World->GetFirstPlayerController() };
	check(PlayerController);

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);

	SessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSessionComplete);
	SessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSessionComplete);
	
	SessionsSubsystem->StartWaitForInviteAccept();
	SessionsSubsystem->MultiplayerOnSessionUserInviteAccepted.AddUObject(this, &ThisClass::OnUserInviteAccepted);
}

void UServerBrowser::HostSession()
{
	SessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
}

void UServerBrowser::OnCreateSessionComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful) return;

	GetWorld()->ServerTravel(LobbyPath);
}

void UServerBrowser::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	FString Address;
	SessionsSubsystem->GetResolvedConnectString(NAME_GameSession, Address);

	APlayerController* PlayerController{ GetGameInstance()->GetFirstLocalPlayerController() };
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController invalid!"));
		return;
	}
	
	PlayerController->ClientTravel(Address, TRAVEL_Absolute);
}


void UServerBrowser::OnUserInviteAccepted(const FOnlineSessionSearchResult& InviteResult, const bool bWasSuccessful)
{
	if (SessionsSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem is nullptr!"));
		return;
	}

	SessionsSubsystem->JoinSession(InviteResult);
}
