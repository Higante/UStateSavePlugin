#pragma once

#include "ROSBridgeSrvServer.h"
#include "DeleteModel.h"

DECLARE_DELEGATE_OneParam(FRosCallback, FString);

class FROSSaveStateLevel final : public FROSBridgeSrvServer
{
public:
	FRosCallback OnRosCallback;
	FROSSaveStateLevel(const FString InName, FString InType) : FROSBridgeSrvServer(InName, InType) {}

	TSharedPtr<FROSDeleteModelSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
	{
		const TSharedPtr<FROSDeleteModelSrv::Request> Request = MakeShareable(new FROSDeleteModelSrv::Request());
		Request->FromJson(JsonObject);
		return TSharedPtr<FROSDeleteModelSrv::SrvRequest>(Request);
	}

	TSharedPtr<FROSDeleteModelSrv::SrvResponse> Callback(TSharedPtr<FROSDeleteModelSrv::SrvRequest> InRequest) override
	{
		TSharedPtr<FROSDeleteModelSrv::Request> Request = StaticCastSharedPtr<FROSDeleteModelSrv::Request>(InRequest);
		OnRosCallback.ExecuteIfBound(Request->GetId());
		return MakeShareable<FROSDeleteModelSrv::SrvResponse>(new FROSDeleteModelSrv::Response(true));
	}
};