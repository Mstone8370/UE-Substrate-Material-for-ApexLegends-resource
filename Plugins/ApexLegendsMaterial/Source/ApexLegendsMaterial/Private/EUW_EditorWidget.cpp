// Copyright (c) 2024 Minseok Kim


#include "EUW_EditorWidget.h"

#include "AAU_AutoTextureMapping.h"
#include "VersionChecker.h"

#include "Engine/SkinnedAssetCommon.h"
#include "EditorAssetLibrary.h"

bool UEUW_EditorWidget::GetMaterialSlotNames(UObject* Object, TArray<FName>& OutNames, FString& OutSkinName)
{
    OutNames.Empty();

    if (USkeletalMesh* SK = Cast<USkeletalMesh>(Object))
    {
        TArray<FSkeletalMaterial>& Materials = SK->GetMaterials();
        for (const FSkeletalMaterial& Material : Materials)
        {
            OutNames.Add(Material.MaterialSlotName);
        }
    }
    else if (UStaticMesh* SM = Cast<UStaticMesh>(Object))
    {
        TArray<FStaticMaterial>& Materials = SM->GetStaticMaterials();
        for (const FStaticMaterial& Material : Materials)
        {
            OutNames.Add(Material.MaterialSlotName);
        }
    }
    else
    {
        return false;
    }
    
    FName eyecornea(TEXT("wraith_base_eyecornea"));
    FName eyeshadow(TEXT("wraith_base_eyeshadow"));

    OutSkinName = "";
    int32 MaxCnt = 0;
    TMap<FString, int32> SkinNameCounter;
    for (const FName& SlotName : OutNames)
    {
        if (SlotName.IsEqual(eyecornea) || SlotName.IsEqual(eyeshadow))
        {
            continue;
        }

        FString SkinName;
        if (FindSkinNameFromMaterialSlotName(SlotName, SkinName))
        {
            int32& Cnt = SkinNameCounter.FindOrAdd(SkinName, 0);
            Cnt++;
            if (Cnt > MaxCnt)
            {
                OutSkinName = SkinName;
                MaxCnt = Cnt;
            }
        }
    }

    return true;
}

bool UEUW_EditorWidget::FindSkinNameFromMaterialSlotName(const FName& SlotName, FString& OutSkinName)
{
    OutSkinName = "";

    FString SlotNameStr = SlotName.ToString();
    int32 LeftIdx = -1;
    SlotNameStr.FindChar('_', LeftIdx);
    if (LeftIdx >= 0)
    {
        SlotNameStr = SlotNameStr.RightChop(LeftIdx + 1);
        int32 RightIdx = -1;
        SlotNameStr.FindLastChar('_', RightIdx);
        if (RightIdx >= 0)
        {
            OutSkinName = SlotNameStr.Left(RightIdx);
            return true;
        }
    }
    return false;
}

void UEUW_EditorWidget::ChangeMaterialSlotNames(UObject* Object, FString NewSkinName, TArray<FName>& SlotNamesToChange)
{
    TSet<FName> SlotNamesSet(SlotNamesToChange);
    TMap<FName, FName> SucceededMap;
    TMap<FName, FName> FailedMap;

    if (USkeletalMesh* SK = Cast<USkeletalMesh>(Object))
    {
        TArray<FSkeletalMaterial>& Materials = SK->GetMaterials();
        for (FSkeletalMaterial& Material : Materials)
        {
            if (SlotNamesSet.Contains(Material.MaterialSlotName))
            {
                ChangeSlotName_Internal(Material.MaterialSlotName, NewSkinName, SucceededMap, FailedMap);
            }
        }
    }
    else if (UStaticMesh* SM = Cast<UStaticMesh>(Object))
    {
        TArray<FStaticMaterial>& Materials = SM->GetStaticMaterials();
        for (FStaticMaterial& Material : Materials)
        {
            if (SlotNamesSet.Contains(Material.MaterialSlotName))
            {
                ChangeSlotName_Internal(Material.MaterialSlotName, NewSkinName, SucceededMap, FailedMap);
            }
        }
    }

    // Update Object
    Object->PostEditChange(); 

    const FString ObjectPath = Object->GetPathName();
    const FString FilePath = FPaths::GetBaseFilename(ObjectPath, false);
    UEditorAssetLibrary::SaveAsset(FilePath, false);
}

void UEUW_EditorWidget::ChangeSlotName_Internal(FName& MatSlotName, FString NewSkinName, TMap<FName, FName>& SucceededMap, TMap<FName, FName>& FailedMap)
{
    FName PrevName = MatSlotName;

    FString SlotNameStr = MatSlotName.ToString();
    int32 LeftIdx = -1;
    int32 RightIdx = -1;
    SlotNameStr.FindChar('_', LeftIdx);
    SlotNameStr.FindLastChar('_', RightIdx);
    if (LeftIdx < RightIdx && LeftIdx >= 0)
    {
        FString ModelName = SlotNameStr.Left(LeftIdx + 1);
        FString Part = SlotNameStr.RightChop(RightIdx);
        FString NewSlotName = ModelName + NewSkinName + Part;
        MatSlotName = FName(NewSlotName);
        
        SucceededMap.Add(PrevName, MatSlotName);
    }
    else
    {
        FailedMap.Add(PrevName, MatSlotName);
    }
}

void UEUW_EditorWidget::AutoTextureMapping(FString TextureFolderName)
{
    if (GetAAU())
    {
        GetAAU()->AutoTextureMapping(TextureFolderName);
    }
    else
    {
        FMessageDialog::Open(
            EAppMsgType::Ok,
            FText::FromString(TEXT("AAU is not valid.")),
            FText::FromString(TEXT("Error"))
        );
    }
}

void UEUW_EditorWidget::DisconnectAllMaterials()
{
    if (GetAAU())
    {
        GetAAU()->DisconnectAllMaterials();
    }
    else
    {
        FMessageDialog::Open(
            EAppMsgType::Ok,
            FText::FromString(TEXT("AAU is not valid.")),
            FText::FromString(TEXT("Error"))
        );
    }
}

FString UEUW_EditorWidget::GetPluginVersion()
{
    if (GetVC())
    {
        return GetVC()->GetPluginVersion();
    }
    return FString();
}

void UEUW_EditorWidget::CheckUpdate()
{
    if (GetVC())
    {
        GetVC()->SendRequest();
    }
}

UAAU_AutoTextureMapping* UEUW_EditorWidget::GetAAU()
{
    if (!AAU)
    {
        AAU = NewObject<UAAU_AutoTextureMapping>(this, AAU_Class);
    }
    return AAU;
}

UVersionChecker* UEUW_EditorWidget::GetVC()
{
    if (!VC)
    {
        VC = NewObject<UVersionChecker>(this);
        VC->OnCheckedUpdate.BindLambda(
            [this](bool b)
            {
                OnCheckedUpdateDelegate.Broadcast(b);
            }
        );
    }
    return VC;
}
