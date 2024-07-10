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
	void ScaleAnimation(float Scale = 0.0254f, bool bUnrotateRootBone = false, UPARAM(DisplayName = "Start (unstable)") bool bStart = false);

protected:
	void ScaleAnimation_Internal(UObject* Object, float Scale, bool bUnrotateRootBone, bool bStart);

	FTransform GetStartTransform_RootRelative(const FTransform& root, const FTransform& delta, const FTransform& start);

	FMatrix GetLocalTransformMatrix(const FVector& Translation, const FQuat& Rotation, const FVector& Scale);
};
