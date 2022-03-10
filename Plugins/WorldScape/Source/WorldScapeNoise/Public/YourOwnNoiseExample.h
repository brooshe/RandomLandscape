// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Stats/Stats.h"
#include "Math/RandomStream.h"
#include "CustomNoise.h"
#include "UObject/Object.h"
#include "Components/ActorComponent.h"
#include <string>
#include "NoiseData.h"
#include "WorldScapeNoiseClass.h"
#include "YourOwnNoiseExample.generated.h"

UCLASS(hideCategories = (Code))
class WORLDSCAPENOISE_API UYourNoiseParameter : public UNoiseParameter
{

	GENERATED_BODY()

public:

	UYourNoiseParameter();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		float ExampleParameter;

};


class AnotherExampleBiome : BiomeNoise {
public:
	static float GetNoise(CustomNoise NoiseClass, FVector position);
};

UCLASS()
class WORLDSCAPENOISE_API UYourOwnNoiseExample : public UWorldScapeNoiseClass {

	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UYourNoiseParameter* NoiseParameters;

	FNoiseData GetNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition) override;
	FNoiseData GetOceanNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition) override;


	FNoiseData GetHeight(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition, double& LandMassNoise);
	UYourOwnNoiseExample();
};

