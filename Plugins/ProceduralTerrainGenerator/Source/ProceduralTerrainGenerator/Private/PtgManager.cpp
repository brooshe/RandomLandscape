// Copyright 2021 VICTOR HERNANDEZ MOLPECERES (Rockam). All Rights Reserved.

#include "PtgManager.h"
#include "ProceduralTerrainGenerator.h"
#include "PtgFastNoiseLiteWrapper.h"
#include "PtgModifier.h"
#include "PtgUtils.h"
#include "RuntimeMeshComponent.h"
#include "RuntimeMesh.h"
#include "Providers/RuntimeMeshProviderStatic.h"
#include "Components/PrimitiveComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/Material.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"

#if WITH_EDITORONLY_DATA
#include "Components/BillboardComponent.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region FUNCTION POINTER INDEXES

#define BIOMA_RANDOM_ROTATION_FUNC_INDEX 0
#define BIOMA_PLANE_SHAPE_ROTATION_FUNC_INDEX 1
#define BIOMA_CUBE_SHAPE_ROTATION_FUNC_INDEX 2
#define BIOMA_SPHERE_SHAPE_ROTATION_FUNC_INDEX 3
#define BIOMA_MESH_SURFACE_ROTATION_FUNC_INDEX 4

#define TRIANGLE_IS_INSIDE_PLANE_HEIGHT_RANGE_FUNC_INDEX 0
#define TRIANGLE_IS_INSIDE_CUBE_HEIGHT_RANGE_FUNC_INDEX 1
#define TRIANGLE_IS_INSIDE_SPHERE_HEIGHT_RANGE_FUNC_INDEX 2

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region PTG TRIANGLE DEFINITIONS

bool FPtgTriangle::IsInsideCubeHeigthRange(const float lowerHeight, const float higherHeight, const FTransform& terrainTransform) const
{
	const FPtgTriangle& worldSpaceTriangle = GetWorldSpaceTriangle(terrainTransform);

	// Check if triangle is inside the higher height box
	const FVector higherRangeBoxExtent = FVector(higherHeight, higherHeight, higherHeight);
	const bool bIsVertexA_InsideHighRangeBox = UKismetMathLibrary::IsPointInBoxWithTransform(worldSpaceTriangle.VertexA, terrainTransform, higherRangeBoxExtent);
	const bool bIsVertexB_InsideHighRangeBox = UKismetMathLibrary::IsPointInBoxWithTransform(worldSpaceTriangle.VertexB, terrainTransform, higherRangeBoxExtent);
	const bool bIsVertexC_InsideHighRangeBox = UKismetMathLibrary::IsPointInBoxWithTransform(worldSpaceTriangle.VertexC, terrainTransform, higherRangeBoxExtent);

	// Check if triangle is outside the lower height box
	const FVector lowerRangeBoxExtent = FVector(lowerHeight, lowerHeight, lowerHeight);
	const bool bIsVertexA_OutsideLowRangeBox = !UKismetMathLibrary::IsPointInBoxWithTransform(worldSpaceTriangle.VertexA, terrainTransform, lowerRangeBoxExtent);
	const bool bIsVertexB_OutsideLowRangeBox = !UKismetMathLibrary::IsPointInBoxWithTransform(worldSpaceTriangle.VertexB, terrainTransform, lowerRangeBoxExtent);
	const bool bIsVertexC_OutsideLowRangeBox = !UKismetMathLibrary::IsPointInBoxWithTransform(worldSpaceTriangle.VertexC, terrainTransform, lowerRangeBoxExtent);

	const bool bIsVertexA_InsideHeigthRange = bIsVertexA_InsideHighRangeBox && bIsVertexA_OutsideLowRangeBox;
	const bool bIsVertexB_InsideHeigthRange = bIsVertexB_InsideHighRangeBox && bIsVertexB_OutsideLowRangeBox;
	const bool bIsVertexC_InsideHeigthRange = bIsVertexC_InsideHighRangeBox && bIsVertexC_OutsideLowRangeBox;

	return (bIsVertexA_InsideHeigthRange && bIsVertexB_InsideHeigthRange)
		|| (bIsVertexA_InsideHeigthRange && bIsVertexC_InsideHeigthRange)
		|| (bIsVertexB_InsideHeigthRange && bIsVertexC_InsideHeigthRange);
}

bool FPtgTriangle::IsInsideShape(const UShapeComponent* shapeComponent) const
{
	if (const USphereComponent* sphereComponent = Cast<USphereComponent>(shapeComponent))
	{
		const FVector& shapeWorldPosition = shapeComponent->GetComponentLocation();
		const float sphereRadius = sphereComponent->GetScaledSphereRadius();
		const bool bIsVertexA_InsideShape = FVector::Distance(shapeWorldPosition, VertexA) < sphereRadius;
		const bool bIsVertexB_InsideShape = FVector::Distance(shapeWorldPosition, VertexB) < sphereRadius;
		const bool bIsVertexC_InsideShape = FVector::Distance(shapeWorldPosition, VertexC) < sphereRadius;

		// If two triangle points are inside the sphere then treat the whole triangle as inside the sphere
		return (bIsVertexA_InsideShape && bIsVertexB_InsideShape)
			|| (bIsVertexA_InsideShape && bIsVertexC_InsideShape)
			|| (bIsVertexB_InsideShape && bIsVertexC_InsideShape);
	}
	else if (const UBoxComponent* boxComponent = Cast<UBoxComponent>(shapeComponent))
	{
		const FTransform& shapeTransform = shapeComponent->GetComponentTransform();
		const FVector& boxExtent = boxComponent->GetUnscaledBoxExtent();
		const bool bIsVertexA_InsideShape = UKismetMathLibrary::IsPointInBoxWithTransform(VertexA, shapeTransform, boxExtent);
		const bool bIsVertexB_InsideShape = UKismetMathLibrary::IsPointInBoxWithTransform(VertexB, shapeTransform, boxExtent);
		const bool bIsVertexC_InsideShape = UKismetMathLibrary::IsPointInBoxWithTransform(VertexC, shapeTransform, boxExtent);

		// If two triangle points are inside the box then treat the whole triangle as inside the box
		return (bIsVertexA_InsideShape && bIsVertexB_InsideShape)
			|| (bIsVertexA_InsideShape && bIsVertexC_InsideShape)
			|| (bIsVertexB_InsideShape && bIsVertexC_InsideShape);
	}

	return false;
}

FPtgTriangle FPtgTriangle::GetWorldSpaceTriangle(const FTransform& transform) const
{
	return FPtgTriangle
	(
		UKismetMathLibrary::TransformLocation(transform, VertexA),
		UKismetMathLibrary::TransformLocation(transform, VertexB),
		UKismetMathLibrary::TransformLocation(transform, VertexC)
	);
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int32 SEED_MIN = -1000000;
const int32 SEED_MAX = 1000000;
const float MIN_ANGLE = 0.0f;
const float MAX_ANGLE = 359.99f;
const float PI2 = PI * 2.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region SETUP

APtgManager::APtgManager()
{
	// Disable tick on start, cause is not needed
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Set default values
	Radius = 15000.0f;
	Resolution = 150;
	NumberOfLODs = 8;
	LODScreenSizeMultiplier = 0.5f;
	bGenerateEverythingOnPropertyChange = false;
	Shape = EPtgProcMeshShapes::Plane;
	bUseTerrainTiling = false;
	NoiseInputScale = 1.0f;
	NoiseOutputScale = 1000.0f;
	TerrainMaterial = nullptr;
	bEnableTerrainCollision = true;
	LowestGeneratedHeight = 0.0f;
	HighestGeneratedHeight = 0.0f;
	bGenerateWater = true;
	WaterHeightGenerationType = EPtgWaterHeightGenerationType::RandomPercentage;
	WaterSeed = DEFAULT_SEED;
	WaterRandomHeightRangePercentages = FVector2D(10.0f, 50.0f);
	WaterFixedHeightPercentage = 50.0f;
	WaterFixedHeightValue = 0.0f;
	WaterMaterial = nullptr;
	bEnableWaterCollision = false;
	WaterHeight = 0.0f;
	bGenerateNature = true;
	bGenerateActors = true;
	NoiseType = EPtgFastNoiseLiteWrapperNoiseType::NoiseType_OpenSimplex2;
	RotationType3D = EPtgFastNoiseLiteWrapperRotationType3D::RotationType3D_None;
	Seed = DEFAULT_SEED;
	Frequency = 0.02f;
	FractalType = EPtgFastNoiseLiteWrapperFractalType::FractalType_FBm;
	FractalOctaves = 5;
	FractalLacunarity = 2.0f;
	FractalGain = 0.5f;
	FractalWeightedStrength = 0.0f;
	FractalPingPongStrength = 2.0f;
	CellularDistanceFunction = EPtgFastNoiseLiteWrapperCellularDistanceFunction::CellularDistanceFunction_EuclideanSq;
	CellularReturnType = EPtgFastNoiseLiteWrapperCellularReturnType::CellularReturnType_Distance;
	CellularJitter = 1.0f;
	DomainWarpType = EPtgFastNoiseLiteWrapperDomainWarpType::DomainWarpType_None;
	DomainWarpAmplitude = 30.0f;
	bShowDebugMessages = true;
	DebugMessagesTimeOnScreen = 8.0f;
	terrainRuntimeMesh = nullptr;
	waterRuntimeMesh = nullptr;

	// Create Fast Noise wrapper
	fastNoiseWrapper = CreateDefaultSubobject<UPtgFastNoiseLiteWrapper>(TEXT("FastNoiseLiteWrapper"));

	// Create root component as a scene component
	RootComp = CreateDefaultSubobject<USceneComponent>("RootComponent");
	if (IsValid(RootComp))
	{
		RootComp->SetMobility(EComponentMobility::Movable);
		SetRootComponent(RootComp);

		// Create procedural mesh component for terrain and attach it to the root component
		ProcMeshTerrainComp = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("ProcMeshTerrainComp"));
		if (IsValid(ProcMeshTerrainComp))
		{
			ProcMeshTerrainComp->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			ProcMeshTerrainComp->SetEnableGravity(false);
			ProcMeshTerrainComp->bApplyImpulseOnDamage = false;
			ProcMeshTerrainComp->SetGenerateOverlapEvents(false);
			ProcMeshTerrainComp->SetCastShadow(false);
			ProcMeshTerrainComp->SetMobility(EComponentMobility::Movable);
			ProcMeshTerrainComp->SetupAttachment(RootComp);
		}

		// Create procedural mesh component for water and attach it to the root component
		ProcMeshWaterComp = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("ProcMeshWaterComp"));
		if (IsValid(ProcMeshWaterComp))
		{
			ProcMeshWaterComp->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			ProcMeshWaterComp->SetEnableGravity(false);
			ProcMeshWaterComp->bApplyImpulseOnDamage = false;
			ProcMeshWaterComp->SetGenerateOverlapEvents(false);
			ProcMeshWaterComp->SetCastShadow(false);
			ProcMeshWaterComp->SetMobility(EComponentMobility::Movable);
			ProcMeshWaterComp->SetupAttachment(RootComp);
		}

#if WITH_EDITORONLY_DATA
		SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("PtgManagerBillboard"));
		if (IsValid(SpriteComponent))
		{
			SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
			SpriteComponent->bHiddenInGame = true;
			SpriteComponent->bIsScreenSizeScaled = true;
			SpriteComponent->bIsEditorOnly = true;
			SpriteComponent->SetupAttachment(RootComp);
		}
#endif
	}

	// Disable another actor unused stuff
	SetCanBeDamaged(false);
	bFindCameraComponentWhenViewTarget = 0;

	// Init function pointers
	InitFuncPtrs();
}

void APtgManager::SetupFastNoiseLite()
{
	// Create fast noise lite wrapper in the case that it lost the reference
	if (fastNoiseWrapper == nullptr)
	{
		fastNoiseWrapper = NewObject<UPtgFastNoiseLiteWrapper>(this, TEXT("FastNoiseLiteWrapper"));

		if (fastNoiseWrapper == nullptr)
		{
			UPtgUtils::PrintDebugMessage(this, TEXT("Cannot create fastNoiseWrapper, please, respawn the PTG actor on the map."));
			return;
		}
	}

	fastNoiseWrapper->SetupFastNoiseLite(
		Seed, Frequency, NoiseType, RotationType3D,
		FractalType, FractalOctaves, FractalLacunarity, FractalGain, FractalWeightedStrength, FractalPingPongStrength,
		CellularDistanceFunction, CellularReturnType, CellularJitter,
		DomainWarpType, DomainWarpAmplitude);
}

void APtgManager::Destroyed()
{
	procMeshData.ClearData();
	vertexHeightData.Empty();
	ClearTerrainMesh();
	ClearWaterMesh();
	ClearNatureAndActors();

	Super::Destroyed();
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region FUNCTION POINTERS SETUP

void APtgManager::InitFuncPtrs()
{
	getBiomaRotationFuncs[BIOMA_RANDOM_ROTATION_FUNC_INDEX] = &APtgManager::GetBiomaRandomRotation;
	getBiomaRotationFuncs[BIOMA_PLANE_SHAPE_ROTATION_FUNC_INDEX] = &APtgManager::GetBiomaPlaneShapeRotation;
	getBiomaRotationFuncs[BIOMA_CUBE_SHAPE_ROTATION_FUNC_INDEX] = &APtgManager::GetBiomaCubeShapeRotation;
	getBiomaRotationFuncs[BIOMA_SPHERE_SHAPE_ROTATION_FUNC_INDEX] = &APtgManager::GetBiomaSphereShapeRotation;
	getBiomaRotationFuncs[BIOMA_MESH_SURFACE_ROTATION_FUNC_INDEX] = &APtgManager::GetBiomaMeshSurfaceRotation;

	isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_PLANE_HEIGHT_RANGE_FUNC_INDEX] = &FPtgTriangle::IsInsidePlaneHeigthRange;
	isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_CUBE_HEIGHT_RANGE_FUNC_INDEX] = &FPtgTriangle::IsInsideCubeHeigthRange;
	isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_SPHERE_HEIGHT_RANGE_FUNC_INDEX] = &FPtgTriangle::IsInsideSphereHeigthRange;
}

FQuat APtgManager::GetRandomYawQuat(FRandomStream& randomNumberGenerator) const
{
	return FQuat(FVector::UpVector, randomNumberGenerator.FRandRange(0.0f, PI2));
}

FQuat APtgManager::GetBiomaRandomRotation(FRandomStream& randomNumberGenerator, const FPtgTriangle& triangle, const FVector& location) const
{
	// Get a random rotation
	return FRotator(randomNumberGenerator.FRandRange(MIN_ANGLE, MAX_ANGLE), randomNumberGenerator.FRandRange(MIN_ANGLE, MAX_ANGLE), randomNumberGenerator.FRandRange(MIN_ANGLE, MAX_ANGLE)).Quaternion();
}

FQuat APtgManager::GetBiomaPlaneShapeRotation(FRandomStream& randomNumberGenerator, const FPtgTriangle& triangle, const FVector& location) const
{
	// Get a random yaw rotation
	return GetRandomYawQuat(randomNumberGenerator);
}

FQuat APtgManager::GetBiomaCubeShapeRotation(FRandomStream& randomNumberGenerator, const FPtgTriangle& triangle, const FVector& location) const
{
	// Get cube vertex face normal and apply a random yaw rotation
	return triangle.CubeVertexFaceNormal * GetRandomYawQuat(randomNumberGenerator);
}

FQuat APtgManager::GetBiomaSphereShapeRotation(FRandomStream& randomNumberGenerator, const FPtgTriangle& triangle, const FVector& location) const
{
	// Apply pitch correction and randomize yaw
	return (UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, location) - FRotator(90.0f, 0.0f, 0.0f)).Quaternion() * GetRandomYawQuat(randomNumberGenerator);
}

FQuat APtgManager::GetBiomaMeshSurfaceRotation(FRandomStream& randomNumberGenerator, const FPtgTriangle& triangle, const FVector& location) const
{
	// Get surface normal and apply a random yaw rotation
	return triangle.GetSurfaceNormal() * GetRandomYawQuat(randomNumberGenerator);
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void APtgManager::PlaceTerrain()
{
	
}

#pragma region GENERATION OF EVERYTHING

void APtgManager::GenerateEverything()
{
	const double startTime = FPlatformTime::Seconds();

	// Generate procedural meshes for terrain and water, generate the nature meshes and spawn actors
	GenerateTerrainMesh();
	if (bGenerateWater) GenerateWaterMesh();
	if (bGenerateNature) GenerateNature();
	if (bGenerateActors) GenerateActors();

	// Debug
	if (bShowDebugMessages)
	{
		UPtgUtils::PrintDebugMessage(this, TEXT("Generate everything took ") + FString::SanitizeFloat(FPlatformTime::Seconds() - startTime) + TEXT(" seconds in total."), EPtgDebugMessageTypes::Info, DebugMessagesTimeOnScreen);
	}
}

void APtgManager::GenerateEverythingWithCustomSeed(const int32 terrainSeed, const int32 waterSeed, const int32 natureSeed, const int32 actorsSeed)
{
	// Create fast noise lite wrapper in the case that it lost the reference
	if (fastNoiseWrapper == nullptr)
	{
		fastNoiseWrapper = NewObject<UPtgFastNoiseLiteWrapper>(this, TEXT("FastNoiseLiteWrapper"));

		if (fastNoiseWrapper == nullptr)
		{
			UPtgUtils::PrintDebugMessage(this, TEXT("Cannot create fastNoiseWrapper, please, respawn the PTG actor on the map."), EPtgDebugMessageTypes::Info, DebugMessagesTimeOnScreen);
			return;
		}
	}

	// Set random seeds for everything
	Seed = terrainSeed;
	fastNoiseWrapper->SetSeed(terrainSeed);

	if (bGenerateWater && WaterHeightGenerationType == EPtgWaterHeightGenerationType::RandomPercentage)
	{
		WaterSeed = waterSeed;
	}

	if (bGenerateNature)
	{
		for (FPtgBiomaNature& biomaNature : BiomaNature)
		{
			biomaNature.Seed = natureSeed;
		}
	}

	if (bGenerateActors)
	{
		for (FPtgBiomaActors& biomaActor : BiomaActors)
		{
			biomaActor.Seed = actorsSeed;
		}
	}

	// Then, generate the terrain
	GenerateEverything();
}

void APtgManager::GenerateEverythingWithRandomSeed()
{
	GenerateEverythingWithCustomSeed(FMath::RandRange(SEED_MIN, SEED_MAX), FMath::RandRange(SEED_MIN, SEED_MAX), FMath::RandRange(SEED_MIN, SEED_MAX), FMath::RandRange(SEED_MIN, SEED_MAX));
}

#pragma endregion

#pragma region TERRAIN GENERATION

void APtgManager::GenerateTerrainMesh()
{
	// Create fast noise wrapper in the case that it lost the reference
	if (fastNoiseWrapper == nullptr)
	{
		fastNoiseWrapper = NewObject<UPtgFastNoiseLiteWrapper>(this, TEXT("FastNoiseLiteWrapper"));

		if (fastNoiseWrapper == nullptr)
		{
			UPtgUtils::PrintDebugMessage(this, TEXT("Cannot create fastNoiseWrapper, please, respawn the PTG actor on the map."), EPtgDebugMessageTypes::Error, DebugMessagesTimeOnScreen);
			return;
		}
	}

	// If Fast Noise is not initialized, initialize it first
	if (!fastNoiseWrapper->IsInitialized())
	{
		SetupFastNoiseLite();
	}
	
	const double startTime = FPlatformTime::Seconds();

	// Get or create provider
	URuntimeMeshProviderStatic* staticProvider = nullptr;

	if (URuntimeMeshProvider* provider = ProcMeshTerrainComp->GetProvider())
	{
		staticProvider = Cast<URuntimeMeshProviderStatic>(provider);
	}
	else
	{
		staticProvider = NewObject<URuntimeMeshProviderStatic>(this, TEXT("RuntimeMeshProvider-Static_Terrain"));

		// The static provider should initialize before we use it
		ProcMeshTerrainComp->Initialize(staticProvider);
	}

	if (staticProvider != nullptr)
	{
		ClearTerrainMesh();

		// Generate LODs configurations
		TArray<FRuntimeMeshLODProperties> LODsProperties;
		for (int32 LODIndex = 0; LODIndex < NumberOfLODs; LODIndex++)
		{
			FRuntimeMeshLODProperties currentLOD;
			currentLOD.ScreenSize = (LODIndex == 0) ? 1.0f : FMath::Pow(LODScreenSizeMultiplier, LODIndex);
			LODsProperties.Emplace(currentLOD);
		}
		staticProvider->ConfigureLODs(LODsProperties);
		staticProvider->MarkAllLODsDirty();

		procMeshData.bEnableCollision = bEnableTerrainCollision;

		// Generate meshes for every LOD
		for (int32 LODIndex = 1; LODIndex <= NumberOfLODs; LODIndex++)
		{
			const bool bIsFirstIteration = LODIndex == 1;

			// Only set the Lowest and Highest generated height values on the first iteration (LOD 0), to avoid the rest of the LODs messing everything up
			float dummyHeight = 0.0f;
			float& lowestGeneratedHeightPtr = bIsFirstIteration ? LowestGeneratedHeight : dummyHeight;
			float& highestGeneratedHeightPtr = bIsFirstIteration ? HighestGeneratedHeight : dummyHeight;

			// Generate tile grid depending on the shape using noise
			switch (Shape)
			{
			case EPtgProcMeshShapes::Cube:

				UPtgProcMeshDataHelper::GenerateCubeData(procMeshData, lowestGeneratedHeightPtr, highestGeneratedHeightPtr, Radius, Resolution / LODIndex, fastNoiseWrapper, NoiseInputScale * LODIndex, NoiseOutputScale);
				break;

			case EPtgProcMeshShapes::Sphere:

				UPtgProcMeshDataHelper::GenerateSphereData(procMeshData, lowestGeneratedHeightPtr, highestGeneratedHeightPtr, Radius, Resolution / LODIndex, fastNoiseWrapper, NoiseInputScale * LODIndex, NoiseOutputScale);
				break;

			case EPtgProcMeshShapes::Plane:
			default:

				UPtgProcMeshDataHelper::GeneratePlaneData(procMeshData, lowestGeneratedHeightPtr, highestGeneratedHeightPtr, Radius, Resolution / LODIndex, fastNoiseWrapper, NoiseInputScale * LODIndex, NoiseOutputScale, bUseTerrainTiling ? GetActorLocation() : FVector::ZeroVector);

				// Save vertex height data in the first iteration (LOD 0) to retrieve it later on heightmap creation request
				if (bIsFirstIteration)
				{
					vertexHeightData.Empty();
					for (const FVector& vertexData : procMeshData.Vertices) vertexHeightData.Emplace(vertexData.Z);
				}
			}

			// Regenerate the procedural terrain mesh
			staticProvider->CreateSectionFromComponents(LODIndex - 1, procMeshData.SectionIndex, 0, procMeshData.Vertices, procMeshData.Triangles, procMeshData.Normals, procMeshData.UV0, procMeshData.VertexColors, procMeshData.Tangents, ERuntimeMeshUpdateFrequency::Infrequent, procMeshData.bEnableCollision);

			// Save the terrain triangles in the first iteration (LOD 0)
			if (bIsFirstIteration)
			{
				const uint32 numTris = procMeshData.GetNumTriangles();

				// Get procedural mesh triangles information to retrieve them later on nature & actors generation
				if (Shape == EPtgProcMeshShapes::Cube)
				{
					for (uint32 triangleIndex = 0; triangleIndex < numTris; triangleIndex += 3)
					{
						const int32* currentTriangle = &(procMeshData.Triangles[triangleIndex]);
						terrainTriangles.Emplace(FPtgTriangle(
							procMeshData.Vertices[*currentTriangle],
							procMeshData.Vertices[*(currentTriangle + 1)],
							procMeshData.Vertices[*(currentTriangle + 2)],
							procMeshData.CubeVertexFaceAssetRotation[*currentTriangle]));
					}
				}
				else
				{
					for (uint32 triangleIndex = 0; triangleIndex < numTris; triangleIndex += 3)
					{
						const int32* currentTriangle = &(procMeshData.Triangles[triangleIndex]);
						terrainTriangles.Emplace(FPtgTriangle(
							procMeshData.Vertices[*currentTriangle],
							procMeshData.Vertices[*(currentTriangle + 1)],
							procMeshData.Vertices[*(currentTriangle + 2)]));
					}
				}
			}
		}

		staticProvider->SetupMaterialSlot(0, TEXT("TerrainMaterial"), TerrainMaterial);
	}

	// Debug
	if (bShowDebugMessages)
	{
		UPtgUtils::PrintDebugMessage(this, TEXT("Terrain mesh generation took ") + FString::SanitizeFloat(FPlatformTime::Seconds() - startTime) + TEXT(" seconds."), EPtgDebugMessageTypes::Info, DebugMessagesTimeOnScreen);
	}
}

void APtgManager::GenerateTerrainMeshWithCustomSeed(const int32 terrainSeed)
{
	// Set random seeds for everything
	Seed = terrainSeed;
	fastNoiseWrapper->SetSeed(terrainSeed);

	GenerateTerrainMesh();
}

void APtgManager::GenerateTerrainMeshWithRandomSeed()
{
	GenerateTerrainMeshWithCustomSeed(FMath::RandRange(SEED_MIN, SEED_MAX));
}

void APtgManager::ClearTerrainMesh()
{
	LowestGeneratedHeight = HighestGeneratedHeight = 0.0f;

	// Clear current sections
	if (URuntimeMeshProviderStatic* staticProvider = Cast<URuntimeMeshProviderStatic>(ProcMeshTerrainComp->GetProvider()))
	{
		staticProvider->ClearAllLODsForSection(0);
	}

	terrainTriangles.Empty();
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region WATER GENERATION

void APtgManager::GenerateWaterMesh()
{
	ClearWaterMesh();

	if (bGenerateWater)
	{
		const double startTime = FPlatformTime::Seconds();

		CalculateAndApplyWaterMeshHeight();

		// Get or create provider
		URuntimeMeshProviderStatic* staticProvider = nullptr;

		if (URuntimeMeshProvider* provider = ProcMeshWaterComp->GetProvider())
		{
			staticProvider = Cast<URuntimeMeshProviderStatic>(provider);
		}
		else
		{
			staticProvider = NewObject<URuntimeMeshProviderStatic>(this, TEXT("RuntimeMeshProvider-Static_Water"));

			// The static provider should initialize before we use it
			ProcMeshWaterComp->Initialize(staticProvider);
		}

		if (staticProvider != nullptr)
		{
			// Generate LODs configurations
			TArray<FRuntimeMeshLODProperties> LODsProperties;
			for (int32 LODIndex = 0; LODIndex < NumberOfLODs; LODIndex++)
			{
				FRuntimeMeshLODProperties currentLOD;
				currentLOD.ScreenSize = (LODIndex == 0) ? 1.0f : FMath::Pow(LODScreenSizeMultiplier, LODIndex);
				LODsProperties.Emplace(currentLOD);
			}
			staticProvider->ConfigureLODs(LODsProperties);
			staticProvider->MarkAllLODsDirty();

			procMeshData.bEnableCollision = bEnableWaterCollision;

			// Generate meshes for every LOD
			for (int32 LODIndex = 1; LODIndex <= NumberOfLODs; LODIndex++)
			{
				switch (Shape)
				{
				case EPtgProcMeshShapes::Cube:

					UPtgProcMeshDataHelper::GenerateCubeData(procMeshData, LowestGeneratedHeight, HighestGeneratedHeight, WaterHeight, Resolution / LODIndex);
					break;

				case EPtgProcMeshShapes::Sphere:

					UPtgProcMeshDataHelper::GenerateSphereData(procMeshData, LowestGeneratedHeight, HighestGeneratedHeight, WaterHeight, Resolution / LODIndex);
					break;

				case EPtgProcMeshShapes::Plane:
				default:

					UPtgProcMeshDataHelper::GeneratePlaneData(procMeshData, LowestGeneratedHeight, HighestGeneratedHeight, Radius, Resolution / LODIndex);
				}

				// Regenerate the procedural water mesh
				staticProvider->CreateSectionFromComponents(LODIndex - 1, procMeshData.SectionIndex, 0, procMeshData.Vertices, procMeshData.Triangles, procMeshData.Normals, procMeshData.UV0, procMeshData.VertexColors, procMeshData.Tangents, ERuntimeMeshUpdateFrequency::Infrequent, procMeshData.bEnableCollision);
			}

			staticProvider->SetupMaterialSlot(0, TEXT("WaterMaterial"), WaterMaterial);
		}

		// Debug
		if (bShowDebugMessages)
		{
			UPtgUtils::PrintDebugMessage(this, TEXT("Water mesh generation took ") + FString::SanitizeFloat(FPlatformTime::Seconds() - startTime) + TEXT(" seconds."), EPtgDebugMessageTypes::Info, DebugMessagesTimeOnScreen);
		}
	}
	else
	{
		UPtgUtils::PrintDebugMessage(this, TEXT("To use this action enable Generate Water!"), EPtgDebugMessageTypes::Warning, DebugMessagesTimeOnScreen);
	}
}

void APtgManager::GenerateWaterMeshWithCustomSeed(const int32 waterSeed)
{
	if (bGenerateWater && WaterHeightGenerationType == EPtgWaterHeightGenerationType::RandomPercentage)
	{
		// Set seed for water height
		WaterSeed = waterSeed;

		// Then generate water mesh
		GenerateWaterMesh();
	}
	else
	{
		UPtgUtils::PrintDebugMessage(this, TEXT("To use this action enable Generate Water and set Water Height Generation Type to Random Percentage!"), EPtgDebugMessageTypes::Warning, DebugMessagesTimeOnScreen);
	}
}

void APtgManager::GenerateWaterMeshWithRandomSeed()
{
	GenerateWaterMeshWithCustomSeed(FMath::RandRange(SEED_MIN, SEED_MAX));
}

void APtgManager::ClearWaterMesh()
{
	// If there will be no water, equal its height to the lowest of the terrain, for nature generation purposes
	WaterHeight = LowestGeneratedHeight;

	// Clear current sections
	if (URuntimeMeshProviderStatic* staticProvider = Cast<URuntimeMeshProviderStatic>(ProcMeshWaterComp->GetProvider()))
	{
		staticProvider->ClearAllLODsForSection(0);
	}
}

void APtgManager::CalculateAndApplyWaterMeshHeight()
{
	const float terrainHeightDifference = HighestGeneratedHeight - LowestGeneratedHeight;

	switch (WaterHeightGenerationType)
	{
	case EPtgWaterHeightGenerationType::RandomPercentage:
	{
		FRandomStream randomNumberGenerator;
		randomNumberGenerator.Initialize(WaterSeed);

		WaterHeight = LowestGeneratedHeight + (randomNumberGenerator.RandRange(
			terrainHeightDifference * (WaterRandomHeightRangePercentages.X / 100.0f),
			terrainHeightDifference * (WaterRandomHeightRangePercentages.Y / 100.0f))
			);
		break;
	}

	case EPtgWaterHeightGenerationType::FixedPercentage:

		WaterHeight = LowestGeneratedHeight + (terrainHeightDifference * (WaterFixedHeightPercentage / 100.0f));
		break;

	case EPtgWaterHeightGenerationType::FixedHeight:
	default:
		WaterHeight = WaterFixedHeightValue;
	}

	switch (Shape)
	{
	case EPtgProcMeshShapes::Cube:
	case EPtgProcMeshShapes::Sphere:

		ProcMeshWaterComp->SetRelativeLocation(FVector::ZeroVector);
		
		break;

	case EPtgProcMeshShapes::Plane:
	default:
	{
		const FVector currentWaterRelativeLocation = ProcMeshWaterComp->GetRelativeTransform().GetLocation();
		const FVector newWaterRelativeLocation = FVector(0.0f, 0.0f, WaterHeight);

		// As changing transform could be heavy for huge meshes (as the ones we could have here) set relative location only if needed
		if (currentWaterRelativeLocation != newWaterRelativeLocation)
		{
			ProcMeshWaterComp->SetRelativeLocation(newWaterRelativeLocation);
		}
	}
	}
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region NATURE GENERATION

void APtgManager::GenerateNature()
{
	if (bGenerateNature)
	{
		// Clear the current nature meshes and fill the biomas locations arrays to generate the nature meshes on its corresponding locations
		ClearNature();

		if (terrainTriangles.Num() == 0)
		{
			UPtgUtils::PrintDebugMessage(this, TEXT("No vertex data found! please, generate terrain before generating nature."), EPtgDebugMessageTypes::Error, DebugMessagesTimeOnScreen);
			return;
		}

		// Generate new nature
		GenerateNatureInternal();
	}
	else
	{
		UPtgUtils::PrintDebugMessage(this, TEXT("To use this action enable Generate Nature!"), EPtgDebugMessageTypes::Error, DebugMessagesTimeOnScreen);
	}
}

void APtgManager::GenerateNatureWithCustomSeed(const int32 natureSeed)
{
	// Set random seed for every bioma nature
	for (FPtgBiomaNature& biomaNature : BiomaNature)
	{
		biomaNature.Seed = natureSeed;
	}

	// Then, generate nature
	GenerateNature();
}

void APtgManager::GenerateNatureWithRandomSeed()
{
	GenerateNatureWithCustomSeed(FMath::RandRange(SEED_MIN, SEED_MAX));
}

void APtgManager::GenerateNatureInternal()
{
	const double startTime = FPlatformTime::Seconds();
	uint32 biomaNaturesCounter = 0;
	FRandomStream randomNumberGenerator;
	IsInsideHeigthRangeFuncPtr isInsideHeigthRangeFunction = nullptr;
	
	// Choose function to calculate if a triangle is inside heigh range depending on the terrain shape
	switch (Shape)
	{
	case EPtgProcMeshShapes::Cube:

		isInsideHeigthRangeFunction = isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_CUBE_HEIGHT_RANGE_FUNC_INDEX];
		break;

	case EPtgProcMeshShapes::Sphere:

		isInsideHeigthRangeFunction = isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_SPHERE_HEIGHT_RANGE_FUNC_INDEX];
		break;

	case EPtgProcMeshShapes::Plane:
	default:

		isInsideHeigthRangeFunction = isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_PLANE_HEIGHT_RANGE_FUNC_INDEX];
	}

	// Iterate through every bioma nature locating every random-picked mesh on its corresponding bioma and generating a random bunch of them, with other random stuff...
	for (const FPtgBiomaNature& biomaNature : BiomaNature)
	{
		const uint32 numBiomaMeshes = biomaNature.Meshes.Num();

		// If there are no meshes on this bioma go to the next one
		if (numBiomaMeshes == 0) continue;

		// Initialize seed for random generation
		randomNumberGenerator.Initialize(biomaNature.Seed);

		// Generate a random number of meshes for this bioma nature between a defined min. and max.
		const uint32 meshesToSpawn = randomNumberGenerator.RandRange(biomaNature.MinMeshesToSpawn, biomaNature.MaxMeshesToSpawn);

		// Get the candidate triangles of the terrain taking into account modifiers and height percentage
		TArray<int32> triangleCandidateIndexes;
		TArray<FPtgTriangle> shuffledTerrainTriangles = terrainTriangles;
		GetTriangleCandidates
		(
			biomaNature.Modifiers,
			biomaNature.bUseLocationsOutsideModifiers,
			biomaNature.HeightPercentageRangeToLocateNatureMeshes,
			biomaNature.bUseLocationsOutsideHeightRange,
			meshesToSpawn,
			isInsideHeigthRangeFunction,
			biomaNature.CorrespondingBioma,
			randomNumberGenerator,
			triangleCandidateIndexes,
			shuffledTerrainTriangles
		);

		const uint32 numTriangleCandidates = triangleCandidateIndexes.Num();

		// If there are no triangle candidates on this iteration go to the next one
		if (numTriangleCandidates == 0) continue;

		GetBiomaRotationFuncPtr getBiomaElementRotationFunction = nullptr;

		switch (biomaNature.RotationType)
		{
		case EPtgNatureRotationTypes::Random:

			getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_RANDOM_ROTATION_FUNC_INDEX];
			break;

		case EPtgNatureRotationTypes::TerrainShapeNormal:

			switch (Shape)
			{
			case EPtgProcMeshShapes::Cube:

				getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_CUBE_SHAPE_ROTATION_FUNC_INDEX];
				break;

			case EPtgProcMeshShapes::Sphere:

				getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_SPHERE_SHAPE_ROTATION_FUNC_INDEX];
				break;

			case EPtgProcMeshShapes::Plane:
			default:

				getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_PLANE_SHAPE_ROTATION_FUNC_INDEX];
			}

			break;

		case EPtgNatureRotationTypes::MeshSurfaceNormal:
		default:
			getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_MESH_SURFACE_ROTATION_FUNC_INDEX];
		}

		// Cache other stuff to avoid inecessary access and calculations on loop time
		const uint32 numBiomaMeshesLessOne = numBiomaMeshes - 1;
		const uint32 numTriangleCandidatesLessOne = numTriangleCandidates - 1;
		const float minScale = biomaNature.MinMaxScale.X;
		const float maxScale = biomaNature.MinMaxScale.Y;
		const TArray<UStaticMesh*>& meshes = biomaNature.Meshes;
		const int32 cullDistance = biomaNature.CullDistance;
		FTransform meshTransform;

		// Iterate through every mesh to generate, giving him a new random location, scale and rotation if needed
		for (uint32 newMeshIndex = 0; newMeshIndex < meshesToSpawn; newMeshIndex++)
		{
			// If all triangle candidates has been already used, take a random one
			const int32 triangleCandidateIndex = newMeshIndex <= numTriangleCandidatesLessOne
				? newMeshIndex
				: randomNumberGenerator.RandRange(0, numTriangleCandidatesLessOne);

			const FPtgTriangle& randomTriangle = shuffledTerrainTriangles[triangleCandidateIndexes[triangleCandidateIndex]];

			// Pick a random available location on triangle
			meshTransform.SetLocation(randomTriangle.GetRandomPointOnTriangle(randomNumberGenerator));

			// Set rotation
			meshTransform.SetRotation(((this->*getBiomaElementRotationFunction)(randomNumberGenerator, randomTriangle, meshTransform.GetLocation())));

			// Generate a random mesh scale between a defined min. and max.
			const float randomScale = randomNumberGenerator.FRandRange(minScale, maxScale);
			meshTransform.SetScale3D(FVector(randomScale, randomScale, randomScale));

			// Pick a random mesh from the array
			const int32 meshToPickIndex = randomNumberGenerator.RandRange(0, numBiomaMeshesLessOne);
			UStaticMesh* meshToPick = meshes[meshToPickIndex];
			if (meshToPick == nullptr) continue;	// if mesh is empty, continue

			// If HISM was found, add new instance
			if (UHierarchicalInstancedStaticMeshComponent** HISM_ref = NatureStaticMeshHISM_Correspondence.Find(meshToPick))
			{
				(*HISM_ref)->AddInstance(meshTransform);
				biomaNaturesCounter++;
			}
			// If there is no corresponding HISM component, create one and store it
			else if (UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this))
			{
				// The property documentation says that this should be enabled only for small objects without collision, and that's not the case, so we disable it
				HISM->bEnableDensityScaling = 0;	// false
				HISM->SetStaticMesh(meshToPick);
				HISM->SetEnableGravity(false);
				HISM->bApplyImpulseOnDamage = false;
				HISM->SetGenerateOverlapEvents(false);
				HISM->SetCullDistances(0, cullDistance);
				HISM->SetMobility(EComponentMobility::Movable);
				HISM->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
				HISM->RegisterComponent();

				// Add the new HISM to the map
				NatureStaticMeshHISM_Correspondence.Emplace(meshToPick, HISM);

				// Finally, add instance
				HISM->AddInstance(meshTransform);
				biomaNaturesCounter++;
			}
		}
	}

	// Debug
	if (bShowDebugMessages && biomaNaturesCounter > 0)
	{
		UPtgUtils::PrintDebugMessage(this, FString::FromInt(biomaNaturesCounter) + TEXT(" bioma nature meshes generated in ") + FString::SanitizeFloat(FPlatformTime::Seconds() - startTime) + TEXT(" seconds."), EPtgDebugMessageTypes::Info, DebugMessagesTimeOnScreen);
	}
}

void APtgManager::ClearNature()
{
	for (auto& elem : NatureStaticMeshHISM_Correspondence)
	{
		if (UHierarchicalInstancedStaticMeshComponent* HISM = elem.Value)
		{
			HISM->ClearInstances();
			HISM->DestroyComponent();
			HISM = nullptr;
		}

		elem.Key = nullptr;
	}

	NatureStaticMeshHISM_Correspondence.Empty();
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region ACTORS GENERATION

void APtgManager::GenerateActors()
{
	if (bGenerateActors)
	{
		// Clear the current spawned actors and fill the biomas locations arrays to generate the actors on its corresponding locations
		ClearActors();

		if (terrainTriangles.Num() == 0)
		{
			UPtgUtils::PrintDebugMessage(this, TEXT("No vertex data found! please, generate terrain before generating actors."), EPtgDebugMessageTypes::Error, DebugMessagesTimeOnScreen);
			return;
		}

		// Generate new actors
		GenerateActorsInternal();
	}
	else
	{
		UPtgUtils::PrintDebugMessage(this, TEXT("To use this action enable Generate Actors!"), EPtgDebugMessageTypes::Error, DebugMessagesTimeOnScreen);
	}
}

void APtgManager::GenerateActorsWithCustomSeed(const int32 actorsSeed)
{
	// Set random seed for every bioma actor
	for (FPtgBiomaActors& biomaActors : BiomaActors)
	{
		biomaActors.Seed = actorsSeed;
	}

	// Then, generate actors
	GenerateActors();
}

void APtgManager::GenerateActorsWithRandomSeed()
{
	GenerateActorsWithCustomSeed(FMath::RandRange(SEED_MIN, SEED_MAX));
}

void APtgManager::GenerateActorsInternal()
{
	UWorld* world = GetWorld();
	if (world == nullptr)
	{
		UPtgUtils::PrintDebugMessage(this, TEXT("world is null!!!"), EPtgDebugMessageTypes::Error, DebugMessagesTimeOnScreen);
		return;
	}

	const double startTime = FPlatformTime::Seconds();
	uint32 biomaActorsCounter = 0;
	FRandomStream randomNumberGenerator;
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	IsInsideHeigthRangeFuncPtr isInsideHeigthRangeFunction = nullptr;

	// Choose function to calculate if a triangle is inside heigh range depending on the terrain shape
	switch (Shape)
	{
	case EPtgProcMeshShapes::Cube:

		isInsideHeigthRangeFunction = isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_CUBE_HEIGHT_RANGE_FUNC_INDEX];
		break;

	case EPtgProcMeshShapes::Sphere:

		isInsideHeigthRangeFunction = isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_SPHERE_HEIGHT_RANGE_FUNC_INDEX];
		break;

	case EPtgProcMeshShapes::Plane:
	default:

		isInsideHeigthRangeFunction = isInsideHeigthRangeFunc[TRIANGLE_IS_INSIDE_PLANE_HEIGHT_RANGE_FUNC_INDEX];
	}

	// Iterate through every nature actor setting locating every random-picked mesh on its corresponding height and generating a random bunch of them, with other random stuff...
	for (FPtgBiomaActors& biomaActor : BiomaActors)
	{
		// If the actor is not especified go to the next one
		if (biomaActor.ActorClass == nullptr) continue;

		// Initialize seed for random generation
		randomNumberGenerator.Initialize(biomaActor.Seed);

		// Generate a random number of actors to spawn between a defined min. and max.
		const int32 actorsToSpawn = randomNumberGenerator.RandRange(biomaActor.MinActorsToSpawn, biomaActor.MaxActorsToSpawn);

		// Get the candidate triangles of the terrain taking into account modifiers and height percentage
		TArray<int32> triangleCandidateIndexes;
		TArray<FPtgTriangle> shuffledTerrainTriangles = terrainTriangles;
		GetTriangleCandidates
		(
			biomaActor.Modifiers,
			biomaActor.bUseLocationsOutsideModifiers,
			biomaActor.HeightPercentageRangeToLocateActors,
			biomaActor.bUseLocationsOutsideHeightRange,
			actorsToSpawn,
			isInsideHeigthRangeFunction,
			biomaActor.CorrespondingBioma,
			randomNumberGenerator,
			triangleCandidateIndexes,
			shuffledTerrainTriangles
		);

		const int32 numTriangleCandidates = triangleCandidateIndexes.Num();

		// If there are no triangle candidates on this iteration go to the next one
		if (numTriangleCandidates == 0) continue;

		GetBiomaRotationFuncPtr getBiomaElementRotationFunction = nullptr;

		switch (biomaActor.RotationType)
		{
		case EPtgNatureRotationTypes::Random:

			getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_RANDOM_ROTATION_FUNC_INDEX];
			break;

		case EPtgNatureRotationTypes::TerrainShapeNormal:

			switch (Shape)
			{
			case EPtgProcMeshShapes::Cube:

				getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_CUBE_SHAPE_ROTATION_FUNC_INDEX];
				break;

			case EPtgProcMeshShapes::Sphere:

				getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_SPHERE_SHAPE_ROTATION_FUNC_INDEX];
				break;

			case EPtgProcMeshShapes::Plane:
			default:

				getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_PLANE_SHAPE_ROTATION_FUNC_INDEX];
			}

			break;

		case EPtgNatureRotationTypes::MeshSurfaceNormal:
		default:
			getBiomaElementRotationFunction = getBiomaRotationFuncs[BIOMA_MESH_SURFACE_ROTATION_FUNC_INDEX];
		}

		// Cache other stuff to avoid inecessary access on loop time
		const int32 numActorsToSpawnLessOne = actorsToSpawn - 1;
		const int32 numTriangleCandidatesLessOne = numTriangleCandidates - 1;
		const TSubclassOf<AActor> actorClass = biomaActor.ActorClass;
		const float minScale = biomaActor.MinMaxScale.X;
		const float maxScale = biomaActor.MinMaxScale.Y;
		const int32 cullDistance = biomaActor.CullDistance;
		FTransform actorTransform;

		// Iterate through every actor to generate, giving him a new random location, scale and rotation if needed
		for (int32 newActorIndex = 0; newActorIndex < actorsToSpawn; newActorIndex++)
		{
			// If all triangle candidates has been already used, take a random one
			const int32 triangleCandidateIndex = newActorIndex <= numTriangleCandidatesLessOne
				? newActorIndex
				: randomNumberGenerator.RandRange(0, numTriangleCandidatesLessOne);

			const FPtgTriangle randomTriangle = shuffledTerrainTriangles[triangleCandidateIndexes[triangleCandidateIndex]];

			// Pick a random available location on triangle
			actorTransform.SetLocation(randomTriangle.GetRandomPointOnTriangle(randomNumberGenerator));

			// Set rotation
			actorTransform.SetRotation((this->*getBiomaElementRotationFunction)(randomNumberGenerator, randomTriangle, actorTransform.GetLocation()));

			// Generate a random mesh scale between a defined min. and max.
			const float randomScale = randomNumberGenerator.FRandRange(minScale, maxScale);
			actorTransform.SetScale3D(FVector(randomScale, randomScale, randomScale));

			// Spawn actor
			if (AActor* actor = world->SpawnActor<AActor>(actorClass, actorTransform, spawnParams))
			{
				// Attach actor to root component to have it with the correct relative transform
				actor->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);

				// For some reason, scale has to be manually set, despite of the specified actor transform on the spawn
				actor->SetActorScale3D(actorTransform.GetScale3D());

				// Get all actor's primitive components
				TArray<UPrimitiveComponent*> primitiveComponents;
				actor->GetComponents<UPrimitiveComponent>(primitiveComponents, true);

				// Set cull distance for actor
				for (UPrimitiveComponent* primitiveComponent : primitiveComponents)
				{
					if (primitiveComponent != nullptr)
					{
						primitiveComponent->LDMaxDrawDistance = cullDistance;
					}
				}

				// Save a reference to keep it under control
				biomaActorsSpawned.Emplace(actor);
				biomaActorsCounter++;
			}
		}
	}

	// Debug
	if (bShowDebugMessages && biomaActorsCounter > 0)
	{
		UPtgUtils::PrintDebugMessage(this, FString::FromInt(biomaActorsCounter) + TEXT(" bioma actors generated in ") + FString::SanitizeFloat(FPlatformTime::Seconds() - startTime) + TEXT(" seconds."), EPtgDebugMessageTypes::Info, DebugMessagesTimeOnScreen);
	}
}

void APtgManager::ClearActors()
{
	// Clean up nature meshes
	for (AActor* actor : biomaActorsSpawned)
	{
		if (actor != nullptr)
		{
			actor->Destroy();
			actor = nullptr;
		}
	}

	biomaActorsSpawned.Empty();
}

#pragma endregion

void APtgManager::GetTriangleCandidates
(
	const TArray<APtgModifier*>& modifiers,
	const bool bUseLocationsOutsideModifiers,
	const FVector2D& heightPercentageRangeToLocateElements,
	const bool bUseLocationsOutsideHeightRange,
	const int32 maxCandidatesToGet,
	const IsInsideHeigthRangeFuncPtr isInsideHeigthRangeFunction,
	const EPtgNatureBiomas bioma,
	FRandomStream& randomNumberGenerator,
	TArray<int32>& triangleCandidateIndexes,
	TArray<FPtgTriangle>& shuffledTerrainTriangles
)
{
	// Get the height range depending on the bioma current bioma 
	FVector2D heightRange;
	switch (bioma)
	{
	case EPtgNatureBiomas::Earth:

		heightRange = FVector2D(WaterHeight, HighestGeneratedHeight);
		break;

	case EPtgNatureBiomas::Underwater:

		heightRange = FVector2D(LowestGeneratedHeight, WaterHeight);
		break;

	case EPtgNatureBiomas::Both:
	default:
		heightRange = FVector2D(LowestGeneratedHeight, HighestGeneratedHeight);
	}

	// Get height range
	const FVector2D percentageRange = FVector2D(0.0f, 100.0f);
	const float lowestAllowedHeight = FMath::GetMappedRangeValueClamped(percentageRange, heightRange, heightPercentageRangeToLocateElements.X);
	const float highestAllowedHeight = FMath::GetMappedRangeValueClamped(percentageRange, heightRange, heightPercentageRangeToLocateElements.Y);

	// Cache other stuff
	const int32 numTriangles = shuffledTerrainTriangles.Num();
	const int32 lastIndex = numTriangles - 1;
	const int32 numModifiers = modifiers.Num();
	const FTransform& terrainTransform = GetTransform();

	// Height check is needed only if bioma is Earth or Underwater or percentages are different than 0-100%
	const bool bHeightCheckNeeded = bioma != EPtgNatureBiomas::Both || heightPercentageRangeToLocateElements.X != 0.0f || heightPercentageRangeToLocateElements.Y != 100.0f;

	// Iterate through every terrain triangle checking the given height and modifier rules
	for (int32 i = 0; i < numTriangles; i++)
	{
		// Shuffle current element to randomize order
		const int32 index = randomNumberGenerator.RandRange(i, lastIndex);
		if (i != index)
		{
			shuffledTerrainTriangles.Swap(i, index);
		}

		const FPtgTriangle& triangle = shuffledTerrainTriangles[i];

		// If height check is needed...
		if (bHeightCheckNeeded)
		{
			const bool bIsTriangleInsideHeigthRange = (&triangle->*isInsideHeigthRangeFunction)(lowestAllowedHeight, highestAllowedHeight, terrainTransform);

			// Refuse candidate if:
			// - We only have to use locations outside height range AND triangle is inside the range OR Insie earth or water heigh ranges (needed to avoid locating things in the other bioma)
			// OR
			// - We only have to use locations inside height range AND triangle is outside the range
			if ((bUseLocationsOutsideHeightRange && 
				(
					bIsTriangleInsideHeigthRange || 
					(bioma == EPtgNatureBiomas::Earth && (&triangle->*isInsideHeigthRangeFunction)(LowestGeneratedHeight, WaterHeight, terrainTransform)) ||
					(bioma == EPtgNatureBiomas::Underwater && (&triangle->*isInsideHeigthRangeFunction)(WaterHeight, HighestGeneratedHeight, terrainTransform))
				)) ||
				(!bUseLocationsOutsideHeightRange && !bIsTriangleInsideHeigthRange))
			{
				continue;
			}
		}

		bool bRefuseCandidate = false;
		
		// Check modifiers
		if (numModifiers > 0)
		{
			bRefuseCandidate = !bUseLocationsOutsideModifiers;

			const FPtgTriangle& worldSpaceTriangle = triangle.GetWorldSpaceTriangle(terrainTransform);

			for (const APtgModifier* modifier : modifiers)
			{
				// Refuse candidate if:
				// - Triangle is inside modifier AND we only have to use locations outside modifiers
				// OR
				// - Triangle is outside every modifier AND we only have to use locations inside some modifier
				if (modifier != nullptr && worldSpaceTriangle.IsInsideShape(modifier->GetShapeComponent()))
				{
					bRefuseCandidate = bUseLocationsOutsideModifiers;
					break;
				}
			}
		}

		if (!bRefuseCandidate)
		{
			// Add triangle index as a valid candidate
			triangleCandidateIndexes.Add(i);

			// If we have enough candidates, exit
			if (triangleCandidateIndexes.Num() == maxCandidatesToGet)
			{
				break;
			}
		}
	}
}

const TArray<float>& APtgManager::GetVertexHeightData() const
{
	if (procMeshData.GetNumVertices() == 0)
	{
		UPtgUtils::PrintDebugMessage(this, TEXT("No vertex data found! please, generate terrain before creating heightmap."), EPtgDebugMessageTypes::Error, DebugMessagesTimeOnScreen);
	}

	return vertexHeightData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region EDITOR

#if WITH_EDITOR
void APtgManager::PreEditChange(FProperty* PropertyThatWillChange)
{
	// Sometimes, when modifying actor properties, the RMC unregisters losing its runtime mesh. Here we save both to restore them later, in PostEditChangeProperty 
	terrainRuntimeMesh = (IsValid(ProcMeshTerrainComp)) ? ProcMeshTerrainComp->GetRuntimeMesh() : nullptr;
	waterRuntimeMesh = (IsValid(ProcMeshWaterComp)) ? ProcMeshWaterComp->GetRuntimeMesh() : nullptr;

	Super::PreEditChange(PropertyThatWillChange);
}

void APtgManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Restore runtime mesh for terrain RMC
	if (IsValid(ProcMeshTerrainComp) && IsValid(terrainRuntimeMesh))
	{
		ProcMeshTerrainComp->SetRuntimeMesh(terrainRuntimeMesh);
		terrainRuntimeMesh = nullptr;
	}

	// Restore runtime mesh for water RMC
	if (IsValid(ProcMeshWaterComp) && IsValid(waterRuntimeMesh))
	{
		ProcMeshWaterComp->SetRuntimeMesh(waterRuntimeMesh);
		waterRuntimeMesh = nullptr;
	}

	const FName changedPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, Radius) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, Resolution) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, NumberOfLODs) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, LODScreenSizeMultiplier) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, NoiseInputScale) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, NoiseOutputScale))
	{
		if (bGenerateEverythingOnPropertyChange)
		{
			GenerateEverything();
		}
	}
	else if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, Shape))
	{
		// Force ProceduralMesh on Sphere and Cube shapes
		switch (Shape)
		{
		case EPtgProcMeshShapes::Cube:
		case EPtgProcMeshShapes::Sphere:

			bUseTerrainTiling = false;

			if (WaterHeightGenerationType == EPtgWaterHeightGenerationType::FixedHeight)
			{
				WaterHeightGenerationType = EPtgWaterHeightGenerationType::RandomPercentage;

				UPtgUtils::PrintDebugMessage(this, TEXT("Water Height Generation Type changed automatically to Random Percentage."), EPtgDebugMessageTypes::Info, DebugMessagesTimeOnScreen);
			}

			break;

		case EPtgProcMeshShapes::Plane:
		default:
			break;
		}

		if (bGenerateEverythingOnPropertyChange)
		{
			GenerateEverything();
		}
	}
	// On terrain material changed
	else if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, TerrainMaterial))
	{
		URuntimeMeshProvider* provider = ProcMeshTerrainComp->GetProvider();

		if (provider != nullptr)
		{
			if (TerrainMaterial == nullptr)
			{
				provider->SetupMaterialSlot(0, TEXT("TerrainMaterial"), nullptr);
			}
			else
			{
				UMaterial* material = TerrainMaterial->GetMaterial();
				if (material != nullptr)
				{
					provider->SetupMaterialSlot(0, TEXT("TerrainMaterial"), TerrainMaterial);
				}
			}
		}
	}
	else if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, bGenerateWater))
	{
		if (bGenerateEverythingOnPropertyChange)
		{
			GenerateEverything();
		}
		else if (!bGenerateWater)
		{
			ClearWaterMesh();
		}
	}
	// On any water height property changed
	else if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, WaterHeightGenerationType) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, WaterSeed) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, WaterRandomHeightRangePercentages) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, WaterFixedHeightPercentage) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, WaterFixedHeightValue))
	{
		if (WaterHeightGenerationType == EPtgWaterHeightGenerationType::FixedHeight && Shape != EPtgProcMeshShapes::Plane)
		{
			WaterHeightGenerationType = EPtgWaterHeightGenerationType::RandomPercentage;
			UPtgUtils::PrintDebugMessage(this, TEXT("Fixed Height is only available for plane terrains!"), EPtgDebugMessageTypes::Warning, DebugMessagesTimeOnScreen);
		}

		// Clamp percentages between 0 and 100
		WaterRandomHeightRangePercentages = WaterRandomHeightRangePercentages.ClampAxes(0.0f, 100.0f);

		if (bGenerateEverythingOnPropertyChange)
		{
			GenerateEverything();
		}
	}
	// On water material changed
	else if ((changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, WaterMaterial)))
	{
		URuntimeMeshProvider* provider = ProcMeshWaterComp->GetProvider();

		if (provider != nullptr)
		{
			if (WaterMaterial == nullptr)
			{
				provider->SetupMaterialSlot(0, TEXT("WaterMaterial"), nullptr);
			}
			else
			{
				UMaterial* material = WaterMaterial->GetMaterial();
				if (material != nullptr)
				{
					provider->SetupMaterialSlot(0, TEXT("WaterMaterial"), WaterMaterial);
				}
			}
		}
	}
	else if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, bGenerateNature))
	{
		if (bGenerateNature && bGenerateEverythingOnPropertyChange)
		{
			GenerateNature();
		}
		else if (!bGenerateNature)
		{
			ClearNature();
		}
	}
	else if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, bGenerateActors))
	{
		if (bGenerateActors && bGenerateEverythingOnPropertyChange)
		{
			GenerateActors();
		}
		else if (!bGenerateActors)
		{
			ClearActors();
		}
	}
	else if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, BiomaNature))
	{
		for (FPtgBiomaNature& biomaNature : BiomaNature)
		{
			// Clamp some values to something logical
			if (biomaNature.MinMeshesToSpawn < 1) biomaNature.MinMeshesToSpawn = 1;
			if (biomaNature.MinMeshesToSpawn > biomaNature.MaxMeshesToSpawn) biomaNature.MaxMeshesToSpawn = biomaNature.MinMeshesToSpawn;
			if (biomaNature.MinMaxScale.X > biomaNature.MinMaxScale.Y) biomaNature.MinMaxScale.Y = biomaNature.MinMaxScale.X;

			const int32 cullDistance = biomaNature.CullDistance;

			// Find every static mesh on map to see if we should change cull distance on HISM
			for (const UStaticMesh* mesh : biomaNature.Meshes)
			{
				if (mesh != nullptr)
				{
					if (UHierarchicalInstancedStaticMeshComponent** HISM_ref = NatureStaticMeshHISM_Correspondence.Find(mesh))
					{
						UHierarchicalInstancedStaticMeshComponent* HISM = *HISM_ref;

						// Change cull distance only if its different than the current one
						if (HISM != nullptr && HISM->InstanceEndCullDistance != cullDistance)
						{
							HISM->SetCullDistances(0, cullDistance);
						}
					}
				}
			}
		}
	}
	else if (changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, BiomaActors))
	{
		for (FPtgBiomaActors& biomaActor : BiomaActors)
		{
			// Clamp some values to something logical
			if (biomaActor.MinActorsToSpawn < 1) biomaActor.MinActorsToSpawn = 1;
			if (biomaActor.MinActorsToSpawn > biomaActor.MaxActorsToSpawn) biomaActor.MaxActorsToSpawn = biomaActor.MinActorsToSpawn;

			const int32 cullDistance = biomaActor.CullDistance;
			const TSubclassOf<AActor> actorClass = biomaActor.ActorClass;

			// Set cull distance for bioma actors
			for (const AActor* actor : biomaActorsSpawned)
			{
				if (actor != nullptr && actor->GetClass() == actorClass)
				{
					// Get all actor's primitive components
					TArray<UPrimitiveComponent*> primitiveComponents;
					actor->GetComponents<UPrimitiveComponent>(primitiveComponents, true);

					// Set new distance for every primitive component in actor
					for (UPrimitiveComponent* primitiveComponent : primitiveComponents)
					{
						if (primitiveComponent != nullptr)
						{
							primitiveComponent->SetCullDistance(cullDistance);
						}
					}
				}
			}
		}
	}
	// On any Fast Noise Lite property changed
	else if ((changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, NoiseType)) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, RotationType3D) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, Seed) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, Frequency) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, FractalType) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, FractalOctaves) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, FractalLacunarity) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, FractalGain) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, FractalWeightedStrength) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, FractalWeightedStrength) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, CellularJitter) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, CellularDistanceFunction) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, CellularReturnType) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, DomainWarpType) ||
		changedPropertyName == GET_MEMBER_NAME_CHECKED(APtgManager, DomainWarpAmplitude))
	{
		SetupFastNoiseLite();

		if (bGenerateEverythingOnPropertyChange)
		{
			// Regenerate terrain with new noise settings
			GenerateEverything();
		}
	}
}
#endif

#pragma endregion
