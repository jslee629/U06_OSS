#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "CPlayerState.h"
#include "../Characters/FPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AFPSGameMode::AFPSGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Player/BP_CPlayer"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	HUDClass = AFPSHUD::StaticClass();

	PlayerStateClass = ACPlayerState::StaticClass();
}

void AFPSGameMode::StartPlay()
{
	Super::StartPlay();

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == "Red")
		{
			RedTeamPlayerStarts.Add(*It);
		}
		else
		{
			BlueTeamPlayerStarts.Add(*It);
		}
	}

	UE_LOG(LogTemp, Error, TEXT("RedTeam : %d"), RedTeamPlayerStarts.Num());
	UE_LOG(LogTemp, Error, TEXT("BlueTeam : %d"), BlueTeamPlayerStarts.Num());
}

void AFPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	//TODO: Disable Seamless Travel 
	Super::PostLogin(NewPlayer);

	AFPSCharacter* PlayerCharacter = NewPlayer->GetPawn<AFPSCharacter>();
	ACPlayerState* PS = NewPlayer->GetPlayerState<ACPlayerState>();
	if (PlayerCharacter && PS)
	{
		if (RedTeamPawns.Num() > BlueTeamPawns.Num())
		{
			PS->Team = ETeamType::Blue;
			BlueTeamPawns.Add(PlayerCharacter);
		}
		else
		{
			PS->Team = ETeamType::Red;
			RedTeamPawns.Add(PlayerCharacter);
		}
	}

	PlayerCharacter->SetTeamColor(PS->Team);
}

void AFPSGameMode::OnActorKilled(AActor* VictimActor)
{
	AFPSCharacter* Player = Cast<AFPSCharacter>(VictimActor);
	if (Player)
	{
		FTimerHandle TimeHandle_RespawnDelay;
		float RespawnDelay = 3.f;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController());
		GetWorldTimerManager().SetTimer(TimeHandle_RespawnDelay, Delegate, RespawnDelay, false);
	}
}

void AFPSGameMode::RespawnPlayerElapsed(APlayerController* Controller)
{
	if (ensure(Controller))
	{
		Controller->UnPossess();

		RestartPlayer(Controller);

		AFPSCharacter* NewPlayerCharacter = Controller->GetPawn<AFPSCharacter>();		// 무조건 리스타트 플레이어 이후
		ACPlayerState* PS = Controller->GetPlayerState<ACPlayerState>();
		if (NewPlayerCharacter && PS)
		{
			NewPlayerCharacter->SetTeamColor(PS->Team);
		}
	}
}
