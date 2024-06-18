// Copyright Epic Games, Inc. All Rights Reserved.

#include "ApexLegendsMaterialCommands.h"

#define LOCTEXT_NAMESPACE "FApexLegendsMaterialModule"

void FApexLegendsMaterialCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Apex Legends Tool", "Open Apex Legends Tool Widget", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
