// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include <WorldScapeFoliageCollectionTypeAction.h>


FWorldScapeFoliageCollectionTypeAction::FWorldScapeFoliageCollectionTypeAction(EAssetTypeCategories::Type InAssetCategory)
	: WorldScapeAssetCategory(InAssetCategory)
{
}

FText FWorldScapeFoliageCollectionTypeAction::GetName() const
{
	return FText::FromString("WorldScape Foliage Collection");
}

FColor FWorldScapeFoliageCollectionTypeAction::GetTypeColor() const
{
	return FColor(40, 120, 10);
}


UClass* FWorldScapeFoliageCollectionTypeAction::GetSupportedClass() const
{
	return UWorldScapeFoliageCollection::StaticClass();
}

uint32 FWorldScapeFoliageCollectionTypeAction::GetCategories()
{
	return WorldScapeAssetCategory;
}
