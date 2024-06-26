// Copyright Epic Games, Inc. All Rights Reserved.

#include "ApexLegendsMaterialStyle.h"
#include "ApexLegendsMaterial.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FApexLegendsMaterialStyle::StyleInstance = nullptr;

void FApexLegendsMaterialStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FApexLegendsMaterialStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FApexLegendsMaterialStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ApexLegendsMaterialStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FApexLegendsMaterialStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ApexLegendsMaterialStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ApexLegendsMaterial")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ApexLegendsMaterial.PluginAction", new IMAGE_BRUSH(TEXT("Icon"), Icon20x20));
	return Style;
}

void FApexLegendsMaterialStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FApexLegendsMaterialStyle::Get()
{
	return *StyleInstance;
}
