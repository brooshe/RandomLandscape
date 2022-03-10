// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "WorldScapeRoot.h"


#pragma region Helper

#pragma region CoordinateConversion
DVector AWorldScapeRoot::WorldToECEF(FVector FloatPosition)
{
	return WorldToECEF(DVector(FloatPosition));
}

DVector AWorldScapeRoot::WorldToECEF(DVector Position)
{
	FQuat FQuat = GetActorRotation().Quaternion();

	DVector4 Quaternion = DVector4(FQuat.X, FQuat.Y, FQuat.Z, FQuat.W);

	return (Position - DVector(GetActorLocation())).InverseTransformNoScale(Quaternion);
}

DVector AWorldScapeRoot::ECEFToWorld(FVector FloatPosition)
{
	return ECEFToWorld(DVector(FloatPosition));
}

DVector AWorldScapeRoot::ECEFToWorld(DVector Position)
{
	FQuat FQuat = GetActorRotation().Quaternion();

	DVector4 Quaternion = DVector4(FQuat.X, FQuat.Y, FQuat.Z, FQuat.W);

	return (Position).TransformNoScale(Quaternion) + DVector(GetActorLocation());
}

DVector AWorldScapeRoot::ECEFToProjectedPos(DVector Position)
{
	DVector PlayerPositionNormal = WorldScapeWorld::GetUpVector(Position, bFlatWorld);
	return WorldScapeWorld::GetProjectedPos(PlayerPos, GridAngle, PlanetScaleCode,
		bFlatWorld, PlayerPositionNormal);
}

#pragma endregion


void AWorldScapeRoot::UpdateTerrainMaterial(FWSMaterialLodArray pMaterialLodArray)
{
	TerrainMaterial = pMaterialLodArray;
	for (int32 i = 0; i < WorldScapeLod.Num(); i++)
	{
		WorldScapeLod[i]->MaterialLodArray = TerrainMaterial;

		UMaterialInterface* MaterialToUse = WorldScapeLod[i]->MaterialLodArray.DefaultMaterial;

		for (int j = 0; j < WorldScapeLod[i]->MaterialLodArray.MaterialsLod.Num(); j++)
		{
			if (WorldScapeLod[i]->MaterialLodArray.MaterialsLod[j].LodRange.Contains(WorldScapeLod[i]->MaterialLod))
			{
				MaterialToUse = WorldScapeLod[i]->MaterialLodArray.MaterialsLod[j].Material;
				break;
			}
		}
		WorldScapeLod[i]->Mesh->SetMaterial(0, MaterialToUse);
	}
}

void AWorldScapeRoot::UpdateOceanMaterial(FWSMaterialLodArray pMaterialLodArray)
{
	OceanMaterial = pMaterialLodArray;
	for (int32 i = 0; i < WorldScapeLodOcean.Num(); i++)
	{
		WorldScapeLodOcean[i]->MaterialLodArray = OceanMaterial;

		UMaterialInterface* MaterialToUse = WorldScapeLodOcean[i]->MaterialLodArray.DefaultMaterial;

		for (int j = 0; j < WorldScapeLodOcean[i]->MaterialLodArray.MaterialsLod.Num(); j++)
		{
			if (WorldScapeLodOcean[i]->MaterialLodArray.MaterialsLod[j].LodRange.Contains(WorldScapeLodOcean[i]->MaterialLod))
			{
				MaterialToUse = WorldScapeLodOcean[i]->MaterialLodArray.MaterialsLod[j].Material;
				break;
			}
		}
		WorldScapeLodOcean[i]->Mesh->SetMaterial(0, OceanMaterial.DefaultMaterial);
	}
}

#pragma region NormalTangentBitangentGetter

FVector AWorldScapeRoot::GetPawnNormal(FVector PawnWorldPosition)
{

	DVector Normal = PawnWorldPosition - PlanetLocation.ToFVector();
	Normal.Normalize();
	return Normal.ToFVector();
}

FVector AWorldScapeRoot::GetPawnSnappedNormal(FVector PawnWorldPosition)
{

	DVector Normal = GetPawnNormal(PawnWorldPosition);
	Normal.SnapNormal(GridAngle);
	return Normal.ToFVector();
}

FVector AWorldScapeRoot::GetPawnTangent(FVector PawnWorldPosition)
{
	DVector Tangent;
	DVector Normal = GetPawnNormal(PawnWorldPosition);
	Normal.SnapNormal(GridAngle);

	if (Normal == DVector(0, 0, 1)) {
		return FVector(1, 0, 0);
	}
	else {
		Tangent = DVector::CrossProduct(Normal, DVector(0, 0, 1));
		Tangent.Normalize();
	}
	return Tangent.ToFVector();
}

FVector AWorldScapeRoot::GetPawnBiTangent(FVector PawnWorldPosition)
{
	DVector Tangent;
	DVector BiTangent;
	DVector Normal = GetPawnNormal(PawnWorldPosition);
	Normal.SnapNormal(GridAngle);
	if (abs(Normal.Z) > 0.90) {
		return FVector(0, 1, 0);
	}
	else {
		Tangent = DVector::CrossProduct(Normal, DVector(0, 0, 1));
		Tangent.Normalize();
		BiTangent = DVector::CrossProduct(Normal, Tangent);
		BiTangent.Normalize();
	}
	return BiTangent.ToFVector();
}

FVector AWorldScapeRoot::GetTangentFromECEFPoint(FVector ECEFPosition)
{
	return WorldScapeWorld::GetTangent(ECEFPosition, GridAngle, bFlatWorld);
}

FVector AWorldScapeRoot::GetBiTangentFromECEFPoint(FVector ECEFPosition)
{
	return WorldScapeWorld::GetBiTangent(ECEFPosition, GridAngle, bFlatWorld);
}
#pragma endregion

#pragma region HeightAltitudeDistanceGetter
float AWorldScapeRoot::GetPawnDistanceFromGround(FVector PawnPosition, bool ECEFCoordinate)
{
	return GetPawnDistanceFromGround(DVector(PawnPosition), ECEFCoordinate);
}

float AWorldScapeRoot::GetPawnDistanceFromGround(DVector PawnPosition, bool ECEFCoordinate)
{
	FNoiseData NData;
	DVector ECEFPosition = PawnPosition;

	if (!ECEFCoordinate)
	{
		ECEFPosition = WorldToECEF(ECEFPosition);
	}
	NData = GetGroundNoise(ECEFPosition.ToFVector());

	return WorldScapeWorld::GetPawnDistanceFromGround(ECEFPosition, bFlatWorld, NData, PlanetScaleCode);
}

float AWorldScapeRoot::GetGroundHeight(FVector PawnWorldPosition, bool Water)
{
	FNoiseData NData;

	DVector ECEFPosition = WorldToECEF(DVector(PawnWorldPosition));

	NData = GetGroundNoise(ECEFPosition.ToFVector(), Water);

	return NData.Height;
}

float AWorldScapeRoot::GetGroundHeightNormalize(FVector PawnWorldPosition, bool Water)
{
	FNoiseData NData;

	DVector ECEFPosition = WorldToECEF(DVector(PawnWorldPosition));

	NData = GetGroundNoise(ECEFPosition.ToFVector(), Water);

	return NData.HeightNormalize;
}

float AWorldScapeRoot::GetHeight(FVector PawnWorldPosition, bool Water)
{
	FNoiseData NData;

	DVector ECEFPosition = WorldToECEF(DVector(PawnWorldPosition));

	NData = GetNoise(ECEFPosition.ToFVector(), Water);

	return NData.Height;
}

float AWorldScapeRoot::GetHeightNormalize(FVector PawnWorldPosition, bool Water)
{
	FNoiseData NData;

	DVector ECEFPosition = WorldToECEF(DVector(PawnWorldPosition));

	NData = GetNoise(ECEFPosition.ToFVector(), Water);

	return NData.HeightNormalize;
}

float AWorldScapeRoot::GetPawnAltitude(FVector PawnWorldPosition)
{
	FNoiseData NData;

	DVector ECEFPosition = WorldToECEF(DVector(PawnWorldPosition));
	NData = GetGroundNoise(ECEFPosition.ToFVector());
	return WorldScapeWorld::GetPawnAltitude(ECEFPosition, bFlatWorld, PlanetScaleCode, OceanHeight);
}
#pragma endregion

float AWorldScapeRoot::GetLattitude(FVector Position)
{
	return WorldScapeWorld::GetLattitude(Position, LattitudeRotation, bFlatWorld, PlanetScaleCode);
}

#pragma region WorldScapeVolumeGetter

TArray<AHeightMapVolume*> AWorldScapeRoot::GetHeightMapVolumeList(FVector Position)
{
	TArray<AHeightMapVolume*> HeightVolumeListVar;

	//Make sure only Volumes touching the chunk are proceced to reduce CPU Cost
	if (!(bEnableVolumes && HeightMapVolumeList.Num() > 0))
	{
		return HeightVolumeListVar;
	}

	if (HeightMapVolumeList.Num() < 3) {
		return HeightMapVolumeList;
	}

	for (int32 i = 0; i < HeightMapVolumeList.Num(); i++)
	{
		if (!(HeightMapVolumeList.IsValidIndex(i) && IsValid(HeightMapVolumeList[i]) && HeightMapVolumeList[i]->CanSample()))
		{
			continue;
		}

		if (WorldScapeHelper::IsPointInCube(PlayerPos, WorldToECEF(HeightMapVolumeList[i]->GetActorLocation()), HeightMapVolumeList[i]->GetActorScale() * 500))
		{
			HeightVolumeListVar.Add(HeightMapVolumeList[i]);
		}
	}
	return HeightVolumeListVar;
}

TArray<AHeightMapVolume*> AWorldScapeRoot::GetHeightMapVolumeList(FBox pFBox)
{
	TArray<AHeightMapVolume*> VolumeListVar;
	//Make sure only Volume touching the chunk are proceced to reduce CPU Cost
	if (!(bEnableVolumes && HeightMapVolumeList.Num() > 0))
	{
		return VolumeListVar;
	}

	if (HeightMapVolumeList.Num() < 3) {
		return HeightMapVolumeList;
	}

	for (int32 i = 0; i < HeightMapVolumeList.Num(); i++)
	{
		if (!(HeightMapVolumeList.IsValidIndex(i) && IsValid(HeightMapVolumeList[i]) && HeightMapVolumeList[i]->CanSample()))
		{
			continue;
		}


		float VolumeSize = HeightMapVolumeList[i]->GetActorScale().X * 250;
		FBox VolumeBox = FBox(WorldToECEF(DVector(HeightMapVolumeList[i]->GetActorLocation()) - DVector(VolumeSize)).ToFVector(), WorldToECEF(DVector(HeightMapVolumeList[i]->GetActorLocation()) + DVector(VolumeSize)).ToFVector());

		if (pFBox.Intersect(VolumeBox))	VolumeListVar.Add(HeightMapVolumeList[i]);
	}

	return VolumeListVar;
}

TArray<ANoiseVolume*> AWorldScapeRoot::GetNoiseVolumeList(FVector Position)
{
	TArray<ANoiseVolume*> NoiseVolumeListVar;

	//Make sure only Volume touching the chunk are proceced to reduce CPU Cost
	if (!(bEnableVolumes && NoiseVolumeList.Num() > 0))
	{
		return NoiseVolumeListVar;
	}

	if (NoiseVolumeList.Num() < 3)
	{
		return NoiseVolumeList;
	}

	for (int32 i = 0; i < NoiseVolumeList.Num(); i++)
	{
		if (!(NoiseVolumeList.IsValidIndex(i) && IsValid(NoiseVolumeList[i]) && NoiseVolumeList[i]->IsValidNoise()))
		{
			continue;
		}

		if (WorldScapeHelper::IsPointInCube(PlayerPos, WorldToECEF(NoiseVolumeList[i]->GetActorLocation()), HeightMapVolumeList[i]->GetActorScale() * 500))
		{
			NoiseVolumeListVar.Add(NoiseVolumeList[i]);
		}
	}

	return NoiseVolumeListVar;
}

TArray<ANoiseVolume*> AWorldScapeRoot::GetNoiseVolumeList(FBox pFBox)
{

	TArray<ANoiseVolume*> NoiseVolumeListVar;
	//Make sure only Volume touching the chunk are proceced to reduce CPU Cost
	if (!(bEnableVolumes && NoiseVolumeList.Num() > 0))
	{
		return NoiseVolumeListVar;
	}

	if (NoiseVolumeList.Num() < 3)
	{
		return NoiseVolumeList;
	}

	for (int32 i = 0; i < NoiseVolumeList.Num(); i++)
	{
		if (!(NoiseVolumeList.IsValidIndex(i) && IsValid(NoiseVolumeList[i]) && NoiseVolumeList[i]->IsValidNoise()))
		{
			continue;
		}

		float VolumeSize = NoiseVolumeList[i]->GetActorScale().X * 250;
		FBox VolumeBox = FBox(WorldToECEF(DVector(NoiseVolumeList[i]->GetActorLocation()) - DVector(VolumeSize)).ToFVector(), WorldToECEF(DVector(NoiseVolumeList[i]->GetActorLocation()) + DVector(VolumeSize)).ToFVector());

		if (pFBox.Intersect(VolumeBox))
		{
			NoiseVolumeListVar.Add(NoiseVolumeList[i]);
		}

	}
	return NoiseVolumeListVar;
}

TArray<AFoliageMaskVolume*> AWorldScapeRoot::GetFoliageMaskList(FVector Position)
{

	TArray<AFoliageMaskVolume*> FoliageMaskListVar;

	//Make sure only Volume touching the chunk are proceced to reduce CPU Cost
	if (bEnableVolumes && FoliageMaskList.Num() > 0)
	{
		return FoliageMaskList;
	}

	if (FoliageMaskList.Num() < 3)
	{
		return FoliageMaskList;
	}

	FoliageMaskListVar.Empty();
	for (int32 i = 0; i < FoliageMaskList.Num(); i++)
	{
		if (!(FoliageMaskList.IsValidIndex(i) && IsValid(FoliageMaskList[i])))
		{
			continue;
		}

		if (WorldScapeHelper::IsPointInCube(PlayerPos, WorldToECEF(FoliageMaskList[i]->GetActorLocation()), HeightMapVolumeList[i]->GetActorScale() * 500))
		{
			FoliageMaskListVar.Add(FoliageMaskList[i]);
		}
	}

	return FoliageMaskListVar;
}

TArray<AFoliageMaskVolume*> AWorldScapeRoot::GetFoliageMaskList(FBox pFBox)
{

	TArray<AFoliageMaskVolume*> FoliageMaskListVar;
	//Make sure only Volume touching the chunk are proceced to reduce CPU Cost
	if (bEnableVolumes && FoliageMaskList.Num() > 0)
	{
		return FoliageMaskList;
	}

	if (FoliageMaskList.Num() < 3)
	{
		return FoliageMaskList;
	}

	FoliageMaskListVar.Empty();
	for (int32 i = 0; i < FoliageMaskList.Num(); i++)
	{
		if (!(FoliageMaskList.IsValidIndex(i) && IsValid(FoliageMaskList[i])))
		{
			continue;
		}

		float VolumeSize = FoliageMaskList[i]->GetActorScale().X * 250;
		FBox VolumeBox = FBox(WorldToECEF(DVector(FoliageMaskList[i]->GetActorLocation()) - DVector(VolumeSize)).ToFVector(), WorldToECEF(DVector(FoliageMaskList[i]->GetActorLocation()) + DVector(VolumeSize)).ToFVector());
		if (pFBox.Intersect(VolumeBox))
		{
			FoliageMaskListVar.Add(FoliageMaskList[i]);
		}
	}
	return FoliageMaskListVar;
}

#pragma endregion

FFoliageSector AWorldScapeRoot::GetFoliageSector(DVector ECEFPosition, int SectorLod)
{
	double SectorSize = PlanetScaleCode * pow(0.5, SectorLod);

	DVector GridPosition = DVector(
		round(ECEFPosition.X / SectorSize) * SectorSize,
		round(ECEFPosition.Y / SectorSize) * SectorSize,
		round(ECEFPosition.Z / SectorSize) * SectorSize);
	if (bFlatWorld) GridPosition.Z = 0;

	return FFoliageSector(GridPosition, SectorSize);
}

TArray<FFoliageSector> AWorldScapeRoot::GetSurroundingFoliageSector(DVector ECEFPosition, double pSectorSize)
{
	TArray<FFoliageSector> SectorList;
	double SectorSize = pSectorSize;

	DVector GridPosition = DVector(
		round(ECEFPosition.X / SectorSize) * SectorSize,
		round(ECEFPosition.Y / SectorSize) * SectorSize,
		round(ECEFPosition.Z / SectorSize) * SectorSize);

	if (bFlatWorld) {
		GridPosition.Z = 0;
		SectorList.Add(FFoliageSector(GridPosition, SectorSize));

		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, 0, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, 0, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(0, SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(0, -SectorSize, 0), SectorSize));

		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, -SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, -SectorSize, 0), SectorSize));
	}
	else
	{
		SectorList.Add(FFoliageSector(GridPosition, SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, 0, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, 0, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(0, SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(0, -SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, -SectorSize, 0), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, -SectorSize, 0), SectorSize));

		SectorList.Add(FFoliageSector(GridPosition + DVector(0, 0, SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, 0, SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, 0, SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(0, SectorSize, SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(0, -SectorSize, SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, SectorSize, SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, SectorSize, SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, -SectorSize, SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, -SectorSize, SectorSize), SectorSize));

		SectorList.Add(FFoliageSector(GridPosition + DVector(0, 0, -SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, 0, -SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, 0, -SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(0, SectorSize, -SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(0, -SectorSize, -SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, SectorSize, -SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, SectorSize, -SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(SectorSize, -SectorSize, -SectorSize), SectorSize));
		SectorList.Add(FFoliageSector(GridPosition + DVector(-SectorSize, -SectorSize, -SectorSize), SectorSize));
	}

	return SectorList;
}

#pragma endregion


