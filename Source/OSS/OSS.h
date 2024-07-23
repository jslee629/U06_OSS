// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

static void LogOnScreen(UObject* WorldContext, FString Msg, FColor Color = FColor::White, float Duration = 5.f)
{
	if (!ensure(WorldContext))
	{
		return;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FString Prefix = World->IsNetMode(ENetMode::NM_Client) ? "[Client] " : "[Server] ";

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Prefix + Msg);
	}
}