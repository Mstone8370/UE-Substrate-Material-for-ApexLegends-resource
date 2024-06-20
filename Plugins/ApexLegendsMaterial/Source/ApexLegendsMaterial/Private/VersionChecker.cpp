// Copyright (c) 2024 Minseok Kim


#include "VersionChecker.h"

#include "Interfaces/IPluginManager.h"

#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Internationalization/Regex.h"

FString UVersionChecker::GetPluginVersion()
{
    if (PluginVersion.IsEmpty())
    {
        if (FJsonObject* JsonObj = IPluginManager::Get().FindPlugin("ApexLegendsMaterial")->GetDescriptorJson().Get())
        {
            if (FJsonValue* JsonValue = JsonObj->TryGetField(TEXT("VersionName")).Get())
            {
                PluginVersion = JsonValue->AsString();
            }
        }
    }
    return PluginVersion;
}

void UVersionChecker::SendRequest()
{
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UVersionChecker::OnReceivedResponse);

    Request->SetURL(TEXT("https://api.github.com/repos/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/releases/latest"));
    Request->SetVerb(TEXT("Get"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetActivityTimeout(10.f);
    
    Request->ProcessRequest();
}

void UVersionChecker::OnReceivedResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
    bool bNewVersionAvailable = false;

    if (bConnectedSuccessfully)
    {
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        TSharedPtr<FJsonObject> JsonObject;
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            FString LatestTag = JsonObject->GetStringField(TEXT("tag_name"));
            FString CurrentVersion = GetPluginVersion();
            if (!LatestTag.IsEmpty() && !CurrentVersion.IsEmpty())
            {
                FRegexPattern Pattern(TEXT("(\\d+).(\\d+).(\\d+)"));

                FRegexMatcher Matcher_Latest(Pattern, LatestTag);
                FRegexMatcher Matcher_Current(Pattern, CurrentVersion);
                if (Matcher_Latest.FindNext() && Matcher_Current.FindNext())
                {
                    for (int32 i = 1; i <= 3; i++)
                    {
                        const int32 Latest = FCString::Atoi(*Matcher_Latest.GetCaptureGroup(i));
                        const int32 Current = FCString::Atoi(*Matcher_Current.GetCaptureGroup(i));
                        if (Latest > Current)
                        {
                            bNewVersionAvailable = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (OnCheckedUpdate.IsBound())
    {
        OnCheckedUpdate.ExecuteIfBound(bNewVersionAvailable);
    }
}
