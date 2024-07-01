// Copyright Epic Games, Inc. All Rights Reserved.

#include "ApexLegendsMaterial.h"

#include "ToolMenus.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"

#include "ApexLegendsMaterialStyle.h"
#include "ApexLegendsMaterialCommands.h"

#define LOCTEXT_NAMESPACE "FApexLegendsMaterialModule"

void FApexLegendsMaterialModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    FApexLegendsMaterialStyle::Initialize();
    FApexLegendsMaterialStyle::ReloadTextures();

    FApexLegendsMaterialCommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
        FApexLegendsMaterialCommands::Get().PluginAction,
        FExecuteAction::CreateRaw(this, &FApexLegendsMaterialModule::PluginButtonClicked),
        FCanExecuteAction());

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FApexLegendsMaterialModule::RegisterMenus));
    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FApexLegendsMaterialModule::LoadUtilityBlueprint));
}

void FApexLegendsMaterialModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
    UToolMenus::UnRegisterStartupCallback(this);

    UToolMenus::UnregisterOwner(this);

    FApexLegendsMaterialStyle::Shutdown();

    FApexLegendsMaterialCommands::Unregister();
}

void FApexLegendsMaterialModule::RegisterMenus()
{
    // Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
    FToolMenuOwnerScoped OwnerScoped(this);

    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        {
            FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
            Section.AddMenuEntryWithCommandList(FApexLegendsMaterialCommands::Get().PluginAction, PluginCommands);
        }
    }

    {
        UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
        {
            FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("ApexLegendsTool");
            {
                FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FApexLegendsMaterialCommands::Get().PluginAction));
                Entry.SetCommandList(PluginCommands);
            }
        }
    }
}

void FApexLegendsMaterialModule::PluginButtonClicked()
{
    const FString Path = "/ApexLegendsMaterial/Util/ApexLegendsTool";
    UEditorUtilityWidgetBlueprint* EUW = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, *Path);
    if (EUW)
    {
        UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
        EditorUtilitySubsystem->SpawnAndRegisterTab(EUW);
    }
}

void FApexLegendsMaterialModule::LoadUtilityBlueprint()
{
    // Load Util
    const FString Path_ATM = "/ApexLegendsMaterial/Util/BP_AutoTextureMapping";
    const FString Path_AM = "/ApexLegendsMaterial/Util/BP_AnimModifier";
    StaticLoadObject(UObject::StaticClass(), nullptr, *Path_ATM);
    StaticLoadObject(UObject::StaticClass(), nullptr, *Path_AM);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FApexLegendsMaterialModule, ApexLegendsMaterial)