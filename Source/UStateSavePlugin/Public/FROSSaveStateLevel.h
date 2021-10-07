#pragma once

#include "ROSBridgeSrvServer.h"
#include "std_srvs/Trigger.h"

class FROSSaveStateLevel final : public FROSBridgeSrvServer
{
public:
	FSimpleDelegate OnRosCallsSave;
	FROSSaveStateLevel(const FString InName, FString InType) : FROSBridgeSrvServer(InName, InType) {}

	TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
	{
		const TSharedPtr<std_srvs::Trigger::Request> Request = MakeShareable(new std_srvs::Trigger::Request());
		Request->FromJson(JsonObject);
		return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
	}

	TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
	{
		TSharedPtr<std_srvs::Trigger::Request> Request = StaticCastSharedPtr<std_srvs::Trigger::Request>(InRequest);
		OnRosCallsSave.ExecuteIfBound();
		return MakeShareable<FROSBridgeSrv::SrvResponse>(new std_srvs::Trigger::Response(true, FString("Saved State!")));
	}
};