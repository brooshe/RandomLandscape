// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "WorldScapeCommon/Public/DoublePrecisionUtils.h"
#include "WorldScapeNoise/Public/NoiseData.h"
#include "WorldScapeMeshComponent.h"
#include "Math/Range.h"
#include "Materials/MaterialInterface.h"
#include "Math/Interval.h"
#include "MaterialLod.generated.h"


/*
UCLASS()
class WORLDSCAPECORE_API UInt32Range
{

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Interval)
	int32 Min;

	UPROPERTY(EditAnywhere, Category = Interval)
	int32 Max;

	UFUNCTION(BlueprintCallable, Category = "Defaults")
	bool Contains(int32 value) 
	{
		return (value >= Min && value <= Max);
	}

};
*/


USTRUCT(BlueprintType)
struct FWSMaterialLod 
{

	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AssetRegistrySearchable)
		FInt32Range LodRange = FInt32Range();
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UMaterialInterface* Material = nullptr;
};


USTRUCT(BlueprintType)
struct FWSMaterialLodArray 
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FWSMaterialLod> MaterialsLod;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UMaterialInterface* DefaultMaterial = nullptr;
};