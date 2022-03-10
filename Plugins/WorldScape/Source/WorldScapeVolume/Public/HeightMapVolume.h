// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "HeightMapVolumeData.h"
#include "VolumeInterface.h" 
#include "HeightMapVolume.generated.h"

UCLASS(hideCategories = (Code))
class WORLDSCAPEVOLUME_API AHeightMapVolume : public AVolumeInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHeightMapVolume();

	FName ClassName = "HeightMap Volume";

	virtual void OnConstruction(const FTransform& Transform) override;

	

	float PREVIOUS_MaxHeight = 0;
	float PREVIOUS_MinHeight = 0;

	float PREVIOUS_MaxTemperature = 0;
	float PREVIOUS_MinTemperature = 0;

	float PREVIOUS_MaxHumidity = 0;
	float PREVIOUS_MinHumidity = 0;

	float PREVIOUS_MaxOceanHeight = 0;
	float PREVIOUS_MinOceanHeight = 0;

	float PREVIOUS_EdgeFalloff = 0;

	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defaults")
		UHeightMapVolumeData* HeightMapData = nullptr;

	//Does it Affect Height ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Height")
		bool Height = false;
	//Does it Affect Temperature ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Temperature")
		bool Temperature = false;
	//Does it Affect Humidity ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Humidity")
		bool Humidity = false;
	//Use the HeightMap Alpha ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defaults", meta = (EditCondition = "Height == true||Humidity == true||Temperature == true"))
		bool HeightAlpha = false;
	//Does it Affect Ocean height ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "bOcean")
		bool OceanHeight = false;
	//Use the Ocean HeightMap Alpha ?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "bOcean", meta = (EditCondition = "OceanHeight == true"))
		bool OceanHeightAlpha = false;


	//Height of the heightmap (Red Channel)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Height", meta = (EditCondition = "Height == true"))
		float MapHeight = 10000;
	//Base Altitude of the heightmap (Red Channel)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Height", meta = (EditCondition = "Height == true"))
		float Altitude = 0;

		float MaxHeight = 0;
		float MinHeight = 0;

	//Maximum temperature of the heightmap (Green Chanel)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Temperature", meta = (UIMin = 0, ClampMin = 0, UIMax = 1, ClampMax = 1), meta = (EditCondition = "Temperature == true"))
		float MaxTemperature = 0;
	//Minimum temperature of the heightmap (Green Chanel)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Temperature", meta = (UIMin = 0, ClampMin = 0, UIMax = 1, ClampMax = 1), meta = (EditCondition = "Temperature == true"))
		float MinTemperature = 0;
	//Maximum Humidity of the heightmap (Blue Chanel)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Humidity", meta = (UIMin = 0, ClampMin = 0, UIMax = 1, ClampMax = 1), meta = (EditCondition = "Humidity == true"))
		float MaxHumidity = 0;
	//Minimum Humidity of the heightmap (Blue Chanel)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Humidity", meta = (UIMin = 0, ClampMin = 0, UIMax = 1, ClampMax = 1), meta = (EditCondition = "Humidity == true"))
		float MinHumidity = 0;
	//Maximum Height of the heightmap (Red Channel Second Texture)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "bOcean", meta = (EditCondition = "OceanHeight == true"))
		float MaxOceanHeight = 0;
	//Minimum Height of the heightmap (Alpha Channel Second Texture)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "bOcean", meta = (EditCondition = "OceanHeight == true"))
		float MinOceanHeight = 0;
	//Ammount of Edge Falloff
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defaults", meta = (UIMin = 0, ClampMin = 0, UIMax = 0.5f, ClampMax = 0.5f))
		float EdgeFalloff = 0;

	
	bool IsHeightDataValid();
	
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		float GetHeight(FVector2D UV, float& Intensity, bool Precise = false);
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		float GetTemperature(FVector2D UV, float& Intensity);
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		float GetHumidity(FVector2D UV, float& Intensity);
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		float GetOceanHeight(FVector2D UV, float& Intensity, bool Precise = false);

	float GetPreciseValueFromHeightMap(FIntPoint Size, FVector2D UV);
	float GetPreciseValueFromAlpha(FIntPoint Size, FVector2D UV);
	float GetPreciseValueFromHeightMapOcean(FIntPoint Size, FVector2D UV);
	float GetPreciseValueFromAlphaOcean(FIntPoint Size, FVector2D UV);


	UFUNCTION(BlueprintCallable, Category = "Defaults")
		bool CanSampleHeight();
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		bool CanSampleTemperature();
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		bool CanSampleHumidity();
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		bool CanSampleHeightAlpha();
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		bool CanSampleOceanHeight();
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		bool CanSampleOceanHeightAlpha();
	UFUNCTION(BlueprintCallable, Category = "Defaults")
		bool CanSample();
	//When true, override the actual height of the planet, else, it'll simple add/sub the actual height;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defaults")
		bool OverrideHeight = true;
	
};