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

void UAAU_AnimModifier::ModifyAnimation(float Scale, bool bUnrotateRootBone, bool bStart, FName StartBoneName)
{
    if (Scale < UE_SMALL_NUMBER)
    {
        Scale = 1.f;
    }

    if (FMath::IsNearlyEqual(Scale, 1.f) && !bUnrotateRootBone && !bStart)
    {
        return;
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

        const FString ModifiedFileName = FileName + FString(TEXT("_Modified"));
        
        // Try to duplicate object
        int32 DuplicateTryMax = 999;
        UObject* DuplicatedObject = nullptr;
        for (int32 DuplicateTry = 0; DuplicateTry < DuplicateTryMax; DuplicateTry++)
        {
            FString NewFileName = ModifiedFileName;
            if (DuplicateTry > 0)
            {
                NewFileName += ("_" + FString::FromInt(DuplicateTry));
            }
            const FString NewFilePath = FPaths::ConvertRelativePathToFull(FolderPath, NewFileName);

            DuplicatedObject = UEditorAssetLibrary::DuplicateLoadedAsset(SelectedObject, NewFilePath);
            if (DuplicatedObject)
            {
                break;
            }
        }

        if (DuplicatedObject)
        {
            FAssetRegistryModule::AssetCreated(DuplicatedObject);

            ModifySingleAnimation(DuplicatedObject, Scale, bUnrotateRootBone, bStart, StartBoneName);

            const FString DuplicatedObjectPath = DuplicatedObject->GetPathName();
            const FString DuplicatedFilePath = FPaths::GetBaseFilename(DuplicatedObjectPath, false);
            UEditorAssetLibrary::SaveAsset(DuplicatedFilePath, false);
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

void UAAU_AnimModifier::ModifySingleAnimation(UObject* Object, float Scale, bool bUnrotateRootBone, bool bStart, FName StartBoneName)
{
    UAnimSequence* AnimSeq = Cast<UAnimSequence>(Object);
    if (!AnimSeq)
    {
        return;
    }

    // Get reference pose info
    const USkeleton* Skeleton = AnimSeq->GetSkeleton();
    FAnimModData AnimModData(
        Skeleton->GetReferenceSkeleton().GetRawRefBonePose(),
        Skeleton->GetReferenceSkeleton().GetRawRefBoneInfo(),
        AnimSeq->GetDataModel(),
        AnimSeq->GetController()
    );
    if (!AnimModData.DataModel)
    {
        UE_LOG(LogTemp, Error, TEXT("Animation Data Model is not valid: %s"), *Object->GetPathName());
        return;
    }

    AnimSeq->PreEditChange(nullptr);
    AnimSeq->Modify();

    if (!FMath::IsNearlyEqual(Scale, 1.f))
    {
        ScaleAnimation(Scale, AnimModData);
    }

    if (bStart)
    {
        // Find the Start bone
        int32 StartBoneIdx = INDEX_NONE;
        const int32 BoneNum = AnimModData.RefBonePose.Num();
        for (int32 BoneIdx = 0; BoneIdx < BoneNum; BoneIdx++)
        {
            const FName& BoneName = AnimModData.RefBoneInfo[BoneIdx].Name;
            if (BoneName.IsEqual(StartBoneName))
            {
                StartBoneIdx = BoneIdx;
                break;
            }
        }

        if (StartBoneIdx != INDEX_NONE && StartBoneIdx < BoneNum)
        {
            MakeStartBoneRelative(StartBoneIdx, AnimModData);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[Animation Modifier] Could not find the [%s] bone."), *StartBoneName.ToString());
            FMessageDialog::Open(
                EAppMsgType::Ok,
                FText::FromString(TEXT("[Warning]\nAnimation Modifier:\n  Start bone relative motion\n\n\nCould not find the [" + StartBoneName.ToString() + "] bone.")),
                FText::FromString(TEXT("Warning"))
            );
        }
    }

    if (bUnrotateRootBone)
    {
        UnrotateRootBone(AnimModData);
    }

    AnimSeq->PostEditChange();
    AnimSeq->MarkPackageDirty();
}

void UAAU_AnimModifier::ScaleAnimation(float Scale, FAnimModData& AnimModData)
{
    const int32 BoneNum = AnimModData.RefBonePose.Num();

    // Show Dialog
    FScopedSlowTask ProgressDialog(BoneNum, FText::FromString(FString("Scaling Animation...")));
    ProgressDialog.MakeDialog();

    // Iterate bones
    for (int32 BoneIdx = 0; BoneIdx < BoneNum; BoneIdx++)
    {
        ProgressDialog.EnterProgressFrame(1, FText::FromString(FString::Printf(TEXT("Scaling Animation... [%d/%d]"), BoneIdx + 1, BoneNum)));

        const int32 KeyNum = AnimModData.DataModel->GetNumberOfKeys();
        const FName& BoneName = AnimModData.RefBoneInfo[BoneIdx].Name;

        const FVector& RefBoneLocation = AnimModData.RefBonePose[BoneIdx].GetLocation();

        // Get animation bone transforms
        TArray<FTransform> OriginalBoneTrack;
        AnimModData.DataModel->GetBoneTrackTransforms(BoneName, OriginalBoneTrack);

        TArray<FVector> PositionalKeys;
        TArray<FQuat> RotationalKeys;
        TArray<FVector> ScalingKeys;

        // Fill scaled bone track keys
        for (int32 KeyIdx = 0; KeyIdx < KeyNum; KeyIdx++)
        {
            const FTransform& OriginalTransform = OriginalBoneTrack[KeyIdx];

            // Scale the travel distance based on the reference bone's location.
            const FVector& OriginalBoneLocation = OriginalTransform.GetLocation();
            const FVector DeltaLocation = OriginalBoneLocation - RefBoneLocation;
            const FVector ScaledBoneLocation = RefBoneLocation + (DeltaLocation * Scale);

            PositionalKeys.Add(ScaledBoneLocation);
            RotationalKeys.Add(OriginalTransform.GetRotation());
            ScalingKeys.Add(OriginalTransform.GetScale3D());
        }

        // Set scale bone track keys
        AnimModData.DataController.SetBoneTrackKeys(BoneName, PositionalKeys, RotationalKeys, ScalingKeys);
    }
}

void UAAU_AnimModifier::MakeStartBoneRelative(int32 StartBoneIdx, FAnimModData& AnimModData)
{
    const int32 KeyNum = AnimModData.DataModel->GetNumberOfKeys();

    // Find the ancestors of the Start bone.
    TArray<int32> StartToRootIndeces;
    int32 Idx = StartBoneIdx;
    while (Idx != INDEX_NONE)
    {
        StartToRootIndeces.Add(Idx);
        Idx = AnimModData.RefBoneInfo[Idx].ParentIndex;
    }

    // Show Dialog
    FScopedSlowTask ProgressDialog(StartToRootIndeces.Num() + 1, FText::FromString(FString("Converting to Start bone relative...")));
    ProgressDialog.MakeDialog();

    TArray<FTransform> ComponentRelativeStart;
    for (int32 KeyIdx = 0; KeyIdx < KeyNum; KeyIdx++)
    {
        ComponentRelativeStart.Add(FTransform::Identity);
    }

    int32 ProgressCnt = 0; // for ProgressDialog

    // Space transformation accumulation.
    for (auto it = StartToRootIndeces.rbegin(); it != StartToRootIndeces.rend(); ++it)
    {
        ProgressDialog.EnterProgressFrame(1, FText::FromString(FString::Printf(TEXT("Converting to Start bone relative... [%d/%d]"), ++ProgressCnt, StartToRootIndeces.Num() + 1)));

        const FName& BoneName = AnimModData.RefBoneInfo[*it].Name;

        TArray<FTransform> BoneTrack;
        AnimModData.DataModel->GetBoneTrackTransforms(BoneName, BoneTrack);

        for (int32 KeyIdx = 0; KeyIdx < KeyNum; KeyIdx++)
        {
            const FTransform& BoneTransform = BoneTrack[KeyIdx];

            /**
            * UKismetMathLibrary::ComposeTransforms(A, B) == A * B.
            * ChildTransformInOuterParentSpace = ChildLocalTransform * ParentTransform;
            */
            ComponentRelativeStart[KeyIdx] = UKismetMathLibrary::ComposeTransforms(
                BoneTransform,
                ComponentRelativeStart[KeyIdx]
            );
        }
    }

    ProgressDialog.EnterProgressFrame(1, FText::FromString(FString::Printf(TEXT("Converting to Start bone relative... [%d/%d]"), ++ProgressCnt, StartToRootIndeces.Num() + 1)));

    // Move the root bone to keep the Start bone fixed at the origin.
    const FName& RootBoneName = AnimModData.RefBoneInfo[0].Name;

    TArray<FTransform> RootTrack;
    AnimModData.DataModel->GetBoneTrackTransforms(RootBoneName, RootTrack);

    TArray<FVector> PositionalKeys;
    TArray<FQuat> RotationalKeys;
    TArray<FVector> ScalingKeys;

    for (int32 KeyIdx = 0; KeyIdx < KeyNum; KeyIdx++)
    {
        // Translate first, then rotate around the origin

        FVector RootTranslation = RootTrack[KeyIdx].GetLocation() - ComponentRelativeStart[KeyIdx].GetLocation();

        const FQuat RotationQuat = ComponentRelativeStart[KeyIdx].GetRotation().GetAxisX().ToOrientationQuat().Inverse();
        RootTranslation = RotationQuat.RotateVector(RootTranslation);

        PositionalKeys.Add(RootTranslation);
        RotationalKeys.Add(RotationQuat * RootTrack[KeyIdx].GetRotation());
        ScalingKeys.Add(RootTrack[KeyIdx].GetScale3D());
    }

    AnimModData.DataController.SetBoneTrackKeys(RootBoneName, PositionalKeys, RotationalKeys, ScalingKeys);
}

void UAAU_AnimModifier::UnrotateRootBone(FAnimModData& AnimModData)
{
    const int32 KeyNum = AnimModData.DataModel->GetNumberOfKeys();
    const FName& BoneName = AnimModData.RefBoneInfo[0].Name;

    // Get animation bone transforms
    TArray<FTransform> OriginalBoneTrack;
    AnimModData.DataModel->GetBoneTrackTransforms(BoneName, OriginalBoneTrack);

    TArray<FVector> PositionalKeys;
    TArray<FQuat> RotationalKeys;
    TArray<FVector> ScalingKeys;

    for (int32 KeyIdx = 0; KeyIdx < KeyNum; KeyIdx++)
    {
        const FTransform& OriginalTransform = OriginalBoneTrack[KeyIdx];
        const FQuat RotationQuat = FRotator(0.f, 0.f, 90.f).Quaternion();

        FVector RootTranslation = OriginalBoneTrack[KeyIdx].GetLocation();
        RootTranslation = RotationQuat.RotateVector(RootTranslation);
        
        FQuat RootRotation = OriginalTransform.GetRotation();
        RootRotation = RotationQuat * RootRotation;

        PositionalKeys.Add(RootTranslation);
        RotationalKeys.Add(RootRotation);
        ScalingKeys.Add(OriginalTransform.GetScale3D());
    }

    AnimModData.DataController.SetBoneTrackKeys(BoneName, PositionalKeys, RotationalKeys, ScalingKeys);
}
