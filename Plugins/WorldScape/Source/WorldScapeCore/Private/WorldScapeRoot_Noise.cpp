// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "WorldScapeRoot.h"


FNoiseData AWorldScapeRoot::GetGroundNoise(FVector Position, bool Water)
{
	TArray<AHeightMapVolume*> VolumeListVar = GetHeightMapVolumeList(Position);
	TArray<ANoiseVolume*> NoiseVolumeListVar = GetNoiseVolumeList(Position);
	TArray<FTransform> VolumeTransformArray;
	for (int32 i = 0; i < VolumeListVar.Num(); i++)
	{
		if (VolumeListVar.IsValidIndex(i) && IsValid(VolumeListVar[i]))
		{
			VolumeTransformArray.Add(VolumeListVar[i]->GetTransform());
		}
	}
	return GetGroundNoise(Position, NoiseVolumeListVar, VolumeListVar, VolumeTransformArray, Water);
}

FNoiseData AWorldScapeRoot::GetGroundNoise(FVector Position, TArray<ANoiseVolume*> NoiseVolumeListVar, TArray<AHeightMapVolume*> VolumeListVar, TArray<FTransform> VolumeTransformList, bool Water)
{
	Position = WorldScapeWorld::GetHeightZeroPosition(Position, PlanetScaleCode, bFlatWorld).ToFVector();
	return GetNoise(Position, NoiseVolumeListVar, VolumeListVar, VolumeTransformList, Water);
}


FNoiseData AWorldScapeRoot::GetNoise(FVector Position, bool Water)
{
	TArray<AHeightMapVolume*> VolumeListVar = GetHeightMapVolumeList(Position);
	TArray<ANoiseVolume*> NoiseVolumeListVar = GetNoiseVolumeList(Position);
	TArray<FTransform> VolumeTransformArray;

	for (int32 i = 0; i < VolumeListVar.Num(); i++)
	{
		if (VolumeListVar.IsValidIndex(i) && IsValid(VolumeListVar[i]))
		{
			VolumeTransformArray.Add(VolumeListVar[i]->GetTransform());
		}
	}
	return GetNoise(Position, NoiseVolumeListVar, VolumeListVar, VolumeTransformArray, Water);
}

FNoiseData AWorldScapeRoot::GetNoise(FVector Position, TArray<ANoiseVolume*> NoiseVolumeListVar, TArray<AHeightMapVolume*> VolumeListVar, TArray<FTransform> VolumeTransformList, bool Water)
{
	FVector PositionWorldTransformedPosition;
	FVector PositionTransformedPosition;
	FVector PositionNormal = Position;
	PositionNormal.Normalize();
	FVector NoisePosition;

	FNoiseData Data = FNoiseData();
	FNoiseData TempData = FNoiseData();
	float lattitude = GetLattitude(Position);

	FVector SeedOffset = FVector(0);
	if (bFlatWorld) SeedOffset = WorldScapeHelper::GetSeedOffset(Seed);

	if (!IsValid(WorldScapeNoise))
	{
		return Data;
	}
	if (Water)
	{
		Data = WorldScapeNoise->GetOceanNoise(PlanetNoise, Position + NoiseOffset + SeedOffset, PlanetLocation.ToFVector(), NoiseScale, NoiseIntensity, PlanetScaleCode, bFlatWorld, lattitude, NoisePosition);
		if (Data.Height < OceanHeight) Data.Height = OceanHeight;
	}
	else
		Data = WorldScapeNoise->GetNoise(PlanetNoise, Position + NoiseOffset + SeedOffset, PlanetLocation.ToFVector(), NoiseScale, NoiseIntensity, PlanetScaleCode, bFlatWorld, lattitude, NoisePosition);

	if (!(VolumeListVar.Num() > 0 || NoiseVolumeListVar.Num() > 0))
	{
		return Data;
	}

	FVector Normal = Position;
	Normal.Normalize();

	if (bFlatWorld) {
		Normal = FVector(0, 0, 1);
	}

	PositionWorldTransformedPosition = Position + Normal * Data.Height;
	FVector PlanetSurfacePos = PositionWorldTransformedPosition;
	PositionWorldTransformedPosition = ECEFToWorld(PositionWorldTransformedPosition).ToFVector();


	if (NoiseVolumeListVar.Num() > 0)
	{
		for (int32 l = 0; l < NoiseVolumeListVar.Num(); l++)
		{
			if (!(NoiseVolumeListVar.IsValidIndex(l) && IsValid(NoiseVolumeListVar[l])))
			{
				continue;
			}
			ANoiseVolume* ActualNoiseVolume = NoiseVolumeListVar[l];

			float MaxDistance = (ActualNoiseVolume->GetActorScale().X + ActualNoiseVolume->GetActorScale().Y + ActualNoiseVolume->GetActorScale().Z) * 2;

			if ((PositionWorldTransformedPosition - ActualNoiseVolume->GetActorLocation()).Size() > MaxDistance) continue;


			PositionTransformedPosition = ActualNoiseVolume->GetActorTransform().InverseTransformPositionNoScale(PositionWorldTransformedPosition);

			if (WorldScapeHelper::IsPointInCube(PositionTransformedPosition, FVector(0, 0, 0), ActualNoiseVolume->GetActorScale() * 100))
			{
				FVector VolumeRelative = PositionTransformedPosition;
				VolumeRelative /= ActualNoiseVolume->GetActorScale() * 100;
				VolumeRelative += FVector(0.5f, 0.5f, 0.5f);

				if (bFlatWorld) SeedOffset = WorldScapeHelper::GetSeedOffset(ActualNoiseVolume->Seed);

				if (Water) {
					TempData = ActualNoiseVolume->WorldScapeNoise->GetOceanNoise(ActualNoiseVolume->CustomVolumeNoise, Position + ActualNoiseVolume->NoiseOffset + SeedOffset, PlanetLocation.ToFVector(), ActualNoiseVolume->NoiseScale, ActualNoiseVolume->NoiseIntensity, PlanetScaleCode, bFlatWorld, lattitude, NoisePosition);
				}
				else {
					TempData = ActualNoiseVolume->WorldScapeNoise->GetNoise(ActualNoiseVolume->CustomVolumeNoise, Position + ActualNoiseVolume->NoiseOffset + SeedOffset, PlanetLocation.ToFVector(), ActualNoiseVolume->NoiseScale, ActualNoiseVolume->NoiseIntensity, PlanetScaleCode, bFlatWorld, lattitude, NoisePosition);
					TempData.Height += ActualNoiseVolume->NoiseHeightOffset;
				}
				float edgefalloff = WorldScapeHelper::GetFalloff(VolumeRelative.X, ActualNoiseVolume->EdgeFalloff) * WorldScapeHelper::GetFalloff(VolumeRelative.Y, ActualNoiseVolume->EdgeFalloff) * WorldScapeHelper::GetFalloff(VolumeRelative.Z, ActualNoiseVolume->EdgeFalloff);
				Data = FNoiseData::LerpData(Data, TempData, edgefalloff);
			}
		}
	}

	PositionWorldTransformedPosition = Position + Normal * Data.Height;
	PositionWorldTransformedPosition = ECEFToWorld(PositionWorldTransformedPosition).ToFVector();


	if (VolumeListVar.Num() > 0)
	{
		for (int32 j = 0; j < VolumeListVar.Num(); j++) {

			if (!(VolumeListVar.IsValidIndex(j) && IsValid(VolumeListVar[j]) && VolumeTransformList.IsValidIndex(j)))
			{
				continue;
			}
			AHeightMapVolume* ActualVolume = VolumeListVar[j];
			float VolumeSize = VolumeTransformList[j].GetScale3D().X * 50;

			

			if ((PositionWorldTransformedPosition - VolumeTransformList[j].GetLocation()).Size() > VolumeSize * 5)
			{
				continue;
			}

			PositionTransformedPosition = VolumeTransformList[j].InverseTransformPositionNoScale(PositionWorldTransformedPosition);
			PositionTransformedPosition.Z = 0;

			if (WorldScapeHelper::IsPointInRange(PositionTransformedPosition, FVector(0, 0, 0), VolumeSize))
			{
				

				FVector2D VolumeUV = FVector2D(PositionTransformedPosition.X, PositionTransformedPosition.Y);
				VolumeUV /= VolumeSize * 2;
				VolumeUV += FVector2D(0.5f, 0.5f);


				float edgefalloff = WorldScapeHelper::GetFalloff(VolumeUV.X, ActualVolume->EdgeFalloff) * WorldScapeHelper::GetFalloff(VolumeUV.Y, ActualVolume->EdgeFalloff);
				double value;
				float intensity = 1;
				if (!Water && ActualVolume->CanSampleHeight())
				{
					value = ActualVolume->GetHeight(VolumeUV, intensity, true);

					if (ActualVolume->OverrideHeight)
					{
						Data.HeightNormalize = FMath::Lerp(Data.HeightNormalize, value / NoiseIntensity, edgefalloff * intensity);
						Data.Height = FMath::Lerp(Data.Height, value, edgefalloff * intensity);
					}
					else
					{
						Data.HeightNormalize = Data.HeightNormalize + (value / NoiseIntensity) * edgefalloff * intensity;
						Data.Height = Data.Height + value * edgefalloff * intensity;
					}

					if (ActualVolume->CanSampleOceanHeight())
					{
						float waterHeight = FMath::Lerp(OceanHeight, ActualVolume->GetOceanHeight(VolumeUV, intensity, true), edgefalloff * intensity);

						if (Data.WaterMask < 0.5 && waterHeight > Data.Height)
						{
							Data.WaterMask = 1;
						}
						else if (Data.WaterMask >= 0.5 && waterHeight < Data.Height)
						{
							Data.WaterMask = 0;
						}
					}
				}
				if (ActualVolume->CanSampleTemperature())
				{
					value = ActualVolume->GetTemperature(VolumeUV, intensity);
					Data.Temperature = FMath::Lerp(Data.Temperature, (float)value, edgefalloff * intensity);
				}
				if (ActualVolume->CanSampleHumidity())
				{
					value = ActualVolume->GetHumidity(VolumeUV, intensity);
					Data.Humidity = FMath::Lerp(Data.Humidity, (float)value, edgefalloff * intensity);
				}
				if (Water && ActualVolume->CanSampleOceanHeight())
				{
					value = ActualVolume->GetOceanHeight(VolumeUV, intensity);
					Data.HeightNormalize = FMath::Lerp(Data.HeightNormalize, value / NoiseIntensity, edgefalloff * intensity);
					Data.Height = FMath::Lerp(Data.Height, value, edgefalloff * intensity);
				}
			}
		}
	}

	PositionWorldTransformedPosition = Position + Normal * Data.Height;
	PositionWorldTransformedPosition = ECEFToWorld(PositionWorldTransformedPosition).ToFVector();

	return Data;
}

