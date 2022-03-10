// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "NoiseData.generated.h"


USTRUCT(Blueprintable, BlueprintType)
struct WORLDSCAPENOISE_API FNoiseData 
{

	GENERATED_BODY()
public:

	FNoiseData();
	UPROPERTY(EditAnywhere, Category = NoiseData)
	double HeightNormalize = 0;
	UPROPERTY(EditAnywhere, Category = NoiseData)
	double Height = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NoiseData)
	float Temperature = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NoiseData)
	float Humidity = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NoiseData)
	float WaterMask = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NoiseData)
	float FoliageMask = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NoiseData)
	bool Hole = false;

	static FNoiseData LerpData(FNoiseData& A, FNoiseData& B, double Alpha) 
	{
		FNoiseData NewData = FNoiseData();
		NewData.HeightNormalize = A.HeightNormalize + ((B.HeightNormalize - A.HeightNormalize) * Alpha);
		NewData.Height = A.Height + ((B.Height - A.Height) * Alpha);
		NewData.Temperature = A.Temperature + ((B.Temperature - A.Temperature) * Alpha);
		NewData.Humidity = A.Humidity + ((B.Humidity - A.Humidity) * Alpha);
		NewData.WaterMask = A.WaterMask + ((B.WaterMask - A.WaterMask) * Alpha);
		NewData.FoliageMask = A.FoliageMask + ((B.FoliageMask - A.FoliageMask) * Alpha);

		return NewData;
	}
};
