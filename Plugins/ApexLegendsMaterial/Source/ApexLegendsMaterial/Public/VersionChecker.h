// Copyright (c) 2024 Minseok Kim

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "VersionChecker.generated.h"

DECLARE_DELEGATE_OneParam(FOnCheckedUpdate, bool /*bNewVersionAvailable*/);

/**
 * 
 */
UCLASS()
class APEXLEGENDSMATERIAL_API UVersionChecker : public UObject
{
	GENERATED_BODY()

public:
	FOnCheckedUpdate OnCheckedUpdate;

	FString PluginVersion = "";

	FString GetPluginVersion();

	void SendRequest();

	void OnReceivedResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
};
