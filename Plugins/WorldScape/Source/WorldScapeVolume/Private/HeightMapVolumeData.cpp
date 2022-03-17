// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "HeightMapVolumeData.h" 





UHeightMapVolumeData::UHeightMapVolumeData() {

	HighQuality = true;
	OceanHighQuality = true;

}
#if WITH_EDITOR
void UHeightMapVolumeData::CleanData()
{
	SampledHeightMap.Empty();
	SampledTemperature.Empty();
	SampledHumidity.Empty();
	SampledHeightMapAlpha.Empty();

	SampledWaterHeightMap.Empty();
	SampledWaterHeightMapAlpha.Empty();
}

	void UHeightMapVolumeData::Generate()
	{
		/*
		bool BuildHeight = false;
		bool BuildOcean= false;

		if (PREVIOUS_HeightMap != HeightMap)
		{
			PREVIOUS_HeightMap = HeightMap;
			BuildHeight = true;
		}

		if (PREVIOUS_HighQuality != HighQuality) {

			PREVIOUS_HighQuality = HighQuality;
			BuildHeight = true;
		}

		if (PREVIOUS_SmoothValue != SmoothValue) {

			PREVIOUS_SmoothValue = SmoothValue;
			BuildHeight = true;
		}


		if (PREVIOUS_HeightMapOcean != HeightMapOcean)
		{
			PREVIOUS_HeightMapOcean = HeightMapOcean;
			BuildOcean = true;
		}

		if (PREVIOUS_OceanHighQuality != OceanHighQuality) {

			PREVIOUS_OceanHighQuality = OceanHighQuality;
			BuildOcean = true;
		}

		if (PREVIOUS_OceanSmoothValue != OceanSmoothValue) {

			PREVIOUS_OceanSmoothValue = OceanSmoothValue;
			BuildOcean = true;
		}
		

		if (BuildHeight)
			BuildTexture();
		if (BuildOcean)
			BuildOceanTexture();
			*/
		if (IsValid(HeightMap))
			BuildTexture();
		if (IsValid(HeightMapOcean))
			BuildOceanTexture();
	}
#endif
void UHeightMapVolumeData::BuildTexture()
{
#if WITH_EDITOR

	if (HeightMap == nullptr)
		return;

	TextureCompressionSettings OldCompressionSettings = HeightMap->CompressionSettings;

	TextureMipGenSettings OldMipGenSettings = HeightMap->MipGenSettings;

	bool OldSRGB = HeightMap->SRGB;
	HeightMap->CompressionSettings = TextureCompressionSettings::TC_HDR;
	HeightMap->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;

	EPixelFormat PixelFormat = HeightMap->GetPixelFormat();

	FString PixFormat = UEnum::GetValueAsString<EPixelFormat>(PixelFormat);

	HeightMap->SRGB = false;
	HeightMap->UpdateResource();

	FTexture2DMipMap* HeightMipMap = &HeightMap->PlatformData->Mips[0];

	SampledHeightMap.Empty();
	SampledTemperature.Empty();
	SampledHumidity.Empty();
	SampledHeightMapAlpha.Empty();

	Width = HeightMipMap->SizeX;
	Height = HeightMipMap->SizeY;


	if (HighQuality)
	{
		if (HeightMapOnly)
		{
			UINT16* FormatedImageDataLinear = static_cast<UINT16*>(HeightMap->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));

			if (FormatedImageDataLinear != nullptr && (FormatedImageDataLinear + (Width * Height) - 1) != nullptr)
			{
				for (int i = 0; i < Width * Height; i++)
				{
					SampledHeightMap.Add((FormatedImageDataLinear[i] - 32768)/32768.f);
				}
			}
			else
			{
				//Texture is in 8bit;
				HighQuality = false;
				BuildTexture();
				return;
			}
		}
		else
		{
			FFloat16Color* FormatedImageDataLinear = static_cast<FFloat16Color*>(HeightMap->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));
    
    		FVector4 Value, Average;
    		float AverageCount = 0.0f;
    		
    		if (FormatedImageDataLinear != nullptr && (FormatedImageDataLinear + (Width * Height)-1) != nullptr )
    		{
    			for (int i = 0; i < Width * Height; i++)
    			{
    				Value = FVector4(FormatedImageDataLinear[i].R, FormatedImageDataLinear[i].G, FormatedImageDataLinear[i].B, FormatedImageDataLinear[i].A);
    				SampledHeightMap.Add(Value.X);
    				SampledTemperature.Add(Value.Y);
    				SampledHumidity.Add(Value.Z);
    				SampledHeightMapAlpha.Add(Value.W);
    			}
    		}
    		else
    		{
    			//Texture is in 8bit;
    			HighQuality = false;
    			BuildTexture();
    			return;
    		}		
		}
	}
	else {
			FColor* FormatedImageData = static_cast<FColor*>(HeightMap->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));
			FVector4 Value, Average;
			float AverageCount = 0.0f;

			for (int i = 0; i < Width * Height; i++)
			{


				Value = FVector4(FormatedImageData[i].R, FormatedImageData[i].G, FormatedImageData[i].B, FormatedImageData[i].A);
				SampledHeightMap.Add(Value.X / 256.);
				SampledTemperature.Add(Value.Y / 256.);
				SampledHumidity.Add(Value.Z / 256.);
				SampledHeightMapAlpha.Add(Value.W / 256.);
			}
	}

	HeightMap->PlatformData->Mips[0].BulkData.Unlock();

	HeightMap->CompressionSettings = OldCompressionSettings;

	HeightMap->MipGenSettings = OldMipGenSettings;

	HeightMap->SRGB = OldSRGB;
	HeightMap->UpdateResource();
#endif
	PlanedUpdated = false;

}

void UHeightMapVolumeData::BuildOceanTexture()
{
#if WITH_EDITOR

	if (HeightMapOcean == nullptr)
		return;

	TextureCompressionSettings OldCompressionSettings = HeightMapOcean->CompressionSettings;

	TextureMipGenSettings OldMipGenSettings = HeightMapOcean->MipGenSettings;

	bool OldSRGB = HeightMapOcean->SRGB;

	HeightMapOcean->CompressionSettings = TextureCompressionSettings::TC_HDR;

	HeightMapOcean->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;

	EPixelFormat PixelFormat = HeightMapOcean->GetPixelFormat();

	FString PixFormat = UEnum::GetValueAsString<EPixelFormat>(PixelFormat);

	HeightMapOcean->SRGB = false;
	HeightMapOcean->UpdateResource();

	FTexture2DMipMap* HeightMipMap = &HeightMapOcean->PlatformData->Mips[0];

	SampledWaterHeightMap.Empty();
	SampledWaterHeightMapAlpha.Empty();

	WidthOcean = HeightMipMap->SizeX;
	HeightOcean = HeightMipMap->SizeY;

	if (OceanHighQuality) 
	{
		FFloat16Color* FormatedImageDataLinear = static_cast<FFloat16Color*>(HeightMapOcean->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));

		FVector2D Value, Average;
		float AverageCount = 0.0f;

		if (FormatedImageDataLinear != nullptr && (FormatedImageDataLinear + (Width * Height) - 1) != nullptr)
		{
			for (int i = 0; i < WidthOcean * HeightOcean; i++)
			{

				Value = FVector2D(FormatedImageDataLinear[i].R, FormatedImageDataLinear[i].A);

				SampledWaterHeightMap.Add(Value.X);
				SampledWaterHeightMapAlpha.Add(Value.Y);
			}
		}
		else
		{
			//Texture is in 8bit;
			OceanHighQuality = false;
			BuildOceanTexture();
			return;
		}
	}
	else 
	{
		FColor* FormatedImageData = static_cast<FColor*>(HeightMapOcean->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));
		FVector2D Value, Average;
		float AverageCount = 0.0f;

		for (int i = 0; i < WidthOcean * HeightOcean; i++)
		{

			Value = FVector2D(FormatedImageData[i].R, FormatedImageData[i].A);

			SampledWaterHeightMap.Add(Value.X / 256.);
			SampledWaterHeightMapAlpha.Add(Value.Y / 256.);
		}
	}

	HeightMapOcean->PlatformData->Mips[0].BulkData.Unlock();

	HeightMapOcean->CompressionSettings = OldCompressionSettings;

	HeightMapOcean->MipGenSettings = OldMipGenSettings;

	HeightMapOcean->SRGB = OldSRGB;
	HeightMapOcean->UpdateResource();
#endif
	PlanedUpdated = false;

}
 