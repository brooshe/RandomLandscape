// Copyright 2021 IOLACORP STUDIO. All Rights Reserved

#include "YourOwnNoiseExample.h" 


UYourNoiseParameter::UYourNoiseParameter() {
	ExampleParameter = 1;
}

float AnotherExampleBiome::GetNoise(CustomNoise NoiseClass, FVector position) {
	return NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f);
}

FNoiseData UYourOwnNoiseExample::GetHeight(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition, double& LandMassNoise)
{


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
	Data.HeightNormalize = AnotherExampleBiome::GetNoise(NoiseClass,NoisePosition) * NoiseParameters->ExampleParameter;
	Data.FoliageMask = 1;



	return Data;
}

FNoiseData UYourOwnNoiseExample::GetNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition)
{
	double LandMassNoise;
	FNoiseData Data = FNoiseData();
	//Noise scale with the planet, to avoid issue if the position value is too high
	NoisePosition = (position / PlanetScale) * NoiseScale;

	Data = GetHeight(NoiseClass, position, PlanetPosition, NoiseScale, NoiseIntensity, PlanetScale, FlatWorld, lattitude, NoisePosition, LandMassNoise);

	Data.Height = Data.HeightNormalize * NoiseIntensity;

	return Data;

}


FNoiseData UYourOwnNoiseExample::GetOceanNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition)
{
	FNoiseData Data = FNoiseData();

	Data.HeightNormalize = 0;
	Data.Height = Data.HeightNormalize * NoiseIntensity;

	return Data;

}

UYourOwnNoiseExample::UYourOwnNoiseExample() {
	NoiseParameters = CreateDefaultSubobject<UYourNoiseParameter>(TEXT("Noise Parameters"));
}