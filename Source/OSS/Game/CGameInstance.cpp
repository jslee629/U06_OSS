#include "CGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "../OSS.h"
#include "../UI/CMainMenuWidget.h"

const static FName SESSION_NAME = TEXT("GameSession");			// UE4의 버그: SessionName은 무조건 "GameSession"을 넣어줘야 한다
const static FName SESSION_SETTINGS_KEY = TEXT("ToreKey");

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
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UCGameInstance::OnJoinSessionCompleted);
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("OSS Not found"));
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UCGameInstance::OnNetworkFailure);
	}
}

void UCGameInstance::Host(FString InDesiredSessionName)
{
	DesiredSessionName = InDesiredSessionName;

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

		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
		{
			SessionSettings.bIsLANMatch = true;			// 같은 사설망 안에 있는 클라이언트만
		}
		else
		{
			SessionSettings.bIsLANMatch = false;		// 인터넷을 향해
		}

		SessionSettings.NumPublicConnections = 5;		// 세션 안에 접근하여 들어올 수 있는 사람 수
		SessionSettings.bShouldAdvertise = true;		// 세션 검색이 되도록 하느냐 안 되도록 하느냐
		SessionSettings.bUsesPresence = true;			// Steam을 사용할 때: true면 CreateLobbySession, false면 CreateInternetSession
		SessionSettings.Set								// 세션에 접근할 key와 Value를 저장 -> Value에 세션 이름이 들어갈 예정
		(
			SESSION_SETTINGS_KEY, 
			DesiredSessionName, 
			EOnlineDataAdvertisementType::Type::ViaOnlineServiceAndPing
		);

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}

void UCGameInstance::Join(uint32 InIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid())
	{
		return;
	}

	if (MainMenu)
	{
		MainMenu->SetInputToGame();
	}

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[InIndex]);
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

		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UCGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->StartSession(SESSION_NAME);
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

	World->ServerTravel("/Game/Maps/Lobby?listen");
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

		TArray<FSessionData> SessionList;

		for (const auto& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Error, TEXT("Found Session ID : %s"), *SearchResult.GetSessionIdStr());
			UE_LOG(LogTemp, Error, TEXT("Ping(ms) : %d"), SearchResult.PingInMs);

			FSessionData Data;
			Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			Data.CurPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
			Data.HostUserName = SearchResult.Session.OwningUserName;
			
			FString SessionName;
			if (SearchResult.Session.SessionSettings.Get(SESSION_SETTINGS_KEY, SessionName))
			{
				Data.Name = SessionName;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("SessionSettings Key is not Valid"));
			}

			SessionList.Add(Data);
		}

		MainMenu->SetSessionList(SessionList);
	}
}

void UCGameInstance::OnJoinSessionCompleted(FName InSessionName, EOnJoinSessionCompleteResult::Type OutResult)
{
	//Get the Address from Session Name
	FString Address;
	if (SessionInterface->GetResolvedConnectString(InSessionName, Address) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Could Not Resolve to IP Address"));
		return;
	}

	LogOnScreen(this, "Join to " + Address, FColor::Green);

	//ClientTravel
	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		return;
	}

	PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UCGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDrive, ENetworkFailure::Type FailureType, const FString& ErrorMessage)
{
	LogOnScreen(this, ErrorMessage, FColor::Red);
	OpenMainMenuLevel();
}
