#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSessionRowWidget.generated.h"

class UTextBlock;
class UCMainMenuWidget;
class UButton;

UCLASS()
class OSS_API UCSessionRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(UCMainMenuWidget* InParentWidget, uint32 InSelfIndex);

private:
	UFUNCTION()
	void OnClicked();

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SessionName; 
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HostName; 
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNumber; 

private:
	UPROPERTY(meta = (BindWidget))
	UButton* RowButton;

public:
	UPROPERTY(BlueprintReadOnly)
	bool bEverClicked;

private:
	UCMainMenuWidget* ParentWidget;
	uint32 SelfIndex;
};
