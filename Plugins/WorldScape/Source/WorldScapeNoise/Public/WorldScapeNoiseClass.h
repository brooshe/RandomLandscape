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
#include "NoiseMathUtils.h"
#include "NoiseData.h"
#include "WorldScapeNoiseClass.generated.h"





//Defaults Classes
UCLASS(hideCategories = (Code))
class WORLDSCAPENOISE_API UNoiseParameter : public UActorComponent
{
	GENERATED_BODY()
};

UCLASS(BlueprintType)
class WORLDSCAPENOISE_API UWorldScapeNoiseClass : public UObject 
{
	GENERATED_BODY()

public:


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		bool bNeedPlanetRefresh;

	virtual FNoiseData GetNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition)
		{ return FNoiseData(); };
	virtual FNoiseData GetOceanNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition) 
		{ return FNoiseData(); };

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif
};





UENUM(BlueprintType)
enum class EWorldGenerationType : uint8
{
	Earth 	UMETA(DisplayName = "Earth"),
	Moon	UMETA(DisplayName = "Moon")
};

UCLASS(hideCategories = (Code))
class WORLDSCAPENOISE_API UCustomNoiseParameter: public UNoiseParameter
{
	GENERATED_BODY()

public:
	
	UCustomNoiseParameter();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "General")
		EWorldGenerationType WorldType;


	//You can add any parameters you need for your planet


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

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | Example Biome A")
	float A_Intensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | Example Biome A")
	float A_Scale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | Example Biome B")
	float B_Multiplier;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | Example Biome B")
	float B_Intensity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome | Example Biome B")
	float B_Scale;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterFrequency;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		int CraterOctave;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterLacunarity;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterPersistence;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterHeightMultiplier;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterSize;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterRimHeight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterRimOctaveLimit;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		int CraterRimPower;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterWarp;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		float CraterWarpFrequency;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Moon")
		int CraterWarpOctave;
};


//
class BiomeNoise 
{
public :
	static float GetNoise(CustomNoise NoiseClass,FVector position, float LandMassNoise, float PlanetScale = 0, FVector PlanetPosition = FVector(0, 0, 0), int octave = 5);
	static float GetMask(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale = 0, FVector PlanetPosition = FVector(0, 0, 0), int octave = 5);
};

class ExampleANoise : BiomeNoise 
{
public:
	static float GetNoise(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale = 0, FVector PlanetPosition = FVector(0, 0, 0), int octave = 5);
	static float GetMask(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale = 0, FVector PlanetPosition = FVector(0, 0, 0), int octave = 5);
};
class ExampleBNoise : BiomeNoise 
{
public:
	static float GetNoise(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale = 0, FVector PlanetPosition = FVector(0, 0, 0), int octave = 5);
	static float GetMask(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale = 0, FVector PlanetPosition = FVector(0, 0, 0), int octave = 5);
};

class CraterGeneration 
{
public:
	static float CavityShape(float distance);
	static float RimShape(float distance, float rimSteepness, int power);
	static float CraterShape(float distance, float rimSteepness, float floorHeight, float smoothness, int rimPower);


	//TODO : Replace NoiseParameter by a struct to gain flexibility
	static float GetCrater(CustomNoise NoiseClass, FVector position, UCustomNoiseParameter* NoiseParam, int SizeGeneration);
	static float GetCraterFractal(CustomNoise NoiseClass, FVector position, UCustomNoiseParameter* NoiseParam, int octave, float lacunarity, float persistence, bool Min = false);
};




UCLASS()
class WORLDSCAPENOISE_API UWorldScapeCustomNoise : public UWorldScapeNoiseClass 
{

	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UCustomNoiseParameter* NoiseParameters;

	FNoiseData GetHeight(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition, double& LandMassNoise);

	FNoiseData GetNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition) override;
	FNoiseData GetOceanNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition) override;
	

	UWorldScapeCustomNoise();
};

