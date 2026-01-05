/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "bctoolbox/defs.h"

#include "ortp/rtpsession.h"

#include "TunnelManager.hh"
#include "account/account.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/core.h"
#include "linphone/core_utils.h"
#include "private.h"
#include "private_functions.h"

belledonnecomm::TunnelManager *bcTunnel(const LinphoneTunnel *tunnel);

using namespace belledonnecomm;
using namespace LinphonePrivate;
using namespace ::std;

void TunnelManager::addServer(const char *ip, int port, unsigned int udpMirrorPort, unsigned int delay) {
	if (ip == NULL) {
		ms_warning("Adding tunnel server with empty ip, it will not work!");
		return;
	}
	addServer(ip, port);
	mUdpMirrorClients.push_back(UdpMirrorClient(ServerAddr(ip, (int)udpMirrorPort), delay));
}

void TunnelManager::addServer(const char *ip, int port) {
	if (ip == NULL) {
		ms_warning("Adding tunnel server with empty ip, it will not work!");
		return;
	}
	if (mUseDualClient) {
		ms_warning("TunnelManager is configured in dual mode, use addServerPair instead");
		return;
	}

	mServerAddrs.push_back(ServerAddr(ip, port));
	if (mTunnelClient && !mTunnelClient->isDualTunnelClient()) {
		static_cast<TunnelClient *>(mTunnelClient)->addServer(ip, port);
	}
}

void TunnelManager::addServerPair(
    const char *ip1, int port1, const char *ip2, int port2, unsigned int udpMirrorPort, unsigned int delay) {
	if (ip1 == NULL || ip2 == NULL) {
		ms_warning("Adding tunnel server with empty ip, it will not work!");
		return;
	}
	addServerPair(ip1, port1, ip2, port2);
	mUdpMirrorClients.push_back(UdpMirrorClient(ServerAddr(ip1, (int)udpMirrorPort), delay));
}

void TunnelManager::addServerPair(const char *ip1, int port1, const char *ip2, int port2) {
	if (ip1 == NULL || ip2 == NULL) {
		ms_warning("Adding tunnel server with empty ip, it will not work!");
		return;
	}
	if (!mUseDualClient) {
		ms_warning("TunnelManager is configured in single mode, use addServer instead");
		return;
	}

	mDualServerAddrs.push_back(DualServerAddr(ip1, port1, ip2, port2));
	if (mTunnelClient && mTunnelClient->isDualTunnelClient()) {
		static_cast<DualTunnelClient *>(mTunnelClient)->addServerPair(ip1, port1, ip2, port2);
	}
}

void TunnelManager::cleanServers() {
	mServerAddrs.clear();
	mDualServerAddrs.clear();
	if (mLongRunningTaskId > 0) {
		sal_end_background_task(mLongRunningTaskId);
		mLongRunningTaskId = 0;
	}
	for (auto &udpMirrorClient : mUdpMirrorClients)
		udpMirrorClient.stop();
	mUdpMirrorClients.clear();
	mCurrentUdpMirrorClient = mUdpMirrorClients.end();
	if (mTunnelClient) mTunnelClient->cleanServers();
}

void TunnelManager::enableDualMode(bool enable) {
	mUseDualClient = enable;
}

bool TunnelManager::isDualModeEnabled() {
	return mUseDualClient;
}

void TunnelManager::reconnect() {
	if (mTunnelClient) mTunnelClient->reconnect();
}

static void sCloseRtpTransport(RtpTransport *t) {
	DualSocket *ds = (DualSocket *)t->data;
	TunnelSocket *sendSocket = ds->sendSocket;
	TunnelSocket *recvSocket = ds->recvSocket;
	TunnelManager *manager = (TunnelManager *)sendSocket->getUserPointer();
	manager->closeRtpTransport(t, sendSocket);
	manager->closeRtpTransport(t, recvSocket);
	ms_free(ds);
}

void TunnelManager::closeRtpTransport(BCTBX_UNUSED(RtpTransport *t), TunnelSocket *s) {
	mTunnelClient->closeSocket(s);
}

static RtpTransport *sCreateRtpTransport(void *userData, int port) {
	return ((TunnelManager *)userData)->createRtpTransport(port);
}

void sDestroyRtpTransport(RtpTransport *t) {
	ms_free(t);
}

RtpTransport *TunnelManager::createRtpTransport(int port) {
	DualSocket *dualSocket = ms_new0(DualSocket, 1);
	if (!mUseDualClient) {
		TunnelSocket *socket = ((TunnelClient *)mTunnelClient)->createSocket(port);
		socket->setUserPointer(this);
		dualSocket->sendSocket = socket;
		dualSocket->recvSocket = socket;
	} else {
		dualSocket->sendSocket = ((DualTunnelClient *)mTunnelClient)->createSocket(TunnelSendOnly, port);
		dualSocket->sendSocket->setUserPointer(this);
		dualSocket->recvSocket = ((DualTunnelClient *)mTunnelClient)->createSocket(TunnelRecvOnly, port);
		dualSocket->recvSocket->setUserPointer(this);
	}

	RtpTransport *t = ms_new0(RtpTransport, 1);
	t->t_getsocket = NULL;
	t->t_recvfrom = customRecvfrom;
	t->t_sendto = customSendto;
	t->t_close = sCloseRtpTransport;
	t->t_destroy = sDestroyRtpTransport;
	t->data = dualSocket;
	ms_message("Creating tunnel RTP transport for local virtual port %i", port);
	return t;
}

bool TunnelManager::isTunnelRtpTransport(const RtpTransport *tp) const {
	return tp->t_recvfrom == customRecvfrom && tp->t_sendto == customSendto;
}

void TunnelManager::setUsername(const char *username) {
	mUsername = std::string(username == NULL ? "" : username);
}

const std::string &TunnelManager::getUsername() const {
	return mUsername;
}

void TunnelManager::setDomain(const char *domain) {
	mDomain = std::string(domain == NULL ? "" : domain);
}

const std::string &TunnelManager::getDomain() const {
	return mDomain;
}

bctbx_x509_certificate_t *TunnelManager::getCertificate() const {
	return mCertificate;
}

void TunnelManager::setCertificate(bctbx_x509_certificate_t *certificate) {
	mCertificate = certificate;
}

bctbx_signing_key_t *TunnelManager::getKey() const {
	return mKey;
}

void TunnelManager::setKey(bctbx_signing_key_t *key) {
	mKey = key;
}

int TunnelManager::tlsUpdateClientCertificate() {
	int error;

	if (!getCertificate() || !getKey()) {
		const LinphoneAuthInfo *authInfo =
		    _linphone_core_find_indexed_tls_auth_info(getLinphoneCore(), getUsername().c_str(), getDomain().c_str());
		if (authInfo == NULL) {
			ms_error("TunnelManager: Cannot find auth info for user '%s'", getUsername().c_str());
			return -1;
		}

		if (!getCertificate()) {
			bctbx_x509_certificate_t *cert = bctbx_x509_certificate_new();
			if (linphone_auth_info_get_tls_cert(authInfo)) {
				const char *cert_buffer = linphone_auth_info_get_tls_cert(authInfo);
				if ((error = bctbx_x509_certificate_parse(cert, cert_buffer, strlen(cert_buffer) + 1)) < 0) {
					char tmp[128];
					bctbx_strerror(error, tmp, sizeof(tmp));

					ms_error("TunnelManager: Cannot parse certificate for user '%s' : %s", getUsername().c_str(), tmp);
					bctbx_x509_certificate_free(cert);
					return -1;
				}
			} else if (linphone_auth_info_get_tls_cert_path(authInfo)) {
				if ((error = bctbx_x509_certificate_parse_file(cert, linphone_auth_info_get_tls_cert_path(authInfo))) <
				    0) {
					char tmp[128];
					bctbx_strerror(error, tmp, sizeof(tmp));

					ms_error("TunnelManager: Cannot parse certificate file for user '%s' : %s", getUsername().c_str(),
					         tmp);
					bctbx_x509_certificate_free(cert);
					return -1;
				}
			} else {
				ms_error("TunnelManager: Cannot find TLS cert in auth info for user '%s'", getUsername().c_str());
				bctbx_x509_certificate_free(cert);
				return -1;
			}
			setCertificate(cert);
		}

		if (!getKey()) {
			bctbx_signing_key_t *key = bctbx_signing_key_new();
			if (linphone_auth_info_get_tls_key(authInfo)) {
				const char *key_buffer = linphone_auth_info_get_tls_key(authInfo);
				const char *passwd = linphone_auth_info_get_tls_key_password(authInfo);
				if ((error = bctbx_signing_key_parse(key, key_buffer, strlen(key_buffer) + 1,
				                                     (const unsigned char *)passwd, passwd ? strlen(passwd) : 0)) < 0) {
					char tmp[128];
					bctbx_strerror(error, tmp, sizeof(tmp));

					ms_error("TunnelManager: Cannot parse TLS key for user '%s' : %s", getUsername().c_str(), tmp);
					bctbx_signing_key_free(key);
					return -1;
				}
			} else if (linphone_auth_info_get_tls_key_path(authInfo)) {
				if ((error = bctbx_signing_key_parse_file(key, linphone_auth_info_get_tls_key_path(authInfo),
				                                          linphone_auth_info_get_tls_key_password(authInfo))) < 0) {
					char tmp[128];
					bctbx_strerror(error, tmp, sizeof(tmp));

					ms_error("TunnelManager: Cannot parse TLS key file for user '%s' : %s", getUsername().c_str(), tmp);
					bctbx_signing_key_free(key);
					return -1;
				}
			} else {
				ms_error("TunnelManager: Cannot find TLS key in auth info for user '%s'", getUsername().c_str());
				bctbx_signing_key_free(key);
				return -1;
			}

			setKey(key);
		}
	}

	return 0;
}

void TunnelManager::startClient() {
	ms_message("TunnelManager: Starting tunnel client");
	if (!mTunnelClient) {
		if (mUseDualClient) {
			mTunnelClient = DualTunnelClient::create(TRUE);
		} else {
			mTunnelClient = TunnelClient::create(TRUE);
		}

		mCore->sal->setTunnel(mTunnelClient);
		if (!mUsername.empty()) {
			tlsUpdateClientCertificate();
		}

		if (!mUseDualClient) {
			static_cast<TunnelClient *>(mTunnelClient)->setStateCallback(tunnelCallback, this);
			if (getCertificate() != NULL && getKey() != NULL) {
				static_cast<TunnelClient *>(mTunnelClient)->setClientCertificate(getCertificate(), getKey());
			}
		} else {
			static_cast<DualTunnelClient *>(mTunnelClient)->setStateCallback(tunnelCallback2, this);
			if (getCertificate() != NULL && getKey() != NULL) {
				static_cast<DualTunnelClient *>(mTunnelClient)->setClientCertificate(getCertificate(), getKey());
			}
		}
	}

	if (mVerifyServerCertificate) {
		const char *rootCertificatePath = linphone_core_get_root_ca(mCore);
		if (rootCertificatePath != NULL) {
			ms_message("TunnelManager: Load root certificate from %s", rootCertificatePath);
			mTunnelClient->setRootCertificate(
			    rootCertificatePath); /* give the path to root certificate to the tunnel client in order to be able to
			                             verify the server certificate */
		} else {
			ms_warning("TunnelManager is set to verify server certificate but no root certificate is available in "
			           "linphoneCore");
		}
	}
	mTunnelClient->cleanServers();
	if (mUseDualClient) {
		list<DualServerAddr>::iterator it;
		for (it = mDualServerAddrs.begin(); it != mDualServerAddrs.end(); ++it) {
			const DualServerAddr &addr = *it;
			static_cast<DualTunnelClient *>(mTunnelClient)
			    ->addServerPair(addr.mAddr1.c_str(), addr.mPort1, addr.mAddr2.c_str(), addr.mPort2);
		}
	} else {
		list<ServerAddr>::iterator it;
		for (it = mServerAddrs.begin(); it != mServerAddrs.end(); ++it) {
			const ServerAddr &addr = *it;
			static_cast<TunnelClient *>(mTunnelClient)->addServer(addr.mAddr.c_str(), addr.mPort);
		}
	}

	mTunnelClient->setHttpProxy(mHttpProxyHost.c_str(), mHttpProxyPort, mHttpUserName.c_str(), mHttpPasswd.c_str());
	if (!mTunnelClient->isStarted()) {
		ms_message("Starting tunnel client");
		mTunnelClient->start();
	} else {
		ms_message("Reconnecting tunnel client");
		mTunnelClient->reconnect(); /*force a reconnection to take into account new parameters*/
	}
}

void TunnelManager::stopClient() {
	if (mTunnelClient) {
		ms_message("TunnelManager: stoppping tunnel client");
		mTunnelClient->stop();

		/* We only delete the tunnel client if there is no call running */
		if (linphone_core_get_calls_nb(mCore) == 0) {
			delete mTunnelClient;
			mTunnelClient = NULL;
		}
	}
}

bool TunnelManager::isConnected() const {
	return mState == On;
}

int TunnelManager::customSendto(
    struct _RtpTransport *t, mblk_t *msg, BCTBX_UNUSED(int flags), const struct sockaddr *to, socklen_t tolen) {
	int size;
	DualSocket *ds = (DualSocket *)t->data;
	msgpullup(msg, (size_t)-1);
	size = (int)msgdsize(msg);
	ds->sendSocket->sendto(msg->b_rptr, (size_t)size, to, tolen);
	return size;
}

int TunnelManager::customRecvfrom(
    struct _RtpTransport *t, mblk_t *msg, BCTBX_UNUSED(int flags), struct sockaddr *from, socklen_t *fromlen) {
	DualSocket *ds = (DualSocket *)t->data;
	memset(&msg->recv_addr, 0, sizeof(msg->recv_addr));
	long err = ds->recvSocket->recvfrom(msg->b_wptr, (size_t)(dblk_lim(msg->b_datap) - dblk_base(msg->b_datap)), from,
	                                    *fromlen);
	// to make ice happy
	inet_aton(((TunnelManager *)(ds->recvSocket)->getUserPointer())->mLocalAddr, &msg->recv_addr.addr.ipi_addr);
	msg->recv_addr.family = AF_INET;
	msg->recv_addr.port = htons((unsigned short)(ds->recvSocket)->getPort());
	if (err > 0) return (int)err;
	return 0;
}

TunnelManager::TunnelManager(LinphoneCore *lc)
    : mCore(lc), mMode(LinphoneTunnelModeDisable), mTunnelClient(NULL), mHttpProxyPort(0), mCoreCbs(NULL),
      mLongRunningTaskId(0), mSimulateUdpLoss(false), mUseDualClient(false) {
	mIterateTimer = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->createTimer(
	    [this]() -> bool {
		    this->onIterate();
		    return true;
	    },
	    20, "Tunnel timer");
	mTransportFactories.audio_rtcp_func = sCreateRtpTransport;
	mTransportFactories.audio_rtcp_func_data = this;
	mTransportFactories.audio_rtp_func = sCreateRtpTransport;
	mTransportFactories.audio_rtp_func_data = this;
	mTransportFactories.video_rtcp_func = sCreateRtpTransport;
	mTransportFactories.video_rtcp_func_data = this;
	mTransportFactories.video_rtp_func = sCreateRtpTransport;
	mTransportFactories.video_rtp_func_data = this;
	mCoreCbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_network_reachable(mCoreCbs, networkReachableCb);
	linphone_core_cbs_set_global_state_changed(mCoreCbs, globalStateChangedCb);
	linphone_core_cbs_set_user_data(mCoreCbs, this);
	_linphone_core_add_callbacks(mCore, mCoreCbs, true);
	linphone_core_get_local_ip_for(AF_INET, NULL, mLocalAddr);
	mAutodetectionRunning = false;
	mState = Off;
	mTargetState = Off;
	mStarted = false;
	mTunnelizeSipPackets = true;
	mCertificate = NULL;
	mKey = NULL;
}
void TunnelManager::unlinkLinphoneCore() {
	if (mCore) {
		stopClient();
		if (mCore->sal) {
			mCore->sal->setTunnel(NULL);
			L_GET_CPP_PTR_FROM_C_OBJECT(mCore)->destroyTimer(mIterateTimer);
		}
		linphone_core_remove_callbacks(mCore, mCoreCbs);
		linphone_core_cbs_unref(mCoreCbs);
		linphone_core_set_rtp_transport_factories(mCore, NULL);
		mCore = nullptr;
		mCoreCbs = nullptr;
	} else {
		ms_message("Core already cleaned up");
	}
}
TunnelManager::~TunnelManager() {
	if (mLongRunningTaskId > 0) {
		sal_end_background_task(mLongRunningTaskId);
		mLongRunningTaskId = 0;
	}
	for (UdpMirrorClientList::iterator udpMirror = mUdpMirrorClients.begin(); udpMirror != mUdpMirrorClients.end();
	     udpMirror++) {
		udpMirror->stop();
	}
	if (mCertificate) bctbx_x509_certificate_free(mCertificate);
	if (mKey) bctbx_signing_key_free(mKey);

	unlinkLinphoneCore();
}

void TunnelManager::doRegistration() {
	LinphoneAccount *account = linphone_core_get_default_account(mCore);
	if (account) {
		ms_message("TunnelManager: New registration");
		Account::toCpp(account)->setNeedToRegister(true);
	}
}

void TunnelManager::doUnregistration() {
	LinphoneAccount *account = linphone_core_get_default_account(mCore);
	if (account) {
		Account::toCpp(account)->unregister();
	}
}

void TunnelManager::tunnelizeLiblinphone() {
	ms_message("LinphoneCore goes into tunneled mode.");
	mState = On; /*do this first because _linphone_core_apply_transports() will use it to know if tunnel listening point
	                is to be used*/
	linphone_core_set_rtp_transport_factories(mCore, &mTransportFactories);
	if (mTunnelizeSipPackets) {
		doUnregistration();
		_linphone_core_apply_transports(mCore);
		doRegistration();
	}
}

void TunnelManager::untunnelizeLiblinphone() {
	ms_message("LinphoneCore leaves tunneled mode.");
	mState = Off;
	linphone_core_set_rtp_transport_factories(mCore, NULL);
	if (mTunnelizeSipPackets) {
		doUnregistration();
		_linphone_core_apply_transports(mCore);
		doRegistration();
	}
}

void TunnelManager::applyState() {
	if (!linphone_core_is_network_reachable(mCore)) return;
	if (mTargetState == On && mState == Off) {
		if (!mTunnelClient || !mTunnelClient->isStarted()) {
			startClient();
		}
		if (mTunnelClient->isReady()) tunnelizeLiblinphone();
	} else if (mTargetState == Off && mState == On) {
		untunnelizeLiblinphone();
		stopClient();
	}
}

void TunnelManager::setState(TunnelManager::State state) {
	mTargetState = state;
	applyState();
}

void TunnelManager::processTunnelEvent(const Event &ev) {
	if (ev.mData.mConnected) {
		ms_message("TunnelManager: tunnel is connected");
		applyState();
	} else {
		ms_error("TunnelManager: tunnel has been disconnected");
	}
}

void TunnelManager::applyMode() {
	switch (mMode) {
		case LinphoneTunnelModeEnable:
			stopAutoDetection();
			setState(On);
			break;
		case LinphoneTunnelModeDisable:
			stopAutoDetection();
			setState(Off);
			break;
		case LinphoneTunnelModeAuto:
			if (linphone_core_is_network_reachable(mCore)) startAutoDetection();
			break;
		default:
			ms_error("TunnelManager::setMode(): invalid mode (%d)", (int)mMode);
	}
}

void TunnelManager::setMode(LinphoneTunnelMode mode) {
	if (mMode == mode) return;
	ms_message("TunnelManager: switching mode from %s to %s", linphone_tunnel_mode_to_string(mMode),
	           linphone_tunnel_mode_to_string(mode));
	mMode = mode;
	applyMode();
}

void TunnelManager::stopLongRunningTask() {
	if (mLongRunningTaskId != 0) {
		sal_end_background_task(mLongRunningTaskId);
		mLongRunningTaskId = 0;
	}
}

void TunnelManager::tunnelCallback(bool connected, void *user_pointer) {
	TunnelManager *zis = static_cast<TunnelManager *>(user_pointer);
	Event ev;

	ev.mType = TunnelEvent;
	ev.mData.mConnected = connected;
	zis->postEvent(ev);
}

void TunnelManager::tunnelCallback2(BCTBX_UNUSED(TunnelDirection direction), bool connected, void *user_pointer) {
	TunnelManager *zis = static_cast<TunnelManager *>(user_pointer);
	Event ev;

	ev.mType = TunnelEvent;
	ev.mData.mConnected = connected;
	zis->postEvent(ev);
}

void TunnelManager::onIterate() {
	mMutex.lock();
	while (!mEvq.empty()) {
		Event ev = mEvq.front();
		mEvq.pop();
		mMutex.unlock();
		if (ev.mType == TunnelEvent) processTunnelEvent(ev);
		else if (ev.mType == UdpMirrorClientEvent) {
			processUdpMirrorEvent(ev);
		}
		mMutex.lock();
	}
	mMutex.unlock();
}

LinphoneTunnelMode TunnelManager::getMode() const {
	return mMode;
}

void TunnelManager::processUdpMirrorEvent(const Event &ev) {
	if (mAutodetectionRunning == false)
		return; /*auto detection was cancelled, for example by switching to disabled state*/
	if (mSimulateUdpLoss || !ev.mData.mHaveUdp) {
		if (mSimulateUdpLoss) {
			ms_message("TunnelManager: simulate UDP lost on %s:%d",
			           mCurrentUdpMirrorClient->getServerAddress().mAddr.c_str(),
			           mCurrentUdpMirrorClient->getServerAddress().mPort);
		} else {
			ms_message("TunnelManager: UDP mirror test failed on %s:%d",
			           mCurrentUdpMirrorClient->getServerAddress().mAddr.c_str(),
			           mCurrentUdpMirrorClient->getServerAddress().mPort);
		}
		mCurrentUdpMirrorClient++;
		if (mCurrentUdpMirrorClient != mUdpMirrorClients.end()) {
			ms_message("TunnelManager: trying another UDP mirror on %s:%d",
			           mCurrentUdpMirrorClient->getServerAddress().mAddr.c_str(),
			           mCurrentUdpMirrorClient->getServerAddress().mPort);
			UdpMirrorClient &lUdpMirrorClient = *mCurrentUdpMirrorClient;
			lUdpMirrorClient.start(TunnelManager::sUdpMirrorClientCallback, (void *)this);
			mAutodetectionRunning = true;
			return;
		} else {
			ms_message("TunnelManager: all UDP mirror tests failed");
			setState(On);
		}
	} else {
		ms_message("TunnelManager: UDP mirror test success on %s:%d",
		           mCurrentUdpMirrorClient->getServerAddress().mAddr.c_str(),
		           mCurrentUdpMirrorClient->getServerAddress().mPort);
		setState(Off);
	}
	mAutodetectionRunning = false;
	stopLongRunningTask();
}

void TunnelManager::postEvent(const Event &ev) {
	mMutex.lock();
	mEvq.push(ev);
	mMutex.unlock();
}

void TunnelManager::sUdpMirrorClientCallback(bool isUdpAvailable, void *data) {
	TunnelManager *thiz = (TunnelManager *)data;
	Event ev;
	ev.mType = UdpMirrorClientEvent;
	ev.mData.mHaveUdp = isUdpAvailable;
	thiz->postEvent(ev);
}

void TunnelManager::networkReachableCb(LinphoneCore *lc, bool_t reachable) {
	TunnelManager *tunnel = bcTunnel(linphone_core_get_tunnel(lc));

	if (reachable) {
		ms_message("TunnelManager: Network is reachable, starting tunnel client");
		linphone_core_get_local_ip_for(AF_INET, NULL, tunnel->mLocalAddr);
		if (tunnel->getMode() == LinphoneTunnelModeAuto) {
			tunnel->startAutoDetection();
			/*autodetection will call applyState() when finished*/
		} else {
			tunnel->applyState();
		}
	} else if (!reachable) {
		ms_message("TunnelManager: Network is unreachable, stopping tunnel client");
		// if network is no more reachable, cancel autodetection if any
		tunnel->stopAutoDetection();
		// turn off the tunnel connection
		tunnel->untunnelizeLiblinphone();
		tunnel->stopClient();
	}
}

void TunnelManager::globalStateChangedCb(LinphoneCore *lc,
                                         LinphoneGlobalState gstate,
                                         BCTBX_UNUSED(const char *message)) {
	if (gstate == LinphoneGlobalOff) {
		ms_message("Core [%p] is Off, unlinking TunnelManager to core", lc);
		// calling same core as for object destruction
		TunnelManager *thiz = (TunnelManager *)linphone_core_cbs_get_user_data(linphone_core_get_current_callbacks(lc));
		if (thiz) thiz->unlinkLinphoneCore();
	}
}

void TunnelManager::stopAutoDetection() {
	if (mAutodetectionRunning) {
		for (UdpMirrorClientList::iterator udpMirror = mUdpMirrorClients.begin(); udpMirror != mUdpMirrorClients.end();
		     udpMirror++) {
			udpMirror->stop();
		}
		mAutodetectionRunning = false;
		stopLongRunningTask();
	}
}

bool TunnelManager::startAutoDetection() {
	if (mUdpMirrorClients.empty()) {
		ms_error("TunnelManager: No UDP mirror server configured aborting auto detection");
		return false;
	}
	ms_message("TunnelManager: Starting auto-detection");
	mCurrentUdpMirrorClient = mUdpMirrorClients.begin();
	if (mLongRunningTaskId == 0) mLongRunningTaskId = sal_begin_background_task("Tunnel auto detect", NULL, NULL);
	UdpMirrorClient &lUdpMirrorClient = *mCurrentUdpMirrorClient;
	mAutodetectionRunning = true;
	lUdpMirrorClient.start(TunnelManager::sUdpMirrorClientCallback, (void *)this);
	return true;
}

bool TunnelManager::isActivated() const {
	return mState == On;
}

void TunnelManager::setHttpProxyAuthInfo(const char *username, const char *passwd) {
	mHttpUserName = username ? username : "";
	mHttpPasswd = passwd ? passwd : "";
	if (mTunnelClient) mTunnelClient->setHttpProxyAuthInfo(username, passwd);
}

void TunnelManager::tunnelizeSipPackets(bool enable) {
	mTunnelizeSipPackets = enable;
}

bool TunnelManager::tunnelizeSipPacketsEnabled() const {
	return mTunnelizeSipPackets;
}

void TunnelManager::verifyServerCertificate(bool enable) {
	mVerifyServerCertificate = enable;
}

bool TunnelManager::verifyServerCertificateEnabled() const {
	return mVerifyServerCertificate;
}

void TunnelManager::setHttpProxy(const char *host, int port, const char *username, const char *passwd) {
	mHttpUserName = username ? username : "";
	mHttpPasswd = passwd ? passwd : "";
	mHttpProxyPort = (port > 0) ? port : 0;
	mHttpProxyHost = host ? host : "";
	if (mTunnelClient) mTunnelClient->setHttpProxy(host, port, username, passwd);
}

LinphoneCore *TunnelManager::getLinphoneCore() const {
	return mCore;
}

void TunnelManager::simulateUdpLoss(bool enabled) {
	mSimulateUdpLoss = enabled;
}
