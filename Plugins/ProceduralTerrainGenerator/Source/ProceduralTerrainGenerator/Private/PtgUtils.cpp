// Copyright 2021 VICTOR HERNANDEZ MOLPECERES (Rockam). All Rights Reserved.

#include "PtgUtils.h"
#include "ProceduralTerrainGenerator.h"
#include "Engine/Engine.h"

void UPtgUtils::PrintDebugMessage(const UObject* caller, const FString& msg, const EPtgDebugMessageTypes type, const float timeOnScreen)
{
	const FString composedMsg = (caller == nullptr) ? msg : caller->GetName() + TEXT(": ") + msg;
	FColor msgColor;

	// Choose message color and verbosity depending on type
	switch (type)
	{
	case EPtgDebugMessageTypes::Error:
		msgColor = FColor::Red;
		UE_LOG(LogProceduralTerrainGenerator, Error, TEXT("%s"), *composedMsg);
		break;

	case EPtgDebugMessageTypes::Warning:
		msgColor = FColor::Yellow;
		UE_LOG(LogProceduralTerrainGenerator, Warning, TEXT("%s"), *composedMsg);
		break;

	case EPtgDebugMessageTypes::Info:
	default:
		msgColor = FColor::Green;
		UE_LOG(LogProceduralTerrainGenerator, Log, TEXT("%s"), *composedMsg);
		break;
	}

#if !UE_BUILD_SHIPPING
	// Print to screen and log
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, timeOnScreen, msgColor, composedMsg);
	}
#endif
}
