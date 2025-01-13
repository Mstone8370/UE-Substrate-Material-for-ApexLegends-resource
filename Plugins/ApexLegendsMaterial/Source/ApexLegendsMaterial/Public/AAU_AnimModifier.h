// Copyright (c) 2024 Minseok Kim

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AAU_AnimModifier.generated.h"

/**
 * 
 */

struct FAnimModData
{
	const TArray<FTransform>& RefBonePose;
	const TArray<FMeshBoneInfo>& RefBoneInfo;
	const IAnimationDataModel* DataModel;
	IAnimationDataController& DataController;

	explicit FAnimModData(const TArray<FTransform>& InRefBonePose,
		const TArray<FMeshBoneInfo>& InRefBoneInfo,
		const IAnimationDataModel* InDataModel,
		IAnimationDataController& InDataController)
		: RefBonePose(InRefBonePose)
		, RefBoneInfo(InRefBoneInfo)
		, DataModel(InDataModel)
		, DataController(InDataController)
	{}

	FAnimModData(const FAnimModData&) = delete;
	FAnimModData& operator=(const FAnimModData&) = delete;
};

UCLASS()
class APEXLEGENDSMATERIAL_API UAAU_AnimModifier : public UAssetActionUtility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(CallInEditor, Category = "Apex Legends Tool", meta = (DisplayName = "Modify Animation"))
	void ModifyAnimation(float Scale = 1.f, bool bUnrotateRootBone = false, UPARAM(DisplayName = "jx_c_start Bone Relative Motion") bool bStart = false, FName StartBoneName = FName(TEXT("jx_c_start")));

protected:
	void ModifySingleAnimation(UObject* Object, float Scale, bool bUnrotateRootBone, bool bStart, FName StartBoneName);

	void ScaleAnimation(float Scale, FAnimModData& AnimModData);

	int32 FindStartBoneIndex(const FName& StartBoneName, const FAnimModData& AnimModData);

	void MakeStartBoneRelative(int32 StartBoneIdx, FAnimModData& AnimModData);

	void UnrotateRootBone(FAnimModData& AnimModData);
};
