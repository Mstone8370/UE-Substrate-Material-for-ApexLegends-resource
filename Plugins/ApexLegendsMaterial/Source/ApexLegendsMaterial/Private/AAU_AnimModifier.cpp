// Copyright (c) 2024 Minseok Kim


#include "AAU_AnimModifier.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/Skeleton.h"

#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"

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

    // Start Test
    TArray<FTransform> root;
    TArray<FTransform> delta;
    TArray<FTransform> start;
    FName RootBoneName = RefBoneInfo[0].Name;

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
                // Start Test
                if (BoneName.IsEqual(RootBoneName))
                {
                    root.Add(FTransform(BoneRotation, ScaledBoneLocation, FVector::OneVector));
                }
                if (BoneName.IsEqual(FName("jx_c_delta"))) // TODO: The bone names differ for each skeleton. Use BoneInfo index?
                {
                    delta.Add(FTransform(BoneRotation, ScaledBoneLocation, FVector::OneVector));
                }
                if (BoneName.IsEqual(FName("jx_c_start")))
                {
                    start.Add(FTransform(BoneRotation, ScaledBoneLocation, FVector::OneVector));
                }
            }
        }

        // Set scale bone track keys
        AnimDataController.SetBoneTrackKeys(BoneName, PositionalKeys, RotationalKeys, ScalingKeys);

        ProgressDialog.EnterProgressFrame(1, FText::FromString(FString::Printf(TEXT("Converting Animation Scale... [%d/%d]"), i + 1, BoneNum)));
    }

    if (bStart)
    {
        // TODO: Change from adding additive curves to modifying the animation directly.
        // Start Test
        FAnimationCurveIdentifier Id(RootBoneName, ERawCurveTrackTypes::RCT_Transform);
        double PlayLength = AnimDataModel->GetPlayLength();
        int32 Keys = AnimDataModel->GetNumberOfKeys();
        double FrameRate = AnimDataModel->GetFrameRate().AsDecimal();
        double Interval = AnimDataModel->GetFrameRate().AsInterval();

        AnimDataController.AddCurve(Id);
        TArray<FTransform> Transforms;
        TArray<float> Times;
        float CurrentTime = 0.f;

        for (int32 i = 0; i < Keys; i++)
        {
            FTransform x = GetStartTransform_RootRelative(root[i], delta[i], start[i]);

            FRotator originalRootRotation = root[i].GetRotation().Rotator();
            FRotator startRotation = x.GetRotation().Rotator();

            FRotator deltaRotation = (originalRootRotation - startRotation).GetNormalized();

            Transforms.Add(FTransform(deltaRotation, -x.GetLocation()));
            Times.Add(CurrentTime);
            CurrentTime += Interval;
        }
        AnimDataController.SetTransformCurveKeys(Id, Transforms, Times);
    }

    AnimSeq->PostEditChange();
    AnimSeq->MarkPackageDirty();
}

FTransform UAAU_AnimModifier::GetStartTransform_RootRelative(const FTransform& root, const FTransform& delta, const FTransform& start)
{
    FMatrix LocalTransformStart = GetLocalTransformMatrix(start.GetLocation(), start.GetRotation(), start.GetScale3D());
    FMatrix LocalTransformDelta = GetLocalTransformMatrix(delta.GetLocation(), delta.GetRotation(), delta.GetScale3D());
    FMatrix LocalTransformRoot = GetLocalTransformMatrix(root.GetLocation(), root.GetRotation(), root.GetScale3D());
    
    FMatrix WorldTransform = LocalTransformStart * LocalTransformDelta * LocalTransformRoot;

    FVector Location = WorldTransform.GetOrigin();
    FQuat Rotation = WorldTransform.ToQuat();
    FVector Scale = WorldTransform.GetScaleVector();

    return FTransform(Rotation, Location, Scale);
}

FMatrix UAAU_AnimModifier::GetLocalTransformMatrix(const FVector& Translation, const FQuat& Rotation, const FVector& Scale)
{
    FMatrix ScaleMatrix = FScaleMatrix(Scale);
    FMatrix RotationMatrix = FRotationMatrix::Make(Rotation);
    FMatrix TranslationMatrix = FTranslationMatrix(Translation);

    FMatrix LocalTransform = ScaleMatrix * RotationMatrix * TranslationMatrix;

    return LocalTransform;
}
