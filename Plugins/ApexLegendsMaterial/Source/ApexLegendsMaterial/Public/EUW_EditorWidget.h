// Copyright (c) 2024 Minseok Kim

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "EUW_EditorWidget.generated.h"

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
};
