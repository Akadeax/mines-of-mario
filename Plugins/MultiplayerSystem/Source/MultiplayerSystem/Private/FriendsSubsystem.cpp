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

void UFriendsSubsystem::ReadFriendsList()
{
	const bool bSuccess{ FriendsInterface->ReadFriendsList(
			0, ToString(EFriendsLists::OnlinePlayers),
			FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnReadFriendsListComplete)
		) };

	if (!bSuccess)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red,
			TEXT("Failed to read friends list!")
		);
	}
}

void UFriendsSubsystem::OpenSessionFriendInviteUI()
{
	ExternalUIInterface->ShowInviteUI(0);
}

void UFriendsSubsystem::OpenFriendsUI()
{
	ExternalUIInterface->ShowFriendsUI(0);
}

void UFriendsSubsystem::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName,
                                                  const FString& ErrorStr)
{
	TArray<TSharedRef<FOnlineFriend>> Friends;
	FriendsInterface->GetFriendsList(0, ToString(EFriendsLists::OnlinePlayers), Friends);

	for (const TSharedRef<FOnlineFriend>& Friend : Friends)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Purple,
		// 	Friend->GetUserId()->ToString()
		// );
	}
}

FUniqueNetIdPtr UFriendsSubsystem::CreateUniqueIdFromString(const FString& StringId) const
{
	if (!IdentityInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("IdentityInterface invalid!"));
	}
	
	return IdentityInterface->CreateUniquePlayerId(StringId);
}
