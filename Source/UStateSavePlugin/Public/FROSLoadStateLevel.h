#pragma once

#include "ROSBridgeSrvServer.h"
#include "DeleteModel.h"

DECLARE_DELEGATE_OneParam(FRosCallback, FString);

class FROSLoadStateLevel final : public FROSBridgeSrvServer
{
public:
	FRosCallback OnRosCallback;
	FROSLoadStateLevel(const FString InName, FString InType) : FROSBridgeSrvServer(InName, InType) {}

	TSharedPtr<FROSDeleteModelSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
	{
		const TSharedPtr<FROSDeleteModelSrv::Request> Request = MakeShareable(new FROSDeleteModelSrv::Request());
		Request->FromJson(JsonObject);
		return TSharedPtr<FROSDeleteModelSrv::SrvRequest>(Request);
	}

	TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
	{
		TSharedPtr<FROSDeleteModelSrv::Request> Request = StaticCastSharedPtr<FROSDeleteModelSrv::Request>(InRequest);
		OnRosCallback.ExecuteIfBound(Request->GetId());
		return MakeShareable<FROSDeleteModelSrv::SrvResponse>(new FROSDeleteModelSrv::Response(true));
	}
};
