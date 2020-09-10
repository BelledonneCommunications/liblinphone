/*
 *  C Implementation: tunnel
 *
 * Description:
 *
 *
 *
 *Copyright (C) 2011  Belledonne Comunications, Grenoble, France
 */

#ifndef __TUNNEL_CLIENT_MANAGER_H__
#define __TUNNEL_CLIENT_MANAGER_H__
#include <list>
#include <string>
#include <tunnel/client.hh>
#include <tunnel/udp_mirror.hh>
#include "linphone/core.h"
#include "linphone/tunnel.h"
#include "bctoolbox/crypto.h"

#ifndef USE_BELLESIP
extern "C" {
	#include <eXosip2/eXosip_transport_hook.h>
}
#endif

namespace belledonnecomm {
/**
 * @addtogroup tunnel_client
 * @{
**/
	struct DualSocket {
		TunnelSocket *sendSocket;
		TunnelSocket *recvSocket;
	};
		
	/**
	 * The TunnelManager class extends the LinphoneCore functionnality in order to provide an easy to use API to
	 * - provision tunnel servers ip addresses and ports
	 * - start/stop the tunneling service
	 * - be informed of connection and disconnection events to the tunnel server
	 * - perform auto-detection whether tunneling is required, based on a test of sending/receiving a flow of UDP packets.
	 *
	 * It takes in charge automatically the SIP registration procedure when connecting or disconnecting to a tunnel server.
	 * No other action on LinphoneCore is required to enable full operation in tunnel mode.
	**/
	class TunnelManager {

	public:
		/**
		 * Add a tunnel server. At least one should be provided to be able to connect.
		 * When several addresses are provided, the tunnel client may try each of them until it gets connected.
		 *
		 * @param ip tunnMethod definition for '-isInitialStateOn' not foundel server ip address
		 * @param port tunnel server tls port, recommended value is 443
		 */
		void addServer(const char *ip, int port);
		/**
		 *Add tunnel server with auto detection capabilities
		 *
		 * @param ip tunnel server ip address
		 * @param port tunnel server tls port, recommended value is 443
		 * @param udpMirrorPort remote port on the tunnel server side  used to test udp reachability
		 * @param delay udp packet round trip delay in ms considered as acceptable. recommended value is 1000 ms.
		 */
		void addServer(const char *ip, int port,unsigned int udpMirrorPort,unsigned int delay);
		/**
		 * Add a tunnel server couple. At least one should be provided to be able to connect.
		 * This is used when using the dual socket mode where one client will connect to one ip and the other client to the other ip.
		 *
		 * @param ip1 server ip address n°1
		 * @param port1 tunnel server tls port, recommended value is 443
		 * @param ip2 server ip address n°2
		 * @param port2 tunnel server tls port, recommended value is 443
		 * @param udpMirrorPort remote port on the tunnel server 1 side  used to test udp reachability
		 * @param delay udp packet round trip delay in ms considered as acceptable. recommended value is 1000 ms.
		 */
		void addServerPair(const char *ip1, int port1, const char *ip2, int port2, unsigned int udpMirrorPort, unsigned int delay);
		/**
		 * Add a tunnel server couple. At least one should be provided to be able to connect.
		 * This is used when using the dual socket mode where one client will connect to one ip and the other client to the other ip.
		 *
		 * @param ip1 server ip address n°1
		 * @param port1 tunnel server tls port, recommended value is 443
		 * @param ip2 server ip address n°2
		 * @param port2 tunnel server tls port, recommended value is 443
		 */
		void addServerPair(const char *ip1, int port1, const char *ip2, int port2);
		/**
		 * Removes all tunnel server address previously entered with addServer()
		**/
		void cleanServers();
		
		/**
		 * Enables the dual socket mode. In this mode, we have to configure pairs or ServerAddr
		 * 2 TunneClient will be used, one for each IP and each one will only either send or receive the data stream.
		 * @param enable true to enable the DualMode, false otherwise
		 */
		void enableDualMode(bool enable);
		/**
		 * Returns whether or not the DualMode is enabled
		 * @return true if it is enabled, false otherwise
		 */
		bool isDualModeEnabled();
		/**
		 * Forces reconnection to the tunnel server.
		 * This method is useful when the device switches from wifi to Edge/3G or vice versa. In most cases the tunnel client socket
		 * won't be notified promptly that its connection is now zombie, so it is recommended to call this method that will cause
		 * the lost connection to be closed and new connection to be issued.
		**/
		void reconnect();
		/**
		 * @brief setMode
		 * @param mode
		 */
		void setMode(LinphoneTunnelMode mode);
		/**
		 * @brief Return the tunnel mode
		 * @return #LinphoneTunnelMode
		 */
		LinphoneTunnelMode getMode() const;
		/**
		 * iOS only feature: specify http proxy credentials.
		 * When the iOS device has an http proxy configured in the iOS settings, the tunnel client will connect to the server
		 * through this http proxy. Credentials might be needed depending on the proxy configuration.
		 * @param username The username.
		 * @param passwd The password.
		**/
		void setHttpProxyAuthInfo(const char* username,const char* passwd);
		void setHttpProxy(const char *host,int port, const char *username, const char *passwd);
		/**
		 * Indicate to the tunnel manager whether SIP packets must pass
		 * through the tunnel. That featurte is automatically enabled at
		 * the creation of the TunnelManager instance.
		 * @param enable If set to TRUE, SIP packets will pass through the tunnel.
		 * If set to FALSE, SIP packets will pass by the configured proxies.
		 */
		void tunnelizeSipPackets(bool enable);
		/**
		 * @brief Check whether the tunnel manager is set to tunnelize SIP packets
		 * @return True, SIP packets pass through the tunnel
		 */
		bool tunnelizeSipPacketsEnabled() const;
		/**
		 * Indicate to the tunnel manager wether server certificate
		 * must be verified during TLS handshake. Default: disabled
		 * @param enable If set to TRUE, SIP packets will pass through the tunnel.
		 * If set to FALSE, SIP packets will pass by the configured proxies.
		 */
		void verifyServerCertificate(bool enable);
		/**
		 * Check wether the tunnel manager is set to verify server certificate during TLS handshake
		 * @return True, server certificate is verified(using the linphonecore root certificate)
		 */
		bool verifyServerCertificateEnabled() const;
		/**
		 * @brief Constructor
		 * @param lc The LinphoneCore instance of which the TunnelManager will be associated to.
		 */
		TunnelManager(LinphoneCore* lc);
		/**
		 * @brief Destructor
		 */
		~TunnelManager();
		/**
		 * @brief Create an RtpTransport
		 * @param port
		 * @return
		 */
		RtpTransport *createRtpTransport(int port);
		/**
		 * @brief Destroy the given RtpTransport
		 * @param t
		 * @param s
		 */
		void closeRtpTransport(RtpTransport *t, TunnelSocket *s);
		/**
		 * @brief Get associated Linphone Core
		 * @return pointer on the associated LinphoneCore
		 */
		LinphoneCore *getLinphoneCore() const;
		/**
		 * @brief Check wehter the tunnel is connected
		 * @return True whether the tunnel is connected
		 */
		bool isConnected() const;

		bool isActivated() const;

		void simulateUdpLoss(bool enabled);

		void setUsername(const char* username);
		const std::string& getUsername() const;
		void setDomain(const char *domain);
		const std::string& getDomain() const;

	private:
		enum EventType{
			UdpMirrorClientEvent,
			TunnelEvent,
		};
		struct Event{
			EventType mType;
			union EventData{
				bool mConnected;
				bool mHaveUdp;
			}mData;
		};
		typedef std::list<UdpMirrorClient> UdpMirrorClientList;
		static int customSendto(struct _RtpTransport *t, mblk_t *msg , int flags, const struct sockaddr *to, socklen_t tolen);
		static int customRecvfrom(struct _RtpTransport *t, mblk_t *msg, int flags, struct sockaddr *from, socklen_t *fromlen);
		static void tunnelCallback(bool connected, void *zis);
		static void tunnelCallback2(TunnelDirection direction, bool connected, void *zis);
		static bool_t sOnIterate(TunnelManager *zis);
		static void sUdpMirrorClientCallback(bool result, void* data);
		static void networkReachableCb(LinphoneCore *lc, bool_t reachable);
		static void globalStateChangedCb(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message);
		static int tlsCallbackClientCertificate(void *data, bctbx_ssl_context_t *ctx, const bctbx_list_t *names);
		

	private:
		enum State{
			Off, /*no tunneling */
			On /*tunneling activated*/
		};
		void onIterate();
		void doRegistration();
		void doUnregistration();
		void startClient();
		bool startAutoDetection();
		void processTunnelEvent(const Event &ev);
		void processUdpMirrorEvent(const Event &ev);
		void postEvent(const Event &ev);
		void stopClient();
		void stopAutoDetection();
		void stopLongRunningTask();
		void applyMode();
		void setState(State state);
		void applyState();
		void tunnelizeLiblinphone();
		void untunnelizeLiblinphone();
		void unlinkLinphoneCore();
		bctbx_x509_certificate_t *getCertificate() const;
		void setCertificate(bctbx_x509_certificate_t *certificate);
		bctbx_signing_key_t *getKey() const;
		void setKey(bctbx_signing_key_t *key);
	private:
		
		LinphoneCore* mCore;
		LinphoneTunnelMode mMode;
		TunnelClientI* mTunnelClient;
		std::string mHttpUserName;
		std::string mHttpPasswd;
		std::string mHttpProxyHost;
		int mHttpProxyPort;
		LinphoneCoreCbs *mCoreCbs;
		std::list<ServerAddr> mServerAddrs;
		std::list<DualServerAddr> mDualServerAddrs;
		UdpMirrorClientList mUdpMirrorClients;
		UdpMirrorClientList::iterator mCurrentUdpMirrorClient;
		LinphoneRtpTransportFactories mTransportFactories;
		Mutex mMutex;
		std::queue<Event> mEvq;
		char mLocalAddr[64];
		unsigned long mLongRunningTaskId;
		State mTargetState;
		State mState;
		bool mVerifyServerCertificate;
		bool mStarted;
		bool mAutodetectionRunning;
		bool mTunnelizeSipPackets;
		bool mSimulateUdpLoss;
		bool mUseDualClient;
		std::string mUsername;
		std::string mDomain;
		bctbx_x509_certificate_t *mCertificate;
		bctbx_signing_key_t *mKey;
	};

/**
 * @}
**/

}



#endif /*__TUNNEL_CLIENT_MANAGER_H__*/
