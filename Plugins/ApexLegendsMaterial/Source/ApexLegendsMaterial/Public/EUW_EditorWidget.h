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
	UFUNCTION(BlueprintCallable)
	bool GetMaterialSlotNames(UObject* Object, TArray<FName>& OutNames, FString& OutSkinName);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool FindSkinNameFromMaterialSlotName(UPARAM(ref) const FName& SlotName, FString& OutSkinName);

	UFUNCTION(BlueprintCallable)
	void ChangeMaterialSlotNames(UObject* Object, FString NewSkinName, UPARAM(ref) TArray<FName>& SlotNamesToChange);

	void ChangeSlotName_Internal(FName& MatSlotName, FString NewSkinName, TMap<FName, FName>& SucceededMap, TMap<FName, FName>& FailedMap);
};
