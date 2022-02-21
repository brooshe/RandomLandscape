// Copyright 2021 VICTOR HERNANDEZ MOLPECERES (Rockam). All rights reserved.

#include "PTG_EditorDetails.h"
#include "RuntimeMeshComponent.h"
#include "RuntimeMesh.h"
#include "RuntimeMeshProvider.h"
#include "PtgManager.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "RawMesh.h"
#include "ImageUtils.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "PhysicsEngine/BodySetup.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "PTG_Details"

TSharedRef<IDetailCustomization> FPTG_EditorDetails::MakeInstance()
{
	return MakeShareable(new FPTG_EditorDetails);
}

void FPTG_EditorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("PTG - Editor actions");

	const FText ConvertToStaticMeshText = LOCTEXT("ConvertToStaticMesh", "Convert Terrain to Static Mesh");
	const FText CreateTerrainHeightmapText = LOCTEXT("CreateTerrainHeightmap", "Create Terrain Heightmap");

	// Cache set of selected things
	SelectedObjectsList = DetailBuilder.GetDetailsView()->GetSelectedObjects();

	// Add the Convert Terrain to StaticMesh button
	category.AddCustomRow(ConvertToStaticMeshText, false)
		.NameContent()
		[
			SNullWidget::NullWidget
		]
		.ValueContent()
		.VAlign(VAlign_Center)
		.HAlign(EHorizontalAlignment::HAlign_Fill)
		.MaxDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.ToolTipText(LOCTEXT("ConvertToStaticMeshTooltip", "Create a new Static Mesh asset using current geometry from generated terrain mesh. Does not modify instance."))
			.OnClicked(this, &FPTG_EditorDetails::ClickedOnConvertToStaticMesh)
			.IsEnabled(this, &FPTG_EditorDetails::ConvertToStaticMeshEnabled)
			.Content()
			[
				SNew(STextBlock)
				.Text(ConvertToStaticMeshText)
			]
		];

	{
		// Add all the default properties
		TArray<TSharedRef<IPropertyHandle>> AllProperties;
		const bool bSimpleProperties = true;
		const bool bAdvancedProperties = false;

		// Add all properties in the category in order
		category.GetDefaultProperties(AllProperties, bSimpleProperties, bAdvancedProperties);
		for (auto& Property : AllProperties)
		{
			category.AddProperty(Property);
		}
	}

	// Add the Create Terrain Heightmap button
	category.AddCustomRow(ConvertToStaticMeshText, false)
		.NameContent()
		[
			SNullWidget::NullWidget
		]
		.ValueContent()
		.VAlign(VAlign_Center)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
		.MaxDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.ToolTipText(LOCTEXT("CreateTerrainHeightmapTooltip", "Create a new .png image using current vertex data from the generated terrain mesh. Only available on Plane terrain shape."))
			.OnClicked(this, &FPTG_EditorDetails::ClickedOnCreateTerrainHeightmap)
			.IsEnabled(this, &FPTG_EditorDetails::CreateTerrainHeightmapEnabled)
			.Content()
			[
				SNew(STextBlock)
				.Text(CreateTerrainHeightmapText)
			]
		];

	{
		// Add all the default properties
		TArray<TSharedRef<IPropertyHandle>> AllProperties;
		const bool bSimpleProperties = true;
		const bool bAdvancedProperties = false;

		// Add all properties in the category in order
		category.GetDefaultProperties(AllProperties, bSimpleProperties, bAdvancedProperties);
		for (auto& Property : AllProperties)
		{
			category.AddProperty(Property);
		}
	}
}

APtgManager* FPTG_EditorDetails::GetProceduralTerrain() const
{
	APtgManager* proceduralTerrain = nullptr;

	// Find procedural terrain
	for (const TWeakObjectPtr<UObject>& Object : SelectedObjectsList)
	{
		proceduralTerrain = Cast<APtgManager>(Object.Get());

		// See if this one is good.
		if (proceduralTerrain != nullptr && !proceduralTerrain->IsTemplate())
		{
			return proceduralTerrain;
		}
	}

	return nullptr;
}

bool FPTG_EditorDetails::ConvertToStaticMeshEnabled() const
{
	const APtgManager* proceduralTerrain = GetProceduralTerrain();

	return proceduralTerrain != nullptr && proceduralTerrain->GetProcMeshTerrainComp() != nullptr;
}

bool FPTG_EditorDetails::CreateTerrainHeightmapEnabled() const
{
	const APtgManager* proceduralTerrain = GetProceduralTerrain();

	return proceduralTerrain != nullptr && proceduralTerrain->GetShape() == EPtgProcMeshShapes::Plane;
}

FReply FPTG_EditorDetails::ClickedOnConvertToStaticMesh()
{
	// Find procedural terrain terrain RuntimeMeshComp
	const APtgManager* proceduralTerrain = GetProceduralTerrain();
	URuntimeMeshComponent* runtimeMeshComp = IsValid(proceduralTerrain) ? proceduralTerrain->GetProcMeshTerrainComp() : nullptr;
	UPtgRuntimeMesh* runtimeMesh = (runtimeMeshComp != nullptr) ? runtimeMeshComp->GetRuntimeMesh() : nullptr;
	URuntimeMeshProvider* meshProvider = (runtimeMesh != nullptr) ? runtimeMesh->GetProviderPtr() : nullptr;

	if (runtimeMeshComp != nullptr && runtimeMesh != nullptr && meshProvider != nullptr)
	{
		const FString NewNameSuggestion = FString(TEXT("PTG_Mesh"));
		FString PackageName = FString(TEXT("/Game/PTG_Converted_Meshes/")) + NewNameSuggestion;
		FString Name;
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().CreateUniqueAssetName(PackageName, TEXT(""), PackageName, Name);

		TSharedPtr<SDlgPickAssetPath> PickAssetPathWidget =
			SNew(SDlgPickAssetPath)
			.Title(LOCTEXT("ConvertToStaticMeshPickName", "Choose new StaticMesh location"))
			.DefaultAssetPath(FText::FromString(PackageName));

		if (PickAssetPathWidget->ShowModal() == EAppReturnType::Ok)
		{
			// Get the full name of where we want to create the physics asset.
			FString UserPackageName = PickAssetPathWidget->GetFullAssetPath().ToString();
			FName MeshName(*FPackageName::GetLongPackageAssetName(UserPackageName));

			// Check if the user inputed a valid asset name, if they did not, give it the generated default name
			if (MeshName == NAME_None)
			{
				// Use the defaults that were already generated.
				UserPackageName = PackageName;
				MeshName = *Name;
			}

			// Create the package to save the static mesh
			UPackage* Package = CreatePackage(*UserPackageName);
			check(Package);

			// Create StaticMesh object
			UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, MeshName, RF_Public | RF_Standalone);
			StaticMesh->InitResources();

#if ENGINE_MINOR_VERSION <= 26
			StaticMesh->LightingGuid = FGuid::NewGuid();
#else
			StaticMesh->SetLightingGuid(FGuid::NewGuid());
#endif

			// Copy the material slots
#if ENGINE_MINOR_VERSION <= 26
			TArray<FStaticMaterial>& Materials = StaticMesh->StaticMaterials;
#else
			TArray<FStaticMaterial>& Materials = StaticMesh->GetStaticMaterials();
#endif
			const auto RMCMaterialSlots = runtimeMesh->GetMaterialSlots();
			Materials.SetNum(RMCMaterialSlots.Num());
			for (int32 Index = 0; Index < RMCMaterialSlots.Num(); Index++)
			{
				UMaterialInterface* Mat = runtimeMeshComp->OverrideMaterials.Num() > Index ? runtimeMeshComp->OverrideMaterials[Index] : nullptr;
				Mat = Mat ? Mat : RMCMaterialSlots[Index].Material;
				Materials[Index] = FStaticMaterial(Mat, RMCMaterialSlots[Index].SlotName);
			}

			const auto LODConfig = runtimeMesh->GetCopyOfConfiguration();

			for (int32 LODIndex = 0; LODIndex < LODConfig.Num(); LODIndex++)
			{
				const auto& LOD = LODConfig[LODIndex];

				// Raw mesh data we are filling in
				FRawMesh RawMesh;
				bool bUseHighPrecisionTangents = false;
				bool bUseFullPrecisionUVs = false;
				int32 MaxUVs = 1;

				int32 VertexBase = 0;

				for (const auto& SectionEntry : LOD.Sections)
				{
					const int32 SectionId = SectionEntry.Key;
					const auto& Section = SectionEntry.Value;

					// Here we need to direct query the provider the mesh is using
					// We also go ahead and use high precision tangents/uvs so we don't loose 
					// quality passing through the build pipeline after quantizing it once
					FRuntimeMeshRenderableMeshData MeshData(true, true, Section.NumTexCoords, true);
					if (meshProvider->GetSectionMeshForLOD(LODIndex, SectionEntry.Key, MeshData))
					{
						MaxUVs = FMath::Max<int32>(MaxUVs, Section.NumTexCoords);

						// Fill out existing UV channels to start of this one
						for (int32 Index = 0; Index < MaxUVs; Index++)
						{
							RawMesh.WedgeTexCoords[Index].SetNumZeroed(RawMesh.WedgeIndices.Num());
						}

						// Copy the vertex positions
						int32 NumVertices = MeshData.Positions.Num();
						for (int32 Index = 0; Index < NumVertices; Index++)
						{
							RawMesh.VertexPositions.Add(MeshData.Positions.GetPosition(Index));
						}

						// Copy wedges
						int32 NumTris = MeshData.Triangles.Num();
						for (int32 Index = 0; Index < NumTris; Index++)
						{
							int32 VertexIndex = MeshData.Triangles.GetVertexIndex(Index);
							RawMesh.WedgeIndices.Add(VertexIndex + VertexBase);

							FVector TangentX, TangentY, TangentZ;
							MeshData.Tangents.GetTangents(VertexIndex, TangentX, TangentY, TangentZ);
							RawMesh.WedgeTangentX.Add(TangentX);
							RawMesh.WedgeTangentY.Add(TangentY);
							RawMesh.WedgeTangentZ.Add(TangentZ);

							for (int32 UVIndex = 0; UVIndex < Section.NumTexCoords; UVIndex++)
							{
								RawMesh.WedgeTexCoords[UVIndex].Add(MeshData.TexCoords.GetTexCoord(VertexIndex, UVIndex));
							}

							RawMesh.WedgeColors.Add(MeshData.Colors.GetColor(VertexIndex));
						}

						// Copy face info
						for (int32 TriIdx = 0; TriIdx < NumTris / 3; TriIdx++)
						{
							// Set the face material
							RawMesh.FaceMaterialIndices.Add(SectionId);
							RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
						}

						// Update offset for creating one big index/vertex buffer
						VertexBase += NumVertices;
					}
				}

				// Fill out the UV channels to the same length as the indices
				for (int32 Index = 0; Index < MaxUVs; Index++)
				{
					RawMesh.WedgeTexCoords[Index].SetNumZeroed(RawMesh.WedgeIndices.Num());
				}

				// If we got some valid data.
				if (RawMesh.IsValid())
				{
					// Add source to new StaticMesh
					FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
					SrcModel.BuildSettings.bRecomputeNormals = false;
					SrcModel.BuildSettings.bRecomputeTangents = false;
					SrcModel.BuildSettings.bRemoveDegenerates = true;
					SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = bUseHighPrecisionTangents;
					SrcModel.BuildSettings.bUseFullPrecisionUVs = bUseFullPrecisionUVs;
					SrcModel.BuildSettings.bGenerateLightmapUVs = true;
					SrcModel.BuildSettings.SrcLightmapIndex = 0;
					SrcModel.BuildSettings.DstLightmapIndex = 1;
					SrcModel.RawMeshBulkData->SaveRawMesh(RawMesh);

					SrcModel.ScreenSize = LOD.Properties.ScreenSize;

					// Set the materials used for this static mesh
#if ENGINE_MINOR_VERSION <= 26
					int32 NumMaterials = StaticMesh->StaticMaterials.Num();
#else
					int32 NumMaterials = StaticMesh->GetStaticMaterials().Num();
#endif
					

					// Set up the SectionInfoMap to enable collision
					for (int32 SectionIdx = 0; SectionIdx < NumMaterials; SectionIdx++)
					{
						FMeshSectionInfoMap& SectionInfoMap = StaticMesh->GetSectionInfoMap();
						FMeshSectionInfo Info = SectionInfoMap.Get(LODIndex, SectionIdx);
						Info.MaterialIndex = SectionIdx;
						// TODO: Is this the correct way to handle this by just turning on collision in the top level LOD?
						Info.bEnableCollision = LODIndex == 0;
						SectionInfoMap.Set(LODIndex, SectionIdx, Info);
					}
				}
			}

#if ENGINE_MINOR_VERSION <= 26
			StaticMesh->StaticMaterials = Materials;
#else
			StaticMesh->SetStaticMaterials(Materials);
#endif
			

			// Configure body setup for working collision.
			StaticMesh->CreateBodySetup();
#if ENGINE_MINOR_VERSION <= 26
			StaticMesh->BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
#else
			StaticMesh->GetBodySetup()->CollisionTraceFlag = CTF_UseComplexAsSimple;
#endif

			// Build mesh from source
			StaticMesh->Build(false);

			// Make package dirty.
			StaticMesh->MarkPackageDirty();

			StaticMesh->PostEditChange();

			// Notify asset registry of new asset
			FAssetRegistryModule::AssetCreated(StaticMesh);
		}
	}

	return FReply::Handled();
}

FReply FPTG_EditorDetails::ClickedOnCreateTerrainHeightmap()
{
	if (const APtgManager* proceduralTerrain = GetProceduralTerrain())
	{
		const TArray<float> vertexHeightData = proceduralTerrain->GetVertexHeightData();
		const int32 numVertices = vertexHeightData.Num();

		if (numVertices > 0)
		{
			const int32 resolution = FMath::Sqrt((float)numVertices);
			const FString windowTitle = FString(TEXT("Choose new Heightmap location"));
			const FString suggestedLocation = FString(TEXT(""));
			const FString newNameSuggestion = FString(TEXT("PTG_Heightmap.png"));
			const FString format = FString(TEXT("png Image|*.png"));
			TArray<FString> filenames;

			if (FDesktopPlatformModule::Get()->SaveFileDialog(nullptr, windowTitle, suggestedLocation, newNameSuggestion, format, 0, filenames))
			{
				// Fill pixels with vertex data
				const FVector2D heightRange = FVector2D(proceduralTerrain->GetLowestGeneratedHeight(), proceduralTerrain->GetHighestGeneratedHeight());
				const FVector2D colorRange = FVector2D(0.0f, 1.0f);
				const int32 pixelsSize = numVertices * 4;	// 4 -> RGBA
				TArray<FColor> pixels;

				for (const float currentHeight : vertexHeightData)
				{
					const float rgbColor = FMath::GetMappedRangeValueClamped(heightRange, colorRange, currentHeight);

					pixels.Emplace(FLinearColor(rgbColor, rgbColor, rgbColor).ToFColor(true));
				}

				// Save heightmap values to png image
				SaveImage(filenames[0], resolution, pixels);
			}
		}
	}

	return FReply::Handled();
}

bool FPTG_EditorDetails::SaveImage(const FString& fullFilePath, int32 resolution, const TArray<FColor>& imagePixels)
{
	FString errorString = TEXT("Success creating heightmap!");

	if (fullFilePath.Len() < 1)
	{
		errorString = TEXT("Error in SaveImage: no file path");
		return false;
	}

	// Ensure target directory exists or can be created
	FString newAbsoluteFolderPath = FPaths::GetPath(fullFilePath);
	FPaths::NormalizeDirectoryName(newAbsoluteFolderPath);

	if (!CreateDirectory(newAbsoluteFolderPath))
	{
		errorString = TEXT("Error in SaveImage: folder could not be created, check read/write permissions~ ") + newAbsoluteFolderPath;
		return false;
	}

	if (imagePixels.Num() != resolution * resolution)
	{
		errorString = TEXT("Error in SaveImage: resolution x resolution is not equal to the total pixel array length!");
		return false;
	}

	// Remove any supplied file extension and/or add accurate one
	FString FinalFilename = FPaths::GetBaseFilename(fullFilePath, false) + TEXT(".png");
	TArray<uint8> CompressedPNG;

	FImageUtils::CompressImageArray(resolution, resolution, imagePixels, CompressedPNG);

	bool saveResult = FFileHelper::SaveArrayToFile(CompressedPNG, *FinalFilename);

	if (!saveResult)
	{
		errorString = TEXT("Error in SaveImage: saving of file to disk did not succeed for File IO reasons");
	}

	if (GEngine != nullptr) GEngine->AddOnScreenDebugMessage(-1, 5.0f, saveResult ? FColor::Green : FColor::Red, errorString);

	return saveResult;
}

bool FPTG_EditorDetails::CreateDirectory(FString FolderToMake)
{
	if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*FolderToMake))
	{
		return true;
	}

	// Normalize all / and \ to TEXT("/") and remove any trailing TEXT("/") if the character before that is not a TEXT("/") or a colon
	FPaths::NormalizeDirectoryName(FolderToMake);

	// Normalize removes the last "/", but is needed by algorithm. Guarantees While loop will end in a timely fashion.
	FolderToMake += "/";

	FString Base;
	FString Left;
	FString Remaining;

	// Split off the Root
	FolderToMake.Split(TEXT("/"), &Base, &Remaining);
	Base += "/"; // add root text and Split Text to Base

	while (Remaining != "")
	{
		Remaining.Split(TEXT("/"), &Left, &Remaining);

		// Add to the Base
		Base += Left + FString("/"); // add left and split text to Base

		// Create Incremental Directory Structure!
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*Base) &&
			!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*Base))
		{
			return false;
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
