// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CoreDelegates.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include <algorithm>
#include <map>
#include "WorldScapeMeshComponent.h"
#include <WorldScapeCommon/Public/DoublePrecisionUtils.h>
#include <WorldScapeNoise/Public/NoiseData.h>
#include <WorldScapeNoise/Public/WorldScapeNoiseClass.h>
#include "WorldScapeVolume/Public/NoiseVolume.h"
#include "WorldScapeVolume/Public/HeightMapVolume.h"
#include "WorldScapeVolume/Public/FoliageMaskVolume.h"
#include "WorldScapeCommon/Public/WorldScapeHelper.h"
#include "WorldScapeLod.h"
#include "LodData.h"
#include "WorldScapeFoliage.h"
#include "FoliageSector.h"
#include "MaterialLod.h"
#include "WorldScapeWorldType.h"
#include "WorldScapeFoliageCollection.h"
#if WITH_EDITOR
#include <WorldScapeEditor/Public/PCUtils.h>
#endif

#include "WorldScapeRoot.generated.h"



UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EWorldScapeType : uint8
{
	Flat 	UMETA(DisplayName = "Flat World"),
	Planet 	UMETA(DisplayName = "Planet")
};

static void ProcessVertices(LodData& Data, int32 MinY, int32 MinX, int32 MaxY, int32 MaxX, DVector Tangent, DVector BiTangent, double StepSize, double HalfSize, double uvSpacing,
	FVector2D SubPosition, int32 MinBorderY, int32 MinBorderX, int32 MaxBorderY, int32 MaxBorderX, double OffsetX, double OffsetY, bool InvertOrder, double PlanetScaleCode,
	DVector PPos, bool FlatWorld, bool DealSewing = true)
{
	DVector X, Y;
	DVector PositionNormal;
	bool doSewing = false;



	if (InvertOrder)
	{
		for (int32 y = MinY; y < MaxY; y++)
		{
			for (int32 x = MaxX - 1; x >= MinX; x--)
			{

				if (DealSewing)
				{

					doSewing = (bool)(y % 2);
					if (SubPosition.Y > 0.5) doSewing = !doSewing;

					if (x == MinBorderX && doSewing)
					{
						Y = BiTangent * ((y + 1) * StepSize - HalfSize + OffsetY);
					}
					else if (x == MaxBorderX && doSewing)
					{
						Y = BiTangent * ((y - 1) * StepSize - HalfSize + OffsetY);
					}
					else
					{
						Y = BiTangent * (y * StepSize - HalfSize + OffsetY);
					}

					doSewing = (bool)(x % 2);
					if (SubPosition.X > 0.5) doSewing = !doSewing;

					if (y == MinBorderY && doSewing)
					{
						X = Tangent * ((x + 1) * StepSize - HalfSize + OffsetX);
					}
					else if (y == MaxBorderY && doSewing)
					{
						X = Tangent * ((x - 1) * StepSize - HalfSize + OffsetX);
					}
					else
					{
						X = Tangent * (x * StepSize - HalfSize + OffsetX);
					}
				}
				else
				{
					Y = BiTangent * (y * StepSize - HalfSize + OffsetY);
					X = Tangent * (x * StepSize - HalfSize + OffsetX);
				}

				if (FlatWorld)
				{

					Data.Vertices.Add(DVector(X + Y).ToFVector());
				}
				else
				{
					PositionNormal = DVector(X + Y); // Plane Position
					PositionNormal = PositionNormal + PPos; // ECEF position
					PositionNormal.Normalize();
					Data.Vertices.Add(((PositionNormal * PlanetScaleCode) - PPos).ToFVector());
				}

				Data.Normals.Add(FVector(0.0f, 0.0f, 1.0f));
				Data.UV.Add(FVector2D((x + (int32)SubPosition.X) * uvSpacing, (y + (int32)SubPosition.Y) * uvSpacing));
				Data.Tangents.Add(FWorldScapeMeshTangent(Tangent.X, Tangent.Y, Tangent.Z));
			}
		}
	}
	else
	{
		for (int32 y = MinY; y < MaxY; y++)
		{
			for (int32 x = MinX; x < MaxX; x++)
			{
				if (DealSewing) {

					doSewing = (bool)(y % 2);
					if (SubPosition.Y > 0.5) doSewing = !doSewing;

					if (x == MinBorderX && doSewing)
					{
						Y = BiTangent * ((y + 1) * StepSize - HalfSize + OffsetY);
					}
					else if (x == MaxBorderX && doSewing)
					{
						Y = BiTangent * ((y - 1) * StepSize - HalfSize + OffsetY);
					}
					else
					{
						Y = BiTangent * (y * StepSize - HalfSize + OffsetY);
					}

					doSewing = (bool)(x % 2);
					if (SubPosition.X > 0.5) doSewing = !doSewing;

					if (y == MinBorderY && doSewing)
					{
						X = Tangent * ((x + 1) * StepSize - HalfSize + OffsetX);
					}
					else if (y == MaxBorderY && doSewing)
					{
						X = Tangent * ((x - 1) * StepSize - HalfSize + OffsetX);
					}
					else
					{
						X = Tangent * (x * StepSize - HalfSize + OffsetX);
					}
				}
				else
				{
					Y = BiTangent * (y * StepSize - HalfSize + OffsetY);
					X = Tangent * (x * StepSize - HalfSize + OffsetX);
				}

				if (FlatWorld)
				{

					Data.Vertices.Add(DVector(X + Y).ToFVector());
				}
				else
				{
					PositionNormal = DVector(X + Y); // Plane Position
					PositionNormal = PositionNormal + PPos; // ECEF position
					PositionNormal.Normalize();
					Data.Vertices.Add(((PositionNormal * PlanetScaleCode) - PPos).ToFVector());
				}

				Data.Normals.Add(FVector(0.0f, 0.0f, 1.0f));
				Data.UV.Add(FVector2D((x + (int32)SubPosition.X) * uvSpacing, (y + (int32)SubPosition.Y) * uvSpacing));
				Data.Tangents.Add(FWorldScapeMeshTangent(Tangent.X, Tangent.Y, Tangent.Z));
			}
		}
	}

}

//DECLARE_LOG_CATEGORY_EXTERN(LogWorldScapeCore, Log, All);

//UCLASS(hideCategories = ( Code,Debug,MeshInfo,Rebase))
UCLASS(hideCategories = (MeshInfo, Rebase))
class WORLDSCAPECORE_API AWorldScapeRoot : public AActor
{
	GENERATED_BODY()

public:
	AWorldScapeRoot();


	virtual void Tick(float DeltaTime) override;
	/** Allows Tick To happen in the editor viewport*/
	virtual bool ShouldTickIfViewportsOnly() const override;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostLoad() override;
	virtual void ClearCrossLevelReferences() override;



	//void UpdateWorldOrigin(FIntVector NewPosition);


		//Number of tick per seconds
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor")
		float TickPerSecond;



	//Default Properties : 

	//WorldScape Generation Type
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	EWorldScapeType GenerationType;
	//InCose Used bool
	bool bFlatWorld;
	

	//Start or stop planet generation, when disabled, the planet is destroyed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (DisplayName = "Generate"))
	bool bGenerateWorldScape;
	//Freeze the Planet Generation without destroying the planet
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (DisplayName = "Freeze Generation"))
	bool bFreezeGeneration;
	//Generate The Ocean (and river/pond if noise is set for)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (DisplayName = "Generate Ocean"))
	bool bOcean;
	//Radius of the planet in Cm (UE4 units)
	UPROPERTY(EditAnywhere, Category = "Default|Scale", meta = (EditCondition = "GenerationType == EWorldScapeType::Planet"))
	double PlanetScale;

	double PlanetScaleCode;
	
	FVector LastGenerationNormal;

	

	

	float GridAngle;



	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default|Material")
	FWSMaterialLodArray TerrainMaterial;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default|Material", meta = (EditCondition = "bOcean == true"))
	FWSMaterialLodArray OceanMaterial;



	//Project the editor position to where the camera look
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (DisplayName = "Projected Position (Experimental)"))
	bool bProjectPosition;

	//Foliages List
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (DisplayName = "Foliages (Experimental)"))
		TArray<UWorldScapeFoliageCollection*> Foliages;

	//Override the player 0 position (for visual generation)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default|Override", meta = (DisplayName = "Override Player Position"))
	bool bOverridePlayerPosition;
	//Overrided Player Position in World Position
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default|Override", meta = (DisplayName = "New Player Position", EditCondition = "bOverridePlayerPosition == true"))
	FVector OverridedPlayerPosition;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (ToolTip = "Distance from the planet at which the planet highly slowdown the update to prevent performance impact"))
		float DistanceToFreezeGeneration;

	//Maximum Lod when in Runtime
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters")
	int32 MaxLod;

	//Number of Polygon on X axis per on Lod 0 , Need to be a multiplier or 4
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters")
	int32 LodResolution;
	
	//Triangle Size in Unit;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters")
	float TriangleSize;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	AActor* DebugObject;




	//Maximum Lod when in Runtime
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters", meta = (EditCondition = "bOcean == true"))
	int32 OceanMaxLod;

	//Number of Polygon on X axis per on Lod 0 , Need to be a multiplier of 4
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters", meta = (EditCondition = "bOcean == true"))
	int32 OceanLodResolution;

	//Triangle Size in Unit;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters", meta = (EditCondition = "bOcean == true"))
	float OceanTriangleSize;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision", meta = (DisplayName = "RunTime Collision"))
	bool bGenerateCollision;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision", meta = (DisplayName = "All Collision For All Player", ToolTip = "Generate collision for all Collision dependant actor for all player when on server and not just the server/host player"))
	bool bGenerateCollisionForAllPlayer;
	//bCollision Generation in Editor
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision", meta = (DisplayName = "Editor Collision"))
	bool bGenerateCollisionInEditor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision", meta = (EditCondition = "bGenerateCollisionInEditor == true", DisplayName = "Static Editor Collision"))
	bool bStaticCollisionInEditor;

	//Number of Polygon on X axis per on Lod 0 , Need to be a multiplier of 4
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision", meta = (EditCondition = "bGenerateCollision == true || bGenerateCollisionInEditor == true"))
	int32 CollisionResolution;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision", meta = (EditCondition = "bGenerateCollision == true || bGenerateCollisionInEditor == true"))
	float CollisionTriangleSize;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision", meta = (EditCondition = "bGenerateCollision == true || bGenerateCollisionInEditor == true", DisplayName = "Display Collision"))
	bool bDisplayCollision;

	bool Prev_bDisplayCollision;

	//List of Actor generating collision on WorldScape;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision", meta = (EditCondition = "bGenerateCollision == true || bGenerateCollisionInEditor == true"))
	TArray<AActor*> CollisionDependantActor;

	//At which Altitue shall the Triangle Size increase in scale
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters")
	float HeightAnchor;
	//Maximum value of the grid angle at high altitude used by the terrain
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters", meta = (DisplayName = "Lod Angle Max (Experimental)"))
	float GridAngleMax;
	//Minimum value of the grid angle at low altitude used by the terrain
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lod Parameters", meta = (DisplayName = "Lod Angle min (Experimental)"))
	float GridAngleMin;


	



	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	TArray<FVegetation> FoliageDataList;
	TAtomic<bool> FoliageGenerationDone;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	bool FoliageGenerationInProgress;
	
	

	


	//Noise Properties
	//The Scale of the noise, Higger value = wide details
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise")
	float NoiseScale;
	//Intensity of the noise in cm, Higger value = Higger montains
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise")
	float NoiseIntensity;
	//Seed value
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise")
	int32 Seed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise")
	FRotator LattitudeRotation;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise")
	FVector NoiseOffset;
	//Height of the ocean
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise", meta = (EditCondition = "bOcean == true"))
	float OceanHeight;
	//Custom Noise Subclass
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Noise")
	UWorldScapeNoiseClass* WorldScapeNoise;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Volumes")
	bool bEnableVolumes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Volumes")
	TArray<AHeightMapVolume*> HeightMapVolumeList;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Volumes")
	TArray<ANoiseVolume*> NoiseVolumeList;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Volumes")
	TArray<AFoliageMaskVolume*> FoliageMaskList;

	

	//Player Distance to the ground (based on noise and Height map Volumes)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info" , meta = (EditCondition = "false"))
	float PlayerDistanceToGround;
	//Player Altitudes to the Ocean Level
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info", meta = (EditCondition = "false"))
	float PlayerAltitude;
	//Foliages instance Left to generate in the current sector
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info", meta = (EditCondition = "false"))
	int32 FoliageCountLeft;
	//Foliages Sector Left to generate for the current foliage
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info", meta = (EditCondition = "false"))
	int32 FoliageSectorLeft;
	//Foliages Left to generate
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info", meta = (EditCondition = "false"))
	int32 FoliageLeft;

	

	
	

	


	
	

	/**
	* Component Used so the Actor keep it's Transform... It's totaly an hack
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	UWorldScapeLod* TransformKeeper;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	TArray<UHierarchicalInstancedStaticMeshComponent*> InstancedMeshs;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	bool bGenerateFoliages;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
	TArray<UWorldScapeLod*> WorldScapeLod;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
	TArray<UWorldScapeLod*> WorldScapeLodOcean;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
	TArray<UWorldScapeLod*> CollisionLods;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Code")
	TMap<UWorldScapeLod*, bool> WorldScapeLodInGeneration;
	UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = "Code")
	TArray <UProceduralMeshComponent*> LodMesh;
	UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = "Code")
	TArray<UProceduralMeshComponent*> LodMeshOcean;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	FVector PlayerPos;

	/**
	* The Function that check for Update on Important parameters change based on buffer parameters
	* @param update does it update the Buffer ?
	* @return true if the Planet need to be regenerated, false otherwise.
	*/
	bool CheckForRegenerate(bool update = true);
	/**
	* The Function that check for HeightMapVolume modifications and regenerate the planet if that the case
	* TODO : Make update only on needed Vertex;
	*/
	void CheckForHeightmapModifier();


	//Position TransformFunctions

	//Convert from RebasedWorld to 'Earth-Centered,Earth-Fixed Coordinate'
	DVector WorldToECEF(DVector Position);
	/**Convert from RebasedWorld to 'Earth-Centered,Earth-Fixed Coordinate'
	* #WARNING : MAY LACK PRECISION
	**/
	DVector WorldToECEF(FVector FloatPosition);



	//Convert from 'Earth-Centered,Earth-Fixed Coordinate' to Rebased World
	DVector ECEFToWorld(DVector Position);
	/**Convert from 'Earth-Centered,Earth-Fixed Coordinate' to Rebased World
	* #WARNING : MAY LACK PRECISION
	**/
	DVector ECEFToWorld(FVector FloatPosition);


	DVector ECEFToProjectedPos(DVector Position);


	DVector ECEFToCube(DVector position);


	FFoliageSector GetFoliageSector(DVector Position, int32 SectorLod);
	TArray<FFoliageSector> GetSurroundingFoliageSector(DVector Position, double pSectorSize);


	/**
	* Get the List of HeightMapVolume from position
	* @param Position Position to obtain the Volume from
	* @return List of Volume affecting the noise at Position
	*/
	TArray<AHeightMapVolume*> GetHeightMapVolumeList(FVector Position);
	/**
	* Get the List of HeightMapVolume from FBox
	* @param pFBox FBox used for Intersect
	* @return List of Volume affecting the noise in the FBox
	*/
	TArray<AHeightMapVolume*> GetHeightMapVolumeList(FBox pFBox);


	/**
	* Get the List of NoiseVolume from position
	* @param Position Position to obtain the NoiseVolume from
	* @return List of NoiseVolume affecting the noise at Position
	*/
	TArray<ANoiseVolume*> GetNoiseVolumeList(FVector Position);
	/**
	* Get the List of NoiseVolume from FBox
	* @param pFBox FBox used for Intersect
	* @return List of NoiseVolume affecting the noise in the FBox
	*/
	TArray<ANoiseVolume*> GetNoiseVolumeList(FBox pFBox);

	/**
	* Get the List of NoiseVolume from position
	* @param Position Position to obtain the NoiseVolume from
	* @return List of NoiseVolume affecting the noise at Position
	*/
	TArray<AFoliageMaskVolume*> GetFoliageMaskList(FVector Position);
	/**
	* Get the List of NoiseVolume from FBox
	* @param pFBox FBox used for Intersect
	* @return List of NoiseVolume affecting the noise in the FBox
	*/
	TArray<AFoliageMaskVolume*> GetFoliageMaskList(FBox pFBox);


	/**
	* Get the Noise from Position
	* @param Position Position to obtain the Noise From
	* @param Water Is the noise sampled for water or ground
	* @return Noises Data
	*/
	UFUNCTION(BlueprintCallable, Category = "Info")
	FNoiseData GetNoise(FVector Position, bool Water = false);
	FNoiseData GetNoise(FVector Position, TArray<ANoiseVolume*> NoiseVolumeListVar, TArray<AHeightMapVolume*> VolumeListVar, TArray<FTransform> VolumeTransformList, bool Water = false);

	/**
	* Get the Noise from Position projected to the ground
	* @param Position Position to obtain the Noise From
	* @param Water Is the noise sampled for water or ground
	* @return Noises Data
	*/
	UFUNCTION(BlueprintCallable, Category = "Info")
	FNoiseData GetGroundNoise(FVector Position, bool Water = false);
	FNoiseData GetGroundNoise(FVector Position, TArray<ANoiseVolume*> NoiseVolumeListVar, TArray<AHeightMapVolume*> VolumeListVar, TArray<FTransform> VolumeTransformList, bool Water = false);
	
	
	/*
	UFUNCTION(BlueprintCallable, Category = "Info")
		FVector GetGroundNormal(FVector ECEFPosition);
	*/

	FVector GetTangentFromECEFPoint(FVector ECEFPosition);
	FVector GetBiTangentFromECEFPoint(FVector ECEFPosition);
	UFUNCTION(BlueprintCallable, Category = "Info")
	FVector GetPawnNormal(FVector PawnWorldPosition);
	UFUNCTION(BlueprintCallable, Category = "Info")
	FVector GetPawnSnappedNormal(FVector PawnWorldPosition);
	UFUNCTION(BlueprintCallable, Category = "Info")
	FVector GetPawnTangent(FVector PawnWorldPosition);
	UFUNCTION(BlueprintCallable, Category = "Info")
	FVector GetPawnBiTangent(FVector PawnWorldPosition);
	UFUNCTION(BlueprintCallable, Category = "Info")
	float GetPawnAltitude(FVector PawnWorldPosition);
	UFUNCTION(BlueprintCallable, Category = "Info")
	float GetPawnDistanceFromGround(FVector PawnPosition,bool ECEFCoordinate = false);

	float GetPawnDistanceFromGround(DVector PawnPosition, bool ECEFCoordinate = false);

	/**
	* Get the Noise Height at set position
	* @param Position Position to obtain the Noise From
	* @param Water Is the noise sampled for water or ground
	* @return float
	*/
	UFUNCTION(BlueprintCallable, Category = "Info")
	float GetGroundHeight(FVector PawnWorldPosition, bool Water = false);
	/**
	* Get the Noise Normalized Height at set position
	* @param Position Position to obtain the Noise From
	* @param Water Is the noise sampled for water or ground
	* @return float
	*/
	UFUNCTION(BlueprintCallable, Category = "Info")
	float GetGroundHeightNormalize(FVector PawnWorldPosition, bool Water = false);


	/**
	* Get the Noise Height at set position
	* @param Position Position to obtain the Noise From
	* @param Water Is the noise sampled for water or ground
	* @return float
	*/
	UFUNCTION(BlueprintCallable, Category = "Info")
	float GetHeight(FVector PawnWorldPosition, bool Water = false);
	/**
	* Get the Noise Normalized Height at set position
	* @param Position Position to obtain the Noise From
	* @param Water Is the noise sampled for water or ground
	* @return float
	*/
	UFUNCTION(BlueprintCallable, Category = "Info")
	float GetHeightNormalize(FVector PawnWorldPosition, bool Water = false);

	float GetLattitude(FVector Position);

	/**
	* Generate the Base Mesh, used when Check for Regenerate is true
	*/
	void GenerateBaseMesh();
	/**
	* Delete all chunks and mesh instances
	*/
	void CleanComponents();
	//Main Update Function.
	void UpdatePosition();
	//Check if lod generation are done, and update them.
	void CheckForLodGeneration();
	//Foliages Related Ticks
	void FoliageHandleTick();

	void CreateInstancedMesh(FFoliageSector* Sector, UWorldScapeFoliage* Foliage);
	void CollisionLodHandler(DVector HardenNormal, double AltitudeMultiplier, bool ResetCollisions, double GroundHeight);
	void CheckCollisionDeletion(DVector HardenNormal, bool ResetCollisions);
	int32 CreateNewColisionMesh(DVector RelativePosition);





	bool init;
	bool HMIForceUpdate;

	//Altitude of the player at the last tick
	float LastAltitude;
	//PlayerPosition In WorldSpace
	DVector PlayerWorldPos;
	//Actual Planet Location 
	DVector PlanetLocation;

	TArray<DVector> CollisionActorList;
	TArray<DVector> SnapedCollisionActorList;
	CustomNoise PlanetNoise;
	TArray<UWorldScapeFoliage*> FoliagesList;

	//Save of previous State of default variable (change lead to a complete new regen)
	EWorldScapeType GenerationType_Previous;
	int32 Prev_MaxLoD;
	int32 Prev_OceanMaxLoD;
	int32 Prev_LodResolution;
	int32 Prev_OceanLodResolution;
	double Prev_PlanetScale;
	float Prev_OceanHeight;
	float Prev_HeightAnchor;
	float Prev_TriangleSize;
	float Prev_OceanTriangleSize;
	UWorldScapeNoiseClass* Prev_WorldScapeNoise;
	bool Prev_Ocean;
	FTransform Prev_PlanetTransform;
	TArray<UWorldScapeFoliageCollection*> Prev_Foliages;
	int32 Prev_NoiseScale;
	int32 Prev_NoiseIntensity;
	int32 Prev_Seed;
	FVector Prev_NoiseOffset;
	FRotator Prev_LattitudeRotation;
	DVector Prev_Normal;
	int32 Prev_HeightMult;

	//Check if a rebase is in progress to wait for it before regenerating lod and foliages 
	UFUNCTION(BlueprintCallable, Category = "Defaults")
	void OnBeginRebase();
	UFUNCTION(BlueprintCallable, Category = "Defaults")
	void OnFinishedRebase();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rebase")
	bool RebaseInProgress = false;


	UFUNCTION(BlueprintCallable, Category = "Defaults")
	void UpdateTerrainMaterial(FWSMaterialLodArray pMaterialLodArray);
	UFUNCTION(BlueprintCallable, Category = "Defaults")
	void UpdateOceanMaterial(FWSMaterialLodArray pMaterialLodArray);
};



class LodGenerationThread : public FNonAbandonableTask
{
public:
	LodGenerationThread(UWorldScapeLod* pWorldScapeLod, AWorldScapeRoot* pRoot, DVector pRelativePosition, DVector pPlanetLocation, FVector2D pSubPosition,
		double pPlanetScale, int32 pLod, float pSize, int32 pVertWidth, double pPlayerAltitude, DVector pHardenNormal, bool pFlatWorld, bool pIsInViewPort);

	~LodGenerationThread();

	//required by ue4, is required 


	bool IsInViewPort;

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(NormalThreadTask, STATGROUP_ThreadPoolAsyncTasks);
	}
	UWorldScapeLod* WorldScapeLodRef;
	AWorldScapeRoot* RootRef;


	LodData MainPatch;
	LodData PatchA;
	LodData PatchB;

	void DoWork();
	void DoWorkMain();
	

	void SetPointPosition();

	//Step 3
	void CalculateNoise();
	void CalculateNormal(LodData &PatchData, int32 PatchID = 0);

	int32 Lod;
	
	DVector OffSetHelper;
	DVector RelativePosition;
	DVector HardenNormal;
	double PlanetScaleCode;

	float Size;
	int32 TriResolution;
	double PlayerDistanceToGround;
	FVector2D SubPosition;

	bool bFlatWorld;
};


class ColisionGeneration
{
public:
	ColisionGeneration(UWorldScapeLod* pWorldScapeLod, AWorldScapeRoot* pRoot, DVector pRelativePosition, DVector pPlanetLocation, FVector2D pSubPosition,
		double pPlanetScale, int32 pLod, float pSize, int32 pVertWidth, double pPlayerAltitude, DVector pHardenNormal, bool pFlatWorld);

	//required by ue4, is required 



	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(NormalThreadTask, STATGROUP_ThreadPoolAsyncTasks);
	}
	UWorldScapeLod* WorldScapeLodRef;
	AWorldScapeRoot* RootRef;


	LodData MainPatch;
	LodData PatchA;
	LodData PatchB;

	void SetPointPosition();
	//Step 3
	void CalculateNoise();

	int32 Lod;

	DVector RelativePosition;
	DVector HardenNormal;
	double PlanetScaleCode;

	float Size;
	int32 TriResolution;
	double PlayerDistanceToGround;
	FVector2D SubPosition;

	bool bFlatWorld;

};


class FoliageGenerationThread : public FNonAbandonableTask
{
public:
	FoliageGenerationThread(AWorldScapeRoot* pRoot,
		double pPlanetScale, bool pFlatWorld,DVector pPlayerPosition ,TArray< FVegetation> pVegetationList, double pPlayerDistanceToGround);

	~FoliageGenerationThread();

	//required by ue4, is required 

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(NormalThreadTask, STATGROUP_ThreadPoolAsyncTasks);
	}
	UWorldScapeLod* WorldScapeLodRef;
	AWorldScapeRoot* RootRef;

	void DoWork();
	void DoWorkMain();


	void GenerateVegetationForSector(FFoliageSector& Sector, int32 FoliageCollectionID, int32 FoliageID, int32 FLID);

	int32 Lod;

	DVector RelativePosition;
	DVector PlayerPos;
	double PlanetScaleCode;

	bool bFlatWorld;
	
	TArray< FVegetation> VegetationList;

	double PlayerDistanceToGround;
};



