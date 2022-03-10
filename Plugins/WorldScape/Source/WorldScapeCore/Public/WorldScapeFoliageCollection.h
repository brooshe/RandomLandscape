// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"

#include <random>
#include <limits>
#include "Kismet/KismetMathLibrary.h"
#include "WorldScapeFoliage.h"
#include "WorldScapeFoliageCollection.generated.h"

UCLASS(ClassGroup = WorldScape, Category = "WorldScape",editinlinenew, BlueprintType, Blueprintable)
class WORLDSCAPECORE_API UWorldScapeFoliageCollection : public UObject
{
	GENERATED_BODY()

public:
	// The foliage's mesh
	UPROPERTY(EditAnywhere, Category = "Foliages", meta = (DisplayThumbnail = "true"))
		TArray<UWorldScapeFoliage*> FoliageList;



	// The minimum elevation this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MinElevation = -200000.0f;
	// The maximum elevation this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MaxElevation = 200000.0f;
	// The minimum Temperature this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MinTemperature = 0.5f;
	// The maximum Temperature this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MaxTemperature = 1.f;
	// The minimum Humidity this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MinHumidity = 0.5f;
	// The maximum Humidity this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MaxHumidity = 1.f;
	// The minimum Humidity this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MinSlope = 0.f;
	// The maximum Humidity this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement")
		float MaxSlope = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Placement")
		bool bSpawnInWater = false;
	UPROPERTY(EditAnywhere, Category = "Placement")
		int FoliageLayer = 1;

protected:
	UPROPERTY(VisibleAnywhere, Category = WorldScapeFoliage)
		FString Description;

	UPROPERTY(VisibleAnywhere, Category = WorldScapeFoliage)
		bool bIsActive;
};
