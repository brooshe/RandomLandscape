// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "WorldScapeRoot.h"



DEFINE_LOG_CATEGORY(LogWorldScapeCore);

static int32 MaxSafeRecursion = 8;
static int32 MaxSafeLoD = 99;


AWorldScapeRoot::AWorldScapeRoot() {


	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;

	PrimaryActorTick.EndTickGroup = ETickingGroup::TG_PrePhysics;

	//PrimaryActorTick.bTickEvenWhenPaused = true;

	//Setting Default Data : 
	NoiseIntensity = 1200000;
	PlanetScaleCode = 637817792;
	PlanetScale = 637817792;
	DistanceToFreezeGeneration = PlanetScale * 10;
	NoiseScale = 800;
	Seed = 10;

	

	//RootComponent->Bounds = FBoxSphereBounds(FVector(0), FVector(637817792 * 0.5f), 637817792 * 0.5f);

	MaxLod = 8;
	OceanMaxLod = 8;

	HeightAnchor = 10000;

	bGenerateFoliages = true;

	bFreezeGeneration = false;

	

	bEnableVolumes = true;

	LodResolution = 128;
	TriangleSize = 100;

	OceanLodResolution = 32;
	OceanTriangleSize = 400;

	bGenerateCollision = true;
	CollisionResolution = 16;
	CollisionTriangleSize = 200;

	TickPerSecond = 60;

	GridAngleMin = 5;
	GridAngleMax = 5;

	FCoreDelegates::PreWorldOriginOffset.AddUFunction(this, "OnBeginRebase");
	FCoreDelegates::PostWorldOriginOffset.AddUFunction(this, "OnFinishedRebase");


	PlanetNoise = CustomNoise(Seed);
	WorldScapeNoise = CreateDefaultSubobject<UWorldScapeCustomNoise>(TEXT("Default Noise Generator"));
	TransformKeeper = CreateDefaultSubobject<UWorldScapeLod>(TEXT("Transform_Keeper"));
	TransformKeeper->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	TransformKeeper->SetMobility(EComponentMobility::Movable);
	TransformKeeper->Mesh->SetMobility(EComponentMobility::Movable);
	PlayerWorldPos = DVector(0, 0, 0);
	PlayerPos = FVector(0, 0, 0);
	init = false;
	SetActorTickInterval(1.0f / TickPerSecond);
	//TransformKeeper->Mesh->DestroyComponent();
}

bool AWorldScapeRoot::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AWorldScapeRoot::OnBeginRebase() 
{
	RebaseInProgress = true;

	UE_LOG(LogWorldScapeCore, Log, TEXT("Begin Rebase"));
}

void AWorldScapeRoot::OnFinishedRebase() 
{
	RebaseInProgress = false;

	UE_LOG(LogWorldScapeCore, Log, TEXT("Rebase Fininished"));
}

void AWorldScapeRoot::PostLoad() 
{
	Super::PostLoad();
	CleanComponents();
}

//Called on PreSave
void AWorldScapeRoot::ClearCrossLevelReferences() 
{
	Super::PostLoad();
	CleanComponents();
}

void AWorldScapeRoot::OnConstruction(const FTransform& Transform)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
		return;
	
	if (!WorldScapeNoise)
		WorldScapeNoise = NewObject<UWorldScapeCustomNoise>(this);


	if (GenerationType == EWorldScapeType::Flat) {
		PlanetScaleCode = 637817792 * 0.5f;
		bFlatWorld = true;
	}
	else {
		PlanetScaleCode = PlanetScale;
		bFlatWorld = false;
	}

	//UE_LOG(LogWorldScapeCore, Log, TEXT("Construction Script"));
	RootComponent->SetMobility(EComponentMobility::Movable);
#if WITH_EDITOR
	if (WorldScapeEditorUtils::IsInViewPort() && bStaticCollisionInEditor) 
	{
		RootComponent->SetMobility(EComponentMobility::Static);
	}
#endif

	if (!TransformKeeper) 
	{
		TransformKeeper = NewObject<UWorldScapeLod>(this);
		TransformKeeper->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		TransformKeeper->SetMobility(EComponentMobility::Movable);
	}

	if (TickPerSecond <= 0) TickPerSecond = 1;

	if (TriangleSize < 0.1) TriangleSize = 0.1f;

	if (HeightAnchor < 1) HeightAnchor = 1;

	if (LodResolution < 8) LodResolution = 8;

	if (GridAngleMin <= 0) GridAngleMin = 0.0001;
	if (GridAngleMax < GridAngleMin) GridAngleMax = GridAngleMin;
	if (GridAngleMax > 90) GridAngleMax = 90;



	if (OceanTriangleSize < 0.1) OceanTriangleSize = 0.1f;


	if (OceanLodResolution < 8) OceanLodResolution = 8;

	if (CollisionResolution < 8) CollisionResolution = 8;



	LodResolution = FMath::RoundToInt(((float)LodResolution / 4.f)) * 4;
	OceanLodResolution = FMath::RoundToInt(((float)OceanLodResolution / 4.f)) * 4;


	if (GenerationType_Previous != GenerationType) 
	{
		GenerationType_Previous = GenerationType;
		CleanComponents();
	}
		


	//Add Parameter for this
	SetActorTickInterval(1.0f/TickPerSecond);

	if (MaxLod < 1)
		MaxLod = 1;

	if (OceanMaxLod < 1)
		OceanMaxLod = 1;

	if (bGenerateWorldScape)
	{
		if (WorldScapeLod.Num() == 0 || CheckForRegenerate())
		{
			GenerateBaseMesh();
		}

	}
	else 
	{
		CleanComponents();
	}
}

void AWorldScapeRoot::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	if (Prev_bDisplayCollision != bDisplayCollision) 
	{
		Prev_bDisplayCollision = bDisplayCollision;
		for (int32 i = 0; i < CollisionLods.Num(); i++) 
		{
			CollisionLods[i]->Mesh->SetVisibility(bDisplayCollision);
		}

		for (int32 i = 0; i < WorldScapeLod.Num(); i++) 
		{
			WorldScapeLod[i]->Mesh->SetVisibility(!bDisplayCollision);
		}

		if (bOcean) {
			for (int32 i = 0; i < WorldScapeLodOcean.Num(); i++) 
			{
				WorldScapeLodOcean[i]->Mesh->SetVisibility(!bDisplayCollision);
			}
		}

		
	}



	PlanetLocation = GetActorLocation();

	bool InnerFreeze = false;
	FVector Normal = FVector(0, 0, 1);
	if (GenerationType == EWorldScapeType::Planet)
	{
		double Distance = 0;

		if (IsValid(GetWorld()) && IsValid(GetWorld()->GetFirstPlayerController()) && IsValid(GetWorld()->GetFirstPlayerController()->GetPawn())) 
		{
			Normal = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() - GetActorLocation();
			Normal.Normalize();
			Distance = DVector::Dist(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), GetActorLocation());
		}
		
#if WITH_EDITOR
		if (WorldScapeEditorUtils::IsInViewPort()) 
		{
			Normal = WorldScapeEditorUtils::GetViewPortCameraPosition() - GetActorLocation();
			Normal.Normalize();
			Distance = DVector::Dist(WorldScapeEditorUtils::GetViewPortCameraPosition(), GetActorLocation());
		}
#endif

	
		Distance -= PlanetScale;
		
		

		if (WorldScapeLod.Num() > 0 && WorldScapeLodInGeneration.Num() == 0&&  DistanceToFreezeGeneration > 0 && Distance > DistanceToFreezeGeneration) 
		{
			InnerFreeze = true;

			if ( FVector::DotProduct(Normal, LastGenerationNormal) < 0.8f) InnerFreeze = false;
		}
	}
	

	if (!(bFreezeGeneration|| InnerFreeze) && bGenerateWorldScape) 
	{
		if (WorldScapeLodInGeneration.Num() == 0)LastGenerationNormal = Normal;

		if (!init){
			GenerateBaseMesh();
			init = true;
		}

		if (WorldScapeLod.Num() == 0 || CheckForRegenerate()) 
		{
			GenerateBaseMesh();
		}
			

		

		UpdatePosition();

		if (bGenerateFoliages)
		{
			if (!RebaseInProgress) {
				FoliageHandleTick();

			}
		}
		else 
		{
			for (int32 i = 0; i < FoliageDataList.Num(); i++) 
			{
				FoliageDataList[i].ActiveFoliageSector.Empty();
			}
		}

		CheckForLodGeneration();
	}
	else if (!bGenerateWorldScape && WorldScapeLod.Num() > 0)
	{
		CleanComponents();
	}

	CheckForHeightmapModifier();
}

void AWorldScapeRoot::CleanComponents() {

	if (GenerationType == EWorldScapeType::Flat) 
	{
		PlanetScaleCode = 637817792 * 0.5;
		bFlatWorld = true;
	}
	else 
	{
		PlanetScaleCode = PlanetScale;
		bFlatWorld = false;
	}

	FoliageGenerationDone = false;
	FoliageGenerationInProgress = false;
	//UE_LOG(LogWorldScapeCore, Log, TEXT("Clean Components"));
	FoliageDataList.Empty();

	if (WorldScapeLod.Num() > 0)
	{

		for (int32 i = 0; i < WorldScapeLod.Num(); i++) 
		{
			if (IsValid(WorldScapeLod[i])) 
			{
				if (IsValid(WorldScapeLod[i]->Mesh))
				{
					WorldScapeLod[i]->Mesh->DestroyComponent();
				}
				WorldScapeLod[i]->DestroyComponent();
			}
		}
	}

	if (WorldScapeLodOcean.Num() > 0) 
	{

		for (int32 i = 0; i < WorldScapeLodOcean.Num(); i++) 
		{
			if (IsValid(WorldScapeLodOcean[i])) 
			{
				if (IsValid(WorldScapeLodOcean[i]->Mesh))
				{ 
					WorldScapeLodOcean[i]->Mesh->DestroyComponent();
				}
				WorldScapeLodOcean[i]->DestroyComponent();
			}
		}
	}

	if (CollisionLods.Num() > 0) 
	{
		for (int32 i = 0; i < CollisionLods.Num(); i++) {
			if (IsValid(CollisionLods[i])) 
			{
				if (IsValid(CollisionLods[i]->Mesh))
				{
					CollisionLods[i]->Mesh->DestroyComponent();
				}
				CollisionLods[i]->DestroyComponent();
			}
		}
	}

	for (int32 i = 0; i < InstancedMeshs.Num(); i++) 
	{
		if (InstancedMeshs[i]) 
		{
			this->RemoveInstanceComponent(InstancedMeshs[i]);
			InstancedMeshs[i]->DestroyComponent();
		}
	}
	TArray<UActorComponent*> Components;
	GetComponents(UWorldScapeLod::StaticClass(), Components, true);

	for (UActorComponent* PC : Components)
	{
		if (PC != TransformKeeper)
		{
			PC->DestroyComponent();
		}
	}
	GetComponents(UHierarchicalInstancedStaticMeshComponent::StaticClass(), Components,true);

	for (UActorComponent* HISMC : Components)
	{
		HISMC->DestroyComponent();
	}

	TArray<USceneComponent*> ActorsList;
	
	ActorsList = TransformKeeper->GetAttachChildren();
	for (USceneComponent* WSMC : ActorsList)
	{
		if (IsValid(WSMC) && WSMC->GetClass() == (UWorldScapeMeshComponent::StaticClass()))
		{
			WSMC->DestroyComponent();
		}

	}


	WorldScapeLodOcean.Empty();
	WorldScapeLod.Empty();
	WorldScapeLodInGeneration.Empty();
	CollisionLods.Empty();

	InstancedMeshs.Empty();
}

void AWorldScapeRoot::GenerateBaseMesh() 
{
	Prev_HeightMult = -1;
	if (MaxLod < 1)
		MaxLod = 1;

	PlanetNoise.SetSeed(Seed);
	CleanComponents();

	//UE_LOG(LogWorldScapeCore, Log, TEXT("Generate Base Mesh"));

	FoliagesList.Empty();
	DVector PPos = FVector(0,0,0);

	// if (IsValid(GetWorld()) && IsValid(GetWorld()->GetFirstPlayerController()) && IsValid(GetWorld()->GetFirstPlayerController()->GetPawn()))
	// {
	// 	PPos = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	// }

#if WITH_EDITOR
	if (WorldScapeEditorUtils::IsInViewPort()) 
	{
		// PPos = WorldScapeEditorUtils::GetViewPortCameraPosition();
	}
#endif
	double AltitudeMultiplier = pow(2, round(log2(PlayerDistanceToGround)));

	/*
	UE_LOG(LogWorldScapeCore, Log, TEXT("Altitude : %f Lods"), (float)PlayerDistanceToGround);
	UE_LOG(LogWorldScapeCore, Log, TEXT("log 2 altitude : %f Lods"), (float)log2(PlayerDistanceToGround));
	UE_LOG(LogWorldScapeCore, Log, TEXT("rounded log 2 altitude : %f Lods"), (float)round(log2(PlayerDistanceToGround)));
	UE_LOG(LogWorldScapeCore, Log, TEXT("Altitude Multiplier : %f Lods"), (float)AltitudeMultiplier);
	*/
	float Size = (float)(FMath::Max( (double)HeightAnchor, AltitudeMultiplier) * (double)TriangleSize * (1./ (double)HeightAnchor));

	PPos = PPos + PlanetLocation;
	PPos.Normalize();

	if (IsValid(GetWorld()) )
	{
		for (int32 i = 0; i < MaxLod; i++) 
		{
			WorldScapeLod.Add(NewObject <UWorldScapeLod>(this));
			WorldScapeLod[i]->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
			WorldScapeLod[i]->SetRelativeLocation(FVector(0, 0, 0));
			WorldScapeLod[i]->RegisterComponentWithWorld(GetWorld());
			WorldScapeLod[i]->Init(i, LodResolution, TriangleSize * (2 ^ i), (PPos*PlanetScaleCode - PlanetLocation), TerrainMaterial, PlanetScaleCode, PlanetLocation, false, false);
		}
		for (int32 i = 0; i < OceanMaxLod; i++) 
		{
			if (bOcean)
			{
				WorldScapeLodOcean.Add(NewObject <UWorldScapeLod>(this));
				WorldScapeLodOcean[i]->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
				WorldScapeLodOcean[i]->SetRelativeLocation(FVector(0, 0, 0));
				WorldScapeLodOcean[i]->RegisterComponentWithWorld(GetWorld());
				WorldScapeLodOcean[i]->Init(i, OceanLodResolution, OceanTriangleSize * (2 ^ i), (PPos * PlanetScaleCode - PlanetLocation), OceanMaterial, PlanetScaleCode, PlanetLocation, true, false);
			}
		}
		for (int32 c = 0; c < Foliages.Num(); c++) 
		{
			if (IsValid(Foliages[c]))
			{
				for (int32 i = 0; i < Foliages[c]->FoliageList.Num(); i++) 
				{
					FVegetation VegData = FVegetation();
					VegData.FoliageCollectionIndex = c;
					VegData.FoliageIndex = i;
					FoliagesList.Add(Foliages[c]->FoliageList[i]);
					FoliageDataList.Add(VegData);
				}
			}
		}
		
	}
}

void AWorldScapeRoot::CreateInstancedMesh(FFoliageSector* Sector, UWorldScapeFoliage* Foliage) {
	if (Foliage && Foliage->StaticMesh) 
	{
		//UE_LOG(LogWorldScapeCore, Log, TEXT("Add Foliage"))

		Sector->InstancedMesh = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		Sector->InstancedMesh->RegisterComponentWithWorld(GetWorld());
		Sector->InstancedMesh->SetStaticMesh(Foliage->StaticMesh);
		Sector->InstancedMesh->SetFlags(RF_Transactional);
		auto mobility = EComponentMobility::Movable;
#if WITH_EDITOR
		if (WorldScapeEditorUtils::IsInViewPort() && bStaticCollisionInEditor) 
		{
			mobility = EComponentMobility::Static;
		}
#endif
		Sector->InstancedMesh->SetMobility(mobility);
		Sector->InstancedMesh->SetReceivesDecals(Foliage->bReceiveDecal);
		Sector->InstancedMesh->SetCastShadow(Foliage->bCastShadows);
		Sector->InstancedMesh->bCastStaticShadow = Foliage->bCastShadows;
		Sector->InstancedMesh->bDisableCollision = !Foliage->bCollision;
		Sector->InstancedMesh->SetCullDistances(0, Foliage->CullingDistance);
		if (Foliage->bCollision)
		{ 
			Sector->InstancedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			Sector->InstancedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		Sector->InstancedMesh->SetSimulatePhysics(Foliage->bCollision);
		Sector->InstancedMesh->bEnableDensityScaling = Foliage->bCollision ? 0 : 1;
		//Sector->InstancedMesh->OverrideMaterials
		this->AddInstanceComponent(Sector->InstancedMesh);
	}
}

bool AWorldScapeRoot::CheckForRegenerate(bool update)
{
	bool Regenerate = false;

	if (Prev_PlanetScale != PlanetScaleCode)			Regenerate = true;
	if (Prev_WorldScapeNoise != WorldScapeNoise)		Regenerate = true;
	if (Prev_NoiseOffset != NoiseOffset)				Regenerate = true;
	if (Prev_MaxLoD != MaxLod)							Regenerate = true;
	if (Prev_LodResolution != LodResolution)			Regenerate = true;
	if (Prev_TriangleSize != TriangleSize)				Regenerate = true;
	if (Prev_HeightAnchor != HeightAnchor)				Regenerate = true;
	if (Prev_Ocean != bOcean)							Regenerate = true;
	if (Prev_OceanMaxLoD != OceanMaxLod)				Regenerate = true; 
	if (Prev_OceanLodResolution != OceanLodResolution)	Regenerate = true;
	if (Prev_OceanTriangleSize != OceanTriangleSize)	Regenerate = true;
	if (Prev_NoiseIntensity != NoiseIntensity)			Regenerate = true;
	if (Prev_NoiseScale != NoiseScale)					Regenerate = true;
	if (Prev_Seed != Seed)								Regenerate = true;
	if (Prev_LattitudeRotation != LattitudeRotation)	Regenerate = true;
	if (Prev_OceanHeight != OceanHeight)				Regenerate = true;
	if (Prev_Foliages != Foliages)						Regenerate = true;
	
	if (update)
	{
		Prev_NoiseOffset = NoiseOffset;
		Prev_LodResolution = LodResolution;
		Prev_MaxLoD = MaxLod;
		Prev_PlanetScale = PlanetScaleCode;
		Prev_NoiseIntensity = NoiseIntensity;
		Prev_NoiseScale = NoiseScale;
		Prev_Seed = Seed;
		Prev_PlanetTransform = GetTransform();
		Prev_LattitudeRotation = LattitudeRotation;
		Prev_TriangleSize = TriangleSize;
		Prev_Ocean = bOcean;
		Prev_HeightAnchor = HeightAnchor;
		Prev_OceanMaxLoD = OceanMaxLod;
		Prev_OceanLodResolution = OceanLodResolution;
		Prev_OceanTriangleSize = OceanTriangleSize;
		Prev_OceanHeight = OceanHeight;
		Prev_Foliages = Foliages;
		Prev_WorldScapeNoise = WorldScapeNoise;
	}


	for (int32 i = 0; i < FoliagesList.Num(); i++) 
	{
		if (FoliagesList[i] && FoliagesList[i]->StaticMesh && InstancedMeshs.IsValidIndex(i) &&
			InstancedMeshs[i]->GetStaticMesh() &&
			FoliagesList[i]->StaticMesh != InstancedMeshs[i]->GetStaticMesh()
			)
		{
			Regenerate = true;
		}
	}
	return Regenerate;
}

void AWorldScapeRoot::CheckForHeightmapModifier()
{
	if (!bEnableVolumes)
		return;

	bool Regen = false;
	for (int32 i = 0; i < HeightMapVolumeList.Num();i++)
	{
		if (HeightMapVolumeList.IsValidIndex(i) && IsValid(HeightMapVolumeList[i])) 
		{
			if (HeightMapVolumeList[i]->NeedRefresh)
			{
				HeightMapVolumeList[i]->NeedRefresh = false;
				Regen = true;
			}
			if (HeightMapVolumeList[i]->AlignToPlanetSurface && !bFlatWorld)
			{
				FVector SurfaceNormal = GetPawnNormal(HeightMapVolumeList[i]->GetActorLocation());
				HeightMapVolumeList[i]->SetActorRotation(UKismetMathLibrary::MakeRotFromZ(SurfaceNormal).Quaternion()* HeightMapVolumeList[i]->SurfaceAlignmentRotationOffset.Quaternion());
				HeightMapVolumeList[i]->SetActorLocation((DVector((GetGroundHeight(HeightMapVolumeList[i]->GetActorLocation()) + PlanetScaleCode) * SurfaceNormal) + PlanetLocation).ToFVector());
			}
		}
	}

	for (int32 i = 0; i < NoiseVolumeList.Num(); i++)
	{
		if (NoiseVolumeList.IsValidIndex(i) && IsValid(NoiseVolumeList[i])) 
		{
			if (NoiseVolumeList[i]->NeedRefresh) {
				NoiseVolumeList[i]->NeedRefresh = false;
				Regen = true;
			}
			if (NoiseVolumeList[i]->AlignToPlanetSurface && !bFlatWorld) {
				FVector SurfaceNormal = GetPawnNormal(NoiseVolumeList[i]->GetActorLocation());
				NoiseVolumeList[i]->SetActorRotation(UKismetMathLibrary::MakeRotFromZ(SurfaceNormal).Quaternion()* NoiseVolumeList[i]->SurfaceAlignmentRotationOffset.Quaternion());
				NoiseVolumeList[i]->SetActorLocation((DVector((GetGroundHeight(NoiseVolumeList[i]->GetActorLocation()) + PlanetScaleCode) * SurfaceNormal) + PlanetLocation).ToFVector());
			}
		}
	}

	for (int32 i = 0; i < FoliageMaskList.Num(); i++)
	{
		if (FoliageMaskList.IsValidIndex(i) && IsValid(FoliageMaskList[i])) 
		{
			if (FoliageMaskList[i]->AlignToPlanetSurface && !bFlatWorld) {
				FVector SurfaceNormal = GetPawnNormal(FoliageMaskList[i]->GetActorLocation());
				FoliageMaskList[i]->SetActorRotation(UKismetMathLibrary::MakeRotFromZ(SurfaceNormal).Quaternion() * FoliageMaskList[i]->SurfaceAlignmentRotationOffset.Quaternion());
				FoliageMaskList[i]->SetActorLocation((DVector((GetGroundHeight(FoliageMaskList[i]->GetActorLocation()) + PlanetScaleCode) * SurfaceNormal) + PlanetLocation).ToFVector());
			}
		}
	}

	if (IsValid(WorldScapeNoise) && WorldScapeNoise->bNeedPlanetRefresh) 
	{
		WorldScapeNoise->bNeedPlanetRefresh = false;
		Regen = true;
	}

	
	if (Regen) HMIForceUpdate = true;
}

void AWorldScapeRoot::UpdatePosition()
{
	DVector PlayerPositionNormal;

	bool DoColision = bGenerateCollision;

	if (IsValid(GetWorld()) && IsValid(GetWorld()->GetFirstPlayerController()) && IsValid(GetWorld()->GetFirstPlayerController()->GetPawn()))
	{
		PlayerPos = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	}
#if WITH_EDITOR
	if (WorldScapeEditorUtils::IsInViewPort()) 
	{
		PlayerPos = WorldScapeEditorUtils::GetViewPortCameraPosition();
		DoColision = bGenerateCollisionInEditor;
	}
#endif
	PlayerWorldPos = PlayerPos;
	PlayerPos = WorldToECEF(PlayerPos).ToFVector();

	FNoiseData Data;
	bool NormalChanged = false;


	PlayerDistanceToGround = GetPawnDistanceFromGround(PlayerWorldPos.ToFVector());
	PlayerAltitude = GetPawnAltitude(PlayerWorldPos.ToFVector());
	

	PlayerPositionNormal = WorldScapeWorld::GetUpVector(PlayerPos, bFlatWorld);
	int32 AltitudeMultiplier = pow(2, round(FMath::Max(0.f, log2(FMath::Max(PlayerDistanceToGround, 0.f) / HeightAnchor))));

#if WITH_EDITOR
	if (bProjectPosition) 
	{
		FVector ProjectedPosition;

		if (WorldScapeEditorUtils::IsInViewPort() && AltitudeMultiplier >= 8) 
		{
			double ViewOffset = PlayerDistanceToGround * 1.732;
			//prevent position to go to the other side of the planet
			if (ViewOffset >= PlanetScale * 0.5)
				ViewOffset = PlanetScale * 0.5;
			PlayerWorldPos = WorldScapeEditorUtils::GetViewPortCameraLookAt(ViewOffset);
			PlayerPos = WorldToECEF(PlayerWorldPos).ToFVector();
			PlayerPositionNormal = WorldScapeWorld::GetUpVector(PlayerPos, bFlatWorld);
		}
	}
#endif
	
	if (bOverridePlayerPosition) 
	{
		PlayerWorldPos = OverridedPlayerPosition;
		PlayerPos = WorldToECEF(OverridedPlayerPosition).ToFVector();
		PlayerDistanceToGround = GetPawnDistanceFromGround(PlayerWorldPos.ToFVector());
		PlayerAltitude = GetPawnAltitude(PlayerWorldPos.ToFVector());
		PlayerPositionNormal = WorldScapeWorld::GetUpVector(PlayerPos, bFlatWorld);
		AltitudeMultiplier = pow(2, round(FMath::Max(0.f, log2(FMath::Max(PlayerDistanceToGround, 0.f) / HeightAnchor))));
	}

	GridAngle = GridAngleMin * AltitudeMultiplier;

	if (GridAngle > GridAngleMax) GridAngle = GridAngleMax;

	//TODO : snap Grid Angle to a divisor of 360

	DVector PPos = ECEFToProjectedPos(PlayerPos);

	if (!bFlatWorld)
	{
		DVector SnappedNormal = WorldScapeWorld::GetUpVectorSnapped(PlayerPos, GridAngle, bFlatWorld);

		if (SnappedNormal != Prev_Normal)
		{
			NormalChanged = true;
			Prev_Normal = SnappedNormal;
		}
	}

	

	if (DoColision)
		CollisionLodHandler(PlayerPositionNormal,AltitudeMultiplier, NormalChanged, Data.Height);
	else if (CollisionLods.Num()> 0) 
	{
		for (int32 i = 0; i < CollisionLods.Num(); i++) 
		{
			CollisionLods[i]->Mesh->DestroyComponent();
			CollisionLods[i]->DestroyComponent();
		}
		CollisionLods.Empty();
	}

	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		if (WorldScapeLodInGeneration.Num() == 0) 
		{
			bool IsInViewPort = false;
#if WITH_EDITOR
			if (WorldScapeEditorUtils::IsInViewPort()) 
			{
				IsInViewPort = true;
			}
#endif


			DVector SnappedPosition, SnappedPositionPO;
			FVector2D SubPosition;

			if (DebugObject) DebugObject->SetActorLocation(ECEFToWorld(PPos).ToFVector());

			double StepSize;
			bool ForceUpdate = false;
			if (Prev_HeightMult != AltitudeMultiplier) 
			{
				Prev_HeightMult = AltitudeMultiplier;
				ForceUpdate = true;
			}
			for (int32 i = 0; i < WorldScapeLod.Num(); i++) 
			{
				StepSize = (double)TriangleSize * pow(2, i) * AltitudeMultiplier;
				WorldScapeWorld::GetSnappedPosition(SnappedPosition, PlayerPositionNormal, SubPosition, NormalChanged, StepSize, ForceUpdate, PPos, 
					PlanetLocation, i, AltitudeMultiplier, TriangleSize, PlanetScaleCode, GridAngle, bFlatWorld, false);
				
				
				if (DVector::Dist(WorldScapeLod[i]->RelativePosition, SnappedPosition) > StepSize * 0.5 || ForceUpdate || HMIForceUpdate) 
				{
					WorldScapeLodInGeneration.Add(WorldScapeLod[i], false);
					(new FAutoDeleteAsyncTask <LodGenerationThread>(WorldScapeLod[i], this, SnappedPosition, PlanetLocation, SubPosition, PlanetScaleCode, i,
						TriangleSize * AltitudeMultiplier, LodResolution, PlayerDistanceToGround, PlayerPositionNormal, bFlatWorld, IsInViewPort))->StartBackgroundTask();
				}
			}
			if (bOcean)
			{
				for (int32 i = 0; i < WorldScapeLodOcean.Num(); i++) 
				{

					StepSize = (double)OceanTriangleSize * pow(2, i) * AltitudeMultiplier;
					WorldScapeWorld::GetSnappedPosition(SnappedPosition, PlayerPositionNormal, SubPosition, NormalChanged, StepSize, ForceUpdate, PPos, 
						PlanetLocation, i, AltitudeMultiplier, OceanTriangleSize, PlanetScaleCode, GridAngle, bFlatWorld,false);
					if (DVector::Dist(WorldScapeLodOcean[i]->RelativePosition, SnappedPosition) > StepSize * 0.5 || ForceUpdate || HMIForceUpdate) 
					{
						WorldScapeLodInGeneration.Add(WorldScapeLodOcean[i], false);
						(new FAutoDeleteAsyncTask <LodGenerationThread>(WorldScapeLodOcean[i], this, SnappedPosition, PlanetLocation, SubPosition, PlanetScaleCode, i,
							OceanTriangleSize * AltitudeMultiplier, OceanLodResolution, PlayerDistanceToGround, PlayerPositionNormal, bFlatWorld, IsInViewPort))->StartBackgroundTask();
					}
				}
			}

			if (HMIForceUpdate) 
			{
				HMIForceUpdate = false;
			}
		}
	}

}

void AWorldScapeRoot::CheckForLodGeneration()
{

	bool DoneGenerating = true;


	for (const TPair<UWorldScapeLod*, bool>& pair : WorldScapeLodInGeneration)
	{
		if (!pair.Value) 
		{
			DoneGenerating = false;
			break;
		}
	}
	if (DoneGenerating)
	{
		for (const TPair<UWorldScapeLod*, bool>& pair : WorldScapeLodInGeneration)
		{
			if (IsValid(pair.Key)) 
			{
				pair.Key->UpdateMesh();
			}
		}
		RootComponent->UpdateBounds();
		WorldScapeLodInGeneration.Empty();
	}
}

void AWorldScapeRoot::FoliageHandleTick() 
{

	if (FoliageGenerationDone)
	{
		if (FoliageGenerationInProgress)
		{
			FVector PlayerPosSurface = PlayerPos;
			if (bFlatWorld) PlayerPosSurface.Z = 0;

			FoliageGenerationInProgress = false;
			FoliageGenerationDone = false;

			if (FoliageDataList.Num() > 0)
			{
				for (int32 i = 0; i < FoliageDataList.Num(); i++)
				{

					if (!(FoliageDataList.IsValidIndex(i) && FoliageDataList[i].ActiveFoliageSector.Num() > 0 && FoliageDataList.IsValidIndex(i)))
					{
						break;
					}

					FVegetation* ActualFoliageData = &FoliageDataList[i];
					for (int32 j = FoliageDataList[i].ActiveFoliageSector.Num() - 1; j >= 0 ; j--) {

						if (!(ActualFoliageData->ActiveFoliageSector.IsValidIndex(j) && ActualFoliageData->ActiveFoliageSector[j].VegetationTransform.Num() > 0))
						{
							break;
						}
						FFoliageSector* ActualFoliageSector = &ActualFoliageData->ActiveFoliageSector[j];

						if (!ActualFoliageSector->FoliageSpawned)
						{
							for (int32 f = 0; f < ActualFoliageSector->VegetationTransform.Num(); f++) 
							{
								ActualFoliageSector->VegetationPosition[f] = ActualFoliageSector->VegetationPosition[f] + DVector(GetActorLocation());
								ActualFoliageSector->VegetationTransform[f].SetTranslation(ActualFoliageSector->VegetationPosition[f].ToFVector());
							}
							CreateInstancedMesh(ActualFoliageSector, Foliages[ActualFoliageData->FoliageCollectionIndex]->FoliageList[ActualFoliageData->FoliageIndex]);

							if (IsValid(ActualFoliageSector->InstancedMesh)) 
							{ 
								ActualFoliageSector->InstancedMesh->AddInstances(ActualFoliageSector->VegetationTransform, false);
							}
							ActualFoliageSector->FoliageSpawned = true;
						}
						else 
						{
							if (!WorldScapeHelper::IsPointInCube(PlayerPosSurface, ActualFoliageSector->Position, ActualFoliageSector->Size * 4))
							{
								this->RemoveInstanceComponent(ActualFoliageSector->InstancedMesh);
								ActualFoliageSector->InstancedMesh->DestroyComponent();
								ActualFoliageData->ActiveFoliageSector.RemoveAt(j);
							}
						}
					}
				}
			}
		}
	}
	else if (!FoliageGenerationInProgress) 
	{
		if (FoliageDataList.Num() > 0)
		{
			FoliageGenerationInProgress = true;
			(new FAutoDeleteAsyncTask <FoliageGenerationThread>(this,PlanetScaleCode, bFlatWorld,PlayerPos,FoliageDataList, PlayerDistanceToGround))->StartBackgroundTask();
		}
	}


}
