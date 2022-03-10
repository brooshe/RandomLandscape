// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "HeightMapVolumeData.generated.h"


//Data of an HeightMap Volume, Can be shared by multiple HeightMap Volume

UCLASS(hideCategories = (Code))
class WORLDSCAPEVOLUME_API UHeightMapVolumeData : public UObject
{
	GENERATED_BODY()

public:

	FName ClassName = "HeightMap Volume Data";

	UHeightMapVolumeData();

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		void Generate();
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		void CleanData();
#endif

	UFUNCTION(BlueprintCallable, Category = "Texture")
		void BuildTexture();
	UFUNCTION(BlueprintCallable, Category = "Texture")
		void BuildOceanTexture();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Texture", meta = (ExposeOnSpawn = true))
		UTexture2D* HeightMap;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Texture", meta = (ExposeOnSpawn = true))
		UTexture2D* HeightMapOcean;

	//Is the texture 16bit ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TextureInfo", meta = (ExposeOnSpawn = true))
		bool HighQuality;
	//Is the bOcean texture 16bit ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TextureInfo", meta = (ExposeOnSpawn = true))
		bool OceanHighQuality;


	UTexture2D* PREVIOUS_HeightMap;
	UTexture2D* PREVIOUS_HeightMapOcean;
	bool PREVIOUS_HighQuality;
	bool PREVIOUS_OceanHighQuality;
	bool PREVIOUS_SmoothValue;
	bool PREVIOUS_OceanSmoothValue;



	//Sampled Heightmap
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		TArray<float> SampledHeightMap;
	//Sampled Temperature
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		TArray<float> SampledTemperature;
	//Sampled Humidity
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		TArray<float> SampledHumidity;
	//Sampled Heightmap Alpha
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		TArray<float> SampledHeightMapAlpha;
	//Sampled Water Heightmap
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		TArray<float> SampledWaterHeightMap;
	//Sampled Water Heightmap Alpha
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		TArray<float> SampledWaterHeightMapAlpha;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		int Height;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		int Width;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		int HeightOcean;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		int WidthOcean;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
		bool PlanedUpdated;
};