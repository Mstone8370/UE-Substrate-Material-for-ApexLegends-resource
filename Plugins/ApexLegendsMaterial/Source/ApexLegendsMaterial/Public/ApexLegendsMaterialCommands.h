// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ApexLegendsMaterialStyle.h"

class FApexLegendsMaterialCommands : public TCommands<FApexLegendsMaterialCommands>
{
public:

	FApexLegendsMaterialCommands()
		: TCommands<FApexLegendsMaterialCommands>(TEXT("ApexLegendsMaterial"), NSLOCTEXT("Contexts", "ApexLegendsMaterial", "ApexLegendsMaterial Plugin"), NAME_None, FApexLegendsMaterialStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
