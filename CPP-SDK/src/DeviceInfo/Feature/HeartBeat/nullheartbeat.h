#pragma once

class NullHeartBeat : public IHeartBeat
{
public:
	NullHeartBeat() = default;
	virtual ~NullHeartBeat() = default;
	virtual void startHeartBeatLoop() override
	{
	}
	virtual void stopHeartBeatLoop() override
	{
	}
	virtual void heartBeatLoop() override
	{
	}
};