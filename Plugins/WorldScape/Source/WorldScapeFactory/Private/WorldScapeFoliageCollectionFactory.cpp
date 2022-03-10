// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include <WorldScapeFoliageCollectionFactory.h>


UWorldScapeFoliageCollectionFactory::UWorldScapeFoliageCollectionFactory()
{
	// Provide the factory with information about how to handle our asset
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UWorldScapeFoliageCollection::StaticClass();
}

UObject* UWorldScapeFoliageCollectionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Create and return a new instance of our MyCustomAsset object
	UWorldScapeFoliageCollection* CreatedAsset = NewObject<UWorldScapeFoliageCollection>(InParent, Class, Name, Flags);
	return CreatedAsset;
}