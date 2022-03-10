// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "HeightMapVolume.h" 



void AHeightMapVolume::OnConstruction(const FTransform& Transform)
{
	MaxHeight = MapHeight + Altitude;
	MinHeight = Altitude;
	
		if (PREVIOUS_Location != GetActorLocation()) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_Rotation != GetActorRotation()) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_Scale != GetActorScale()) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_MaxHeight != MaxHeight) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_MinHeight != MinHeight) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_MaxOceanHeight != MaxOceanHeight) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_MinOceanHeight != MinOceanHeight) 
		{
			NeedRefresh = true;
		}
		
		if (PREVIOUS_MaxHumidity != MaxHumidity) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_MinHumidity != MinHumidity) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_MaxTemperature != MaxTemperature) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_MinTemperature != MinTemperature) 
		{
			NeedRefresh = true;
		}

		if (PREVIOUS_EdgeFalloff != EdgeFalloff) 
		{
			NeedRefresh = true;
		}

		if (IsValid(HeightMapData) && HeightMapData->PlanedUpdated == false) {
			HeightMapData->PlanedUpdated = true;
			NeedRefresh = true;
		}

		if (NeedRefresh)
		{

			PREVIOUS_Location = GetActorLocation();
			PREVIOUS_Rotation = GetActorRotation();
			PREVIOUS_Scale = GetActorScale();
			PREVIOUS_MaxHeight = MaxHeight;
			PREVIOUS_MinHeight = MinHeight;
			PREVIOUS_MaxOceanHeight = MaxOceanHeight;
			PREVIOUS_MinOceanHeight = MinOceanHeight;
			PREVIOUS_MaxHumidity = MaxHumidity;
			PREVIOUS_MinHumidity = MinHumidity;
			PREVIOUS_MaxTemperature = MaxTemperature;
			PREVIOUS_MinTemperature = MinTemperature;
			PREVIOUS_EdgeFalloff = EdgeFalloff;
		}

}





AHeightMapVolume::AHeightMapVolume()
{
	if (!HeightMapData) HeightMapData = CreateDefaultSubobject<UHeightMapVolumeData>(TEXT("HeightMapData"));
	PrimaryActorTick.bCanEverTick = false;
	MaxHeight = MapHeight + Altitude;
	MinHeight = Altitude;
}


bool AHeightMapVolume::IsHeightDataValid() 
{
	if (IsValid(HeightMapData)) 
	{
		//TODO : Print warning because the HMI don't have data 
		return true;
	}
	return false;
}


bool AHeightMapVolume::CanSampleHeight() 
{
	MaxHeight = MapHeight + Altitude;
	MinHeight = Altitude;

	if (!IsHeightDataValid())
		return false;

	if (HeightMapData->SampledHeightMap.Num() <= 0)
		return false;
	return Height;
}
bool AHeightMapVolume::CanSampleTemperature() 
{
	if (!IsHeightDataValid())
		return false;
	if (HeightMapData->SampledTemperature.Num() <= 0)
		return false;
	return Temperature;
}
bool AHeightMapVolume::CanSampleHumidity() 
{

	if (!IsHeightDataValid())
		return false;
	if (HeightMapData->SampledHumidity.Num() <= 0)
		return false;
	return Humidity;
}
bool AHeightMapVolume::CanSampleHeightAlpha() 
{
	if (!IsHeightDataValid())
		return false;
	if (HeightMapData->SampledHeightMapAlpha.Num() <= 0)
		return false;
	return HeightAlpha;
}

bool AHeightMapVolume::CanSampleOceanHeight() 
{
	if (!IsHeightDataValid())
		return false;
	if (HeightMapData->SampledWaterHeightMap.Num() <= 0)
		return false;
	return OceanHeight;
}
bool AHeightMapVolume::CanSampleOceanHeightAlpha() 
{
	if (!IsHeightDataValid())
		return false;
	if (HeightMapData->SampledWaterHeightMapAlpha.Num() <= 0)
		return false;
	return OceanHeightAlpha;
}


bool AHeightMapVolume::CanSample() {

	return (CanSampleOceanHeight() || CanSampleHumidity() || CanSampleTemperature() || CanSampleHeight());
}


//Get Precise Value allow to sample the height between the 4 closest pixel and lerp between them
//to do so, I first check if I can sample the pixel left,right,top and down, if tru, I simply lerp between them depending on the position, 
//else, I check if I can do so for top/down or Left/right (for the sides of the heightmap)

float AHeightMapVolume::GetPreciseValueFromHeightMap(FIntPoint Size, FVector2D UV) 
{
	float PosX = Size.X * UV.X;
	float PosY = Size.Y * UV.Y;
	float PosXFrac = FMath::Frac(PosX), PosYFrac = FMath::Frac(PosY);
	int PixelX = FMath::FloorToInt(PosX), PixelY = FMath::FloorToInt(PosY);

	float ValueA, ValueB;
	if (HeightMapData->SampledHeightMap.IsValidIndex(PixelY * Size.X + PixelX)){
		if (PosXFrac > 0 && PosYFrac > 0 && (PixelX+1)< HeightMapData->Width && (PixelY+1) < HeightMapData->Height && HeightMapData->SampledHeightMap.IsValidIndex((PixelY + 1) * Size.X + PixelX + 1))
		{
			ValueA = FMath::Lerp(HeightMapData->SampledHeightMap[PixelY * Size.X + PixelX], HeightMapData->SampledHeightMap[PixelY * Size.X + PixelX + 1], PosXFrac);
			ValueB = FMath::Lerp(HeightMapData->SampledHeightMap[(PixelY + 1) * Size.X + PixelX], HeightMapData->SampledHeightMap[(PixelY + 1) * Size.X + PixelX + 1], PosXFrac);
			return FMath::Lerp(ValueA, ValueB, PosYFrac);
		}
		else if (PosXFrac > 0 && (PixelX+1) < HeightMapData->Width && (PixelY * Size.X + PixelX + 1) < HeightMapData->SampledHeightMap.Num())
		{
			return FMath::Lerp(HeightMapData->SampledHeightMap[PixelY * Size.X + PixelX], HeightMapData->SampledHeightMap[PixelY * Size.X + PixelX + 1], PosXFrac);
		}
		else if (PosYFrac > 0 && ((PixelY + 1) * Size.X + PixelX) < HeightMapData->SampledHeightMap.Num())
		{
			return FMath::Lerp(HeightMapData->SampledHeightMap[PixelY * Size.X + PixelX], HeightMapData->SampledHeightMap[(PixelY + 1) * Size.X + PixelX], PosYFrac);
		}
		return HeightMapData->SampledHeightMap[PixelY * Size.X + PixelX];
	}
	return 0;
}

float AHeightMapVolume::GetPreciseValueFromAlpha(FIntPoint Size, FVector2D UV) 
{
	float PosX = Size.X * UV.X;
	float PosY = Size.Y * UV.Y;
	float PosXFrac = FMath::Frac(PosX), PosYFrac = FMath::Frac(PosY);
	int PixelX = FMath::FloorToInt(PosX), PixelY = FMath::FloorToInt(PosY);

	float ValueA, ValueB;
	if (PixelY * Size.X + PixelX < HeightMapData->SampledHeightMapAlpha.Num())
	{
		if (PosXFrac > 0 && PosYFrac > 0 && (PixelX + 1) < HeightMapData->Width && ((PixelY + 1) * Size.X + PixelX + 1) < HeightMapData->SampledHeightMapAlpha.Num())
		{

			ValueA = FMath::Lerp(HeightMapData->SampledHeightMapAlpha[PixelY * Size.X + PixelX], HeightMapData->SampledHeightMapAlpha[PixelY * Size.X + PixelX + 1], PosXFrac);
			ValueB = FMath::Lerp(HeightMapData->SampledHeightMapAlpha[(PixelY + 1) * Size.X + PixelX], HeightMapData->SampledHeightMapAlpha[(PixelY + 1) * Size.X + PixelX + 1], PosXFrac);
			return FMath::Lerp(ValueA, ValueB, PosYFrac);
		}
		else if (PosXFrac > 0 && (PixelX + 1) < HeightMapData->Width && (PixelY * Size.X + PixelX + 1) < HeightMapData->SampledHeightMapAlpha.Num())
		{
			return FMath::Lerp(HeightMapData->SampledHeightMapAlpha[PixelY * Size.X + PixelX], HeightMapData->SampledHeightMapAlpha[PixelY * Size.X + PixelX + 1], PosXFrac);
		}
		else if (PosYFrac > 0 && ((PixelY + 1) * Size.X + PixelX) < HeightMapData->SampledHeightMapAlpha.Num())
		{
			return FMath::Lerp(HeightMapData->SampledHeightMapAlpha[PixelY * Size.X + PixelX], HeightMapData->SampledHeightMapAlpha[(PixelY + 1) * Size.X + PixelX], PosYFrac);
		}
		return HeightMapData->SampledHeightMapAlpha[PixelY * Size.X + PixelX];
	}
	return 0;
}

float AHeightMapVolume::GetPreciseValueFromHeightMapOcean(FIntPoint Size, FVector2D UV) 
{
	float PosX = Size.X * UV.X;
	float PosY = Size.Y * UV.Y;
	float PosXFrac = FMath::Frac(PosX), PosYFrac = FMath::Frac(PosY);
	int PixelX = FMath::FloorToInt(PosX), PixelY = FMath::FloorToInt(PosY);

	float ValueA, ValueB;
	if (PixelY * Size.X + PixelX < HeightMapData->SampledWaterHeightMap.Num())
	{
		if (PosXFrac > 0 && PosYFrac > 0 && (PixelX + 1) < HeightMapData->WidthOcean && ((PixelY + 1) * Size.X + PixelX + 1) < HeightMapData->SampledWaterHeightMap.Num()) 
		{

			ValueA = FMath::Lerp(HeightMapData->SampledWaterHeightMap[PixelY * Size.X + PixelX], HeightMapData->SampledWaterHeightMap[PixelY * Size.X + PixelX + 1], PosXFrac);
			ValueB = FMath::Lerp(HeightMapData->SampledWaterHeightMap[(PixelY + 1) * Size.X + PixelX], HeightMapData->SampledWaterHeightMap[(PixelY + 1) * Size.X + PixelX + 1], PosXFrac);
			return FMath::Lerp(ValueA, ValueB, PosYFrac);
		}
		else if (PosXFrac > 0 && (PixelX + 1) < HeightMapData->WidthOcean && (PixelY * Size.X + PixelX + 1) < HeightMapData->SampledWaterHeightMap.Num()) {
			return FMath::Lerp(HeightMapData->SampledWaterHeightMap[PixelY * Size.X + PixelX], HeightMapData->SampledWaterHeightMap[PixelY * Size.X + PixelX + 1], PosXFrac);
		}
		else if (PosYFrac > 0 && ((PixelY + 1) * Size.X + PixelX) < HeightMapData->SampledWaterHeightMap.Num())
		{
			return FMath::Lerp(HeightMapData->SampledWaterHeightMap[PixelY * Size.X + PixelX], HeightMapData->SampledWaterHeightMap[(PixelY + 1) * Size.X + PixelX], PosYFrac);
		}
		return HeightMapData->SampledWaterHeightMap[PixelY * Size.X + PixelX];
	}
	return 0;
}

float AHeightMapVolume::GetPreciseValueFromAlphaOcean(FIntPoint Size, FVector2D UV) 
{
	float PosX = Size.X * UV.X;
	float PosY = Size.Y * UV.Y;
	float PosXFrac = FMath::Frac(PosX), PosYFrac = FMath::Frac(PosY);
	int PixelX = FMath::FloorToInt(PosX), PixelY = FMath::FloorToInt(PosY);

	float ValueA, ValueB;

	if (PixelY * Size.X + PixelX < HeightMapData->SampledWaterHeightMapAlpha.Num())
	{
		if (PosXFrac > 0 && PosYFrac > 0 && (PixelX + 1) < HeightMapData->WidthOcean && ((PixelY + 1) * Size.X + PixelX + 1) < HeightMapData->SampledWaterHeightMapAlpha.Num()) 
		{

			ValueA = FMath::Lerp(HeightMapData->SampledWaterHeightMapAlpha[PixelY * Size.X + PixelX], HeightMapData->SampledWaterHeightMapAlpha[PixelY * Size.X + PixelX + 1], PosXFrac);
			ValueB = FMath::Lerp(HeightMapData->SampledWaterHeightMapAlpha[(PixelY + 1) * Size.X + PixelX], HeightMapData->SampledWaterHeightMapAlpha[(PixelY + 1) * Size.X + PixelX + 1], PosXFrac);
			return FMath::Lerp(ValueA, ValueB, PosYFrac);
		}
		else if (PosXFrac > 0 && (PixelX + 1) < HeightMapData->WidthOcean && (PixelY * Size.X + PixelX + 1) < HeightMapData->SampledWaterHeightMapAlpha.Num()) 
		{
			return FMath::Lerp(HeightMapData->SampledWaterHeightMapAlpha[PixelY * Size.X + PixelX], HeightMapData->SampledWaterHeightMapAlpha[PixelY * Size.X + PixelX + 1], PosXFrac);
		}
		else if (PosYFrac > 0 && ((PixelY + 1) * Size.X + PixelX) < HeightMapData->SampledWaterHeightMapAlpha.Num())
		{
			return FMath::Lerp(HeightMapData->SampledWaterHeightMapAlpha[PixelY * Size.X + PixelX], HeightMapData->SampledWaterHeightMapAlpha[(PixelY + 1) * Size.X + PixelX], PosYFrac);
		}
		return HeightMapData->SampledWaterHeightMapAlpha[PixelY * Size.X + PixelX];
	}
	return 0;
}





float AHeightMapVolume::GetHeight(FVector2D UV, float& Intensity, bool Precise) 
{
	if (!IsHeightDataValid()) 
	{
		Intensity = 0;
		return 0;
	}

	UV = FVector2D::Min(UV, FVector2D(1, 1));
	UV = FVector2D::Max(UV, FVector2D(0, 0));

	FIntPoint Size = FIntPoint(HeightMapData->Width, HeightMapData->Height);

	int PixelX = HeightMapData->Width * UV.X, PixelY = HeightMapData->Height * UV.Y;


	if ((PixelY * HeightMapData->Width + PixelX) < HeightMapData->SampledHeightMap.Num()) 
	{
		if ((PixelY * HeightMapData->Width + PixelX) < HeightMapData->SampledHeightMapAlpha.Num() && CanSampleHeightAlpha()) 
		{
			if (Precise) Intensity = GetPreciseValueFromAlpha(Size, UV);
			else Intensity = HeightMapData->SampledHeightMapAlpha[PixelY * HeightMapData->Width + PixelX];
		}
			
		else
			Intensity = 1;


		if (Precise) 
		{
			return MinHeight + (GetPreciseValueFromHeightMap(Size, UV)) * (MaxHeight - MinHeight);

		}
		else 
		{
			return MinHeight + (HeightMapData->SampledHeightMap[PixelY * HeightMapData->Width + PixelX]) * (MaxHeight - MinHeight);
		}
	}
	else 
	{
		Intensity = 0;
		return 0;
	}
}

float AHeightMapVolume::GetTemperature(FVector2D UV, float& Intensity) 
{
	if (!IsHeightDataValid()) 
	{
		Intensity = 0;
		return 0;
	}

	UV = FVector2D::Min(UV, FVector2D(1, 1));
	UV = FVector2D::Max(UV, FVector2D(0, 0));

	int PixelX = HeightMapData->Width * UV.X, PixelY = HeightMapData->Height * UV.Y;

	if ((PixelY * HeightMapData->Width + PixelX) < HeightMapData->SampledTemperature.Num()) 
	{

		if (CanSampleHeightAlpha()) 
		{
			Intensity = HeightMapData->SampledHeightMapAlpha[PixelY * HeightMapData->Width + PixelX];
		}
		else
			Intensity = 1;

		return MinTemperature + (HeightMapData->SampledTemperature[PixelY * HeightMapData->Width + PixelX]) * (MaxTemperature - MinTemperature);
	}
	else 
	{
		Intensity = 0;
		return 0;
	}
}

float AHeightMapVolume::GetHumidity(FVector2D UV, float& Intensity) 
{

	if (!IsHeightDataValid()) 
	{
		Intensity = 0;
		return 0;
	}


	UV = FVector2D::Min(UV, FVector2D(1, 1));
	UV = FVector2D::Max(UV, FVector2D(0, 0));

	int PixelX = HeightMapData->Width * UV.X, PixelY = HeightMapData->Height * UV.Y;


	if ((PixelY * HeightMapData->Width + PixelX) < HeightMapData->SampledHumidity.Num()) 
	{
		if (CanSampleHeightAlpha()) 
		{
			Intensity = HeightMapData->SampledHeightMapAlpha[PixelY * HeightMapData->Width + PixelX];
		}
		else
			Intensity = 1;

		return MinHumidity + (HeightMapData->SampledHumidity[PixelY * HeightMapData->Width + PixelX]) * (MaxHumidity - MinHumidity);
	}
	else 
	{
		Intensity = 0;
		return 0;
	}
}

float AHeightMapVolume::GetOceanHeight(FVector2D UV, float& Intensity, bool Precise) 
{
	if (!IsHeightDataValid()) 
	{
		Intensity = 0;
		return 0;
	}

	UV = FVector2D::Min(UV, FVector2D(1, 1));
	UV = FVector2D::Max(UV, FVector2D(0, 0));

	FIntPoint Size = FIntPoint(HeightMapData->Width, HeightMapData->Height);

	int PixelX = HeightMapData->WidthOcean * UV.X, PixelY = HeightMapData->HeightOcean * UV.Y;


	if ((PixelY * HeightMapData->WidthOcean + PixelX) < HeightMapData->SampledWaterHeightMapAlpha.Num()) 
	{

		if ((PixelY * HeightMapData->WidthOcean + PixelX) < HeightMapData->SampledWaterHeightMapAlpha.Num() && CanSampleHeightAlpha()) 
		{
			if (Precise) Intensity = GetPreciseValueFromAlphaOcean(Size, UV);
			else Intensity = HeightMapData->SampledWaterHeightMapAlpha[PixelY * HeightMapData->WidthOcean + PixelX];
		}

		else
			Intensity = 1;


		if (Precise) 
		{
			return MinOceanHeight + (GetPreciseValueFromHeightMapOcean(Size, UV)) * (MaxOceanHeight - MinOceanHeight);
		}
		else 
		{
			return MinOceanHeight + (HeightMapData->SampledWaterHeightMap[PixelY * HeightMapData->WidthOcean + PixelX]) * (MaxOceanHeight - MinOceanHeight);
		}
	}
	else 
	{
		Intensity = 0;
		return 0;
	}
}

//PixelY * HeightMipMap->SizeX + PixelX

