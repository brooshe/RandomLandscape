// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include <WorldScapeFoliageTypeAction.h>


FWorldScapeFoliageTypeAction::FWorldScapeFoliageTypeAction(EAssetTypeCategories::Type InAssetCategory)
	: WorldScapeAssetCategory(InAssetCategory)
{
}

FText FWorldScapeFoliageTypeAction::GetName() const
{
	return FText::FromString("WorldScape Foliage");
}

FColor FWorldScapeFoliageTypeAction::GetTypeColor() const
{
	return FColor(10, 150, 20);
}


UClass* FWorldScapeFoliageTypeAction::GetSupportedClass() const
{
	return UWorldScapeFoliage::StaticClass();
}

uint32 FWorldScapeFoliageTypeAction::GetCategories()
{
	return WorldScapeAssetCategory;
}
