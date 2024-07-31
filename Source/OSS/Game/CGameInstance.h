#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CMenuInterface.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "CGameInstance.generated.h"

class UUserWidget;
class UCMainMenuWidget;
class FOnlineSessionSearch;

UCLASS()
class OSS_API UCGameInstance : public UGameInstance, public ICMenuInterface
{
	GENERATED_BODY()

public:
	UCGameInstance();
	virtual void Init() override;

public:
	UFUNCTION(Exec)
	virtual void Host(FString InDesiredSessionName) override;
	UFUNCTION(Exec)
	virtual void Join(uint32 InIndex) override;

	virtual void OpenMainMenuLevel() override;
	virtual void StartFindSession() override;

	void StartSession();

private:	
	void CreateSession_Internal();

public:
	UFUNCTION(BlueprintCallable, Exec)
	void LoadMainMenu();
	UFUNCTION(BlueprintCallable, Exec)
	void LoadInGameMenu();

private:
	void OnCreateSessionCompleted(FName InSessionName, bool bWasSuccessful);
	void OnDestroySessionCompleted(FName InSessionName, bool bWasSuccessful);
	void OnFindSessionsCompleted(bool bWasSuccessful);
	void OnJoinSessionCompleted(FName InSessionName, EOnJoinSessionCompleteResult::Type OutResult);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDrive, ENetworkFailure::Type FailureType, const FString& ErrorMessage);

private:
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UCMainMenuWidget* MainMenu;
	TSubclassOf<UUserWidget> InGameMenuWidgetClass;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FString DesiredSessionName;
};
