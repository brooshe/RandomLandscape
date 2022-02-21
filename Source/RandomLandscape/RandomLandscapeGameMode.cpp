// Copyright Epic Games, Inc. All Rights Reserved.

#include "RandomLandscapeGameMode.h"
#include "RandomLandscapePawn.h"

ARandomLandscapeGameMode::ARandomLandscapeGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ARandomLandscapePawn::StaticClass();
}
