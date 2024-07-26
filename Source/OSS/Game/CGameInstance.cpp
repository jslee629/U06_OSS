#include "CGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "../OSS.h"
#include "../UI/CMainMenuWidget.h"

const static FName SESSION_NAME = TEXT("GameSession");

UCGameInstance::UCGameInstance()
{
	ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWidgetClassAsset(TEXT("/Game/UI/WB_MainMenu"));
	if (MainMenuWidgetClassAsset.Succeeded())
	{
		MainMenuWidgetClass = MainMenuWidgetClassAsset.Class;
	}

	ConstructorHelpers::FClassFinder<UUserWidget> InGameWidgetClassAsset(TEXT("/Game/UI/WB_InGameMenu"));
	if (InGameWidgetClassAsset.Succeeded())
	{
		InGameMenuWidgetClass = InGameWidgetClassAsset.Class;
	}
}

void UCGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();

	if (OSS)
	{
		UE_LOG(LogTemp, Display, TEXT("OSS Name is %s"), *(OSS->GetSubsystemName().ToString()));

		SessionInterface = OSS->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Display, TEXT("Session Interface is found"));

			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCGameInstance::OnCreateSessionCompleted);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UCGameInstance::OnDestroySessionCompleted);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCGameInstance::OnFindSessionsCompleted);
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("OSS Not found"));
	}
}

void UCGameInstance::Host()
{
	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);

		if (ExistingSession)
		{
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			CreateSession_Internal();
		}
	}
}

void UCGameInstance::CreateSession_Internal()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = true;		// 같은 사설망 안에 있는 클라이언트만
		SessionSettings.NumPublicConnections = 2;	// 세션 안에 접근하여 들어올 수 있는 사람 수
		SessionSettings.bShouldAdvertise = true;	// 세션 검색이 되도록 하느냐 안 되도록 하느냐
		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}

void UCGameInstance::Join(const FString& InAddress)
{
	if (InAddress == "")
	{
		LogOnScreen(this, "Enter the correct IP" + InAddress, FColor::Green);
		return;
	}

	LogOnScreen(this, "Join to " + InAddress, FColor::Green);

	if (MainMenu)
	{
		MainMenu->SetInputToGame();
	}

	//ClientTravel
	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		return;
	}

	PC->ClientTravel(InAddress, ETravelType::TRAVEL_Absolute);
}

void UCGameInstance::OpenMainMenuLevel()
{
	//ClientTravel
	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		return;
	}

	PC->ClientTravel("/Game/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UCGameInstance::StartFindSession()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		LogOnScreen(this, "Start Finding Session");
		SessionSearch->bIsLanQuery = true;
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UCGameInstance::LoadMainMenu()
{
	ensure(MainMenuWidgetClass);

	MainMenu = CreateWidget<UCMainMenuWidget>(this, MainMenuWidgetClass);
	if (!MainMenu)
	{
		return;
	}

	MainMenu->SetOwningInstance(this);
	MainMenu->SetInputToUI();
}

void UCGameInstance::LoadInGameMenu()
{
	ensure(InGameMenuWidgetClass);

	UCMenuWidgetBase* InGameMenu = CreateWidget<UCMenuWidgetBase>(this, InGameMenuWidgetClass);
	if (!InGameMenu)
	{
		return;
	}

	InGameMenu->SetOwningInstance(this);
	InGameMenu->SetInputToUI();
}

void UCGameInstance::OnCreateSessionCompleted(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Error, TEXT("Created Session"));

	if (!bWasSuccessful)
	{
		LogOnScreen(this, "Could not create session!", FColor::Red);
		return;
	}

	LogOnScreen(this, "Create Session Completed, Session Name : " + InSessionName.ToString(), FColor::Green);

	if (MainMenu)
	{
		MainMenu->SetInputToGame();
	}

	//ServerTravel
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->ServerTravel("/Game/Maps/CoOp?listen");
}

void UCGameInstance::OnDestroySessionCompleted(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Error, TEXT("Destroied Session"));

	if (bWasSuccessful)
	{
		CreateSession_Internal();
	}
}

void UCGameInstance::OnFindSessionsCompleted(bool bWasSuccessful)
{
	if (bWasSuccessful && SessionSearch.IsValid() && MainMenu)
	{
		LogOnScreen(this, "Finish Finding Session");

		TArray<FString> SessionList;

		for (const auto& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Error, TEXT("Found Session ID : %s"), *SearchResult.GetSessionIdStr());
			UE_LOG(LogTemp, Error, TEXT("Ping(ms) : %d"), SearchResult.PingInMs);

			SessionList.Add(SearchResult.GetSessionIdStr());
		}

		MainMenu->SetSessionList(SessionList);
	}
}
