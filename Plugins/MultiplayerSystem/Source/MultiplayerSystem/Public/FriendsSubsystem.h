// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FriendsSubsystem.generated.h"

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
	
	UFUNCTION(BlueprintCallable) void ReadFriendsList();
	UFUNCTION(BlueprintCallable) void OpenSessionFriendInviteUI();
	UFUNCTION(BlueprintCallable) void OpenFriendsUI();
	
private:
	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	FUniqueNetIdPtr CreateUniqueIdFromString(const FString& StringId) const;

	IOnlineFriendsPtr FriendsInterface;
	IOnlineIdentityPtr IdentityInterface;
	IOnlineExternalUIPtr ExternalUIInterface;
};
