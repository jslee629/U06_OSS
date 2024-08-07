#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CPlayerState.h"
#include "FPSGameMode.generated.h"

class APlayerStart;

UCLASS(minimalapi)
class AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSGameMode();

protected:
	virtual void StartPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	void OnActorKilled(AActor* VictimActor);

private:
	UFUNCTION()
	void RespawnPlayerElapsed(APlayerController* Controller);

	void MoveToPlayerStart(APawn* InPawn, ETeamType InTeam);

private:
	TArray<APlayerStart*> RedTeamPlayerStarts;
	TArray<APlayerStart*> BlueTeamPlayerStarts;

	TArray<APawn*> RedTeamPawns;
	TArray<APawn*> BlueTeamPawns;
};



