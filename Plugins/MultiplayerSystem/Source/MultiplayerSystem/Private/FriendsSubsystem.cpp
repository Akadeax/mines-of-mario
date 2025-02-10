// Fill out your copyright notice in the Description page of Project Settings.


#include "FriendsSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"

UFriendsSubsystem::UFriendsSubsystem()
{
}

void UFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* OnlineSubsystem{ IOnlineSubsystem::Get() };
	check(OnlineSubsystem);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Blue,
			FString::Printf(TEXT("Initializing friends with subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString())
		);
	}
	
	FriendsInterface = OnlineSubsystem->GetFriendsInterface();
	if (!FriendsInterface) UE_LOG(LogTemp, Warning, TEXT("FriendsInterface invalid!"));
	 
	IdentityInterface = OnlineSubsystem->GetIdentityInterface();
	if (!IdentityInterface) UE_LOG(LogTemp, Warning, TEXT("IdentityInterface invalid!"));

	ExternalUIInterface = OnlineSubsystem->GetExternalUIInterface();
	if (!ExternalUIInterface) UE_LOG(LogTemp, Warning, TEXT("ExternalUIInterface invalid!"));
}

void UFriendsSubsystem::ReadOnlinePlayers()
{
	const bool bSuccess{ FriendsInterface->ReadFriendsList(
			0, ToString(EFriendsLists::OnlinePlayers),
			FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::ReadFriendsListComplete)
		) };

	if (!bSuccess)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red,
			TEXT("Failed to read online friends list!")
		);
	}
}

void UFriendsSubsystem::ReadInGamePlayers()
{
	const bool bSuccess{ FriendsInterface->ReadFriendsList(
		0, ToString(EFriendsLists::InGamePlayers),
		FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::ReadFriendsListComplete)
	) };

	if (!bSuccess)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red,
			TEXT("Failed to read in game friends list!")
		);
	}
}

void UFriendsSubsystem::OpenSessionFriendInviteUI() const
{
	ExternalUIInterface->ShowInviteUI(0);
}

void UFriendsSubsystem::OpenFriendsUI() const
{
	ExternalUIInterface->ShowFriendsUI(0);
}

void UFriendsSubsystem::ReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName,
                                                  const FString& ErrorStr)
{
	if (!bWasSuccessful)
	{
		if (ListName == ToString(EFriendsLists::OnlinePlayers))
		{
			OnlinePlayersRead.Broadcast({});
		}
		else if (ListName == ToString(EFriendsLists::InGamePlayers))
		{
			InGamePlayersRead.Broadcast({});
		}
		
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorStr);
		return;
	}
	
	TArray<TSharedRef<FOnlineFriend>> Friends;
	FriendsInterface->GetFriendsList(0, ListName, Friends);
	
	TArray<FFriendData> ProcessedFriends;
	ProcessedFriends.Reserve(Friends.Num());
	
	for (const TSharedRef<FOnlineFriend>& Friend : Friends)
	{
		FFriendData Data = FFriendData{
			Friend->GetDisplayName(),
			Friend->GetUserId()->ToString()
		};

		ProcessedFriends.Add(MoveTemp(Data));
	}

	if (ListName == ToString(EFriendsLists::OnlinePlayers))
	{
		OnlineFriends = ProcessedFriends;
		OnlinePlayersRead.Broadcast(OnlineFriends);
		return;
	}
	if (ListName == ToString(EFriendsLists::InGamePlayers))
	{
		InGameFriends = ProcessedFriends;
		InGamePlayersRead.Broadcast(InGameFriends);
		return;
	}
	
	check(false);
}

FUniqueNetIdPtr UFriendsSubsystem::CreateUniqueIdFromString(const FString& StringId) const
{
	if (!IdentityInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("IdentityInterface invalid!"));
	}
	
	return IdentityInterface->CreateUniquePlayerId(StringId);
}
