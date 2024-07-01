// Copyright (c) 2024 Minseok Kim

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AAU_AnimModifier.generated.h"

/**
 * 
 */
UCLASS()
class APEXLEGENDSMATERIAL_API UAAU_AnimModifier : public UAssetActionUtility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(CallInEditor, Category = "Apex Legends Tool", meta = (DisplayName = "Scale Animation"))
	void ScaleAnimation(float Scale = 0.0254f, bool bUnrotateRootBone = false);

protected:
	void ScaleAnimation_Internal(UObject* Object, float Scale, bool bUnrotateRootBone);

	UPROPERTY(EditDefaultsOnly)
	FName RootBoneName = FName("jx_c_delta");
};
