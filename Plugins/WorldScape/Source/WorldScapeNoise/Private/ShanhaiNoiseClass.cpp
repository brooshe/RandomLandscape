// Copyright 2021 IOLACORP STUDIO. All Rights Reserved

#include "ShanhaiNoiseClass.h" 


UShanhaiNoiseParameter::UShanhaiNoiseParameter()
{
	
}

float FlatLandBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float FlatLandBiome::GetMask(CustomNoise NoiseClass, FVector position)
{
	return 1.f;
}


float HighLandBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float HighLandBiome::GetMask(CustomNoise NoiseClass, FVector position)
{
	return 0.f;
}


float SwampBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float SwampBiome::GetMask(CustomNoise NoiseClass, FVector position)
{
	return 0.f;
}

float SnowMountainBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float SnowMountainBiome::GetMask(CustomNoise NoiseClass, FVector position)
{
	return 0.f;
}

float DesertBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float DesertBiome::GetMask(CustomNoise NoiseClass, FVector position)
{
	return 0.f;
}


FNoiseData UShanhaiNoise::GetHeight(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition, double& LandMassNoise)
{
	float warp = NoiseClass.Noise_01(NoisePosition * NoiseParameters->Warp2Scale);
	FVector WarpedPosition = NoisePosition * NoiseParameters->WarpScale + FVector(warp, warp, warp) * NoiseParameters->WarpIntensity;

	float GeneralNoise = NoiseClass.Noise_01(NoisePosition * 0.005f);

	/*
	**	FNoiseData, is a struct to register every data you want from the noise generation, you can edit them as you wish and use the param here, 
	* 
	**	the only parametter the PlanetRoot is using are : 
	**	HeightNormalize		- Height value in a range of 0 to 1, used to register the height value in the Red Vertex Color.
	**	Height;				- Height value in Unreal Unit, allow to get the real height of the ground.
	**	Temperature;		- Temperature from a range of 0 to 1.
	**	Humidity;			- Humidity from a range of 0 to 1.
	**	FoliageMask			- Used for foliage densitty (not used right now but will be is near future).
	*/
	FNoiseData Data = FNoiseData();
	Data.Humidity = 0.5;
	Data.Temperature = 0.5;
	Data.HeightNormalize = 0;
	Data.FoliageMask = 1;


	//Adding a bit of fractal noise to get Nice Continent
	LandMassNoise = NoiseClass.Fractal(WarpedPosition * 0.001f * NoiseParameters->LandMassScale, 6, 4.f, 0.30f) * 1.5f;

	const float FlatLandMask = NoiseMathUtils::Clamp01(FlatLandBiome::GetMask(NoiseClass, position));
	if (FlatLandMask > 0)
	{
		Data.HeightNormalize += FlatLandMask * FlatLandBiome::GetNoise(NoiseClass, WarpedPosition) * NoiseParameters->FlatLandIntensity;
	}

	const float HighLandMask = NoiseMathUtils::Clamp01(HighLandBiome::GetMask(NoiseClass, position));
	if (HighLandMask > 0)
	{
		Data.HeightNormalize += HighLandMask * HighLandBiome::GetNoise(NoiseClass, WarpedPosition) * NoiseParameters->HighLandIntensity;
	}

	const float SwampMask = NoiseMathUtils::Clamp01(SwampBiome::GetMask(NoiseClass, position));
	if (SwampMask > 0)
	{
		Data.HeightNormalize += SwampMask * SwampBiome::GetNoise(NoiseClass, WarpedPosition) * NoiseParameters->SwampIntensity;
	}

	const float SnowMountainMask = NoiseMathUtils::Clamp01(SnowMountainBiome::GetMask(NoiseClass, position));
	if (SnowMountainMask > 0)
	{
		Data.HeightNormalize += SnowMountainMask * SnowMountainBiome::GetNoise(NoiseClass, WarpedPosition) * NoiseParameters->SnowMountainIntensity;
	}

	const float DesertMask = NoiseMathUtils::Clamp01(DesertBiome::GetMask(NoiseClass, position));
	if (DesertMask > 0)
	{
		Data.HeightNormalize += DesertMask * DesertBiome::GetNoise(NoiseClass, WarpedPosition) * NoiseParameters->DesertIntensity;
	}
	return Data;
}

FNoiseData UShanhaiNoise::GetNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition)
{
	double LandMassNoise;
	FNoiseData Data = FNoiseData();
	//Noise scale with the planet, to avoid issue if the position value is too high
	NoisePosition = (position / PlanetScale) * NoiseScale;

	Data = GetHeight(NoiseClass, position, PlanetPosition, NoiseScale, NoiseIntensity, PlanetScale, FlatWorld, lattitude, NoisePosition, LandMassNoise);

	Data.Height = Data.HeightNormalize * NoiseIntensity;

	return Data;

}


FNoiseData UShanhaiNoise::GetOceanNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition)
{
	FNoiseData Data = FNoiseData();

	Data.HeightNormalize = 0;
	Data.Height = Data.HeightNormalize * NoiseIntensity;

	return Data;

}

UShanhaiNoise::UShanhaiNoise()
{
	NoiseParameters = CreateDefaultSubobject<UShanhaiNoiseParameter>(TEXT("Noise Parameters"));
}