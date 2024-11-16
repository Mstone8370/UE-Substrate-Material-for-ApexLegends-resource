// Copyright (c) 2024 Minseok Kim

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"

#include "AAU_AutoTextureMapping.generated.h"

class UMaterialInstanceConstant;
class UMaterialInterface;

UCLASS()
class APEXLEGENDSMATERIAL_API UAAU_AutoTextureMapping : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	UAAU_AutoTextureMapping();

	UFUNCTION(CallInEditor, Category = "Apex Legends Tool", meta = (DisplayName = "Disconnect All Materials"))
	void DisconnectAllMaterials();

	UFUNCTION(CallInEditor, Category = "Apex Legends Tool", meta = (DisplayName = "Auto Texture Mapping"))
	void AutoTextureMapping(UPARAM(DisplayName = "Custom Texture Folder") FString TextureFolderNameOverride, UPARAM(DisplayName = "Flip Normal Map Green Channel") bool bFlipNormalGreen);

protected:
	bool CheckMasterMaterial();

	// Read Mesh's materials info, find existing Material Instance or create new Material Instance, and set.
	void SetMaterialInstances(UObject* MeshObject, TMap<FString, TArray<UMaterialInstance*>>& OutMaterialNameMap);

    // Template function for mesh types
    template<typename MeshType, typename MaterialType>
    void SetMaterialInstances_Generic(MeshType* Mesh, TMap<FString, TArray<UMaterialInstance*>>& OutMaterialMap, TArray<MaterialType>& (MeshType::* GetMaterialsFunc)());
    
	UMaterialInstance* CastOrFindOrCreateMaterialInstance(UMaterialInterface*& MaterialInterface, const FString& BasePath, const FString& MaterialSlotName, UMaterialInterface* ParentMaterial);

	// Create new Material Instance asset
	UMaterialInstanceConstant* CreateMaterialInstance(UMaterialInterface* ParentMaterial, FString FullPath);

    // Get all texture asset's path
	void GetTexturePaths(TSet<UObject*> Objects, const FString& TextureFolderName, TSet<FString>& OutPaths);

    // Texture mapping
	void MapTexturesToMaterial(TMap<FString, TArray<UMaterialInstance*>>& InMaterialMap, TSet<FString>& InTexturePaths, bool bFlipNormalGreen);

    // Helper function to set parameters of a material instance
	void SetMaterialParamValue(UMaterialInstance* MatInst, const FName& ParamName, FMaterialParameterValue ParamValue);

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup")
	FString DefaultTextureFolderName;

	FString DefaultMasterMaterialPath;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup|Material Setup")
	TObjectPtr<UMaterialInterface> MasterMaterialOverride;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup|Material Setup", meta = (DisplayName = "Custom Material Overrides"))
	TMap<FName, TObjectPtr<UMaterialInterface>> CustomMaterialMap;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup")
	TMap<FString, FName> TextureTypeToParamName;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup")
	TSet<FString> LinearTextureTypes;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInterface> MasterMaterial;
};

template<typename MeshType, typename MaterialType>
inline void UAAU_AutoTextureMapping::SetMaterialInstances_Generic(MeshType* Mesh, TMap<FString, TArray<UMaterialInstance*>>& OutMaterialMap, TArray<MaterialType>& (MeshType::* GetMaterialsFunc)())
{
    // Set Material Instances
    TArray<MaterialType>& MaterialList = (Mesh->*GetMaterialsFunc)();
    
    for (MaterialType& Material : MaterialList)
    {
        FName MaterialSlotName = Material.MaterialSlotName;
        if (CustomMaterialMap.Contains(MaterialSlotName))
        {
            Material.MaterialInterface = *CustomMaterialMap.Find(MaterialSlotName);
            continue;
        }

        // Trying cast Material Interface to Material Instance. If failed, create new Material Instance.
        UMaterialInterface* MaterialInterface = Material.MaterialInterface;
        UMaterialInstance* MaterialInstance = CastOrFindOrCreateMaterialInstance(
            MaterialInterface,
            FPaths::GetPath(Mesh->GetPathName()),
            MaterialSlotName.ToString(),
            MasterMaterial
        );
        if (!MaterialInstance)
        {
            continue;
        }

        // Set Material for when a new Material Instance is created
        Material.MaterialInterface = MaterialInstance;

        // Add to map for texture mapping
        FString MaterialSlotNameStr = MaterialSlotName.ToString();
        if (OutMaterialMap.Contains(MaterialSlotNameStr))
        {
            OutMaterialMap.Find(MaterialSlotNameStr)->Add(MaterialInstance);
        }
        else
        {
            OutMaterialMap.Add(MaterialSlotNameStr, TArray<UMaterialInstance*>({ MaterialInstance }));
        }
    }
}
