// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map2GameMode.h"
#include "Map2Pawn.h"
#include "Map2Hud.h"

AMap2GameMode::AMap2GameMode()
{
	DefaultPawnClass = AMap2Pawn::StaticClass();
	HUDClass = AMap2Hud::StaticClass();
}
