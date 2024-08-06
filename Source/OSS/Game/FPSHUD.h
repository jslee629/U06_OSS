#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FPSHUD.generated.h"

class UCPlayerStatusWidget;

UCLASS()
class AFPSHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFPSHUD();

	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;

private:
	class UTexture2D* CrosshairTex;

//Player Status Widget
private:
	TSubclassOf<UCPlayerStatusWidget> PlayerStatusWidgetClass;

	UPROPERTY()
	UCPlayerStatusWidget* PlayerStatusWidget;
};

