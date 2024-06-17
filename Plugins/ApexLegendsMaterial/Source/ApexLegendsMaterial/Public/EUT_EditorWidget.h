// Copyright (c) 2024 Minseok Kim

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "EUT_EditorWidget.generated.h"

/**
 * 
 */
UCLASS()
class APEXLEGENDSMATERIAL_API UEUT_EditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(BlueprintCallable)
	void Func();
};
