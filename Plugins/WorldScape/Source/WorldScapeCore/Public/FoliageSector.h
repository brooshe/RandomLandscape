// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "WorldScapeCommon/Public/DoublePrecisionUtils.h"
#include "WorldScapeNoise/Public/NoiseData.h"
#include "WorldScapeMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "FoliageSector.generated.h"



USTRUCT(BlueprintType)
struct FFoliageSector {

	GENERATED_BODY()

public:

	FFoliageSector(DVector pPosition = DVector(0, 0, 0), double pSize = 0);

	DVector Position;
	double Size;
	TArray<FTransform> VegetationTransform;
	TArray<DVector> VegetationPosition;
	UHierarchicalInstancedStaticMeshComponent* InstancedMesh;
	FBox GetFBox(DVector Offset = DVector(0), double scale = 1);

	bool FoliageSpawned;
	bool operator==(const FFoliageSector& sector) const;
	bool operator!=(const FFoliageSector& sector) const;

};


