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

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER

#if (__GNUC__ == 9 && __GNUC_MINOR__ >= 1)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

#include <ctime>

#include <bctoolbox/defs.h>

#include "linphone/utils/algorithm.h"
#include "linphone/utils/static-string.h"

#include "c-wrapper/internal/c-tools.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/basic-chat-room.h"
#include "chat/chat-room/chat-room.h"
#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-chat-room.h"
#include "chat/chat-room/server-chat-room.h"
#endif
#include "conference/conference.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "core/core-p.h"
#include "event-log/event-log-p.h"
#include "event-log/events.h"
#include "friend/friend-device.h"
#include "friend/friend-list.h"
#include "friend/friend.h"
#include "main-db-key-p.h"
#include "main-db-p.h"
#include "vcard/vcard-context.h"
#include "vcard/vcard.h"

#ifdef HAVE_DB_STORAGE
#include "internal/db-transaction.h"
#endif
#include "internal/statements.h"

#include "c-wrapper/c-wrapper.h"

// =============================================================================

// See: http://soci.sourceforge.net/doc/3.2/exchange.html
// Part: Object lifetime and immutability

// -----------------------------------------------------------------------------

using namespace std;

LINPHONE_BEGIN_NAMESPACE

#ifdef HAVE_DB_STORAGE
namespace {
constexpr unsigned int ModuleVersionEvents = makeVersion(1, 0, 31);
constexpr unsigned int ModuleVersionFriends = makeVersion(1, 0, 1);
constexpr unsigned int ModuleVersionLegacyFriendsImport = makeVersion(1, 0, 0);
constexpr unsigned int ModuleVersionLegacyHistoryImport = makeVersion(1, 0, 0);
constexpr unsigned int ModuleVersionLegacyCallLogsImport = makeVersion(1, 0, 0);

constexpr int LegacyFriendListColId = 0;
constexpr int LegacyFriendListColName = 1;
constexpr int LegacyFriendListColRlsUri = 2;
constexpr int LegacyFriendListColSyncUri = 3;
constexpr int LegacyFriendListColRevision = 4;

constexpr int LegacyFriendColFriendListId = 1;
constexpr int LegacyFriendColSipAddress = 2;
constexpr int LegacyFriendColSubscribePolicy = 3;
constexpr int LegacyFriendColSendSubscribe = 4;
constexpr int LegacyFriendColRefKey = 5;
constexpr int LegacyFriendColVCard = 6;
constexpr int LegacyFriendColVCardEtag = 7;
constexpr int LegacyFriendColVCardSyncUri = 8;
constexpr int LegacyFriendColPresenceReceived = 9;

constexpr int LegacyMessageColLocalAddress = 1;
constexpr int LegacyMessageColRemoteAddress = 2;
constexpr int LegacyMessageColDirection = 3;
constexpr int LegacyMessageColText = 4;
constexpr int LegacyMessageColState = 7;
constexpr int LegacyMessageColUrl = 8;
constexpr int LegacyMessageColDate = 9;
constexpr int LegacyMessageColAppData = 10;
constexpr int LegacyMessageColContentId = 11;
constexpr int LegacyMessageColContentType = 13;
constexpr int LegacyMessageColIsSecured = 14;

constexpr int LegacyCallLogColFrom = 1;
constexpr int LegacyCallLogColTo = 2;
constexpr int LegacyCallLogColDirection = 3;
constexpr int LegacyCallLogColDuration = 4;
constexpr int LegacyCallLogColStartTime = 5;
constexpr int LegacyCallLogColConnectedTime = 6;
constexpr int LegacyCallLogColStatus = 7;
constexpr int LegacyCallLogColVideoEnabled = 8;
constexpr int LegacyCallLogColQuality = 9;
constexpr int LegacyCallLogColCallId = 10;
constexpr int LegacyCallLogColRefKey = 11;
} // namespace
#endif

// -----------------------------------------------------------------------------
// soci helpers.
// -----------------------------------------------------------------------------
#ifdef HAVE_DB_STORAGE
static inline vector<char> blobToVector(soci::blob &in) {
	size_t len = in.get_len();
	if (!len) return vector<char>();
	vector<char> out(len);
	in.read(0, &out[0], len);
	return out;
}

static inline string blobToString(soci::blob &in) {
	vector<char> out = blobToVector(in);
	return string(out.begin(), out.end());
}

static constexpr string &blobToString(string &in) {
	return in;
}

template <typename T>
static T getValueFromRow(const soci::row &row, int index, bool &isNull) {
	isNull = false;

	if (row.get_indicator(size_t(index)) == soci::i_null) {
		isNull = true;
		return T();
	}
	return row.get<T>(size_t(index));
}
#endif

// -----------------------------------------------------------------------------
// Event filter tools.
// -----------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
// Some tools to build filters at compile time.
template <typename T>
struct EnumToSql {
	T first;
	const char *second;
};

template <typename T>
static constexpr const char *mapEnumToSql(const EnumToSql<T> enumToSql[], size_t n, T key) {
	return n == 0 ? ""
	              : (enumToSql[n - 1].first == key ? enumToSql[n - 1].second : mapEnumToSql(enumToSql, n - 1, key));
}

template <EventLog::Type... Type>
struct SqlEventFilterBuilder {};

template <EventLog::Type Type, EventLog::Type... List>
struct SqlEventFilterBuilder<Type, List...> {
	static constexpr auto get()
	    L_AUTO_RETURN(StaticIntString<int(Type)>() + "," + SqlEventFilterBuilder<List...>::get());
};

template <EventLog::Type Type>
struct SqlEventFilterBuilder<Type> {
	static constexpr auto get() L_AUTO_RETURN(StaticIntString<int(Type)>());
};
#endif

// -----------------------------------------------------------------------------
// Event filters.
// -----------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
namespace {
#ifdef _WIN32
// TODO: Find a workaround to deal with StaticString concatenation!!!
constexpr char ConferenceCallFilter[] = "3,21,4";
constexpr char ConferenceChatMessageFilter[] = "5";
constexpr char ConferenceInfoNoDeviceFilter[] = "1,2,6,7,8,9,12,13,14,15,16";
constexpr char ConferenceInfoFilter[] = "1,2,6,7,8,9,10,11,12,13,14,15,16";
constexpr char ConferenceChatMessageSecurityFilter[] = "5,13,14,15,16";
#else
constexpr auto ConferenceCallFilter = SqlEventFilterBuilder<EventLog::Type::ConferenceCallStarted,
                                                            EventLog::Type::ConferenceCallConnected,
                                                            EventLog::Type::ConferenceCallEnded>::get();

constexpr auto ConferenceChatMessageFilter = SqlEventFilterBuilder<EventLog::Type::ConferenceChatMessage>::get();

constexpr auto ConferenceInfoNoDeviceFilter =
    SqlEventFilterBuilder<EventLog::Type::ConferenceCreated,
                          EventLog::Type::ConferenceTerminated,
                          EventLog::Type::ConferenceParticipantAdded,
                          EventLog::Type::ConferenceParticipantRemoved,
                          EventLog::Type::ConferenceParticipantSetAdmin,
                          EventLog::Type::ConferenceParticipantUnsetAdmin,
                          EventLog::Type::ConferenceSubjectChanged,
                          EventLog::Type::ConferenceSecurityEvent,
                          EventLog::Type::ConferenceEphemeralMessageLifetimeChanged,
                          EventLog::Type::ConferenceEphemeralMessageManagedByAdmin,
                          EventLog::Type::ConferenceEphemeralMessageManagedByParticipants,
                          EventLog::Type::ConferenceEphemeralMessageEnabled,
                          EventLog::Type::ConferenceEphemeralMessageDisabled>::get();

constexpr auto ConferenceInfoFilter =
    ConferenceInfoNoDeviceFilter + "," +
    SqlEventFilterBuilder<EventLog::Type::ConferenceParticipantDeviceAdded,
                          EventLog::Type::ConferenceParticipantDeviceRemoved,
                          EventLog::Type::ConferenceParticipantDeviceStatusChanged,
                          EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged,
                          EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged>::get();

constexpr auto ConferenceChatMessageSecurityFilter =
    ConferenceChatMessageFilter + "," +
    SqlEventFilterBuilder<EventLog::Type::ConferenceSecurityEvent,
                          EventLog::Type::ConferenceEphemeralMessageLifetimeChanged,
                          EventLog::Type::ConferenceEphemeralMessageEnabled,
                          EventLog::Type::ConferenceEphemeralMessageDisabled,
                          EventLog::Type::ConferenceEphemeralMessageManagedByAdmin,
                          EventLog::Type::ConferenceEphemeralMessageManagedByParticipants>::get();
#endif // ifdef _WIN32

constexpr EnumToSql<MainDb::Filter> EventFilterToSql[] = {
    {MainDb::ConferenceCallFilter, ConferenceCallFilter},
    {MainDb::ConferenceChatMessageFilter, ConferenceChatMessageFilter},
    {MainDb::ConferenceInfoNoDeviceFilter, ConferenceInfoNoDeviceFilter},
    {MainDb::ConferenceInfoFilter, ConferenceInfoFilter},
    {MainDb::ConferenceChatMessageSecurityFilter, ConferenceChatMessageSecurityFilter}};
} // namespace

static const char *mapEventFilterToSql(MainDb::Filter filter) {
	return mapEnumToSql(EventFilterToSql, sizeof EventFilterToSql / sizeof EventFilterToSql[0], filter);
}

// -----------------------------------------------------------------------------

static string
buildSqlEventFilter(const list<MainDb::Filter> &filters, MainDb::FilterMask mask, const string &condKeyWord = "WHERE") {
	L_ASSERT(findIf(filters, [](const MainDb::Filter &filter) { return filter == MainDb::NoFilter; }) ==
	         filters.cend());

	if (mask == MainDb::NoFilter) return "";

	bool isStart = true;
	string sql;
	for (const auto &filter : filters) {
		if (!mask.isSet(filter)) continue;

		if (isStart) {
			isStart = false;
			sql += " " + condKeyWord + " type IN (";
		} else sql += ", ";
		sql += mapEventFilterToSql(filter);
	}

	if (!isStart) sql += ") ";

	return sql;
}
#endif

// -----------------------------------------------------------------------------
// Misc helpers.
// -----------------------------------------------------------------------------

shared_ptr<AbstractChatRoom> MainDbPrivate::findChatRoom(const ConferenceId &conferenceId) const {
	L_Q();
	shared_ptr<AbstractChatRoom> chatRoom = q->getCore()->findChatRoom(conferenceId, false);
	if (!chatRoom) lError() << "Unable to find chat room: " << conferenceId << ".";
	return chatRoom;
}

shared_ptr<Conference> MainDbPrivate::findConference(const ConferenceId &conferenceId) const {
	L_Q();
	shared_ptr<Conference> conference = q->getCore()->findConference(conferenceId, false);
	if (!conference) lError() << "Unable to find audio video conference: " << conferenceId << ".";
	return conference;
}
// -----------------------------------------------------------------------------
// Low level API.
// -----------------------------------------------------------------------------

long long MainDbPrivate::insertSipAddress(const Address &address) {
	if (!address.isValid()) {
		return -1;
	}
	// This is a hack, because all addresses don't print their parameters in the same order.
	string sipAddress = address.toStringUriOnlyOrdered();
	string displayName = address.getDisplayName();
	return insertSipAddress(sipAddress, displayName);
}

long long MainDbPrivate::insertSipAddress(const std::shared_ptr<Address> &address) {
	if (!address || !address->isValid()) {
		return -1;
	}
	// This is a hack, because all addresses don't print their parameters in the same order.
	string sipAddress = address->toStringUriOnlyOrdered();
	string displayName = address->getDisplayName();
	return insertSipAddress(sipAddress, displayName);
}

long long MainDbPrivate::insertSipAddress(BCTBX_UNUSED(const std::string &sipAddress),
                                          BCTBX_UNUSED(const std::string &displayName)) {
#ifdef HAVE_DB_STORAGE
	long long sipAddressId = selectSipAddressId(sipAddress, true);
	if (sipAddressId < 0) {
		lInfo() << "Insert new sip address in database: `" << sipAddress << "`.";
		soci::indicator displayNameInd = displayName.empty() ? soci::i_null : soci::i_ok;

		*dbSession.getBackendSession()
		    << "INSERT INTO sip_address (value, display_name) VALUES (:sipAddress, :displayName)",
		    soci::use(sipAddress), soci::use(displayName, displayNameInd);

		return dbSession.getLastInsertId();
	} else if (sipAddressId >= 0 && !displayName.empty()) {
		lInfo() << "Updating sip address display name in database: `" << sipAddress << "`.";

		*dbSession.getBackendSession() << "UPDATE sip_address SET display_name = :displayName WHERE id = :id",
		    soci::use(displayName), soci::use(sipAddressId);
	}

	return sipAddressId;
#else
	return -1;
#endif
}

void MainDbPrivate::insertContent(long long chatMessageId, const Content &content) {
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();

	const long long &contentTypeId = insertContentType(content.getContentType().getMediaType());
	const string &body = content.getBodyAsUtf8String();
	*session << "INSERT INTO chat_message_content (event_id, content_type_id, body, body_encoding_type) VALUES"
	            " (:chatMessageId, :contentTypeId, :body, 1)",
	    soci::use(chatMessageId), soci::use(contentTypeId), soci::use(body);

	const long long &chatMessageContentId = dbSession.getLastInsertId();
	if (content.isFile()) {
		const FileContent &fileContent = static_cast<const FileContent &>(content);
		const string &name = fileContent.getFileName();
		const size_t &size = fileContent.getFileSize();
		const string &path = fileContent.getFilePath();
		int duration = fileContent.getFileDuration();
		*session << "INSERT INTO chat_message_file_content (chat_message_content_id, name, size, path, duration) VALUES"
		            " (:chatMessageContentId, :name, :size, :path, :duration)",
		    soci::use(chatMessageContentId), soci::use(name), soci::use(size), soci::use(path), soci::use(duration);
	}

	for (const auto &property : content.getProperties()) {
		*session << "INSERT INTO chat_message_content_app_data (chat_message_content_id, name, data) VALUES"
		            " (:chatMessageContentId, :name, :data)",
		    soci::use(chatMessageContentId), soci::use(property.first), soci::use(property.second.getValue<string>());
	}
#endif
}

long long MainDbPrivate::insertContentType(const string &contentType) {
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();

	long long contentTypeId;
	*session << "SELECT id FROM content_type WHERE value = :contentType", soci::use(contentType),
	    soci::into(contentTypeId);
	if (session->got_data()) return contentTypeId;

	lInfo() << "Insert new content type in database: `" << contentType << "`.";
	*session << "INSERT INTO content_type (value) VALUES (:contentType)", soci::use(contentType);
	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateImportedBasicChatRoom(long long peerSipAddressId,
                                                             long long localSipAddressId,
                                                             const time_t &time) {
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();

	auto creationTime = dbSession.getTimeWithSociIndicator(time);
	long long chatRoomId = selectChatRoomId(peerSipAddressId, localSipAddressId);
	if (chatRoomId >= 0) {
		*session << "UPDATE chat_room SET last_update_time = :lastUpdateTime WHERE id = :chatRoomId",
		    soci::use(creationTime.first, creationTime.second), soci::use(chatRoomId);
		return chatRoomId;
	}

	const int capabilities =
	    ChatRoom::CapabilitiesMask({ChatRoom::Capabilities::OneToOne, ChatRoom::Capabilities::Basic});
	lInfo() << "Insert new chat room in database: (peer=" << peerSipAddressId << ", local=" << localSipAddressId
	        << ", capabilities=" << capabilities << ").";
	*session << "INSERT INTO chat_room ("
	            "  peer_sip_address_id, local_sip_address_id, creation_time, last_update_time, capabilities"
	            ") VALUES (:peerSipAddressId, :localSipAddressId, :creationTime, :lastUpdateTime, :capabilities)",
	    soci::use(peerSipAddressId), soci::use(localSipAddressId), soci::use(creationTime.first, creationTime.second),
	    soci::use(creationTime.first, creationTime.second), soci::use(capabilities);

	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

long long MainDbPrivate::insertChatRoom(const shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (q->isInitialized()) {
		const ConferenceId &conferenceId = chatRoom->getConferenceId();
		const auto &peerAddress = conferenceId.getPeerAddress();
		const long long &peerSipAddressId = insertSipAddress(peerAddress);
		const long long &localSipAddressId = insertSipAddress(conferenceId.getLocalAddress());

		long long chatRoomId = selectChatRoomId(peerSipAddressId, localSipAddressId);
		const int flags = chatRoom->hasBeenLeft() ? 1 : 0;
		const auto &chatRoomParams = chatRoom->getCurrentParams();

		if (chatRoomId >= 0) {
			// The chat room is already stored in DB, but still update the notify id and the flags that might have
			// changed
			lInfo() << "Update chat room in database: " << conferenceId << ".";
			*dbSession.getBackendSession() << "UPDATE chat_room SET"
			                                  " last_notify_id = :lastNotifyId, "
			                                  " flags = :flags "
			                                  " WHERE id = :chatRoomId",
			    soci::use(notifyId), soci::use(flags), soci::use(chatRoomId);
		} else {

			lInfo() << "Insert new chat room in database: " << conferenceId << ".";

			auto creationTime = dbSession.getTimeWithSociIndicator(chatRoom->getCreationTime());
			auto lastUpdateTime = dbSession.getTimeWithSociIndicator(chatRoom->getLastUpdateTime());

			// Remove capabilities like `Proxy`.
			const int &capabilities =
			    chatRoom->getCapabilities() & ~ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::Proxy);

			const string &subject = chatRoomParams->getUtf8Subject();
			int ephemeralEnabled = chatRoom->ephemeralEnabled() ? 1 : 0;
			long ephemeralLifeTime = chatRoom->getEphemeralLifetime();
			const long long peerSipAddressNoGruuId = selectSipAddressId(peerAddress->getUriWithoutGruu(), true);
			const long long &dbConferenceInfoId = selectConferenceInfoId(peerSipAddressNoGruuId);
			const long long conferenceInfoId = (dbConferenceInfoId <= 0) ? 0 : dbConferenceInfoId;
			*dbSession.getBackendSession() << "INSERT INTO chat_room ("
			                                  "  peer_sip_address_id, local_sip_address_id, creation_time,"
			                                  "  last_update_time, capabilities, subject, flags, last_notify_id, "
			                                  "ephemeral_enabled, ephemeral_messages_lifetime, conference_info_id"
			                                  ") VALUES ("
			                                  "  :peerSipAddressId, :localSipAddressId, :creationTime,"
			                                  "  :lastUpdateTime, :capabilities, :subject, :flags, :lastNotifyId, "
			                                  ":ephemeralEnabled, :ephemeralLifeTime, :conferenceInfoId"
			                                  ")",
			    soci::use(peerSipAddressId), soci::use(localSipAddressId),
			    soci::use(creationTime.first, creationTime.second),
			    soci::use(lastUpdateTime.first, lastUpdateTime.second), soci::use(capabilities), soci::use(subject),
			    soci::use(flags), soci::use(notifyId), soci::use(ephemeralEnabled), soci::use(ephemeralLifeTime),
			    soci::use(conferenceInfoId);

			chatRoomId = dbSession.getLastInsertId();
		}

		bool isBasic = (chatRoomParams->getChatParams()->getBackend() == ChatParams::Backend::Basic);
		// Do not add 'me' when creating a server-chat-room.
		if (*conferenceId.getLocalAddress() != *conferenceId.getPeerAddress()) {
			shared_ptr<Participant> me = chatRoom->getMe();
			long long meId = insertChatRoomParticipant(chatRoomId, insertSipAddress(me->getAddress()), me->isAdmin());
			for (const auto &device : me->getDevices()) {
				insertChatRoomParticipantDevice(meId, device);
			}
		}

		for (const auto &participant : (isBasic ? dynamic_pointer_cast<BasicChatRoom>(chatRoom)->getParticipants()
		                                        : chatRoom->getParticipants())) {
			long long participantId = insertChatRoomParticipant(chatRoomId, insertSipAddress(participant->getAddress()),
			                                                    participant->isAdmin());
			for (const auto &device : participant->getDevices()) {
				insertChatRoomParticipantDevice(participantId, device);
			}
		}

		return chatRoomId;
	}
#endif
	return -1;
}

long long
MainDbPrivate::insertChatRoomParticipant(long long chatRoomId, long long participantSipAddressId, bool isAdmin) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (q->isInitialized()) {
		soci::session *session = dbSession.getBackendSession();
		long long chatRoomParticipantId = selectChatRoomParticipantId(chatRoomId, participantSipAddressId);
		auto isAdminInt = static_cast<int>(isAdmin);
		if (chatRoomParticipantId >= 0) {
			// See: https://stackoverflow.com/a/15299655 (cast to reference)
			*session << "UPDATE chat_room_participant SET is_admin = :isAdmin WHERE id = :chatRoomParticipantId",
			    soci::use(isAdminInt), soci::use(chatRoomParticipantId);
			return chatRoomParticipantId;
		}

		*session << "INSERT INTO chat_room_participant (chat_room_id, participant_sip_address_id, is_admin)"
		            " VALUES (:chatRoomId, :participantSipAddressId, :isAdmin)",
		    soci::use(chatRoomId), soci::use(participantSipAddressId), soci::use(isAdminInt);

		return dbSession.getLastInsertId();
	}
#endif
	return -1;
}

void MainDbPrivate::insertChatRoomParticipantDevice(long long participantId,
                                                    long long participantDeviceSipAddressId,
                                                    const string &deviceName) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (q->isInitialized()) {
		soci::session *session = dbSession.getBackendSession();
		long long count;
		*session << "SELECT COUNT(*) FROM chat_room_participant_device"
		            " WHERE chat_room_participant_id = :participantId"
		            " AND participant_device_sip_address_id = :participantDeviceSipAddressId",
		    soci::into(count), soci::use(participantId), soci::use(participantDeviceSipAddressId);
		if (count) return;

		*session << "INSERT INTO chat_room_participant_device (chat_room_participant_id, "
		            "participant_device_sip_address_id, name)"
		            " VALUES (:participantId, :participantDeviceSipAddressId, :participantDeviceName)",
		    soci::use(participantId), soci::use(participantDeviceSipAddressId), soci::use(deviceName);
	}
#endif
}

void MainDbPrivate::insertChatRoomParticipantDevice(long long participantId,
                                                    const shared_ptr<ParticipantDevice> &device) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (q->isInitialized()) {
		soci::session *session = dbSession.getBackendSession();
		const long long &participantDeviceSipAddressId = insertSipAddress(device->getAddress());

		long long count;
		*session << "SELECT COUNT(*) FROM chat_room_participant_device"
		            " WHERE chat_room_participant_id = :participantId"
		            " AND participant_device_sip_address_id = :participantDeviceSipAddressId",
		    soci::into(count), soci::use(participantId), soci::use(participantDeviceSipAddressId);
		if (count) return;

		unsigned int state = static_cast<unsigned int>(device->getState());
		const std::string deviceName = device->getName();
		auto joiningTime = dbSession.getTimeWithSociIndicator(device->getTimeOfJoining());
		unsigned int joiningMethod = static_cast<unsigned int>(device->getJoiningMethod());
		*session << "INSERT INTO chat_room_participant_device (chat_room_participant_id, "
		            "participant_device_sip_address_id, name, state, joining_time, joining_method)"
		            " VALUES (:participantId, :participantDeviceSipAddressId, :participantDeviceName, "
		            ":participantDeviceState, :joiningTime, :joiningMethod)",
		    soci::use(participantId), soci::use(participantDeviceSipAddressId), soci::use(deviceName), soci::use(state),
		    soci::use(joiningTime.first, joiningTime.second), soci::use(joiningMethod);
	}
#endif
}

void MainDbPrivate::insertChatMessageParticipant(long long chatMessageId,
                                                 long long sipAddressId,
                                                 int state,
                                                 time_t stateChangeTime) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (q->isInitialized()) {
		auto stateChangeTm = dbSession.getTimeWithSociIndicator(stateChangeTime);
		*dbSession.getBackendSession()
		    << "INSERT INTO chat_message_participant (event_id, participant_sip_address_id, state, state_change_time)"
		       " VALUES (:chatMessageId, :sipAddressId, :state, :stateChangeTm)",
		    soci::use(chatMessageId), soci::use(sipAddressId), soci::use(state),
		    soci::use(stateChangeTm.first, stateChangeTm.second);
	}
#endif
}

long long MainDbPrivate::insertConferenceInfo(const std::shared_ptr<ConferenceInfo> &conferenceInfo,
                                              const std::shared_ptr<ConferenceInfo> &oldConferenceInfo) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (!q->isInitialized()) {
		lError() << "Database hasn't been initialized";
		return -1;
	}

	const auto &conferenceUri = conferenceInfo->getUri();
	const auto &organizer = conferenceInfo->getOrganizer();
	const auto &organizerAddress = organizer ? organizer->getAddress() : nullptr;
	const auto organizerAddressOk = (organizerAddress && organizerAddress->isValid());
	const auto conferenceUriOk = (conferenceUri && conferenceUri->isValid());
	if (!organizerAddressOk || !conferenceUriOk) {
		const auto organizerAddressString = organizerAddressOk ? organizerAddress->toString() : std::string("sip:");
		const auto conferenceUriString = conferenceUriOk ? conferenceUri->toString() : std::string("sip:");
		lError() << "Trying to insert a Conference Info without a valid organizer SIP address ( "
		         << organizerAddressString << ") or URI ( " << conferenceUriString << ")!";
		return -1;
	}

	const int audioEnabled = conferenceInfo->getCapability(LinphoneStreamTypeAudio) ? 1 : 0;
	const int videoEnabled = conferenceInfo->getCapability(LinphoneStreamTypeVideo) ? 1 : 0;
	const int chatEnabled = conferenceInfo->getCapability(LinphoneStreamTypeText) ? 1 : 0;
	const auto &ccmpUri = conferenceInfo->getCcmpUri();
	const long long &organizerSipAddressId = insertSipAddress(organizerAddress);
	const long long &uriSipAddressId = insertSipAddress(conferenceUri);
	const auto &dateTime = conferenceInfo->getDateTime();
	auto startTime = dbSession.getTimeWithSociIndicator(dateTime);
	const auto &earlierJoiningTime = conferenceInfo->getEarlierJoiningTime();
	auto joiningTime = dbSession.getTimeWithSociIndicator(earlierJoiningTime);
	const auto &expiryTime = conferenceInfo->getExpiryTime();
	auto expiryTimeSoci = dbSession.getTimeWithSociIndicator(expiryTime);
	const unsigned int duration = conferenceInfo->getDuration();
	const string &subject = conferenceInfo->getUtf8Subject();
	const string &description = conferenceInfo->getUtf8Description();
	const unsigned int state = static_cast<unsigned int>(conferenceInfo->getState());
	const unsigned int &sequence = conferenceInfo->getIcsSequence();
	const string &uid = conferenceInfo->getIcsUid();
	const unsigned int security_level = static_cast<unsigned int>(conferenceInfo->getSecurityLevel());

	long long conferenceInfoId = selectConferenceInfoId(uriSipAddressId);
	ConferenceInfo::participant_list_t dbParticipantList;
	if (conferenceInfoId >= 0) {
		// The conference info is already stored in DB, but still update it some information might have changed
		lInfo() << "Update " << *conferenceInfo << " in database: id " << conferenceInfoId << ".";
		if (oldConferenceInfo) {
			dbParticipantList = oldConferenceInfo->getParticipants();
		}

		*dbSession.getBackendSession()
		    << "UPDATE conference_info SET audio = :audioEnabled, video = :videoEnabled, chat = :chatEnabled, "
		       "  ccmp_uri = :ccmpUri,"
		       "  organizer_sip_address_id = :organizerSipAddressId,"
		       "  start_time = :startTime,"
		       "  earlier_joining_time = :earlierJoiningTime,"
		       "  expiry_time = :expiryTime,"
		       "  duration = :duration,"
		       "  subject = :subject,"
		       "  description = :description,"
		       "  state = :state,"
		       "  ics_sequence = :sequence,"
		       "  ics_uid = :uid,"
		       "  security_level = :security_level"
		       " WHERE id = :conferenceInfoId",
		    soci::use(audioEnabled), soci::use(videoEnabled), soci::use(chatEnabled), soci::use(ccmpUri),
		    soci::use(organizerSipAddressId), soci::use(startTime.first, startTime.second),
		    soci::use(joiningTime.first, joiningTime.second), soci::use(expiryTimeSoci.first, expiryTimeSoci.second),
		    soci::use(duration), soci::use(subject), soci::use(description), soci::use(state), soci::use(sequence),
		    soci::use(uid), soci::use(security_level), soci::use(conferenceInfoId);
	} else {
		*dbSession.getBackendSession()
		    << "INSERT INTO conference_info (audio, video, chat, ccmp_uri, organizer_sip_address_id, "
		       "uri_sip_address_id, start_time, earlier_joining_time, expiry_time, duration, subject, description, "
		       "state, ics_sequence, ics_uid, "
		       "security_level) VALUES (:audioEnabled, :videoEnabled, :chatEnabled, :ccmpUri, :organizerSipAddressId, "
		       ":uriSipAddressId, :startTime, :earlierJoiningTime, :expiryTime, :duration, :subject, :description, "
		       ":state, :sequence, :uid, "
		       ":security_level)",
		    soci::use(audioEnabled), soci::use(videoEnabled), soci::use(chatEnabled), soci::use(ccmpUri),
		    soci::use(organizerSipAddressId), soci::use(uriSipAddressId), soci::use(startTime.first, startTime.second),
		    soci::use(joiningTime.first, joiningTime.second), soci::use(expiryTimeSoci.first, expiryTimeSoci.second),
		    soci::use(duration), soci::use(subject), soci::use(description), soci::use(state), soci::use(sequence),
		    soci::use(uid), soci::use(security_level);

		conferenceInfoId = dbSession.getLastInsertId();
		lInfo() << "Insert new " << *conferenceInfo << " in database: id " << conferenceInfoId << ".";
	}

	const bool isOrganizerAParticipant = conferenceInfo->hasParticipant(organizer);
	insertOrUpdateConferenceInfoOrganizer(conferenceInfoId, organizer, isOrganizerAParticipant);

	const auto &participantList = conferenceInfo->getParticipants();
	for (const auto &participantInfo : participantList) {
		const auto isOrganizer = conferenceInfo->isOrganizer(participantInfo);
		insertOrUpdateConferenceInfoParticipant(conferenceInfoId, participantInfo, false, isOrganizer, true);
	}

	for (const auto &oldParticipantInfo : dbParticipantList) {
		const bool deleted =
		    (std::find_if(participantList.cbegin(), participantList.cend(), [&oldParticipantInfo](const auto &p) {
			     return (p->getAddress()->weakEqual(*oldParticipantInfo->getAddress()));
		     }) == participantList.cend());
		if (deleted) {
			const auto isOrganizer = conferenceInfo->isOrganizer(oldParticipantInfo);
			// If the participant to be deleted is the organizer, do not change the participant information parameters
			const auto &info = isOrganizer ? organizer : oldParticipantInfo;
			insertOrUpdateConferenceInfoParticipant(conferenceInfoId, info, true, isOrganizer, true);
		}
	}

	cache(conferenceInfo, conferenceInfoId);

	return conferenceInfoId;
#else
	return -1;
#endif
}

void MainDbPrivate::insertOrUpdateConferenceInfoParticipantParams(
    long long conferenceInfoParticipantId, const ParticipantInfo::participant_params_t params) const {
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();
	ParticipantInfo::participant_params_t paramsCopy = params;

	static const string participantParamsQuery = "SELECT id, name FROM conference_info_participant_params WHERE "
	                                             "conference_info_participant_id = :participantId ";
	soci::rowset<soci::row> participantParamsRows =
	    (session->prepare << participantParamsQuery, soci::use(conferenceInfoParticipantId));

	// Update existing keys
	for (const auto &participantParamsRow : participantParamsRows) {
		const long long &id = dbSession.resolveId(participantParamsRow, 0);
		const auto name = participantParamsRow.get<string>(1);
		if (auto el = paramsCopy.find(name); el != paramsCopy.end()) {
			*session << "UPDATE conference_info_participant_params SET  value = :value WHERE id = :id",
			    soci::use(el->second), soci::use(id);
			paramsCopy.erase(el);
		} else {
			*session << "DELETE FROM conference_info_participant_params WHERE id = :id", soci::use(id);
		}
	}

	// Add new name values pairs
	for (const auto &[name, value] : paramsCopy) {
		try {
			*session << "INSERT INTO conference_info_participant_params (conference_info_participant_id, name, value)  "
			            "VALUES ( :participantId, :name, :value )",
			    soci::use(conferenceInfoParticipantId), soci::use(name), soci::use(value);
		} catch (const soci::soci_error &e) {
			lDebug() << "Caught exception " << e.what() << ": participant info with ID " << conferenceInfoParticipantId
			         << " has already parameter " << name;
			*session << "REPLACE INTO conference_info_participant_params (conference_info_participant_id, name, value) "
			            " VALUES ( :participantId, :name, :value )",
			    soci::use(conferenceInfoParticipantId), soci::use(name), soci::use(value);
		}
	}
#endif
}

long long MainDbPrivate::insertOrUpdateConferenceInfoOrganizer(long long conferenceInfoId,
                                                               const std::shared_ptr<ParticipantInfo> &organizer,
                                                               bool isParticipant) {
	return insertOrUpdateConferenceInfoOrganizer(conferenceInfoId, insertSipAddress(organizer->getAddress()),
	                                             organizer->getAllParameters(), isParticipant, organizer->getCcmpUri());
}

long long MainDbPrivate::insertOrUpdateConferenceInfoOrganizer(long long conferenceInfoId,
                                                               long long organizerSipAddressId,
                                                               const ParticipantInfo::participant_params_t params,
                                                               bool isParticipant,
                                                               const std::string ccmpUri) {
	return insertOrUpdateConferenceInfoParticipant(conferenceInfoId, organizerSipAddressId, false, params, true,
	                                               isParticipant, ccmpUri);
}

long long
MainDbPrivate::insertOrUpdateConferenceInfoParticipant(long long conferenceInfoId,
                                                       const std::shared_ptr<ParticipantInfo> &participantInfo,
                                                       bool deleted,
                                                       bool isOrganizer,
                                                       bool isParticipant) {
	return insertOrUpdateConferenceInfoParticipant(conferenceInfoId, insertSipAddress(participantInfo->getAddress()),
	                                               deleted, participantInfo->getAllParameters(), isOrganizer,
	                                               isParticipant, participantInfo->getCcmpUri());
}

long long MainDbPrivate::insertOrUpdateConferenceInfoParticipant(long long conferenceInfoId,
                                                                 long long participantSipAddressId,
                                                                 bool deleted,
                                                                 const ParticipantInfo::participant_params_t params,
                                                                 bool isOrganizer,
                                                                 bool isParticipant,
                                                                 const std::string ccmpUri) {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoParticipantId =
	    selectConferenceInfoParticipantId(conferenceInfoId, participantSipAddressId);
	auto paramsStr = ParticipantInfo::memberParametersToString(params);
	int participantDeleted = deleted ? 1 : 0;
	int isOrganizerInt = isOrganizer ? 1 : 0;
	int isParticipantInt = isParticipant ? 1 : 0;
	if (conferenceInfoParticipantId >= 0) {
		*dbSession.getBackendSession() << "UPDATE conference_info_participant SET deleted = :deleted, is_organizer = "
		                                  ":isOrganizer, is_participant = :isParticipant WHERE id = :id",
		    soci::use(participantDeleted), soci::use(isOrganizerInt), soci::use(isParticipantInt),
		    soci::use(conferenceInfoParticipantId);
		if (!ccmpUri.empty()) {
			*dbSession.getBackendSession()
			    << "UPDATE conference_info_participant SET ccmp_uri = :ccmpUri WHERE id = :id",
			    soci::use(ccmpUri);
		}
	} else {
		*dbSession.getBackendSession()
		    << "INSERT INTO conference_info_participant (conference_info_id, participant_sip_address_id, ccmp_uri, "
		       "deleted, is_organizer, is_participant) VALUES (:conferenceInfoId, :participantSipAddressId, :ccmpUri, "
		       ":deleted, :isOrganizer, :isParticipant)",
		    soci::use(conferenceInfoId), soci::use(participantSipAddressId), soci::use(ccmpUri),
		    soci::use(participantDeleted), soci::use(isOrganizerInt), soci::use(isParticipantInt);

		conferenceInfoParticipantId = dbSession.getLastInsertId();
	}

	insertOrUpdateConferenceInfoParticipantParams(conferenceInfoParticipantId, params);

	return conferenceInfoParticipantId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateConferenceCall(const std::shared_ptr<CallLog> &callLog,
                                                      const std::shared_ptr<ConferenceInfo> &conferenceInfo) {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoId = -1;

	if (conferenceInfo != nullptr) {
		const auto &conferenceAddress = conferenceInfo->getUri();
		if (conferenceAddress) {
			const long long &uriSipAddressId = insertSipAddress(conferenceAddress);
			conferenceInfoId = selectConferenceInfoId(uriSipAddressId);
			if (conferenceInfoId < 0) {
				conferenceInfoId = insertConferenceInfo(conferenceInfo, nullptr);
			}
			callLog->setConferenceInfoId(conferenceInfoId);
		}
	}

	int duration = callLog->getDuration();
	auto connectedTime = dbSession.getTimeWithSociIndicator(callLog->getConnectedTime());
	int status = callLog->getStatus();
	int videoEnabled = callLog->isVideoEnabled() ? 1 : 0;
	double quality = static_cast<double>(callLog->getQuality());
	const string &callId = callLog->getCallId();
	const string &refKey = callLog->getRefKey();
	soci::indicator confInfoInd = conferenceInfoId > -1 ? soci::i_ok : soci::i_null;

	long long conferenceCallId = selectConferenceCallId(callLog->getCallId());
	if (conferenceCallId < 0) {
		lInfo() << "Insert new conference call in database: " << callLog->getCallId();

		std::shared_ptr<Address> from = callLog->getFromAddress() ? callLog->getFromAddress() : nullptr;
		const long long fromSipAddressId = insertSipAddress(from);
		std::shared_ptr<Address> to = callLog->getToAddress() ? callLog->getToAddress() : nullptr;
		const long long toSipAddressId = insertSipAddress(to);
		int direction = static_cast<int>(callLog->getDirection());
		auto startTime = dbSession.getTimeWithSociIndicator(callLog->getStartTime());

		*dbSession.getBackendSession() << "INSERT INTO conference_call ("
		                                  "  from_sip_address_id, to_sip_address_id, direction, duration, start_time, "
		                                  "connected_time, status, video_enabled,"
		                                  "  quality, call_id, refkey, conference_info_id"
		                                  ") VALUES ("
		                                  "  :fromSipAddressId, :toSipAddressId, :direction, :duration, :startTime, "
		                                  ":connectedTime, :status, :videoEnabled,"
		                                  "  :quality, :callId, :refKey, :conferenceInfoId"
		                                  ")",
		    soci::use(fromSipAddressId), soci::use(toSipAddressId), soci::use(direction), soci::use(duration),
		    soci::use(startTime.first, startTime.second), soci::use(connectedTime.first, connectedTime.second),
		    soci::use(status), soci::use(videoEnabled), soci::use(quality), soci::use(callId), soci::use(refKey),
		    soci::use(conferenceInfoId, confInfoInd);

		conferenceCallId = dbSession.getLastInsertId();
	} else {
		lInfo() << "Update conference call in database: " << callLog->getCallId();

		*dbSession.getBackendSession()
		    << "UPDATE conference_call SET"
		       "  duration = :duration, connected_time = :connectedTime, status = :status, video_enabled = "
		       ":videoEnabled,"
		       "  quality = :quality, call_id = :callId, refkey = :refKey, conference_info_id = :conferenceInfoId"
		       " WHERE id = :conferenceCallId",
		    soci::use(duration), soci::use(connectedTime.first, connectedTime.second), soci::use(status),
		    soci::use(videoEnabled), soci::use(quality), soci::use(callId), soci::use(refKey),
		    soci::use(conferenceInfoId, confInfoInd), soci::use(conferenceCallId);
	}

	cache(callLog, conferenceCallId);

	return conferenceCallId;
#else
	return -1;
#endif
}

long long MainDbPrivate::updateConferenceCall(const std::shared_ptr<CallLog> &callLog) {
#ifdef HAVE_DB_STORAGE
	int duration = callLog->getDuration();
	auto connectedTime = dbSession.getTimeWithSociIndicator(callLog->getConnectedTime());
	int status = callLog->getStatus();
	int videoEnabled = callLog->isVideoEnabled() ? 1 : 0;
	double quality = static_cast<double>(callLog->getQuality());
	const string &callId = callLog->getCallId();
	const string &refKey = callLog->getRefKey();

	long long conferenceCallId = selectConferenceCallId(callLog->getCallId());
	if (conferenceCallId >= 0) {
		lInfo() << "Update conference call in database: " << callLog->getCallId();

		std::shared_ptr<Address> from = callLog->getFromAddress() ? callLog->getFromAddress() : nullptr;
		const long long fromSipAddressId = insertSipAddress(from);
		std::shared_ptr<Address> to = callLog->getToAddress() ? callLog->getToAddress() : nullptr;
		const long long toSipAddressId = insertSipAddress(to);

		*dbSession.getBackendSession()
		    << "UPDATE conference_call SET"
		       "  from_sip_address_id = :fromSipAddressId, to_sip_address_id = :toSipAddressId,"
		       "  duration = :duration, connected_time = :connectedTime, status =  :status,"
		       "  video_enabled = :videoEnabled, quality = :quality, call_id = :callId, refkey = :refKey"
		       " WHERE id = :conferenceCallId",
		    soci::use(fromSipAddressId), soci::use(toSipAddressId), soci::use(duration),
		    soci::use(connectedTime.first, connectedTime.second), soci::use(status), soci::use(videoEnabled),
		    soci::use(quality), soci::use(callId), soci::use(refKey), soci::use(conferenceCallId);
	}

	cache(callLog, conferenceCallId);

	return conferenceCallId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateFriend(const std::shared_ptr<Friend> &f) {
#ifdef HAVE_DB_STORAGE
	std::shared_ptr<Vcard> vcard = nullptr;
	if (linphone_core_vcard_supported()) vcard = f->getVcard();

	long long friendId = f->mStorageId;
	long long friendListId = f->mFriendList->mStorageId;
	int subscribePolicy = f->getIncSubscribePolicy();
	int sendSubscribe = f->subscribesEnabled() ? 1 : 0;
	std::string refKey = f->getRefKey();
	std::string vcardStr = vcard ? vcard->asVcard4String() : "";
	std::string vcardEtag = vcard ? vcard->getEtag() : "";
	std::string vcardUrl = vcard ? vcard->getUrl() : "";
	int presenceReceived = f->isPresenceReceived() ? 1 : 0;
	int starred = f->getStarred() ? 1 : 0;

	std::shared_ptr<Address> addr = f->getAddress();
	long long sipAddressId = -1;
	soci::indicator sipAddressIndicator = soci::i_null;
	if (addr) {
		sipAddressId = insertSipAddress(addr);
		sipAddressIndicator = soci::i_ok;
	}

	if (friendId > 0) {
		lInfo() << "Update friend in database: " << f->getName();

		*dbSession.getBackendSession()
		    << "UPDATE friend SET "
		       "friends_list_id = :friendListId, sip_address_id = :sipAddressId, subscribe_policy = :subscribePolicy, "
		       "send_subscribe = :sendSubscribe, ref_key = :refKey, v_card = :vcardStr, v_card_etag = :vcardEtag, "
		       "v_card_sync_uri = :vcardUrl, presence_received = :presenceReceived, starred = :starred "
		       "WHERE id = :friendId",
		    soci::use(friendListId), soci::use(sipAddressId, sipAddressIndicator), soci::use(subscribePolicy),
		    soci::use(sendSubscribe), soci::use(refKey), soci::use(vcardStr), soci::use(vcardEtag), soci::use(vcardUrl),
		    soci::use(presenceReceived), soci::use(starred), soci::use(friendId);
	} else {
		lInfo() << "Insert new friend in database: " << f->getName();

		*dbSession.getBackendSession() << "INSERT INTO friend ("
		                                  "friends_list_id, sip_address_id, subscribe_policy, send_subscribe, ref_key, "
		                                  "v_card, v_card_etag, v_card_sync_uri, presence_received, starred"
		                                  ") VALUES ("
		                                  ":friendListId, :sipAddressId, :subscribePolicy, :sendSubscribe, :refKey, "
		                                  ":vcardStr, :vcardEtag, :vcardUrl, :presenceReceived, :starred"
		                                  ")",
		    soci::use(friendListId), soci::use(sipAddressId, sipAddressIndicator), soci::use(subscribePolicy),
		    soci::use(sendSubscribe), soci::use(refKey), soci::use(vcardStr), soci::use(vcardEtag), soci::use(vcardUrl),
		    soci::use(presenceReceived), soci::use(starred);

		friendId = dbSession.getLastInsertId();
	}

	return friendId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateFriendList(const std::shared_ptr<FriendList> &list) {
#ifdef HAVE_DB_STORAGE
	long long friendListId = list->mStorageId;
	std::string name = list->getDisplayName();
	std::string rlsUri = list->getRlsUri();
	std::string syncUri = list->getUri();
	std::string ctag = list->getRevision();
	int type = list->getType();

	if (friendListId > 0) {
		*dbSession.getBackendSession()
		    << "UPDATE friends_list SET "
		       "name = :name, rls_uri = :rlsUri, sync_uri = :syncUri, type = :type, ctag = :ctag "
		       "WHERE id = :friendListId",
		    soci::use(name), soci::use(rlsUri), soci::use(syncUri), soci::use(type), soci::use(ctag),
		    soci::use(friendListId);
	} else {
		lInfo() << "Insert new friend list in database: " << name;

		*dbSession.getBackendSession() << "INSERT INTO friends_list ("
		                                  "name, rls_uri, sync_uri, revision, type, ctag"
		                                  ") VALUES ("
		                                  ":name, :rlsUri, :syncUri, 0, :type, :ctag"
		                                  ")",
		    soci::use(name), soci::use(rlsUri), soci::use(syncUri), soci::use(type), soci::use(ctag);

		friendListId = dbSession.getLastInsertId();
	}

	return friendListId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateDevice(const std::shared_ptr<Address> &addressWithGruu,
                                              const std::string &displayName) {
#ifdef HAVE_DB_STORAGE

	// insertSipAddress will simply return the ID of the SIP address if it already exists.
	// No update of the associated display name is done if "" is provided.
	auto deviceAddressId = insertSipAddress(addressWithGruu->toStringUriOnlyOrdered(), "");

	auto withoutGruu = addressWithGruu->getUriWithoutGruu().toStringUriOnlyOrdered();
	auto sipAaddressId = insertSipAddress(withoutGruu, "");

	soci::session *session = dbSession.getBackendSession();
	long long deviceId = -1;
	*session << "SELECT device_id from friend_devices WHERE sip_address_id = :sipAaddressId AND device_address_id = "
	            ":deviceAddressId",
	    soci::use(sipAaddressId), soci::use(deviceAddressId), soci::into(deviceId);
	if (session->got_data() && deviceId > 0) {
		*session << "UPDATE friend_devices SET "
		            " display_name = :displayName "
		            " WHERE device_id = :deviceId ",
		    soci::use(displayName), soci::use(deviceId);
	} else {
		*session << "INSERT INTO friend_devices ( "
		            " sip_address_id, device_address_id, display_name "
		            " ) VALUES ( "
		            " :sipAaddressId, :deviceAddressId, :displayName "
		            " ) ",
		    soci::use(sipAaddressId), soci::use(deviceAddressId), soci::use(displayName);

		deviceId = dbSession.getLastInsertId();
	}

	return deviceId;
#else
	return -1;
#endif
}

// -----------------------------------------------------------------------------

long long MainDbPrivate::selectSipAddressId(const Address &address, const bool caseSensitive) const {
#ifdef HAVE_DB_STORAGE
	// This is a hack, because all addresses don't print their parameters in the same order.
	const string sipAddress = address.toStringUriOnlyOrdered();
	return selectSipAddressId(sipAddress, caseSensitive);
#else
	return -1;
#endif
}

long long MainDbPrivate::selectSipAddressId(const std::shared_ptr<Address> &address, const bool caseSensitive) const {
#ifdef HAVE_DB_STORAGE
	// This is a hack, because all addresses don't print their parameters in the same order.
	const string sipAddress = address->toStringUriOnlyOrdered();
	return selectSipAddressId(sipAddress, caseSensitive);
#else
	return -1;
#endif
}

long long MainDbPrivate::selectSipAddressId(const string &sipAddress, const bool caseSensitive) const {
#ifdef HAVE_DB_STORAGE
	long long sipAddressId;
	soci::session *session = dbSession.getBackendSession();
	if (caseSensitive) {
		*session << Statements::get(Statements::SelectSipAddressIdCaseSensitive), soci::use(sipAddress),
		    soci::into(sipAddressId);
	} else {
		*session << Statements::get(Statements::SelectSipAddressIdCaseInsensitive), soci::use(sipAddress),
		    soci::into(sipAddressId);
	}
	return session->got_data() ? sipAddressId : -1;
#else
	return -1;
#endif
}

std::string MainDbPrivate::selectSipAddressFromId(long long sipAddressId) const {
#ifdef HAVE_DB_STORAGE
	std::string sipAddress;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectSipAddressFromId), soci::use(sipAddressId), soci::into(sipAddress);

	return session->got_data() ? sipAddress : std::string();
#else
	return std::string();
#endif
}

void MainDbPrivate::deleteChatRoom(const long long &dbId) const {
#ifdef HAVE_DB_STORAGE
	invalidConferenceEventsFromQuery("SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId", dbId);

	*dbSession.getBackendSession() << "DELETE FROM chat_room WHERE id = :chatRoomId", soci::use(dbId);
#endif
}

void MainDbPrivate::deleteChatRoom(const ConferenceId &conferenceId) {
#ifdef HAVE_DB_STORAGE
	const long long &dbChatRoomId = selectChatRoomId(conferenceId);
	deleteChatRoom(dbChatRoomId);
	unreadChatMessageCountCache.insert(conferenceId, 0);
#endif
}

long long MainDbPrivate::selectChatRoomId(long long peerSipAddressId) const {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;

	std::string query = "SELECT id FROM chat_room WHERE peer_sip_address_id = :1";

	soci::session *session = dbSession.getBackendSession();
	*session << query, soci::use(peerSipAddressId), soci::into(chatRoomId);

	return session->got_data() ? chatRoomId : -1;
#else
	return -1;
#endif
}
long long MainDbPrivate::selectChatRoomId(long long peerSipAddressId, long long localSipAddressId) const {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectChatRoomId), soci::use(peerSipAddressId),
	    soci::use(localSipAddressId), soci::into(chatRoomId);

	return session->got_data() ? chatRoomId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectChatRoomId(const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	long long peerSipAddressId =
	    conferenceId.getPeerAddress() ? selectSipAddressId(conferenceId.getPeerAddress(), true) : -1;
	if (peerSipAddressId < 0) return -1;

	long long localSipAddressId =
	    conferenceId.getLocalAddress() ? selectSipAddressId(conferenceId.getLocalAddress(), true) : -1;
	if (localSipAddressId < 0) return -1;

	long long id = selectChatRoomId(peerSipAddressId, localSipAddressId);
	if (id != -1) {
		cache(conferenceId, id);
	}

	return id;
#else
	return -1;
#endif
}

ConferenceId MainDbPrivate::selectConferenceId(const long long chatRoomId) const {
#ifdef HAVE_DB_STORAGE
	L_Q();
	string peerSipAddress;
	string localSipAddress;

	string query = "SELECT peer_sip_address_id, local_sip_address_id FROM chat_room WHERE id = :1";
	soci::session *session = dbSession.getBackendSession();
	*session << query, soci::use(chatRoomId), soci::into(peerSipAddress), soci::into(localSipAddress);

	ConferenceId conferenceId(Address(peerSipAddress), Address(localSipAddress),
	                          q->getCore()->createConferenceIdParams());

	if (conferenceId.isValid()) {
		cache(conferenceId, chatRoomId);
	}

	return conferenceId;
#else
	return ConferenceId();
#endif
}

long long MainDbPrivate::selectChatRoomParticipantId(long long chatRoomId, long long participantSipAddressId) const {
#ifdef HAVE_DB_STORAGE
	long long chatRoomParticipantId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectChatRoomParticipantId), soci::use(chatRoomId),
	    soci::use(participantSipAddressId), soci::into(chatRoomParticipantId);

	return session->got_data() ? chatRoomParticipantId : -1;
#else
	return -1;
#endif
}

long long
MainDbPrivate::selectOneToOneChatRoomId(long long sipAddressIdA, long long sipAddressIdB, bool encrypted) const {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const int encryptedCapability = int(ChatRoom::Capabilities::Encrypted);
	const int expectedCapabilities = encrypted ? encryptedCapability : 0;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectOneToOneChatRoomId), soci::use(sipAddressIdA, "1"),
	    soci::use(sipAddressIdB, "2"), soci::use(encryptedCapability, "3"), soci::use(expectedCapabilities, "4"),
	    soci::into(chatRoomId);

	return session->got_data() ? chatRoomId : -1;
#else
	return -1;
#endif
}

#ifdef HAVE_DB_STORAGE
long long MainDbPrivate::findExpiredConferenceId(const std::shared_ptr<Address> &uri) {
	soci::session *session = dbSession.getBackendSession();
	const long long &uriSipAddressId = insertSipAddress(uri);
	long long id;
	*session << "SELECT id FROM expired_conferences WHERE (expired_conferences.uri_sip_address_id == "
	            ":uriSipAddressId)",
	    soci::use(uriSipAddressId), soci::into(id);
	return session->got_data() ? id : -1;
}
#endif

long long MainDbPrivate::selectConferenceInfoId(long long uriSipAddressId) {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectConferenceInfoId), soci::use(uriSipAddressId),
	    soci::into(conferenceInfoId);

	return session->got_data() ? conferenceInfoId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectConferenceInfoParticipantId(long long conferenceInfoId,
                                                           long long participantSipAddressId) const {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoParticipantId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectConferenceInfoParticipantId), soci::use(conferenceInfoId),
	    soci::use(participantSipAddressId), soci::into(conferenceInfoParticipantId);

	return session->got_data() ? conferenceInfoParticipantId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectConferenceCallId(const std::string &callId) {
#ifdef HAVE_DB_STORAGE
	long long conferenceCallId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectConferenceCall), soci::use(callId), soci::into(conferenceCallId);

	return session->got_data() ? conferenceCallId : -1;
#else
	return -1;
#endif
}

// -----------------------------------------------------------------------------

void MainDbPrivate::deleteContents(long long chatMessageId) {
#ifdef HAVE_DB_STORAGE
	*dbSession.getBackendSession() << "DELETE FROM chat_message_content WHERE event_id = :chatMessageId",
	    soci::use(chatMessageId);
#endif
}

void MainDbPrivate::deleteChatRoomParticipant(long long chatRoomId, long long participantSipAddressId) {
#ifdef HAVE_DB_STORAGE
	*dbSession.getBackendSession()
	    << "DELETE FROM chat_room_participant"
	       " WHERE chat_room_id = :chatRoomId AND participant_sip_address_id = :participantSipAddressId",
	    soci::use(chatRoomId), soci::use(participantSipAddressId);
#endif
}

void MainDbPrivate::deleteChatRoomParticipantDevice(long long participantId, long long participantDeviceSipAddressId) {
#ifdef HAVE_DB_STORAGE
	*dbSession.getBackendSession() << "DELETE FROM chat_room_participant_device"
	                                  " WHERE chat_room_participant_id = :participantId"
	                                  " AND participant_device_sip_address_id = :participantDeviceSipAddressId",
	    soci::use(participantId), soci::use(participantDeviceSipAddressId);
#endif
}

// -----------------------------------------------------------------------------
// Events API.
// -----------------------------------------------------------------------------
#ifdef HAVE_DB_STORAGE
shared_ptr<EventLog> MainDbPrivate::selectGenericConferenceEvent(const shared_ptr<AbstractChatRoom> &chatRoom,
                                                                 const soci::row &row) const {
	L_ASSERT(chatRoom);
	EventLog::Type type = EventLog::Type(row.get<int>(1));
	if (type == EventLog::Type::ConferenceChatMessage) {
		long long eventId = getConferenceEventIdFromRow(row);
		shared_ptr<EventLog> eventLog = getEventFromCache(eventId);
		if (!eventLog) {
			eventLog = selectConferenceChatMessageEvent(chatRoom, type, row);
			if (eventLog) cache(eventLog, eventId);
		}
		return eventLog;
	}

	return selectConferenceInfoEvent(chatRoom->getConferenceId(), row);
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceInfoEvent(const ConferenceId &conferenceId,
                                                              const soci::row &row) const {
	long long eventId = getConferenceEventIdFromRow(row);
	shared_ptr<EventLog> eventLog = getEventFromCache(eventId);
	if (eventLog) return eventLog;

	EventLog::Type type = EventLog::Type(row.get<int>(1));
	switch (type) {
		case EventLog::Type::None:
		case EventLog::Type::ConferenceChatMessage:
		case EventLog::Type::ConferenceChatMessageReaction:
		case EventLog::Type::ConferenceCallStarted:
		case EventLog::Type::ConferenceCallConnected:
		case EventLog::Type::ConferenceCallEnded:
		case EventLog::Type::ConferenceAllowedParticipantListChanged:
			return nullptr;

		case EventLog::Type::ConferenceCreated:
		case EventLog::Type::ConferenceTerminated:
			eventLog = selectConferenceEvent(conferenceId, type, row);
			break;

		case EventLog::Type::ConferenceParticipantAdded:
		case EventLog::Type::ConferenceParticipantRemoved:
		case EventLog::Type::ConferenceParticipantRoleUnknown:
		case EventLog::Type::ConferenceParticipantRoleSpeaker:
		case EventLog::Type::ConferenceParticipantRoleListener:
		case EventLog::Type::ConferenceParticipantSetAdmin:
		case EventLog::Type::ConferenceParticipantUnsetAdmin:
			eventLog = selectConferenceParticipantEvent(conferenceId, type, row);
			break;

		case EventLog::Type::ConferenceParticipantDeviceAdded:
		case EventLog::Type::ConferenceParticipantDeviceRemoved:
		case EventLog::Type::ConferenceParticipantDeviceJoiningRequest:
		case EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged:
		case EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged:
		case EventLog::Type::ConferenceParticipantDeviceStatusChanged:
			eventLog = selectConferenceParticipantDeviceEvent(conferenceId, type, row);
			break;

		case EventLog::Type::ConferenceAvailableMediaChanged:
			eventLog = selectConferenceAvailableMediaEvent(conferenceId, type, row);
			break;

		case EventLog::Type::ConferenceSubjectChanged:
			eventLog = selectConferenceSubjectEvent(conferenceId, type, row);
			break;

		case EventLog::Type::ConferenceSecurityEvent:
			eventLog = selectConferenceSecurityEvent(conferenceId, type, row);
			break;

		case EventLog::Type::ConferenceEphemeralMessageLifetimeChanged:
		case EventLog::Type::ConferenceEphemeralMessageManagedByAdmin:
		case EventLog::Type::ConferenceEphemeralMessageManagedByParticipants:
		case EventLog::Type::ConferenceEphemeralMessageEnabled:
		case EventLog::Type::ConferenceEphemeralMessageDisabled:
			eventLog = selectConferenceEphemeralMessageEvent(conferenceId, type, row);
	}

	if (eventLog) cache(eventLog, eventId);

	return eventLog;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceEvent(const ConferenceId &conferenceId,
                                                          EventLog::Type type,
                                                          const soci::row &row) const {
	return make_shared<ConferenceEvent>(type, getConferenceEventCreationTimeFromRow(row), conferenceId);
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceChatMessageEvent(const shared_ptr<AbstractChatRoom> &chatRoom,
                                                                     BCTBX_UNUSED(EventLog::Type type),
                                                                     const soci::row &row) const {
	L_Q();
	if (!q->isInitialized()) {
		lWarning() << "Database has not been initialized";
		return nullptr;
	}

	long long eventId = getConferenceEventIdFromRow(row);
	shared_ptr<ChatMessage> chatMessage = getChatMessageFromCache(eventId);
	if (!chatMessage) {
		chatMessage = shared_ptr<ChatMessage>(new ChatMessage(chatRoom, ChatMessage::Direction(row.get<int>(8))));

		chatMessage->setIsSecured(!!row.get<int>(9));

		ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
		ChatMessage::State messageState = ChatMessage::State(row.get<int>(7));
		// This is necessary if linphone has crashed while sending a message. It will set the correct state so the user
		// can resend it.
		if (messageState == ChatMessage::State::Idle || messageState == ChatMessage::State::InProgress ||
		    messageState == ChatMessage::State::FileTransferInProgress) {
			messageState = ChatMessage::State::NotDelivered;
		}
		dChatMessage->forceState(messageState);

		dChatMessage->forceFromAddress(Address::create(row.get<string>(3)));
		dChatMessage->forceToAddress(Address::create(row.get<string>(4)));

		dChatMessage->setTime(dbSession.getTime(row, 5));
		dChatMessage->setImdnMessageId(row.get<string>(6));
		dChatMessage->setPositiveDeliveryNotificationRequired(!!row.get<int>(14));
		dChatMessage->setDisplayNotificationRequired(!!row.get<int>(15));

		dChatMessage->markContentsAsNotLoaded();
		dChatMessage->setIsReadOnly(true);

		if (!!row.get<int>(18)) {
			dChatMessage->markAsRead();
		}
		dChatMessage->setForwardInfo(row.get<string>(19));

		if (row.get_indicator(20) != soci::i_null) {
			dChatMessage->enableEphemeralWithTime((long)row.get<double>(20));
			dChatMessage->setEphemeralExpireTime(dbSession.getTime(row, 21));
		}
		if (row.get_indicator(24) != soci::i_null) {
			dChatMessage->setReplyToMessageIdAndSenderAddress(row.get<string>(23),
			                                                  Address::create(row.get<string>(24)));
		}
		dChatMessage->setMessageId(row.get<string>(25));

		cache(chatMessage, eventId);
	}

	return make_shared<ConferenceChatMessageEvent>(getConferenceEventCreationTimeFromRow(row), chatMessage);
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceParticipantEvent(const ConferenceId &conferenceId,
                                                                     EventLog::Type type,
                                                                     const soci::row &row) const {

	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId);
	std::shared_ptr<Address> participantAddress = Address::create(row.get<string>(12));

	std::shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
	    type, getConferenceEventCreationTimeFromRow(row), conferenceId, participantAddress);
	event->setNotifyId(getConferenceEventNotifyIdFromRow(row));
	return event;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceParticipantDeviceEvent(const ConferenceId &conferenceId,
                                                                           EventLog::Type type,
                                                                           const soci::row &row) const {
	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId);
	std::shared_ptr<Address> participantAddress = Address::create(row.get<string>(12));
	std::shared_ptr<Address> deviceAddress = Address::create(row.get<string>(11));

	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
	    type, getConferenceEventCreationTimeFromRow(row), conferenceId, participantAddress, deviceAddress);
	event->setNotifyId(getConferenceEventNotifyIdFromRow(row));
	return event;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceSecurityEvent(const ConferenceId &conferenceId,
                                                                  BCTBX_UNUSED(EventLog::Type type),
                                                                  const soci::row &row) const {
	return make_shared<ConferenceSecurityEvent>(
	    getConferenceEventCreationTimeFromRow(row), conferenceId,
	    static_cast<ConferenceSecurityEvent::SecurityEventType>(row.get<int>(16)),
	    Address::create(row.get<string>(17)));
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceEphemeralMessageEvent(const ConferenceId &conferenceId,
                                                                          EventLog::Type type,
                                                                          const soci::row &row) const {
	return make_shared<ConferenceEphemeralMessageEvent>(type, getConferenceEventCreationTimeFromRow(row), conferenceId,
	                                                    (long)row.get<double>(22));
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceAvailableMediaEvent(const ConferenceId &conferenceId,
                                                                        BCTBX_UNUSED(EventLog::Type type),
                                                                        const soci::row &row) const {
	std::map<ConferenceMediaCapabilities, bool> mediaCapabilities;
	// TODO: choose rows
	mediaCapabilities[ConferenceMediaCapabilities::Audio] = false;
	mediaCapabilities[ConferenceMediaCapabilities::Video] = false;
	mediaCapabilities[ConferenceMediaCapabilities::Text] = false;
	shared_ptr<ConferenceAvailableMediaEvent> event = make_shared<ConferenceAvailableMediaEvent>(
	    getConferenceEventCreationTimeFromRow(row), conferenceId, mediaCapabilities);
	event->setNotifyId(getConferenceEventNotifyIdFromRow(row));
	return event;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceSubjectEvent(const ConferenceId &conferenceId,
                                                                 BCTBX_UNUSED(EventLog::Type type),
                                                                 const soci::row &row) const {
	shared_ptr<ConferenceSubjectEvent> event = make_shared<ConferenceSubjectEvent>(
	    getConferenceEventCreationTimeFromRow(row), conferenceId, row.get<string>(13));
	event->setNotifyId(getConferenceEventNotifyIdFromRow(row));
	return event;
}
#endif

// -----------------------------------------------------------------------------

long long MainDbPrivate::insertEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	const int &type = int(eventLog->getType());
	auto creationTime = dbSession.getTimeWithSociIndicator(eventLog->getCreationTime());
	*dbSession.getBackendSession() << "INSERT INTO event (type, creation_time) VALUES (:type, :creationTime)",
	    soci::use(type), soci::use(creationTime.first, creationTime.second);

	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceEvent(const shared_ptr<EventLog> &eventLog, long long *chatRoomId) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ConferenceEvent> conferenceEvent = static_pointer_cast<ConferenceEvent>(eventLog);

	long long eventId = -1;
	const ConferenceId &conferenceId = conferenceEvent->getConferenceId();
	const long long &curChatRoomId = selectChatRoomId(conferenceId);
	if (curChatRoomId < 0) {
		// A conference event can be inserted in database only if chat room exists.
		// Otherwise it's an error.
		lError() << "Unable to find chat room storage id of: " << conferenceId << ".";
	} else {
		eventId = insertEvent(eventLog);

		soci::session *session = dbSession.getBackendSession();
		*session << "INSERT INTO conference_event (event_id, chat_room_id)"
		            " VALUES (:eventId, :chatRoomId)",
		    soci::use(eventId), soci::use(curChatRoomId);

		if (eventLog->getType() == EventLog::Type::ConferenceTerminated)
			*session << "UPDATE chat_room SET flags = 1, last_notify_id = 0 WHERE id = :chatRoomId",
			    soci::use(curChatRoomId);
		else if (eventLog->getType() == EventLog::Type::ConferenceCreated)
			*session << "UPDATE chat_room SET flags = 0 WHERE id = :chatRoomId", soci::use(curChatRoomId);
	}

	if (chatRoomId) *chatRoomId = curChatRoomId;

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceCallEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ConferenceCallEvent> conferenceCallEvent = static_pointer_cast<ConferenceCallEvent>(eventLog);

	long long eventId = -1;
	auto callLog = conferenceCallEvent->getCallLog();
	auto conferenceInfo = conferenceCallEvent->getConferenceInfo();
	long long conferenceCallId = selectConferenceCallId(callLog->getCallId());

	EventLog::Type type = conferenceCallEvent->getType();
	switch (type) {
		case EventLog::Type::ConferenceCallStarted:
			if (conferenceCallId >= 0) {
				lWarning()
				    << "Cannot add ConferenceCallStarted event as conference call is already stored in db for call-id: "
				    << callLog->getCallId();
				return -1;
			}
			break;
		case EventLog::Type::ConferenceCallConnected:
			if (conferenceCallId < 0) {
				lWarning()
				    << "Adding ConferenceCallConnected event but conference call is not present in db for call-id: "
				    << callLog->getCallId();
			}
			break;
		case EventLog::Type::ConferenceCallEnded:
			if (conferenceCallId < 0) {
				lWarning() << "Adding ConferenceCallEnded event but conference call is not present in db for call-id: "
				           << callLog->getCallId();
			}
			break;
		default:
			lError() << "Trying to insert a conference call without the correct event type!";
			return -1;
	}

	conferenceCallId = insertOrUpdateConferenceCall(callLog, conferenceInfo);

	eventId = insertEvent(eventLog);

	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_call_event (event_id, conference_call_id)"
	            " VALUES (:eventId, :conferenceCallId)",
	    soci::use(eventId), soci::use(conferenceCallId);

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceChatMessageEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	const long long &eventId = insertConferenceEvent(eventLog);
	if (eventId < 0) return -1;

	shared_ptr<ChatMessage> chatMessage = static_pointer_cast<ConferenceChatMessageEvent>(eventLog)->getChatMessage();
	const long long &fromSipAddressId = insertSipAddress(chatMessage->getFromAddress());
	const long long &toSipAddressId = insertSipAddress(chatMessage->getToAddress());
	const string &forwardInfo = chatMessage->getForwardInfo();
	auto messageTime = dbSession.getTimeWithSociIndicator(chatMessage->getTime());
	const int &state = int(chatMessage->getState());
	const int &direction = int(chatMessage->getDirection());
	const string &imdnMessageId = chatMessage->getImdnMessageId();
	const int &isSecured = chatMessage->isSecured() ? 1 : 0;
	const int &deliveryNotificationRequired = chatMessage->getPrivate()->getPositiveDeliveryNotificationRequired();
	const int &displayNotificationRequired = chatMessage->getPrivate()->getDisplayNotificationRequired();
	const int &markedAsRead = chatMessage->getPrivate()->isMarkedAsRead() ? 1 : 0;
	const bool &isEphemeral = chatMessage->isEphemeral();
	const string &callId = chatMessage->getPrivate()->getCallId();
	const string &replyMessageId = chatMessage->getReplyToMessageId();
	const string &messageId = chatMessage->getPrivate()->getMessageId();
	long long sipAddressId = 0;
	if (!replyMessageId.empty()) {
		sipAddressId = insertSipAddress(chatMessage->getReplyToSenderAddress());
	}
	const long long &replyToSipAddressId = sipAddressId;

	*dbSession.getBackendSession()
	    << "INSERT INTO conference_chat_message_event ("
	       "  event_id, from_sip_address_id, to_sip_address_id,"
	       "  time, state, direction, imdn_message_id, is_secured,"
	       "  delivery_notification_required, display_notification_required,"
	       "  marked_as_read, forward_info, call_id, reply_message_id, reply_sender_address_id, message_id"
	       ") VALUES ("
	       "  :eventId, :localSipaddressId, :remoteSipaddressId,"
	       "  :time, :state, :direction, :imdnMessageId, :isSecured,"
	       "  :deliveryNotificationRequired, :displayNotificationRequired,"
	       "  :markedAsRead, :forwardInfo, :callId, :replyMessageId, :replyToSipAddressId, :messageId"
	       ")",
	    soci::use(eventId), soci::use(fromSipAddressId), soci::use(toSipAddressId),
	    soci::use(messageTime.first, messageTime.second), soci::use(state), soci::use(direction),
	    soci::use(imdnMessageId), soci::use(isSecured), soci::use(deliveryNotificationRequired),
	    soci::use(displayNotificationRequired), soci::use(markedAsRead), soci::use(forwardInfo), soci::use(callId),
	    soci::use(replyMessageId), soci::use(replyToSipAddressId), soci::use(messageId);

	if (isEphemeral) {
		long ephemeralLifetime = chatMessage->getEphemeralLifetime();
		auto expireTime = dbSession.getTimeWithSociIndicator(chatMessage->getEphemeralExpireTime());
		*dbSession.getBackendSession() << "INSERT INTO chat_message_ephemeral_event ("
		                                  "  event_id, ephemeral_lifetime,  expired_time"
		                                  ") VALUES ("
		                                  "  :eventId, :ephemeralLifetime, :expireTime"
		                                  ")",
		    soci::use(eventId), soci::use(ephemeralLifetime), soci::use(expireTime.first);
	}

	for (const auto &content : chatMessage->getContents())
		insertContent(eventId, *content);

	shared_ptr<AbstractChatRoom> chatRoom(chatMessage->getChatRoom());
	for (const auto &participant : chatRoom->getParticipants()) {
		const long long &participantSipAddressId = selectSipAddressId(participant->getAddress(), true);
		insertChatMessageParticipant(eventId, participantSipAddressId, state, chatMessage->getTime());
	}

	const long long &dbChatRoomId = selectChatRoomId(chatRoom->getConferenceId());
	*dbSession.getBackendSession() << "UPDATE chat_room SET last_message_id = :1 WHERE id = :2", soci::use(eventId),
	    soci::use(dbChatRoomId);

	if (direction == int(ChatMessage::Direction::Incoming) && !markedAsRead) {
		int *count = unreadChatMessageCountCache[chatRoom->getConferenceId()];
		if (count) ++*count;
	}
	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceChatMessageReactionEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	const long long &eventId = insertConferenceEvent(eventLog);
	if (eventId < 0) return -1;

	shared_ptr<ChatMessage> chatMessage = static_pointer_cast<ConferenceChatMessageEvent>(eventLog)->getChatMessage();

	// Need to remove the GRUU to prevent multiple reactions from the same SIP identity
	std::shared_ptr<Address> from = chatMessage->getFromAddress()->clone()->toSharedPtr();
	from->clean();
	const long long &fromSipAddressId = insertSipAddress(from);
	std::shared_ptr<Address> to = chatMessage->getToAddress()->clone()->toSharedPtr();
	to->clean();
	const long long &toSipAddressId = insertSipAddress(to);

	const tm &time = Utils::getTimeTAsTm(chatMessage->getTime());
	const string &imdnMessageId = chatMessage->getImdnMessageId();
	const string &callId = chatMessage->getPrivate()->getCallId();
	const string &reactionToMessageId = chatMessage->getReactionToMessageId();
	const string &reaction = chatMessage->getPrivate()->getUtf8Text();

	// Use REPLACE instead of INSERT so if (fromSipAddressId, reaction_to_message_id) constraint is triggered, reaction
	// will be updated
	*dbSession.getBackendSession() << "REPLACE INTO conference_chat_message_reaction_event ("
	                                  "  event_id, from_sip_address_id, to_sip_address_id,"
	                                  "  time, body, imdn_message_id, call_id, reaction_to_message_id"
	                                  ") VALUES ("
	                                  "  :eventId, :fromSipAddressId, :toSipAddressId,"
	                                  "  :time, :reaction, :imdnMessageId, :callId, :reactionToMessageId"
	                                  ")",
	    soci::use(eventId), soci::use(fromSipAddressId), soci::use(toSipAddressId), soci::use(time),
	    soci::use(reaction), soci::use(imdnMessageId), soci::use(callId), soci::use(reactionToMessageId);

	return eventId;
#else
	return -1;
#endif
}

void MainDbPrivate::updateConferenceChatMessageEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ChatMessage> chatMessage = static_pointer_cast<ConferenceChatMessageEvent>(eventLog)->getChatMessage();

	const EventLogPrivate *dEventLog = eventLog->getPrivate();
	MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
	const long long &eventId = dEventKey->storageId;
	soci::session *session = dbSession.getBackendSession();

	// 1. Get current chat message state and database state.
	const ChatMessage::State state = chatMessage->getState();
	ChatMessage::State dbState;
	bool dbMarkedAsRead;
	{
		int intState;
		int intMarkedAsRead;
		*session << "SELECT state, marked_as_read FROM conference_chat_message_event WHERE event_id = :eventId",
		    soci::into(intState), soci::into(intMarkedAsRead), soci::use(eventId);
		dbState = ChatMessage::State(intState);
		dbMarkedAsRead = intMarkedAsRead == 1;
	}
	const bool markedAsRead = chatMessage->getPrivate()->isMarkedAsRead();

	// 2. Update unread chat message count if necessary.
	const bool isOutgoing = chatMessage->getDirection() == ChatMessage::Direction::Outgoing;
	shared_ptr<AbstractChatRoom> chatRoom = chatMessage->getChatRoom();
	if (!isOutgoing && markedAsRead) {
		int *count = unreadChatMessageCountCache[chatRoom->getConferenceId()];
		if (count && !dbMarkedAsRead) {
			L_ASSERT(*count > 0);
			--*count;
		}
	}

	// 3. Update chat message event.
	{
		const string &imdnMessageId = chatMessage->getImdnMessageId();
		// Do not store transfer state.
		const int stateInt =
		    int(state == ChatMessage::State::InProgress || state == ChatMessage::State::FileTransferDone ||
		                state == ChatMessage::State::FileTransferInProgress ||
		                state == ChatMessage::State::FileTransferError
		            ? dbState
		            : state);
		const int markedAsReadInt = markedAsRead ? 1 : 0;
		*session << "UPDATE conference_chat_message_event SET state = :state, imdn_message_id = :imdnMessageId, "
		            "marked_as_read = :markedAsRead WHERE event_id = :eventId",
		    soci::use(stateInt), soci::use(imdnMessageId), soci::use(markedAsReadInt), soci::use(eventId);
	}

	// 4. Update contents.
	deleteContents(eventId);
	for (const auto &content : chatMessage->getContents())
		insertContent(eventId, *content);

	bool stateRequiresUpdatingParticipants = false;
	if (state == ChatMessage::State::NotDelivered) {
		const auto &meAddress = chatRoom->getMe()->getAddress();
		long long meAddressId = insertSipAddress(meAddress);
		static const string query =
		    "SELECT chat_message_participant.state FROM chat_message_participant WHERE event_id = :eventId AND "
		    "chat_message_participant.participant_sip_address_id = :meAddressId";
		soci::rowset<soci::row> rows =
		    (session->prepare << query, soci::use(chatMessage->getStorageId()), soci::use(meAddressId));
		ChatMessage::State meParticipantState = ChatMessage::State::Idle;
		for (const auto &row : rows) {
			meParticipantState = static_cast<ChatMessage::State>(row.get<int>(0));
		}
		// Set all participants to NotDelivered only if the sender state is NotDelivered
		stateRequiresUpdatingParticipants = (meParticipantState == ChatMessage::State::NotDelivered);
	} else {
		// Participants ar enot required to be updated if the chat message is moving from an IMDN controlled state
		// (NotDelivered, DeliveredToUser and Displayed) to Delivered state
		stateRequiresUpdatingParticipants =
		    (!ChatMessagePrivate::isImdnControlledState(dbState)) && (state == ChatMessage::State::Delivered);
	}

	// 5. Update participants.
	bool updateParticipants =
	    (isOutgoing && stateRequiresUpdatingParticipants &&
	     (chatRoom->getCapabilities() & AbstractChatRoom::Capabilities::Conference) && chatMessage->isValid());
	if (updateParticipants) {
		static const string query = "SELECT sip_address.value"
		                            " FROM sip_address, chat_message_participant"
		                            " WHERE event_id = :eventId"
		                            " AND sip_address.id = chat_message_participant.participant_sip_address_id";
		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(chatMessage->getStorageId()));

		// Use list of participants the client is sure have received the message and not the actual list of participants
		// being part of the chatroom
		for (const auto &row : rows) {
			const auto address = Address::create(row.get<string>(0));
			setChatMessageParticipantState(eventLog, address, state, std::time(nullptr));
		}
	}
#endif
}

long long MainDbPrivate::insertConferenceNotifiedEvent(const shared_ptr<EventLog> &eventLog, long long *chatRoomId) {
#ifdef HAVE_DB_STORAGE
	long long curChatRoomId;
	const long long &eventId = insertConferenceEvent(eventLog, &curChatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert participant event of type " << eventLog->getType() << " in database.";
		return -1;
	}

	const unsigned int &lastNotifyId = static_pointer_cast<ConferenceNotifiedEvent>(eventLog)->getNotifyId();
	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_notified_event (event_id, notify_id)"
	            " VALUES (:eventId, :notifyId)",
	    soci::use(eventId), soci::use(lastNotifyId);
	*session << "UPDATE chat_room SET last_notify_id = :lastNotifyId WHERE id = :chatRoomId", soci::use(lastNotifyId),
	    soci::use(curChatRoomId);

	if (chatRoomId) *chatRoomId = curChatRoomId;

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceParticipantEvent(const shared_ptr<EventLog> &eventLog,
                                                          long long *chatRoomId,
                                                          bool executeAction) {
#ifdef HAVE_DB_STORAGE
	long long curChatRoomId;
	const long long &eventId = insertConferenceNotifiedEvent(eventLog, &curChatRoomId);
	if (eventId < 0) return -1;

	shared_ptr<ConferenceParticipantEvent> participantEvent = static_pointer_cast<ConferenceParticipantEvent>(eventLog);

	const long long &participantAddressId = insertSipAddress(participantEvent->getParticipantAddress());

	*dbSession.getBackendSession() << "INSERT INTO conference_participant_event (event_id, participant_sip_address_id)"
	                                  " VALUES (:eventId, :participantAddressId)",
	    soci::use(eventId), soci::use(participantAddressId);

	if (executeAction) {
		bool isAdmin = eventLog->getType() == EventLog::Type::ConferenceParticipantSetAdmin;
		switch (eventLog->getType()) {
			case EventLog::Type::ConferenceParticipantAdded:
			case EventLog::Type::ConferenceParticipantSetAdmin:
			case EventLog::Type::ConferenceParticipantUnsetAdmin:
				insertChatRoomParticipant(curChatRoomId, participantAddressId, isAdmin);
				break;

			case EventLog::Type::ConferenceParticipantRemoved:
				deleteChatRoomParticipant(curChatRoomId, participantAddressId);
				break;

			default:
				break;
		}
	}

	if (chatRoomId) *chatRoomId = curChatRoomId;

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceParticipantDeviceEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceParticipantEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert participant device event of type " << eventLog->getType() << " in database.";
		return -1;
	}

	shared_ptr<ConferenceParticipantDeviceEvent> participantDeviceEvent =
	    static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);

	const auto &participantAddress = participantDeviceEvent->getParticipantAddress();
	const long long &participantAddressId = selectSipAddressId(participantAddress, true);
	if (participantAddressId < 0) {
		lError() << "Unable to find sip address id of: `" << participantAddress << "`.";
		return -1;
	}
	const long long &participantId = selectChatRoomParticipantId(chatRoomId, participantAddressId);
	if (participantId < 0) {
		lError() << "Unable to find valid participant id in database with chat room id = " << chatRoomId
		         << " and participant address id = " << participantAddressId;
		return -1;
	}
	const long long &deviceAddressId = insertSipAddress(participantDeviceEvent->getDeviceAddress());

	*dbSession.getBackendSession()
	    << "INSERT INTO conference_participant_device_event (event_id, device_sip_address_id)"
	       " VALUES (:eventId, :deviceAddressId)",
	    soci::use(eventId), soci::use(deviceAddressId);

	switch (eventLog->getType()) {
		case EventLog::Type::ConferenceParticipantDeviceAdded:
			insertChatRoomParticipantDevice(participantId, deviceAddressId, participantDeviceEvent->getDeviceName());
			break;

		case EventLog::Type::ConferenceParticipantDeviceRemoved:
			deleteChatRoomParticipantDevice(participantId, deviceAddressId);
			break;

		default:
			break;
	}

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceSecurityEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert security event of type " << eventLog->getType() << " in database.";
		return -1;
	}

	const int &securityEventType = int(static_pointer_cast<ConferenceSecurityEvent>(eventLog)->getSecurityEventType());
	const string &faultyDevice =
	    static_pointer_cast<ConferenceSecurityEvent>(eventLog)->getFaultyDeviceAddress()->toStringUriOnlyOrdered();

	// insert security event into new table "conference_security_event"
	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_security_event (event_id, security_alert, faulty_device)"
	            " VALUES (:eventId, :securityEventType, :faultyDevice)",
	    soci::use(eventId), soci::use(securityEventType), soci::use(faultyDevice);

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceAvailableMediaEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceNotifiedEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert conference available media event of type " << eventLog->getType()
		         << " in database.";
		return -1;
	}

	const int audio = static_pointer_cast<ConferenceAvailableMediaEvent>(eventLog)->audioEnabled() ? 1 : 0;
	const int video = static_pointer_cast<ConferenceAvailableMediaEvent>(eventLog)->videoEnabled() ? 1 : 0;
	const int chat = static_pointer_cast<ConferenceAvailableMediaEvent>(eventLog)->chatEnabled() ? 1 : 0;

	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_available_media_event (event_id, audio, video, chat)"
	            " VALUES (:eventId, :audio, :video, :chat)",
	    soci::use(eventId), soci::use(audio), soci::use(video), soci::use(chat);

	*session << "UPDATE chat_room SET audio = :audio, video = :video, chat = :chat"
	            " WHERE id = :chatRoomId",
	    soci::use(audio), soci::use(video), soci::use(chat), soci::use(chatRoomId);

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceSubjectEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceNotifiedEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert conference subject event of type " << eventLog->getType() << " in database.";
		return -1;
	}

	const string &subject = static_pointer_cast<ConferenceSubjectEvent>(eventLog)->getSubject();

	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_subject_event (event_id, subject)"
	            " VALUES (:eventId, :subject)",
	    soci::use(eventId), soci::use(subject);

	*session << "UPDATE chat_room SET subject = :subject"
	            " WHERE id = :chatRoomId",
	    soci::use(subject), soci::use(chatRoomId);

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceEphemeralMessageEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceNotifiedEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert conference ephemeral message event of type " << eventLog->getType()
		         << " in database.";
		return -1;
	}

	long lifetime = static_pointer_cast<ConferenceEphemeralMessageEvent>(eventLog)->getEphemeralMessageLifetime();

	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_ephemeral_message_event (event_id, lifetime)"
	            " VALUES (:eventId, :lifetime)",
	    soci::use(eventId), soci::use(lifetime);

	return eventId;
#else
	return -1;
#endif
}

void MainDbPrivate::setChatMessageParticipantState(const shared_ptr<EventLog> &eventLog,
                                                   const std::shared_ptr<Address> &participantAddress,
                                                   ChatMessage::State state,
                                                   time_t stateChangeTime) {
#ifdef HAVE_DB_STORAGE
	const EventLogPrivate *dEventLog = eventLog->getPrivate();
	MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
	const long long &eventId = dEventKey->storageId;
	auto participantAddressWithoutGruu = participantAddress->getUriWithoutGruu();
	long long participantSipAddressId = selectSipAddressId(participantAddressWithoutGruu, true);
	long long nbEntries;
	*dbSession.getBackendSession() << "SELECT count(*) FROM chat_message_participant WHERE event_id = :eventId AND "
	                                  "participant_sip_address_id = :participantSipAddressId",
	    soci::into(nbEntries), soci::use(eventId), soci::use(participantSipAddressId);

	int stateInt = int(state);

	if (nbEntries == 0) {
		if (participantSipAddressId <= 0) {
			// If the address is not found in the DB, add it
			participantSipAddressId = insertSipAddress(participantAddressWithoutGruu);
		}
		// We may be receiving an IMDN for a participant that received the message but we weren't aware of
		insertChatMessageParticipant(eventId, participantSipAddressId, stateInt, stateChangeTime);
	} else {
		/* setChatMessageParticipantState can be called by updateConferenceChatMessageEvent, which try to update
		 participant state by message state. However, we can not change state Displayed/DeliveredToUser to
		 Delivered/NotDelivered. */
		int intState;
		*dbSession.getBackendSession() << "SELECT state FROM chat_message_participant WHERE event_id = :eventId AND "
		                                  "participant_sip_address_id = :participantSipAddressId",
		    soci::into(intState), soci::use(eventId), soci::use(participantSipAddressId);
		ChatMessage::State dbState = ChatMessage::State(intState);

		if (int(state) < intState &&
		    (dbState == ChatMessage::State::Displayed || dbState == ChatMessage::State::DeliveredToUser)) {
			lInfo() << "setChatMessageParticipantState: can not change state from " << dbState << " to " << state;
			return;
		}

		auto stateChangeTm = dbSession.getTimeWithSociIndicator(stateChangeTime);
		*dbSession.getBackendSession()
		    << "UPDATE chat_message_participant SET state = :state,"
		       " state_change_time = :stateChangeTm"
		       " WHERE event_id = :eventId AND participant_sip_address_id = :participantSipAddressId",
		    soci::use(stateInt), soci::use(stateChangeTm.first, stateChangeTm.second), soci::use(eventId),
		    soci::use(participantSipAddressId);
	}
#endif
}

// ---------------------------------------------------------------------------
// Call log API.
// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
std::shared_ptr<CallLog> MainDbPrivate::selectCallLog(const soci::row &row) const {
	L_Q();

	const long long &dbCallLogId = dbSession.resolveId(row, 0);

	auto callLog = getCallLogFromCache(dbCallLogId);
	if (callLog) return callLog;

	const std::shared_ptr<Address> from = Address::create(row.get<string>(1));
	if (row.get_indicator(2) == soci::i_ok) from->setDisplayName(row.get<string>(2));

	const std::shared_ptr<Address> to = Address::create(row.get<string>(3));
	if (row.get_indicator(4) == soci::i_ok) to->setDisplayName(row.get<string>(4));

	callLog = CallLog::create(q->getCore(), static_cast<LinphoneCallDir>(row.get<int>(5)), from, to);

	callLog->setDuration(row.get<int>(6));
	callLog->setStartTime(dbSession.getTime(row, 7));
	callLog->setConnectedTime(dbSession.getTime(row, 8));
	callLog->setStatus(static_cast<LinphoneCallStatus>(row.get<int>(9)));
	callLog->setVideoEnabled(!!row.get<int>(10));
	callLog->setQuality((float)row.get<double>(11));

	soci::indicator ind = row.get_indicator(12);
	if (ind == soci::i_ok) callLog->setCallId(row.get<string>(12));

	ind = row.get_indicator(13);
	if (ind == soci::i_ok) callLog->setRefKey(row.get<string>(13));

	ind = row.get_indicator(14);
	if (ind == soci::i_ok) callLog->setConferenceInfoId(dbSession.resolveId(row, 14));

	cache(callLog, dbCallLogId);

	return callLog;
}
#endif

// ---------------------------------------------------------------------------
// Conference Info API.
// ---------------------------------------------------------------------------

ParticipantInfo::participant_params_t
MainDbPrivate::selectConferenceInfoParticipantParams(BCTBX_UNUSED(const long long participantId)) const {
	ParticipantInfo::participant_params_t participantParams;
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();
	static const string participantParamsQuery = "SELECT name, value FROM conference_info_participant_params WHERE "
	                                             "conference_info_participant_id = :participantId";
	soci::rowset<soci::row> participantParamsRows =
	    (session->prepare << participantParamsQuery, soci::use(participantId));
	for (const auto &participantParamsRow : participantParamsRows) {
		const auto name = participantParamsRow.get<string>(0);
		const auto value = participantParamsRow.get<string>(1);
		participantParams.insert(std::make_pair(name, value));
	}
#endif // HAVE_DB_STORAGE
	return participantParams;
}

ParticipantInfo::participant_params_t MainDbPrivate::migrateConferenceInfoParticipantParams(
    BCTBX_UNUSED(const ParticipantInfo::participant_params_t &unprocessedParticipantParams),
    BCTBX_UNUSED(const long long participantId)) const {
	ParticipantInfo::participant_params_t participantParams;
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();
	// Migrate participant parameters to the new table
	for (const auto &[name, value] : unprocessedParticipantParams) {
		std::string actualValue = value;
		// To keep the backward compatibility, switch role from Unknown to Speaker during the migration
		if (name.compare(ParticipantInfo::roleParameter) == 0) {
			const auto roleEnum = Participant::textToRole(value);
			if (roleEnum == Participant::Role::Unknown) {
				actualValue = Participant::roleToText(Participant::Role::Speaker);
			}
		}
		participantParams.insert(std::make_pair(name, actualValue));

		try {
			*session << "INSERT INTO conference_info_participant_params (conference_info_participant_id, name, value)  "
			            "VALUES ( :participantId, :name, :value )",
			    soci::use(participantId), soci::use(name), soci::use(actualValue);
		} catch (const soci::soci_error &e) {
			lDebug() << "Caught exception " << e.what() << ": participant info with ID " << participantId
			         << " has already parameter " << name;
			*session << "REPLACE INTO conference_info_participant_params (conference_info_participant_id, name, value) "
			            " VALUES ( :participantId, :name, :value )",
			    soci::use(participantId), soci::use(name), soci::use(actualValue);
		}
	}
#endif // HAVE_DB_STORAGE
	return participantParams;
}

#ifdef HAVE_DB_STORAGE
shared_ptr<ConferenceInfo> MainDbPrivate::selectConferenceInfo(const soci::row &row) {
	L_Q();
	const long long &dbConferenceInfoId = dbSession.resolveId(row, 0);

	auto conferenceInfo = getConferenceInfoFromCache(dbConferenceInfoId);
	if (conferenceInfo) {
		return conferenceInfo;
	}

	bool serverMode = linphone_core_conference_server_enabled(q->getCore()->getCCore());
	bool updateConferenceAddress = false;
	bool updateJoiningWindow = false;
	conferenceInfo = ConferenceInfo::create();
	const std::string uriString = row.get<string>(2);
	std::shared_ptr<Address> uri = Address::create(uriString);
	conferenceInfo->setUri(uri);
	const auto &conferenceUri = conferenceInfo->getUri();
	if (conferenceUri && conferenceUri->isValid()) {
		const auto &uri = conferenceUri->getUriWithoutGruu();
		const auto &uriStringOrdered = uri.toStringUriOnlyOrdered();
		updateConferenceAddress = (uriStringOrdered != uriString);
	}

	const auto startTime = dbSession.getTime(row, 3);
	conferenceInfo->setDateTime(startTime);
	const auto duration = dbSession.getUnsignedInt(row, 4, 0);
	conferenceInfo->setDuration(duration);
	conferenceInfo->setUtf8Subject(row.get<string>(5));
	conferenceInfo->setUtf8Description(row.get<string>(6));
	conferenceInfo->setState(
	    ConferenceInfo::State(row.get<int>(7))); // state is a TinyInt in database, don't cast it to unsigned,
	                                             // otherwise you'll get a std::bad_cast from soci.
	unsigned int icsSequence = dbSession.getUnsignedInt(row, 8, 0);
	conferenceInfo->setIcsSequence(icsSequence);

	conferenceInfo->setIcsUid(row.get<string>(9));
	conferenceInfo->setSecurityLevel(
	    static_cast<ConferenceParams::SecurityLevel>(dbSession.getUnsignedInt(row, 10, 0)));
	conferenceInfo->setCapability(LinphoneStreamTypeAudio, (row.get<int>(11) == 0) ? false : true);
	conferenceInfo->setCapability(LinphoneStreamTypeVideo, (row.get<int>(12) == 0) ? false : true);
	conferenceInfo->setCapability(LinphoneStreamTypeText, (row.get<int>(13) == 0) ? false : true);
	conferenceInfo->setCcmpUri(row.get<string>(14));
	const auto dbEarlierJoiningTime = dbSession.getTime(row, 15);
	auto earlierJoiningTime = dbEarlierJoiningTime;
	if (serverMode && (earlierJoiningTime == 0)) {
		updateJoiningWindow = true;
		earlierJoiningTime = startTime;
	}
	conferenceInfo->setEarlierJoiningTime(earlierJoiningTime);
	const auto dbExpiryTime = dbSession.getTime(row, 16);
	auto expiryTime = dbExpiryTime;
	if (serverMode && (expiryTime == 0) && (duration > 0) && (startTime > 0)) {
		updateJoiningWindow = true;
		expiryTime = startTime + 60 * duration;
	}
	conferenceInfo->setExpiryTime(expiryTime);

	if (updateJoiningWindow || updateConferenceAddress) {
		// Update conference address to ensure that a conference info can be successfully searched by its address as
		// well as the joining window. In fact earlier version of the SDK didn't have such a capability therefore we
		// default the window as the time between the start and the end time of the conference
		const long long &uriSipAddressId = insertSipAddress(uri);
		auto joiningTime = dbSession.getTimeWithSociIndicator(earlierJoiningTime);
		auto expiryTimeSoci = dbSession.getTimeWithSociIndicator(expiryTime);
		lInfo() << "Updating conference information with address " << *uri << ": earlier joining time "
		        << Utils::timeToIso8601(earlierJoiningTime) << " expiry time " << Utils::timeToIso8601(expiryTime);
		*dbSession.getBackendSession()
		    << "UPDATE conference_info SET uri_sip_address_id = :uriSipAddressId, earlier_joining_time = "
		       ":earlierJoiningTime, expiry_time = :expiryTime WHERE id = :conferenceInfoId",
		    soci::use(uriSipAddressId), soci::use(joiningTime.first, joiningTime.second),
		    soci::use(expiryTimeSoci.first, expiryTimeSoci.second), soci::use(dbConferenceInfoId);
	}

	// For backward compability purposes, get the organizer from conference_info table and set the sequence number
	// to that of the conference info stored in the db It may be overridden if the conference organizer has been
	// stored in table conference_info_organizer.
	std::shared_ptr<Address> organizerAddress = Address::create(row.get<string>(1));
	std::string organizerCcmpUri;
	ParticipantInfo::participant_params_t organizerParams;
	organizerParams.insert(std::make_pair(ParticipantInfo::sequenceParameter, std::to_string(icsSequence)));
	static const string organizerQuery =
	    "SELECT sip_address.id, sip_address.value, conference_info_organizer.params, conference_info_organizer.id"
	    " FROM sip_address, conference_info, conference_info_organizer"
	    " WHERE conference_info.id = :conferenceInfoId"
	    " AND sip_address.id = conference_info_organizer.organizer_sip_address_id"
	    " AND conference_info_organizer.conference_info_id = conference_info.id";

	soci::session *session = dbSession.getBackendSession();
	soci::rowset<soci::row> organizerRows = (session->prepare << organizerQuery, soci::use(dbConferenceInfoId));
	const auto &organizerRowIt = organizerRows.begin();
	if (organizerRowIt != organizerRows.end()) {
		const auto &organizerRow = (*organizerRowIt);
		const long long &organizerSipAddressId = dbSession.resolveId(organizerRow, 0);
		organizerAddress = Address::create(organizerRow.get<string>(1));
		const string organizerParamsStr = organizerRow.get<string>(2);
		const long long &organizerId = dbSession.resolveId(organizerRow, 3);

		// The flag is_participant is set to true here as by default the organizer is also a participant
		const long long organizerIdInParticipantTable = insertOrUpdateConferenceInfoOrganizer(
		    dbConferenceInfoId, organizerSipAddressId, organizerParams, true, std::string());
		const auto unprocessedOrganizerParams = ParticipantInfo::stringToMemberParameters(organizerParamsStr);
		organizerParams =
		    migrateConferenceInfoParticipantParams(unprocessedOrganizerParams, organizerIdInParticipantTable);

		// Delete entry from conference info organizer
		*session << "DELETE FROM conference_info_organizer WHERE id = :organizerId", soci::use(organizerId);
	} else {
		static const string organizerInParticipantTableQuery =
		    "SELECT sip_address.value, conference_info_participant.id, conference_info_participant.ccmp_uri FROM "
		    "sip_address, conference_info, conference_info_participant WHERE conference_info.id = :conferenceInfoId "
		    "AND sip_address.id = conference_info_participant.participant_sip_address_id AND "
		    "conference_info_participant.conference_info_id = conference_info.id AND "
		    "conference_info_participant.is_organizer = 1";

		soci::rowset<soci::row> organizerInParticipantTableRows =
		    (session->prepare << organizerInParticipantTableQuery, soci::use(dbConferenceInfoId));
		const auto &organizerInParticipantTableRowIt = organizerInParticipantTableRows.begin();
		if (organizerInParticipantTableRowIt != organizerInParticipantTableRows.end()) {
			const auto &organizerInParticipantTableRow = (*organizerInParticipantTableRowIt);
			organizerAddress = Address::create(organizerInParticipantTableRow.get<string>(0));
			organizerCcmpUri = organizerInParticipantTableRow.get<string>(2);
			const long long &organizerId = dbSession.resolveId(organizerInParticipantTableRow, 1);
			organizerParams = selectConferenceInfoParticipantParams(organizerId);
		}
	}

	auto organizerInfo = ParticipantInfo::create(organizerAddress);
	if (!organizerCcmpUri.empty()) {
		organizerInfo->setCcmpUri(organizerCcmpUri);
	}
	organizerInfo->setParameters(organizerParams);
	conferenceInfo->setOrganizer(organizerInfo);

	static const string participantQuery =
	    "SELECT sip_address.value, conference_info_participant.deleted, conference_info_participant.params, "
	    "conference_info_participant.id, conference_info_participant.ccmp_uri"
	    " FROM sip_address, conference_info, conference_info_participant"
	    " WHERE conference_info.id = :conferenceInfoId"
	    " AND sip_address.id = conference_info_participant.participant_sip_address_id"
	    " AND conference_info_participant.conference_info_id = conference_info.id AND "
	    "conference_info_participant.is_participant = 1";

	soci::rowset<soci::row> participantRows = (session->prepare << participantQuery, soci::use(dbConferenceInfoId));
	std::string emptyString;
	for (const auto &participantRow : participantRows) {
		int deleted = participantRow.get<int>(1);
		if (deleted == 0) {
			std::shared_ptr<Address> participantAddress = Address::create(participantRow.get<string>(0));
			const string participantParamsStr = participantRow.get<string>(2);
			const long long &participantId = dbSession.resolveId(participantRow, 3);
			const auto ccmpUri = participantRow.get<string>(4);
			ParticipantInfo::participant_params_t participantParams;
			auto participantInfo = ParticipantInfo::create(participantAddress);
			if (!ccmpUri.empty()) {
				participantInfo->setCcmpUri(ccmpUri);
			}
			if (participantParamsStr.empty()) {
				participantParams = selectConferenceInfoParticipantParams(participantId);
			} else {
				const auto unprocessedParticipantParams =
				    ParticipantInfo::stringToMemberParameters(participantParamsStr);
				participantParams = migrateConferenceInfoParticipantParams(unprocessedParticipantParams, participantId);
				// Set parameter string to an empty string
				*session << "UPDATE conference_info_participant SET params = :paramsStr  WHERE conference_info_id  = "
				            ":conferenceInfoId",
				    soci::use(emptyString), soci::use(dbConferenceInfoId);
			}
			participantInfo->setParameters(participantParams);
			conferenceInfo->addParticipant(participantInfo, false);
		}
	}

	cache(conferenceInfo, dbConferenceInfoId);

	return conferenceInfo;
}
#endif

// ---------------------------------------------------------------------------
// Friend API.
// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
std::list<std::shared_ptr<Friend>> MainDbPrivate::getFriends(const std::shared_ptr<FriendList> &list) {
	std::list<std::shared_ptr<Friend>> clList;
	long long dbFriendListId = list->mStorageId;

	soci::session *session = dbSession.getBackendSession();

	soci::rowset<soci::row> rows =
	    (session->prepare
	         << "SELECT id, sip_address_id, subscribe_policy, send_subscribe, ref_key, v_card, v_card_etag, "
	            "v_card_sync_uri, presence_received, starred "
	            "FROM friend "
	            "WHERE friends_list_id = :dbFriendListId "
	            "ORDER BY id",
	     soci::use(dbFriendListId));
	for (const auto &row : rows) {
		auto f = selectFriend(row);
		f->mFriendList = list.get();
		f->addAddressesAndNumbersIntoMaps(list);
		clList.push_back(f);
	}

	return clList;
}

std::shared_ptr<Friend> MainDbPrivate::selectFriend(const soci::row &row) const {
	L_Q();

	auto core = q->getCore();
	LinphoneCore *lc = core->getCCore();
	const long long &dbFriendId = dbSession.resolveId(row, 0);

	std::shared_ptr<Friend> f;
	const std::shared_ptr<Vcard> vcard = VcardContext::toCpp(lc->vcard_context)->getVcardFromBuffer(row.get<string>(5));
	if (vcard) {
		vcard->setEtag(row.get<string>(6));
		vcard->setUrl(row.get<string>(7));
		f = Friend::create(core, vcard);
	} else {
		f = Friend::create(core);
		switch (row.get_indicator(1)) {
			case soci::i_ok: {
				long long addrId = dbSession.resolveId(row, 1);
				std::shared_ptr<Address> addr = Address::create(selectSipAddressFromId(addrId));
				if (addr) f->setAddress(addr);
			} break;
			case soci::i_null:
			default:
				break;
		}
	}

	f->setIncSubscribePolicy(static_cast<LinphoneSubscribePolicy>(row.get<int>(2)));
	f->enableSubscribes(!!row.get<int>(3));
	f->setRefKey(row.get<string>(4));
	f->mPresenceReceived = !!row.get<int>(8);
	f->setStarred(!!row.get<int>(9));
	f->mStorageId = dbFriendId;

	return f;
}

std::shared_ptr<FriendList> MainDbPrivate::selectFriendList(const soci::row &row) const {
	const long long &dbFriendListId = dbSession.resolveId(row, 0);
	std::shared_ptr<FriendList> friendList = FriendList::create(nullptr);
	friendList->mStoreInDb = true; // Obviously
	friendList->mStorageId = dbFriendListId;
	friendList->setDisplayName(row.get<string>(1));
	friendList->setRlsUri(row.get<string>(2));
	friendList->setUri(row.get<string>(3));
	friendList->setType((LinphoneFriendListType)row.get<int>(5));

	int revision = row.get<int>(4);
	std::string ctag = row.get<string>(6);
	if (ctag.empty() && revision != 0) {
		friendList->mRevision = std::to_string(revision);
	} else {
		friendList->mRevision = ctag;
	}

	return friendList;
}
#endif

// -----------------------------------------------------------------------------
// Cache API.
// -----------------------------------------------------------------------------

shared_ptr<EventLog> MainDbPrivate::getEventFromCache(long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToEvent.find(storageId);
	if (it == storageIdToEvent.cend()) return nullptr;

	shared_ptr<EventLog> eventLog = it->second.lock();
	L_ASSERT(eventLog);
	return eventLog;
#else
	return nullptr;
#endif
}

shared_ptr<ChatMessage> MainDbPrivate::getChatMessageFromCache(long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToChatMessage.find(storageId);
	if (it == storageIdToChatMessage.cend()) return nullptr;

	shared_ptr<ChatMessage> chatMessage = it->second.lock();

	L_ASSERT(chatMessage);
	return chatMessage;
#else
	return nullptr;
#endif
}

ConferenceId MainDbPrivate::getConferenceIdFromCache(long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToConferenceId.find(storageId);
	if (it == storageIdToConferenceId.cend()) return ConferenceId();

	ConferenceId conferenceId = it->second;
	return conferenceId;
#else
	return ConferenceId();
#endif
}

shared_ptr<CallLog> MainDbPrivate::getCallLogFromCache(long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToCallLog.find(storageId);
	if (it == storageIdToCallLog.cend()) return nullptr;

	shared_ptr<CallLog> callLog = it->second.lock();
	return callLog;
#else
	return nullptr;
#endif
}

shared_ptr<ConferenceInfo> MainDbPrivate::getConferenceInfoFromCache(long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToConferenceInfo.find(storageId);
	if (it == storageIdToConferenceInfo.cend()) return nullptr;

	shared_ptr<ConferenceInfo> conferenceInfo = it->second.lock();
	return conferenceInfo ? conferenceInfo->clone()->toSharedPtr() : nullptr;
#else
	return nullptr;
#endif
}

void MainDbPrivate::cache(const shared_ptr<EventLog> &eventLog, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	L_Q();

	EventLogPrivate *dEventLog = eventLog->getPrivate();
	L_ASSERT(!dEventLog->dbKey.isValid());
	dEventLog->dbKey = MainDbEventKey(q->getCore(), storageId);
	storageIdToEvent[storageId] = eventLog;
	L_ASSERT(dEventLog->dbKey.isValid());
#endif
}

void MainDbPrivate::cache(const shared_ptr<ChatMessage> &chatMessage, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	L_ASSERT(!chatMessage->isValid());
	dChatMessage->setStorageId(storageId);
	storageIdToChatMessage[storageId] = chatMessage;
	L_ASSERT(chatMessage->isValid());
#endif
}

void MainDbPrivate::cache(const ConferenceId &conferenceId, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	L_ASSERT(conferenceId.isValid());
	storageIdToConferenceId[storageId] = conferenceId;
#endif
}

void MainDbPrivate::cache(const std::shared_ptr<CallLog> &callLog, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	storageIdToCallLog[storageId] = callLog;
#endif
}

void MainDbPrivate::cache(const std::shared_ptr<ConferenceInfo> &conferenceInfo, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	storageIdToConferenceInfo[storageId] = conferenceInfo;
#endif
}

void MainDbPrivate::invalidConferenceEventsFromQuery(const string &query, long long chatRoomId) const {
#ifdef HAVE_DB_STORAGE
	soci::rowset<soci::row> rows = (dbSession.getBackendSession()->prepare << query, soci::use(chatRoomId));
	for (const auto &row : rows) {
		long long eventId = dbSession.resolveId(row, 0);
		shared_ptr<EventLog> eventLog = getEventFromCache(eventId);
		if (eventLog) {
			const EventLogPrivate *dEventLog = eventLog->getPrivate();
			L_ASSERT(dEventLog->dbKey.isValid());
			// Reset storage ID as event is not valid anymore
			const_cast<EventLogPrivate *>(dEventLog)->resetStorageId();
		}
		shared_ptr<ChatMessage> chatMessage = getChatMessageFromCache(eventId);
		if (chatMessage) {
			L_ASSERT(chatMessage->isValid());
			ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
			dChatMessage->resetStorageId();
		}
	}
#endif
}

// -----------------------------------------------------------------------------
// Versions.
// -----------------------------------------------------------------------------

unsigned int MainDbPrivate::getModuleVersion(const string &name) {
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();

	unsigned int version;
	*session << "SELECT version FROM db_module_version WHERE name = :name", soci::into(version), soci::use(name);
	return session->got_data() ? version : 0;
#else
	return 0;
#endif
}

void MainDbPrivate::updateModuleVersion(const string &name, unsigned int version) {
#ifdef HAVE_DB_STORAGE
	unsigned int oldVersion = getModuleVersion(name);
	if (version <= oldVersion) return;

	soci::session *session = dbSession.getBackendSession();
	*session << "REPLACE INTO db_module_version (name, version) VALUES (:name, :version)", soci::use(name),
	    soci::use(version);
#endif
}

void MainDbPrivate::updateSchema() {
#ifdef HAVE_DB_STORAGE
	L_Q();

	// MySQL : Modified display_name in order to set explicitely this column to utf8mb4, while the default character
	// set of the table is set to ascii (this allows special characters in display name without breaking
	// compatibility with mysql 5.5) 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset
	// utf8mb4
	MainDb::Backend backend = q->getBackend();
	const string charset = backend == MainDb::Backend::Mysql ? "DEFAULT CHARSET=utf8mb4" : "";

	soci::session *session = dbSession.getBackendSession();
	unsigned int eventsDbVersionInt = getModuleVersion("events");
	Utils::Version eventDbVersion((eventsDbVersionInt >> 16) & 0xFF, (eventsDbVersionInt >> 8) & 0xFF,
	                              eventsDbVersionInt & 0xFF);
	lInfo() << "Event table version is " << eventDbVersion.toString();

	if (eventsDbVersionInt < makeVersion(1, 0, 1))
		*session << "ALTER TABLE chat_room_participant_device ADD COLUMN state TINYINT UNSIGNED DEFAULT 0";
	if (eventsDbVersionInt < makeVersion(1, 0, 2)) {
		*session << "DROP TRIGGER IF EXISTS chat_message_participant_deleter";
		*session << "ALTER TABLE chat_message_participant ADD COLUMN state_change_time" + dbSession.timestampType() +
		                " NOT NULL DEFAULT " + dbSession.currentTimestamp();
	}
	if (eventsDbVersionInt < makeVersion(1, 0, 3)) {
		// Remove client group one-to-one chat rooms for the moment as there are still some issues
		// with them and we prefer to keep using basic chat rooms instead
		const int &capabilities = ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::Conference) |
		                          ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::OneToOne);
		*session << "DELETE FROM chat_room WHERE (capabilities & :capabilities1) = :capabilities2",
		    soci::use(capabilities), soci::use(capabilities);
		linphone_config_set_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc", "prefer_basic_chat_room",
		                         TRUE);
	}
	if (eventsDbVersionInt < makeVersion(1, 0, 4)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN delivery_notification_required BOOLEAN NOT "
		            "NULL DEFAULT 0";
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN display_notification_required BOOLEAN NOT "
		            "NULL DEFAULT 0";
	}
	if (eventsDbVersionInt < makeVersion(1, 0, 5)) {
		const string queryDelivery = "UPDATE conference_chat_message_event"
		                             "  SET delivery_notification_required = 0"
		                             "  WHERE direction = " +
		                             Utils::toString(int(ChatMessage::Direction::Incoming)) +
		                             "  AND state = " + Utils::toString(int(ChatMessage::State::Delivered));

		*session << queryDelivery;

		const string queryDisplay = "UPDATE conference_chat_message_event"
		                            "  SET delivery_notification_required = 0, display_notification_required = 0"
		                            "  WHERE direction = " +
		                            Utils::toString(int(ChatMessage::Direction::Incoming)) +
		                            "  AND state = " + Utils::toString(int(ChatMessage::State::Displayed));

		*session << queryDisplay;
	}
	if (eventsDbVersionInt < makeVersion(1, 0, 6)) {
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		            "  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, "
		            "imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, "
		            "participant_sip_address_id, subject, delivery_notification_required, "
		            "display_notification_required, security_alert, faulty_device"
		            "  FROM event"
		            "  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		            "  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		            "  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		            "  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = "
		            "event.id"
		            "  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		            "  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		            "  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id";
	}
	if (eventsDbVersionInt < makeVersion(1, 0, 6) &&
	    linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "lime", "migrate_to_secured_room",
	                             FALSE)) {
		*session << "UPDATE chat_room "
		            "SET capabilities = capabilities | " +
		                Utils::toString(int(ChatRoom::Capabilities::Encrypted));
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 7)) {
		*session << "ALTER TABLE chat_room_participant_device ADD COLUMN name VARCHAR(255)";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 8)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN marked_as_read BOOLEAN NOT NULL DEFAULT 1";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		            "  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, "
		            "imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, "
		            "participant_sip_address_id, subject, delivery_notification_required, "
		            "display_notification_required, security_alert, faulty_device, marked_as_read"
		            "  FROM event"
		            "  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		            "  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		            "  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		            "  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = "
		            "event.id"
		            "  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		            "  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		            "  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 9)) {
		*session
		    << "ALTER TABLE conference_chat_message_event ADD COLUMN forward_info VARCHAR(255) NOT NULL DEFAULT ''";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		            "  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, "
		            "imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, "
		            "participant_sip_address_id, subject, delivery_notification_required, "
		            "display_notification_required, security_alert, faulty_device, marked_as_read, forward_info"
		            "  FROM event"
		            "  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		            "  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		            "  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		            "  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = "
		            "event.id"
		            "  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		            "  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		            "  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 10)) {
		*session << "CREATE INDEX incoming_not_delivered_index ON conference_chat_message_event "
		            "(delivery_notification_required, direction)";
		*session << "CREATE INDEX unread_index ON conference_chat_message_event (marked_as_read)";
		*session << "CREATE VIEW conference_event_simple_view AS SELECT id, type, chat_room_id FROM event LEFT JOIN "
		            "conference_event ON conference_event.event_id = event.id LEFT JOIN conference_chat_message_event "
		            "ON conference_chat_message_event.event_id = event.id";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 11)) {
		*session << "ALTER TABLE chat_room ADD COLUMN last_message_id " +
		                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL DEFAULT 0";
		*session << "UPDATE chat_room SET last_message_id = IFNULL((SELECT id FROM conference_event_simple_view WHERE "
		            "chat_room_id = chat_room.id AND type = 5 ORDER BY id DESC LIMIT 1), 0)";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 12)) {
		*session << "ALTER TABLE chat_room ADD COLUMN ephemeral_enabled BOOLEAN NOT NULL DEFAULT 0";
		*session << "ALTER TABLE chat_room ADD COLUMN ephemeral_messages_lifetime DOUBLE NOT NULL DEFAULT 86400";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		            "  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, "
		            "imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, "
		            "participant_sip_address_id, subject, delivery_notification_required, "
		            "display_notification_required, "
		            "security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, "
		            "lifetime"
		            "  FROM event"
		            "  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		            "  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		            "  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		            "  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = "
		            "event.id"
		            "  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		            "  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		            "  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id"
		            "  LEFT JOIN chat_message_ephemeral_event ON chat_message_ephemeral_event.event_id = event.id"
		            "  LEFT JOIN conference_ephemeral_message_event ON conference_ephemeral_message_event.event_id = "
		            "event.id";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 13)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN call_id VARCHAR(255) DEFAULT ''";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		            "  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, "
		            "imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, "
		            "participant_sip_address_id, subject, delivery_notification_required, "
		            "display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, "
		            "ephemeral_lifetime, expired_time, lifetime, call_id"
		            "  FROM event"
		            "  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		            "  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		            "  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		            "  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = "
		            "event.id"
		            "  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		            "  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		            "  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id"
		            "  LEFT JOIN chat_message_ephemeral_event ON chat_message_ephemeral_event.event_id = event.id"
		            "  LEFT JOIN conference_ephemeral_message_event ON conference_ephemeral_message_event.event_id = "
		            "event.id";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 14)) {
		*session
		    << "ALTER TABLE chat_message_content ADD COLUMN body_encoding_type TINYINT NOT NULL DEFAULT 0"; // Older
		                                                                                                    // table
		                                                                                                    // contains
		                                                                                                    // Local
		                                                                                                    // encoding.
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 15)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN reply_message_id VARCHAR(255) DEFAULT ''";
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN reply_sender_address_id " +
		                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") + " DEFAULT 0";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		            "  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, "
		            "imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, "
		            "participant_sip_address_id, subject, delivery_notification_required, "
		            "display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, "
		            "ephemeral_lifetime, expired_time, lifetime, call_id, reply_message_id, reply_sender_address_id"
		            "  FROM event"
		            "  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		            "  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		            "  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		            "  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = "
		            "event.id"
		            "  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		            "  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		            "  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id"
		            "  LEFT JOIN chat_message_ephemeral_event ON chat_message_ephemeral_event.event_id = event.id"
		            "  LEFT JOIN conference_ephemeral_message_event ON conference_ephemeral_message_event.event_id = "
		            "event.id";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 16)) {
		*session << "ALTER TABLE chat_message_file_content ADD COLUMN duration INT NOT NULL DEFAULT -1";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 17)) {
		*session << "ALTER TABLE sip_address ADD COLUMN display_name VARCHAR(255)";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 18)) {
		// We assume that the following statement is supported on all non-sqlite backends
		if (backend != MainDb::Backend::Sqlite3) {
			*session << "ALTER TABLE sip_address MODIFY COLUMN display_name VARCHAR(191) CHARACTER SET utf8mb4";
		}
		// In sqlite, there is no specific size limit to indexable columns, and text columns are stored in UTF-8 by
		// default. Given this, we assume that if "ALTER TABLE sip_address ADD COLUMN display_name VARCHAR(255)" was
		// run previously on Sqlite there is no need to alter the table to reduce the size (which would need to
		// recreate the whole column anyway as "MODIFY COLUMN" isn't supported in sqlite)
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 19)) {
		*session << "ALTER TABLE conference_info ADD COLUMN state TINYINT UNSIGNED NOT NULL DEFAULT 0";
		*session << "ALTER TABLE conference_info ADD COLUMN ics_sequence INT UNSIGNED DEFAULT 0";
		*session << "ALTER TABLE conference_info ADD COLUMN ics_uid VARCHAR(255) DEFAULT ''";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 20)) {
		*session << "ALTER TABLE conference_info_participant ADD COLUMN deleted BOOLEAN NOT NULL DEFAULT 0";
		*session << "ALTER TABLE conference_info_participant ADD COLUMN params VARCHAR(255) DEFAULT ''";
	}

	if (eventsDbVersionInt < makeVersion(1, 0, 21)) {
		*session << "ALTER TABLE chat_room_participant_device ADD COLUMN joining_method TINYINT UNSIGNED DEFAULT 0";
		*session << "ALTER TABLE chat_room_participant_device ADD COLUMN joining_time" + dbSession.timestampType() +
		                " NOT NULL DEFAULT " + dbSession.currentTimestamp();
	}

	try {
		*session << "ALTER TABLE conference_info ADD COLUMN security_level INT UNSIGNED DEFAULT 0";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'security_level' already exists in table 'conference_info'";
	}

	try {
		*session << "ALTER TABLE conference_info ADD COLUMN ccmp_uri VARCHAR(255) DEFAULT ''";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what() << ": Column 'ccmp_uri' already exists in table 'conference_info'";
	}

	*session << "CREATE TABLE IF NOT EXISTS chat_room_participant_device_clone ("
	            "  chat_room_participant_id" +
	                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
	                ","
	                "  participant_device_sip_address_id" +
	                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
	                ","

	                " state TINYINT UNSIGNED DEFAULT 0,"
	                " name VARCHAR(255),"
	                " joining_method TINYINT UNSIGNED DEFAULT 0,"
	                " joining_time" +
	                dbSession.timestampType() + " DEFAULT " + dbSession.currentTimestamp() + ") " + charset;

	soci::rowset<soci::row> originalParticipantDeviceRows =
	    (session->prepare << "SELECT chat_room_participant_id, participant_device_sip_address_id, state, name, "
	                         "joining_method, joining_time FROM chat_room_participant_device");
	for (const auto &row : originalParticipantDeviceRows) {
		const auto participantId = dbSession.resolveId(row, 0);
		const auto deviceId = dbSession.resolveId(row, 1);
		const auto state = row.get<int>(2);
		const auto name = row.get<string>(3, "");
		const auto joiningMethod = row.get<int>(4);
		const auto joiningTime = dbSession.getTime(row, 5);
		auto joiningTimeDb = dbSession.getTimeWithSociIndicator(joiningTime);
		*session << "INSERT INTO chat_room_participant_device_clone (chat_room_participant_id, "
		            "participant_device_sip_address_id, state, name, joining_method, joining_time)"
		            " VALUES (:participantId, :participantDeviceSipAddressId, :participantDeviceState, "
		            ":participantDeviceName, :participantDeviceJoiningMethod, :participantDeviceJoiningTime)",
		    soci::use(participantId), soci::use(deviceId), soci::use(state), soci::use(name), soci::use(joiningMethod),
		    soci::use(joiningTimeDb.first, joiningTimeDb.second);
	}

	*session << "DROP TABLE IF EXISTS chat_room_participant_device";

	*session << "CREATE TABLE IF NOT EXISTS chat_room_participant_device ("
	            "  chat_room_participant_id" +
	                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
	                ","
	                "  participant_device_sip_address_id" +
	                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
	                ","

	                " state TINYINT UNSIGNED DEFAULT 0,"
	                " name VARCHAR(255),"
	                " joining_method TINYINT UNSIGNED DEFAULT 0,"
	                " joining_time" +
	                dbSession.timestampType() + " DEFAULT " + dbSession.currentTimestamp() +
	                ","

	                "  PRIMARY KEY (chat_room_participant_id, participant_device_sip_address_id),"

	                "  FOREIGN KEY (chat_room_participant_id)"
	                "    REFERENCES chat_room_participant(id)"
	                "    ON DELETE CASCADE,"
	                "  FOREIGN KEY (participant_device_sip_address_id)"
	                "    REFERENCES sip_address(id)"
	                "    ON DELETE CASCADE"
	                ") " +
	                charset;

	soci::rowset<soci::row> participantDeviceRows =
	    (session->prepare << "SELECT chat_room_participant_id, participant_device_sip_address_id, state, name, "
	                         "joining_method, joining_time FROM chat_room_participant_device_clone");
	for (const auto &row : participantDeviceRows) {
		const auto participantId = dbSession.resolveId(row, 0);
		const auto deviceId = dbSession.resolveId(row, 1);
		const auto state = row.get<int>(2);
		const auto name = row.get<string>(3, "");
		const auto joiningMethod = row.get<int>(4);
		const auto joiningTime = dbSession.getTime(row, 5);
		auto joiningTimeDb = dbSession.getTimeWithSociIndicator(joiningTime);
		*session << "INSERT INTO chat_room_participant_device (chat_room_participant_id, "
		            "participant_device_sip_address_id, state, name, joining_method, joining_time)"
		            " VALUES (:participantId, :participantDeviceSipAddressId, :participantDeviceState, "
		            ":participantDeviceName, :participantDeviceJoiningMethod, :participantDeviceJoiningTime)",
		    soci::use(participantId), soci::use(deviceId), soci::use(state), soci::use(name), soci::use(joiningMethod),
		    soci::use(joiningTimeDb.first, joiningTimeDb.second);
	}

	try {
		*session << "ALTER TABLE conference_info_participant_params RENAME COLUMN \"key\" TO name";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'key' does not exists in table 'conference_info_participant_params' therefore it cannot "
		            "be renames as 'name'";
	}
	// Sanity check
	*session << "SELECT name FROM conference_info_participant_params";

	try {
		*session << "ALTER TABLE conference_info_participant ADD COLUMN is_participant BOOLEAN NOT NULL DEFAULT 1";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'is_participant' already exists in table 'conference_info_participant'";
	}

	try {
		*session << "ALTER TABLE conference_info_participant ADD COLUMN ccmp_uri VARCHAR(255) DEFAULT ''";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'ccmp_uri' already exists in table 'conference_info_participant'";
	}

	if (backend == MainDb::Backend::Sqlite3) {
		*session << "DELETE FROM conference_info_participant WHERE id IN (SELECT id FROM conference_info_participant "
		            "GROUP BY conference_info_id, participant_sip_address_id HAVING COUNT(*) > 1)";
	} else {
		*session << "DELETE p1 FROM conference_info_participant p1 INNER JOIN conference_info_participant p2 WHERE "
		            "p1.id < p2.id AND p1.conference_info_id = p2.conference_info_id AND p1.participant_sip_address_id "
		            "= p2.participant_sip_address_id";
	}

	try {
		*session << "ALTER TABLE chat_room ADD COLUMN muted BOOLEAN NOT NULL DEFAULT 0";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what() << ": Column 'muted' already exists in table 'chat_room'";
	}

	try {
		*session << "ALTER TABLE friends_list ADD COLUMN type INT NOT NULL DEFAULT -1";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what() << ": Column 'type' already exists in table 'friends_list'";
	}

	try {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN message_id VARCHAR(255) DEFAULT ''";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		            "  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, "
		            "imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, "
		            "participant_sip_address_id, subject, delivery_notification_required, "
		            "display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, "
		            "ephemeral_lifetime, expired_time, lifetime, call_id, reply_message_id, reply_sender_address_id, "
		            "message_id"
		            "  FROM event"
		            "  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		            "  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		            "  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		            "  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = "
		            "event.id"
		            "  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		            "  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		            "  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id"
		            "  LEFT JOIN chat_message_ephemeral_event ON chat_message_ephemeral_event.event_id = event.id"
		            "  LEFT JOIN conference_ephemeral_message_event ON conference_ephemeral_message_event.event_id = "
		            "event.id";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'message_id' already exists in table 'conference_chat_message_event'";
	}

	try {
		*session << "ALTER TABLE conference_info_participant ADD COLUMN is_organizer BOOLEAN NOT NULL DEFAULT 0";
		// We must recreate table conference_info_participant to change the UNIQUE constraint.
		*session << "CREATE TABLE IF NOT EXISTS conference_info_participant_clone ("
		            "  id" +
		                dbSession.primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  conference_info_id" +
		                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  participant_sip_address_id" +
		                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                " deleted BOOLEAN NOT NULL DEFAULT 0,"
		                " params VARCHAR(2048) DEFAULT '',"
		                " is_organizer BOOLEAN NOT NULL DEFAULT 0,"

		                "  UNIQUE (conference_info_id, participant_sip_address_id),"

		                "  FOREIGN KEY (conference_info_id)"
		                "    REFERENCES conference_info(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;
		*session << "INSERT INTO conference_info_participant_clone SELECT * FROM conference_info_participant";
		*session << "DROP TABLE IF EXISTS conference_info_participant";

		*session << "CREATE TABLE IF NOT EXISTS conference_info_participant ("
		            "  id" +
		                dbSession.primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  conference_info_id" +
		                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  participant_sip_address_id" +
		                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                " deleted BOOLEAN NOT NULL DEFAULT 0,"
		                " params VARCHAR(255) DEFAULT '',"
		                " is_organizer BOOLEAN NOT NULL DEFAULT 0,"

		                "  UNIQUE (conference_info_id, participant_sip_address_id, is_organizer),"

		                "  FOREIGN KEY (conference_info_id)"
		                "    REFERENCES conference_info(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;
		*session << "INSERT INTO conference_info_participant SELECT * FROM conference_info_participant_clone";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'is_organizer' already exists in table 'conference_info_participant'";
	}

	try {
		*session << "ALTER TABLE friends_list ADD COLUMN ctag VARCHAR(255) NOT NULL DEFAULT ''";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what() << ": Column 'ctag' already exists in table 'friends_list'";
	}

	try {
		*session << "ALTER TABLE conference_info ADD COLUMN audio BOOLEAN NOT NULL DEFAULT 1";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what() << ": Column 'audio' already exists in table 'conference_info'";
	}

	try {
		*session << "ALTER TABLE conference_info ADD COLUMN video BOOLEAN NOT NULL DEFAULT 1";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what() << ": Column 'video' already exists in table 'conference_info'";
	}

	try {
		*session << "ALTER TABLE conference_info ADD COLUMN chat BOOLEAN NOT NULL DEFAULT 0";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what() << ": Column 'chat' already exists in table 'conference_info'";
	}

	try {
		*session << "ALTER TABLE chat_room ADD COLUMN conference_info_id " +
		                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL DEFAULT 0";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'conference_info_id' already exists in table 'chat_room'";
	}

	try {
		*session << "ALTER TABLE conference_info ADD COLUMN earlier_joining_time " + dbSession.timestampType() +
		                " NULL DEFAULT NULL";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'earlier_joining_time' already exists in table 'conference_info'";
	}

	try {
		*session << "ALTER TABLE conference_info ADD COLUMN expiry_time " + dbSession.timestampType() +
		                " NULL DEFAULT NULL";
	} catch (const soci::soci_error &e) {
		lDebug() << "Caught exception " << e.what()
		         << ": Column 'expiry_time' already exists in table 'conference_info'";
	}

	// /!\ Warning : if varchar columns < 255 were to be indexed, their size must be set back to 191 = max indexable
	// (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in column creation)
	//
	// Using DB table version cause issues when updating or downgrading the SDK version. It has been decided to drop
	// this mechanism starting from DB version 21 and execute all MySql query at startup. Developpers must be
	// careful either to catch exceptions or to make sure that the modification to the database is only applied once
	// and the following restarts of the core will not do anything.

	unsigned int friendsDbVersionInt = getModuleVersion("friends");
	Utils::Version friendsDbVersion((friendsDbVersionInt >> 16) & 0xFF, (friendsDbVersionInt >> 8) & 0xFF,
	                                friendsDbVersionInt & 0xFF);
	lInfo() << "Friends table version is " << friendsDbVersion.toString();

	if (friendsDbVersionInt < makeVersion(1, 0, 1)) {
		// The sip_address_id field needs to be nullable.
		// Do not try to copy data from the old table because it was not used before this version (use of an other
		// database external to the mainDb)
		*session << "DROP TABLE IF EXISTS friend";

		*session << "CREATE TABLE IF NOT EXISTS friend ("
		            "  id" +
		                dbSession.primaryKeyStr("INT UNSIGNED") +
		                ","

		                "  sip_address_id" +
		                dbSession.primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NULL,"
		                "  friends_list_id" +
		                dbSession.primaryKeyRefStr("INT UNSIGNED") +
		                " NOT NULL,"

		                "  subscribe_policy TINYINT UNSIGNED NOT NULL,"
		                "  send_subscribe BOOLEAN NOT NULL,"
		                "  presence_received BOOLEAN NOT NULL,"
		                "  starred BOOLEAN NOT NULL,"
		                "  ref_key VARCHAR(255),"

		                "  v_card MEDIUMTEXT,"
		                "  v_card_etag VARCHAR(255),"
		                "  v_card_sync_uri VARCHAR(2047),"

		                "  FOREIGN KEY (sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (friends_list_id)"
		                "    REFERENCES friends_list(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;
	}

#endif
}

// -----------------------------------------------------------------------------
// Import.
// -----------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
static inline bool checkLegacyTableExists(soci::session &session, const string &name) {
	session << "SELECT name FROM sqlite_master WHERE type='table' AND name = :name", soci::use(name);
	return session.got_data();
}

static inline bool checkLegacyFriendsTableExists(soci::session &session) {
	return checkLegacyTableExists(session, "friends");
}

static inline bool checkLegacyHistoryTableExists(soci::session &session) {
	return checkLegacyTableExists(session, "history");
}

static inline bool checkLegacyCallLogsTableExists(soci::session &session) {
	return checkLegacyTableExists(session, "call_history");
}
#endif

#ifdef HAVE_DB_STORAGE
bool MainDbPrivate::importLegacyFriends(DbSession &inDbSession) {
	L_Q();
	bool ret = false;
	if (!q->isInitialized()) {
		lWarning() << "Unable to import legacy friend because the database has not been initialized";
		return false;
	}

	L_DB_TRANSACTION_C(q) {
		if (getModuleVersion("legacy-friends-import") >= makeVersion(1, 0, 0)) return;

		soci::session *inSession = inDbSession.getBackendSession();
		if (!checkLegacyFriendsTableExists(*inSession)) return;

		unordered_map<int, long long> resolvedListsIds;
		soci::session *session = dbSession.getBackendSession();

		soci::rowset<soci::row> friendsLists = (inSession->prepare << "SELECT * FROM friends_lists");

		set<string> names;
		for (const auto &friendList : friendsLists) {
			const string &name = friendList.get<string>(LegacyFriendListColName, "");
			const string &rlsUri = friendList.get<string>(LegacyFriendListColRlsUri, "");
			const string &syncUri = friendList.get<string>(LegacyFriendListColSyncUri, "");
			const int &revision = friendList.get<int>(LegacyFriendListColRevision, 0);
			string ctag = std::to_string(revision);
			int type = -1;

			string uniqueName = name;
			for (int id = 0; names.find(uniqueName) != names.end(); uniqueName = name + "-" + Utils::toString(id++))
				;
			names.insert(uniqueName);

			*session << "INSERT INTO friends_list (name, rls_uri, sync_uri, revision, type, ctag) VALUES ("
			            "  :name, :rlsUri, :syncUri, 0, :type, :ctag"
			            ")",
			    soci::use(uniqueName), soci::use(rlsUri), soci::use(syncUri), soci::use(type), soci::use(ctag);
			resolvedListsIds[friendList.get<int>(LegacyFriendListColId)] = dbSession.getLastInsertId();
		}

		soci::rowset<soci::row> friends = (inSession->prepare << "SELECT * FROM friends");
		for (const auto &friendInfo : friends) {
			long long friendsListId;
			{
				auto it = resolvedListsIds.find(friendInfo.get<int>(LegacyFriendColFriendListId, -1));
				if (it == resolvedListsIds.end()) continue;
				friendsListId = it->second;
			}

			long long sipAddressId = -1;
			soci::indicator sipAddressIndicator = soci::i_null;
			const auto legacyFriendAddress = Address::create(friendInfo.get<string>(LegacyFriendColSipAddress, ""));
			if (legacyFriendAddress) {
				sipAddressId = insertSipAddress(legacyFriendAddress);
				sipAddressIndicator = soci::i_ok;
			}
			const int &subscribePolicy = friendInfo.get<int>(LegacyFriendColSubscribePolicy, LinphoneSPAccept);
			const int &sendSubscribe = friendInfo.get<int>(LegacyFriendColSendSubscribe, 1);
			const string &refKey = friendInfo.get<string>(LegacyFriendColRefKey, "");
			const string &vCard = friendInfo.get<string>(LegacyFriendColVCard, "");
			const string &vCardEtag = friendInfo.get<string>(LegacyFriendColVCardEtag, "");
			const string &vCardSyncUri = friendInfo.get<string>(LegacyFriendColVCardSyncUri, "");
			const int &presenceReveived = friendInfo.get<int>(LegacyFriendColPresenceReceived, 0);
			const int &starred = 0;

			*session << "INSERT INTO friend ("
			            "  sip_address_id, friends_list_id, subscribe_policy, send_subscribe, ref_key,"
			            "  presence_received, starred, v_card, v_card_etag, v_card_sync_uri"
			            ") VALUES ("
			            "  :sipAddressId, :friendsListId, :subscribePolicy, :sendSubscribe, :refKey,"
			            "  :presenceReceived, :starred, :vCard, :vCardEtag, :vCardSyncUri"
			            ")",
			    soci::use(sipAddressId, sipAddressIndicator), soci::use(friendsListId), soci::use(subscribePolicy),
			    soci::use(sendSubscribe), soci::use(refKey), soci::use(presenceReveived), soci::use(starred),
			    soci::use(vCard), soci::use(vCardEtag), soci::use(vCardSyncUri);
		}
		tr.commit();

		// Only update the module version once the import has been done.
		updateModuleVersion("legacy-friends-import", ModuleVersionLegacyFriendsImport);

		lInfo() << "Successful import of legacy friends.";
		ret = true;
	};
	return ret;
}

#ifdef HAVE_XML2
// TODO: Move in a helper file? With others xml.
struct XmlCharObjectDeleter {
	void operator()(void *ptr) const {
		xmlFree(ptr);
	}
};
using XmlCharObject = unique_ptr<xmlChar, XmlCharObjectDeleter>;

struct XmlDocObjectDeleter {
	void operator()(xmlDocPtr ptr) const {
		xmlFreeDoc(ptr);
	}
};
using XmlDocObject = unique_ptr<remove_pointer<xmlDocPtr>::type, XmlDocObjectDeleter>;

typedef const xmlChar *XmlCharPtr;
#endif /* HAVE_XML2 */

static string extractLegacyFileContentType(const string &xml) {
#ifdef HAVE_XML2
	XmlDocObject xmlMessageBody(xmlParseDoc(XmlCharPtr(xml.c_str())));
	xmlNodePtr xmlElement = xmlDocGetRootElement(xmlMessageBody.get());
	if (!xmlElement) return "";

	for (xmlElement = xmlElement->xmlChildrenNode; xmlElement; xmlElement = xmlElement->next) {
		if (xmlStrcmp(xmlElement->name, XmlCharPtr("file-info"))) continue;

		XmlCharObject typeAttribute(xmlGetProp(xmlElement, XmlCharPtr("type")));
		if (xmlStrcmp(typeAttribute.get(), XmlCharPtr("file"))) continue;

		for (xmlElement = xmlElement->xmlChildrenNode; xmlElement; xmlElement = xmlElement->next)
			if (!xmlStrcmp(xmlElement->name, XmlCharPtr("content-type"))) {
				XmlCharObject xmlContentType(
				    xmlNodeListGetString(xmlMessageBody.get(), xmlElement->xmlChildrenNode, 1));
				return ContentType(reinterpret_cast<const char *>(xmlContentType.get())).getMediaType();
			}
	}
#endif /* HAVE_XML2 */

	return "";
}

bool MainDbPrivate::importLegacyHistory(DbSession &inDbSession) {
	L_Q();
	bool ret = false;

	if (!q->isInitialized()) {
		lWarning() << "Unable to import legacy history because the database has not been initialized";
		return false;
	}

	L_DB_TRANSACTION_C(q) {
		if (getModuleVersion("legacy-history-import") >= makeVersion(1, 0, 0)) return;

		soci::session *inSession = inDbSession.getBackendSession();
		if (!checkLegacyHistoryTableExists(*inSession)) return;

		soci::rowset<soci::row> messages = (inSession->prepare << "SELECT * FROM history");
		for (const auto &message : messages) {
			const int direction = message.get<int>(LegacyMessageColDirection);
			if (direction != 0 && direction != 1) {
				lWarning() << "Unable to import legacy message with invalid direction.";
				continue;
			}

			const int &state = message.get<int>(LegacyMessageColState, int(ChatMessage::State::Displayed));
			if (state < 0 || state > int(ChatMessage::State::Displayed)) {
				lWarning() << "Unable to import legacy message with invalid state.";
				continue;
			}

			auto time = message.get<int>(LegacyMessageColDate, 0);
			auto creationTime = dbSession.getTimeWithSociIndicator(time);

			bool isNull;
			getValueFromRow<string>(message, LegacyMessageColUrl, isNull);

			const int &contentId = message.get<int>(LegacyMessageColContentId, -1);
			ContentType contentType(message.get<string>(LegacyMessageColContentType, ""));
			if (!contentType.isValid())
				contentType = contentId != -1 ? ContentType::FileTransfer
				                              : (isNull ? ContentType::PlainText : ContentType::ExternalBody);
			if (contentType == ContentType::ExternalBody) {
				lInfo() << "Import of external body content is skipped.";
				continue;
			}

			const string &text = getValueFromRow<string>(message, LegacyMessageColText, isNull);

			shared_ptr<Content> content;
			if (contentType == ContentType::FileTransfer) {
				const string appData = getValueFromRow<string>(message, LegacyMessageColAppData, isNull);
				if (isNull) {
					lWarning() << "Unable to import legacy file message without app data.";
					continue;
				}

				string contentTypeString = extractLegacyFileContentType(text);
				if (contentTypeString.empty()) {
					lWarning() << "Unable to extract file content type form legacy transfer message";
					continue;
				}
				ContentType fileContentType(contentTypeString);
				content = FileContent::create<FileContent>();
				content->setContentType(fileContentType);
				content->setProperty("legacy", Variant{appData});
				content->setBodyFromLocale(text);
			} else {
				content = Content::create();
				content->setContentType(contentType);
				if (contentType == ContentType::PlainText) {
					if (isNull) {
						lWarning() << "Unable to import legacy message with no text.";
						continue;
					}
					content->setBodyFromLocale(text);
				} else {
					lWarning() << "Unable to import unsupported legacy content.";
					continue;
				}
			}

			soci::session *session = dbSession.getBackendSession();
			const int &eventType = int(EventLog::Type::ConferenceChatMessage);
			*session << "INSERT INTO event (type, creation_time) VALUES (:type, :creationTime)", soci::use(eventType),
			    soci::use(creationTime.first, creationTime.second);
			const long long &eventId = dbSession.getLastInsertId();
			const auto localAddress = Address(message.get<string>(LegacyMessageColLocalAddress));
			const long long &localSipAddressId = insertSipAddress(localAddress);
			const auto remoteAddress = Address(message.get<string>(LegacyMessageColRemoteAddress));
			const long long &remoteSipAddressId = insertSipAddress(remoteAddress);
			const long long &chatRoomId =
			    insertOrUpdateImportedBasicChatRoom(remoteSipAddressId, localSipAddressId, time);
			const int &isSecured = message.get<int>(LegacyMessageColIsSecured, 0);
			const int deliveryNotificationRequired = 0;
			const int displayNotificationRequired = 0;

			*session << "INSERT INTO conference_event (event_id, chat_room_id)"
			            " VALUES (:eventId, :chatRoomId)",
			    soci::use(eventId), soci::use(chatRoomId);

			*session << "INSERT INTO conference_chat_message_event ("
			            "  event_id, from_sip_address_id, to_sip_address_id,"
			            "  time, state, direction, imdn_message_id, is_secured,"
			            "  delivery_notification_required, display_notification_required,"
			            "  marked_as_read"
			            ") VALUES ("
			            "  :eventId, :localSipAddressId, :remoteSipAddressId,"
			            "  :creationTime, :state, :direction, '', :isSecured,"
			            "  :deliveryNotificationRequired, :displayNotificationRequired,"
			            "  1"
			            ")",
			    soci::use(eventId), soci::use(localSipAddressId), soci::use(remoteSipAddressId),
			    soci::use(creationTime.first, creationTime.second), soci::use(state), soci::use(direction),
			    soci::use(isSecured), soci::use(deliveryNotificationRequired), soci::use(displayNotificationRequired);

			if (content) insertContent(eventId, *content);
			insertChatRoomParticipant(chatRoomId, remoteSipAddressId, false);
			insertChatMessageParticipant(eventId, remoteSipAddressId, state, std::time(nullptr));
		}
		// Set last_message_id to the last timed message for all chat room
		*dbSession.getBackendSession()
		    << "UPDATE chat_room SET last_message_id = "
		       "(SELECT COALESCE( (SELECT max(conference_event.event_id) as m " // max(conference_event.event_id)
		                                                                        // ensure to have only one event and
		                                                                        // the last id for the matching time
		       "FROM conference_event, conference_chat_message_event,"
		       "(SELECT max(time) as t, conference_event.chat_room_id as c " // Get Max Time for the chat room
		       "FROM conference_event, conference_chat_message_event "
		       "WHERE conference_event.event_id=conference_chat_message_event.event_id GROUP BY "
		       "conference_event.chat_room_id)"
		       "WHERE conference_chat_message_event.time=t AND "
		       "conference_chat_message_event.event_id=conference_event.event_id AND "
		       "conference_event.chat_room_id=c "
		       "AND conference_event.chat_room_id=chat_room.id "
		       "GROUP BY conference_event.chat_room_id),0))"; // if there are no messages, the first is NULL. So put
		                                                      // a 0 to the ID
		tr.commit();

		// Only update the module version once the import has been done.
		updateModuleVersion("legacy-history-import", ModuleVersionLegacyHistoryImport);

		lInfo() << "Successful import of legacy messages.";
		ret = true;
	};
	return ret;
}

bool MainDbPrivate::importLegacyCallLogs(DbSession &inDbSession) {
	L_Q();
	bool ret = false;

	if (!q->isInitialized()) {
		lWarning() << "Unable to import legacy call logs because the database has not been initialized";
		return false;
	}

	L_DB_TRANSACTION_C(q) {
		if (getModuleVersion("legacy-call-logs-import") >= makeVersion(1, 0, 0)) return;

		soci::session *inSession = inDbSession.getBackendSession();
		if (!checkLegacyCallLogsTableExists(*inSession)) return;

		soci::rowset<soci::row> logs = (inSession->prepare << "SELECT * FROM call_history");
		for (const auto &log : logs) {
			const int direction = log.get<int>(LegacyCallLogColDirection);
			if (direction != 0 && direction != 1) {
				lWarning() << "Unable to import legacy call log with invalid direction.";
				continue;
			}

			const int status = log.get<int>(LegacyCallLogColStatus);
			if (status < 0 || status > LinphoneCallDeclinedElsewhere) {
				lWarning() << "Unable to import legacy call log with invalid status.";
				continue;
			}

			const std::shared_ptr<Address> from = Address::create(log.get<string>(LegacyCallLogColFrom));
			const std::shared_ptr<Address> to = Address::create(log.get<string>(LegacyCallLogColTo));
			auto callLog = CallLog::create(q->getCore(), static_cast<LinphoneCallDir>(direction), from, to);

			callLog->setDuration(log.get<int>(LegacyCallLogColDuration));
			callLog->setStartTime((time_t)std::stoul(log.get<string>(LegacyCallLogColStartTime)));
			callLog->setConnectedTime((time_t)std::stoul(log.get<string>(LegacyCallLogColConnectedTime)));
			callLog->setStatus(static_cast<LinphoneCallStatus>(status));
			callLog->setVideoEnabled(!!log.get<int>(LegacyCallLogColVideoEnabled));
			callLog->setQuality((float)log.get<double>(LegacyCallLogColQuality));

			soci::indicator ind = log.get_indicator(LegacyCallLogColCallId);
			if (ind == soci::i_ok) callLog->setCallId(log.get<string>(LegacyCallLogColCallId));

			ind = log.get_indicator(LegacyCallLogColRefKey);
			if (ind == soci::i_ok) callLog->setRefKey(log.get<string>(LegacyCallLogColRefKey));

			insertOrUpdateConferenceCall(callLog, nullptr);
		}

		tr.commit();

		// Only update the module version once the import has been done.
		updateModuleVersion("legacy-call-logs-import", ModuleVersionLegacyCallLogsImport);

		lInfo() << "Successful import of legacy call logs.";
		ret = true;
	};
	return ret;
}
#endif

// =============================================================================

MainDb::MainDb(const shared_ptr<Core> &core) : AbstractDb(*new MainDbPrivate), CoreAccessor(core) {
}

void MainDb::init() {
#ifdef HAVE_DB_STORAGE
	L_D();

	Backend backend = getBackend();
	const string charset = backend == Mysql ? "DEFAULT CHARSET=utf8mb4" : "";
	soci::session *session = d->dbSession.getBackendSession();

	using namespace placeholders;
	auto primaryKeyRefStr = bind(&DbSession::primaryKeyRefStr, &d->dbSession, _1);
	auto primaryKeyStr = bind(&DbSession::primaryKeyStr, &d->dbSession, _1);
	auto timestampType = bind(&DbSession::timestampType, &d->dbSession);
	auto varcharPrimaryKeyStr = bind(&DbSession::varcharPrimaryKeyStr, &d->dbSession, _1);

	initCleanup();

	session->begin();

	try {
		/* Enable secure delete - so that erased chat messages are really erased and not just marked as unused.
		 * See https://sqlite.org/pragma.html#pragma_secure_delete
		 * This setting is global for the database.
		 * It is enabled only for sqlite3 backend, which is the one used for liblinphone clients.
		 * The mysql backend (used server-side) doesn't support this PRAGMA.
		 */
		if (backend == Sqlite3) *session << string("PRAGMA secure_delete = ON");

		// Charset set to ascii for mysql/mariadb to allow creation of indexed collumns of size > 191. We assume
		// that for the given fields ascii will not cause any display issue.

		// 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4

		*session << "CREATE TABLE IF NOT EXISTS sip_address ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","
		                "  value VARCHAR(255) UNIQUE NOT NULL"
		                ") " +
		                (backend == Mysql ? "DEFAULT CHARSET=ascii" : "");

		*session << "CREATE TABLE IF NOT EXISTS content_type ("
		            "  id" +
		                primaryKeyStr("SMALLINT UNSIGNED") +
		                ","
		                "  value VARCHAR(255) UNIQUE NOT NULL"
		                ") " +
		                (backend == Mysql ? "DEFAULT CHARSET=ascii" : "");

		*session << "CREATE TABLE IF NOT EXISTS event ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","
		                "  type TINYINT UNSIGNED NOT NULL,"
		                "  creation_time" +
		                timestampType() +
		                " NOT NULL"
		                ") " +
		                charset;

		*session
		    << "CREATE TABLE IF NOT EXISTS chat_room ("
		       "  id" +
		           primaryKeyStr("BIGINT UNSIGNED") +
		           ","

		           // Server (for conference) or user sip address.
		           "  peer_sip_address_id" +
		           primaryKeyRefStr("BIGINT UNSIGNED") +
		           " NOT NULL,"

		           "  local_sip_address_id" +
		           primaryKeyRefStr("BIGINT UNSIGNED") +
		           " NOT NULL,"

		           // Dialog creation time.
		           "  creation_time" +
		           timestampType() +
		           " NOT NULL,"

		           // Last event time (call, message...).
		           "  last_update_time" +
		           timestampType() +
		           " NOT NULL,"

		           // ConferenceChatRoom, BasicChatRoom, RTT...
		           "  capabilities TINYINT UNSIGNED NOT NULL,"

		           // Chatroom subject.
		           // /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY
		           // or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
		           "  subject VARCHAR(255),"

		           "  last_notify_id INT UNSIGNED DEFAULT 0,"

		           "  flags INT UNSIGNED DEFAULT 0,"

		           "  UNIQUE (peer_sip_address_id, local_sip_address_id),"

		           "  FOREIGN KEY (peer_sip_address_id)"
		           "    REFERENCES sip_address(id)"
		           "    ON DELETE CASCADE,"
		           "  FOREIGN KEY (local_sip_address_id)"
		           "    REFERENCES sip_address(id)"
		           "    ON DELETE CASCADE"
		           ") " +
		           charset;

		*session << "CREATE TABLE IF NOT EXISTS one_to_one_chat_room ("
		            "  chat_room_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  participant_a_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  participant_b_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"

		                "  FOREIGN KEY (chat_room_id)"
		                "    REFERENCES chat_room(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_a_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_b_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS chat_room_participant ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  chat_room_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","
		                "  participant_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","

		                "  is_admin BOOLEAN NOT NULL,"

		                "  UNIQUE (chat_room_id, participant_sip_address_id),"

		                "  FOREIGN KEY (chat_room_id)"
		                "    REFERENCES chat_room(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS chat_room_participant_device ("
		            "  chat_room_participant_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","
		                "  participant_device_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","

		                "  PRIMARY KEY (chat_room_participant_id, participant_device_sip_address_id),"

		                "  FOREIGN KEY (chat_room_participant_id)"
		                "    REFERENCES chat_room_participant(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_device_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_event ("
		            "  event_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  chat_room_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES event(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (chat_room_id)"
		                "    REFERENCES chat_room(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_notified_event ("
		            "  event_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  notify_id INT UNSIGNED NOT NULL,"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES conference_event(event_id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_participant_event ("
		            "  event_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  participant_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES conference_notified_event(event_id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_participant_device_event ("
		            "  event_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  device_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES conference_participant_event(event_id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (device_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session
		    << "CREATE TABLE IF NOT EXISTS conference_security_event ("
		       "  event_id" +
		           primaryKeyStr("BIGINT UNSIGNED") +
		           ","

		           "  security_alert TINYINT UNSIGNED NOT NULL,"
		           // /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY
		           // or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
		           "  faulty_device VARCHAR(255) NOT NULL,"

		           "  FOREIGN KEY (event_id)"
		           "    REFERENCES conference_event(event_id)"
		           "    ON DELETE CASCADE"
		           ") " +
		           charset;

		*session
		    << "CREATE TABLE IF NOT EXISTS conference_subject_event ("
		       "  event_id" +
		           primaryKeyStr("BIGINT UNSIGNED") +
		           ","

		           // /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY
		           // or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
		           "  subject VARCHAR(255) NOT NULL,"

		           "  FOREIGN KEY (event_id)"
		           "    REFERENCES conference_notified_event(event_id)"
		           "    ON DELETE CASCADE"
		           ") " +
		           charset;

		*session
		    << "CREATE TABLE IF NOT EXISTS conference_chat_message_event ("
		       "  event_id" +
		           primaryKeyStr("BIGINT UNSIGNED") +
		           ","

		           "  from_sip_address_id" +
		           primaryKeyRefStr("BIGINT UNSIGNED") +
		           " NOT NULL,"
		           "  to_sip_address_id" +
		           primaryKeyRefStr("BIGINT UNSIGNED") +
		           " NOT NULL,"

		           "  time" +
		           timestampType() +
		           " ,"

		           // See: https://tools.ietf.org/html/rfc5438#section-6.3
		           // /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY
		           // or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
		           "  imdn_message_id VARCHAR(255) NOT NULL,"

		           "  state TINYINT UNSIGNED NOT NULL,"
		           "  direction TINYINT UNSIGNED NOT NULL,"
		           "  is_secured BOOLEAN NOT NULL,"

		           "  FOREIGN KEY (event_id)"
		           "    REFERENCES conference_event(event_id)"
		           "    ON DELETE CASCADE,"
		           "  FOREIGN KEY (from_sip_address_id)"
		           "    REFERENCES sip_address(id)"
		           "    ON DELETE CASCADE,"
		           "  FOREIGN KEY (to_sip_address_id)"
		           "    REFERENCES sip_address(id)"
		           "    ON DELETE CASCADE"
		           ") " +
		           charset;

		*session << "CREATE TABLE IF NOT EXISTS chat_message_participant ("
		            "  event_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","
		                "  participant_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","
		                "  state TINYINT UNSIGNED NOT NULL,"

		                "  PRIMARY KEY (event_id, participant_sip_address_id),"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES conference_chat_message_event(event_id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS chat_message_content ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  event_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  content_type_id" +
		                primaryKeyRefStr("SMALLINT UNSIGNED") +
		                " NOT NULL,"
		                "  body TEXT NOT NULL,"

		                "  UNIQUE (id, event_id),"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES conference_chat_message_event(event_id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (content_type_id)"
		                "    REFERENCES content_type(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session
		    << "CREATE TABLE IF NOT EXISTS chat_message_file_content ("
		       "  chat_message_content_id" +
		           primaryKeyStr("BIGINT UNSIGNED") +
		           ","

		           // /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY
		           // or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
		           "  name VARCHAR(256) NOT NULL,"
		           "  size INT UNSIGNED NOT NULL,"

		           // /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY
		           // or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
		           "  path VARCHAR(512) NOT NULL,"

		           "  FOREIGN KEY (chat_message_content_id)"
		           "    REFERENCES chat_message_content(id)"
		           "    ON DELETE CASCADE"
		           ") " +
		           charset;

		*session << "CREATE TABLE IF NOT EXISTS chat_message_content_app_data ("
		            "  chat_message_content_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","

		                "  name VARCHAR(191)," // 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7
		                                       // with charset utf8mb4
		                "  data BLOB NOT NULL,"

		                "  PRIMARY KEY (chat_message_content_id, name),"
		                "  FOREIGN KEY (chat_message_content_id)"
		                "    REFERENCES chat_message_content(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_message_crypto_data ("
		            "  event_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","

		                "  name VARCHAR(191)," // 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7
		                                       // with charset utf8mb4
		                "  data BLOB NOT NULL,"

		                "  PRIMARY KEY (event_id, name),"
		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES conference_chat_message_event(event_id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS friends_list ("
		            "  id" +
		                primaryKeyStr("INT UNSIGNED") +
		                ","

		                "  name VARCHAR(191) UNIQUE," // 191 = max indexable (KEY or UNIQUE) varchar size for mysql
		                                              // < 5.7 with charset utf8mb4

		                // /!\ Warning : if varchar columns > 255 are indexed, their size must be set back to 191 =
		                // max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both
		                // here and in migrations)
		                "  rls_uri VARCHAR(2047),"
		                "  sync_uri VARCHAR(2047),"
		                "  revision INT UNSIGNED NOT NULL"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS friend ("
		            "  id" +
		                primaryKeyStr("INT UNSIGNED") +
		                ","

		                "  sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  friends_list_id" +
		                primaryKeyRefStr("INT UNSIGNED") +
		                " NOT NULL,"

		                "  subscribe_policy TINYINT UNSIGNED NOT NULL,"
		                "  send_subscribe BOOLEAN NOT NULL,"
		                "  presence_received BOOLEAN NOT NULL,"

		                "  v_card MEDIUMTEXT,"

		                // /!\ Warning : if these varchar columns are indexed, their size must be set back to 191 =
		                // max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both
		                // here and in migrations)
		                "  v_card_etag VARCHAR(255),"
		                "  v_card_sync_uri VARCHAR(2047),"

		                "  FOREIGN KEY (sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (friends_list_id)"
		                "    REFERENCES friends_list(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS friend_app_data ("
		            "  friend_id" +
		                primaryKeyRefStr("INT UNSIGNED") +
		                ","

		                "  name VARCHAR(191)," // 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7
		                                       // with charset utf8mb4
		                "  data BLOB NOT NULL,"

		                "  PRIMARY KEY (friend_id, name),"
		                "  FOREIGN KEY (friend_id)"
		                "    REFERENCES friend(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS db_module_version ("
		            "  name" +
		                varcharPrimaryKeyStr(191) +
		                "," // 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4
		                "  version INT UNSIGNED NOT NULL"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS chat_message_ephemeral_event ("
		            "  event_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","
		                "  ephemeral_lifetime DOUBLE NOT NULL,"
		                "  expired_time" +
		                timestampType() +
		                " NOT NULL,"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES conference_event(event_id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_ephemeral_message_event ("
		            "  event_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  lifetime DOUBLE NOT NULL,"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES conference_event(event_id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS one_to_one_chat_room_previous_conference_id ("
		            "  id" +
		                primaryKeyStr("INT UNSIGNED") +
		                ","

		                "  sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  chat_room_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"

		                "  FOREIGN KEY (sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (chat_room_id)"
		                "    REFERENCES chat_room(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS expired_conferences ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  uri_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_info ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  organizer_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  uri_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  start_time" +
		                timestampType() +
		                ","
		                "  duration INT UNSIGNED,"

		                // /!\ Warning : if these varchar columns are indexed, their size must be set back to 191 =
		                // max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both
		                // here and in migrations)
		                "  subject VARCHAR(256) NOT NULL,"
		                "  description VARCHAR(2048),"

		                "  FOREIGN KEY (organizer_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (uri_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_info_participant ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  conference_info_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  participant_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"

		                "  UNIQUE (conference_info_id, participant_sip_address_id),"

		                "  FOREIGN KEY (conference_info_id)"
		                "    REFERENCES conference_info(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (participant_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_info_organizer ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  conference_info_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  organizer_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  params VARCHAR(2048) DEFAULT '',"

		                "  UNIQUE (conference_info_id, organizer_sip_address_id),"

		                "  FOREIGN KEY (conference_info_id)"
		                "    REFERENCES conference_info(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (organizer_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_info_participant_params ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  conference_info_participant_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  name VARCHAR(191) NOT NULL DEFAULT '',"
		                "  value VARCHAR(191) DEFAULT '',"

		                "  UNIQUE (conference_info_participant_id, name),"

		                "  FOREIGN KEY (conference_info_participant_id)"
		                "    REFERENCES conference_info_participant(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_call ("
		            "  id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  from_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  to_sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  direction TINYINT UNSIGNED NOT NULL,"
		                "  duration INT UNSIGNED,"
		                "  start_time" +
		                timestampType() +
		                " NOT NULL,"
		                "  connected_time" +
		                timestampType() +
		                ","
		                "  status TINYINT UNSIGNED NOT NULL,"
		                "  video_enabled BOOLEAN NOT NULL,"
		                "  quality DOUBLE,"
		                "  call_id VARCHAR(64),"
		                "  refkey VARCHAR(64),"
		                "  conference_info_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                ","

		                "  FOREIGN KEY (from_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (to_sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (conference_info_id)"
		                "    REFERENCES conference_info(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "CREATE TABLE IF NOT EXISTS conference_call_event ("
		            "  event_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  conference_call_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"

		                "  FOREIGN KEY (event_id)"
		                "    REFERENCES event(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (conference_call_id)"
		                "    REFERENCES conference_call(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		*session << "DROP VIEW IF EXISTS conference_call_event_view";
		*session << "CREATE VIEW conference_call_event_view AS"
		            "  SELECT event.id, type, creation_time, conference_call_id, from_sip_address_id, "
		            "to_sip_address_id, direction, conference_call.duration AS call_duration,"
		            "    conference_call.start_time AS call_start_time, connected_time, status, video_enabled, "
		            "quality, call_id, refkey, conference_info_id,"
		            "    organizer_sip_address_id, uri_sip_address_id, conference_info.start_time AS conf_start_time, "
		            "conference_info.duration AS conf_duration,"
		            "    subject, description"
		            "  FROM event"
		            "  LEFT JOIN conference_call_event ON conference_call_event.event_id = event.id"
		            "  LEFT JOIN conference_call ON conference_call.id = conference_call_event.conference_call_id"
		            "  LEFT JOIN conference_info ON conference_info.id = conference_call.conference_info_id";

		*session
		    << "CREATE TABLE IF NOT EXISTS conference_chat_message_reaction_event ("
		       "  event_id" +
		           primaryKeyStr("BIGINT UNSIGNED") +
		           ","

		           "  from_sip_address_id" +
		           primaryKeyRefStr("BIGINT UNSIGNED") +
		           " NOT NULL,"
		           "  to_sip_address_id" +
		           primaryKeyRefStr("BIGINT UNSIGNED") +
		           " NOT NULL,"

		           "  time" +
		           timestampType() +
		           " ,"
		           "  body TEXT NOT NULL,"

		           // See: https://tools.ietf.org/html/rfc5438#section-6.3
		           // /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY
		           // or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
		           "  imdn_message_id VARCHAR(255) NOT NULL,"
		           "  call_id VARCHAR(255) NOT NULL,"
		           "  reaction_to_message_id VARCHAR(191) NOT NULL,"

		           // One reaction maximum per user for a given message
		           "  UNIQUE (from_sip_address_id, reaction_to_message_id),"

		           "  FOREIGN KEY (from_sip_address_id)"
		           "    REFERENCES sip_address(id)"
		           "    ON DELETE CASCADE,"
		           "  FOREIGN KEY (to_sip_address_id)"
		           "    REFERENCES sip_address(id)"
		           "    ON DELETE CASCADE"
		           ") " +
		           charset;

		*session << "CREATE TABLE IF NOT EXISTS friend_devices ("
		            "  device_id" +
		                primaryKeyStr("BIGINT UNSIGNED") +
		                ","

		                "  sip_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"
		                "  device_address_id" +
		                primaryKeyRefStr("BIGINT UNSIGNED") +
		                " NOT NULL,"

		                "  display_name TEXT NOT NULL," +

		                "  UNIQUE (sip_address_id, device_address_id),"

		                "  FOREIGN KEY (sip_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE,"
		                "  FOREIGN KEY (device_address_id)"
		                "    REFERENCES sip_address(id)"
		                "    ON DELETE CASCADE"
		                ") " +
		                charset;

		d->updateSchema();

		migrateConferenceInfos();

		d->updateModuleVersion("events", ModuleVersionEvents);
		d->updateModuleVersion("friends", ModuleVersionFriends);
	} catch (const soci::soci_error &e) {
		lError() << "Exception while creating or updating the database's schema : " << e.what();
		session->rollback();
		// Throw exception so that it can be catched by the calling function
		throw e;
		return;
	}
	session->commit();

	initCleanup();
#endif
}

void MainDb::migrateConferenceInfos() {
#ifdef HAVE_DB_STORAGE
	L_D();
	// Search all conference information whose URI has the gr parameter in order to drop it.
	// This will ensure the backward compatiblity for future releases of the SDK
	std::string query =
	    "SELECT conference_info.id, uri_sip_address.value  FROM conference_info, sip_address AS uri_sip_address "
	    "WHERE "
	    "conference_info.uri_sip_address_id = uri_sip_address.id AND uri_sip_address.value LIKE '%gr=%'";
	soci::session *session = d->dbSession.getBackendSession();
	soci::rowset<soci::row> rows = (session->prepare << query);

	for (const auto &row : rows) {
		const long long &dbConferenceInfoId = d->dbSession.resolveId(row, 0);
		const std::string uriString = row.get<string>(1);
		Address uri(uriString);
		// Update conference address to ensure that a conference info can be successfully searched by its
		// address
		const long long &uriSipAddressId = d->insertSipAddress(uri.getUriWithoutGruu());
		*session << "UPDATE conference_info SET uri_sip_address_id = :uriSipAddressId WHERE id = :conferenceInfoId",
		    soci::use(uriSipAddressId), soci::use(dbConferenceInfoId);
	}
#endif
}

void MainDb::initCleanup() {
#ifdef HAVE_DB_STORAGE
	L_D();

	soci::session *session = d->dbSession.getBackendSession();

	session->begin();

	try {
		*session << "DROP TABLE IF EXISTS chat_room_participant_device_clone";
		*session << "DROP TABLE IF EXISTS conference_info_participant_clone";
	} catch (const soci::soci_error &e) {
		lError() << "Exception while executing database's schema cleanup : " << e.what();
		session->rollback();
		// Throw exception so that it can be catched by the calling function
		throw e;
		return;
	}
	session->commit();
#endif
}

bool MainDb::addEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	if (!isInitialized()) {
		lWarning() << "Database has not been initialized";
		return false;
	}
	if (eventLog->getPrivate()->dbKey.isValid()) {
		lWarning() << "Unable to add an event twice!!!";
		return false;
	}

	return L_DB_TRANSACTION {
		L_D();

		long long eventId = -1;

		EventLog::Type type = eventLog->getType();
		lInfo() << "MainDb::addEvent() of type " << type << " (value " << static_cast<int>(type) << ")";
		switch (type) {
			case EventLog::Type::None:
			case EventLog::Type::ConferenceAllowedParticipantListChanged:
				return false;

			case EventLog::Type::ConferenceCreated:
			case EventLog::Type::ConferenceTerminated:
				eventId = d->insertConferenceEvent(eventLog);
				break;

			case EventLog::Type::ConferenceCallStarted:
			case EventLog::Type::ConferenceCallConnected:
			case EventLog::Type::ConferenceCallEnded:
				eventId = d->insertConferenceCallEvent(eventLog);
				break;

			case EventLog::Type::ConferenceChatMessage:
				eventId = d->insertConferenceChatMessageEvent(eventLog);
				break;

			case EventLog::Type::ConferenceChatMessageReaction:
				eventId = d->insertConferenceChatMessageReactionEvent(eventLog);
				break;

			case EventLog::Type::ConferenceParticipantAdded:
			case EventLog::Type::ConferenceParticipantRemoved:
			case EventLog::Type::ConferenceParticipantRoleUnknown:
			case EventLog::Type::ConferenceParticipantRoleSpeaker:
			case EventLog::Type::ConferenceParticipantRoleListener:
			case EventLog::Type::ConferenceParticipantSetAdmin:
			case EventLog::Type::ConferenceParticipantUnsetAdmin:
				eventId = d->insertConferenceParticipantEvent(eventLog);
				break;

			case EventLog::Type::ConferenceParticipantDeviceAdded:
			case EventLog::Type::ConferenceParticipantDeviceRemoved:
			case EventLog::Type::ConferenceParticipantDeviceJoiningRequest:
			case EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged:
			case EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged:
			case EventLog::Type::ConferenceParticipantDeviceStatusChanged:
				eventId = d->insertConferenceParticipantDeviceEvent(eventLog);
				break;

			case EventLog::Type::ConferenceSecurityEvent:
				eventId = d->insertConferenceSecurityEvent(eventLog);
				break;

			case EventLog::Type::ConferenceAvailableMediaChanged:
				eventId = d->insertConferenceAvailableMediaEvent(eventLog);
				break;

			case EventLog::Type::ConferenceSubjectChanged:
				eventId = d->insertConferenceSubjectEvent(eventLog);
				break;

			case EventLog::Type::ConferenceEphemeralMessageLifetimeChanged:
			case EventLog::Type::ConferenceEphemeralMessageEnabled:
			case EventLog::Type::ConferenceEphemeralMessageDisabled:
			case EventLog::Type::ConferenceEphemeralMessageManagedByAdmin:
			case EventLog::Type::ConferenceEphemeralMessageManagedByParticipants:
				eventId = d->insertConferenceEphemeralMessageEvent(eventLog);
				break;
		}

		if (eventId >= 0) {
			tr.commit();
			d->cache(eventLog, eventId);

			if (type == EventLog::Type::ConferenceChatMessage)
				d->cache(static_pointer_cast<ConferenceChatMessageEvent>(eventLog)->getChatMessage(), eventId);

			return true;
		}
		lError() << "MainDb::addEvent() of type " << type << " failed.";
		return false;
	};
#else
	return false;
#endif
}

bool MainDb::updateEvent(const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	if (!eventLog->getPrivate()->dbKey.isValid()) {
		lWarning() << "Unable to update an event that wasn't inserted yet!!!";
		return false;
	}

	return L_DB_TRANSACTION {
		L_D();

		switch (eventLog->getType()) {
			case EventLog::Type::None:
				return false;

			case EventLog::Type::ConferenceChatMessage:
				d->updateConferenceChatMessageEvent(eventLog);
				break;

			case EventLog::Type::ConferenceChatMessageReaction:
			case EventLog::Type::ConferenceCreated:
			case EventLog::Type::ConferenceTerminated:
			case EventLog::Type::ConferenceCallStarted:
			case EventLog::Type::ConferenceCallConnected:
			case EventLog::Type::ConferenceCallEnded:
			case EventLog::Type::ConferenceParticipantAdded:
			case EventLog::Type::ConferenceParticipantRemoved:
			case EventLog::Type::ConferenceParticipantRoleUnknown:
			case EventLog::Type::ConferenceParticipantRoleSpeaker:
			case EventLog::Type::ConferenceParticipantRoleListener:
			case EventLog::Type::ConferenceParticipantSetAdmin:
			case EventLog::Type::ConferenceParticipantUnsetAdmin:
			case EventLog::Type::ConferenceParticipantDeviceAdded:
			case EventLog::Type::ConferenceParticipantDeviceRemoved:
			case EventLog::Type::ConferenceParticipantDeviceJoiningRequest:
			case EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged:
			case EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged:
			case EventLog::Type::ConferenceParticipantDeviceStatusChanged:
			case EventLog::Type::ConferenceSecurityEvent:
			case EventLog::Type::ConferenceAvailableMediaChanged:
			case EventLog::Type::ConferenceSubjectChanged:
			case EventLog::Type::ConferenceEphemeralMessageLifetimeChanged:
			case EventLog::Type::ConferenceEphemeralMessageEnabled:
			case EventLog::Type::ConferenceEphemeralMessageDisabled:
			case EventLog::Type::ConferenceEphemeralMessageManagedByAdmin:
			case EventLog::Type::ConferenceEphemeralMessageManagedByParticipants:
			case EventLog::Type::ConferenceAllowedParticipantListChanged:
				return false;
		}

		tr.commit();

		return true;
	};
#else
	return false;
#endif
}

bool MainDb::deleteEvent(const shared_ptr<const EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	const EventLogPrivate *dEventLog = eventLog->getPrivate();
	if (!dEventLog->dbKey.isValid()) {
		lWarning() << "Unable to delete invalid event.";
		return false;
	}

	MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
	shared_ptr<Core> core = dEventKey->core.lock();
	L_ASSERT(core);

	MainDb &mainDb = *core->getPrivate()->mainDb.get();

	return L_DB_TRANSACTION_C(&mainDb) {
		MainDbPrivate *const d = mainDb.getPrivate();
		soci::session *session = d->dbSession.getBackendSession();
		*session << "DELETE FROM event WHERE id = :id", soci::use(dEventKey->storageId);

		if (eventLog->getType() == EventLog::Type::ConferenceChatMessage) {
			shared_ptr<ChatMessage> chatMessage(
			    static_pointer_cast<const ConferenceChatMessageEvent>(eventLog)->getChatMessage());
			shared_ptr<AbstractChatRoom> chatRoom(chatMessage->getChatRoom());
			const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
			*session << "UPDATE chat_room SET last_message_id = IFNULL((SELECT id FROM conference_event_simple_view "
			            "WHERE chat_room_id = chat_room.id AND type = "
			         << mapEventFilterToSql(ConferenceChatMessageFilter)
			         << " ORDER BY id DESC LIMIT 1), 0) WHERE id = :1",
			    soci::use(dbChatRoomId);
			// Delete chat message from cache as the event is deleted
			ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
			dChatMessage->resetStorageId();
		}

		tr.commit();

		// Reset storage ID as event is not valid anymore
		const_cast<EventLogPrivate *>(dEventLog)->resetStorageId();

		if (eventLog->getType() == EventLog::Type::ConferenceChatMessage) {
			shared_ptr<ChatMessage> chatMessage(
			    static_pointer_cast<const ConferenceChatMessageEvent>(eventLog)->getChatMessage());
			if (chatMessage->getDirection() == ChatMessage::Direction::Incoming &&
			    !chatMessage->getPrivate()->isMarkedAsRead()) {
				int *count = d->unreadChatMessageCountCache[chatMessage->getChatRoom()->getConferenceId()];
				if (count) --*count;
			}
		}

		return true;
	};
#else
	return false;
#endif
}

int MainDb::getEventCount(FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	const string query =
	    "SELECT COUNT(*) FROM event" + buildSqlEventFilter({ConferenceCallFilter, ConferenceChatMessageFilter,
	                                                        ConferenceInfoFilter, ConferenceInfoNoDeviceFilter},
	                                                       mask);

	// DurationLogger durationLogger("Get event count with mask=" + Utils::toString(mask) + ".");

	return L_DB_TRANSACTION {
		L_D();

		int count;
		*d->dbSession.getBackendSession() << query, soci::into(count);
		return count;
	};
#else
	return 0;
#endif
}

shared_ptr<EventLog> MainDb::getEvent(const unique_ptr<MainDb> &mainDb, const long long &storageId) {
#ifdef HAVE_DB_STORAGE
	if ((storageId < 0) || (mainDb == nullptr)) {
		if ((storageId < 0)) {
			lDebug() << "Unable to get event from invalid storage ID " << storageId;
		}
		return nullptr;
	}

	MainDbPrivate *d = mainDb->getPrivate();

	shared_ptr<EventLog> event = d->getEventFromCache(storageId);
	if (event) return event;

	return L_DB_TRANSACTION_C(mainDb.get()) {
		// TODO: Improve. Deal with all events in the future.
		soci::row row;
		*d->dbSession.getBackendSession() << Statements::get(Statements::SelectConferenceEvent), soci::into(row),
		    soci::use(storageId);

		ConferenceId conferenceId(Address(row.get<string>(16)), Address(row.get<string>(17)),
		                          mainDb->getCore()->createConferenceIdParams());
		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom) return shared_ptr<EventLog>();

		return d->selectGenericConferenceEvent(chatRoom, row);
	};
#else
	return nullptr;
#endif
}

shared_ptr<EventLog> MainDb::getEventFromKey(const MainDbKey &dbKey) {
#ifdef HAVE_DB_STORAGE
	if (!dbKey.isValid()) {
		lWarning() << "Unable to get event from invalid key.";
		return nullptr;
	}

	unique_ptr<MainDb> &q = dbKey.getPrivate()->core.lock()->getPrivate()->mainDb;
	const long long &eventId = dbKey.getPrivate()->storageId;
	return MainDb::getEvent(q, eventId);
#else
	return nullptr;
#endif
}

list<shared_ptr<EventLog>> MainDb::getConferenceNotifiedEvents(const ConferenceId &conferenceId,
                                                               unsigned int lastNotifyId) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	const string query = Statements::get(Statements::SelectConferenceEvents) + string(" AND notify_id > :lastNotifyId");

	/*
	DurationLogger durationLogger(
	    "Get conference notified events of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() +
	    ", lastNotifyId=" + Utils::toString(lastNotifyId) + ")."
	);
	*/

	return L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);

		list<shared_ptr<EventLog>> events;
		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(dbChatRoomId), soci::use(lastNotifyId));
		for (const auto &row : rows)
			events.push_back(d->selectConferenceInfoEvent(conferenceId, row));
		return events;
	};
#else
	return list<shared_ptr<EventLog>>();
#endif
}

int MainDb::getChatMessageCount(const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	/*
	DurationLogger durationLogger(
	    "Get chat messages count of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ")."
	);
	*/

	return L_DB_TRANSACTION {
		L_D();

		int count;

		soci::session *session = d->dbSession.getBackendSession();

		string query = "SELECT COUNT(*) FROM conference_chat_message_event";
		if (!conferenceId.isValid()) *session << query, soci::into(count);
		else {
			query += " WHERE event_id IN ("
			         "  SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId"
			         ")";

			const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
			*session << query, soci::use(dbChatRoomId), soci::into(count);
		}

		return count;
	};
#else
	return 0;
#endif
}

int MainDb::getUnreadChatMessageCount(const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	L_D();

	if (conferenceId.isValid()) {
		const int *count = d->unreadChatMessageCountCache[conferenceId];
		if (count) return *count;
	}

	string query = "SELECT COUNT(*) FROM conference_chat_message_event WHERE";
	if (conferenceId.isValid())
		query += " event_id IN ("
		         "  SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId"
		         ") AND";

	query += " marked_as_read = 0 ";

	/*
	DurationLogger durationLogger(
	    "Get unread chat messages count of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ")."
	);
	*/

	return L_DB_TRANSACTION {
		int count = 0;

		soci::session *session = d->dbSession.getBackendSession();

		if (!conferenceId.isValid()) *session << query, soci::into(count);
		else {
			const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
			*session << query, soci::use(dbChatRoomId), soci::into(count);
		}

		d->unreadChatMessageCountCache.insert(conferenceId, count);
		return count;
	};
#else
	return 0;
#endif
}

void MainDb::markChatMessagesAsRead(const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	if (getUnreadChatMessageCount(conferenceId) == 0) return;

	static const string query = "UPDATE conference_chat_message_event"
	                            "  SET marked_as_read = 1"
	                            "  WHERE marked_as_read = 0"
	                            "  AND event_id IN ("
	                            "    SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId"
	                            "  )";

	/*
	DurationLogger durationLogger(
	    "Mark chat messages as read of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ")."
	);
	*/

	L_DB_TRANSACTION {
		L_D();

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		*d->dbSession.getBackendSession() << query, soci::use(dbChatRoomId);

		tr.commit();
		d->unreadChatMessageCountCache.insert(conferenceId, 0);
	};
#endif
}

void MainDb::updateChatRoomEphemeralEnabled(const ConferenceId &conferenceId, bool ephemeralEnabled) const {
#ifdef HAVE_DB_STORAGE
	static const string query = "UPDATE chat_room"
	                            "  SET ephemeral_enabled = :ephemeralEnabled"
	                            " WHERE id = :chatRoomId";

	int isEphemeralEnabled = ephemeralEnabled ? 1 : 0;

	L_DB_TRANSACTION {
		L_D();
		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		*d->dbSession.getBackendSession() << query, soci::use(isEphemeralEnabled), soci::use(dbChatRoomId);
		tr.commit();
	};
#endif
}

void MainDb::updateChatRoomEphemeralLifetime(const ConferenceId &conferenceId, long time) const {
#ifdef HAVE_DB_STORAGE
	static const string query = "UPDATE chat_room"
	                            "  SET ephemeral_messages_lifetime = :ephemeralLifetime"
	                            " WHERE id = :chatRoomId";

	L_DB_TRANSACTION {
		L_D();
		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		*d->dbSession.getBackendSession() << query, soci::use(time), soci::use(dbChatRoomId);

		tr.commit();
	};
#endif
}

void MainDb::updateEphemeralMessageInfos(const long long &eventId, const time_t &eTime) const {
#ifdef HAVE_DB_STORAGE
	static const string query = "UPDATE chat_message_ephemeral_event"
	                            "  SET expired_time = :expireTime"
	                            "  WHERE event_id = :eventId";

	L_DB_TRANSACTION {
		L_D();
		auto expireTime = d->dbSession.getTimeWithSociIndicator(eTime);
		*d->dbSession.getBackendSession() << query, soci::use(expireTime.first), soci::use(eventId);
		tr.commit();
	};
#endif
}

list<shared_ptr<ChatMessage>> MainDb::getUnreadChatMessages(const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	static const string query = Statements::get(Statements::SelectConferenceEvents) + string(" AND marked_as_read = 0");

	DurationLogger durationLogger(
	    "Get unread chat messages: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ").");

	return L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();

		long long dbChatRoomId = d->selectChatRoomId(conferenceId);
		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		list<shared_ptr<ChatMessage>> chatMessages;
		if (!chatRoom) return chatMessages;

		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(dbChatRoomId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);

			if (event) chatMessages.push_back(static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
		}

		return chatMessages;
	};
#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<shared_ptr<ChatMessage>> MainDb::getEphemeralMessages() const {
#ifdef HAVE_DB_STORAGE
	// Keep chat_room_id at the end of the query !!!
	string query =
	    "SELECT conference_event_view.id, type, creation_time, from_sip_address.value, to_sip_address.value, time, "
	    "imdn_message_id, state, direction, is_secured, notify_id, device_sip_address.value, "
	    "participant_sip_address.value, subject, delivery_notification_required, display_notification_required, "
	    "security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, "
	    "reply_message_id, reply_sender_address.value, message_id, chat_room_id"
	    " FROM conference_event_view"
	    " LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
	    " LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
	    " LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
	    " LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = "
	    "participant_sip_address_id"
	    " LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
	    " WHERE conference_event_view.id in ("
	    " SELECT event_id"
	    " FROM chat_message_ephemeral_event"
	    " WHERE expired_time > :nullTime"
	    " ORDER BY expired_time ASC";
	query += getBackend() == MainDb::Backend::Sqlite3 ? " LIMIT :maxMessages) ORDER BY expired_time ASC"
	                                                  : " ) ORDER BY expired_time ASC";

	return L_DB_TRANSACTION {
		L_D();
		list<shared_ptr<ChatMessage>> chatMessages;
		auto epoch = d->dbSession.getTimeWithSociIndicator(0);
		soci::rowset<soci::row> rows =
		    getBackend() == MainDb::Backend::Sqlite3
		        ? (d->dbSession.getBackendSession()->prepare << query, soci::use(epoch.first),
		           soci::use(EPHEMERAL_MESSAGE_TASKS_MAX_NB))
		        : (d->dbSession.getBackendSession()->prepare << query, soci::use(epoch.first));
		for (const auto &row : rows) {
			const long long &dbChatRoomId = d->dbSession.resolveId(row, (int)row.size() - 1);
			ConferenceId conferenceId = d->getConferenceIdFromCache(dbChatRoomId);
			if (!conferenceId.isValid()) {
				conferenceId = d->selectConferenceId(dbChatRoomId);
			}

			if (conferenceId.isValid()) {
				shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
				if (chatRoom) {
					shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
					if (event) {
						L_ASSERT(event->getType() == EventLog::Type::ConferenceChatMessage);
						chatMessages.push_back(
						    static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
					}
				}
			}
		}
		return chatMessages;
	};
#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<MainDb::ParticipantState> MainDb::getChatMessageParticipantsByImdnState(const shared_ptr<EventLog> &eventLog,
                                                                             ChatMessage::State state) const {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		const EventLogPrivate *dEventLog = eventLog->getPrivate();
		MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
		const long long &eventId = dEventKey->storageId;
		int stateInt = int(state);

		static const string query = "SELECT sip_address.value, chat_message_participant.state_change_time"
		                            " FROM sip_address, chat_message_participant"
		                            " WHERE event_id = :eventId AND state = :state"
		                            " AND sip_address.id = chat_message_participant.participant_sip_address_id";
		soci::rowset<soci::row> rows =
		    (d->dbSession.getBackendSession()->prepare << query, soci::use(eventId), soci::use(stateInt));

		list<MainDb::ParticipantState> result;
		for (const auto &row : rows)
			result.emplace_back(Address::create(row.get<string>(0)), state, d->dbSession.getTime(row, 1));
		return result;
	};
#else
	return list<MainDb::ParticipantState>();
#endif
}

list<MainDb::ParticipantState> MainDb::getChatMessageParticipantStates(const shared_ptr<EventLog> &eventLog) const {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		const EventLogPrivate *dEventLog = eventLog->getPrivate();
		MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
		const long long &eventId = dEventKey->storageId;

		static const string query =
		    "SELECT sip_address.value, chat_message_participant.state, chat_message_participant.state_change_time"
		    " FROM sip_address, chat_message_participant"
		    " WHERE event_id = :eventId"
		    " AND sip_address.id = chat_message_participant.participant_sip_address_id";
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(eventId));

		list<MainDb::ParticipantState> states;
		for (const auto &row : rows) {
			states.emplace_back(Address::create(row.get<string>(0)), ChatMessage::State(row.get<int>(1)),
			                    d->dbSession.getTime(row, 2));
		}
		return states;
	};
#else
	return list<MainDb::ParticipantState>();
#endif
}

ChatMessage::State MainDb::getChatMessageParticipantState(const shared_ptr<EventLog> &eventLog,
                                                          const std::shared_ptr<Address> &participantAddress) const {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		const EventLogPrivate *dEventLog = eventLog->getPrivate();
		MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
		const long long &eventId = dEventKey->storageId;
		const long long &participantSipAddressId = d->selectSipAddressId(participantAddress, true);

		unsigned int state = (unsigned int)ChatMessage::State::Idle;
		*d->dbSession.getBackendSession()
		    << "SELECT state FROM chat_message_participant"
		       " WHERE event_id = :eventId AND participant_sip_address_id = :participantSipAddressId",
		    soci::into(state), soci::use(eventId), soci::use(participantSipAddressId);

		return ChatMessage::State(state);
	};
#else
	return ChatMessage::State::Idle;
#endif
}

void MainDb::setChatMessageParticipantState(const shared_ptr<EventLog> &eventLog,
                                            const std::shared_ptr<Address> &participantAddress,
                                            ChatMessage::State state,
                                            time_t stateChangeTime) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();
		d->setChatMessageParticipantState(eventLog, participantAddress, state, stateChangeTime);
		tr.commit();
	};
#endif
}

list<shared_ptr<Content>> MainDb::getMediaContents(const ConferenceId &conferenceId) const {
	list<shared_ptr<Content>> result = list<shared_ptr<Content>>();
#ifdef HAVE_DB_STORAGE
	static const string query =
	    "SELECT name, path, size, content_type.value, conference_chat_message_event.time, "
	    "conference_chat_message_event.imdn_message_id "
	    " FROM chat_message_file_content "
	    " JOIN chat_message_content ON chat_message_content.id = chat_message_file_content.chat_message_content_id "
	    " JOIN content_type ON content_type.id = chat_message_content.content_type_id "
	    " JOIN conference_chat_message_event ON conference_chat_message_event.event_id = "
	    "chat_message_content.event_id "
	    " JOIN conference_event ON conference_event.event_id = chat_message_content.event_id AND "
	    " conference_event.chat_room_id = :chatRoomId "
	    " WHERE content_type.value LIKE 'video/%' OR content_type.value LIKE 'image/%' OR content_type.value LIKE "
	    "'audio/%' "
	    " ORDER BY chat_message_content.event_id DESC";
	return L_DB_TRANSACTION {
		L_D();
		const long long &chatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(chatRoomId));
		for (const auto &row : rows) {
			string name = row.get<string>(0);
			string path = row.get<string>(1);
			int size = row.get<int>(2);
			ContentType contentType(row.get<string>(3));
			time_t creation = d->dbSession.getTime(row, 4);
			string messageId = row.get<string>(5);
			lDebug() << "Fetched media content [" << name << "] message id is [" << messageId << "]";

			auto fileContent = FileContent::create<FileContent>();
			fileContent->setFileName(name);
			fileContent->setFileSize(size_t(size));
			fileContent->setFilePath(path);
			fileContent->setContentType(contentType);
			fileContent->setCreationTimestamp(creation);
			fileContent->setRelatedChatMessageId(messageId);

			result.push_back(fileContent);
		}
		return result;
	};
#else
	return result;
#endif
}

list<shared_ptr<Content>> MainDb::getDocumentContents(const ConferenceId &conferenceId) const {
	list<shared_ptr<Content>> result = list<shared_ptr<Content>>();
#ifdef HAVE_DB_STORAGE
	static const string query =
	    "SELECT name, path, size, content_type.value, conference_chat_message_event.time, "
	    "conference_chat_message_event.imdn_message_id "
	    " FROM chat_message_file_content "
	    " JOIN chat_message_content ON chat_message_content.id = chat_message_file_content.chat_message_content_id "
	    " JOIN content_type ON content_type.id = chat_message_content.content_type_id "
	    " JOIN conference_chat_message_event ON conference_chat_message_event.event_id = "
	    "chat_message_content.event_id "
	    " JOIN conference_event ON conference_event.event_id = chat_message_content.event_id AND "
	    " conference_event.chat_room_id = :chatRoomId "
	    " WHERE content_type.value LIKE 'text/%' OR content_type.value LIKE 'application/%' "
	    " ORDER BY chat_message_content.event_id DESC";
	return L_DB_TRANSACTION {
		L_D();
		const long long &chatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(chatRoomId));
		for (const auto &row : rows) {
			string name = row.get<string>(0);
			string path = row.get<string>(1);
			int size = row.get<int>(2);
			ContentType contentType(row.get<string>(3));
			time_t creation = d->dbSession.getTime(row, 4);
			string messageId = row.get<string>(5);
			lDebug() << "Fetched document content [" << name << "] message id is [" << messageId << "]";

			auto fileContent = FileContent::create<FileContent>();
			fileContent->setFileName(name);
			fileContent->setFileSize(size_t(size));
			fileContent->setFilePath(path);
			fileContent->setContentType(contentType);
			fileContent->setCreationTimestamp(creation);
			fileContent->setRelatedChatMessageId(messageId);

			result.push_back(fileContent);
		}
		return result;
	};
#else
	return result;
#endif
}

bool MainDb::isChatRoomEmpty(const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	static const string query = "SELECT last_message_id FROM chat_room WHERE id = :1";

	return L_DB_TRANSACTION {
		L_D();

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId));

		for (const auto &row : rows) {
			return d->dbSession.resolveId(row, 0) == 0;
		}

		return true;
	};
#endif
	return true;
}

shared_ptr<ChatMessage> MainDb::getLastChatMessage(const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	static const string query =
	    "SELECT conference_event_view.id AS event_id, type, conference_event_view.creation_time, "
	    "from_sip_address.value, to_sip_address.value, time, imdn_message_id, state, direction, is_secured, "
	    "notify_id, "
	    "device_sip_address.value, participant_sip_address.value, conference_event_view.subject, "
	    "delivery_notification_required, display_notification_required, peer_sip_address.value, "
	    "local_sip_address.value, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, "
	    "reply_message_id, reply_sender_address.value, message_id"
	    " FROM conference_event_view"
	    " JOIN chat_room ON chat_room.id = chat_room_id"
	    " JOIN sip_address AS peer_sip_address ON peer_sip_address.id = peer_sip_address_id"
	    " JOIN sip_address AS local_sip_address ON local_sip_address.id = local_sip_address_id"
	    " LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
	    " LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
	    " LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
	    " LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = "
	    "participant_sip_address_id"
	    " LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
	    " WHERE event_id = (SELECT last_message_id FROM chat_room WHERE id = :1)";

	return L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();
		shared_ptr<ChatMessage> chatMessage = nullptr;

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom) return chatMessage;

		long long dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(dbChatRoomId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
			if (event) return static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage();
		}

		return chatMessage;
	};
#else
	return nullptr;
#endif
}

shared_ptr<EventLog> MainDb::findEventLog(const ConferenceId &conferenceId, const string &imdnMessageId) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	static const string query =
	    Statements::get(Statements::SelectConferenceEvents) + string(" AND imdn_message_id = :imdnMessageId");

	/*
	DurationLogger durationLogger(
	    "Find event log: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ")."
	);
	*/
	return L_DB_TRANSACTION->shared_ptr<EventLog> {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom) return nullptr;

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows =
		    (d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId), soci::use(imdnMessageId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
			if (event) {
				return event;
			}
		}
		return nullptr;
	};
#endif
	return nullptr;
}

list<shared_ptr<ChatMessage>> MainDb::findChatMessages(const ConferenceId &conferenceId,
                                                       const string &imdnMessageId) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	static const string query =
	    Statements::get(Statements::SelectConferenceEvents) + string(" AND imdn_message_id = :imdnMessageId");

	/*
	DurationLogger durationLogger(
	    "Find chat messages: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ")."
	);
	*/
	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		list<shared_ptr<ChatMessage>> chatMessages;
		if (!chatRoom) return chatMessages;

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows =
		    (d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId), soci::use(imdnMessageId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
			if (event) {
				L_ASSERT(event->getType() == EventLog::Type::ConferenceChatMessage);
				chatMessages.push_back(static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
			}
		}

		return chatMessages;
	};
#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<shared_ptr<ChatMessage>> MainDb::findChatMessages(const ConferenceId &conferenceId,
                                                       const list<string> &imdnMessageIds) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	static const string query =
	    Statements::get(Statements::SelectConferenceEvents) + string(" AND ( imdn_message_id = ");

	/*
	DurationLogger durationLogger(
	    "Find chat messages: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ")."
	);
	*/
	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		list<shared_ptr<ChatMessage>> chatMessages;
		if (!chatRoom) return chatMessages;

		ostringstream ostr;
		ostr << query;
		size_t index = 0;
		size_t listSize = imdnMessageIds.size();
		for (const auto &id : imdnMessageIds) {
			ostr << "'" << id << "'";
			if (index < listSize - 1) {
				ostr << " OR imdn_message_id = ";
			} else {
				ostr << " ) ";
			}
			index += 1;
		}
		string computedQuery = ostr.str();

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows =
		    (d->dbSession.getBackendSession()->prepare << computedQuery, soci::use(dbChatRoomId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
			if (event) {
				L_ASSERT(event->getType() == EventLog::Type::ConferenceChatMessage);
				chatMessages.push_back(static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
			}
		}

		return chatMessages;
	};
#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<shared_ptr<ChatMessage>> MainDb::findChatMessagesFromMessageId(const std::string &messageId) const {
#ifdef HAVE_DB_STORAGE
	// Keep chat_room_id at the end of the query !!!
	static const string query =
	    "SELECT conference_event_view.id AS event_id, type, creation_time, from_sip_address.value, "
	    "to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, "
	    "device_sip_address.value, participant_sip_address.value, subject, delivery_notification_required, "
	    "display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, "
	    "ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value, message_id, "
	    "chat_room_id"
	    " FROM conference_event_view"
	    " LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
	    " LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
	    " LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
	    " LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = participant_sip_address_id"
	    " LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
	    " WHERE message_id = :messageId";

	return L_DB_TRANSACTION {
		L_D();

		list<shared_ptr<ChatMessage>> chatMessages;
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(messageId));

		for (const auto &row : rows) {
			const long long &dbChatRoomId = d->dbSession.resolveId(row, (int)row.size() - 1);
			ConferenceId conferenceId = d->getConferenceIdFromCache(dbChatRoomId);
			if (!conferenceId.isValid()) {
				conferenceId = d->selectConferenceId(dbChatRoomId);
			}

			if (conferenceId.isValid()) {
				shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
				if (chatRoom) {
					shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
					if (event) {
						L_ASSERT(event->getType() == EventLog::Type::ConferenceChatMessage);
						chatMessages.push_back(
						    static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
					}
				}
			}
		}

		return chatMessages;
	};

#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<shared_ptr<ChatMessage>>
MainDb::findChatMessagesFromImdnMessageId(const std::list<std::string> &imdnMessageIds) const {
#ifdef HAVE_DB_STORAGE
	// Keep chat_room_id at the end of the query !!!
	static const string query =
	    "SELECT conference_event_view.id AS event_id, type, creation_time, from_sip_address.value, "
	    "to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, "
	    "device_sip_address.value, participant_sip_address.value, subject, delivery_notification_required, "
	    "display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, "
	    "ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value, message_id, "
	    "chat_room_id"
	    " FROM conference_event_view"
	    " LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
	    " LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
	    " LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
	    " LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = participant_sip_address_id"
	    " LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
	    " WHERE (imdn_message_id = ";

	return L_DB_TRANSACTION {
		L_D();

		ostringstream ostr;
		ostr << query;
		size_t index = 0;
		size_t listSize = imdnMessageIds.size();
		for (const auto &id : imdnMessageIds) {
			ostr << "'" << id << "'";
			if (index < listSize - 1) {
				ostr << " OR imdn_message_id = ";
			} else {
				ostr << " ) ";
			}
			index += 1;
		}
		string computedQuery = ostr.str();
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << computedQuery);

		list<shared_ptr<ChatMessage>> chatMessages;
		for (const auto &row : rows) {
			const long long &dbChatRoomId = d->dbSession.resolveId(row, (int)row.size() - 1);
			ConferenceId conferenceId = d->getConferenceIdFromCache(dbChatRoomId);
			if (!conferenceId.isValid()) {
				conferenceId = d->selectConferenceId(dbChatRoomId);
			}

			if (conferenceId.isValid()) {
				shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
				if (chatRoom) {
					shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
					if (event) {
						L_ASSERT(event->getType() == EventLog::Type::ConferenceChatMessage);
						chatMessages.push_back(
						    static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
					}
				}
			}
		}

		return chatMessages;
	};

#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<shared_ptr<ChatMessage>> MainDb::findChatMessagesFromCallId(const std::string &callId) const {
#ifdef HAVE_DB_STORAGE
	// Keep chat_room_id at the end of the query !!!
	static const string query =
	    "SELECT conference_event_view.id AS event_id, type, creation_time, from_sip_address.value, "
	    "to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, "
	    "device_sip_address.value, participant_sip_address.value, subject, delivery_notification_required, "
	    "display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, "
	    "ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value, message_id, "
	    "chat_room_id"
	    " FROM conference_event_view"
	    " LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
	    " LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
	    " LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
	    " LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = "
	    "participant_sip_address_id"
	    " LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
	    " WHERE call_id = :callId";

	return L_DB_TRANSACTION {
		L_D();

		list<shared_ptr<ChatMessage>> chatMessages;
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(callId));

		for (const auto &row : rows) {
			const long long &dbChatRoomId = d->dbSession.resolveId(row, (int)row.size() - 1);
			ConferenceId conferenceId = d->getConferenceIdFromCache(dbChatRoomId);
			if (!conferenceId.isValid()) {
				conferenceId = d->selectConferenceId(dbChatRoomId);
			}

			if (conferenceId.isValid()) {
				shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
				if (chatRoom) {
					shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
					if (event) {
						L_ASSERT(event->getType() == EventLog::Type::ConferenceChatMessage);
						chatMessages.push_back(
						    static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
					}
				}
			}
		}

		return chatMessages;
	};

#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<shared_ptr<ChatMessage>> MainDb::findChatMessagesToBeNotifiedAsDelivered() const {
#ifdef HAVE_DB_STORAGE
	// Keep chat_room_id at the end of the query !!!
	static const string query =
	    "SELECT conference_event_view.id AS event_id, type, creation_time, from_sip_address.value, "
	    "to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, "
	    "device_sip_address.value, participant_sip_address.value, subject, delivery_notification_required, "
	    "display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, "
	    "ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value, message_id, "
	    "chat_room_id"
	    " FROM conference_event_view"
	    " LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
	    " LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
	    " LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
	    " LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = "
	    "participant_sip_address_id"
	    " LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
	    " WHERE conference_event_view.id IN (SELECT event_id FROM conference_chat_message_event WHERE "
	    "delivery_notification_required <> 0 AND direction = :direction)";

	/*
	DurationLogger durationLogger(
	    "Find chat messages to be notified as delivered: (peer=" +
	conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ")."
	);
	*/

	return L_DB_TRANSACTION {
		L_D();

		list<shared_ptr<ChatMessage>> chatMessages;
		const int &direction = int(ChatMessage::Direction::Incoming);
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(direction));

		for (const auto &row : rows) {
			// chat_room_id is the last element of row
			const long long &dbChatRoomId = d->dbSession.resolveId(row, (int)row.size() - 1);
			ConferenceId conferenceId = d->getConferenceIdFromCache(dbChatRoomId);
			if (!conferenceId.isValid()) {
				conferenceId = d->selectConferenceId(dbChatRoomId);
			}

			if (conferenceId.isValid()) {
				shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
				if (chatRoom) {
					shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
					if (event) {
						L_ASSERT(event->getType() == EventLog::Type::ConferenceChatMessage);
						chatMessages.push_back(
						    static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
					}
				}
			}
		}

		return chatMessages;
	};
#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<shared_ptr<EventLog>> MainDb::getHistory(const ConferenceId &conferenceId, int nLast, FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	return getHistoryRange(conferenceId, 0, nLast, mask);
#else
	return list<shared_ptr<EventLog>>();
#endif
}

list<shared_ptr<EventLog>>
MainDb::getHistoryRange(const ConferenceId &conferenceId, int begin, int end, FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	L_D();

	if (begin < 0) begin = 0;

	list<shared_ptr<EventLog>> events;
	if (end > 0 && begin > end) {
		lWarning() << "Unable to get history. Invalid range.";
		return events;
	}

	string query = Statements::get(Statements::SelectConferenceEvents) +
	               buildSqlEventFilter({ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter,
	                                    ConferenceInfoNoDeviceFilter, ConferenceChatMessageSecurityFilter},
	                                   mask, "AND");
	query += " ORDER BY event_id DESC";

	if (end > 0) query += " LIMIT " + Utils::toString(end - begin);
	else query += " LIMIT " + d->dbSession.noLimitValue();

	if (begin > 0) query += " OFFSET " + Utils::toString(begin);

	/*
	DurationLogger durationLogger(
	    "Get history range of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() +
	    ", begin=" + Utils::toString(begin) + ", end=" + Utils::toString(end) + ")."
	);
	*/

	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom) return events;

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
			if (event) events.push_front(event);
		}

		return events;
	};
#else
	return list<shared_ptr<EventLog>>();
#endif
}

list<shared_ptr<EventLog>> MainDb::getHistoryRangeNear(const ConferenceId &conferenceId,
                                                       unsigned int before,
                                                       unsigned int after,
                                                       const shared_ptr<EventLog> &event,
                                                       FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	list<shared_ptr<EventLog>> events;

	if (before == 0 && after == 0) {
		return events;
	}

	// Build the base query
	const string baseQuery = Statements::get(Statements::SelectConferenceEvents);
	const string baseFilter =
	    buildSqlEventFilter({ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter,
	                         ConferenceInfoNoDeviceFilter, ConferenceChatMessageSecurityFilter},
	                        mask, "AND");

	string query;
	if (event == nullptr) {
		query = baseQuery + baseFilter +
		        " ORDER BY event_id DESC"
		        " LIMIT :2";
	} else {
		// Apply filtering for events before the one we want
		string beforeQuery = baseQuery + baseFilter +
		                     " AND event_id <= :2"
		                     " ORDER BY event_id DESC"
		                     " LIMIT :3";

		// Same but for after
		string afterQuery = baseQuery + baseFilter +
		                    " AND event_id >= :2"
		                    " ORDER BY event_id ASC"
		                    " LIMIT :4";

		// Make an union
		query = "SELECT * FROM (" + beforeQuery +
		        ") "
		        "UNION "
		        "SELECT * FROM (" +
		        afterQuery +
		        ") "
		        "ORDER BY event_id DESC";
	}

	// DurationLogger durationLogger(
	//     "Get history range near of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	//     ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ", before=" +
	//     Utils::toString(before)
	//     +
	//     ", after=" + Utils::toString(after) + ", event=" + Utils::toString(event) + ").");

	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom) return events;

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);

		if (event == nullptr) {
			soci::rowset<soci::row> rows =
			    (d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId), soci::use(before));

			for (const auto &row : rows) {
				shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
				if (event) events.push_front(event);
			}
		} else {
			const EventLogPrivate *dEventLog = event->getPrivate();
			MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
			const long long &dbEventId = dEventKey->storageId;

			soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId),
			                                soci::use(dbEventId), soci::use(before + 1), soci::use(after + 1));

			for (const auto &row : rows) {
				shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
				if (event) events.push_front(event);
			}
		}

		return events;
	};
#else
	return list<shared_ptr<EventLog>>();
#endif
}

list<shared_ptr<EventLog>> MainDb::getHistoryRangeBetween(const ConferenceId &conferenceId,
                                                          const shared_ptr<EventLog> &firstEvent,
                                                          const shared_ptr<EventLog> &lastEvent,
                                                          FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	list<shared_ptr<EventLog>> events;

	if (firstEvent == nullptr || lastEvent == nullptr) {
		return events;
	}

	// Build the base query
	const string baseQuery = Statements::get(Statements::SelectConferenceEvents);
	const string baseFilter =
	    buildSqlEventFilter({ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter,
	                         ConferenceInfoNoDeviceFilter, ConferenceChatMessageSecurityFilter},
	                        mask, "AND");

	string query = "SELECT * FROM (" + baseQuery + baseFilter +
	               " AND event_id > :2 AND event_id < :3) "
	               "ORDER BY event_id DESC";

	// DurationLogger durationLogger(
	//     "Get history range between of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	//     ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ", after=" +
	//     Utils::toString(lastEvent)
	//     +
	//     ", before=" + Utils::toString(firstEvent) + ").");

	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom) return events;

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);

		const EventLogPrivate *dFirstEventLog = firstEvent->getPrivate();
		MainDbKeyPrivate *dFirstEventKey = static_cast<MainDbKey &>(dFirstEventLog->dbKey).getPrivate();
		const long long &dbFirstEventId = dFirstEventKey->storageId;

		const EventLogPrivate *dLastEventLog = lastEvent->getPrivate();
		MainDbKeyPrivate *dLastEventKey = static_cast<MainDbKey &>(dLastEventLog->dbKey).getPrivate();
		const long long &dbLastEventId = dLastEventKey->storageId;

		long long lowerId = dbFirstEventId;
		long long higherId = dbLastEventId;
		if (lowerId > higherId) {
			lowerId = dbLastEventId;
			higherId = dbFirstEventId;
		}

		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId),
		                                soci::use(lowerId), soci::use(higherId));

		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
			if (event) events.push_front(event);
		}

		return events;
	};
#else
	return list<shared_ptr<EventLog>>();
#endif
}

int MainDb::getHistorySize(const ConferenceId &conferenceId, FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	const string query = "SELECT COUNT(*) FROM event, conference_event"
	                     "  WHERE chat_room_id = :chatRoomId"
	                     "  AND event_id = event.id" +
	                     buildSqlEventFilter({ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter,
	                                          ConferenceInfoNoDeviceFilter, ConferenceChatMessageSecurityFilter},
	                                         mask, "AND");

	return L_DB_TRANSACTION {
		L_D();

		int count;
		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		*d->dbSession.getBackendSession() << query, soci::into(count), soci::use(dbChatRoomId);

		return count;
	};
#else
	return 0;
#endif
}

void MainDb::cleanHistory(const ConferenceId &conferenceId, FilterMask mask) {
#ifdef HAVE_DB_STORAGE
	const string query = "SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId" +
	                     buildSqlEventFilter({ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter,
	                                          ConferenceInfoNoDeviceFilter},
	                                         mask);

	const string query2 = "UPDATE chat_room SET last_message_id = 0 WHERE id = :1";

	/*
	DurationLogger durationLogger(
	    "Clean history of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	    ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() +
	    ", mask=" + Utils::toString(mask) + ")."
	);
	*/

	L_DB_TRANSACTION {
		L_D();

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);

		d->invalidConferenceEventsFromQuery(query, dbChatRoomId);
		*d->dbSession.getBackendSession() << "DELETE FROM event WHERE id IN (" + query + ")", soci::use(dbChatRoomId);
		*d->dbSession.getBackendSession() << query2, soci::use(dbChatRoomId);
		tr.commit();

		if (!mask || (mask & ConferenceChatMessageFilter)) d->unreadChatMessageCountCache.insert(conferenceId, 0);
	};
#endif
}

// -----------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
template <typename T>
static void fetchContentAppData(soci::session *session, Content &content, long long contentId, T &data) {
	static const string query = "SELECT name, data FROM chat_message_content_app_data"
	                            " WHERE chat_message_content_id = :contentId";

	string name;
	soci::statement statement = (session->prepare << query, soci::use(contentId), soci::into(name), soci::into(data));
	statement.execute();
	while (statement.fetch())
		content.setProperty(name, Variant{blobToString(data)});
}
#endif

void MainDb::loadChatMessageContents(const shared_ptr<ChatMessage> &chatMessage) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();

		bool hasFileTransferContent = false;

		ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
		const long long &eventId = chatMessage->getStorageId();

		static const string query =
		    "SELECT chat_message_content.id, content_type.id, content_type.value, body, body_encoding_type"
		    " FROM chat_message_content, content_type"
		    " WHERE event_id = :eventId AND content_type_id = content_type.id";
		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(eventId));
		for (const auto &row : rows) {
			ContentType contentType(row.get<string>(2));
			const long long &contentId = d->dbSession.resolveId(row, 0);
			shared_ptr<Content> content;
			int bodyEncodingType = row.get<int>(4);

			if (contentType == ContentType::FileTransfer) {
				hasFileTransferContent = true;
				content = FileTransferContent::create<FileTransferContent>();
			} else {
				// 1.1 - Fetch contents' file information if they exist
				string name;
				int size;
				string path;
				int duration;

				*session << "SELECT name, size, path, duration FROM chat_message_file_content"
				            " WHERE chat_message_content_id = :contentId",
				    soci::into(name), soci::into(size), soci::into(path), soci::into(duration), soci::use(contentId);
				if (session->got_data()) {
					auto fileContent = FileContent::create<FileContent>();
					fileContent->setFileName(name);
					fileContent->setFileSize(size_t(size));
					fileContent->setFilePath(path);
					fileContent->setFileDuration(duration);
					fileContent->setCreationTimestamp(chatMessage->getTime());
					content = fileContent;
				} else {
					content = Content::create();
				}
			}

			content->setContentType(contentType);
			if (bodyEncodingType == 1) content->setBodyFromUtf8(row.get<string>(3));
			else content->setBodyFromLocale(row.get<string>(3));

			content->setRelatedChatMessageId(chatMessage->getImdnMessageId());

			// 1.2 - Fetch contents' app data.
			// TODO: Do not test backend, encapsulate!!!
			if (getBackend() == MainDb::Backend::Sqlite3) {
				soci::blob data(*session);
				fetchContentAppData(session, *content, contentId, data);
			} else {
				string data;
				fetchContentAppData(session, *content, contentId, data);
			}
			chatMessage->addContent(content);
		}

		// 2 - Load external body url from body into FileTransferContent if needed.
		if (hasFileTransferContent) dChatMessage->loadFileTransferUrlFromBodyToContent();
	};
#endif
}

list<shared_ptr<ChatMessageReaction>> MainDb::getChatMessageReactions(const shared_ptr<ChatMessage> &chatMessage) {
	list<shared_ptr<ChatMessageReaction>> reactions;
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();
		const string &messageId = chatMessage->getImdnMessageId();

		static const string query =
		    "SELECT body, from_sip_address.value, call_id"
		    " FROM conference_chat_message_reaction_event"
		    " LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
		    " WHERE reaction_to_message_id = :messageId"
		    " ORDER BY body ASC";
		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(messageId));
		for (const auto &row : rows) {
			string body = row.get<string>(0);
			if (body.empty()) {
				lDebug() << "Found empty reaction for message [" << chatMessage << "], skipping";
				continue;
			}
			shared_ptr<Address> fromAddress = make_shared<Address>(row.get<string>(1));
			string callId = row.get<string>(2);
			shared_ptr<ChatMessageReaction> reaction =
			    ChatMessageReaction::create(messageId, body, fromAddress, callId);
			reactions.push_back(reaction);
		}
	};
#endif
	return reactions;
}

void MainDb::removeConferenceChatMessageReactionEvent(const string &messageId,
                                                      const std::shared_ptr<const Address> &fromAddress) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		// Same as insertion, we have to remove the gruu
		std::shared_ptr<Address> from = fromAddress->clone()->toSharedPtr();
		from->clean();
		const long long &fromSipAddressId = d->selectSipAddressId(from, true);

		*d->dbSession.getBackendSession()
		    << "DELETE FROM conference_chat_message_reaction_event WHERE"
		       " from_sip_address_id = :from_sip_address_id AND reaction_to_message_id = :messageId",
		    soci::use(fromSipAddressId), soci::use(messageId);
		tr.commit();
	};
#endif
}

// -----------------------------------------------------------------------------

void MainDb::disableDeliveryNotificationRequired(const std::shared_ptr<const EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ChatMessage> chatMessage(
	    static_pointer_cast<const ConferenceChatMessageEvent>(eventLog)->getChatMessage());
	const long long &eventId = static_cast<MainDbKey &>(eventLog->getPrivate()->dbKey).getPrivate()->storageId;

	L_DB_TRANSACTION {
		L_D();
		*d->dbSession.getBackendSession()
		    << "UPDATE conference_chat_message_event SET delivery_notification_required = 0"
		       " WHERE event_id = :eventId",
		    soci::use(eventId);
		tr.commit();
	};
#endif
}

void MainDb::disableDisplayNotificationRequired(const std::shared_ptr<const EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ChatMessage> chatMessage(
	    static_pointer_cast<const ConferenceChatMessageEvent>(eventLog)->getChatMessage());
	const long long &eventId = static_cast<MainDbKey &>(eventLog->getPrivate()->dbKey).getPrivate()->storageId;

	L_DB_TRANSACTION {
		L_D();
		*d->dbSession.getBackendSession()
		    << "UPDATE conference_chat_message_event"
		       " SET delivery_notification_required = 0, display_notification_required = 0"
		       " WHERE event_id = :eventId",
		    soci::use(eventId);
		tr.commit();
	};
#endif
}

// -----------------------------------------------------------------------------

// Add a chatroom to the list passed as first argument if it is not a duplicate.
// In case a chatroom with the same conference id (where the comparison doesn't take into account the gr parameters)
// is already found, then a merge is executed:
// - keep the chatroom with the oldest creation time
// - set the creation time to the earliest one
// - assign all events preceeding the latest creation time to the kept chatroom
// - destroy the deleted chatroom from DB
// This function returns TRUE if a chatroom has replaced an existing one or added otherwise FALSE
bool MainDb::addChatroomToList(ChatRoomWeakCompareMap &chatRoomsMap,
                               const shared_ptr<AbstractChatRoom> &chatRoom,
                               long long id,
                               int unreadMessageCount) const {
#ifdef HAVE_DB_STORAGE
	L_D();
	const auto &chatRoomConferenceId = chatRoom->getConferenceId();
	shared_ptr<AbstractChatRoom> chatRoomToAdd = nullptr;
	shared_ptr<AbstractChatRoom> chatRoomToRemove = nullptr;
	bool ret = false;
	try {
		auto storedContext = chatRoomsMap.at(chatRoomConferenceId);
		const auto &storedChatRoom = storedContext.sChatRoom;
		chatRoomToAdd = mergeChatRooms(chatRoom, storedChatRoom, id, storedContext.sDbId, unreadMessageCount);
		if (storedChatRoom == chatRoomToAdd) {
			chatRoomToRemove = chatRoom;
			ret = false;
		} else {
			chatRoomToRemove = storedChatRoom;
			d->cache(chatRoomConferenceId, id);
			ret = true;
		}
	} catch (std::out_of_range &) {
		chatRoomToAdd = chatRoom;
		d->unreadChatMessageCountCache.insert(chatRoomConferenceId, unreadMessageCount);
		d->cache(chatRoomConferenceId, id);
		ret = true;
	}
	if (chatRoomToRemove && chatRoomToRemove->getConference()) {
		chatRoomToRemove->getConference()->terminate();
	}
	return ret;
#endif
	return false;
}

shared_ptr<AbstractChatRoom> MainDb::mergeChatRooms(const shared_ptr<AbstractChatRoom> &chatRoom1,
                                                    const shared_ptr<AbstractChatRoom> &chatRoom2,
                                                    long long id1,
                                                    long long id2,
                                                    int unreadMessageCount) const {
#ifdef HAVE_DB_STORAGE
	L_D();
	shared_ptr<AbstractChatRoom> chatRoomToAdd = nullptr;
	size_t chatRoom1LastNotify = 0;
	const auto &chatRoom1Backend = chatRoom1->getCurrentParams()->getChatParams()->getBackend();
	auto chatRoom1IsFlexisipChat = (chatRoom1Backend == ChatParams::Backend::FlexisipChat);
	if (chatRoom1IsFlexisipChat) {
		chatRoom1LastNotify = chatRoom1->getConference()->getLastNotify();
	}
	const auto chatRoom1ConferenceId = chatRoom1->getConferenceId();
	const auto chatRoom1CreationTime = chatRoom1->getCreationTime();

	size_t chatRoom2LastNotify = 0;
	const auto &chatRoom2Backend = chatRoom2->getCurrentParams()->getChatParams()->getBackend();
	auto chatRoom2IsFlexisipChat = (chatRoom2Backend == ChatParams::Backend::FlexisipChat);
	if (chatRoom2IsFlexisipChat) {
		chatRoom2LastNotify = chatRoom2->getConference()->getLastNotify();
	}
	const auto chatRoom2ConferenceId = chatRoom2->getConferenceId();
	const auto chatRoom2CreationTime = chatRoom2->getCreationTime();
	lInfo() << "Chat rooms with conference id " << chatRoom1ConferenceId << " and " << chatRoom2ConferenceId
	        << " will be merged as they have the same peer address";
	time_t creationTimeToAdd = 0;
	time_t creationTimeToDelete = 0;

	long long dbChatRoomToAddId = 0;
	long long dbChatRoomToRemoveId = 0;
	// The boolean updateChatRoomTable is used to ensure that only the most up-to-date informations stay in the
	// chat_room table. In fact during a merge it may happen that the same chat room lays in multiple lines of table
	// chat_room. Here below a snapshot of all entries for the same chat room in a database:
	// ==========================================
	// | ID |    Creation Time    | Last Notify |
	// ==========================================
	// | 1  | 2023-11-08 08:10:39 |     5       |
	// | 20 | 2023-11-08 08:20:39 |     5       |
	// | 24 | 2023-11-08 08:21:39 |     22      |
	// | 27 | 2023-11-08 08:40:39 |     25      |
	// ==========================================
	// Obviously the most recent chat room is the one with ID 27 and therefore we should keep its line but change
	// the creation time Unfortunately the chat rooms are not returned ordered by ID therefore it is necessary to
	// take into account the lastNotify column if the chat room is conference based The first merge operation is the
	// merge of chat room with ID 24 into the one with ID 27. The former chat room is deleted from the DB and the
	// creation time of the latter chat room is updated to 2023-11-08 08:21:39 Then a second merge operation occurs
	// between ID 1 and ID 24. The former chat room is deleted and the latter's creation time is updated to
	// 2023-11-08 08:10:39 Ultimately the last merge operation occurs between ID 27 and ID 20. This time around, we
	// cannot look to the creation time anymore as if we would so, we would end up with the wrong Last Notify and
	// subject at least. We should therefore delete chat room with ID 20 and move all its conference events to chat
	// room with ID 27 Update chatroom with the largest creation time and update its creation time with the lowest
	// value.
	if ((chatRoom2LastNotify < chatRoom1LastNotify) ||
	    ((chatRoom2LastNotify == chatRoom1LastNotify) && (chatRoom2CreationTime < chatRoom1CreationTime))) {
		chatRoomToAdd = chatRoom1;
		creationTimeToAdd = chatRoom1CreationTime;
		creationTimeToDelete = chatRoom2CreationTime;
		dbChatRoomToAddId = id1;
		dbChatRoomToRemoveId = id2;
	} else {
		chatRoomToAdd = chatRoom2;
		creationTimeToAdd = chatRoom1CreationTime;
		creationTimeToDelete = chatRoom2CreationTime;
		dbChatRoomToAddId = id2;
		dbChatRoomToRemoveId = id1;
	}

	time_t creationTime = 0;
	if (chatRoom2CreationTime < chatRoom1CreationTime) {
		creationTime = chatRoom2CreationTime;
	} else {
		creationTime = chatRoom1CreationTime;
	}

	chatRoomToAdd->setCreationTime(creationTime);
	const auto unreadChatMessageCountChatRoom2 = d->unreadChatMessageCountCache[chatRoom2ConferenceId];
	int unreadChatMessageCount = unreadMessageCount;
	if (unreadChatMessageCountChatRoom2) {
		unreadChatMessageCount += *unreadChatMessageCountChatRoom2;
	}
	d->unreadChatMessageCountCache.insert(chatRoom2ConferenceId, unreadChatMessageCount);

	auto creationTimeToAddSoci = d->dbSession.getTimeWithSociIndicator(creationTimeToAdd);
	soci::session *session = d->dbSession.getBackendSession();
	lInfo() << "Moving all event of chatroom with ID " << dbChatRoomToRemoveId << " to chatroom with ID "
	        << dbChatRoomToAddId;
	// Move conference event that occurred before the latest chatroom was created.
	// Events such as chat messages are already stored in both chat rooms
	auto creationTimeToDeleteSoci = d->dbSession.getTimeWithSociIndicator(creationTimeToDelete);
	std::pair<tm, soci::indicator> oldestCreationTimeSoci;
	std::pair<tm, soci::indicator> newestCreationTimeSoci;
	if (creationTimeToAdd < creationTimeToDelete) {
		oldestCreationTimeSoci = creationTimeToAddSoci;
		newestCreationTimeSoci = creationTimeToDeleteSoci;
	} else {
		oldestCreationTimeSoci = creationTimeToDeleteSoci;
		newestCreationTimeSoci = creationTimeToAddSoci;
	}

	soci::rowset<soci::row> rows =
	    (session->prepare
	         << "SELECT conference_event.event_id, conference_event.chat_room_id FROM conference_event, event "
	            "WHERE "
	            "event.id = conference_event.event_id AND conference_event.chat_room_id = :chatRoomId AND "
	            "event.creation_time > :creationTimeMin AND event.creation_time < :creationTimeMax",
	     soci::use(dbChatRoomToRemoveId), soci::use(oldestCreationTimeSoci.first, oldestCreationTimeSoci.second),
	     soci::use(newestCreationTimeSoci.first, newestCreationTimeSoci.second));

	// Update row one by one to avoid disk IO Errors from SQL
	for (const auto &row : rows) {
		auto eventid = d->dbSession.getUnsignedInt(row, 0, 0);
		*session << "UPDATE conference_event SET chat_room_id = :newChatRoomid WHERE event_id = :eventId",
		    soci::use(dbChatRoomToAddId), soci::use(eventid);
	}

	if (dbChatRoomToRemoveId != -1) {
		lInfo() << "Deleting chatroom with ID " << dbChatRoomToRemoveId;
		*session << "DELETE FROM chat_room WHERE id = :chatRoomId", soci::use(dbChatRoomToRemoveId);
	}
	return chatRoomToAdd;
#else
	return nullptr;
#endif
}

// TODO: the const attribute has been removed because of a compile error when calling selectConferenceInfo:
// src/db/main-db.cpp:6073:102: error: passing ‘const LinphonePrivate::MainDbPrivate’ as ‘this’ argument discards
// qualifiers [-fpermissive] 6073 |                                         shared_ptr<ConferenceInfo> confInfo =
// d->selectConferenceInfo(row);

list<shared_ptr<AbstractChatRoom>> MainDb::getChatRooms() {
#ifdef HAVE_DB_STORAGE
	static const string query =
	    "SELECT chat_room.id, peer_sip_address.value, local_sip_address.value,"
	    " creation_time, last_update_time, capabilities, subject, last_notify_id, flags, last_message_id,"
	    " ephemeral_enabled, ephemeral_messages_lifetime,"
	    " unread_messages_count.message_count, muted, conference_info_id"
	    " FROM chat_room"
	    " LEFT JOIN (SELECT conference_event.chat_room_id, count(*) as message_count"
	    " FROM conference_chat_message_event, conference_event"
	    " WHERE conference_chat_message_event.event_id=conference_event.event_id AND "
	    "conference_chat_message_event.marked_as_read = 0"
	    " GROUP BY conference_event.chat_room_id) AS unread_messages_count"
	    " ON unread_messages_count.chat_room_id = chat_room.id"
	    " , sip_address AS peer_sip_address, sip_address AS local_sip_address"
	    " WHERE chat_room.peer_sip_address_id = peer_sip_address.id AND chat_room.local_sip_address_id = "
	    "local_sip_address.id"
	    " ORDER BY last_update_time DESC";

	DurationLogger durationLogger("Get chat rooms.");

	return L_DB_TRANSACTION {
		L_D();

		ChatRoomWeakCompareMap chatRoomsMap;

		shared_ptr<Core> core = getCore();
		LinphoneCore *cCore = getCore()->getCCore();
		bool serverMode = linphone_core_conference_server_enabled(core->getCCore());

		soci::session *session = d->dbSession.getBackendSession();

		soci::rowset<soci::row> rows = (session->prepare << query);
		// SOCI uses a hack for sqlite3:
		// "sqlite3 type system does not have a date or time field.  Also it does not reliably id other data types.
		// It has a tendency to see everything as text.
		// sqlite3_column_decltype returns the text that is used in the create table statement"
		// The data type on the count can be a string (as of SOCI 4.0.0) but as it is a "hack", we have to work with
		// both types (integer and string)
		soci::data_type unreadMessageCountType;
		bool typeHasBeenSet = false;
		d->unreadChatMessageCountCache.clear();

		auto conferenceIdParams = core->createConferenceIdParams();
		conferenceIdParams.enableExtractUri(false);
		bool keepGruu = conferenceIdParams.getKeepGruu();

		bool unifyChatroomAddress =
		    !!linphone_config_get_bool(linphone_core_get_config(cCore), "misc", "unify_chatroom_address", FALSE);
		std::string chatroomDomain;
		std::string chatroomGr;
		if (unifyChatroomAddress) {
			// No need to read the configuration if the core is not configure to unify chatroom addresses
			chatroomDomain = L_C_TO_STRING(
			    linphone_config_get_string(linphone_core_get_config(cCore), "misc", "force_chatroom_domain", ""));
			if (keepGruu) {
				chatroomGr = L_C_TO_STRING(
				    linphone_config_get_string(linphone_core_get_config(cCore), "misc", "force_chatroom_gr", ""));
			}
		}

		for (const auto &row : rows) {
			if (!typeHasBeenSet) {
				unreadMessageCountType = row.get_properties(12).get_data_type();
				typeHasBeenSet = true;
			}

			Address pAddress(row.get<string>(1), true);
			Address oldPAddress(pAddress);
			Address lAddress(row.get<string>(2), true);
			Address oldLAddress(lAddress);
			bool conferenceIdChanged = (!keepGruu && (pAddress.hasUriParam("gr") || lAddress.hasUriParam("gr")));
			if (!chatroomDomain.empty()) {
				if (chatroomDomain.compare(pAddress.getDomain()) != 0) {
					pAddress.setDomain(chatroomDomain);
					conferenceIdChanged = true;
				}
				if (keepGruu) {
					auto pAddressGr = pAddress.getUriParamValue("gr");
					if (!chatroomGr.empty() && (pAddressGr.empty() || chatroomGr.compare(pAddressGr) != 0)) {
						pAddress.setUriParam("gr", chatroomGr);
						conferenceIdChanged = true;
					}
				}
			}
			ConferenceId conferenceId(std::move(pAddress), std::move(lAddress), conferenceIdParams);

			const long long &dbChatRoomId = d->dbSession.resolveId(row, 0);
			shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId, false);
			if (chatRoom) {
				ChatRoomContext context(chatRoom, dbChatRoomId, false, false);
				chatRoomsMap.insert(std::make_pair(conferenceId, context));
				continue;
			}

			bool updateFlags = false;
			time_t creationTime = d->dbSession.getTime(row, 3);
			time_t lastUpdateTime = d->dbSession.getTime(row, 4);
			int capabilities = row.get<int>(5);
			string subject = row.get<string>(6, "");
			const long long &lastMessageId = d->dbSession.resolveId(row, 9);
			bool muted = !!row.get<int>(13);

			shared_ptr<ConferenceParams> params = ConferenceParams::fromCapabilities(capabilities, core);
			const auto backend = params->getChatParams()->getBackend();
			if (backend == ChatParams::Backend::Basic) {
				chatRoom = core->getPrivate()->createBasicChatRoom(conferenceId, params);
				chatRoom->setUtf8Subject(subject);
			} else if (backend == ChatParams::Backend::FlexisipChat) {
#ifdef HAVE_ADVANCED_IM
				const auto &localAddress = conferenceId.getLocalAddress();
				unsigned int lastNotifyId = d->dbSession.getUnsignedInt(row, 7, 0);
				list<shared_ptr<Participant>> participants = selectChatRoomParticipants(dbChatRoomId);
				const auto meIt =
				    std::find_if(participants.begin(), participants.end(), [&localAddress](const auto &participant) {
					    return (participant->getAddress()->weakEqual(*localAddress));
				    });
				shared_ptr<Participant> me;
				if (meIt != participants.end()) {
					me = *meIt;
					participants.erase(meIt);
				}

				params->setUtf8Subject(subject);
				params->getChatParams()->setEphemeralLifetime((long)row.get<double>(11));
				params->getChatParams()->enableEphemeral(!!row.get<int>(10, 0));
				const auto &conferenceAddress = conferenceId.getPeerAddress();
				params->setConferenceAddress(conferenceAddress);

				const long long &conferenceInfoId = d->dbSession.resolveId(row, 14);
				shared_ptr<ConferenceInfo> confInfo;
				if (conferenceInfoId > 0) {
					soci::row conferenceInfoRow;
					*session << Statements::get(Statements::SelectConferenceInfoFromId), soci::into(conferenceInfoRow),
					    soci::use(conferenceInfoId);
					confInfo = d->selectConferenceInfo(conferenceInfoRow);
				}

				if (confInfo) {
					params->enableAudio(confInfo->getCapability(LinphoneStreamTypeAudio));
					params->enableVideo(confInfo->getCapability(LinphoneStreamTypeVideo));
					params->setDescription(confInfo->getDescription());
					const auto startTime = confInfo->getDateTime();
					params->setStartTime(startTime);
					const auto duration = confInfo->getDuration();
					if ((startTime >= 0) && (duration > 0)) {
						// The duration of the conference is stored in minutes in the conference information
						params->setEndTime(startTime + duration * 60);
					}
					params->setEarlierJoiningTime(confInfo->getEarlierJoiningTime());
					params->setExpiryTime(confInfo->getExpiryTime());
				}

				std::shared_ptr<Conference> conference = nullptr;
				if (serverMode) {
					params->enableLocalParticipant(false);
					conference = (new ServerConference(core, nullptr, params))->toSharedPtr();
					conference->initFromDb(nullptr, conferenceId, lastNotifyId, false);
					chatRoom = conference->getChatRoom();
					conference->setState(ConferenceInterface::State::Created);
					if (me) {
						lInfo() << "Deleting me participant " << *me->getAddress()
						        << " from the list of participants of " << *conference
						        << " as a server chat room is not expected to have it.";
						deleteChatRoomParticipant(chatRoom, me->getAddress());
					}
				} else {
					if (!me) {
						lError() << "Unable to find me (" << *localAddress << ") in: " << conferenceId;
						continue;
					}
					bool hasBeenLeft = !!row.get<int>(8, 0);
					conference = (new ClientConference(core, nullptr, params))->toSharedPtr();
					conference->initFromDb(me, conferenceId, lastNotifyId, hasBeenLeft);
					chatRoom = conference->getChatRoom();
					updateFlags = confInfo && !hasBeenLeft;
					if (hasBeenLeft) {
						conference->setState(ConferenceInterface::State::Terminated);
					} else {
						conference->setState(ConferenceInterface::State::Created);
					}
					if (!params->isGroup()) {
						auto conferenceIdParams = core->createConferenceIdParams();
						conferenceIdParams.enableExtractUri(false);
						// TODO: load previous IDs if any
						static const string query =
						    "SELECT sip_address.value FROM one_to_one_chat_room_previous_conference_id, sip_address"
						    " WHERE chat_room_id = :chatRoomId"
						    " AND sip_address_id = sip_address.id";
						soci::rowset<soci::row> rows = (session->prepare << query, soci::use(dbChatRoomId));
						for (const auto &row : rows) {
							ConferenceId previousId = ConferenceId(Address::create(row.get<string>(0), true),
							                                       localAddress, conferenceIdParams);
							if (previousId != conferenceId) {
								lInfo() << "Keeping around previous chat room ID [" << previousId
								        << "] in case BYE is received for exhumed chat room " << *conference << " ["
								        << conferenceId << "]";
								auto clientChatRoom = dynamic_pointer_cast<ClientChatRoom>(chatRoom);
								clientChatRoom->addConferenceIdToPreviousList(previousId);
							}
						}
					}
				}
				for (auto participant : participants) {
					participant->setConference(conference);
					if (confInfo) {
						const auto confInfoParticipant = confInfo->findParticipant(participant->getAddress());
						if (confInfoParticipant) {
							participant->setRole(confInfoParticipant->getRole());
							participant->setSequenceNumber(confInfoParticipant->getSequenceNumber());
						}
					}
				}
				if (!conference->supportsMedia()) {
					conference->setParticipants(std::move(participants));
				}

				std::list<std::shared_ptr<Participant>> invitedParticipants;
				if (confInfo) {
					for (const auto &participantInfo : confInfo->getParticipants()) {
						auto participant = Participant::create(participantInfo->getAddress());
						participant->setRole(participantInfo->getRole());
						participant->setSequenceNumber(participantInfo->getSequenceNumber());
						invitedParticipants.push_back(participant);
					}
				} else {
					invitedParticipants = conference->getParticipants();
				}
				conference->setInvitedParticipants(invitedParticipants);

				if (confInfo) {
					conference->setOrganizer(confInfo->getOrganizerAddress());
				}
#else
				lWarning() << "Advanced IM such as group chat is disabled!";
#endif
			} else {
				lError() << "Unable to retrieve chat room from database because its type is neither Basic nor "
				            "Conference";
			}

			if (!chatRoom) continue; // Not fetched.

			chatRoom->setCreationTime(creationTime);
			chatRoom->setLastUpdateTime(lastUpdateTime);
			chatRoom->setIsEmpty(lastMessageId == 0);
			chatRoom->setIsMuted(muted, false);

			int unreadMessagesCount = 0;
			if (unreadMessageCountType == soci::dt_string) unreadMessagesCount = std::stoi(row.get<string>(12, "0"));
			else unreadMessagesCount = row.get<int>(12, 0);

			lDebug() << "Found chat room in DB: " << conferenceId;

			if (addChatroomToList(chatRoomsMap, chatRoom, dbChatRoomId, unreadMessagesCount)) {
				ChatRoomContext context(chatRoom, dbChatRoomId, conferenceIdChanged, updateFlags);
				chatRoomsMap.insert_or_assign(conferenceId, context);
			}
		}

		for (const auto &[conferenceId, context] : chatRoomsMap) {
			const auto &chatRoom = context.sChatRoom;
			const auto &dbId = context.sDbId;
			const auto &conferenceIdChanged = context.sConferenceIdChanged;
			const auto &updateFlags = context.sUpdateFlags;
			if (updateFlags || conferenceIdChanged) {
				std::string query("UPDATE chat_room SET ");
				if (conferenceIdChanged) {
					// If the conference ID changed, then update the information in the database so that the next time
					// around everything will be alright
					const auto &conferenceId = chatRoom->getConferenceId();
					const auto &peerAddress = conferenceId.getPeerAddress();
					const auto &localAddress = conferenceId.getLocalAddress();
					lInfo() << "Change peer and local address of chatroom [" << chatRoom << "] with ID " << dbId << ":";
					lInfo() << "- peer: " << *peerAddress;
					lInfo() << "- local: " << *localAddress;
					// lInfo() << "- peer: " << oldPAddress << " -> " << *peerAddress;
					// lInfo() << "- local: " << oldLAddress << " -> " << *localAddress;
					const long long &peerSipAddressId = d->insertSipAddress(peerAddress);
					const long long &localSipAddressId = d->insertSipAddress(localAddress);
					query += "peer_sip_address_id = " + Utils::toString(peerSipAddressId) +
					         ", local_sip_address_id = " + Utils::toString(localSipAddressId) + " ";
				}
				if (updateFlags) {
					// If we end up here, it means that there was a problem with a media conference supporting chat
					// capabilities. The core might have lost the connection while the conference was ongoing therefore
					// the database could not be updated to reflect the termination of the conference.
					lInfo() << "Updating flags of chatroom [" << chatRoom << "] with ID " << dbId
					        << ": setting it as terminated and reset the last notify ID to 0";
					query += "flags = 1, last_notify_id = 0 ";
					auto chatConference = chatRoom->getConference();
					if (chatConference) {
						chatConference->resetLastNotify();
						chatConference->setState(ConferenceInterface::State::Terminated);
					}
				}
				query += "WHERE id = :chatRoomId";
				*session << query, soci::use(dbId);
			}
		}

		tr.commit();

		std::list<std::shared_ptr<AbstractChatRoom>> chatRooms;
		for (const auto &[conferenceId, context] : chatRoomsMap) {
			const auto &chatRoom = context.sChatRoom;
			chatRooms.push_back(chatRoom);
			const auto &conference = chatRoom->getConference();
			if (conference) {
				core->insertConference(conference);
			}
		}
		return chatRooms;
	};
#else
	return list<shared_ptr<AbstractChatRoom>>();
#endif
}

void MainDbPrivate::insertNewPreviousConferenceId(const ConferenceId &currentConfId,
                                                  const ConferenceId &previousConfId) {
#ifdef HAVE_DB_STORAGE
	const long long &previousConferenceSipAddressId = selectSipAddressId(previousConfId.getPeerAddress(), true);
	const long long &chatRoomId = selectChatRoomId(currentConfId);

	*dbSession.getBackendSession() << "INSERT INTO one_to_one_chat_room_previous_conference_id ("
	                                  "  chat_room_id, sip_address_id"
	                                  ") VALUES ("
	                                  "  :chatRoomId, :previousConferenceSipAddressId"
	                                  ")",
	    soci::use(chatRoomId), soci::use(previousConferenceSipAddressId);
#endif
}

void MainDb::insertNewPreviousConferenceId(const ConferenceId &currentConfId, const ConferenceId &previousConfId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		lInfo() << "Inserting previous conf ID [" << previousConfId << "] in database for [" << currentConfId << "]";
		d->insertNewPreviousConferenceId(currentConfId, previousConfId);
		tr.commit();
	};
#endif
}

void MainDbPrivate::removePreviousConferenceId(const ConferenceId &previousConfId) {
#ifdef HAVE_DB_STORAGE
	const long long &previousConferenceSipAddressId = selectSipAddressId(previousConfId.getPeerAddress(), true);

	*dbSession.getBackendSession() << "DELETE FROM one_to_one_chat_room_previous_conference_id WHERE "
	                                  "sip_address_id = :previousConferenceSipAddressId",
	    soci::use(previousConferenceSipAddressId);
#endif
}

void MainDb::removePreviousConferenceId(const ConferenceId &previousConfId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		lInfo() << "Removing previous conf ID [" << previousConfId << "] from database";
		d->removePreviousConferenceId(previousConfId);
		tr.commit();
	};
#endif
}

void MainDb::insertChatRoom(const shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();
		d->insertChatRoom(chatRoom, notifyId);
		tr.commit();
	};
#endif
}

void MainDb::deleteChatRoom(const ConferenceId &conferenceId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();
		d->deleteChatRoom(conferenceId);
		tr.commit();
	};
#endif
}

void MainDb::updateChatRoomConferenceId(const ConferenceId oldConferenceId, const ConferenceId &newConferenceId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		const long long &peerSipAddressId = d->insertSipAddress(newConferenceId.getPeerAddress());
		const long long &localSipAddressId = d->insertSipAddress(newConferenceId.getLocalAddress());
		const long long &dbChatRoomId = d->selectChatRoomId(oldConferenceId);

		*d->dbSession.getBackendSession() << "UPDATE chat_room"
		                                     " SET peer_sip_address_id = :peerSipAddressId,"
		                                     " local_sip_address_id = :localSipAddressId"
		                                     " WHERE id = :chatRoomId",
		    soci::use(peerSipAddressId), soci::use(localSipAddressId), soci::use(dbChatRoomId);

		tr.commit();

		d->cache(newConferenceId, dbChatRoomId);
	};
#endif
}

void MainDb::updateChatRoomLastUpdatedTime(const ConferenceId &conferenceId, time_t lastUpdatedTime) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();
		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		auto lastUpdateTimeTm = d->dbSession.getTimeWithSociIndicator(lastUpdatedTime);

		*d->dbSession.getBackendSession() << "UPDATE chat_room SET last_update_time = :lastUpdateTime"
		                                     " WHERE id = :chatRoomId",
		    soci::use(lastUpdateTimeTm.first, lastUpdateTimeTm.second), soci::use(dbChatRoomId);

		tr.commit();
	};
#endif
}

void MainDb::updateChatRoomMutedState(const ConferenceId &conferenceId, bool isMuted) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();
		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		int muted = isMuted ? 1 : 0;

		*d->dbSession.getBackendSession() << "UPDATE chat_room SET muted = :muted"
		                                     " WHERE id = :chatRoomId",
		    soci::use(muted), soci::use(dbChatRoomId);

		tr.commit();
	};
#endif
}

long long MainDb::addConferenceParticipantEventToDb(const shared_ptr<EventLog> &eventLog, long long *chatRoomId) {

	L_D();
	return d->insertConferenceParticipantEvent(eventLog, chatRoomId, false);
}

void MainDb::updateNotifyId(const shared_ptr<AbstractChatRoom> &chatRoom, const unsigned int lastNotify) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		if (dbChatRoomId >= 0) {
			*d->dbSession.getBackendSession() << "UPDATE chat_room"
			                                     " SET last_notify_id = :lastNotifyId "
			                                     " WHERE id = :chatRoomId",
			    soci::use(lastNotify), soci::use(dbChatRoomId);
			tr.commit();
		}
	};
#endif
}

std::shared_ptr<Address> MainDb::findMissingOneToOneConferenceChatRoomParticipantAddress(
    const shared_ptr<AbstractChatRoom> &chatRoom, const std::shared_ptr<Address> &presentParticipantAddr) {
#ifdef HAVE_DB_STORAGE
	L_ASSERT(linphone_core_conference_server_enabled(chatRoom->getCore()->getCCore()));
	L_ASSERT(chatRoom->getCapabilities() & ChatRoom::Capabilities::OneToOne);
	L_ASSERT(chatRoom->getConference()->getParticipantCount() == 1);

	return L_DB_TRANSACTION {
		L_D();

		string missingParticipantAddress;
		string participantASipAddress;
		string participantBSipAddress;

		const long long &chatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		L_ASSERT(chatRoomId != -1);

		*d->dbSession.getBackendSession() << "SELECT participant_a_sip_address.value, participant_b_sip_address.value"
		                                     " FROM one_to_one_chat_room, sip_address AS participant_a_sip_address, "
		                                     "sip_address AS participant_b_sip_address"
		                                     " WHERE chat_room_id = :chatRoomId"
		                                     " AND participant_a_sip_address_id = participant_a_sip_address.id"
		                                     " AND participant_b_sip_address_id = participant_b_sip_address.id",
		    soci::into(participantASipAddress), soci::into(participantBSipAddress), soci::use(chatRoomId);

		if (*presentParticipantAddr == Address(participantASipAddress))
			missingParticipantAddress = participantBSipAddress;
		else if (*presentParticipantAddr == Address(participantBSipAddress))
			missingParticipantAddress = participantASipAddress;

		auto missingParticipant = Address::create(missingParticipantAddress);
		return missingParticipant;
	};
#else
	return nullptr;
#endif
}

std::shared_ptr<Address> MainDb::findOneToOneConferenceChatRoomAddress(const std::shared_ptr<Address> &participantA,
                                                                       const std::shared_ptr<Address> &participantB,
                                                                       bool encrypted) const {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		std::shared_ptr<Address> address = nullptr;
		const long long &participantASipAddressId = d->selectSipAddressId(participantA, true);
		const long long &participantBSipAddressId = d->selectSipAddressId(participantB, true);
		if (participantASipAddressId == -1 || participantBSipAddressId == -1) return address;

		const long long &chatRoomId =
		    d->selectOneToOneChatRoomId(participantASipAddressId, participantBSipAddressId, encrypted);
		if (chatRoomId == -1) return address;

		string chatRoomAddress;
		*d->dbSession.getBackendSession()
		    << "SELECT sip_address.value"
		       " FROM chat_room, sip_address"
		       " WHERE chat_room.id = :chatRoomId AND peer_sip_address_id = sip_address.id",
		    soci::use(chatRoomId), soci::into(chatRoomAddress);

		address = Address::create(chatRoomAddress);
		return address;
	};
#else
	return nullptr;
#endif
}

void MainDb::insertOneToOneConferenceChatRoom(const shared_ptr<AbstractChatRoom> &chatRoom, bool encrypted) {
#ifdef HAVE_DB_STORAGE
	L_ASSERT(linphone_core_conference_server_enabled(chatRoom->getCore()->getCCore()));
	L_ASSERT(chatRoom->getCapabilities() & ChatRoom::Capabilities::OneToOne);

	L_DB_TRANSACTION {
		L_D();

		const list<shared_ptr<Participant>> &participants = chatRoom->getParticipants();
		const long long &participantASipAddressId = d->selectSipAddressId(participants.front()->getAddress(), true);
		const long long &participantBSipAddressId = d->selectSipAddressId(participants.back()->getAddress(), true);
		L_ASSERT(participantASipAddressId != -1);
		L_ASSERT(participantBSipAddressId != -1);

		long long chatRoomId =
		    d->selectOneToOneChatRoomId(participantASipAddressId, participantBSipAddressId, encrypted);
		if (chatRoomId == -1) {
			chatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
			*d->dbSession.getBackendSession() << Statements::get(Statements::InsertOneToOneChatRoom, getBackend()),
			    soci::use(chatRoomId), soci::use(participantASipAddressId), soci::use(participantBSipAddressId);
		}

		tr.commit();
	};
#endif
}

void MainDb::updateChatRoomParticipantDevice(const shared_ptr<AbstractChatRoom> &chatRoom,
                                             const shared_ptr<ParticipantDevice> &device) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		L_DB_TRANSACTION {
			L_D();
			const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
			const long long &participantSipAddressId =
			    d->selectSipAddressId(device->getParticipant()->getAddress(), true);
			const long long &participantId = d->selectChatRoomParticipantId(dbChatRoomId, participantSipAddressId);
			const long long &participantDeviceSipAddressId = d->selectSipAddressId(device->getAddress(), true);
			unsigned int state = static_cast<unsigned int>(device->getState());
			auto joiningTime = d->dbSession.getTimeWithSociIndicator(device->getTimeOfJoining());
			unsigned int joiningMethod = static_cast<unsigned int>(device->getJoiningMethod());
			*d->dbSession.getBackendSession() << "UPDATE chat_room_participant_device SET state = :state, name = "
			                                     ":name, joining_time = :joiningTime, joining_method = :joiningMethod"
			                                     " WHERE chat_room_participant_id = :participantId AND "
			                                     "participant_device_sip_address_id = :participantDeviceSipAddressId",
			    soci::use(state), soci::use(device->getName()), soci::use(joiningTime.first, joiningTime.second),
			    soci::use(joiningMethod), soci::use(participantId), soci::use(participantDeviceSipAddressId);

			tr.commit();
		};
	}
#endif
}

list<shared_ptr<Participant>> MainDb::selectChatRoomParticipants(const long long chatRoomId) const {
	list<shared_ptr<Participant>> participants;
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		soci::session *session = d->dbSession.getBackendSession();
		static const string participantQuery =
		    "SELECT chat_room_participant.id, sip_address.value, is_admin FROM sip_address, chat_room, "
		    "chat_room_participant WHERE chat_room.id = :chatRoomId AND sip_address.id = "
		    "chat_room_participant.participant_sip_address_id AND chat_room_participant.chat_room_id = "
		    "chat_room.id";

		// Fetch participants.
		soci::rowset<soci::row> participantRows = (session->prepare << participantQuery, soci::use(chatRoomId));
		for (const auto &participantRow : participantRows) {
			shared_ptr<Participant> participant =
			    Participant::create(Address::create(participantRow.get<string>(1), true));
			participant->setAdmin(!!participantRow.get<int>(2));

			// Fetch devices.
			{
				const long long &participantId = d->dbSession.resolveId(participantRow, 0);
				static const string deviceQuery =
				    "SELECT sip_address.value, state, name, joining_time, joining_method FROM "
				    "chat_room_participant_device, sip_address WHERE chat_room_participant_id = :participantId AND "
				    "participant_device_sip_address_id = sip_address.id";

				soci::rowset<soci::row> deviceRows = (session->prepare << deviceQuery, soci::use(participantId));
				for (const auto &deviceRow : deviceRows) {
					shared_ptr<ParticipantDevice> device = participant->addDevice(
					    Address::create(deviceRow.get<string>(0), true), deviceRow.get<string>(2, ""));
					device->setState(ParticipantDevice::State(static_cast<unsigned int>(deviceRow.get<int>(1, 0))),
					                 false);
					device->setJoiningMethod(
					    ParticipantDevice::JoiningMethod(static_cast<unsigned int>(deviceRow.get<int>(4, 0))));
					time_t joiningTime = d->dbSession.getTime(deviceRow, 3);
					device->setTimeOfJoining(joiningTime);
				}
			}
			participants.push_back(participant);
		}
	}
#endif
	return participants;
}

list<shared_ptr<Participant>>
MainDb::selectChatRoomParticipants(const std::shared_ptr<AbstractChatRoom> &chatRoom) const {
	list<shared_ptr<Participant>> participants;
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		participants = selectChatRoomParticipants(dbChatRoomId);
	}
#endif
	return participants;
}

void MainDb::insertChatRoomParticipant(const std::shared_ptr<AbstractChatRoom> &chatRoom,
                                       const std::shared_ptr<Participant> &participant) {
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		const long long &participantSipAddressId = d->selectSipAddressId(participant->getAddress(), true);
		d->insertChatRoomParticipant(dbChatRoomId, participantSipAddressId, participant->isAdmin());
	}
#endif
}

void MainDb::deleteChatRoomParticipant(const std::shared_ptr<AbstractChatRoom> &chatRoom,
                                       const std::shared_ptr<Address> &participant) {
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		const long long &participantSipAddressId = d->selectSipAddressId(participant, true);
		d->deleteChatRoomParticipant(dbChatRoomId, participantSipAddressId);
	}
#endif
}

void MainDb::insertChatRoomParticipantDevice(const shared_ptr<AbstractChatRoom> &chatRoom,
                                             const shared_ptr<ParticipantDevice> &device) {
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		const long long &participantDeviceSipAddressId = d->selectSipAddressId(device->getAddress(), true);
		const long long &participantSipAddressId = d->selectSipAddressId(device->getParticipant()->getAddress(), true);
		const long long &participantId = d->selectChatRoomParticipantId(dbChatRoomId, participantSipAddressId);
		d->insertChatRoomParticipantDevice(participantId, participantDeviceSipAddressId, device->getName());
	}
#endif
}

void MainDb::deleteChatRoomParticipantDevice(const shared_ptr<AbstractChatRoom> &chatRoom,
                                             const shared_ptr<ParticipantDevice> &device) {
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		const long long &participantDeviceSipAddressId = d->selectSipAddressId(device->getAddress(), true);
		const long long &participantSipAddressId = d->selectSipAddressId(device->getParticipant()->getAddress(), true);
		const long long &participantId = d->selectChatRoomParticipantId(dbChatRoomId, participantSipAddressId);
		d->deleteChatRoomParticipantDevice(participantId, participantDeviceSipAddressId);
	}
#endif
}

shared_ptr<EventLog> MainDb::searchChatMessagesByText(const ConferenceId &conferenceId,
                                                      const std::string &text,
                                                      const shared_ptr<const EventLog> &from,
                                                      LinphoneSearchDirection direction) {
#ifdef HAVE_DB_STORAGE
	std::string patternToFind = Utils::replaceAll(text, "'", "''"); // replace all ' by ''
	string query =
	    "SELECT conference_event_view.id AS event_id, type, conference_event_view.creation_time, "
	    "  from_sip_address.value, to_sip_address.value, time, imdn_message_id, state, direction, is_secured, "
	    "  notify_id, device_sip_address.value, participant_sip_address.value, conference_event_view.subject, "
	    "  delivery_notification_required, display_notification_required, peer_sip_address.value, "
	    "  local_sip_address.value, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, "
	    "  reply_message_id, reply_sender_address.value, message_id "
	    "FROM conference_event_view "
	    "JOIN chat_room ON chat_room.id = chat_room_id "
	    "JOIN sip_address AS peer_sip_address ON peer_sip_address.id = peer_sip_address_id "
	    "JOIN sip_address AS local_sip_address ON local_sip_address.id = local_sip_address_id "
	    "LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id "
	    "LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id "
	    "LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id "
	    "LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = "
	    "participant_sip_address_id "
	    "LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id "
	    "LEFT JOIN chat_message_content ON chat_message_content.event_id = conference_event_view.id "
	    "WHERE chat_room_id = :chatRoomId AND chat_message_content.content_type_id = 1 "
	    "  AND chat_message_content.body LIKE '%" +
	    patternToFind + "%' ";

	if (from != nullptr) {
		const EventLogPrivate *dEventLog = from->getPrivate();
		MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
		const long long &dbEventId = dEventKey->storageId;

		query += "AND chat_message_content.event_id ";
		query += direction == LinphoneSearchDirectionUp ? "< " : "> ";
		query += Utils::toString(dbEventId);
	}

	query += " ORDER BY event_id ";
	query += direction == LinphoneSearchDirectionUp ? "DESC " : "ASC ";

	query += "LIMIT 1";

	// DurationLogger durationLogger(
	//     "Search chat message of: (peer=" + conferenceId.getPeerAddress()->toStringUriOnlyOrdered() +
	//     ", local=" + conferenceId.getLocalAddress()->toStringUriOnlyOrdered() + ", text=" + text +
	//     ", fromPosition=" + Utils::toString(from) + ", direction=" + Utils::toString(direction) + ").");

	return L_DB_TRANSACTION {
		L_D();

		const long long &chatRoomId = d->selectChatRoomId(conferenceId);
		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);

		shared_ptr<EventLog> message;
		if (!chatRoom) return message;

		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(chatRoomId));

		const auto &row = rows.begin();
		if (row != rows.end()) {
			message = d->selectGenericConferenceEvent(chatRoom, *row);
		}

		return message;
	};
#else
	return nullptr;
#endif
}

// -----------------------------------------------------------------------------

std::list<std::shared_ptr<ConferenceInfo>>
MainDb::getConferenceInfos(time_t afterThisTime, const std::list<LinphoneStreamType> capabilities) {
#ifdef HAVE_DB_STORAGE
	string query = "SELECT conference_info.id, organizer_sip_address.value, uri_sip_address.value,"
	               " start_time, duration, subject, description, state, ics_sequence, ics_uid, security_level, audio, "
	               "video, chat, ccmp_uri, earlier_joining_time, expiry_time FROM conference_info, sip_address AS "
	               "organizer_sip_address, sip_address AS "
	               "uri_sip_address WHERE conference_info.organizer_sip_address_id = organizer_sip_address.id AND "
	               "conference_info.uri_sip_address_id = uri_sip_address.id";
	if (afterThisTime > -1) query += " AND start_time >= :startTime";

	const auto capabilitiesQuery = getConferenceInfoTypeQuery(capabilities);
	if (!capabilitiesQuery.empty()) {
		query += " AND " + capabilitiesQuery;
	}

	query += " ORDER BY start_time";

	DurationLogger durationLogger("Get conference infos.");

	return L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();

		list<shared_ptr<ConferenceInfo>> conferenceInfos;
		// We cannot create an empty rowset so each "if" will make one
		if (afterThisTime > -1) {
			auto startTime = d->dbSession.getTimeWithSociIndicator(afterThisTime);
			soci::rowset<soci::row> rows = (session->prepare << query, soci::use(startTime.first, startTime.second));
			for (const auto &row : rows) {
				auto confInfo = d->selectConferenceInfo(row);
				conferenceInfos.push_back(confInfo);
			}
		} else {
			soci::rowset<soci::row> rows = (session->prepare << query);
			for (const auto &row : rows) {
				auto confInfo = d->selectConferenceInfo(row);
				conferenceInfos.push_back(confInfo);
			}
		}

		tr.commit();

		return conferenceInfos;
	};
#else
	return list<shared_ptr<ConferenceInfo>>();
#endif
}

std::string MainDb::getConferenceInfoTypeQuery(const std::list<LinphoneStreamType> &capabilities) const {
	std::string capabilitiesQuery;
	for (const auto &capability : capabilities) {
		std::string capabilityString;
		switch (capability) {
			case LinphoneStreamTypeAudio:
				capabilityString = "audio";
				break;
			case LinphoneStreamTypeVideo:
				capabilityString = "video";
				break;
			case LinphoneStreamTypeText:
				capabilityString = "chat";
				break;
			case LinphoneStreamTypeUnknown:
				break;
		}
		if (!capabilityString.empty()) {
			if (!capabilitiesQuery.empty()) {
				capabilitiesQuery += " AND ";
			}
			capabilitiesQuery += "(" + capabilityString + " = 1)";
		}
	}
	return capabilitiesQuery;
}

std::list<std::shared_ptr<ConferenceInfo>>
MainDb::getConferenceInfosWithParticipant(BCTBX_UNUSED(const std::shared_ptr<Address> &address),
                                          BCTBX_UNUSED(const std::list<LinphoneStreamType> capabilities)) {
#ifdef HAVE_DB_STORAGE
	// In order to ensure the compatibility with SQLite3 and MySQL, we have ot define two variable sipAddressId and
	// sipAddressId2 that have the same value. The SOCI backend of SQLite3 understands that it will only need one
	// argument if we define only one variable name. Nonetheless MySQL needs two. Even though the MySQL backend doesn't
	// complain when the twi variables have the sale name (e.g. sipAddressId), SQLite3 does
	string query =
	    "SELECT conference_info.id, organizer_sip_address.value, uri_sip_address.value, start_time, duration, subject, "
	    "description, state, ics_sequence, ics_uid, security_level, audio, video, chat, ccmp_uri, "
	    "earlier_joining_time, expiry_time FROM conference_info, "
	    "sip_address AS organizer_sip_address, sip_address AS uri_sip_address WHERE "
	    "conference_info.organizer_sip_address_id = organizer_sip_address.id AND conference_info.uri_sip_address_id = "
	    "uri_sip_address.id AND (conference_info.organizer_sip_address_id = :sipAddressId OR :sipAddressId2 IN (SELECT "
	    "participant_sip_address_id FROM conference_info_participant WHERE conference_info_id = conference_info.id))";

	const auto capabilitiesQuery = getConferenceInfoTypeQuery(capabilities);
	if (!capabilitiesQuery.empty()) {
		query += " AND " + capabilitiesQuery;
	}
	query += " ORDER BY start_time";

	DurationLogger durationLogger("Get conference infos for address " + address->toString());

	return L_DB_TRANSACTION {
		L_D();

		list<shared_ptr<ConferenceInfo>> conferenceInfos;

		const long long &sipAddressId = d->selectSipAddressId(address, true);

		if (sipAddressId != -1) {
			soci::session *session = d->dbSession.getBackendSession();
			soci::rowset<soci::row> rows =
			    (session->prepare << query, soci::use(sipAddressId), soci::use(sipAddressId));

			for (const auto &row : rows) {
				auto confInfo = d->selectConferenceInfo(row);
				conferenceInfos.push_back(confInfo);
			}

			tr.commit();
		} else {
			lError() << "Unable search conference informations with a participant or organizer with address "
			         << *address << " because it cannot be found in the database";
		}

		return conferenceInfos;
	};
#else
	return list<shared_ptr<ConferenceInfo>>();
#endif
}

std::shared_ptr<ConferenceInfo> MainDb::getConferenceInfo(long long conferenceInfoId) {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();
		soci::row row;
		soci::session *session = d->dbSession.getBackendSession();
		*session << Statements::get(Statements::SelectConferenceInfoFromId), soci::into(row),
		    soci::use(conferenceInfoId);
		shared_ptr<ConferenceInfo> confInfo = d->selectConferenceInfo(row);

		tr.commit();

		return confInfo;
	};
#else
	return nullptr;
#endif
}

std::shared_ptr<ConferenceInfo> MainDb::getConferenceInfoFromCcmpUri(const std::string &uri) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized() && !uri.empty()) {
		string query =
		    "SELECT conference_info.id, organizer_sip_address.value, uri_sip_address.value,"
		    " start_time, duration, subject, description, state, ics_sequence, ics_uid, security_level, audio, video, "
		    "chat, ccmp_uri, earlier_joining_time, expiry_time FROM conference_info, sip_address AS "
		    "organizer_sip_address, sip_address AS uri_sip_address "
		    "WHERE conference_info.organizer_sip_address_id = organizer_sip_address.id AND "
		    "conference_info.uri_sip_address_id = uri_sip_address.id AND conference_info.ccmp_uri = '" +
		    uri + "'";

		return L_DB_TRANSACTION {
			L_D();
			shared_ptr<ConferenceInfo> confInfo = nullptr;
			soci::session *session = d->dbSession.getBackendSession();
			soci::rowset<soci::row> rows = (session->prepare << query);
			const auto &row = rows.begin();
			if (row != rows.end()) {
				confInfo = d->selectConferenceInfo(*row);
			}

			tr.commit();

			return confInfo;
		};
	}
#endif
	return nullptr;
}

std::shared_ptr<ConferenceInfo> MainDb::getConferenceInfoFromURI(const std::shared_ptr<Address> &uri) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized() && uri) {
		string query = "SELECT conference_info.id, organizer_sip_address.value, uri_sip_address.value,"
		               " start_time, duration, subject, description, state, ics_sequence, ics_uid, security_level, "
		               "audio, video, chat, ccmp_uri, earlier_joining_time, expiry_time"
		               " FROM conference_info, sip_address AS organizer_sip_address, sip_address AS uri_sip_address"
		               " WHERE conference_info.organizer_sip_address_id = organizer_sip_address.id AND "
		               "conference_info.uri_sip_address_id = uri_sip_address.id"
		               " AND uri_sip_address.value LIKE '" +
		               uri->getUriWithoutGruu().toStringUriOnlyOrdered() + "'";

		return L_DB_TRANSACTION {
			L_D();
			shared_ptr<ConferenceInfo> confInfo = nullptr;
			soci::session *session = d->dbSession.getBackendSession();
			soci::rowset<soci::row> rows = (session->prepare << query);
			const auto &row = rows.begin();
			if (row != rows.end()) {
				confInfo = d->selectConferenceInfo(*row);
			}

			tr.commit();

			return confInfo;
		};
	}
#endif
	return nullptr;
}

long long MainDb::insertConferenceInfo(const std::shared_ptr<ConferenceInfo> &conferenceInfo) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		auto dbConfInfo = (conferenceInfo->getState() == ConferenceInfo::State::New)
		                      ? nullptr
		                      : getConferenceInfoFromURI(conferenceInfo->getUri());
		return L_DB_TRANSACTION {
			L_D();
			auto id = d->insertConferenceInfo(conferenceInfo, dbConfInfo);
			tr.commit();
			return id;
		};
	}
#endif
	return -1;
}

void MainDb::cleanupConferenceInfo(time_t expiredBeforeThisTime) {
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		if (expiredBeforeThisTime < 0) {
			lError() << "Unable to delete conference informations whose expire time is negative";
			return;
		}
		auto timestampType = d->dbSession.timestampType();
		// Compute the time difference in minutes.
		// MySQL provides a dedicated function: TIMESTAMPDIFF
		// https://dev.mysql.com/doc/refman/8.0/en/date-and-time-functions.html#function_timestampdiff In the case of an
		// Sqlite3 database, we have to compute the difference in days between two juliandays then convert it to
		// seconds by multiplying the result (that is a floating point number) by 24*60*60
		const std::string timediffExpression =
		    (getBackend() == MainDb::Backend::Sqlite3)
		        ? "(julianday(:maxExpireTime) - julianday(conference_info.expiry_time)) * 24 * 60 * 60"
		        : "TIMESTAMPDIFF(SECOND, :maxExpireTime, conference_info.expiry_time)";
		std::string findQuery = "SELECT conference_info.id, uri_sip_address.value FROM conference_info, sip_address AS "
		                        "uri_sip_address WHERE CAST(" +
		                        timediffExpression +
		                        " As Integer) >= 0 AND conference_info.uri_sip_address_id = uri_sip_address.id";
		L_DB_TRANSACTION {
			L_D();
			soci::session *session = d->dbSession.getBackendSession();
			auto expiryTime = d->dbSession.getTimeWithSociIndicator(expiredBeforeThisTime);
			soci::rowset<soci::row> rows =
			    (session->prepare << findQuery, soci::use(expiryTime.first, expiryTime.second));
			for (const auto &row : rows) {
				long long dbConferenceId = d->dbSession.resolveId(row, 0);
				const std::string uriString = row.get<string>(1);
				const auto uri = Address::create(uriString);
				const auto &conference = getCore()->searchConference(uri);
				// Delete conference only if it is not in the core's cache or it has no participants (i.e. it was
				// terminated before its end time)
				if (!conference || (conference->getParticipantDevices(false).size() == 0)) {
					lInfo() << "Deleting conference information linked to conference " << uriString
					        << " because it has no participants or it is not found in the core cache";
					deleteConferenceInfo(dbConferenceId);
					insertExpiredConference(uri);
				}
			}
			tr.commit();
		};
	}
#endif
}

long long MainDb::findExpiredConferenceId(const std::shared_ptr<Address> &uri) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		return L_DB_TRANSACTION {
			L_D();
			long long expiredConferenceId = d->findExpiredConferenceId(uri);
			tr.commit();
			return expiredConferenceId;
		};
	}
#endif
	return -1;
}

void MainDb::insertExpiredConference(const std::shared_ptr<Address> &uri) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		L_D();
		soci::session *session = d->dbSession.getBackendSession();
		long long expiredConferenceId = d->findExpiredConferenceId(uri);
		if (expiredConferenceId != -1) {
			return;
		}
		const long long &uriSipAddressId = d->insertSipAddress(uri);
		*session << "INSERT INTO expired_conferences (uri_sip_address_id) VALUES (:uriSipAddressId)",
		    soci::use(uriSipAddressId);
	}
#endif
}

void MainDb::deleteConferenceInfo(long long dbConferenceId) {
#ifdef HAVE_DB_STORAGE
	L_D();
	long long peerId;
	soci::session *session = d->dbSession.getBackendSession();
	std::string peerIdQuery =
	    "SELECT uri_sip_address_id FROM conference_info WHERE (conference_info.id = :conferenceInfoId)";
	*session << peerIdQuery, soci::use(dbConferenceId), soci::into(peerId);
	if (session->got_data()) {
		long long chatRoomId = d->selectChatRoomId(peerId);
		if (chatRoomId >= 0) {
			long long localId;
			std::string localIdQuery = "SELECT local_sip_address_id FROM chat_room WHERE (id = :chatRoomId)";
			*session << localIdQuery, soci::use(dbConferenceId), soci::into(localId);
			Address peer(d->selectSipAddressFromId(peerId));
			Address local(d->selectSipAddressFromId(localId));
			d->deleteChatRoom(ConferenceId(std::move(peer), std::move(local), getCore()->createConferenceIdParams()));
		}
	}

	*session << "DELETE FROM conference_info WHERE id = :conferenceId", soci::use(dbConferenceId);
	d->storageIdToConferenceInfo.erase(dbConferenceId);
#endif
}

void MainDb::deleteConferenceInfo(const std::shared_ptr<Address> &address) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		L_DB_TRANSACTION {
			L_D();
			if (address) {
				auto prunedAddress = address->getUriWithoutGruu();
				lInfo() << "Deleting conference information linked to conference " << prunedAddress;
				const long long &uriSipAddressId = d->selectSipAddressId(prunedAddress, false);
				const long long &dbConferenceId = d->selectConferenceInfoId(uriSipAddressId);
				deleteConferenceInfo(dbConferenceId);
			}
			tr.commit();
		};
	}
#endif
}

void MainDb::deleteConferenceInfo(const std::shared_ptr<ConferenceInfo> &conferenceInfo) {
	deleteConferenceInfo(conferenceInfo->getUri());
}

// -----------------------------------------------------------------------------

long long MainDb::insertCallLog(const std::shared_ptr<CallLog> &callLog) {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		long long id = d->insertOrUpdateConferenceCall(callLog, nullptr);
		tr.commit();

		return id;
	};
#else
	return -1;
#endif
}

void MainDb::updateCallLog(const std::shared_ptr<CallLog> &callLog) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		d->updateConferenceCall(callLog);
		tr.commit();
	};
#endif
}

void MainDb::deleteCallLog(const std::shared_ptr<CallLog> &callLog) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		const long long &dbConferenceCallId = d->selectConferenceCallId(callLog->getCallId());

		*d->dbSession.getBackendSession() << "DELETE FROM conference_call WHERE id = :conferenceCallId",
		    soci::use(dbConferenceCallId);
		d->storageIdToCallLog.erase(dbConferenceCallId),

		    tr.commit();
	};
#endif
}

std::shared_ptr<CallLog> MainDb::getCallLog(const std::string &callId, int limit) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		string query = "SELECT c.id, from_sip_address.value, from_sip_address.display_name, to_sip_address.value, "
		               "to_sip_address.display_name,"
		               "  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, "
		               "refkey, conference_info_id"
		               " FROM (conference_call as c, sip_address AS from_sip_address, sip_address AS to_sip_address)";

		if (limit > 0) {
			query += " INNER JOIN (SELECT id from conference_call ORDER BY id DESC LIMIT " + std::to_string(limit) +
			         ") as c2 ON c.id = c2.id";
		}

		query += " WHERE c.from_sip_address_id = from_sip_address.id AND c.to_sip_address_id = to_sip_address.id"
		         "  AND call_id = :callId";

		DurationLogger durationLogger("Get call log.");

		return L_DB_TRANSACTION {
			L_D();

			std::shared_ptr<CallLog> callLog = nullptr;

			soci::session *session = d->dbSession.getBackendSession();
			soci::rowset<soci::row> rows = (session->prepare << query, soci::use(callId));

			const auto &row = rows.begin();
			if (row != rows.end()) {
				callLog = d->selectCallLog(*row);
			}

			tr.commit();

			return callLog;
		};
	}
#endif
	return nullptr;
}

std::list<std::shared_ptr<CallLog>> MainDb::getCallHistory(int limit) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		if (limit == 0) return list<shared_ptr<CallLog>>();
		string query = "SELECT conference_call.id, from_sip_address.value, from_sip_address.display_name, "
		               "to_sip_address.value, to_sip_address.display_name,"
		               "  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, "
		               "refkey, conference_info_id"
		               " FROM conference_call, sip_address AS from_sip_address, sip_address AS to_sip_address"
		               " WHERE conference_call.from_sip_address_id = from_sip_address.id AND "
		               "conference_call.to_sip_address_id = to_sip_address.id"
		               " ORDER BY conference_call.id DESC";

		if (limit > 0) query += " LIMIT " + to_string(limit);

		DurationLogger durationLogger("Get call history.");

		return L_DB_TRANSACTION {
			L_D();

			list<shared_ptr<CallLog>> clList;

			soci::session *session = d->dbSession.getBackendSession();
			soci::rowset<soci::row> rows = (session->prepare << query);
			for (const auto &row : rows) {
				auto callLog = d->selectCallLog(row);
				clList.push_back(callLog);
			}

			tr.commit();

			return clList;
		};
	}
#endif
	return list<shared_ptr<CallLog>>();
}

std::list<std::shared_ptr<CallLog>> MainDb::getCallHistoryForLocalAddress(const std::shared_ptr<Address> &localAddress,
                                                                          int limit) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		string query = "SELECT conference_call.id, from_sip_address.value, from_sip_address.display_name, "
		               "to_sip_address.value, to_sip_address.display_name,"
		               "  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, "
		               "refkey, conference_info_id"
		               " FROM conference_call, sip_address AS from_sip_address, sip_address AS to_sip_address"
		               " WHERE conference_call.from_sip_address_id = from_sip_address.id AND "
		               "conference_call.to_sip_address_id = to_sip_address.id"
		               "  AND ((from_sip_address.value LIKE '%%" +
		               localAddress->toStringUriOnlyOrdered() +
		               "%%' AND direction = 0) OR" // 0 == outgoing
		               "  (to_sip_address.value LIKE '%%" +
		               localAddress->toStringUriOnlyOrdered() +
		               "%%' AND direction = 1))" // 1 == incoming
		               " ORDER BY conference_call.id DESC";

		if (limit > 0) query += " LIMIT " + to_string(limit);

		DurationLogger durationLogger("Get call history for address " + localAddress->toString());

		return L_DB_TRANSACTION {
			L_D();

			list<shared_ptr<CallLog>> clList;

			soci::session *session = d->dbSession.getBackendSession();
			soci::rowset<soci::row> rows = (session->prepare << query);
			for (const auto &row : rows) {
				auto callLog = d->selectCallLog(row);
				clList.push_back(callLog);
			}

			tr.commit();

			return clList;
		};
	}
#endif
	return list<shared_ptr<CallLog>>();
}

std::list<std::shared_ptr<CallLog>> MainDb::getCallHistory(const std::shared_ptr<const Address> &peer,
                                                           const std::shared_ptr<const Address> &local,
                                                           int limit) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		string query =
		    "SELECT conference_call.id, from_sip_address.value, from_sip_address.display_name, "
		    "to_sip_address.value, to_sip_address.display_name,"
		    "  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, "
		    "refkey, conference_info_id"
		    " FROM conference_call, sip_address AS from_sip_address, sip_address AS to_sip_address"
		    " WHERE conference_call.from_sip_address_id = from_sip_address.id AND "
		    "conference_call.to_sip_address_id = to_sip_address.id"
		    "  AND ((from_sip_address.value LIKE '%%" +
		    local->toStringUriOnlyOrdered() + "%%' AND to_sip_address.value LIKE '%%" + peer->toStringUriOnlyOrdered() +
		    "%%' AND direction = 0) OR" // 0 == outgoing
		    "  (from_sip_address.value LIKE '%%" +
		    peer->toStringUriOnlyOrdered() + "%%' AND to_sip_address.value LIKE '%%" + local->toStringUriOnlyOrdered() +
		    "%%' AND direction = 1))" // 1 == incoming
		    " ORDER BY conference_call.id DESC";

		if (limit > 0) query += " LIMIT " + to_string(limit);

		DurationLogger durationLogger("Get call history for local address " + local->toString() + " and peer address " +
		                              peer->toString());

		return L_DB_TRANSACTION {
			L_D();

			list<shared_ptr<CallLog>> clList;

			soci::session *session = d->dbSession.getBackendSession();

			soci::rowset<soci::row> rows = (session->prepare << query);
			for (const auto &row : rows) {
				auto callLog = d->selectCallLog(row);
				clList.push_back(callLog);
			}

			tr.commit();

			return clList;
		};
	}
#endif
	return list<shared_ptr<CallLog>>();
}

std::shared_ptr<CallLog> MainDb::getLastOutgoingCall() {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		static const string query =
		    "SELECT conference_call.id, from_sip_address.value, from_sip_address.display_name, "
		    "to_sip_address.value, to_sip_address.display_name,"
		    "  direction, duration, start_time, connected_time, status, video_enabled, quality, "
		    "call_id, refkey, conference_info_id"
		    " FROM conference_call, sip_address AS from_sip_address, sip_address AS to_sip_address"
		    " WHERE conference_call.from_sip_address_id = from_sip_address.id AND "
		    "conference_call.to_sip_address_id = to_sip_address.id"
		    "  AND direction = 0 AND conference_info_id IS NULL" // 0 == outgoing
		    " ORDER BY conference_call.id DESC LIMIT 1";

		DurationLogger durationLogger("Get last outgoing call.");

		return L_DB_TRANSACTION {
			L_D();

			std::shared_ptr<CallLog> callLog = nullptr;

			soci::session *session = d->dbSession.getBackendSession();

			soci::rowset<soci::row> rows = (session->prepare << query);

			const auto &row = rows.begin();
			if (row != rows.end()) {
				callLog = d->selectCallLog(*row);
			}

			tr.commit();

			return callLog;
		};
	}
#endif
	return nullptr;
}

void MainDb::deleteCallHistory() {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		L_DB_TRANSACTION {
			L_D();

			soci::session *session = d->dbSession.getBackendSession();

			*session << "DELETE FROM conference_call";

			tr.commit();
		};
	}
#endif
}

void MainDb::deleteCallHistoryForLocalAddress(const std::shared_ptr<Address> &localAddress) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		L_DB_TRANSACTION {
			L_D();

			soci::session *session = d->dbSession.getBackendSession();

			const long long &sipAddressId = d->selectSipAddressId(localAddress, true);

			*session << "DELETE FROM conference_call WHERE"
			            " ((from_sip_address_id = :sipAddressId  AND direction = 0) OR" // 0 == outgoing
			            " (to_sip_address_id = :sipAddressId AND direction = 1))",      // 1 == incoming
			    soci::use(sipAddressId);

			tr.commit();
		};
	}
#endif
}

int MainDb::getCallHistorySize() {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		return L_DB_TRANSACTION {
			L_D();

			int count = 0;

			soci::session *session = d->dbSession.getBackendSession();

			soci::statement st = (session->prepare << "SELECT count(*) FROM conference_call", soci::into(count));
			st.execute();
			st.fetch();

			tr.commit();

			return count;
		};
	}
#endif
	return -1;
}

// -----------------------------------------------------------------------------

long long MainDb::insertFriend(const std::shared_ptr<Friend> &f) {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		long long id = d->insertOrUpdateFriend(f);
		tr.commit();

		return id;
	};
#else
	return -1;
#endif
}

long long MainDb::insertFriendList(const std::shared_ptr<FriendList> &list) {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		long long id = d->insertOrUpdateFriendList(list);
		tr.commit();

		return id;
	};
#else
	return -1;
#endif
}

void MainDb::deleteFriend(const std::shared_ptr<Friend> &f) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		long long dbFriendId = f->mStorageId;
		*d->dbSession.getBackendSession() << "DELETE FROM friend WHERE id = :dbFriendId", soci::use(dbFriendId);
		tr.commit();
	};
#endif
}

void MainDb::deleteFriendList(const std::shared_ptr<FriendList> &list) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		long long dbFriendListId = list->mStorageId;
		*d->dbSession.getBackendSession() << "DELETE FROM friends_list WHERE id = :dbFriendListId",
		    soci::use(dbFriendListId);
		tr.commit();
	};
#endif
}

void MainDb::deleteFriendList(const std::string &name) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		*d->dbSession.getBackendSession() << "DELETE FROM friends_list WHERE name = :name", soci::use(name);
		tr.commit();
	};
#endif
}

void MainDb::deleteOrphanFriends() {
#ifdef HAVE_DB_STORAGE

	if (!isInitialized()) {
		lWarning() << "Unable to delete orphan friends because the database has not been initialized";
		return;
	}

	L_DB_TRANSACTION {
		L_D();

		*d->dbSession.getBackendSession()
		    << "DELETE FROM friend WHERE friends_list_id NOT IN (SELECT id from friends_list)";
		tr.commit();
	};
#endif
}

std::list<std::shared_ptr<Friend>> MainDb::getFriends(const std::shared_ptr<FriendList> &list) {
#ifdef HAVE_DB_STORAGE
	DurationLogger durationLogger("Get friends.");

	if (list->mStorageId < 0) return std::list<std::shared_ptr<Friend>>();

	return L_DB_TRANSACTION {
		L_D();

		std::list<std::shared_ptr<Friend>> clList = d->getFriends(list);
		tr.commit();

		return clList;
	};
#else
	return std::list<std::shared_ptr<Friend>>();
#endif
}

std::list<std::shared_ptr<FriendList>> MainDb::getFriendLists() {
#ifdef HAVE_DB_STORAGE
	DurationLogger durationLogger("Get friend lists.");

	if (!isInitialized()) {
		lWarning() << "Unable to get friend list because the database has not been initialized";
		return std::list<std::shared_ptr<FriendList>>();
	}

	return L_DB_TRANSACTION {
		L_D();

		std::list<std::shared_ptr<FriendList>> clList;

		soci::session *session = d->dbSession.getBackendSession();

		soci::rowset<soci::row> rows =
		    (session->prepare
		     << "SELECT id, name, rls_uri, sync_uri, revision, type, ctag FROM friends_list ORDER BY id");
		for (const auto &row : rows) {
			auto list = d->selectFriendList(row);
			list->setCore(getCore());
			auto friends = d->getFriends(list);
			list->setFriends(friends);
			clList.push_back(list);
		}

		tr.commit();

		return clList;
	};
#else
	return std::list<std::shared_ptr<FriendList>>();
#endif
}

void MainDb::insertDevices(const std::list<std::pair<std::shared_ptr<Address>, std::string>> &devices) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();
		for (auto &deviceAndName : devices) {
			d->insertOrUpdateDevice(deviceAndName.first, deviceAndName.second);
		}
		tr.commit();
	};
#endif
}

long long MainDb::insertDevice(BCTBX_UNUSED(const std::shared_ptr<Address> &addressWithGruu),
                               BCTBX_UNUSED(const std::string &displayName)) {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		long long id = d->insertOrUpdateDevice(addressWithGruu, displayName);
		tr.commit();

		return id;
	};
#else
	return -1;
#endif
}

void MainDb::removeDevice(BCTBX_UNUSED(const std::shared_ptr<Address> &addressWithGruu)) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		const string query = "DELETE FROM friend_devices WHERE device_address_id = :1";
		long long deviceAddressId = d->selectSipAddressId(addressWithGruu, false);

		if (deviceAddressId > 0) {
			soci::session *session = d->dbSession.getBackendSession();
			(session->prepare << query, soci::use(deviceAddressId));
		}

		tr.commit();
	};
#endif
}

std::list<std::shared_ptr<FriendDevice>> MainDb::getDevices(BCTBX_UNUSED(const std::shared_ptr<Address> &address)) {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		const string query = "SELECT device_address_id, display_name FROM friend_devices WHERE sip_address_id = :1";
		std::list<std::shared_ptr<FriendDevice>> devicesList;

		const auto sipAddress = address->getUriWithoutGruu();
		long long sipAddressId = d->selectSipAddressId(sipAddress, false);

		if (sipAddressId > 0) {
			soci::session *session = d->dbSession.getBackendSession();
			soci::rowset<soci::row> rows = (session->prepare << query, soci::use(sipAddressId));

			for (const auto &row : rows) {
				long long addrId = d->dbSession.resolveId(row, 0);
				Address gruuAddress = Address(d->selectSipAddressFromId(addrId));

				std::string displayName = row.get<string>(1);

				auto device = FriendDevice::create(gruuAddress, displayName,
				                                   LinphoneSecurityLevelNone); // Security level isn't available here
				devicesList.push_back(device);
			}
		}

		tr.commit();

		return devicesList;
	};
#else
	return std::list<std::shared_ptr<FriendDevice>>();
#endif
}

// -----------------------------------------------------------------------------

bool MainDb::import(Backend, const string &parameters) {
#ifdef HAVE_DB_STORAGE
	L_D();
	bool ret = false;

	// Backend is useless, it's sqlite3. (Only available legacy backend.)
	const string uri = "sqlite3://" + LinphonePrivate::Utils::localeToUtf8(parameters);
	DbSession inDbSession(uri);

	if (!inDbSession) {
		lWarning() << "Unable to connect to: `" << uri << "`.";
		return false;
	}

	ret |= d->importLegacyFriends(inDbSession);
	ret |= d->importLegacyHistory(inDbSession);
	ret |= d->importLegacyCallLogs(inDbSession);

	return ret;
#else
	return false;
#endif
}

MainDb::FilterMask MainDb::getFilterMaskFromHistoryFilterMask(AbstractChatRoom::HistoryFilterMask historyFilterMask) {
	FilterMask mask;

	if (historyFilterMask.isSet(AbstractChatRoom::HistoryFilter::None)) mask |= NoFilter;
	if (historyFilterMask.isSet(AbstractChatRoom::HistoryFilter::Call)) mask |= ConferenceCallFilter;
	if (historyFilterMask.isSet(AbstractChatRoom::HistoryFilter::ChatMessage)) mask |= ConferenceChatMessageFilter;
	if (historyFilterMask.isSet(AbstractChatRoom::HistoryFilter::ChatMessageSecurity))
		mask |= ConferenceChatMessageSecurityFilter;
	if (historyFilterMask.isSet(AbstractChatRoom::HistoryFilter::Info)) mask |= ConferenceInfoFilter;
	if (historyFilterMask.isSet(AbstractChatRoom::HistoryFilter::InfoNoDevice)) mask |= ConferenceInfoNoDeviceFilter;

	return mask;
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

LINPHONE_END_NAMESPACE
