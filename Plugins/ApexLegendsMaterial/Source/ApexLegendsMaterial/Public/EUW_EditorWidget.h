// Copyright (c) 2024 Minseok Kim

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "EUW_EditorWidget.generated.h"

class UAAU_AutoTextureMapping;
class UVersionChecker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCheckedUpdateSignature, bool, bNewVersionAvailable);

/**
 * 
 */
UCLASS()
class APEXLEGENDSMATERIAL_API UEUW_EditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, Category = "Apex Legends Tool")
	bool GetMaterialSlotNames(UObject* Object, TArray<FName>& OutNames, FString& OutSkinName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Apex Legends Tool")
	bool FindSkinNameFromMaterialSlotName(UPARAM(ref) const FName& SlotName, FString& OutSkinName);

	UFUNCTION(BlueprintCallable, Category = "Apex Legends Tool")
	void ChangeMaterialSlotNames(UObject* Object, FString NewSkinName, UPARAM(ref) TArray<FName>& SlotNamesToChange);

	void ChangeSlotName_Internal(FName& MatSlotName, FString NewSkinName, TMap<FName, FName>& SucceededMap, TMap<FName, FName>& FailedMap);

	UFUNCTION(BlueprintCallable, Category = "Apex Legends Tool")
	void AutoTextureMapping(FString TextureFolderName);

	UFUNCTION(BlueprintCallable, Category = "Apex Legends Tool")
	void DisconnectAllMaterials();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Apex Legends Tool")
	FString GetPluginVersion();

	UFUNCTION(BlueprintCallable, Category = "Apex Legends Tool")
	void CheckUpdate();

	UFUNCTION(BlueprintCallable, Category = "Apex Legends Tool")
	void OpenAAU();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Apex Legends Tool")
	TSubclassOf<UAAU_AutoTextureMapping> AAU_Class;

	UPROPERTY(BlueprintAssignable)
	FOnCheckedUpdateSignature OnCheckedUpdateDelegate;

private:
	UPROPERTY()
	TObjectPtr<UAAU_AutoTextureMapping> AAU;

	UAAU_AutoTextureMapping* GetAAU();

	UPROPERTY()
	TObjectPtr<UVersionChecker> VC;

	UVersionChecker* GetVC();
};
