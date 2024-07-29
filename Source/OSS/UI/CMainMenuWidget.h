#pragma once

#include "CoreMinimal.h"
#include "CMenuWidgetBase.h"
#include "../Game/CMenuInterface.h"
#include "CMainMenuWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class UPanelWidget;

USTRUCT(BlueprintType)
struct FSessionData
{
	GENERATED_BODY()

public:
	FString Name;
	uint16 CurPlayers;
	uint16 MaxPlayers;
	FString HostUserName;
};

UCLASS()
class OSS_API UCMainMenuWidget : public UCMenuWidgetBase
{
	GENERATED_BODY()

public:
	UCMainMenuWidget();

protected:
	virtual bool Initialize() override;

private:
	UFUNCTION()
	void HostServer();
	UFUNCTION()
	void SwitchJoinMenu();
	UFUNCTION()
	void SwitchMainMenu();
	UFUNCTION()
	void JoinServer();
	UFUNCTION()
	void QuitPressed();

public:
	void SetSessionList(TArray<FSessionData> InSessionDatas);
	void SetSelectedIndex(uint32 InIndex);

protected:
	//Buttons
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;
	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinServerButton;
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

	//Widgets
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* MenuSwitcher;
	UPROPERTY(meta = (BindWidget))
	UWidget* MainMenu;
	UPROPERTY(meta = (BindWidget))
	UWidget* JoinMenu;
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* SessionList;

private:
	TSubclassOf<UUserWidget> SessionRowClass;
	TOptional<uint32> SelectedIndex;
};
