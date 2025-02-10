// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "FriendData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FriendsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOnlinePlayersRead, const TArray<FFriendData>&, FriendsList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInGamePlayersRead, const TArray<FFriendData>&, FriendsList);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSYSTEM_API UFriendsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFriendsSubsystem();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION(BlueprintCallable) void ReadOnlinePlayers();
	UFUNCTION(BlueprintCallable) void ReadInGamePlayers();
	
	UFUNCTION(BlueprintCallable) void OpenSessionFriendInviteUI() const;
	UFUNCTION(BlueprintCallable) void OpenFriendsUI() const;

	UPROPERTY(BlueprintAssignable) FOnOnlinePlayersRead OnlinePlayersRead;
	UPROPERTY(BlueprintAssignable) FOnInGamePlayersRead InGamePlayersRead;
	
	FUniqueNetIdPtr CreateUniqueIdFromString(const FString& StringId) const;
	
private:
	void ReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	TArray<FFriendData> OnlineFriends;
	TArray<FFriendData> InGameFriends;
	
	IOnlineFriendsPtr FriendsInterface;
	IOnlineIdentityPtr IdentityInterface;
	IOnlineExternalUIPtr ExternalUIInterface;
};
