#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "CMenuInterface.h"
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
	virtual void Host() override;
	UFUNCTION(Exec)
	virtual void Join(const FString& InAddress) override;

	virtual void OpenMainMenuLevel() override;
	virtual void StartFindSession() override;

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

private:
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UCMainMenuWidget* MainMenu;
	TSubclassOf<UUserWidget> InGameMenuWidgetClass;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
