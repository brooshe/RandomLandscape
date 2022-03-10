// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "HIDEditorCommands.h"

#define LOCTEXT_NAMESPACE "HIDEditorCommands"


FHIDEditorCommands::FHIDEditorCommands()
	: TCommands<FHIDEditorCommands>("HIDEditor", LOCTEXT("General", "General"), NAME_None, FEditorStyle::GetStyleSetName())
{
}


void FHIDEditorCommands::RegisterCommands()
{
	UI_COMMAND(Generate, "Generate", "Generate The HeightMap Volume Data.", EUserInterfaceActionType::Button, FInputGesture(EModifierKey::Control, EKeys::G));
	UI_COMMAND(CleanData, "CleanData", "Clean all generated data, Usefull to edit property without freeze .", EUserInterfaceActionType::Button, FInputGesture(EModifierKey::Control, EKeys::C));
}


#undef LOCTEXT_NAMESPACE