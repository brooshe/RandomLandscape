// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include <random>
#include <limits>
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"
#include "WorldScapeFoliage.generated.h"

UCLASS(ClassGroup = WorldScape, Category = "WorldScapeCore",editinlinenew, BlueprintType, Blueprintable)
class WORLDSCAPECORE_API UWorldScapeFoliage : public UObject
{
	GENERATED_BODY()

public:
	// The foliage's mesh
	UPROPERTY(EditAnywhere, Category = "Mesh", meta = (DisplayThumbnail = "true"))
		UStaticMesh* StaticMesh;
	UPROPERTY(EditAnywhere, Category = "Mesh")
		bool bCastShadows = true;
	UPROPERTY(EditAnywhere, Category = "Mesh")
		bool bReceiveDecal = false;
	UPROPERTY(EditAnywhere, Category = "Mesh")
		float MinScale = 0.8f;
	UPROPERTY(EditAnywhere, Category = "Mesh")
		float MaxScale = 1.2f;
	UPROPERTY(EditAnywhere, Category = "Mesh")
		bool bCollision = false;
	UPROPERTY(EditAnywhere, Category = "Mesh")
		FVector Offset = FVector(0,0,0);

	// Set the size of each Foliage Sector
	UPROPERTY(EditAnywhere, Category = "Spacing")
		double FoliageSectorSize = 2000;

	//Ammount of foliages spawn per sector
	UPROPERTY(EditAnywhere, Category = "Spacing")
		float FoliagesCount = 100.0f;

	
	/** Distance (in centimeters) from camera at which each generated instance fades out. A value of 0 means infinite. */
	UPROPERTY(EditAnywhere, Category = "Spacing", meta = (ClampMin = 0, UIMin = 0))
	int32 CullingDistance = 15000;

	/*
	* Non-Implemented feathures, may be implemented in a later date
	* 
	// If set to true this foliage will ignore the shade distance and use safe distance
	UPROPERTY(EditAnywhere, Category = "Spacing")
		bool GrowInShade = false;
	// The Number of generated Foliage per chunk
	
	// The minimum safe space between this mesh and other meshes
	UPROPERTY(EditAnywhere, Category = "Spacing")
		float SafeDistance = 100.0f;
	// The safe distance between this mesh and meshes that grow outside shade
	UPROPERTY(EditAnywhere, Category = "Spacing")
		float ShadeDistance = 200.0f;
	// If set to true this foliage will ignore the shade distance and use safe distance
	UPROPERTY(EditAnywhere, Category = "Clustering")
		bool GrowInCluster = false;
	// The minimum safe space between this mesh and other meshes
	UPROPERTY(EditAnywhere, Category = "Clustering", meta = (EditCondition = "GrowInCluster == true"))
		int ClusterMin = 1;
	// The safe distance between this mesh and meshes that grow outside shade
	UPROPERTY(EditAnywhere, Category = "Clustering", meta = (EditCondition = "GrowInCluster == true"))
		int ClusterMax = 1;
	// The radius of clusters in world space units
	UPROPERTY(EditAnywhere, Category = "Clustering", meta = (EditCondition = "GrowInCluster == true"))
		float ClusterRadius = 500.0f;
		*/

	UPROPERTY(EditAnywhere, Category = "Placement")
		bool bOverrideCollectionContraint = false;
	// The minimum elevation this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bOverrideCollectionContraint == true"))
		float MinElevation = -200000.0f;
	// The maximum elevation this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bOverrideCollectionContraint == true"))
		float MaxElevation = 200000.0f;
	// The minimum Temperature this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bOverrideCollectionContraint == true"))
		float MinTemperature = 0.5f;
	// The maximum Temperature this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bOverrideCollectionContraint == true"))
		float MaxTemperature = 1.f;
	// The minimum Humidity this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bOverrideCollectionContraint == true"))
		float MinHumidity = 0.5f;
	// The maximum Humidity this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bOverrideCollectionContraint == true"))
		float MaxHumidity = 1.f;
	// The minimum Humidity this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bOverrideCollectionContraint == true"))
		float MinSlope = 0.f;
	// The maximum Humidity this mesh will generate at
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bOverrideCollectionContraint == true"))
		float MaxSlope = 0.5f;


	// Set to true to allign the mesh to the ground
	UPROPERTY(EditAnywhere, Category = "Placement")
		bool bAllignedToGround = true;
	// Set to true to rotate the mesh randomly
	UPROPERTY(EditAnywhere, Category = "Placement")
		bool bRandomRotation = false;
	//How much the mesh rotation is influenced by the ground
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bAllignedToGround == true"))
		float GroundRotationInfluenceMin = 0.35f;
	//How much the mesh rotation is influenced by the ground
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bAllignedToGround == true"))
		float GroundRotationInfluenceMax = 0.6f;
	// Wether or not it use the Tree mask from noise generation
	UPROPERTY(EditAnywhere, Category = "Placement")
		bool bUseFoliageNoiseMask = false;
	UPROPERTY(EditAnywhere, Category = "Placement")
		bool bSpawnInWater = false;

	//Value < 0 mean use of collection's value instead;
	UPROPERTY(EditAnywhere, Category = "Placement")
		int FoliageLayer = -1;


protected:
	UPROPERTY(VisibleAnywhere, Category = WorldScapeFoliage)
		FString Description;

	UPROPERTY(VisibleAnywhere, Category = WorldScapeFoliage)
		bool bIsActive;
};
