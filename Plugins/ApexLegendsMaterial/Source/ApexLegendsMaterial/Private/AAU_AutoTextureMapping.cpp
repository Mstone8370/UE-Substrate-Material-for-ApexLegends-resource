// Copyright (c) 2024 Minseok Kim


#include "AAU_AutoTextureMapping.h"

#include "Engine/SkinnedAssetCommon.h"
#include "Materials/MaterialInstanceConstant.h"

#include "Engine/ObjectLibrary.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"

UAAU_AutoTextureMapping::UAAU_AutoTextureMapping()
    : DefaultTextureFolderName(TEXT("Textures"))
    , DefaultMasterMaterialPath(TEXT("/ApexLegendsMaterial/Materials/M_Master"))
    , MasterMaterialOverride(nullptr)
{
    // CustomMaterialMap
    CustomMaterialMap.Empty();

    // TextureTypeToParamName
    TextureTypeToParamName.Empty();

    TextureTypeToParamName.Add(TEXT("albedoTexture"), FName("Albedo"));
    TextureTypeToParamName.Add(TEXT("aoTexture"), FName("AO"));
    TextureTypeToParamName.Add(TEXT("cavityTexture"), FName("Cavity"));
    TextureTypeToParamName.Add(TEXT("emissiveTexture"), FName("Emissive"));
    TextureTypeToParamName.Add(TEXT("glossTexture"), FName("Gloss"));
    TextureTypeToParamName.Add(TEXT("normalTexture"), FName("Normal"));
    TextureTypeToParamName.Add(TEXT("opacityMultiplyTexture"), FName("Opacity"));
    TextureTypeToParamName.Add(TEXT("scatterThicknessTexture"), FName("ScatterThickness"));
    TextureTypeToParamName.Add(TEXT("specTexture"), FName("Specular"));
    TextureTypeToParamName.Add(TEXT("anisoSpecDirTexture"), FName("Anisotropy"));
    TextureTypeToParamName.Add(TEXT("ao"), FName("AO"));
    TextureTypeToParamName.Add(TEXT("col"), FName("Albedo"));
    TextureTypeToParamName.Add(TEXT("cvt"), FName("Cavity"));
    TextureTypeToParamName.Add(TEXT("gls"), FName("Gloss"));
    TextureTypeToParamName.Add(TEXT("ilm"), FName("Emissive"));
    TextureTypeToParamName.Add(TEXT("nml"), FName("Normal"));
    TextureTypeToParamName.Add(TEXT("opa"), FName("Opacity"));
    TextureTypeToParamName.Add(TEXT("spc"), FName("Specular"));

    // LinearTextureTypes
    LinearTextureTypes.Empty();

    LinearTextureTypes.Add(TEXT("aoTexture"));
    LinearTextureTypes.Add(TEXT("opacityMultiplyTexture"));
    LinearTextureTypes.Add(TEXT("cavityTexture"));
    LinearTextureTypes.Add(TEXT("glossTexture"));
    LinearTextureTypes.Add(TEXT("normalTexture"));
    LinearTextureTypes.Add(TEXT("anisoSpecDirTexture"));
    LinearTextureTypes.Add(TEXT("ao"));
    LinearTextureTypes.Add(TEXT("cvt"));
    LinearTextureTypes.Add(TEXT("gls"));
    LinearTextureTypes.Add(TEXT("nml"));
    LinearTextureTypes.Add(TEXT("opa"));
}

void UAAU_AutoTextureMapping::DisconnectAllMaterials()
{
    TArray<FAssetData> SelectedAssetDatas = UEditorUtilityLibrary::GetSelectedAssetData();
    for (FAssetData& SelectedAssetData : SelectedAssetDatas)
    {
        UObject* SelectedObject = SelectedAssetData.GetAsset();
        if (!SelectedObject || !(SelectedObject->IsA<USkeletalMesh>() || SelectedObject->IsA<UStaticMesh>()))
        {
            continue;
        }

        SelectedObject->PreEditChange(nullptr);
        SelectedObject->Modify();

        if (USkeletalMesh* SK = Cast<USkeletalMesh>(SelectedObject))
        {
            TArray<FSkeletalMaterial>& Materials = SK->GetMaterials();
            for (FSkeletalMaterial& Material : Materials)
            {
                Material.MaterialInterface = nullptr;
            }
        }
        if (UStaticMesh* SM = Cast<UStaticMesh>(SelectedObject))
        {
            TArray<FStaticMaterial>& Materials = SM->GetStaticMaterials();
            for (FStaticMaterial& Material : Materials)
            {
                Material.MaterialInterface = nullptr;
            }
        }

        // Update Object
        SelectedObject->PostEditChange();

        // Save Object
        const FString ObjectPath = SelectedObject->GetPathName();
        const FString FilePath = FPaths::GetBaseFilename(ObjectPath, false);
        UEditorAssetLibrary::SaveAsset(FilePath, false);
    }
}

void UAAU_AutoTextureMapping::AutoTextureMapping(FString TextureFolderNameOverride, bool bFlipNormalGreen)
{
    if (!CheckMasterMaterial())
    {
        UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Failed to load Master Material"));
        return;
    }

    // Object filtering
    TSet<UObject*> SelectedObjects;
    TArray<FAssetData> SelectedAssetDatas = UEditorUtilityLibrary::GetSelectedAssetData();
    for (FAssetData& SelectedAssetData : SelectedAssetDatas)
    {
        if (UObject* SelectedObject = SelectedAssetData.GetAsset())
        {
            if (SelectedObject->IsA<USkeletalMesh>() || SelectedObject->IsA<UStaticMesh>())
            {
                SelectedObjects.Add(SelectedObject);
            }
        }
    }
    if (SelectedObjects.IsEmpty())
    {
        return;
    }

    // Set Material Instances
    TMap<FString, TArray<UMaterialInstance*>> MaterialMap;  // key: Material Slot Name, Value: Array of Material Instances
    for (UObject* Obj : SelectedObjects)
    {
        Obj->PreEditChange(nullptr);
        Obj->Modify();

        SetMaterialInstances(Obj, MaterialMap);

        Obj->PostEditChange();

        // Save object
        const FString ObjectPath = Obj->GetPathName();
        const FString FilePath = FPaths::GetBaseFilename(ObjectPath, false);
        UEditorAssetLibrary::SaveAsset(FilePath, false);
    }

    // Gather texture paths
    TSet<FString> TexturePaths;
    const FString TextureFolderName = TextureFolderNameOverride.TrimStartAndEnd().IsEmpty() ? DefaultTextureFolderName : TextureFolderNameOverride;
    GetTexturePaths(SelectedObjects, TextureFolderName, TexturePaths);
    
    // Read Texture and connect to Material Instance
    MapTexturesToMaterial(MaterialMap, TexturePaths, bFlipNormalGreen);
}

bool UAAU_AutoTextureMapping::CheckMasterMaterial()
{
    // Check Master Material
    if (!MasterMaterial)
    {
        if (MasterMaterialOverride)
        {
            MasterMaterial = MasterMaterialOverride;
        }
        else
        {
            MasterMaterial = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(DefaultMasterMaterialPath));
        }
    }
    return MasterMaterial != nullptr;
}

void UAAU_AutoTextureMapping::SetMaterialInstances(UObject* MeshObject, TMap<FString, TArray<UMaterialInstance*>>& OutMaterialMap)
{
    if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(MeshObject))
    {
        SetMaterialInstances_Generic(SkeletalMesh, OutMaterialMap, &USkeletalMesh::GetMaterials);
    }
    else if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshObject))
    {
        SetMaterialInstances_Generic(StaticMesh, OutMaterialMap, &UStaticMesh::GetStaticMaterials);
    }
}

UMaterialInstance* UAAU_AutoTextureMapping::CastOrFindOrCreateMaterialInstance(UMaterialInterface*& MaterialInterface, const FString& BasePath, const FString& MaterialSlotName, UMaterialInterface* ParentMaterial)
{
    // Try to cast
    UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(MaterialInterface);
    if (MaterialInstance)
    {
        return MaterialInstance;
    }

    // Try to find existing Material Instance
    const FString NewMaterialInstanceName = FString("MI_") + MaterialSlotName;
    const FString MaterialInstanceFullPath = FPaths::ConvertRelativePathToFull(BasePath, NewMaterialInstanceName);
    if (UEditorAssetLibrary::DoesAssetExist(MaterialInstanceFullPath))
    {
        MaterialInstance = Cast<UMaterialInstance>(UEditorAssetLibrary::LoadAsset(MaterialInstanceFullPath));
    }

    // Create new
    if (!MaterialInstance)
    {
        MaterialInstance = CreateMaterialInstance(ParentMaterial, MaterialInstanceFullPath);
        if (!MaterialInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Failed to create Material Instance: %s"), *NewMaterialInstanceName);
        }
    }

    MaterialInstance->PreEditChange(nullptr);
    MaterialInstance->Modify();

    return MaterialInstance;
}

UMaterialInstanceConstant* UAAU_AutoTextureMapping::CreateMaterialInstance(UMaterialInterface* ParentMaterial, FString FullPath)
{
    UPackage* Package = CreatePackage(*FullPath);
    Package->FullyLoad();

    UMaterialInstanceConstant* MaterialInstanceAsset = NewObject<UMaterialInstanceConstant>(
        Package,
        *FPaths::GetCleanFilename(FullPath),
        RF_Public | RF_Standalone | RF_MarkAsRootSet
    );
    MaterialInstanceAsset->Parent = ParentMaterial;

    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(MaterialInstanceAsset);

    FString PackageFileName = FPackageName::LongPackageNameToFilename(
        FullPath,
        FPackageName::GetAssetPackageExtension()
    );

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
    SaveArgs.Error = GError;
    SaveArgs.bForceByteSwapping = true;
    SaveArgs.bWarnOfLongFilename = true;
    SaveArgs.SaveFlags = SAVE_NoError;

    bool bSaved = UPackage::SavePackage(
        Package,
        MaterialInstanceAsset,
        *PackageFileName,
        SaveArgs
    );
    return MaterialInstanceAsset;
}

void UAAU_AutoTextureMapping::GetTexturePaths(TSet<UObject*> Objects, const FString& TextureFolderName, TSet<FString>& OutPaths)
{
    UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(nullptr, false, GIsEditor);
    ObjectLibrary->bRecursivePaths = true;
    ObjectLibrary->ObjectBaseClass = UTexture2D::StaticClass();

    for (UObject* Obj : Objects)
    {
        const FString ObjectPath = Obj->GetPathName();
        const FString ObjectFolderPath = FPaths::GetPath(ObjectPath);
        const FString TextureFolderPath = FPaths::ConvertRelativePathToFull(ObjectFolderPath, TextureFolderName);

        ObjectLibrary->LoadAssetDataFromPath(TextureFolderPath);

        TArray<FAssetData> TextureAssetDatas;
        ObjectLibrary->GetAssetDataList(TextureAssetDatas);

        for (FAssetData& TextureAssetData : TextureAssetDatas)
        {
            const FString TextureObjectPath = TextureAssetData.GetObjectPathString();
            const FString TextureFilePath = FPaths::GetBaseFilename(TextureObjectPath, false);
            OutPaths.Add(TextureFilePath);
        }
    }
}

void UAAU_AutoTextureMapping::MapTexturesToMaterial(TMap<FString, TArray<UMaterialInstance*>>& InMaterialMap, TSet<FString>& InTexturePaths, bool bFlipNormalGreen)
{
    for (const FString& TexturePath : InTexturePaths)
    {
        // Cleanup Texture file name
        FString TextureName = FPaths::GetBaseFilename(TexturePath);
        if (TextureName.Len() < 1)
        {
            continue;
        }
        if (TextureName.StartsWith(TEXT("T_")))
        {
            TextureName = TextureName.RightChop(2);
        }

        // Get Material name and Texture type from Texture file name
        FString MaterialSlotName = TextureName;
        FString TextureType = "";
        int32 DelimeterIndex;
        if (TextureName.FindLastChar('_', DelimeterIndex))
        {
            MaterialSlotName = TextureName.Left(DelimeterIndex);
            TextureType = TextureName.RightChop(DelimeterIndex + 1);
        }

        // Check if MaterialSlotName exists
        if (!InMaterialMap.Contains(MaterialSlotName))
        {
            UE_LOG(LogTemp, Warning, TEXT("[AutoTextureMapping] Material Not Found: %s"), *MaterialSlotName);
            continue;
        }

        // Load Texture
        UTexture2D* Texture = Cast<UTexture2D>(UEditorAssetLibrary::LoadAsset(TexturePath));
        if (LinearTextureTypes.Contains(TextureType))
        {
            Texture->PreEditChange(nullptr);
            Texture->Modify();

            Texture->SRGB = 0;
            if (TextureType == TEXT("normalTexture") || TextureType == TEXT("nml"))
            {
                Texture->LODGroup = TextureGroup::TEXTUREGROUP_WorldNormalMap;
                Texture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
                Texture->bFlipGreenChannel = bFlipNormalGreen;
            }
            if (TextureType == TEXT("anisoSpecDirTexture"))
            {
                Texture->Filter = TextureFilter::TF_Nearest;
                Texture->CompressionSettings = TextureCompressionSettings::TC_BC7;
            }

            // Update and save texture
            Texture->PostEditChange();
            Texture->UpdateResource();
            UEditorAssetLibrary::SaveAsset(TexturePath, false);
        }

        // Set Material Instance Texture Parameter
        FName* ParamName = TextureTypeToParamName.Find(TextureType);
        if (!ParamName)
        {
            UE_LOG(LogTemp, Warning, TEXT("[AutoTextureMapping] Unknown Texture Type: %s"), *TexturePath);
            continue;
        }

        TArray<UMaterialInstance*> MaterialInstances = *InMaterialMap.Find(MaterialSlotName); // The existence of MaterialSlotName has already been checked above
        for (UMaterialInstance* TargetMaterialInstance : MaterialInstances)
        {
            if (ParamName->IsEqual(FName("Opacity")))
            {
                SetMaterialParamValue(TargetMaterialInstance, FName("AlbedoAlphaAsOpacityMask"), FMaterialParameterValue(false));
            }
            if (ParamName->IsEqual(FName("ScatterThickness")))
            {
                SetMaterialParamValue(TargetMaterialInstance, FName("Subsurface"), FMaterialParameterValue(true));
            }
            SetMaterialParamValue(TargetMaterialInstance, *ParamName, FMaterialParameterValue(Texture));
        }
    }

    // Save Material Instances
    for (TPair<FString, TArray<UMaterialInstance*>>& Item : InMaterialMap)
    {
        for (UMaterialInstance* MatInst : Item.Value)
        {
            MatInst->PostEditChange();
            FString PathName = MatInst->GetPathName();
            UEditorAssetLibrary::SaveAsset(FPaths::GetBaseFilename(PathName, false), false);
        }
    }
}

void UAAU_AutoTextureMapping::SetMaterialParamValue(UMaterialInstance* MatInst, const FName& ParamName, FMaterialParameterValue ParamValue)
{
    FMaterialInstanceParameterUpdateContext Context(MatInst);
    FMaterialParameterInfo ParamInfo(ParamName);
    FMaterialParameterMetadata Data(ParamValue);
    Context.SetParameterValueEditorOnly(ParamInfo, Data);
}
