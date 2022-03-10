// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "WorldScapeCommon/Public/DoublePrecisionUtils.h"
#include "WorldScapeNoise/Public/NoiseData.h"
#include "WorldScapeMeshComponent.h"
#include "FoliageSector.h"
#include "LodData.generated.h"

//USTRUCT()
struct LodData {

	//GENERATED_BODY()

public:
	LodData(TArray<FVector> pVertices, TArray<FLinearColor> pVertexColors, TArray<FVector> pNormals, TArray<FWorldScapeMeshTangent> pTangents, TArray<FVector2D> pUV);

	LodData();

	TArray<DVector> Vertices;
	TArray<FLinearColor> VertexColors;
	TArray<FVector> Normals;
	TArray<FWorldScapeMeshTangent> Tangents;
	TArray<FVector2D> UV;
};


USTRUCT(BlueprintType)
struct FVegetation {

	GENERATED_BODY()

public:
	int FoliageIndex = 0;
	int FoliageCollectionIndex = 0;
	int FoliageListIndex = 0;
	TArray<FFoliageSector> ActiveFoliageSector;
};
