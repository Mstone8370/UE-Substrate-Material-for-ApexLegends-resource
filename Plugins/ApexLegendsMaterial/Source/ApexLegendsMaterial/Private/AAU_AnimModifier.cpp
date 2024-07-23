// Copyright (c) 2024 Minseok Kim


#include "AAU_AnimModifier.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/Skeleton.h"

#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "Kismet/KismetMathLibrary.h"

void UAAU_AnimModifier::ScaleAnimation(float Scale, bool bUnrotateRootBone, bool bStart)
{
    if (Scale < UE_KINDA_SMALL_NUMBER)
    {
        Scale = 0.0254f;
    }

    TArray<FAssetData> SelectedAssetDatas = UEditorUtilityLibrary::GetSelectedAssetData();
    for (FAssetData& SelectedAssetData : SelectedAssetDatas)
    {
        UObject* SelectedObject = SelectedAssetData.GetAsset();
        if (!SelectedObject || !SelectedObject->IsA<UAnimSequence>())
        {
            continue;
        }

        const FString ObjectPath = SelectedObject->GetPathName();
        const FString FilePath = FPaths::GetBaseFilename(ObjectPath, false);
        const FString FolderPath = FPaths::GetPath(FilePath);
        const FString FileName = FPaths::GetBaseFilename(FilePath, true);

        const FString NewFileName = FileName + FString(TEXT("_Scaled"));
        const FString NewFilePath = FPaths::ConvertRelativePathToFull(FolderPath, NewFileName);
        
        if (UObject* DuplicatedObject = UEditorAssetLibrary::DuplicateLoadedAsset(SelectedObject, NewFilePath))
        {
            FAssetRegistryModule::AssetCreated(DuplicatedObject);

            ScaleAnimation_Internal(DuplicatedObject, Scale, bUnrotateRootBone, bStart);

            UEditorAssetLibrary::SaveAsset(NewFilePath, false);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Apex Legends Tool] Asset Duplication Failed"));
            FMessageDialog::Open(
                EAppMsgType::Ok,
                FText::FromString(TEXT("Asset Duplication Failed.")),
                FText::FromString(TEXT("Error"))
            );
        }
    }
}

void UAAU_AnimModifier::ScaleAnimation_Internal(UObject* Object, float Scale, bool bUnrotateRootBone, bool bStart)
{
    UAnimSequence* AnimSeq = Cast<UAnimSequence>(Object);
    if (!AnimSeq)
    {
        return;
    }

    // Get base pose info
    USkeleton* Skeleton = AnimSeq->GetSkeleton();
    const TArray<FTransform>& RefBonePose = Skeleton->GetReferenceSkeleton().GetRawRefBonePose();
    const TArray<FMeshBoneInfo>& RefBoneInfo = Skeleton->GetReferenceSkeleton().GetRawRefBoneInfo();
    const int32 BoneNum = RefBonePose.Num();
    if (BoneNum < 1)
    {
        return;
    }

    IAnimationDataModel* AnimDataModel = AnimSeq->GetDataModel();
    IAnimationDataController& AnimDataController = AnimSeq->GetController();

    // Show Dialog
    FScopedSlowTask ProgressDialog(BoneNum, FText::FromString(FString("Converting Animation Scale...")));
    ProgressDialog.MakeDialog();

    // Start
    TArray<FTransform> root;
    TArray<FTransform> delta;
    TArray<FTransform> start;

    for (int32 i = 0; i < BoneNum; i++)
    {
        const FName& BoneName = RefBoneInfo[i].Name;
        const FTransform& RefBoneTransform = RefBonePose[i];

        // Get animation bone transforms
        TArray<FTransform> BoneTrack;
        AnimDataModel->GetBoneTrackTransforms(BoneName, BoneTrack);

        TArray<FVector> PositionalKeys;
        TArray<FQuat> RotationalKeys;
        TArray<FVector> ScalingKeys;

        // Fill scaled bone track keys
        for (const FTransform& Transform : BoneTrack)
        {
            const FVector RefBoneLocation = RefBoneTransform.GetLocation();
            const FVector AnimBoneLocation = Transform.GetLocation();

            const FVector DeltaLocation = AnimBoneLocation - RefBoneLocation;
            const FVector DeltaDirection = DeltaLocation.GetSafeNormal();
            const double DeltaLength = DeltaLocation.Length();

            const FVector ScaledBoneLocation = RefBoneLocation + (DeltaDirection * (DeltaLength * Scale));

            FQuat BoneRotation = Transform.GetRotation();
            if (bUnrotateRootBone && i == 0)
            {
                FQuat RotationQuat = FRotator(0.f, 0.f, -90.f).Quaternion();
                BoneRotation = RotationQuat * BoneRotation;
            }

            PositionalKeys.Add(ScaledBoneLocation);
            RotationalKeys.Add(BoneRotation);
            ScalingKeys.Add(Transform.GetScale3D());

            if (bStart)
            {
                if (i == 0)
                {
                    root.Add(FTransform(BoneRotation, ScaledBoneLocation, Transform.GetScale3D()));
                }
                if (i == 1)
                {
                    delta.Add(FTransform(BoneRotation, ScaledBoneLocation, Transform.GetScale3D()));
                }
                if (i == 2)
                {
                    start.Add(FTransform(BoneRotation, ScaledBoneLocation, Transform.GetScale3D()));
                }
            }
        }

        // Set scale bone track keys
        AnimDataController.SetBoneTrackKeys(BoneName, PositionalKeys, RotationalKeys, ScalingKeys);

        ProgressDialog.EnterProgressFrame(1, FText::FromString(FString::Printf(TEXT("Converting Animation Scale... [%d/%d]"), i + 1, BoneNum)));
    }

    if (bStart)
    {
        TArray<FVector> PositionalKeys;
        TArray<FQuat> RotationalKeys;
        TArray<FVector> ScalingKeys;

        int32 Keys = AnimDataModel->GetNumberOfKeys();
        for (int32 i = 0; i < Keys; i++)
        {
            FTransform RootRelativeStart = start[i] * delta[i] * root[i];

            PositionalKeys.Add(root[i].GetLocation() - RootRelativeStart.GetLocation());
            RotationalKeys.Add(root[i].GetRotation());
            ScalingKeys.Add(root[i].GetScale3D());
        }

        AnimDataController.SetBoneTrackKeys(RefBoneInfo[0].Name, PositionalKeys, RotationalKeys, ScalingKeys);
    }

    AnimSeq->PostEditChange();
    AnimSeq->MarkPackageDirty();
}
