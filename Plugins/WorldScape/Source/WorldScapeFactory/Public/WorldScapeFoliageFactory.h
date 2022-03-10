// Copyright 2021 IOLACORP STUDIO. All Rights Reserved

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetTypeCategories.h"
#include "UnrealEd.h"
#include <WorldScapeCore/Public/WorldScapeFoliage.h>

#include "WorldScapeFoliageFactory.generated.h"

UCLASS()
class WORLDSCAPEFACTORY_API UWorldScapeFoliageFactory : public UFactory
{
	GENERATED_BODY()

public:
	UWorldScapeFoliageFactory();
	// Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End UFactory Interface
};