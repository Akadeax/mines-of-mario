// Fill out your copyright notice in the Description page of Project Settings.

#include "SessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

USessionsSubsystem::USessionsSubsystem()
	: CreateSessionCompleteDelegate{ FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete) }
	, FindSessionsCompleteDelegate{ FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete) }
	, JoinSessionCompleteDelegate{ FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete) }
	, DestroySessionCompleteDelegate{ FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete) }
	, StartSessionCompleteDelegate{ FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete) }
	, SessionUserInviteAcceptedDelegate{ FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionUserInviteAccepted) }
{
	const IOnlineSubsystem* OnlineSubsystem{ IOnlineSubsystem::Get() };
	check(OnlineSubsystem);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Blue,
			FString::Printf(TEXT("Initializing sessions with subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString())
		);
	}
	
	SessionInterface = OnlineSubsystem->GetSessionInterface();
}

void USessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FriendsSubsystem = GetGameInstance()->GetSubsystem<UFriendsSubsystem>();
}

void USessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	check(SessionInterface);

	const FNamedOnlineSession* ExistingSession{ SessionInterface->GetNamedSession(NAME_GameSession) };
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		
		DestroySession();
		return;
	}

	// Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShared<FOnlineSessionSettings>();
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->Set(
		FName("MatchType"),
		FString(MatchType),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing
	);

	const UWorld* World{ GetWorld() };
	check(World);
	const ULocalPlayer* LocalPlayer{ World->GetFirstLocalPlayerFromController() };
	check(LocalPlayer);
	const bool bSuccess{ SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings) };
	if (!bSuccess)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void USessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	check(SessionInterface);

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShared<FOnlineSessionSearch>();
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer{ GetWorld()->GetFirstLocalPlayerFromController() };
	const bool bSuccess{ SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()) };
	if (!bSuccess)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast({}, false);
	}
}

void USessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer{ GetWorld()->GetFirstLocalPlayerFromController() };
	const bool bSuccess{ SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult) };
	if (!bSuccess)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void USessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	const bool bSuccess{ SessionInterface->DestroySession(NAME_GameSession) };
	if (!bSuccess)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void USessionsSubsystem::StartSession()
{
}

void USessionsSubsystem::StartWaitForInviteAccept()
{
	ensureAlwaysMsgf(SessionInterface, TEXT("SessionInterface invalid!"));
	SessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegate);
}

void USessionsSubsystem::StopWaitForInviteAccept()
{
	ensureAlwaysMsgf(SessionInterface, TEXT("SessionInterface invalid!"));
	SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegateHandle);
}

void USessionsSubsystem::InviteUserToSession(const FString& UserId) const
{
	SessionInterface->SendSessionInviteToFriend(
		0, NAME_GameSession,
		*FriendsSubsystem->CreateUniqueIdFromString(UserId)
	);
}

bool USessionsSubsystem::GetResolvedConnectString(FName SessionName, FString& OutAddress) const
{
	return SessionInterface->GetResolvedConnectString(SessionName, OutAddress);
}

void USessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	ensureAlwaysMsgf(SessionInterface, TEXT("SessionInterface invalid!"));
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void USessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	check(SessionInterface);
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	if (LastSessionSearch->SearchResults.Num() == 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast({}, false);
		return;
	}
	
	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void USessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	check(SessionInterface);
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void USessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	check(SessionInterface);
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void USessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void USessionsSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId,
	FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Purple, "Invite Accepted");

	check(SessionInterface);
	SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegateHandle);
	MultiplayerOnSessionUserInviteAccepted.Broadcast(InviteResult, bWasSuccessful);
}
