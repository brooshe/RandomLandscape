// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "Components/BoxComponent.h"
#include "VolumeInterface.h" 
#include "FoliageMaskVolume.generated.h"

UCLASS(hideCategories = (Code))
class WORLDSCAPEVOLUME_API AFoliageMaskVolume : public AVolumeInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFoliageMaskVolume();



	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defaults")
		TArray<int> FoliageLayerMask;

	TArray<int> PREVIOUS_FoliageLayerMask;

	virtual void OnConstruction(const FTransform& Transform) override;
};