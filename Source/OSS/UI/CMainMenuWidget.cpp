#include "CMainMenuWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "CSessionRowWidget.h"

UCMainMenuWidget::UCMainMenuWidget()
{
	ConstructorHelpers::FClassFinder<UUserWidget> SessionRowClassAsset(TEXT("/Game/UI/WB_SessionRow"));
	if (SessionRowClassAsset.Succeeded())
	{
		SessionRowClass = SessionRowClassAsset.Class;
	}
}

bool UCMainMenuWidget::Initialize()
{
	bool bSuccess = Super::Initialize();

	if (bSuccess == false)
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UCMainMenuWidget::HostServer);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UCMainMenuWidget::SwitchJoinMenu);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCMainMenuWidget::SwitchMainMenu);
	}

	if (JoinServerButton)
	{
		JoinServerButton->OnClicked.AddDynamic(this, &UCMainMenuWidget::JoinServer);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UCMainMenuWidget::QuitPressed);
	}
	return true;
}

void UCMainMenuWidget::HostServer()
{
	ensure(OwningInstance);

	OwningInstance->Host();
}

void UCMainMenuWidget::SwitchJoinMenu()
{
	ensure(MenuSwitcher);
	ensure(JoinMenu);
	ensure(OwningInstance);

	OwningInstance->StartFindSession();

	MenuSwitcher->SetActiveWidget(JoinMenu);

}

void UCMainMenuWidget::SwitchMainMenu()
{
	ensure(MenuSwitcher);
	ensure(MainMenu);

	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UCMainMenuWidget::JoinServer()
{
	if (SelectedIndex.IsSet() && OwningInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectedIndex : %d"), SelectedIndex.GetValue());
		OwningInstance->Join(SelectedIndex.GetValue());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectedIndex is not Set yet"));
	}
}

void UCMainMenuWidget::QuitPressed()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();

	PC->ConsoleCommand("Quit");
}

void UCMainMenuWidget::SetSessionList(TArray<FSessionData> InSessionDatas)
{
	SessionList->ClearChildren();

	uint32 i = 0;

	for (const auto& SessionData : InSessionDatas)
	{
		UCSessionRowWidget* SessionRow = CreateWidget<UCSessionRowWidget>(this, SessionRowClass);
		if (SessionRow)
		{
			SessionRow->SessionName->SetText(FText::FromString(SessionData.Name));
			SessionRow->HostName->SetText(FText::FromString(SessionData.HostUserName));
			SessionRow->PlayerNumber->SetText(FText::FromString(SessionData.CurPlayers + "/" + SessionData.MaxPlayers));
			SessionRow->Setup(this, i++);
			SessionList->AddChild(SessionRow);
		}
	}
}

void UCMainMenuWidget::SetSelectedIndex(uint32 InIndex)
{
	SelectedIndex = InIndex;

	int32 SessionCount = SessionList->GetChildrenCount();
	
	for (int i = 0; i < SessionCount; i++)
	{
		UCSessionRowWidget* SessionRow= Cast<UCSessionRowWidget>(SessionList->GetChildAt(i));
		if (SessionRow)
		{
			SessionRow->bEverClicked = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
		}
		
	}
}
