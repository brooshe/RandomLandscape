// Copyright 2021 VICTOR HERNANDEZ MOLPECERES (Rockam). All rights reserved.

#include "ProceduralTerrainGeneratorEditor.h"
#include "PropertyEditorModule.h"
#include "PtgManager.h"
#include "PTG_EditorDetails.h"

void FProceduralTerrainGeneratorEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(APtgManager::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FPTG_EditorDetails::MakeInstance));
}

void FProceduralTerrainGeneratorEditorModule::ShutdownModule() {}

IMPLEMENT_MODULE(FProceduralTerrainGeneratorEditorModule, ProceduralTerrainGeneratorEditor)
