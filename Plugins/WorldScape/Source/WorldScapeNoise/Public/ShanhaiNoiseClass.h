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
#include "ShanhaiNoiseClass.generated.h"

UCLASS(hideCategories = (Code))
class WORLDSCAPENOISE_API UShanhaiNoiseParameter : public UNoiseParameter
{

	GENERATED_BODY()

public:

	UShanhaiNoiseParameter();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
	float LandMass;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
	float LandMassScale;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
	float TemperatureOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
	float HumidityOffset;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		float WarpIntensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		float WarpScale;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		float Warp2Scale;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | PinYuan")
	float FlatLandIntensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | PinYuan")
	float FlatLandScale;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | ZhaoZe")
	float SwampIntensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | ZhaoZe")
	float SwampScale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | XueShan")
	float SnowMountainIntensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | XueShan")
	float SnowMountainScale;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | GaoYuan")
	float HighLandIntensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | GaoYuan")
	float HighLandScale;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | ShaMo")
	float DesertIntensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | ShaMo")
	float DesertScale;

};

// Multiple types of Biomes

class FlatLandBiome : BiomeNoise {
public:
	static float GetNoise(CustomNoise NoiseClass, FVector position);
	static float GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise);
};

class SwampBiome : BiomeNoise {
public:
	static float GetNoise(CustomNoise NoiseClass, FVector position);
	static float GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise);
};

class HighLandBiome : BiomeNoise {
public:
	static float GetNoise(CustomNoise NoiseClass, FVector position);
	static float GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise);
};

class SnowMountainBiome : BiomeNoise {
public:
	static float GetNoise(CustomNoise NoiseClass, FVector position);
	static float GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise);
};

class DesertBiome : BiomeNoise {
public:
	static float GetNoise(CustomNoise NoiseClass, FVector position);
	static float GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise);
};

UCLASS()
class WORLDSCAPENOISE_API UShanhaiNoise : public UWorldScapeNoiseClass {

	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UShanhaiNoiseParameter* NoiseParameters;

	FNoiseData GetNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition) override;
	FNoiseData GetOceanNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition) override;


	FNoiseData GetHeight(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition, double& LandMassNoise);
	UShanhaiNoise();
};

