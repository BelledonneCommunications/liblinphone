
#pragma once

#include <thread>
#include <linphone/core.h>
#include <linphone/lpconfig.h>
//#include <linphone/types.h>

class SipClient {
public:

	//	SipClient(const Settings &settings, std::unique_ptr<AbstractMediaControls> &&controls);

	void doIterate();
	static void mainLoop(SipClient &client, bool &running);
	LinphoneConfig *setupConfig() const;
	void startLibLinphone();
	void destroyLinphoneCore();
	bool isInCall() const;
	bool isRegistered() const;
	void refresh();
	void enableRegister(bool enable);
	bool isRegistrationEnabled();
	void terminateCall();
	void setGlobalProxy();

	bool mRunning;
	bool mPrepareStop;
	bool mTerminateCall;
	std::thread mLoop;
	LinphoneProxyConfig *mGlobalProxy;
	LinphoneCore *mCore;
};
