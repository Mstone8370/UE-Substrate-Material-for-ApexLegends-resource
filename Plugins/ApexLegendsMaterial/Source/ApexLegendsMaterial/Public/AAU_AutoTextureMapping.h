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
	void AutoTextureMapping(UPARAM(DisplayName = "Custom Texture Folder") FString TextureFolderNameOverride);

protected:
	bool CheckMasterMaterial();

	// Read Mesh's materials info, find existing Material Instance or create new Material Instance, and set.
	void SetMaterialInstances(UObject* MeshObject, TMap<FString, TArray<UMaterialInstance*>>& OutMaterialNameMap);

	void SetMaterialInstances_SkeletalMesh(USkeletalMesh* SkeletalMesh, TMap<FString, TArray<UMaterialInstance*>>& OutMaterialNameMap);

	void SetMaterialInstances_StaticMesh(UStaticMesh* StaticMesh, TMap<FString, TArray<UMaterialInstance*>>& OutMaterialNameMap);

	UMaterialInstance* CastOrFindOrCreateMaterialInstance(UMaterialInterface*& MaterialInterface, const FString& BasePath, const FString& MaterialSlotName, UMaterialInterface* ParentMaterial);

	// Create new Material Instance asset
	UMaterialInstanceConstant* CreateMaterialInstance(UMaterialInterface* ParentMaterial, FString FullPath);

	void GetTexturePaths(TSet<UObject*> Objects, const FString& TextureFolderName, TSet<FString> OutPaths);

	void MapTexturesToMaterial(TMap<FString, TArray<UMaterialInstance*>>& InMaterialMap, TSet<FString> InTexturePaths);

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
