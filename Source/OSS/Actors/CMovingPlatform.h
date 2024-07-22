#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "CMovingPlatform.generated.h"

UCLASS()
class OSS_API ACMovingPlatform : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	ACMovingPlatform();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "MoveSpeed")
	float Speed;
	UPROPERTY(EditAnywhere, Category = "TargetLocation", meta = (MakeEditWidget))
	FVector TargetLS;

private:
	FVector StartWS;
	FVector TargetWS;
};
