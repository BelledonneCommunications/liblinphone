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

#ifndef _L_C_WRAPPER_H_
#define _L_C_WRAPPER_H_

#include "linphone/api/c-types.h"
#include "logger/logger.h"

/*
 * This include file provides routines for C to C++ mapping of objects within the library.
 * The liblinphone primary interface is C, so that it can be wrapped in the most large set of "modern" languages.
 * However, internally liblinphone heavily uses C++.
 * It is convenient that C types exposed in the C API have a mapping mechanism with their C++ implementation.
 * The HybridObject templates is the choosen solution. See examples (LinphoneCall, LinphoneAuthInfo, LinphoneConference...).
 * 
 * Formely, a more complex mapping mechanism was in use in liblinphone, based on C macros and a Public/Private pattern.
 * It is provided by file "internal/c-tools.h", included below.
 * This model is now obsolete and replaced by HybridObject<>. It shall not be used anymore in newly written code.
 */


// Convertions integer to pointer and viceversa, useful to store integers into bctbx_list_t.
#define LINPHONE_INT_TO_PTR(x)  ((void*)(intptr_t)(x))
#define LINPHONE_PTR_TO_INT(x)  ((int)(intptr_t)(x))

#include "belle-sip/object++.hh"

/* 
 * Macros to invoke callbacks owned by an HybridObject derived type.
 */

#define LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C(cppType, cppObject, cbGetter, ...) \
	do{ \
		bctbx_list_t *callbacksCopy = bctbx_list_copy_with_data(cppObject->getCallbacksList(), (bctbx_list_copy_func)belle_sip_object_ref); \
		for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) { \
			Linphone ## cppType ## Cbs *cbs = static_cast< Linphone ## cppType ## Cbs *>(bctbx_list_get_data(it)); \
			cppObject->setCurrentCallbacks(cbs); \
			auto cb = cbGetter (cbs); \
			if (cb) \
				cb(cppObject->toC(), __VA_ARGS__); \
		} \
		cppObject->setCurrentCallbacks(nullptr); \
		bctbx_list_free_with_data(callbacksCopy, (bctbx_list_free_func)belle_sip_object_unref);\
	}while(0)

#define LINPHONE_HYBRID_OBJECT_INVOKE_CBS_WITH_C_NO_ARG(cppType, cppObject, cbGetter)\
	do{ \
		bctbx_list_t *callbacksCopy = bctbx_list_copy_with_data(cppObject->getCallbacksList(), (bctbx_list_copy_func)belle_sip_object_ref); \
		for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) { \
			Linphone ## cppType ## Cbs *cbs = static_cast< Linphone ## cppType ## Cbs *>(bctbx_list_get_data(it)); \
			cppObject->setCurrentCallbacks(cbs); \
			auto cb = cbGetter (cbs); \
			if (cb) \
				cb(cppObject->toC()); \
		} \
		cppObject->setCurrentCallbacks(nullptr); \
		bctbx_list_free_with_data(callbacksCopy, (bctbx_list_free_func)belle_sip_object_unref);\
	}while(0)

#define LINPHONE_HYBRID_OBJECT_INVOKE_CBS(cppType, cppObject, cbGetter, ...) \
	do{ \
		std::list< std::shared_ptr< cppType ## Cbs > > callbacksCopy = cppObject->getCallbacksList(); \
		for (auto & cbs : callbacksCopy) { \
			if (cbs->isActive()){ \
				cppObject->setCurrentCallbacks(cbs); \
				auto cb = cbGetter (cbs->toC()); \
				if (cb) \
					cb(cppObject->toC(), __VA_ARGS__); \
			}\
		} \
		cppObject->setCurrentCallbacks(nullptr); \
	}while(0)

#define LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(cppType, cppObject, cbGetter) \
	do{ \
		std::list< std::shared_ptr< cppType ## Cbs > > callbacksCopy = cppObject->getCallbacksList(); \
		for (auto & cbs : callbacksCopy) { \
			if (cbs->isActive()){ \
				cppObject->setCurrentCallbacks(cbs); \
				auto cb = cbGetter (cbs->toC()); \
				if (cb) \
					cb(cppObject->toC()); \
			}\
		} \
		cppObject->setCurrentCallbacks(nullptr); \
	}while(0)

LINPHONE_BEGIN_NAMESPACE

/* Trivial class for providing setUserData()/getUserData() to hybrid objects used in liblinphone.*/
class LINPHONE_PUBLIC UserDataAccessor{
public:
	void *getUserData()const;
	void setUserData(void *ud);
private:
	void *mUserData = nullptr;
};

/*
 * Base class for all '*Cbs' classes.
 */
class LINPHONE_PUBLIC Callbacks : public UserDataAccessor{
	public:
		inline void setActive(bool active){
			mIsActive = active;
		}
		inline bool isActive()const{
			return mIsActive;
		}
	private:
		bool mIsActive = true;
};

/*
 * Template for easy conversion from std::list to bctbx_list_t.
 * The authority list is the C++ one, the C one being only a const view of it.
 * _T must be an HybridObject; so that conversion from C++ type to C type is done automatically.
 */
template <typename _T>
class ListHolder {
public:
	public:
		ListHolder() = default;
		ListHolder(const ListHolder<_T>& other) : mList(other.mList), mCList(nullptr){
		}
		// The STL list is a public member, directly accessible.
		std::list <std::shared_ptr<_T>> mList;
		// Return a C list from the STL list.
		const bctbx_list_t *getCList() const{
			if (mCList) bctbx_list_free(mCList);
			mCList = _T::getCListFromCppList(mList, false);
			return mCList;
		}
		// Assign a C list. This replaces the STL list.
		void setCList(const bctbx_list_t *clist){
			mList =  _T::getCppListFromCList(clist);
		}
		~ListHolder(){
			if (mCList) bctbx_list_free(mCList);
		}
private:
	mutable bctbx_list_t *mCList = nullptr;
};


/*
 * Template class for classes that hold callbacks (such as LinphoneCallCbs, LinphoneAccountCbs etc.
 * The invocation of callbacks can be done with the LINPHONE_HYBRID_OBJECT_INVOKE_CBS() macro.
 */
template <typename _CppCbsType>
class LINPHONE_PUBLIC CallbacksHolder{
	public:
		void addCallbacks (const std::shared_ptr<_CppCbsType> &callbacks){
			if (find(mCallbacksList.mList.begin(), mCallbacksList.mList.end(), callbacks) == mCallbacksList.mList.end()){
				mCallbacksList.mList.push_back(callbacks);
				callbacks->setActive(true);
			}else{
				lError() << "Rejected Callbacks " << typeid(_CppCbsType).name() << " [" << (void*) callbacks.get() << "] added twice.";
			}
		}
		void removeCallbacks (const std::shared_ptr<_CppCbsType> &callbacks){
			auto it = find(mCallbacksList.mList.begin(), mCallbacksList.mList.end(), callbacks);
			if (it != mCallbacksList.mList.end()){
				mCallbacksList.mList.erase(it);
				callbacks->setActive(false);
			}else{
				lError() << "Attempt to remove " << typeid(_CppCbsType).name() << " [" << (void*) callbacks.get() << "] that does not exist.";
			}
		}
		void setCurrentCallbacks (const std::shared_ptr<_CppCbsType> &callbacks){
			mCurrentCallbacks = callbacks;
		}
		std::shared_ptr<_CppCbsType> getCurrentCallbacks () const{
			return mCurrentCallbacks;
		}
		const std::list<std::shared_ptr<_CppCbsType>> & getCallbacksList () const{
			return mCallbacksList.mList;
		}
		const bctbx_list_t * getCCallbacksList() const{
			return mCallbacksList.getCList();
		}
	private:
		ListHolder<_CppCbsType> mCallbacksList;
		std::shared_ptr<_CppCbsType> mCurrentCallbacks;
};




LINPHONE_END_NAMESPACE

#include "internal/c-tools.h"

// TODO: From coreapi. Remove me later.
#include "private_functions.h"

// =============================================================================
// Declare exported C types.
// =============================================================================

#define L_REGISTER_TYPES(F) \
	F(Address, Address) \
	F(ChatMessage, ChatMessage) \
	F(AbstractChatRoom, ChatRoom) \
	F(Core, Core) \
	F(Content, Content) \
	F(EventLog, EventLog) \
	F(MagicSearch, MagicSearch) \
	F(MediaSessionParams, CallParams) \
	F(ParticipantDeviceIdentity, ParticipantDeviceIdentity) \
	F(ParticipantImdnState, ParticipantImdnState) \
	F(SearchResult, SearchResult)

#define L_REGISTER_SUBTYPES(F) \
	F(AbstractChatRoom, BasicChatRoom) \
	F(AbstractChatRoom, BasicToClientGroupChatRoom) \
	F(AbstractChatRoom, ChatRoom) \
	F(AbstractChatRoom, ClientGroupChatRoom) \
	F(AbstractChatRoom, ClientGroupToBasicChatRoom) \
	F(AbstractChatRoom, ServerGroupChatRoom) \
	F(EventLog, ConferenceCallEvent) \
	F(EventLog, ConferenceChatMessageEvent) \
	F(EventLog, ConferenceEvent) \
	F(EventLog, ConferenceNotifiedEvent) \
	F(EventLog, ConferenceParticipantDeviceEvent) \
	F(EventLog, ConferenceParticipantEvent) \
	F(EventLog, ConferenceSecurityEvent) \
	F(EventLog, ConferenceSubjectEvent) \
	F(EventLog, ConferenceEphemeralMessageEvent)

// =============================================================================
// Register belle-sip ID.
// =============================================================================

#define L_REGISTER_ID(CPP_TYPE, C_TYPE) BELLE_SIP_TYPE_ID(Linphone ## C_TYPE),

/* Only pure belle_sip_object_t defined in C shall be declared here.
 * WARNING HybridObject<> derived don't need to be declared here */
BELLE_SIP_DECLARE_TYPES_BEGIN(linphone, 10000)
L_REGISTER_TYPES(L_REGISTER_ID)
BELLE_SIP_TYPE_ID(LinphoneAccountCreator),
BELLE_SIP_TYPE_ID(LinphoneAccountCreatorCbs),
BELLE_SIP_TYPE_ID(LinphoneAccountCreatorService),
BELLE_SIP_TYPE_ID(LinphoneBuffer),
BELLE_SIP_TYPE_ID(LinphoneCallStats),
BELLE_SIP_TYPE_ID(LinphoneChatMessageCbs),
BELLE_SIP_TYPE_ID(LinphoneChatRoomCbs),
BELLE_SIP_TYPE_ID(LinphoneConfig),
BELLE_SIP_TYPE_ID(LinphoneContactProvider),
BELLE_SIP_TYPE_ID(LinphoneContactSearch),
BELLE_SIP_TYPE_ID(LinphoneCoreCbs),
BELLE_SIP_TYPE_ID(LinphoneErrorInfo),
BELLE_SIP_TYPE_ID(LinphoneEvent),
BELLE_SIP_TYPE_ID(LinphoneEventCbs),
BELLE_SIP_TYPE_ID(LinphoneFactory),
BELLE_SIP_TYPE_ID(LinphoneFriend),
BELLE_SIP_TYPE_ID(LinphoneFriendList),
BELLE_SIP_TYPE_ID(LinphoneFriendListCbs),
BELLE_SIP_TYPE_ID(LinphoneImEncryptionEngine),
BELLE_SIP_TYPE_ID(LinphoneImEncryptionEngineCbs),
BELLE_SIP_TYPE_ID(LinphoneImNotifPolicy),
BELLE_SIP_TYPE_ID(LinphoneInfoMessage),
BELLE_SIP_TYPE_ID(LinphoneLDAPContactProvider),
BELLE_SIP_TYPE_ID(LinphoneLDAPContactSearch),
BELLE_SIP_TYPE_ID(LinphoneLoggingService),
BELLE_SIP_TYPE_ID(LinphoneLoggingServiceCbs),
BELLE_SIP_TYPE_ID(LinphoneMagicSearchCbs),
BELLE_SIP_TYPE_ID(LinphoneParticipant),
BELLE_SIP_TYPE_ID(LinphoneParticipantDevice),
BELLE_SIP_TYPE_ID(LinphoneParticipantDeviceCbs),
BELLE_SIP_TYPE_ID(LinphonePayloadType),
BELLE_SIP_TYPE_ID(LinphonePlayer),
BELLE_SIP_TYPE_ID(LinphonePlayerCbs),
BELLE_SIP_TYPE_ID(LinphonePresenceActivity),
BELLE_SIP_TYPE_ID(LinphonePresenceModel),
BELLE_SIP_TYPE_ID(LinphonePresenceNote),
BELLE_SIP_TYPE_ID(LinphonePresencePerson),
BELLE_SIP_TYPE_ID(LinphonePresenceService),
BELLE_SIP_TYPE_ID(LinphoneProxyConfig),
BELLE_SIP_TYPE_ID(LinphoneRange),
BELLE_SIP_TYPE_ID(LinphoneRecorder),
BELLE_SIP_TYPE_ID(LinphoneRecorderParams),
BELLE_SIP_TYPE_ID(LinphoneTransports),
BELLE_SIP_TYPE_ID(LinphoneTunnel),
BELLE_SIP_TYPE_ID(LinphoneTunnelConfig),
BELLE_SIP_TYPE_ID(LinphoneVcard),
BELLE_SIP_TYPE_ID(LinphoneVideoActivationPolicy),
BELLE_SIP_TYPE_ID(LinphoneVideoDefinition),
BELLE_SIP_TYPE_ID(LinphoneXmlRpcRequest),
BELLE_SIP_TYPE_ID(LinphoneXmlRpcRequestCbs),
BELLE_SIP_TYPE_ID(LinphoneXmlRpcSession)
BELLE_SIP_DECLARE_TYPES_END

#undef L_REGISTER_ID

// =============================================================================
// Register C types.
// =============================================================================

L_REGISTER_TYPES(L_REGISTER_TYPE);
L_REGISTER_SUBTYPES(L_REGISTER_SUBTYPE);

#undef L_REGISTER_SUBTYPES
#undef L_REGISTER_TYPES

#endif // ifndef _L_C_WRAPPER_H_
