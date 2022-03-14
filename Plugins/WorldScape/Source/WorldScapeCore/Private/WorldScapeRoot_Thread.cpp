// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "WorldScapeRoot.h"

//ChunkGeneration Thread Functions

LodGenerationThread::~LodGenerationThread()
{

}

FoliageGenerationThread::~FoliageGenerationThread()
{

}





LodGenerationThread::LodGenerationThread(UWorldScapeLod* pWorldScapeLod, AWorldScapeRoot* pRoot, DVector pRelativePosition, DVector pPlanetLocation,FVector2D pSubPosition,
	double pPlanetScale, int32 pLod, float pSize, int32 pVertWidth, double pPlayerAltitude,DVector pHardenNormal, bool pFlatWorld, bool pIsInViewPort)
{
	WorldScapeLodRef = pWorldScapeLod;
	RootRef = pRoot;
	Lod = pLod;
	PlanetScaleCode = pPlanetScale;
	Size = pSize;
	TriResolution = pVertWidth;
	PlayerDistanceToGround = pPlayerAltitude;
	RelativePosition = pRelativePosition;
	SubPosition = pSubPosition;
	HardenNormal = pHardenNormal;
	MainPatch = LodData();
	PatchA = LodData();
	PatchB = LodData();
	bFlatWorld = pFlatWorld;
	IsInViewPort = pIsInViewPort;
}


void LodGenerationThread::CalculateNoise()
{
	DVector VertexNormal;

	FVector NoisePosition;
	FLinearColor VertexColor;
	TArray<AHeightMapVolume*> VolumeListVar;
	TArray<ANoiseVolume*> NoiseVolumeListVar;

	//Make sure only Volume touching the chunk are proceced to reduce CPU Cost


	if (IsValid(RootRef) && IsValid(WorldScapeLodRef)) 
	{
		VolumeListVar = RootRef->GetHeightMapVolumeList(WorldScapeLodRef->GetFBox());
		NoiseVolumeListVar = RootRef->GetNoiseVolumeList(WorldScapeLodRef->GetFBox());
	}

	
		
		

	TArray<FTransform> VolumeTransformArray;
	for (int32 i = 0; i < VolumeListVar.Num(); i++)
	{
		if (VolumeListVar.IsValidIndex(i) && IsValid(VolumeListVar[i])) {
			VolumeTransformArray.Add(VolumeListVar[i]->GetTransform());

			/*
			//Patchwork to make sure it work in shipping :
			VolumeTransformArray.Last().SetScale3D(VolumeListVar[i]->VolumeScale);
			VolumeTransformArray.Last().SetTranslation(VolumeListVar[i]->VolumePosition);
			VolumeTransformArray.Last().SetRotation(FQuat(VolumeListVar[i]->VolumeRotation));
			*/
		}
			
	}

	//UE_LOG(LogWorldScapeCore, Log, TEXT("Volume Size : %i"), VolumeListVar.Num());
	FVector VertexWorldTransformedPosition;
	FVector VertexTransformedPosition;
	for (int32 i = 0; i < MainPatch.Vertices.Num(); i++)
	{
		//SurfacePosition
		FNoiseData Data;

		VertexNormal = MainPatch.Vertices[i] + RelativePosition + OffSetHelper;
		Data = RootRef->GetGroundNoise(VertexNormal.ToFVector(), NoiseVolumeListVar, VolumeListVar, VolumeTransformArray, WorldScapeLodRef->WaterBody);
		if (bFlatWorld) 
		{
			MainPatch.Vertices[i] = (DVector(0,0,1) * Data.Height + MainPatch.Vertices[i]).ToFVector();
		}
		else 
		{
			VertexNormal.Normalize();
			MainPatch.Vertices[i] = (VertexNormal * Data.Height + OffSetHelper + MainPatch.Vertices[i]).ToFVector();
		}
		float Hole = 0;
		if (Data.Hole) Hole = 1;
		VertexColor = FLinearColor(Data.HeightNormalize, Data.Temperature, Data.Humidity, Hole);
		MainPatch.VertexColors.Add(VertexColor);
	}

	for (int32 i = 0; i < PatchA.Vertices.Num(); i++)
	{

		FNoiseData Data;

		VertexNormal = PatchA.Vertices[i] + RelativePosition + OffSetHelper;
		Data = RootRef->GetGroundNoise(VertexNormal.ToFVector(), NoiseVolumeListVar, VolumeListVar, VolumeTransformArray, WorldScapeLodRef->WaterBody);
		if (bFlatWorld) 
		{
			PatchA.Vertices[i] = (DVector(0, 0, 1) * Data.Height + PatchA.Vertices[i]).ToFVector();
		}
		else 
		{
			VertexNormal.Normalize();
			PatchA.Vertices[i] = (VertexNormal * Data.Height + PatchA.Vertices[i] + OffSetHelper).ToFVector();
		}
		float Hole = 0;
		if (Data.Hole) Hole = 1;
		VertexColor = FLinearColor(Data.HeightNormalize, Data.Temperature, Data.Humidity, Hole);
		PatchA.VertexColors.Add(VertexColor);
	}

	for (int32 i = 0; i < PatchB.Vertices.Num(); i++)
	{
		FNoiseData Data;

		VertexNormal = PatchB.Vertices[i] + RelativePosition + OffSetHelper;
		Data = RootRef->GetGroundNoise(VertexNormal.ToFVector(), NoiseVolumeListVar, VolumeListVar, VolumeTransformArray, WorldScapeLodRef->WaterBody);
		if (bFlatWorld) 
		{
			PatchB.Vertices[i] = (DVector(0, 0, 1) * Data.Height + PatchB.Vertices[i]).ToFVector();
		}
		else 
		{
			VertexNormal.Normalize();
			PatchB.Vertices[i] = (VertexNormal * Data.Height + PatchB.Vertices[i] + OffSetHelper).ToFVector();
		}
		float Hole = 0;
		if (Data.Hole) Hole = 1;
		VertexColor = FLinearColor(Data.HeightNormalize, Data.Temperature, Data.Humidity, Hole);
		PatchB.VertexColors.Add(VertexColor);
	}
}

void LodGenerationThread::CalculateNormal(LodData& PatchData, int32 PatchID)
{
	//Patch ID Not Valid
	if (PatchID > 2 || PatchID < 0)
		return;


	TArray<FVector> Normals;
	if (WorldScapeLodRef->WaterBody) 
	{
		FVector Normal;
		for (int32 i = 0; i < PatchData.Vertices.Num(); i++) 
		{
			if (!bFlatWorld){
				Normal = (DVector(PatchData.Vertices[i]) + RelativePosition).ToFVector();
				Normal.Normalize();
				Normals.Add(Normal);
			}
			else Normals.Add(FVector(0,0,1));
		}

	}
	else 
	{
		if (false)
		{
			// Number of triangles
			int32 NumTris = 0;
			if (PatchID == 0) NumTris = WorldScapeLodRef->Triangles.Num() / 3;
			if (PatchID == 1) NumTris = WorldScapeLodRef->TrianglesPatchA.Num() / 3;
			if (PatchID == 2) NumTris = WorldScapeLodRef->TrianglesPatchB.Num() / 3;
			// Number of verts
			const int32 NumVerts = PatchData.Vertices.Num();
			// Map of vertex to triangles in Triangles array
			TMultiMap<int32, int32> VertToTriMap;
			// Map of vertex to triangles to consider for normal calculation
			TMultiMap<int32, int32> VertToTriSmoothMap;

			// Normal/tangents for each face
			TArray<FVector> FaceTangentX, FaceTangentY, FaceTangentZ;
			FaceTangentX.AddUninitialized(NumTris);
			FaceTangentY.AddUninitialized(NumTris);
			FaceTangentZ.AddUninitialized(NumTris);

			TArray<FVector> FVerticeList;
			for (int32 i = 0; i < PatchData.Vertices.Num(); i++) 
			{
				if (PatchData.Vertices.IsValidIndex(i)) FVerticeList.Add(PatchData.Vertices[i].ToFVector());
			}


			// Iterate over triangles
			for (int32 TriIdx = 0; TriIdx < NumTris; TriIdx++)
			{
				FVector P[3];
				for (int32 CornerIdx = 0; CornerIdx < 3; CornerIdx++)
				{
					// Find vert index (clamped within range)

					int32 VertIndex = 0;
					if (PatchID == 0) VertIndex = FMath::Min(WorldScapeLodRef->Triangles[(TriIdx * 3) + CornerIdx], NumVerts - 1);
					if (PatchID == 1) VertIndex = FMath::Min(WorldScapeLodRef->TrianglesPatchA[(TriIdx * 3) + CornerIdx], NumVerts - 1);
					if (PatchID == 2) VertIndex = FMath::Min(WorldScapeLodRef->TrianglesPatchB[(TriIdx * 3) + CornerIdx], NumVerts - 1);
					

					P[CornerIdx] = PatchData.Vertices[VertIndex].ToFVector();;

					// Find/add this vert to index buffer
					TArray<int32> VertOverlaps;
					WorldScapeHelper::FindVertOverlaps(VertIndex, FVerticeList, VertOverlaps);

					// Remember which triangles map to this vert
					VertToTriMap.AddUnique(VertIndex, TriIdx);
					VertToTriSmoothMap.AddUnique(VertIndex, TriIdx);

					// Also update map of triangles that 'overlap' this vert (ie don't match UV, but do match smoothing) and should be considered when calculating normal
					for (int32 OverlapIdx = 0; OverlapIdx < VertOverlaps.Num(); OverlapIdx++)
					{
						// For each vert we overlap..
						int32 OverlapVertIdx = VertOverlaps[OverlapIdx];

						// Add this triangle to that vert
						VertToTriSmoothMap.AddUnique(OverlapVertIdx, TriIdx);

						// And add all of its triangles to us
						TArray<int32> OverlapTris;
						VertToTriMap.MultiFind(OverlapVertIdx, OverlapTris);
						for (int32 OverlapTriIdx = 0; OverlapTriIdx < OverlapTris.Num(); OverlapTriIdx++)
						{
							VertToTriSmoothMap.AddUnique(VertIndex, OverlapTris[OverlapTriIdx]);
						}
					}
				}

				// Calculate triangle edge vectors and normal
				const FVector Edge21 = P[1] - P[2];
				const FVector Edge20 = P[0] - P[2];

				FaceTangentZ[TriIdx] = (Edge21 ^ Edge20).GetSafeNormal();;
			}

			// Arrays to accumulate tangents into
			TArray<FVector>VertexTangentZSum;
			VertexTangentZSum.AddZeroed(NumVerts);

			Normals.AddUninitialized(NumVerts);

			for (int32 VertxIdx = 0; VertxIdx < NumVerts; VertxIdx++)
			{
				TArray<int32> SmoothTris;
				VertToTriSmoothMap.MultiFind(VertxIdx, SmoothTris);

				for (int32 i = 0; i < SmoothTris.Num(); i++)
				{
					VertexTangentZSum[VertxIdx] += FaceTangentZ[SmoothTris[i]];
				}

				FVector& TangentZ = VertexTangentZSum[VertxIdx];
				TangentZ.Normalize();

				Normals[VertxIdx] = TangentZ;
			}
		}
	
		else 
		{
			if (PatchData.Vertices.Num() == 0)
			{
				return;
			}

			for (int32 i = 0; i < PatchData.Vertices.Num(); i++)
			{
				Normals.Add(FVector(0));
			}
			TArray<int32> TrianglesList;
			int32 TriCount = 0;
			if (IsValid(WorldScapeLodRef))
			{
				if (PatchID == 0) TrianglesList = WorldScapeLodRef->Triangles;
				else if (PatchID == 1) TrianglesList = WorldScapeLodRef->TrianglesPatchA;
				else if (PatchID == 2) TrianglesList = WorldScapeLodRef->TrianglesPatchB;

				TriCount = TrianglesList.Num();

				for (int32 i = 0; i < TriCount; i += 3)
				{

					FVector a, b, c;
					int A = 0,B = 1, C = 2;


					A = TrianglesList[i];
					B = TrianglesList[i + 1];
					C = TrianglesList[i + 2];

					if (PatchData.Vertices.IsValidIndex(A) && PatchData.Vertices.IsValidIndex(B) && PatchData.Vertices.IsValidIndex(C))
					{
						a = PatchData.Vertices[A].ToFVector();
						b = PatchData.Vertices[B].ToFVector();
						c = PatchData.Vertices[C].ToFVector();

						FVector Na = FVector::CrossProduct(c - a, b - a);

						Normals[A] += Na;
						Normals[B] += Na;
						Normals[C] += Na;
					}
				}
		
				for (int32 i = 0; i < PatchData.Vertices.Num(); i++)
				{
					Normals[i].Normalize();
				}
			}
		
		}
	}
	PatchData.Normals = Normals;
}

void LodGenerationThread::SetPointPosition()
{
	DVector Normal = HardenNormal;
	
	Normal.SnapNormal(RootRef->GridAngle);


	bool WaterBody = false;

	float uvSpacing = 1.0f / (float)TriResolution;
	double StepSize = (double)Size * pow(2, Lod);
	
	DVector Tangent = DVector::CrossProduct(Normal, DVector(0, 0, 1));
	DVector BiTangent = DVector::CrossProduct(Normal, Tangent);

	Tangent.Normalize();
	BiTangent.Normalize();

	double OffsetX = StepSize * 0.5;
	double OffsetY = StepSize * 0.5;

	bool InvertOrder = false;
	
	if (bFlatWorld) 
	{
		Tangent = FVector(1, 0, 0);
		BiTangent = FVector(0, 1, 0);
	}
	else if (abs(Normal.Z) > 0.9) 
	{
		Tangent = FVector(1,0,0);
		BiTangent = FVector(0, 1, 0);
		if (Normal.Z < 0) 
		{
			InvertOrder = true;
		}
	}

	//DVector Center = FVector(WorldPosition.X, WorldPosition.Y, PlanetScaleCode);

	DVector X, Y;
	DVector PositionNormal;

	//double stepSize = (double)Size * pow(2, Lod);
	double HalfSize = (Size * pow(2, Lod) ) * (TriResolution-1)*0.5;

	/*
	UE_LOG(LogWorldScapeCore, Log, TEXT("Planet Scale : %f"), (float)PlanetScaleCode);
	UE_LOG(LogWorldScapeCore, Log, TEXT("Step Size : %f"), (float)stepSize);
	UE_LOG(LogWorldScapeCore, Log, TEXT("Step Size : %f"), (float)stepSize);
	*/
	int32 MinY;
	int32 MinX; 
	int32 MaxY;
	int32 MaxX;
	int32 MinBorderY;
	int32 MinBorderX;
	int32 MaxBorderY;
	int32 MaxBorderX;

	if (SubPosition.X < 0.5f) 
	{
		MinBorderX = 0;
		MaxBorderX = TriResolution - 2;
	}
	else 
	{
		MinBorderX = -1;
		MaxBorderX = TriResolution - 3;
	}
	if (SubPosition.Y < 0.5f) 
	{
		MinBorderY = 0;
		MaxBorderY = TriResolution - 2;
	}
	else 
	{
		MinBorderY = -1;
		MaxBorderY = TriResolution - 3;
	}

	int32 WorldMinX, WorldMinY;
	int32 WorldMaxX, WorldMaxY;
	WorldMinX = WorldMinY = -65536;
	WorldMaxX = WorldMaxY = 65536;
	if (bFlatWorld && RootRef->WorldSize > 0)
	{
		WorldMinX = floor((-RootRef->WorldSize*0.5f + HalfSize - OffsetX - RelativePosition.X) / StepSize);
		WorldMinY = floor((-RootRef->WorldSize*0.5f + HalfSize - OffsetY - RelativePosition.Y) / StepSize);
		WorldMaxX = ceil((RootRef->WorldSize*0.5f + HalfSize - OffsetX - RelativePosition.X) / StepSize);
		WorldMaxY = ceil((RootRef->WorldSize*0.5f + HalfSize - OffsetY - RelativePosition.Y) / StepSize);
		// UE_LOG(LogTemp, Warning, TEXT("SetpointPosition   WorldSize:%i  RelativePosition:%s  StepSize:%f, WorldMinX:%i  WorldMinY:%i  WorldMaxX:%i   WorldMaxY:%i"), RootRef->WorldSize, *RelativePosition.ToString(), StepSize, WorldMinX, WorldMinY, WorldMaxX, WorldMaxY);
	}

	if (Lod == 0) 
	{
		ProcessVertices(MainPatch, 0, 0, TriResolution - 2, TriResolution - 2, Tangent, BiTangent, StepSize, HalfSize, uvSpacing, SubPosition,
			MinBorderY, MinBorderX, MaxBorderY, MaxBorderX, WorldMinY, WorldMinX, WorldMaxY, WorldMaxX, OffsetX,OffsetY, InvertOrder,PlanetScaleCode, RelativePosition, bFlatWorld);
	}
	else 
	{
		for (int32 step = 0; step < 4; step++) 
		{
			switch (step)
			{
			case 0:
				MinY = 0;
				MaxY = TriResolution / 4;
				MinX = 0;
				MaxX = TriResolution - 2;
				break;
			case 1:
				MinY = TriResolution / 4 - 1;
				MaxY = (3 * TriResolution) / 4 - 1;
				MinX = 0;
				MaxX = TriResolution / 4;
				break;
			case 2:
				MinY = TriResolution / 4 - 1;
				MaxY = (3 * TriResolution) / 4 - 1;
				MinX = (3 * TriResolution) / 4 - 2;
				MaxX = TriResolution - 2;
				break;
			case 3:
				MinY = (3 * TriResolution) / 4 - 2;
				MaxY = TriResolution - 2;
				MinX = 0;
				MaxX = TriResolution - 2;
				break;
			default:
				MinY = 0;
				MaxY = 0;
				MinX = 0;
				MaxX = 0;
				break;
			}

			ProcessVertices(MainPatch, MinY, MinX, MaxY, MaxX, Tangent, BiTangent, StepSize, HalfSize, uvSpacing, SubPosition,
				MinBorderY, MinBorderX, MaxBorderY, MaxBorderX,WorldMinY, WorldMinX, WorldMaxY, WorldMaxX, OffsetX, OffsetY, InvertOrder, PlanetScaleCode, RelativePosition, bFlatWorld);
		}
	}

	if (SubPosition.X < 0.5f) 
	{
		MinX = TriResolution - 3;
		MaxX = TriResolution-1;
	}
	else 
	{
		MinX = -1;
		MaxX =  1;
	}

	if (SubPosition.Y < 0.5f) 
	{
		MinY = 0;
		MaxY = TriResolution - 1;
	}
	else 
	{
		MinY = -1;
		MaxY = TriResolution - 2;
	}

	ProcessVertices(PatchA, MinY, MinX, MaxY, MaxX, Tangent, BiTangent, StepSize, HalfSize, uvSpacing, SubPosition,
		MinBorderY, MinBorderX, MaxBorderY, MaxBorderX, WorldMinY, WorldMinX, WorldMaxY, WorldMaxX, OffsetX, OffsetY, InvertOrder, PlanetScaleCode, RelativePosition, bFlatWorld);

	if (SubPosition.Y < 0.5f) 
	{
		MinY = TriResolution - 3;
		MaxY = TriResolution-1;
	}
	else 
	{
		MinY = -1;
		MaxY = 1;
	}

	MinX = 0;
	MaxX = TriResolution - 2;

	ProcessVertices(PatchB, MinY, MinX, MaxY, MaxX, Tangent, BiTangent, StepSize, HalfSize, uvSpacing, SubPosition, 
		MinBorderY, MinBorderX, MaxBorderY, MaxBorderX, WorldMinY, WorldMinX, WorldMaxY, WorldMaxX, OffsetX, OffsetY, InvertOrder, PlanetScaleCode, RelativePosition, bFlatWorld);
}

void LodGenerationThread::DoWork() 
{

	
	 //Size = (float)(FMath::Max((double)RootRef->HeightAnchor, AltitudeMultiplier) * (double)Size * (1. / (double)RootRef->HeightAnchor));

	double ChunkSize = Size * pow(2, Lod) * TriResolution;
	WorldScapeLodRef->SetData(MainPatch, ChunkSize, -1);

	SetPointPosition();
	
	bool DisplaceSurface = !IsInViewPort;

	OffSetHelper = 0;

	if (!bFlatWorld && DisplaceSurface && IsValid(RootRef))
	{
		DVector OldRelativePosition = RelativePosition;
		RelativePosition.Normalize();
		RelativePosition = DVector((RelativePosition * PlanetScaleCode).ToFVector());
		OffSetHelper = OldRelativePosition - RelativePosition;
	}
	if (IsValid(RootRef) && IsValid(WorldScapeLodRef) && RootRef->WorldScapeLodInGeneration.Contains(WorldScapeLodRef))
	{
		WorldScapeLodRef->SetRelativePosition(RelativePosition);
	}
	if (RootRef->NoiseIntensity > 0)
		CalculateNoise();
	
	CalculateNormal(MainPatch,0);
	CalculateNormal(PatchA, 1);
	CalculateNormal(PatchB, 2);
	
	if (IsValid(RootRef) && IsValid(WorldScapeLodRef) && RootRef->WorldScapeLodInGeneration.Contains(WorldScapeLodRef)) 
	{
		WorldScapeLodRef->SetData(MainPatch, ChunkSize,0);
		WorldScapeLodRef->SetData(PatchA, ChunkSize,1);
		WorldScapeLodRef->SetData(PatchB, ChunkSize, 2);


		int AltitudeLod = (int)round(log2(PlayerDistanceToGround) - log2(RootRef->HeightAnchor)) ;

		if (AltitudeLod < 0) AltitudeLod = 0;

		WorldScapeLodRef->SetMeshLod(Lod+ AltitudeLod);

		RootRef->WorldScapeLodInGeneration[WorldScapeLodRef] = true;
	}
	
}

void LodGenerationThread::DoWorkMain()
{
	DoWork();
}

void FoliageGenerationThread::DoWorkMain()
{
	DoWork();
}


void FoliageGenerationThread::GenerateVegetationForSector(FFoliageSector& Sector, int32 FoliageCollectionID, int32 FoliageID, int32 FLID) {

	

	//Chunk.
	float CoordinateSeed = (Sector.Position.X / PlanetScaleCode) * (Sector.Size * 35000) - (Sector.Position.Y / PlanetScaleCode) * (Sector.Size * 25000) + (Sector.Size * 5000) * (Sector.Position.Z / PlanetScaleCode) + 100 * FoliageCollectionID + 2000 * FoliageID + 50000 * FLID;
	int64 SectorSeed = RootRef->Seed + CoordinateSeed;

	if (abs(SectorSeed) > 2000000000) 
	{
		int32 sign = 1;
		if (SectorSeed < 0)sign = -1;
		int64 SectorRepeat = (abs(SectorSeed) / 2000000000);
		SectorSeed = (abs(SectorSeed) - ((int64)2000000000 * SectorRepeat)) * sign;
	}



	FRandomStream Stream = FRandomStream((int32)SectorSeed);

	//Make sure only Volume touching the chunk are proceced to reduce CPU Cost
	TArray<AHeightMapVolume*> VolumeListVar = RootRef->GetHeightMapVolumeList(Sector.GetFBox());
	TArray<ANoiseVolume*> NoiseVolumeListVar = RootRef->GetNoiseVolumeList(Sector.GetFBox());
	TArray<AFoliageMaskVolume*> FoliageMaskListVar = RootRef->GetFoliageMaskList(Sector.GetFBox());
	

	TArray<FTransform> VolumeTransformArray;
	for (int32 i = 0; i < VolumeListVar.Num(); i++)
	{
		if (VolumeListVar.IsValidIndex(i) && IsValid(VolumeListVar[i]))
		{
			VolumeTransformArray.Add(VolumeListVar[i]->GetTransform());
		}
			
	}

	//UE_LOG(LogWorldScapeCore, Log, TEXT("Seed : %i"), (int)SectorSeed);

	if (IsValid(RootRef) &&
		RootRef->Foliages.IsValidIndex(FoliageCollectionID) && IsValid(RootRef->Foliages[FoliageCollectionID]) &&
		RootRef->Foliages[FoliageCollectionID]->FoliageList.IsValidIndex(FoliageID) && IsValid(RootRef->Foliages[FoliageCollectionID]->FoliageList[FoliageID]))
	{

		UWorldScapeFoliageCollection* ActiveFoliageCollection = RootRef->Foliages[FoliageCollectionID];
		UWorldScapeFoliage* ActiveFoliage = ActiveFoliageCollection->FoliageList[FoliageID];
		double OceanHeight = RootRef->OceanHeight;

		// Get a cluster of points
		TArray<DVector> PointsList;
		FBox ChunkBox = Sector.GetFBox();
		for (int32 j = 0; j < ActiveFoliage->FoliagesCount; j++) 
		{
			//FVector RandomPoint = ;
			PointsList.Add(DVector(WorldScapeHelper::RandPointInBox(ChunkBox, Stream)));



		}



		FVector VertexWorldTransformedPosition;
		FVector VertexTransformedPosition;
		int32 ChunkPosition = 0;
		DVector PNA, PNB;
		DVector Tangent, Bitangent;
		DVector PointPosition;
		for (int32 p = 0; p < PointsList.Num(); p++) 
		{
			RootRef->FoliageCountLeft = PointsList.Num() - p;
			PointPosition = PointsList[p];
			DVector Normal;
			DVector VirtualPos;



			Normal = WorldScapeWorld::GetUpVector(PointPosition, bFlatWorld);
			Tangent = RootRef->GetTangentFromECEFPoint(PointPosition.ToFVector());
			Bitangent = RootRef->GetBiTangentFromECEFPoint(PointPosition.ToFVector());



			VirtualPos = WorldScapeWorld::GetVirtualPosition(PointPosition, PlanetScaleCode, bFlatWorld);


			FNoiseData NoiseData = RootRef->GetGroundNoise(PointPosition.ToFVector(), NoiseVolumeListVar, VolumeListVar, VolumeTransformArray);

			if (NoiseData.Hole)
			{
				continue;
			}

			float randomvalue = 0;

			if (ActiveFoliage->bUseFoliageNoiseMask)
				randomvalue = Stream.FRand();

			int foliageLayer = ActiveFoliage->FoliageLayer;

			if (foliageLayer < 0) {
				foliageLayer = ActiveFoliageCollection->FoliageLayer;
			}

			PointPosition = Normal * NoiseData.Height + VirtualPos;
			if (WorldScapeHelper::IsPointInCube(PointPosition, Sector.Position, Sector.Size) || bFlatWorld)
			{
				bool IsFoliageInForbidenZone = false;
				FVector PositionTransformedPosition;
				FVector PositionWorldTransformedPosition = PointPosition.ToFVector();
				PositionWorldTransformedPosition = RootRef->GetActorTransform().TransformPositionNoScale(PositionWorldTransformedPosition);

				for (int32 h = 0; h < FoliageMaskListVar.Num(); h++) 
				{
					if (!(FoliageMaskListVar.IsValidIndex(h) && IsValid(FoliageMaskListVar[h])))
					{
						continue;
					}
					if (!FoliageMaskListVar[h]->FoliageLayerMask.Contains(foliageLayer))
					{
						continue;
					}

					AFoliageMaskVolume* ActualVolume = FoliageMaskListVar[h];
					float VolumeSize = ActualVolume->GetActorScale3D().X * 50;

					if ((PositionWorldTransformedPosition - ActualVolume->GetActorLocation()).Size() > VolumeSize * 3)
					{
						continue;
					}
								
					PositionTransformedPosition = ActualVolume->GetActorTransform().InverseTransformPositionNoScale(PositionWorldTransformedPosition);
					PositionTransformedPosition /= VolumeSize * 2;

					if (PositionTransformedPosition.X >= -0.5 && PositionTransformedPosition.X <=0.5 &&
						PositionTransformedPosition.Y >= -0.5 && PositionTransformedPosition.Y <= 0.5 &&
						PositionTransformedPosition.Z >= -0.5 && PositionTransformedPosition.Z <= 0.5)
					{
						IsFoliageInForbidenZone = true;
						break;
					}
				}

				if (IsFoliageInForbidenZone)
				{
					continue;
				}

				bool Contraint = false;

				if (ActiveFoliage->bOverrideCollectionContraint) 
				{
					Contraint = (NoiseData.Humidity <= ActiveFoliage->MaxHumidity && NoiseData.Humidity >= ActiveFoliage->MinHumidity &&
						NoiseData.Temperature <= ActiveFoliage->MaxTemperature && NoiseData.Temperature >= ActiveFoliage->MinTemperature &&
						NoiseData.Height <= ActiveFoliage->MaxElevation && NoiseData.Height >= ActiveFoliage->MinElevation &&
						randomvalue <= NoiseData.FoliageMask
						);
				}
				else 
				{
					Contraint = (NoiseData.Humidity <= ActiveFoliageCollection->MaxHumidity && NoiseData.Humidity >= ActiveFoliageCollection->MinHumidity &&
						NoiseData.Temperature <= ActiveFoliageCollection->MaxTemperature && NoiseData.Temperature >= ActiveFoliageCollection->MinTemperature &&
						NoiseData.Height <= ActiveFoliageCollection->MaxElevation && NoiseData.Height >= ActiveFoliageCollection->MinElevation &&
						randomvalue <= NoiseData.FoliageMask
						);
				}

				if (RootRef->bOcean)
				{
					if (Contraint && ActiveFoliageCollection->bSpawnInWater)
					{
						Contraint = (NoiseData.Height < OceanHeight || NoiseData.WaterMask>0.85);
					}
					else if (Contraint)
					{
						Contraint = (NoiseData.Height > OceanHeight && NoiseData.WaterMask < 0.85);
					}
				}

				if (!Contraint)
				{
					continue;
				}

				FVector FoliageNormal;
				float NoiseDataAHeight = RootRef->GetGroundNoise((PointPosition + Tangent * 100).ToFVector(), NoiseVolumeListVar, VolumeListVar, VolumeTransformArray).Height;
				float NoiseDataBHeight = RootRef->GetGroundNoise((PointPosition + Bitangent * 100).ToFVector(), NoiseVolumeListVar, VolumeListVar, VolumeTransformArray).Height;
				PNA = PointPosition + Tangent * 100;
				PNB = PointPosition + Bitangent * 100;
				if (!bFlatWorld) 
				{
					PNB.Normalize();
					PNA.Normalize();

					PNA = PNA * (PlanetScaleCode + NoiseDataAHeight);
					PNB = PNB * (PlanetScaleCode + NoiseDataBHeight);
				}
				else 
				{
					PNA = DVector(PNA.X, PNA.Y, NoiseDataAHeight);
					PNB = DVector(PNB.X, PNB.Y, NoiseDataBHeight);
				}

				DVector GroundNormal = DVector::CrossProduct(PNA - PointPosition, PNB - PointPosition);
				GroundNormal.Normalize();
				if (ActiveFoliage->bAllignedToGround) 
				{
					float LerpValue = (ActiveFoliage->GroundRotationInfluenceMax - ActiveFoliage->GroundRotationInfluenceMin) * Stream.FRand() + ActiveFoliage->GroundRotationInfluenceMin;
					FoliageNormal = FMath::Lerp(Normal.ToFVector(), GroundNormal.ToFVector(), LerpValue);
				}
				else
					FoliageNormal = Normal.ToFVector();

				float Slope = 1 - FMath::Clamp(FVector::DotProduct(Normal.ToFVector(), GroundNormal.ToFVector()), 0.f, 1.f);
				//Slope Contraint
				if (ActiveFoliage->bOverrideCollectionContraint)
					Contraint = (Slope <= ActiveFoliage->MaxSlope && Slope >= ActiveFoliage->MinSlope
						);
				else
					Contraint = (Slope <= ActiveFoliageCollection->MaxSlope && Slope >= ActiveFoliageCollection->MinSlope
						);


				if (!Contraint)
				{
					continue;
				}


				float scale = (ActiveFoliage->MaxScale - ActiveFoliage->MinScale) * Stream.FRand() + ActiveFoliage->MinScale;

				FTransform transform;
				FQuat Rotation;
				FoliageNormal.Normalize();
				float angle = 0;

				if (ActiveFoliage->bAllignedToGround)
				{
					Rotation = UKismetMathLibrary::MakeRotFromZ(FoliageNormal).Quaternion();

					if (ActiveFoliage->bRandomRotation) {
						Rotation*= FRotator(0, Stream.FRand() * 360.f, 0).Quaternion();
					}

				}
				else if (ActiveFoliage->bRandomRotation) {
					//angle = Stream.FRand() * 360.f;
					Rotation = UKismetMathLibrary::RandomRotatorFromStream(true, Stream).Quaternion();
				}
				Rotation.Normalize();

				//Rotation.ToSwingTwist(FoliageNormal, 0, angle);

				//For some reason the rotation is allways set up :/
				// Rotation = FQuat(Normal, FMath::DegreesToRadians(angle));


				FVector Offset = ActiveFoliage->Offset;
				Offset = Rotation.RotateVector(Offset);



				transform.SetLocation((PointPosition + DVector(Offset)).ToFVector());

				transform.SetRotation(Rotation);
				transform.SetScale3D(FVector(scale));

				Sector.VegetationTransform.Add(transform);
				Sector.VegetationPosition.Add(PointPosition + DVector(Offset));
			}
		}
	}
	//delete Stream;
}


bool ListCountainSector(TArray<FFoliageSector>* Array, FFoliageSector item) 
{
	return false;
}

void FoliageGenerationThread::DoWork() 
{
	TArray<FFoliageSector> FoliageSectorList;

	if (!IsValid(RootRef))
	{
		return;
	}
		

	for (int32 i = 0; i < VegetationList.Num(); i++) 
	{
		bool TooHigh = true;

		int32 FCI = VegetationList[i].FoliageCollectionIndex;
		
		int32 FI = VegetationList[i].FoliageIndex;
		if (IsValid(RootRef->Foliages[FCI])) 
		{
			if (RootRef->Foliages[FCI]->FoliageList.IsValidIndex(FI) && IsValid(RootRef->Foliages[FCI]->FoliageList[FI])) 
			{
				FoliageSectorList = RootRef->GetSurroundingFoliageSector(PlayerPos, RootRef->Foliages[FCI]->FoliageList[FI]->FoliageSectorSize);

				if (PlayerDistanceToGround < 4 * RootRef->Foliages[FCI]->FoliageList[FI]->FoliageSectorSize) 
				{
					TooHigh = false;
				}

				//Skip Foliage Generation for server if there is no collisions
				if (RootRef->GetNetMode() == ENetMode::NM_DedicatedServer && !RootRef->Foliages[FCI]->FoliageList[FI]->bCollision) 
				{
					continue;
				}
			}
		}
		//Skip Foliage Generation if altitude is too high;
		if (TooHigh)
			continue;

		RootRef->FoliageLeft = VegetationList.Num() - i;

		for (int32 j = 0; j < FoliageSectorList.Num(); j++) 
		{


			RootRef->FoliageSectorLeft = FoliageSectorList.Num() - j;

			bool SectorAllreadyExist = false;

			for (int32 k = 0; k < VegetationList[i].ActiveFoliageSector.Num(); k++)
			{
				if (VegetationList[i].ActiveFoliageSector[k] == FoliageSectorList[j]) 
				{
					SectorAllreadyExist = true;
					break;
				}
			}

			if (SectorAllreadyExist || !FoliageSectorList.IsValidIndex(j))
			{
				continue;
			}

			GenerateVegetationForSector(FoliageSectorList[j], VegetationList[i].FoliageCollectionIndex, VegetationList[i].FoliageIndex, i);
			VegetationList[i].ActiveFoliageSector.Add(FoliageSectorList[j]);
		}
	}

	if (!IsValid(RootRef)) {
		return;
	}
	RootRef->FoliageCountLeft = 0;
	RootRef->FoliageSectorLeft = 0;
	RootRef->FoliageLeft = 0;
	RootRef->FoliageDataList = VegetationList;
	RootRef->FoliageGenerationDone = true;
}

FoliageGenerationThread::FoliageGenerationThread(AWorldScapeRoot* pRoot, double pPlanetScale, bool pFlatWorld, DVector pPlayerPosition, TArray< FVegetation> pVegetationList, double pPlayerDistanceToGround) 
{
	RootRef = pRoot;
	//PlanetPos  pPlanetLocation
	PlanetScaleCode = pPlanetScale;
	bFlatWorld = pFlatWorld;
	PlayerPos = pPlayerPosition;
	VegetationList = pVegetationList;
	PlayerDistanceToGround = pPlayerDistanceToGround;
}