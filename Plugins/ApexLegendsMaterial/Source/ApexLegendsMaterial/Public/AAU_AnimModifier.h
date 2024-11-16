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
	UFUNCTION(CallInEditor, Category = "Apex Legends Tool", meta = (DisplayName = "Modify Animation"))
	void ModifyAnimation(float Scale = 1.f, bool bUnrotateRootBone = false, UPARAM(DisplayName = "jx_c_start Bone Relative Motion") bool bStart = false);

protected:
	void ModifySingleAnimation(UObject* Object, float Scale, bool bUnrotateRootBone, bool bStart);
};
