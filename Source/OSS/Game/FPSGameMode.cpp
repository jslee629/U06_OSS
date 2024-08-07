#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "../Characters/FPSCharacter.h"
#include "../OSS.h"

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
			if (RedTeamPlayerStarts.Find(*It) == INDEX_NONE)
			{
				RedTeamPlayerStarts.Add(*It);
			}
		}
		else
		{
			if (BlueTeamPlayerStarts.Find(*It) == INDEX_NONE)
			{
				BlueTeamPlayerStarts.Add(*It);
			}
		}
	}

	LogOnScreen(this, "RedTeam : " + FString::FromInt(RedTeamPlayerStarts.Num()), FColor::Red);
	LogOnScreen(this, "BlueTeam : " + FString::FromInt(BlueTeamPlayerStarts.Num()), FColor::Blue);
}

void AFPSGameMode::PostLogin(APlayerController* NewPlayer)
{
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
	MoveToPlayerStart(PlayerCharacter, PS->Team);
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

		AFPSCharacter* NewPlayerCharacter = Controller->GetPawn<AFPSCharacter>();		// 무조건 리스타트 이후
		ACPlayerState* PS = Controller->GetPlayerState<ACPlayerState>();
		if (NewPlayerCharacter && PS)
		{
			NewPlayerCharacter->SetTeamColor(PS->Team);
			MoveToPlayerStart(NewPlayerCharacter, PS->Team);
		}
	}
}

void AFPSGameMode::MoveToPlayerStart(APawn* InPawn, ETeamType InTeam)
{
	ensure(InPawn);

	if (RedTeamPlayerStarts.Num() < 1 || BlueTeamPlayerStarts.Num() < 1)
	{
		StartPlay();
	}

	int32 Random = 0;
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;

	switch (InTeam)
	{
		case ETeamType::Red:
		{
			Random = UKismetMathLibrary::RandomInteger(RedTeamPlayerStarts.Num() - 1);
			Location = RedTeamPlayerStarts[Random]->GetActorLocation();
			Rotation = RedTeamPlayerStarts[Random]->GetActorRotation();
		}
		break;
		case ETeamType::Blue:
		{
			Random = UKismetMathLibrary::RandomInteger(BlueTeamPlayerStarts.Num() - 1);
			Location = BlueTeamPlayerStarts[Random]->GetActorLocation();
			Rotation = BlueTeamPlayerStarts[Random]->GetActorRotation();
		}
		break;
	}

	InPawn->SetActorLocation(Location);
	//InPawn->SetActorRotation(Rotation);		// bUsePawnControlRotation = true로 인해 사용 불가

	if (InPawn->GetController())
	{
		InPawn->GetController()->ClientSetRotation(Rotation, true);
	}
}
