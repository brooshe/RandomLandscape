// Copyright 2021 IOLACORP STUDIO. All Rights Reserved

#include "ShanhaiNoiseClass.h" 

const float MinMountainDistance = 1000.f;

UShanhaiNoiseParameter::UShanhaiNoiseParameter()
{
	
}

float FlatLandBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float FlatLandBiome::GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise)
{
	//total flatland inside 500, not flatland outside 600, smooth from 500 ~ 600
	return 1.f - FMath::SmoothStep(50000.f, 60000.f,position.Size2D());
	// return 1.f;
}


float HighLandBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float HighLandBiome::GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise)
{
	return 0.f;
}


float SwampBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float SwampBiome::GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise)
{
	return 0.f;
}

float SnowMountainBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	float hill = 0.85 * NoiseClass.Fractal(position * 3.5f, 5, 2.75, 0.48f) + 0.15 * NoiseClass.FractalRidge(position * 3.5f, 3, 2.5);
	float Mountain = powf(NoiseClass.FractalRidge(position * 0.55f, 5, 2.5), 4.5) * 2;
	return NoiseMathUtils::SmoothMax(hill, Mountain, 0.05f) * 0.075f + 0.01f;
	// return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float SnowMountainBiome::GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise)
{
	return FMath::SmoothStep(40000.f, 60000.f, position.Y) * NoiseMathUtils::Clamp01(powf(NoiseClass.Fractal(WarpedPosition * 0.0041543, 7, 3, 0.4f) * 1.35f, 3));
}

float DesertBiome::GetNoise(CustomNoise NoiseClass, FVector position)
{
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}
float DesertBiome::GetMask(CustomNoise NoiseClass, FVector position, FVector WarpedPosition, float LandMassNoise)
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

	const float FlatLandMask = NoiseMathUtils::Clamp01(FlatLandBiome::GetMask(NoiseClass, position, WarpedPosition, LandMassNoise));
	// if (FlatLandMask > 0)
	{
		Data.HeightNormalize += /*FlatLandMask */ (FlatLandBiome::GetNoise(NoiseClass, WarpedPosition*NoiseParameters->FlatLandScale) * GeneralNoise * NoiseParameters->FlatLandIntensity + 0.002f * powf(LandMassNoise, 0.25f));
	}

	const float HighLandMask = NoiseMathUtils::Clamp01(HighLandBiome::GetMask(NoiseClass, position, WarpedPosition, LandMassNoise));
	if (HighLandMask > 0)
	{
		Data.HeightNormalize += HighLandMask * HighLandBiome::GetNoise(NoiseClass, WarpedPosition*NoiseParameters->HighLandScale) * GeneralNoise * NoiseParameters->HighLandIntensity;
	}

	const float SwampMask = NoiseMathUtils::Clamp01(SwampBiome::GetMask(NoiseClass, position, WarpedPosition, LandMassNoise));
	if (SwampMask > 0)
	{
		Data.HeightNormalize += SwampMask * SwampBiome::GetNoise(NoiseClass, WarpedPosition*NoiseParameters->SwampScale) * GeneralNoise * NoiseParameters->SwampIntensity;
	}

	const float SnowMountainMask = NoiseMathUtils::Clamp01(SnowMountainBiome::GetMask(NoiseClass, position, WarpedPosition, LandMassNoise));
	if (SnowMountainMask > 0)
	{
		double noise = SnowMountainBiome::GetNoise(NoiseClass, WarpedPosition*NoiseParameters->SnowMountainScale) * GeneralNoise * NoiseParameters->SnowMountainIntensity + 0.005f * powf(LandMassNoise, 0.25f);
		Data.HeightNormalize = FMath::Lerp(Data.HeightNormalize, noise, SnowMountainMask);
	}
	Data.HeightNormalize = SnowMountainMask;

	const float DesertMask = NoiseMathUtils::Clamp01(DesertBiome::GetMask(NoiseClass, position, WarpedPosition, LandMassNoise));
	if (DesertMask > 0)
	{
		Data.HeightNormalize += DesertMask * DesertBiome::GetNoise(NoiseClass, WarpedPosition*NoiseParameters->DesertScale) * GeneralNoise * NoiseParameters->DesertIntensity;
	}

	const float TotalMask = FlatLandMask + HighLandMask + SwampMask + SnowMountainMask + DesertMask;
	// if(TotalMask > 1.f)
	// 	Data.HeightNormalize /= TotalMask;
	Data.HeightNormalize = GetBaseHeight(FVector2D(position.X, position.Y));
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

float UShanhaiNoise::GetBaseHeight(FVector2D position)
{
    position *= 0.01f; //cm to m
    float distance = position.Size();
    const float x = position.X;
    const float y = position.Y;
    float height = 0.f;
    height += FMath::PerlinNoise2D(position * 0.002f * 0.5f) * FMath::PerlinNoise2D(position * 0.003f * 0.5f) * 1.f;
    height += FMath::PerlinNoise2D(position * 0.002f * 1.f) * FMath::PerlinNoise2D(position * 0.003f * 1.f) * height * 0.9f;
    height += FMath::PerlinNoise2D(position * 0.005f * 1.f) * FMath::PerlinNoise2D(position * 0.01f * 1.f) * height * 0.5f;
    height -= 0.07f;
    
    float neighbor0 = FMath::PerlinNoise2D(FVector2D(x*0.002f*0.25f+0.123f, y*0.002*0.25f+0.15123f));
    float neighbor1 = FMath::PerlinNoise2D(FVector2D(x*0.002f*0.25f+0.321f, y*0.002*0.25f+0.231f));
    float alpha = 1.f - NoiseMathUtils::LerpStep(0.02f, 0.12f, FMath::Abs(neighbor0 - neighbor1));
    alpha *= FMath::SmoothStep(744.f, 1000.f, distance);
    height *= 1.f - alpha;
    if (distance < MinMountainDistance && height > 0.28f)
    {
    	float t = NoiseMathUtils::Clamp01((height - 0.28f) / 0.099999994f);
    	height = FMath::Lerp(FMath::Lerp(0.28f, 0.38f, t), height, NoiseMathUtils::LerpStep(MinMountainDistance - 400.f, MinMountainDistance, distance));
    }
    
    return height;

}
