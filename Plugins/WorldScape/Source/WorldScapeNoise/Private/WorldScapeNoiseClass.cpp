// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "WorldScapeNoiseClass.h" 


#if WITH_EDITOR
void UWorldScapeNoiseClass::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);
	bNeedPlanetRefresh = true;
}
#endif


UCustomNoiseParameter::UCustomNoiseParameter() {

	WorldType = EWorldGenerationType::Earth;

	LandMass = 1;
	LandMassScale = 1;

	WarpIntensity = 0.1f;
	WarpScale = 10;
	Warp2Scale = 100;

	A_Intensity = 1;
	A_Scale = 1;

	B_Multiplier = 1;
	B_Intensity = 1;
	B_Scale = 1;

	CraterSize = 8;
	CraterFrequency = 1;
	CraterOctave = 8;
	CraterLacunarity = 1.25f;
	CraterPersistence = 0.75f;
	CraterHeightMultiplier = 1;
	CraterRimHeight = 15;
	CraterRimOctaveLimit = 3;
	CraterRimPower = 4;
	CraterWarp = 1;
	CraterWarpFrequency = 1;
	CraterWarpOctave = 4;
}







// CUSTOM NOISE
float ExampleANoise::GetNoise(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale, FVector PlanetPosition, int octave) {


	float hill = 0.85 * NoiseClass.Fractal(position * 2.5f, 5, 2.75, 0.48f) + 0.15 * NoiseClass.FractalRidge(position * 3.5f, 3, 2.5);
	return hill * 0.075f + 0.01f;
}
float ExampleANoise::GetMask(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale, FVector PlanetPosition, int octave) {
	return 1;
}

float ExampleBNoise::GetNoise(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale, FVector PlanetPosition, int octave) {
	float hill = 0.85 * NoiseClass.Fractal(position * 3.5f, 5, 2.75, 0.48f) + 0.15 * NoiseClass.FractalRidge(position * 3.5f, 3, 2.5);
	float Mountain = powf(NoiseClass.FractalRidge(position * 0.55f, 5, 2.5), 4.5) * 2;
	return NoiseMathUtils::SmoothMax(hill, Mountain, 0.05f) * 0.075f + 0.01f;
}
float ExampleBNoise::GetMask(CustomNoise NoiseClass, FVector position, float LandMassNoise, float PlanetScale, FVector PlanetPosition, int octave) {
	return NoiseMathUtils::Clamp01(powf(NoiseClass.Fractal(position * 0.0041543, 7, 3, 0.4f) * 1.35f, 3));
}

//Crater Noise Generation for the moon
float CraterGeneration::CavityShape(float distance)
{
	return distance * distance;

}
float CraterGeneration::RimShape(float distance, float rimSteepness, int power)
{
	distance = distance - 1;
	if (power == 1)
		return 1 + rimSteepness * distance;
	if (power == 2)
		return 1 + rimSteepness * distance * distance;
	if (power == 3)
		return 1 + rimSteepness * distance * distance * distance;
	if (power == 4)
		return 1 + rimSteepness * distance * distance * distance * distance;
	if (power == 5)
		return 1 + rimSteepness * distance * distance * distance * distance * distance;

	return 1 + rimSteepness * powf(distance, power);

}
float CraterGeneration::CraterShape(float distance, float rimSteepness, float Height, float smoothness, int rimPower)
{

	return NoiseMathUtils::SmoothMax(NoiseMathUtils::SmoothMin(CavityShape(distance * 2.5f), RimShape(distance, rimSteepness, rimPower), smoothness), Height, smoothness);
}
float CraterGeneration::GetCrater(CustomNoise NoiseClass, FVector position, UCustomNoiseParameter* NoiseParam, int SizeGeneration)
{


	float warp = 0;
	if (NoiseParam->CraterWarpOctave > 1)
		warp = NoiseClass.Fractal(position * 5 * NoiseParam->CraterWarpFrequency, NoiseParam->CraterWarpOctave);
	else
		warp = NoiseClass.Noise_01(position * 5 * NoiseParam->CraterWarpFrequency);
	FVector WapredPosition = position + FVector(warp, warp, warp) * 0.05f * NoiseParam->CraterWarp;

	float CraterDistance = NoiseMathUtils::Clamp01(NoiseClass.Noise_01(WapredPosition, ENoiseType::Cellular, ECellularDistanceType::EuclideanSq, ECellularType::Distance, 0.5f) * NoiseParam->CraterSize);
	float CraterAge = NoiseClass.Noise(WapredPosition, ENoiseType::Cellular, ECellularDistanceType::EuclideanSq, ECellularType::CellValue, 0.5f);

	float CraterRim = NoiseParam->CraterRimHeight;
	if (NoiseParam->CraterRimOctaveLimit < SizeGeneration) CraterRim = 0;


	return CraterShape(CraterDistance, (0.15f + (1 - CraterAge) * 0.15f) * CraterRim, 0.15f + 0.35f * CraterAge, 0.025f + 0.15f * powf(1 - CraterAge, 2), NoiseParam->CraterRimPower) * NoiseParam->CraterHeightMultiplier;
}
float CraterGeneration::GetCraterFractal(CustomNoise NoiseClass, FVector position, UCustomNoiseParameter* NoiseParam, int octave, float lacunarity, float persistence, bool Min)
{

	if (Min) {
		float noise = 1;
		float intensity = 1;
		float scale = 1;

		for (int i = 0; i < octave; i++) {
			noise = NoiseMathUtils::SmoothMin(noise, (1 - intensity) + (intensity * GetCrater(NoiseClass, position * scale, NoiseParam, i)), 0.1f);
			intensity *= persistence;
			scale *= lacunarity;
		}

		return noise;
	}
	else {
		float noise = 0;
		float intensity = 1;
		float scale = 1;
		float noiseRemap = 0;

		for (int i = 0; i < octave; i++) {
			noise += (intensity * GetCrater(NoiseClass, position * scale, NoiseParam, i));
			noiseRemap += intensity;
			intensity *= persistence;
			scale *= lacunarity;
		}

		return noise;
	}


}




FNoiseData UWorldScapeCustomNoise::GetHeight(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition, double& LandMassNoise)
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


	if (FlatWorld){
		Data.Temperature = NoiseClass.Fractal(WarpedPosition * 0.028143f, 6);
		Data.Humidity = NoiseClass.Fractal(WarpedPosition * 0.032537f, 6);
	}
	else{
		//Humidity and temperature based on planet latitude, you can use these value for terrain generation (using them as mask for biome) 
	//and for texture (they are saved as Green (temperature) and Blue (Humidity) in the vertex Color of the terrain Mesh;
	//Temperature based on Planet Latitude;
		Data.Temperature = powf(lattitude, 1.25f) + NoiseParameters->TemperatureOffset;
		Data.Temperature *= 0.8f + 0.2f * NoiseClass.Fractal(WarpedPosition * 0.028143f, 6);

		//Humidity based on Planet Latitude;
		Data.Humidity = NoiseMathUtils::SmoothMax(-powf(lattitude, 3) + 0.75f, powf(lattitude, 10), 0.2f) + NoiseParameters->HumidityOffset;
		Data.Humidity *= 0.25f + 0.75f * NoiseClass.Fractal(WarpedPosition * 0.032537f, 6);
	}
	


	//Adding a bit of fractal noise to get Nice Continent
	LandMassNoise = NoiseClass.Fractal(WarpedPosition * 0.001f * NoiseParameters->LandMassScale, 6, 4.f, 0.30f) * 1.5f;
	//LandMassNoise -= powf(NoiseClass.Fractal(WarpedPosition * 0.01f, 5), 3.5) * 0.565f;


	//Generating Biome Mask
	float BiomeBMask= NoiseMathUtils::Clamp01(ExampleBNoise::GetMask(NoiseClass, WarpedPosition, LandMassNoise, PlanetScale, PlanetPosition) * NoiseParameters->B_Multiplier);

	Data.Temperature = Data.Temperature * (0.25f + 0.75f * LandMassNoise);
	Data.Humidity = Data.Humidity * (0.25f + 0.75f * (1 - LandMassNoise));

	//Applying LandMass value multiplier from parameter
	LandMassNoise = NoiseMathUtils::DClamp( LandMassNoise* NoiseParameters->LandMass,0.,1.);

	//Generating Base Biome
	double HillNoise = ExampleANoise::GetNoise(NoiseClass, WarpedPosition * NoiseParameters->A_Scale, LandMassNoise, PlanetScale, PlanetPosition) * GeneralNoise * NoiseParameters->A_Intensity + 0.002f * powf(LandMassNoise, 0.25f);


	//Use HillBiome as Base
	Data.HeightNormalize = HillNoise;



	if (BiomeBMask > 0) {
		double ValleyNoise = ExampleBNoise::GetNoise(NoiseClass, WarpedPosition * NoiseParameters->B_Scale, LandMassNoise, PlanetScale, PlanetPosition) * NoiseParameters->B_Intensity * GeneralNoise + 0.005f * powf(LandMassNoise, 0.25f);
		Data.HeightNormalize = FMath::Lerp(Data.HeightNormalize, ValleyNoise, BiomeBMask);
		//Biome B have higger Humidity and temperature value than Biome A so we edit that
		Data.Humidity = NoiseMathUtils::Clamp01(Data.Humidity + powf(BiomeBMask, 7.5f) * 0.8f);
		Data.Temperature = NoiseMathUtils::Clamp01(Data.Temperature + powf(BiomeBMask, 7.5f) * 0.25f);
	}

	if (LandMassNoise > 0)
		LandMassNoise = FMath::Lerp(LandMassNoise, NoiseMathUtils::DClamp(((LandMassNoise - 0.5) * 10000.0), 0, 1.), NoiseMathUtils::Clamp01(powf(NoiseClass.Fractal(NoisePosition * 0.0834151f, 5) * 1.2f, 8)));

	LandMassNoise = LandMassNoise / 2 + 0.5;


	//Should Allow for more beach and keep some cliff
	Data.HeightNormalize = FMath::Lerp(Data.HeightNormalize * (float)LandMassNoise, Data.HeightNormalize, NoiseClass.Fractal(NoisePosition * 0.005f, 6, 4.f, 0.4f));


	
	//Adding a bonus very small noise to avoid terrain to be too flat
	float verysmallgroundNoise = NoiseClass.Fractal(NoisePosition * 512,3) * 0.0005;
	Data.HeightNormalize += verysmallgroundNoise;

	//Add Beach
	double BeachNoise = NoiseClass.FractalRidge(NoisePosition * 64, 3) * 0.002 + 0.002;
	Data.HeightNormalize = FMath::Lerp(Data.HeightNormalize, BeachNoise, pow(NoiseMathUtils::DClamp(1 - abs((LandMassNoise * 4) - 2.1)), 2));


	//Higger altitude lead to lower temperature
	Data.Temperature = NoiseMathUtils::Clamp(Data.Temperature - powf(NoiseMathUtils::Clamp01(Data.HeightNormalize - 0.05f) * 6.5f, 1.2f), 0, Data.Temperature);


	//Add OceanBiome
	Data.HeightNormalize = FMath::Lerp(Data.HeightNormalize * .2 - 0.5, Data.HeightNormalize, pow(LandMassNoise,3));




	return Data;
}
FNoiseData UWorldScapeCustomNoise::GetNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition)
{
	double LandMassNoise;
	FNoiseData Data = FNoiseData();

	//Noise scale with the planet, to avoid issue if the position value is too high
	NoisePosition = (position / PlanetScale) * NoiseScale;

	switch (NoiseParameters->WorldType)
	{
	case EWorldGenerationType::Earth:
		Data = GetHeight(NoiseClass, position, PlanetPosition, NoiseScale, NoiseIntensity, PlanetScale, FlatWorld, lattitude, NoisePosition, LandMassNoise);
		break;
	case EWorldGenerationType::Moon:
		Data.HeightNormalize = CraterGeneration::GetCraterFractal(NoiseClass, NoisePosition * 0.002f * NoiseParameters->CraterFrequency, NoiseParameters, NoiseParameters->CraterOctave, NoiseParameters->CraterLacunarity, NoiseParameters->CraterPersistence);
		Data.HeightNormalize *= 0.9f + 0.05f * ExampleANoise::GetNoise(NoiseClass, NoisePosition * 0.1f, 1, PlanetScale, PlanetPosition) + 0.05f * CraterGeneration::GetCraterFractal(NoiseClass, NoisePosition * 0.5f * NoiseParameters->CraterFrequency, NoiseParameters, NoiseParameters->CraterOctave, NoiseParameters->CraterLacunarity, NoiseParameters->CraterPersistence);;
		break;
	default:
		break;
	}

	Data.Height = Data.HeightNormalize * NoiseIntensity;

	return Data;

}
FNoiseData UWorldScapeCustomNoise::GetOceanNoise(CustomNoise NoiseClass, FVector position, FVector PlanetPosition, float NoiseScale, float NoiseIntensity, float PlanetScale, bool FlatWorld, float lattitude, FVector& NoisePosition)
{


	//Noise scale with the planet, to avoid issue if the position value is too high
	NoisePosition = (position / PlanetScale) * NoiseScale;

	FNoiseData Data = FNoiseData();
	switch (NoiseParameters->WorldType)
	{
	case EWorldGenerationType::Earth:
		Data.HeightNormalize = 0;
		break;
	default:
		break;
	}


	Data.Height = Data.HeightNormalize * NoiseIntensity;

	return Data;

}


UWorldScapeCustomNoise::UWorldScapeCustomNoise() {
	NoiseParameters = CreateDefaultSubobject<UCustomNoiseParameter>(TEXT("Noise Parameters"));
}