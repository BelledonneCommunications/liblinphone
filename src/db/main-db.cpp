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
	#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

#include <ctime>

#include <bctoolbox/defs.h>

#include "linphone/utils/algorithm.h"
#include "linphone/utils/static-string.h"

#include "c-wrapper/internal/c-tools.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room-p.h"
#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-group-chat-room.h"
#include "chat/chat-room/client-group-chat-room-p.h"
#include "chat/chat-room/server-group-chat-room.h"
#endif
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "core/core-p.h"
#include "event-log/event-log-p.h"
#include "event-log/events.h"
#include "main-db-key-p.h"
#include "main-db-p.h"

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
	constexpr unsigned int ModuleVersionEvents = makeVersion(1, 0, 20);
	constexpr unsigned int ModuleVersionFriends = makeVersion(1, 0, 0);
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
}
#endif

// -----------------------------------------------------------------------------
// soci helpers.
// -----------------------------------------------------------------------------
#ifdef HAVE_DB_STORAGE
static inline vector<char> blobToVector (soci::blob &in) {
	size_t len = in.get_len();
	if (!len)
		return vector<char>();
	vector<char> out(len);
	in.read(0, &out[0], len);
	return out;
}

static inline string blobToString (soci::blob &in) {
	vector<char> out = blobToVector(in);
	return string(out.begin(), out.end());
}

static constexpr string &blobToString (string &in) {
	return in;
}

template<typename T>
static T getValueFromRow (const soci::row &row, int index, bool &isNull) {
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
template<typename T>
struct EnumToSql {
	T first;
	const char *second;
};

template<typename T>
static constexpr const char *mapEnumToSql (const EnumToSql<T> enumToSql[], size_t n, T key) {
	return n == 0 ? "" : (
		enumToSql[n - 1].first == key ? enumToSql[n - 1].second : mapEnumToSql(enumToSql, n - 1, key)
	);
}

template<EventLog::Type ...Type>
struct SqlEventFilterBuilder {};

template<EventLog::Type Type, EventLog::Type... List>
struct SqlEventFilterBuilder<Type, List...> {
	static constexpr auto get () L_AUTO_RETURN(
		StaticIntString<int(Type)>() + "," + SqlEventFilterBuilder<List...>::get()
	);
};

template<EventLog::Type Type>
struct SqlEventFilterBuilder<Type> {
	static constexpr auto get () L_AUTO_RETURN(StaticIntString<int(Type)>());
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
		constexpr auto ConferenceCallFilter = SqlEventFilterBuilder<
			EventLog::Type::ConferenceCallStarted,
			EventLog::Type::ConferenceCallConnected,
			EventLog::Type::ConferenceCallEnded
		>::get();

		constexpr auto ConferenceChatMessageFilter = SqlEventFilterBuilder<EventLog::Type::ConferenceChatMessage>::get();

		constexpr auto ConferenceInfoNoDeviceFilter = SqlEventFilterBuilder<
			EventLog::Type::ConferenceCreated,
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
			EventLog::Type::ConferenceEphemeralMessageDisabled
		>::get();

		constexpr auto ConferenceInfoFilter = ConferenceInfoNoDeviceFilter + "," + SqlEventFilterBuilder<
			EventLog::Type::ConferenceParticipantDeviceAdded,
			EventLog::Type::ConferenceParticipantDeviceRemoved,
			EventLog::Type::ConferenceParticipantDeviceStatusChanged,
			EventLog::Type::ConferenceParticipantDeviceMediaCapabilityChanged,
			EventLog::Type::ConferenceParticipantDeviceMediaAvailabilityChanged
		>::get();

		constexpr auto ConferenceChatMessageSecurityFilter = ConferenceChatMessageFilter + "," + SqlEventFilterBuilder<
			EventLog::Type::ConferenceSecurityEvent,
			EventLog::Type::ConferenceEphemeralMessageLifetimeChanged,
			EventLog::Type::ConferenceEphemeralMessageEnabled,
			EventLog::Type::ConferenceEphemeralMessageDisabled,
			EventLog::Type::ConferenceEphemeralMessageManagedByAdmin,
			EventLog::Type::ConferenceEphemeralMessageManagedByParticipants
		>::get();
	#endif // ifdef _WIN32

	constexpr EnumToSql<MainDb::Filter> EventFilterToSql[] = {
		{ MainDb::ConferenceCallFilter, ConferenceCallFilter },
		{ MainDb::ConferenceChatMessageFilter, ConferenceChatMessageFilter },
		{ MainDb::ConferenceInfoNoDeviceFilter, ConferenceInfoNoDeviceFilter },
		{ MainDb::ConferenceInfoFilter, ConferenceInfoFilter },
		{ MainDb::ConferenceChatMessageSecurityFilter, ConferenceChatMessageSecurityFilter }
	};
}

static const char *mapEventFilterToSql (MainDb::Filter filter) {
	return mapEnumToSql(
		EventFilterToSql, sizeof EventFilterToSql / sizeof EventFilterToSql[0], filter
	);
}

// -----------------------------------------------------------------------------

static string buildSqlEventFilter (
	const list<MainDb::Filter> &filters,
	MainDb::FilterMask mask,
	const string &condKeyWord = "WHERE"
) {
	L_ASSERT(findIf(filters, [](const MainDb::Filter &filter) { return filter == MainDb::NoFilter; }) == filters.cend());

	if (mask == MainDb::NoFilter)
		return "";

	bool isStart = true;
	string sql;
	for (const auto &filter : filters) {
		if (!mask.isSet(filter))
			continue;

		if (isStart) {
			isStart = false;
			sql += " " + condKeyWord + " type IN (";
		} else
			sql += ", ";
		sql += mapEventFilterToSql(filter);
	}

	if (!isStart)
		sql += ") ";

	return sql;
}
#endif

// -----------------------------------------------------------------------------
// Misc helpers.
// -----------------------------------------------------------------------------

shared_ptr<AbstractChatRoom> MainDbPrivate::findChatRoom (const ConferenceId &conferenceId) const {
	L_Q();
	shared_ptr<AbstractChatRoom> chatRoom = q->getCore()->findChatRoom(conferenceId);
	if (!chatRoom)
		lError() << "Unable to find chat room: " << conferenceId << ".";
	return chatRoom;
}

shared_ptr<MediaConference::Conference> MainDbPrivate::findAudioVideoConference (const ConferenceId &conferenceId) const {
	L_Q();
	shared_ptr<MediaConference::Conference> conference = q->getCore()->findAudioVideoConference(conferenceId);
	if (!conference)
		lError() << "Unable to find chat room: " << conferenceId << ".";
	return conference;
}
// -----------------------------------------------------------------------------
// Low level API.
// -----------------------------------------------------------------------------

long long MainDbPrivate::insertSipAddress (const Address &address) {
#ifdef HAVE_DB_STORAGE
	// This is a hack, because all addresses don't print their parameters in the same order.
	const string sipAddress = ConferenceAddress(address).asString();
	const string displayName = address.getDisplayName();

	long long sipAddressId = selectSipAddressId(sipAddress);

	if (sipAddressId < 0) {
		lInfo() << "Insert new sip address in database: `" << sipAddress << "`.";
		soci::indicator displayNameInd = displayName.empty() ? soci::i_null : soci::i_ok;

		*dbSession.getBackendSession() << "INSERT INTO sip_address (value, display_name) VALUES (:sipAddress, :displayName)",
			soci::use(sipAddress), soci::use(displayName, displayNameInd);

		return dbSession.getLastInsertId();
	} else if (sipAddressId >=0 && !displayName.empty()) {
		lInfo() << "Updating sip address display name in database: `" << sipAddress << "`.";

		*dbSession.getBackendSession() << "UPDATE sip_address SET display_name = :displayName WHERE id = :id",
			soci::use(displayName), soci::use(sipAddressId);
	}

	return sipAddressId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertSipAddress (const string &sipAddress) {
#ifdef HAVE_DB_STORAGE
	long long sipAddressId = selectSipAddressId(sipAddress);
	if (sipAddressId >= 0)
		return sipAddressId;

	lInfo() << "Insert new sip address in database: `" << sipAddress << "`.";
	*dbSession.getBackendSession() << "INSERT INTO sip_address (value, display_name) VALUES (:sipAddress, NULL)", soci::use(sipAddress);
	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

void MainDbPrivate::insertContent (long long chatMessageId, const Content &content) {
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

	for (const auto &appData : content.getAppDataMap())
		*session << "INSERT INTO chat_message_content_app_data (chat_message_content_id, name, data) VALUES"
			" (:chatMessageContentId, :name, :data)",
			soci::use(chatMessageContentId), soci::use(appData.first), soci::use(appData.second);
#endif
}

long long MainDbPrivate::insertContentType (const string &contentType) {
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();

	long long contentTypeId;
	*session << "SELECT id FROM content_type WHERE value = :contentType", soci::use(contentType), soci::into(contentTypeId);
	if (session->got_data())
		return contentTypeId;

	lInfo() << "Insert new content type in database: `" << contentType << "`.";
	*session << "INSERT INTO content_type (value) VALUES (:contentType)", soci::use(contentType);
	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateImportedBasicChatRoom (
	long long peerSipAddressId,
	long long localSipAddressId,
	const tm &creationTime
) {
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();

	long long chatRoomId = selectChatRoomId(peerSipAddressId, localSipAddressId);
	if (chatRoomId >= 0) {
		*session << "UPDATE chat_room SET last_update_time = :lastUpdateTime WHERE id = :chatRoomId",
			soci::use(creationTime), soci::use(chatRoomId);
		return chatRoomId;
	}

	const int capabilities = ChatRoom::CapabilitiesMask(
		{ ChatRoom::Capabilities::Basic, ChatRoom::Capabilities::Migratable }
	);
	lInfo() << "Insert new chat room in database: (peer=" << peerSipAddressId <<
		", local=" << localSipAddressId << ", capabilities=" << capabilities << ").";
	*session << "INSERT INTO chat_room ("
		"  peer_sip_address_id, local_sip_address_id, creation_time, last_update_time, capabilities"
		") VALUES (:peerSipAddressId, :localSipAddressId, :creationTime, :lastUpdateTime, :capabilities)",
		soci::use(peerSipAddressId), soci::use(localSipAddressId), soci::use(creationTime), soci::use(creationTime),
		soci::use(capabilities);

	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

long long MainDbPrivate::insertChatRoom (const shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (q->isInitialized()) {
		const ConferenceId &conferenceId = chatRoom->getConferenceId();
		const long long &peerSipAddressId = insertSipAddress(conferenceId.getPeerAddress().asString());
		const long long &localSipAddressId = insertSipAddress(conferenceId.getLocalAddress().asString());

		long long chatRoomId = selectChatRoomId(peerSipAddressId, localSipAddressId);
		const int flags = chatRoom->hasBeenLeft() ? 1 : 0;
		if (chatRoomId >= 0) {
			// The chat room is already stored in DB, but still update the notify id and the flags that might have changed
			lInfo() << "Update chat room in database: " << conferenceId << ".";
			*dbSession.getBackendSession() << "UPDATE chat_room SET"
				" last_notify_id = :lastNotifyId, "
				" flags = :flags "
				" WHERE id = :chatRoomId",
				soci::use(notifyId), soci::use(flags), soci::use(chatRoomId);
		} else {

			lInfo() << "Insert new chat room in database: " << conferenceId << ".";

			const tm &creationTime = Utils::getTimeTAsTm(chatRoom->getCreationTime());
			const tm &lastUpdateTime = Utils::getTimeTAsTm(chatRoom->getLastUpdateTime());

			// Remove capabilities like `Proxy`.
			const int &capabilities = chatRoom->getCapabilities() & ~ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::Proxy);

			const string &subject = chatRoom->getUtf8Subject();
			int ephemeralEnabled = chatRoom->ephemeralEnabled() ? 1 : 0;
			long ephemeralLifeTime = chatRoom->getEphemeralLifetime();
			*dbSession.getBackendSession() << "INSERT INTO chat_room ("
			"  peer_sip_address_id, local_sip_address_id, creation_time,"
			"  last_update_time, capabilities, subject, flags, last_notify_id, ephemeral_enabled, ephemeral_messages_lifetime"
			") VALUES ("
			"  :peerSipAddressId, :localSipAddressId, :creationTime,"
			"  :lastUpdateTime, :capabilities, :subject, :flags, :lastNotifyId, :ephemeralEnabled, :ephemeralLifeTime"
			")",
			soci::use(peerSipAddressId), soci::use(localSipAddressId), soci::use(creationTime),
			soci::use(lastUpdateTime), soci::use(capabilities), soci::use(subject), soci::use(flags),
			soci::use(notifyId), soci::use(ephemeralEnabled), soci::use(ephemeralLifeTime);

			chatRoomId = dbSession.getLastInsertId();
		}
		// Do not add 'me' when creating a server-group-chat-room.
		if (conferenceId.getLocalAddress() != conferenceId.getPeerAddress()) {
			shared_ptr<Participant> me = chatRoom->getMe();
			long long meId = insertChatRoomParticipant(
				chatRoomId,
				insertSipAddress(me->getAddress().asString()),
				me->isAdmin()
			);
			for (const auto &device : me->getDevices())
				insertChatRoomParticipantDevice(meId, insertSipAddress(device->getAddress().asString()), device->getName());
		}

		for (const auto &participant : chatRoom->getParticipants()) {
			long long participantId = insertChatRoomParticipant(
				chatRoomId,
				insertSipAddress(participant->getAddress().asString()),
				participant->isAdmin()
			);
			for (const auto &device : participant->getDevices())
				insertChatRoomParticipantDevice(participantId, insertSipAddress(device->getAddress().asString()), device->getName());
		}

		return chatRoomId;
	}
#endif
	return -1;
}

long long MainDbPrivate::insertChatRoomParticipant (
	long long chatRoomId,
	long long participantSipAddressId,
	bool isAdmin
) {
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

void MainDbPrivate::insertChatRoomParticipantDevice (
	long long participantId,
	long long participantDeviceSipAddressId,
	const string &deviceName
) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (q->isInitialized()) {
		soci::session *session = dbSession.getBackendSession();
		long long count;
		*session << "SELECT COUNT(*) FROM chat_room_participant_device"
			" WHERE chat_room_participant_id = :participantId"
			" AND participant_device_sip_address_id = :participantDeviceSipAddressId",
			soci::into(count), soci::use(participantId), soci::use(participantDeviceSipAddressId);
		if (count)
			return;

		*session << "INSERT INTO chat_room_participant_device (chat_room_participant_id, participant_device_sip_address_id, name)"
			" VALUES (:participantId, :participantDeviceSipAddressId, :participantDeviceName)",
			soci::use(participantId), soci::use(participantDeviceSipAddressId), soci::use(deviceName);
	}
#endif
}

void MainDbPrivate::insertChatMessageParticipant (long long chatMessageId, long long sipAddressId, int state, time_t stateChangeTime) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (q->isInitialized()) {
		const tm &stateChangeTm = Utils::getTimeTAsTm(stateChangeTime);
		*dbSession.getBackendSession() <<
			"INSERT INTO chat_message_participant (event_id, participant_sip_address_id, state, state_change_time)"
			" VALUES (:chatMessageId, :sipAddressId, :state, :stateChangeTm)",
			soci::use(chatMessageId), soci::use(sipAddressId), soci::use(state), soci::use(stateChangeTm);
	}
#endif
}

long long MainDbPrivate::insertConferenceInfo (const std::shared_ptr<ConferenceInfo> &conferenceInfo, const std::shared_ptr<ConferenceInfo> &oldConferenceInfo) {
#ifdef HAVE_DB_STORAGE
	L_Q();
	if (!q->isInitialized()) {
		lError() << "Database hasn't been initialized";
		return -1;
	}

	if (!conferenceInfo->getOrganizerAddress().isValid() || !conferenceInfo->getUri().isValid()) {
		lError() << "Trying to insert a Conference Info without organizer or URI!";
		return -1;
	}
	const long long &organizerSipAddressId = insertSipAddress(conferenceInfo->getOrganizerAddress().asString());
	const long long &uriSipAddressid = insertSipAddress(conferenceInfo->getUri().asString());
	const tm &startTime = Utils::getTimeTAsTm(conferenceInfo->getDateTime());
	const unsigned int duration = conferenceInfo->getDuration();
	const string &subject = conferenceInfo->getUtf8Subject();
	const string &description = conferenceInfo->getUtf8Description();
	const unsigned int state = static_cast<unsigned int>(conferenceInfo->getState());
	const unsigned int& sequence = conferenceInfo->getIcsSequence();
	const string& uid = conferenceInfo->getIcsUid();

	long long conferenceInfoId = selectConferenceInfoId(uriSipAddressid);
	ConferenceInfo::participant_list_t dbParticipantList;
	if (conferenceInfoId >= 0) {
		// The conference info is already stored in DB, but still update it some information might have changed
		lInfo() << "Update conferenceInfo in database: " << conferenceInfoId << ".";
		dbParticipantList = oldConferenceInfo->getParticipants();

		*dbSession.getBackendSession() << "UPDATE conference_info SET"
			"  organizer_sip_address_id = :organizerSipAddressId,"
			"  start_time = :startTime,"
			"  duration = :duration,"
			"  subject = :subject,"
			"  description = :description,"
			"  state = :state,"
			"  ics_sequence = :sequence,"
			"  ics_uid = :uid"
			" WHERE id = :conferenceInfoId",
			soci::use(organizerSipAddressId), soci::use(startTime), soci::use(duration),
			soci::use(subject), soci::use(description), soci::use(state),
			soci::use(sequence), soci::use(uid), soci::use(conferenceInfoId);
	} else {
		lInfo() << "Insert new conference info in database.";

		*dbSession.getBackendSession() << "INSERT INTO conference_info ("
		"  organizer_sip_address_id, uri_sip_address_id, start_time, duration, subject, description, state, ics_sequence, ics_uid"
		") VALUES ("
		"  :organizerSipAddressId, :uriSipAddressid, :startTime, :duration, :subject, :description, :state, :sequence, :uid"
		")",
		soci::use(organizerSipAddressId), soci::use(uriSipAddressid), soci::use(startTime),
		soci::use(duration),  soci::use(subject), soci::use(description), soci::use(state),
		soci::use(sequence), soci::use(uid);

		conferenceInfoId = dbSession.getLastInsertId();
	}

	insertOrUpdateConferenceInfoOrganizer(conferenceInfoId, organizerSipAddressId, conferenceInfo->getOrganizer().second);

	const auto & participantList = conferenceInfo->getParticipants();
	for (const auto & participantAddress : participantList) {
		insertOrUpdateConferenceInfoParticipant(
			conferenceInfoId,
			insertSipAddress(participantAddress.first.asString()),
			false,
			participantAddress.second
		);
	}

	for (const auto & oldParticipantAddress : dbParticipantList) {
		const bool deleted = (std::find_if(participantList.cbegin(), participantList.cend(), [&oldParticipantAddress] (const auto & p) {
			return (p.first == oldParticipantAddress.first);
		}) == participantList.cend());
		if (deleted) {
			insertOrUpdateConferenceInfoParticipant(
				conferenceInfoId,
				insertSipAddress(oldParticipantAddress.first.asString()),
				true,
				oldParticipantAddress.second
			);
		}
	}

	cache(conferenceInfo, conferenceInfoId);

	return conferenceInfoId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateConferenceInfoOrganizer (long long conferenceInfoId, long long organizerSipAddressId, const ConferenceInfo::participant_params_t params) {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoOrganizerId = selectConferenceInfoOrganizerId(conferenceInfoId);
	auto paramsStr = ConferenceInfo::memberParametersToString(params);
	if (conferenceInfoOrganizerId >= 0) {
		*dbSession.getBackendSession() << "UPDATE conference_info_organizer SET"
		" organizer_sip_address_id = :organizerSipAddressId, params = :paramsStr"
		" WHERE conference_info_id  = :conferenceInfoId",
		soci::use(organizerSipAddressId), soci::use(paramsStr), soci::use(conferenceInfoId);

		return conferenceInfoOrganizerId;
	}

	*dbSession.getBackendSession() << "INSERT INTO conference_info_organizer (conference_info_id, organizer_sip_address_id, params)"
		" VALUES (:conferenceInfoId, :organizerSipAddressId, :paramsStr)",
		soci::use(conferenceInfoId), soci::use(organizerSipAddressId), soci::use(paramsStr);

	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateConferenceInfoParticipant (long long conferenceInfoId, long long participantSipAddressId, bool deleted, const ConferenceInfo::participant_params_t params) {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoParticipantId = selectConferenceInfoParticipantId(conferenceInfoId, participantSipAddressId);
	auto paramsStr = ConferenceInfo::memberParametersToString(params);
	int participantDeleted = deleted ? 1 : 0;
	if (conferenceInfoParticipantId >= 0) {
		*dbSession.getBackendSession() << "UPDATE conference_info_participant SET"
		" deleted = :deleted, params = :paramsStr"
		" WHERE conference_info_id  = :conferenceInfoId AND participant_sip_address_id = :participantSipAddressId",
		soci::use(participantDeleted), soci::use(paramsStr),
		soci::use(conferenceInfoId), soci::use(participantSipAddressId);

		return conferenceInfoParticipantId;
	}

	*dbSession.getBackendSession() << "INSERT INTO conference_info_participant (conference_info_id, participant_sip_address_id, deleted, params)"
		" VALUES (:conferenceInfoId, :participantSipAddressId, :deleted, :paramsStr)",
		soci::use(conferenceInfoId), soci::use(participantSipAddressId),
		soci::use(participantDeleted), soci::use(paramsStr);

	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

long long MainDbPrivate::insertOrUpdateConferenceCall (const std::shared_ptr<CallLog> &callLog, const std::shared_ptr<ConferenceInfo> &conferenceInfo) {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoId = -1;

	if (conferenceInfo != nullptr) {
		const long long &uriSipAddressid = insertSipAddress(conferenceInfo->getUri().asString());
		conferenceInfoId = selectConferenceInfoId(uriSipAddressid);
		if (conferenceInfoId < 0) {
			conferenceInfoId = insertConferenceInfo(conferenceInfo, nullptr);
		}
		callLog->setConferenceInfoId(conferenceInfoId);
	}

	int duration = callLog->getDuration();
	const tm &connectedTime = Utils::getTimeTAsTm(callLog->getConnectedTime());
	int status = callLog->getStatus();
	int videoEnabled = callLog->isVideoEnabled() ? 1 : 0;
	double quality = static_cast<double>(callLog->getQuality());
	const string &callId = callLog->getCallId();
	const string &refKey = callLog->getRefKey();
	soci::indicator confInfoInd = conferenceInfoId > -1 ? soci::i_ok : soci::i_null;

	long long conferenceCallId = selectConferenceCallId(callLog->getCallId());
	if (conferenceCallId < 0) {
		lInfo() << "Insert new conference call in database: " << callLog->getCallId();

		const long long fromSipAddressId = insertSipAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(callLog->getFromAddress()));
		const long long toSipAddressId = insertSipAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(callLog->getToAddress()));
		int direction = static_cast<int>(callLog->getDirection());
		const tm &startTime = Utils::getTimeTAsTm(callLog->getStartTime());

		*dbSession.getBackendSession() << "INSERT INTO conference_call ("
		"  from_sip_address_id, to_sip_address_id, direction, duration, start_time, connected_time, status, video_enabled,"
		"  quality, call_id, refkey, conference_info_id"
		") VALUES ("
		"  :fromSipAddressId, :toSipAddressId, :direction, :duration, :startTime, :connectedTime, :status, :videoEnabled,"
		"  :quality, :callId, :refKey, :conferenceInfoId"
		")",
		soci::use(fromSipAddressId), soci::use(toSipAddressId), soci::use(direction), soci::use(duration), soci::use(startTime),
		soci::use(connectedTime), soci::use(status), soci::use(videoEnabled), soci::use(quality), soci::use(callId),
		soci::use(refKey), soci::use(conferenceInfoId, confInfoInd);

		conferenceCallId = dbSession.getLastInsertId();
	} else {
		lInfo() << "Update conference call in database: " << callLog->getCallId();

		*dbSession.getBackendSession() << "UPDATE conference_call SET"
		"  duration = :duration, connected_time = :connectedTime, status = :status, video_enabled = :videoEnabled,"
		"  quality = :quality, call_id = :callId, refkey = :refKey, conference_info_id = :conferenceInfoId"
		" WHERE id = :conferenceCallId",
		soci::use(duration), soci::use(connectedTime), soci::use(status), soci::use(videoEnabled), soci::use(quality),
		soci::use(callId), soci::use(refKey), soci::use(conferenceInfoId, confInfoInd),
		soci::use(conferenceCallId);
	}

	cache(callLog, conferenceCallId);

	return conferenceCallId;
#else
	return -1;
#endif
}

// -----------------------------------------------------------------------------

long long MainDbPrivate::selectSipAddressId (const string &sipAddress) const {
#ifdef HAVE_DB_STORAGE
	long long sipAddressId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectSipAddressId),
		soci::use(sipAddress), soci::into(sipAddressId);

	return session->got_data() ? sipAddressId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectChatRoomId (long long peerSipAddressId, long long localSipAddressId) const {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectChatRoomId),
		soci::use(peerSipAddressId), soci::use(localSipAddressId), soci::into(chatRoomId);

	return session->got_data() ? chatRoomId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectChatRoomId (const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	long long peerSipAddressId = selectSipAddressId(conferenceId.getPeerAddress().asString());
	if (peerSipAddressId < 0)
		return -1;

	long long localSipAddressId = selectSipAddressId(conferenceId.getLocalAddress().asString());
	if (localSipAddressId < 0)
		return -1;

	long long id = selectChatRoomId(peerSipAddressId, localSipAddressId);
	if (id != -1) {
		cache(conferenceId, id);
	}

	return id;
#else
	return -1;
#endif
}

ConferenceId MainDbPrivate::selectConferenceId (const long long chatRoomId) const {
#ifdef HAVE_DB_STORAGE
	string peerSipAddress;
	string localSipAddress;

	string query = "SELECT peer_sip_address_id, local_sip_address_id FROM chat_room WHERE id = :1";
	soci::session *session = dbSession.getBackendSession();
	*session << query, soci::use(chatRoomId), soci::into(peerSipAddress), soci::into(localSipAddress);

	ConferenceId conferenceId = ConferenceId(
		IdentityAddress(peerSipAddress),
		IdentityAddress(localSipAddress)
	);

	if (conferenceId.isValid()) {
		cache(conferenceId, chatRoomId);
	}

	return conferenceId;
#else
	return ConferenceId();
#endif
}

long long MainDbPrivate::selectChatRoomParticipantId (long long chatRoomId, long long participantSipAddressId) const {
#ifdef HAVE_DB_STORAGE
	long long chatRoomParticipantId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectChatRoomParticipantId),
		soci::use(chatRoomId), soci::use(participantSipAddressId), soci::into(chatRoomParticipantId);

	return session->got_data() ? chatRoomParticipantId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectOneToOneChatRoomId (long long sipAddressIdA, long long sipAddressIdB, bool encrypted) const {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const int encryptedCapability = int(ChatRoom::Capabilities::Encrypted);
	const int expectedCapabilities = encrypted ? encryptedCapability : 0;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectOneToOneChatRoomId),
		soci::use(sipAddressIdA, "1"), soci::use(sipAddressIdB, "2"),
		soci::use(encryptedCapability, "3"), soci::use(expectedCapabilities, "4"),
		soci::into(chatRoomId);

	return session->got_data() ? chatRoomId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectConferenceInfoId (long long uriSipAddressId) {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectConferenceInfoId),
		soci::use(uriSipAddressId), soci::into(conferenceInfoId);

	return session->got_data() ? conferenceInfoId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectConferenceInfoOrganizerId (long long conferenceInfoId) const {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoOrganizerId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectConferenceInfoOrganizerId),
		soci::use(conferenceInfoId), soci::into(conferenceInfoOrganizerId);

	return session->got_data() ? conferenceInfoOrganizerId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectConferenceInfoParticipantId (long long conferenceInfoId, long long participantSipAddressId) const {
#ifdef HAVE_DB_STORAGE
	long long conferenceInfoParticipantId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectConferenceInfoParticipantId),
		soci::use(conferenceInfoId), soci::use(participantSipAddressId), soci::into(conferenceInfoParticipantId);

	return session->got_data() ? conferenceInfoParticipantId : -1;
#else
	return -1;
#endif
}

long long MainDbPrivate::selectConferenceCallId (const std::string &callId) {
#ifdef HAVE_DB_STORAGE
	long long conferenceCallId;

	soci::session *session = dbSession.getBackendSession();
	*session << Statements::get(Statements::SelectConferenceCall),
		soci::use(callId), soci::into(conferenceCallId);

	return session->got_data() ? conferenceCallId : -1;
#else
	return -1;
#endif
}

// -----------------------------------------------------------------------------

void MainDbPrivate::deleteContents (long long chatMessageId) {
#ifdef HAVE_DB_STORAGE
	*dbSession.getBackendSession() << "DELETE FROM chat_message_content WHERE event_id = :chatMessageId",
		soci::use(chatMessageId);
#endif
}

void MainDbPrivate::deleteChatRoomParticipant (long long chatRoomId, long long participantSipAddressId) {
#ifdef HAVE_DB_STORAGE
	*dbSession.getBackendSession() << "DELETE FROM chat_room_participant"
		" WHERE chat_room_id = :chatRoomId AND participant_sip_address_id = :participantSipAddressId",
		soci::use(chatRoomId), soci::use(participantSipAddressId);
#endif
}

void MainDbPrivate::deleteChatRoomParticipantDevice (
	long long participantId,
	long long participantDeviceSipAddressId
) {
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
shared_ptr<EventLog> MainDbPrivate::selectGenericConferenceEvent (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	const soci::row &row
) const {
	L_ASSERT(chatRoom);
	EventLog::Type type = EventLog::Type(row.get<int>(1));
	if (type == EventLog::Type::ConferenceChatMessage) {
		long long eventId = getConferenceEventIdFromRow(row);
		shared_ptr<EventLog> eventLog = getEventFromCache(eventId);
		if (!eventLog) {
			eventLog = selectConferenceChatMessageEvent(chatRoom, type, row);
			if (eventLog)
				cache(eventLog, eventId);
		}
		return eventLog;
	}

	return selectConferenceInfoEvent(chatRoom->getConferenceId(), row);
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceInfoEvent (
	const ConferenceId &conferenceId,
	const soci::row &row
) const {
	long long eventId = getConferenceEventIdFromRow(row);
	shared_ptr<EventLog> eventLog = getEventFromCache(eventId);
	if (eventLog)
		return eventLog;

	EventLog::Type type = EventLog::Type(row.get<int>(1));
	switch (type) {
		case EventLog::Type::None:
		case EventLog::Type::ConferenceChatMessage:
		case EventLog::Type::ConferenceCallStarted:
		case EventLog::Type::ConferenceCallConnected:
		case EventLog::Type::ConferenceCallEnded:
			return nullptr;

		case EventLog::Type::ConferenceCreated:
		case EventLog::Type::ConferenceTerminated:
			eventLog = selectConferenceEvent(conferenceId, type, row);
			break;

		case EventLog::Type::ConferenceParticipantAdded:
		case EventLog::Type::ConferenceParticipantRemoved:
		case EventLog::Type::ConferenceParticipantSetAdmin:
		case EventLog::Type::ConferenceParticipantUnsetAdmin:
			eventLog = selectConferenceParticipantEvent(conferenceId, type, row);
			break;

		case EventLog::Type::ConferenceParticipantDeviceAdded:
		case EventLog::Type::ConferenceParticipantDeviceRemoved:
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

	if (eventLog)
		cache(eventLog, eventId);

	return eventLog;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceEvent (
	const ConferenceId &conferenceId,
	EventLog::Type type,
	const soci::row &row
) const {
	return make_shared<ConferenceEvent>(
		type,
		getConferenceEventCreationTimeFromRow(row),
		conferenceId
	);
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceCallEvent (
	const soci::row &row
) const {
	L_Q();

	long long eventId = dbSession.resolveId(row, 0);
	shared_ptr<EventLog> eventLog = getEventFromCache(eventId);
	if (eventLog)
		return eventLog;

	EventLog::Type type = EventLog::Type(row.get<int>(1));

	long long conferenceCallId = dbSession.resolveId(row, 3);
	auto callLog = getCallLogFromCache(conferenceCallId);
	if (callLog == nullptr) {

		callLog = CallLog::create(q->getCore(), static_cast<LinphoneCallDir>(row.get<int>(6)), linphone_address_new(row.get<string>(4).c_str()), linphone_address_new(row.get<string>(5).c_str()));
		callLog->setDuration(row.get<int>(7));
		callLog->setStartTime(dbSession.getTime(row, 8));
		callLog->setConnectedTime(dbSession.getTime(row, 9));
		callLog->setStatus(static_cast<LinphoneCallStatus>(row.get<int>(10)));
		callLog->setVideoEnabled(!!row.get<int>(11));
		callLog->setQuality((float) row.get<double>(12));

		soci::indicator ind = row.get_indicator(13);
		if (ind == soci::i_ok) callLog->setCallId(row.get<string>(13));

		ind = row.get_indicator(14);
		if (ind == soci::i_ok) callLog->setRefKey(row.get<string>(14));

		cache(callLog, conferenceCallId);
	}

	std::shared_ptr<ConferenceInfo> conferenceInfo = nullptr;
	if (row.get_indicator(15) != soci::i_null) {
		long long conferenceInfoId = dbSession.resolveId(row, 15);
		callLog->setConferenceInfoId(conferenceInfoId);

		auto conferenceInfo = getConferenceInfoFromCache(conferenceInfoId);
		if (conferenceInfo == nullptr) {
			conferenceInfo = ConferenceInfo::create();

			IdentityAddress organizer = IdentityAddress(row.get<string>(16));
			conferenceInfo->setOrganizer(organizer);

			ConferenceAddress uri = ConferenceAddress(row.get<string>(17));
			conferenceInfo->setUri(uri);

			conferenceInfo->setDateTime(dbSession.getTime(row, 18));
			conferenceInfo->setDuration(dbSession.getUnsignedInt(row, 19, 0));
			conferenceInfo->setUtf8Subject(row.get<string>(20));
			conferenceInfo->setUtf8Description(row.get<string>(21));

			static const string query = "SELECT sip_address.value"
				" FROM sip_address, conference_info, conference_info_participant"
				" WHERE conference_info.id = :conferenceInfoId"
				" AND sip_address.id = conference_info_participant.participant_sip_address_id"
				" AND conference_info_participant.conference_info_id = conference_info.id";

			soci::session *session = dbSession.getBackendSession();
			soci::rowset<soci::row> participantRows = (session->prepare << query, soci::use(conferenceInfoId));
			for (const auto &participantRow : participantRows) {
				IdentityAddress participant(participantRow.get<string>(0));
				ConferenceInfo::participant_params_t participantParams;
				conferenceInfo->addParticipant(participant, participantParams);
			}

			cache(conferenceInfo, conferenceInfoId);
		}
	}

	eventLog = make_shared<ConferenceCallEvent>(
		type,
		dbSession.getTime(row, 2),
		callLog,
		conferenceInfo
	);

	cache(eventLog, eventId);

	return eventLog;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceChatMessageEvent (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	UNUSED(EventLog::Type type),
	const soci::row &row
) const {
	long long eventId = getConferenceEventIdFromRow(row);
	shared_ptr<ChatMessage> chatMessage = getChatMessageFromCache(eventId);
	if (!chatMessage) {
		chatMessage = shared_ptr<ChatMessage>(new ChatMessage(
			chatRoom,
			ChatMessage::Direction(row.get<int>(8))
		));
		chatMessage->setIsSecured(!!row.get<int>(9));

		ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
		ChatMessage::State messageState = ChatMessage::State(row.get<int>(7));
		// This is necessary if linphone has crashed while sending a message. It will set the correct state so the user can resend it.
		if (messageState == ChatMessage::State::Idle
			|| messageState == ChatMessage::State::InProgress
			|| messageState == ChatMessage::State::FileTransferInProgress) {
			messageState = ChatMessage::State::NotDelivered;
		}
		dChatMessage->forceState(messageState);

		dChatMessage->forceFromAddress(IdentityAddress(row.get<string>(3)));
		dChatMessage->forceToAddress(ConferenceAddress(row.get<string>(4)));

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
			dChatMessage->setReplyToMessageIdAndSenderAddress(row.get<string>(23), IdentityAddress(row.get<string>(24)));
		}

		cache(chatMessage, eventId);
	}

	return make_shared<ConferenceChatMessageEvent>(
		getConferenceEventCreationTimeFromRow(row),
		chatMessage
	);
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceParticipantEvent (
	const ConferenceId &conferenceId,
	EventLog::Type type,
	const soci::row &row
) const {

	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId);
	IdentityAddress participantAddress(IdentityAddress(row.get<string>(12)));

	std::shared_ptr<ConferenceParticipantEvent> event = make_shared<ConferenceParticipantEvent>(
		type,
		getConferenceEventCreationTimeFromRow(row),
		conferenceId,
		participantAddress
	);
	event->setNotifyId(getConferenceEventNotifyIdFromRow(row));
	return event;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceParticipantDeviceEvent (
	const ConferenceId &conferenceId,
	EventLog::Type type,
	const soci::row &row
) const {
	shared_ptr<AbstractChatRoom> chatRoom = findChatRoom(conferenceId);
	IdentityAddress participantAddress(IdentityAddress(row.get<string>(12)));
	IdentityAddress deviceAddress(IdentityAddress(row.get<string>(11)));

	shared_ptr<ConferenceParticipantDeviceEvent> event = make_shared<ConferenceParticipantDeviceEvent>(
		type,
		getConferenceEventCreationTimeFromRow(row),
		conferenceId,
		participantAddress,
		deviceAddress
	);
	event->setNotifyId(getConferenceEventNotifyIdFromRow(row));
	return event;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceSecurityEvent (
	const ConferenceId &conferenceId,
	UNUSED(EventLog::Type type),
	const soci::row &row
) const {
	return make_shared<ConferenceSecurityEvent>(
		getConferenceEventCreationTimeFromRow(row),
		conferenceId,
		static_cast<ConferenceSecurityEvent::SecurityEventType>(row.get<int>(16)),
		IdentityAddress(row.get<string>(17))
	);
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceEphemeralMessageEvent (
	const ConferenceId &conferenceId,
	EventLog::Type type,
	const soci::row &row
) const {
	return make_shared<ConferenceEphemeralMessageEvent>(
		type,
		getConferenceEventCreationTimeFromRow(row),
		conferenceId,
		(long)row.get<double>(22)
	);
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceAvailableMediaEvent (
	const ConferenceId &conferenceId,
	UNUSED(EventLog::Type type),
	const soci::row &row
) const {

	std::map<ConferenceMediaCapabilities, bool> mediaCapabilities;
	// TODO: choose rows
	mediaCapabilities[ConferenceMediaCapabilities::Audio] = false;
	mediaCapabilities[ConferenceMediaCapabilities::Video] = false;
	mediaCapabilities[ConferenceMediaCapabilities::Text] = false;
	shared_ptr<ConferenceAvailableMediaEvent> event = make_shared<ConferenceAvailableMediaEvent>(
		getConferenceEventCreationTimeFromRow(row),
		conferenceId,
		mediaCapabilities
	);
	event->setNotifyId(getConferenceEventNotifyIdFromRow(row));
	return event;
}

shared_ptr<EventLog> MainDbPrivate::selectConferenceSubjectEvent (
	const ConferenceId &conferenceId,
	UNUSED(EventLog::Type type),
	const soci::row &row
) const {
	shared_ptr<ConferenceSubjectEvent> event = make_shared<ConferenceSubjectEvent>(
		getConferenceEventCreationTimeFromRow(row),
		conferenceId,
		row.get<string>(13)
	);
	event->setNotifyId(getConferenceEventNotifyIdFromRow(row));
	return event;
}
#endif

// -----------------------------------------------------------------------------

long long MainDbPrivate::insertEvent (const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	const int &type = int(eventLog->getType());
	const tm &creationTime = Utils::getTimeTAsTm(eventLog->getCreationTime());
	*dbSession.getBackendSession() << "INSERT INTO event (type, creation_time) VALUES (:type, :creationTime)",
		soci::use(type), soci::use(creationTime);

	return dbSession.getLastInsertId();
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceEvent (const shared_ptr<EventLog> &eventLog, long long *chatRoomId) {
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
			" VALUES (:eventId, :chatRoomId)", soci::use(eventId), soci::use(curChatRoomId);

		if (eventLog->getType() == EventLog::Type::ConferenceTerminated)
			*session << "UPDATE chat_room SET flags = 1, last_notify_id = 0 WHERE id = :chatRoomId", soci::use(curChatRoomId);
		else if (eventLog->getType() == EventLog::Type::ConferenceCreated)
			*session << "UPDATE chat_room SET flags = 0 WHERE id = :chatRoomId", soci::use(curChatRoomId);
	}

	if (chatRoomId)
		*chatRoomId = curChatRoomId;

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceCallEvent (const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ConferenceCallEvent> conferenceCallEvent = static_pointer_cast<ConferenceCallEvent>(eventLog);

	long long eventId = -1;
	auto callLog = conferenceCallEvent->getCallLog();
	auto conferenceInfo = conferenceCallEvent->getConferenceInfo();
	long long conferenceCallId = selectConferenceCallId(callLog->getCallId());

	EventLog::Type type = conferenceCallEvent->getType();
	switch(type) {
		case EventLog::Type::ConferenceCallStarted:
			if (conferenceCallId >= 0) {
				lWarning() << "Cannot add ConferenceCallStarted event as conference call is already stored in db for call-id: " << callLog->getCallId();
				return -1;
			}
			break;
		case EventLog::Type::ConferenceCallConnected:
			if (conferenceCallId < 0) {
				lWarning() << "Adding ConferenceCallConnected event but conference call is not present in db for call-id: " << callLog->getCallId();
			}
			break;
		case EventLog::Type::ConferenceCallEnded:
			if (conferenceCallId < 0) {
				lWarning() << "Adding ConferenceCallEnded event but conference call is not present in db for call-id: " << callLog->getCallId();
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
		" VALUES (:eventId, :conferenceCallId)", soci::use(eventId), soci::use(conferenceCallId);

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceChatMessageEvent (const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	const long long &eventId = insertConferenceEvent(eventLog);
	if (eventId < 0)
		return -1;

	shared_ptr<ChatMessage> chatMessage = static_pointer_cast<ConferenceChatMessageEvent>(eventLog)->getChatMessage();
	const long long &fromSipAddressId = insertSipAddress(chatMessage->getFromAddress().asString());
	const long long &toSipAddressId = insertSipAddress(chatMessage->getToAddress().asString());
	const string &forwardInfo = chatMessage->getForwardInfo();
	const tm &messageTime = Utils::getTimeTAsTm(chatMessage->getTime());
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
	long long sipAddressId = 0;
	if (!replyMessageId.empty()) {
		sipAddressId = insertSipAddress(chatMessage->getReplyToSenderAddress().asString());
	}
	const long long &replyToSipAddressId = sipAddressId;

	*dbSession.getBackendSession() << "INSERT INTO conference_chat_message_event ("
		"  event_id, from_sip_address_id, to_sip_address_id,"
		"  time, state, direction, imdn_message_id, is_secured,"
		"  delivery_notification_required, display_notification_required,"
		"  marked_as_read, forward_info, call_id, reply_message_id, reply_sender_address_id"
		") VALUES ("
		"  :eventId, :localSipaddressId, :remoteSipaddressId,"
		"  :time, :state, :direction, :imdnMessageId, :isSecured,"
		"  :deliveryNotificationRequired, :displayNotificationRequired,"
		"  :markedAsRead, :forwardInfo, :callId, :replyMessageId, :replyToSipAddressId"
		")", soci::use(eventId), soci::use(fromSipAddressId), soci::use(toSipAddressId),
		soci::use(messageTime), soci::use(state), soci::use(direction),
		soci::use(imdnMessageId), soci::use(isSecured),
		soci::use(deliveryNotificationRequired), soci::use(displayNotificationRequired),
		soci::use(markedAsRead), soci::use(forwardInfo), soci::use(callId),
		soci::use(replyMessageId), soci::use(replyToSipAddressId);

	if (isEphemeral) {
		long ephemeralLifetime = chatMessage->getEphemeralLifetime();
		const tm &expireTime = Utils::getTimeTAsTm(chatMessage->getEphemeralExpireTime());
		*dbSession.getBackendSession() << "INSERT INTO chat_message_ephemeral_event ("
			"  event_id, ephemeral_lifetime,  expired_time"
			") VALUES ("
		"  :eventId, :ephemeralLifetime, :expireTime"
		")", soci::use(eventId), soci::use(ephemeralLifetime),  soci::use(expireTime);
	}

	for (const Content *content : chatMessage->getContents())
		insertContent(eventId, *content);

	shared_ptr<AbstractChatRoom> chatRoom(chatMessage->getChatRoom());
	for (const auto &participant : chatRoom->getParticipants()) {
		const long long &participantSipAddressId = selectSipAddressId(participant->getAddress().asString());
		insertChatMessageParticipant(eventId, participantSipAddressId, state, chatMessage->getTime());
	}

	const long long &dbChatRoomId = selectChatRoomId(chatRoom->getConferenceId());
	*dbSession.getBackendSession() << "UPDATE chat_room SET last_message_id = :1 WHERE id = :2", soci::use(eventId), soci::use(dbChatRoomId);

	if (direction == int(ChatMessage::Direction::Incoming) && !markedAsRead) {
		int *count = unreadChatMessageCountCache[chatRoom->getConferenceId()];
		if (count)
			++*count;
	}

	return eventId;
#else
	return -1;
#endif
}

void MainDbPrivate::updateConferenceChatMessageEvent (const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ChatMessage> chatMessage = static_pointer_cast<ConferenceChatMessageEvent>(eventLog)->getChatMessage();

	const EventLogPrivate *dEventLog = eventLog->getPrivate();
	MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
	const long long &eventId = dEventKey->storageId;

	// 1. Get current chat message state and database state.
	const ChatMessage::State state = chatMessage->getState();
	ChatMessage::State dbState;
	bool dbMarkedAsRead;
	{
		int intState;
		int intMarkedAsRead;
		*dbSession.getBackendSession() << "SELECT state, marked_as_read FROM conference_chat_message_event WHERE event_id = :eventId",
			soci::into(intState), soci::into(intMarkedAsRead), soci::use(eventId);
		dbState = ChatMessage::State(intState);
		dbMarkedAsRead = intMarkedAsRead == 1;
	}
	const bool markedAsRead = chatMessage->getPrivate()->isMarkedAsRead();

	// 2. Update unread chat message count if necessary.
	const bool isOutgoing = chatMessage->getDirection() == ChatMessage::Direction::Outgoing;
	shared_ptr<AbstractChatRoom> chatRoom(chatMessage->getChatRoom());
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
		const int stateInt = int(
			state == ChatMessage::State::InProgress ||
			state == ChatMessage::State::FileTransferDone ||
			state == ChatMessage::State::FileTransferInProgress ||
			state == ChatMessage::State::FileTransferError
				? dbState
				: state
		);
		const int markedAsReadInt = markedAsRead ? 1 : 0;

		*dbSession.getBackendSession() << "UPDATE conference_chat_message_event SET state = :state, imdn_message_id = :imdnMessageId, marked_as_read = :markedAsRead"
			" WHERE event_id = :eventId",
			soci::use(stateInt), soci::use(imdnMessageId), soci::use(markedAsReadInt), soci::use(eventId);
	}

	// 4. Update contents.
	deleteContents(eventId);
	for (const auto &content : chatMessage->getContents())
		insertContent(eventId, *content);

	// 5. Update participants.
	if (isOutgoing && (state == ChatMessage::State::Delivered || state == ChatMessage::State::NotDelivered))
		for (const auto &participant : chatRoom->getParticipants())
			setChatMessageParticipantState(eventLog, participant->getAddress(), state, std::time(nullptr));
#endif
}

long long MainDbPrivate::insertConferenceNotifiedEvent (const shared_ptr<EventLog> &eventLog, long long *chatRoomId) {
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
		" VALUES (:eventId, :notifyId)", soci::use(eventId), soci::use(lastNotifyId);
	*session << "UPDATE chat_room SET last_notify_id = :lastNotifyId WHERE id = :chatRoomId",
		soci::use(lastNotifyId), soci::use(curChatRoomId);

	if (chatRoomId)
		*chatRoomId = curChatRoomId;

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceParticipantEvent (
	const shared_ptr<EventLog> &eventLog,
	long long *chatRoomId,
	bool executeAction
) {
#ifdef HAVE_DB_STORAGE
	long long curChatRoomId;
	const long long &eventId = insertConferenceNotifiedEvent(eventLog, &curChatRoomId);
	if (eventId < 0)
		return -1;

	shared_ptr<ConferenceParticipantEvent> participantEvent =
		static_pointer_cast<ConferenceParticipantEvent>(eventLog);

	const long long &participantAddressId = insertSipAddress(
		participantEvent->getParticipantAddress().asString()
	);

	*dbSession.getBackendSession() << "INSERT INTO conference_participant_event (event_id, participant_sip_address_id)"
		" VALUES (:eventId, :participantAddressId)", soci::use(eventId), soci::use(participantAddressId);

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

	if (chatRoomId)
		*chatRoomId = curChatRoomId;

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceParticipantDeviceEvent (const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceParticipantEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert participant device event of type " << eventLog->getType() << " in database.";
		return -1;
	}

	shared_ptr<ConferenceParticipantDeviceEvent> participantDeviceEvent =
		static_pointer_cast<ConferenceParticipantDeviceEvent>(eventLog);

	const string participantAddress = participantDeviceEvent->getParticipantAddress().asString();
	const long long &participantAddressId = selectSipAddressId(participantAddress);
	if (participantAddressId < 0) {
		lError() << "Unable to find sip address id of: `" << participantAddress << "`.";
		return -1;
	}
	const long long &participantId = selectChatRoomParticipantId(chatRoomId, participantAddressId);
	if (participantId < 0) {
		lError() << "Unable to find valid participant id in database with chat room id = " << chatRoomId <<
		" and participant address id = " << participantAddressId;
		return -1;
	}
	const long long &deviceAddressId = insertSipAddress(
		participantDeviceEvent->getDeviceAddress().asString()
	);

	*dbSession.getBackendSession() << "INSERT INTO conference_participant_device_event (event_id, device_sip_address_id)"
		" VALUES (:eventId, :deviceAddressId)", soci::use(eventId), soci::use(deviceAddressId);

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

long long MainDbPrivate::insertConferenceSecurityEvent (const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert security event of type " << eventLog->getType() << " in database.";
		return -1;
	}

	const int &securityEventType = int(static_pointer_cast<ConferenceSecurityEvent>(eventLog)->getSecurityEventType());
	const string &faultyDevice = static_pointer_cast<ConferenceSecurityEvent>(eventLog)->getFaultyDeviceAddress().asString();

	// insert security event into new table "conference_security_event"
	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_security_event (event_id, security_alert, faulty_device)"
		" VALUES (:eventId, :securityEventType, :faultyDevice)", soci::use(eventId), soci::use(securityEventType), soci::use(faultyDevice);

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceAvailableMediaEvent (const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceNotifiedEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert conference available media event of type " << eventLog->getType() << " in database.";
		return -1;
	}

	const int audio = static_pointer_cast<ConferenceAvailableMediaEvent>(eventLog)->audioEnabled() ? 1 : 0;
	const int video = static_pointer_cast<ConferenceAvailableMediaEvent>(eventLog)->videoEnabled() ? 1 : 0;
	const int chat = static_pointer_cast<ConferenceAvailableMediaEvent>(eventLog)->chatEnabled() ? 1 : 0;

	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_available_media_event (event_id, audio, video, chat)"
		" VALUES (:eventId, :audio, :video, :chat)", soci::use(eventId), soci::use(audio), soci::use(video), soci::use(chat);

	*session << "UPDATE chat_room SET audio = :audio, video = :video, chat = :chat"
		" WHERE id = :chatRoomId", soci::use(audio), soci::use(video), soci::use(chat), soci::use(chatRoomId);

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceSubjectEvent (const shared_ptr<EventLog> &eventLog) {
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
		" VALUES (:eventId, :subject)", soci::use(eventId), soci::use(subject);

	*session << "UPDATE chat_room SET subject = :subject"
		" WHERE id = :chatRoomId", soci::use(subject), soci::use(chatRoomId);

	return eventId;
#else
	return -1;
#endif
}

long long MainDbPrivate::insertConferenceEphemeralMessageEvent (const shared_ptr<EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	long long chatRoomId;
	const long long &eventId = insertConferenceNotifiedEvent(eventLog, &chatRoomId);
	if (eventId < 0) {
		lError() << "Unable to insert conference ephemeral message event of type " << eventLog->getType() << " in database.";
		return -1;
	}

	long lifetime = static_pointer_cast<ConferenceEphemeralMessageEvent>(eventLog)->getEphemeralMessageLifetime();

	soci::session *session = dbSession.getBackendSession();
	*session << "INSERT INTO conference_ephemeral_message_event (event_id, lifetime)"
	" VALUES (:eventId, :lifetime)", soci::use(eventId), soci::use(lifetime);

	return eventId;
#else
	return -1;
#endif
}

void MainDbPrivate::setChatMessageParticipantState (
	const shared_ptr<EventLog> &eventLog,
	const IdentityAddress &participantAddress,
	ChatMessage::State state,
	time_t stateChangeTime
) {
#ifdef HAVE_DB_STORAGE
	const EventLogPrivate *dEventLog = eventLog->getPrivate();
	MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
	const long long &eventId = dEventKey->storageId;
	const long long &participantSipAddressId = selectSipAddressId(participantAddress.asString());

	/* setChatMessageParticipantState can be called by updateConferenceChatMessageEvent, which try to update participant state
	 by message state. However, we can not change state Displayed/DeliveredToUser to Delivered/NotDelivered. */
	int intState;
	*dbSession.getBackendSession() << "SELECT state FROM chat_message_participant WHERE event_id = :eventId AND participant_sip_address_id = :participantSipAddressId",
		soci::into(intState),  soci::use(eventId), soci::use(participantSipAddressId);
	ChatMessage::State dbState = ChatMessage::State(intState);

	if (int(state) < intState && (dbState == ChatMessage::State::Displayed || dbState == ChatMessage::State::DeliveredToUser)) {
		lInfo() << "setChatMessageParticipantState: can not change state from " << dbState << " to " << state;
		return;
	}


	int stateInt = int(state);
	const tm &stateChangeTm = Utils::getTimeTAsTm(stateChangeTime);

	*dbSession.getBackendSession() << "UPDATE chat_message_participant SET state = :state,"
		" state_change_time = :stateChangeTm"
		" WHERE event_id = :eventId AND participant_sip_address_id = :participantSipAddressId",
		soci::use(stateInt), soci::use(stateChangeTm), soci::use(eventId), soci::use(participantSipAddressId);
#endif
}

// ---------------------------------------------------------------------------
// Call log API.
// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
std::shared_ptr<CallLog> MainDbPrivate::selectCallLog (const soci::row &row) const {
	L_Q();

	const long long &dbCallLogId = dbSession.resolveId(row, 0);

	auto callLog = getCallLogFromCache(dbCallLogId);
	if (callLog) return callLog;

	LinphoneAddress *from = linphone_address_new(row.get<string>(1).c_str());
	if (row.get_indicator(2) == soci::i_ok) linphone_address_set_display_name(from, row.get<string>(2).c_str());

	LinphoneAddress *to = linphone_address_new(row.get<string>(3).c_str());
	if (row.get_indicator(4) == soci::i_ok) linphone_address_set_display_name(to, row.get<string>(4).c_str());

	callLog = CallLog::create(q->getCore(),
		static_cast<LinphoneCallDir>(row.get<int>(5)),
		from,
		to);

	callLog->setDuration(row.get<int>(6));
	callLog->setStartTime(dbSession.getTime(row, 7));
	callLog->setConnectedTime(dbSession.getTime(row, 8));
	callLog->setStatus(static_cast<LinphoneCallStatus>(row.get<int>(9)));
	callLog->setVideoEnabled(!!row.get<int>(10));
	callLog->setQuality((float) row.get<double>(11));

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

#ifdef HAVE_DB_STORAGE
shared_ptr<ConferenceInfo> MainDbPrivate::selectConferenceInfo (const soci::row &row) const {
	const long long &dbConferenceInfoId = dbSession.resolveId(row, 0);

	auto conferenceInfo = getConferenceInfoFromCache(dbConferenceInfoId);
	if (conferenceInfo) return conferenceInfo;

	conferenceInfo = ConferenceInfo::create();
	ConferenceAddress uri(row.get<string>(2));
	conferenceInfo->setUri(uri);

	conferenceInfo->setDateTime(dbSession.getTime(row, 3));
	conferenceInfo->setDuration(dbSession.getUnsignedInt(row, 4, 0));
	conferenceInfo->setUtf8Subject(row.get<string>(5));
	conferenceInfo->setUtf8Description(row.get<string>(6));
	conferenceInfo->setState(ConferenceInfo::State(row.get<int>(7))); // state is a TinyInt in database, don't cast it to unsigned, otherwise you'll get a std::bad_cast from soci.
	unsigned int icsSequence = dbSession.getUnsignedInt(row,8,0);
	conferenceInfo->setIcsSequence(icsSequence);

	// For backward compability purposes, get the organizer from conference_info table and set the sequence number to that of the conference info stored in the db
	// It may be overridden if the conference organizer has been stored in table conference_info_organizer.
	IdentityAddress organizer(row.get<string>(1));
	ConferenceInfo::participant_params_t defaultOrganizerParams;
	defaultOrganizerParams.insert(std::make_pair(ConferenceInfo::sequenceParam, std::to_string(icsSequence)));
	conferenceInfo->setOrganizer(organizer, defaultOrganizerParams);
	conferenceInfo->setIcsUid(row.get<string>(9));

	static const string participantQuery = "SELECT sip_address.value, conference_info_participant.deleted, conference_info_participant.params"
		" FROM sip_address, conference_info, conference_info_participant"
		" WHERE conference_info.id = :conferenceInfoId"
		" AND sip_address.id = conference_info_participant.participant_sip_address_id"
		" AND conference_info_participant.conference_info_id = conference_info.id";

	soci::session *session = dbSession.getBackendSession();
	soci::rowset<soci::row> participantRows = (session->prepare << participantQuery, soci::use(dbConferenceInfoId));
	for (const auto &participantRow : participantRows) {
		int deleted = participantRow.get<int>(1);
		if (deleted == 0) {
			IdentityAddress participantAddress(participantRow.get<string>(0));
			const string participantParamsStr = participantRow.get<string>(2);
			ConferenceInfo::participant_params_t participantParams = ConferenceInfo::stringToMemberParameters(participantParamsStr);
			conferenceInfo->addParticipant(participantAddress, participantParams);
		}
	}

	static const string organizerQuery = "SELECT sip_address.value, conference_info_organizer.params"
		" FROM sip_address, conference_info, conference_info_organizer"
		" WHERE conference_info.id = :conferenceInfoId"
		" AND sip_address.id = conference_info_organizer.organizer_sip_address_id"
		" AND conference_info_organizer.conference_info_id = conference_info.id";

	soci::rowset<soci::row> organizerRows = (session->prepare << organizerQuery, soci::use(dbConferenceInfoId));
	for (const auto &organizerRow : organizerRows) {
		IdentityAddress organizerAddress(organizerRow.get<string>(0));
		const string organizerParamsStr = organizerRow.get<string>(1);
		ConferenceInfo::participant_params_t organizerParams = ConferenceInfo::stringToMemberParameters(organizerParamsStr);
		conferenceInfo->setOrganizer(organizerAddress, organizerParams);
	}

	cache(conferenceInfo, dbConferenceInfoId);

	return conferenceInfo;
}
#endif

// -----------------------------------------------------------------------------
// Cache API.
// -----------------------------------------------------------------------------

shared_ptr<EventLog> MainDbPrivate::getEventFromCache (long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToEvent.find(storageId);
	if (it == storageIdToEvent.cend())
		return nullptr;

	shared_ptr<EventLog> eventLog = it->second.lock();
	L_ASSERT(eventLog);
	return eventLog;
#else
	return nullptr;
#endif
}

shared_ptr<ChatMessage> MainDbPrivate::getChatMessageFromCache (long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToChatMessage.find(storageId);
	if (it == storageIdToChatMessage.cend())
		return nullptr;

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
	if (it == storageIdToConferenceId.cend())
		return ConferenceId();

	ConferenceId conferenceId = it->second;
	return conferenceId;
#else
	return ConferenceId();
#endif
}

shared_ptr<CallLog> MainDbPrivate::getCallLogFromCache (long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToCallLog.find(storageId);
	if (it == storageIdToCallLog.cend())
		return nullptr;

	shared_ptr<CallLog> callLog = it->second.lock();
	return callLog;
#else
	return nullptr;
#endif
}

shared_ptr<ConferenceInfo> MainDbPrivate::getConferenceInfoFromCache (long long storageId) const {
#ifdef HAVE_DB_STORAGE
	auto it = storageIdToConferenceInfo.find(storageId);
	if (it == storageIdToConferenceInfo.cend())
		return nullptr;

	shared_ptr<ConferenceInfo> conferenceInfo = it->second.lock();
	return conferenceInfo;
#else
	return nullptr;
#endif
}

void MainDbPrivate::cache (const shared_ptr<EventLog> &eventLog, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	L_Q();

	EventLogPrivate *dEventLog = eventLog->getPrivate();
	L_ASSERT(!dEventLog->dbKey.isValid());
	dEventLog->dbKey = MainDbEventKey(q->getCore(), storageId);
	storageIdToEvent[storageId] = eventLog;
	L_ASSERT(dEventLog->dbKey.isValid());
#endif
}

void MainDbPrivate::cache (const shared_ptr<ChatMessage> &chatMessage, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	L_ASSERT(!chatMessage->isValid());
	dChatMessage->setStorageId(storageId);
	storageIdToChatMessage[storageId] = chatMessage;
	L_ASSERT(chatMessage->isValid());
#endif
}

void MainDbPrivate::cache (const ConferenceId &conferenceId, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	L_ASSERT(conferenceId.isValid());
	storageIdToConferenceId[storageId] = conferenceId;
#endif
}

void MainDbPrivate::cache (const std::shared_ptr<CallLog> &callLog, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	storageIdToCallLog[storageId] = callLog;
#endif
}

void MainDbPrivate::cache (const std::shared_ptr<ConferenceInfo> &conferenceInfo, long long storageId) const {
#ifdef HAVE_DB_STORAGE
	storageIdToConferenceInfo[storageId] = conferenceInfo;
#endif
}

void MainDbPrivate::invalidConferenceEventsFromQuery (const string &query, long long chatRoomId) {
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

unsigned int MainDbPrivate::getModuleVersion (const string &name) {
#ifdef HAVE_DB_STORAGE
	soci::session *session = dbSession.getBackendSession();

	unsigned int version;
	*session << "SELECT version FROM db_module_version WHERE name = :name", soci::into(version), soci::use(name);
	return session->got_data() ? version : 0;
#else
	return 0;
#endif
}

void MainDbPrivate::updateModuleVersion (const string &name, unsigned int version) {
#ifdef HAVE_DB_STORAGE
	unsigned int oldVersion = getModuleVersion(name);
	if (version <= oldVersion)
		return;

	soci::session *session = dbSession.getBackendSession();
	*session << "REPLACE INTO db_module_version (name, version) VALUES (:name, :version)",
		soci::use(name), soci::use(version);
#endif
}

void MainDbPrivate::updateSchema () {
#ifdef HAVE_DB_STORAGE
	L_Q();

	//MySQL : Modified display_name in order to set explicitely this column to utf8mb4, while the default character set of the table is set to ascii (this allows special characters in display name without breaking compatibility with mysql 5.5)
	//191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4
	MainDb::Backend backend = q->getBackend();
	const string charset = backend == MainDb::Backend::Mysql ? "DEFAULT CHARSET=utf8mb4" : "";

	soci::session *session = dbSession.getBackendSession();
	unsigned int version = getModuleVersion("events");

	if (version < makeVersion(1, 0, 1))
		*session << "ALTER TABLE chat_room_participant_device ADD COLUMN state TINYINT UNSIGNED DEFAULT 0";
	if (version < makeVersion(1, 0, 2)) {
		*session << "DROP TRIGGER IF EXISTS chat_message_participant_deleter";
		*session << "ALTER TABLE chat_message_participant ADD COLUMN state_change_time"
			+ dbSession.timestampType() + " NOT NULL DEFAULT " + dbSession.currentTimestamp();
	}
	if (version < makeVersion(1, 0, 3)) {
		// Remove client group one-to-one chat rooms for the moment as there are still some issues
		// with them and we prefer to keep using basic chat rooms instead
		const int &capabilities = ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::Conference)
			| ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::OneToOne);
		*session << "DELETE FROM chat_room WHERE (capabilities & :capabilities1) = :capabilities2",
			soci::use(capabilities), soci::use(capabilities);
		linphone_config_set_bool(linphone_core_get_config(q->getCore()->getCCore()), "misc", "prefer_basic_chat_room", TRUE);
	}
	if (version < makeVersion(1, 0, 4)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN delivery_notification_required BOOLEAN NOT NULL DEFAULT 0";
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN display_notification_required BOOLEAN NOT NULL DEFAULT 0";
	}
	if (version < makeVersion(1, 0, 5)) {
		const string queryDelivery = "UPDATE conference_chat_message_event"
			"  SET delivery_notification_required = 0"
			"  WHERE direction = " + Utils::toString(int(ChatMessage::Direction::Incoming)) +
			"  AND state = " + Utils::toString(int(ChatMessage::State::Delivered));

		*session << queryDelivery;

		const string queryDisplay = "UPDATE conference_chat_message_event"
			"  SET delivery_notification_required = 0, display_notification_required = 0"
			"  WHERE direction = " + Utils::toString(int(ChatMessage::Direction::Incoming)) +
			"  AND state = " + Utils::toString(int(ChatMessage::State::Displayed));

		*session << queryDisplay;
	}
	if (version < makeVersion(1, 0, 6)) {
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
			"  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, participant_sip_address_id, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device"
			"  FROM event"
			"  LEFT JOIN conference_event ON conference_event.event_id = event.id"
			"  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
			"  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
			"  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = event.id"
			"  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
			"  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
			"  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id";
	}
	if (version < makeVersion(1, 0, 6)
		&& linphone_config_get_bool(linphone_core_get_config(q->getCore()->getCCore()), "lime", "migrate_to_secured_room",FALSE)) {
		*session << "UPDATE chat_room "
		"SET capabilities = capabilities | " +  Utils::toString(int(ChatRoom::Capabilities::Encrypted));
	}

	if (version < makeVersion(1, 0, 7)) {
		*session << "ALTER TABLE chat_room_participant_device ADD COLUMN name VARCHAR(255)";
	}

	if (version < makeVersion(1, 0, 8)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN marked_as_read BOOLEAN NOT NULL DEFAULT 1";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
			"  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, participant_sip_address_id, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read"
			"  FROM event"
			"  LEFT JOIN conference_event ON conference_event.event_id = event.id"
			"  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
			"  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
			"  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = event.id"
			"  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
			"  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
			"  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id";
	}

	if (version < makeVersion(1, 0, 9)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN forward_info VARCHAR(255) NOT NULL DEFAULT ''";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		"  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, participant_sip_address_id, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read, forward_info"
		"  FROM event"
		"  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		"  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		"  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		"  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = event.id"
		"  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		"  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		"  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id";
	}

	if (version < makeVersion(1, 0, 10)) {
		*session << "CREATE INDEX incoming_not_delivered_index ON conference_chat_message_event (delivery_notification_required, direction)";
		*session << "CREATE INDEX unread_index ON conference_chat_message_event (marked_as_read)";
		*session << "CREATE VIEW conference_event_simple_view AS SELECT id, type, chat_room_id FROM event LEFT JOIN conference_event ON conference_event.event_id = event.id LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id";
	}

	if (version < makeVersion(1, 0, 11)) {
		*session << "ALTER TABLE chat_room ADD COLUMN last_message_id " + dbSession.primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL DEFAULT 0";
		*session << "UPDATE chat_room SET last_message_id = IFNULL((SELECT id FROM conference_event_simple_view WHERE chat_room_id = chat_room.id AND type = 5 ORDER BY id DESC LIMIT 1), 0)";
	}

	if (version < makeVersion(1, 0, 12)) {
		*session << "ALTER TABLE chat_room ADD COLUMN ephemeral_enabled BOOLEAN NOT NULL DEFAULT 0";
		*session << "ALTER TABLE chat_room ADD COLUMN ephemeral_messages_lifetime DOUBLE NOT NULL DEFAULT 86400";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		"  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, participant_sip_address_id, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime"
		"  FROM event"
		"  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		"  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		"  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		"  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = event.id"
		"  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		"  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		"  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id"
		"  LEFT JOIN chat_message_ephemeral_event ON chat_message_ephemeral_event.event_id = event.id"
		"  LEFT JOIN conference_ephemeral_message_event ON conference_ephemeral_message_event.event_id = event.id";
	}

	if (version < makeVersion(1, 0, 13)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN call_id VARCHAR(255) DEFAULT ''";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		"  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, participant_sip_address_id, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, call_id"
		"  FROM event"
		"  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		"  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		"  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		"  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = event.id"
		"  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		"  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		"  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id"
		"  LEFT JOIN chat_message_ephemeral_event ON chat_message_ephemeral_event.event_id = event.id"
		"  LEFT JOIN conference_ephemeral_message_event ON conference_ephemeral_message_event.event_id = event.id";
	}

	if (version < makeVersion(1, 0, 14)) {
		*session << "ALTER TABLE chat_message_content ADD COLUMN body_encoding_type TINYINT NOT NULL DEFAULT 0";// Older table contains Local encoding.
	}

	if (version < makeVersion(1, 0, 15)) {
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN reply_message_id VARCHAR(255) DEFAULT ''";
		*session << "ALTER TABLE conference_chat_message_event ADD COLUMN reply_sender_address_id " + dbSession.primaryKeyRefStr("BIGINT UNSIGNED") + " DEFAULT 0";
		*session << "DROP VIEW IF EXISTS conference_event_view";
		*session << "CREATE VIEW conference_event_view AS"
		"  SELECT id, type, creation_time, chat_room_id, from_sip_address_id, to_sip_address_id, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address_id, participant_sip_address_id, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, call_id, reply_message_id, reply_sender_address_id"
		"  FROM event"
		"  LEFT JOIN conference_event ON conference_event.event_id = event.id"
		"  LEFT JOIN conference_chat_message_event ON conference_chat_message_event.event_id = event.id"
		"  LEFT JOIN conference_notified_event ON conference_notified_event.event_id = event.id"
		"  LEFT JOIN conference_participant_device_event ON conference_participant_device_event.event_id = event.id"
		"  LEFT JOIN conference_participant_event ON conference_participant_event.event_id = event.id"
		"  LEFT JOIN conference_subject_event ON conference_subject_event.event_id = event.id"
		"  LEFT JOIN conference_security_event ON conference_security_event.event_id = event.id"
		"  LEFT JOIN chat_message_ephemeral_event ON chat_message_ephemeral_event.event_id = event.id"
		"  LEFT JOIN conference_ephemeral_message_event ON conference_ephemeral_message_event.event_id = event.id";
	}

	if (version < makeVersion(1, 0, 16)) {
		*session << "ALTER TABLE chat_message_file_content ADD COLUMN duration INT NOT NULL DEFAULT -1";
	}

	if (version < makeVersion(1, 0, 17)) {
		*session << "ALTER TABLE sip_address ADD COLUMN display_name VARCHAR(255)";
	}

	if (version < makeVersion(1, 0, 18)) {
		//We assume that the following statement is supported on all non-sqlite backends
		if (backend != MainDb::Backend::Sqlite3){
			*session << "ALTER TABLE sip_address MODIFY COLUMN display_name VARCHAR(191) CHARACTER SET utf8mb4";
		}
		//In sqlite, there is no specific size limit to indexable columns, and text columns are stored in UTF-8 by default.
		//Given this, we assume that if "ALTER TABLE sip_address ADD COLUMN display_name VARCHAR(255)" was run previously on Sqlite there is no need to alter the table to reduce the size (which would need to recreate the whole column anyway as "MODIFY COLUMN" isn't supported in sqlite)

	}

	if (version < makeVersion(1, 0, 19)) {
		*session << "ALTER TABLE conference_info ADD COLUMN state TINYINT UNSIGNED NOT NULL DEFAULT 0";
		*session << "ALTER TABLE conference_info ADD COLUMN ics_sequence INT UNSIGNED DEFAULT 0";
		*session << "ALTER TABLE conference_info ADD COLUMN ics_uid VARCHAR(2048) DEFAULT ''";
	}

	if (version < makeVersion(1, 0, 20)) {
		*session << "ALTER TABLE conference_info_participant ADD COLUMN deleted BOOLEAN NOT NULL DEFAULT 0";
		*session << "ALTER TABLE conference_info_participant ADD COLUMN params VARCHAR(2048) DEFAULT ''";
	}

	// /!\ Warning : if varchar columns < 255 were to be indexed, their size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in column creation)

#endif
}

// -----------------------------------------------------------------------------
// Import.
// -----------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
static inline bool checkLegacyTableExists (soci::session &session, const string &name) {
	session << "SELECT name FROM sqlite_master WHERE type='table' AND name = :name", soci::use(name);
	return session.got_data();
}

static inline bool checkLegacyFriendsTableExists (soci::session &session) {
	return checkLegacyTableExists(session, "friends");
}

static inline bool checkLegacyHistoryTableExists (soci::session &session) {
	return checkLegacyTableExists(session, "history");
}

static inline bool checkLegacyCallLogsTableExists (soci::session &session) {
	return checkLegacyTableExists(session, "call_history");
}
#endif

#ifdef HAVE_DB_STORAGE
void MainDbPrivate::importLegacyFriends (DbSession &inDbSession) {
	L_Q();
	L_DB_TRANSACTION_C(q) {
		if (getModuleVersion("legacy-friends-import") >= makeVersion(1, 0, 0))
			return;
		updateModuleVersion("legacy-friends-import", ModuleVersionLegacyFriendsImport);

		soci::session *inSession = inDbSession.getBackendSession();
		if (!checkLegacyFriendsTableExists(*inSession))
			return;

		unordered_map<int, long long> resolvedListsIds;
		soci::session *session = dbSession.getBackendSession();

		soci::rowset<soci::row> friendsLists = (inSession->prepare << "SELECT * FROM friends_lists");

		set<string> names;
		for (const auto &friendList : friendsLists) {
			const string &name = friendList.get<string>(LegacyFriendListColName, "");
			const string &rlsUri = friendList.get<string>(LegacyFriendListColRlsUri, "");
			const string &syncUri = friendList.get<string>(LegacyFriendListColSyncUri, "");
			const int &revision = friendList.get<int>(LegacyFriendListColRevision, 0);

			string uniqueName = name;
			for (int id = 0; names.find(uniqueName) != names.end(); uniqueName = name + "-" + Utils::toString(id++));
			names.insert(uniqueName);

			*session << "INSERT INTO friends_list (name, rls_uri, sync_uri, revision) VALUES ("
				"  :name, :rlsUri, :syncUri, :revision"
				")", soci::use(uniqueName), soci::use(rlsUri), soci::use(syncUri), soci::use(revision);
			resolvedListsIds[friendList.get<int>(LegacyFriendListColId)] = dbSession.getLastInsertId();
		}

		soci::rowset<soci::row> friends = (inSession->prepare << "SELECT * FROM friends");
		for (const auto &friendInfo : friends) {
			long long friendsListId;
			{
				auto it = resolvedListsIds.find(friendInfo.get<int>(LegacyFriendColFriendListId, -1));
				if (it == resolvedListsIds.end())
					continue;
				friendsListId = it->second;
			}

			const long long &sipAddressId = insertSipAddress(friendInfo.get<string>(LegacyFriendColSipAddress, ""));
			const int &subscribePolicy = friendInfo.get<int>(LegacyFriendColSubscribePolicy, LinphoneSPAccept);
			const int &sendSubscribe = friendInfo.get<int>(LegacyFriendColSendSubscribe, 1);
			const string &vCard = friendInfo.get<string>(LegacyFriendColVCard, "");
			const string &vCardEtag = friendInfo.get<string>(LegacyFriendColVCardEtag, "");
			const string &vCardSyncUri = friendInfo.get<string>(LegacyFriendColVCardSyncUri, "");
			const int &presenceReveived = friendInfo.get<int>(LegacyFriendColPresenceReceived, 0);

			*session << "INSERT INTO friend ("
				"  sip_address_id, friends_list_id, subscribe_policy, send_subscribe,"
				"  presence_received, v_card, v_card_etag, v_card_sync_uri"
				") VALUES ("
				"  :sipAddressId, :friendsListId, :subscribePolicy, :sendSubscribe,"
				"  :presenceReceived, :vCard, :vCardEtag, :vCardSyncUri"
				")", soci::use(sipAddressId), soci::use(friendsListId), soci::use(subscribePolicy), soci::use(sendSubscribe),
				soci::use(presenceReveived), soci::use(vCard), soci::use(vCardEtag), soci::use(vCardSyncUri);

			bool isNull;
			const string &data = getValueFromRow<string>(friendInfo, LegacyFriendColRefKey, isNull);
			if (!isNull)
				*session << "INSERT INTO friend_app_data (friend_id, name, data) VALUES"
					" (:friendId, 'legacy', :data)",
					soci::use(dbSession.getLastInsertId()), soci::use(data);
		}
		tr.commit();
		lInfo() << "Successful import of legacy friends.";
	};
}

// TODO: Move in a helper file? With others xml.
struct XmlCharObjectDeleter {
	void operator() (void *ptr) const {
		xmlFree(ptr);
	}
};
using XmlCharObject = unique_ptr<xmlChar, XmlCharObjectDeleter>;

struct XmlDocObjectDeleter {
	void operator() (xmlDocPtr ptr) const {
		xmlFreeDoc(ptr);
	}
};
using XmlDocObject = unique_ptr<remove_pointer<xmlDocPtr>::type, XmlDocObjectDeleter>;

typedef const xmlChar* XmlCharPtr;

static string extractLegacyFileContentType (const string &xml) {
	XmlDocObject xmlMessageBody(xmlParseDoc(XmlCharPtr(xml.c_str())));
	xmlNodePtr xmlElement = xmlDocGetRootElement(xmlMessageBody.get());
	if (!xmlElement)
		return "";

	for (xmlElement = xmlElement->xmlChildrenNode; xmlElement; xmlElement = xmlElement->next) {
		if (xmlStrcmp(xmlElement->name, XmlCharPtr("file-info")))
			continue;

		XmlCharObject typeAttribute(xmlGetProp(xmlElement, XmlCharPtr("type")));
		if (xmlStrcmp(typeAttribute.get(), XmlCharPtr("file")))
			continue;

		for (xmlElement = xmlElement->xmlChildrenNode; xmlElement; xmlElement = xmlElement->next)
			if (!xmlStrcmp(xmlElement->name, XmlCharPtr("content-type"))) {
				XmlCharObject xmlContentType(xmlNodeListGetString(xmlMessageBody.get(), xmlElement->xmlChildrenNode, 1));
				return ContentType(reinterpret_cast<const char *>(xmlContentType.get())).getMediaType();
			}
	}

	return "";
}

void MainDbPrivate::importLegacyHistory (DbSession &inDbSession) {
	L_Q();
	L_DB_TRANSACTION_C(q) {
		if (getModuleVersion("legacy-history-import") >= makeVersion(1, 0, 0))
			return;
		updateModuleVersion("legacy-history-import", ModuleVersionLegacyHistoryImport);

		soci::session *inSession = inDbSession.getBackendSession();
		if (!checkLegacyHistoryTableExists(*inSession))
			return;

		soci::rowset<soci::row> messages = (inSession->prepare << "SELECT * FROM history");
		for (const auto &message : messages) {
			const int direction = message.get<int>(LegacyMessageColDirection);
			if (direction != 0 && direction != 1) {
				lWarning() << "Unable to import legacy message with invalid direction.";
				continue;
			}

			const int &state = message.get<int>(
				LegacyMessageColState, int(ChatMessage::State::Displayed)
			);
			if (state < 0 || state > int(ChatMessage::State::Displayed)) {
				lWarning() << "Unable to import legacy message with invalid state.";
				continue;
			}

			const tm &creationTime = Utils::getTimeTAsTm(message.get<int>(LegacyMessageColDate, 0));

			bool isNull;
			getValueFromRow<string>(message, LegacyMessageColUrl, isNull);

			const int &contentId = message.get<int>(LegacyMessageColContentId, -1);
			ContentType contentType(message.get<string>(LegacyMessageColContentType, ""));
			if (!contentType.isValid())
				contentType = contentId != -1
					? ContentType::FileTransfer
					: (isNull ? ContentType::PlainText : ContentType::ExternalBody);
			if (contentType == ContentType::ExternalBody) {
				lInfo() << "Import of external body content is skipped.";
				continue;
			}

			const string &text = getValueFromRow<string>(message, LegacyMessageColText, isNull);

			unique_ptr<Content> content;
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
				content.reset(new FileContent());
				content->setContentType(fileContentType);
				content->setAppData("legacy", appData);
				content->setBodyFromLocale(text);
			} else {
				content.reset(new Content());
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
			*session << "INSERT INTO event (type, creation_time) VALUES (:type, :creationTime)",
				soci::use(eventType), soci::use(creationTime);

			const long long &eventId = dbSession.getLastInsertId();
			const long long &localSipAddressId = insertSipAddress(
				IdentityAddress(message.get<string>(LegacyMessageColLocalAddress)).asString()
			);
			const long long &remoteSipAddressId = insertSipAddress(
				IdentityAddress(message.get<string>(LegacyMessageColRemoteAddress)).asString()
			);
			const long long &chatRoomId = insertOrUpdateImportedBasicChatRoom(
				remoteSipAddressId,
				localSipAddressId,
				creationTime
			);
			const int &isSecured = message.get<int>(LegacyMessageColIsSecured, 0);
			const int deliveryNotificationRequired = 0;
			const int displayNotificationRequired = 0;

			*session << "INSERT INTO conference_event (event_id, chat_room_id)"
				" VALUES (:eventId, :chatRoomId)", soci::use(eventId), soci::use(chatRoomId);

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
				")", soci::use(eventId), soci::use(localSipAddressId), soci::use(remoteSipAddressId),
				soci::use(creationTime), soci::use(state), soci::use(direction), soci::use(isSecured),
				soci::use(deliveryNotificationRequired), soci::use(displayNotificationRequired);

			if (content)
				insertContent(eventId, *content);
			insertChatRoomParticipant(chatRoomId, remoteSipAddressId, false);
			insertChatMessageParticipant(eventId, remoteSipAddressId, state, std::time(nullptr));
		}
		// Set last_message_id to the last timed message for all chat room
		*dbSession.getBackendSession() << "UPDATE chat_room SET last_message_id = "
			"(SELECT COALESCE( (SELECT max(conference_event.event_id) as m "// max(conference_event.event_id) ensure to have only one event and the last id for the matching time
				"FROM conference_event, conference_chat_message_event,"
				"(SELECT max(time) as t, conference_event.chat_room_id as c "// Get Max Time for the chat room
					"FROM conference_event, conference_chat_message_event "
					"WHERE conference_event.event_id=conference_chat_message_event.event_id GROUP BY conference_event.chat_room_id)"
				"WHERE conference_chat_message_event.time=t AND conference_chat_message_event.event_id=conference_event.event_id AND conference_event.chat_room_id=c "
				"AND conference_event.chat_room_id=chat_room.id "
				"GROUP BY conference_event.chat_room_id),0))";// if there are no messages, the first is NULL. So put a 0 to the ID
		tr.commit();
		lInfo() << "Successful import of legacy messages.";
	};
}

void MainDbPrivate::importLegacyCallLogs (DbSession &inDbSession) {
	L_Q();
	L_DB_TRANSACTION_C(q) {
		if (getModuleVersion("legacy-call-logs-import") >= makeVersion(1, 0, 0))
			return;
		updateModuleVersion("legacy-call-logs-import", ModuleVersionLegacyCallLogsImport);

		soci::session *inSession = inDbSession.getBackendSession();
		if (!checkLegacyCallLogsTableExists(*inSession))
			return;

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

			auto callLog = CallLog::create(q->getCore(),
				static_cast<LinphoneCallDir>(direction),
				linphone_address_new(log.get<string>(LegacyCallLogColFrom).c_str()),
				linphone_address_new(log.get<string>(LegacyCallLogColTo).c_str()));

			callLog->setDuration(log.get<int>(LegacyCallLogColDuration));
			callLog->setStartTime((time_t) std::stoul(log.get<string>(LegacyCallLogColStartTime)));
			callLog->setConnectedTime((time_t) std::stoul(log.get<string>(LegacyCallLogColConnectedTime)));
			callLog->setStatus(static_cast<LinphoneCallStatus>(status));
			callLog->setVideoEnabled(!!log.get<int>(LegacyCallLogColVideoEnabled));
			callLog->setQuality((float) log.get<double>(LegacyCallLogColQuality));

			soci::indicator ind = log.get_indicator(LegacyCallLogColCallId);
			if (ind == soci::i_ok) callLog->setCallId(log.get<string>(LegacyCallLogColCallId));

			ind = log.get_indicator(LegacyCallLogColRefKey);
			if (ind == soci::i_ok) callLog->setRefKey(log.get<string>(LegacyCallLogColRefKey));

			insertOrUpdateConferenceCall(callLog, nullptr);
		}

		tr.commit();
		lInfo() << "Successful import of legacy call logs.";
	};
}
#endif

// =============================================================================

MainDb::MainDb (const shared_ptr<Core> &core) : AbstractDb(*new MainDbPrivate), CoreAccessor(core) {}

void MainDb::init () {
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

	/* Enable secure delete - so that erased chat messages are really erased and not just marked as unused.
	 * See https://sqlite.org/pragma.html#pragma_secure_delete
	 * This setting is global for the database.
	 * It is enabled only for sqlite3 backend, which is the one used for liblinphone clients.
	 * The mysql backend (used server-side) doesn't support this PRAGMA.
	 */

	session->begin();

	try{
		if (backend == Sqlite3) *session << string("PRAGMA secure_delete = ON");

		//Charset set to ascii for mysql/mariadb to allow creation of indexed collumns of size > 191. We assume that for the given fields ascii will not cause any display issue.

		//191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4

		*session <<
			"CREATE TABLE IF NOT EXISTS sip_address ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","
			"  value VARCHAR(255) UNIQUE NOT NULL"
		") " + (backend == Mysql ? "DEFAULT CHARSET=ascii" : "");

		*session <<
			"CREATE TABLE IF NOT EXISTS content_type ("
			"  id" + primaryKeyStr("SMALLINT UNSIGNED") + ","
			"  value VARCHAR(255) UNIQUE NOT NULL"
			") "+ (backend == Mysql ? "DEFAULT CHARSET=ascii" :"");

		*session <<
			"CREATE TABLE IF NOT EXISTS event ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","
			"  type TINYINT UNSIGNED NOT NULL,"
			"  creation_time" + timestampType() + " NOT NULL"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS chat_room ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			// Server (for conference) or user sip address.
			"  peer_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  local_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			// Dialog creation time.
			"  creation_time" + timestampType() + " NOT NULL,"

			// Last event time (call, message...).
			"  last_update_time" + timestampType() + " NOT NULL,"

			// ConferenceChatRoom, BasicChatRoom, RTT...
			"  capabilities TINYINT UNSIGNED NOT NULL,"

			// Chatroom subject.
			// /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
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
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS one_to_one_chat_room ("
			"  chat_room_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  participant_a_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  participant_b_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  FOREIGN KEY (chat_room_id)"
			"    REFERENCES chat_room(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (participant_a_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (participant_b_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS chat_room_participant ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  chat_room_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","
			"  participant_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","

			"  is_admin BOOLEAN NOT NULL,"

			"  UNIQUE (chat_room_id, participant_sip_address_id),"

			"  FOREIGN KEY (chat_room_id)"
			"    REFERENCES chat_room(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (participant_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS chat_room_participant_device ("
			"  chat_room_participant_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","
			"  participant_device_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","

			"  PRIMARY KEY (chat_room_participant_id, participant_device_sip_address_id),"

			"  FOREIGN KEY (chat_room_participant_id)"
			"    REFERENCES chat_room_participant(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (participant_device_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  chat_room_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES event(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (chat_room_id)"
			"    REFERENCES chat_room(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_notified_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  notify_id INT UNSIGNED NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_event(event_id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_participant_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  participant_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_notified_event(event_id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (participant_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_participant_device_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  device_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_participant_event(event_id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (device_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_security_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  security_alert TINYINT UNSIGNED NOT NULL,"
			// /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
			"  faulty_device VARCHAR(255) NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_event(event_id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_subject_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			// /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
			"  subject VARCHAR(255) NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_notified_event(event_id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_chat_message_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  from_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  to_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  time" + timestampType() + " ,"

			// See: https://tools.ietf.org/html/rfc5438#section-6.3
			// /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
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
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS chat_message_participant ("
			"  event_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","
			"  participant_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","
			"  state TINYINT UNSIGNED NOT NULL,"

			"  PRIMARY KEY (event_id, participant_sip_address_id),"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_chat_message_event(event_id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (participant_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS chat_message_content ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  event_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  content_type_id" + primaryKeyRefStr("SMALLINT UNSIGNED") + " NOT NULL,"
			"  body TEXT NOT NULL,"

			"  UNIQUE (id, event_id),"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_chat_message_event(event_id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (content_type_id)"
			"    REFERENCES content_type(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS chat_message_file_content ("
			"  chat_message_content_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			// /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
			"  name VARCHAR(256) NOT NULL,"
			"  size INT UNSIGNED NOT NULL,"

			// /!\ Warning : if this column is indexed, its size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
			"  path VARCHAR(512) NOT NULL,"

			"  FOREIGN KEY (chat_message_content_id)"
			"    REFERENCES chat_message_content(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS chat_message_content_app_data ("
			"  chat_message_content_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","

			"  name VARCHAR(191)," //191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4
			"  data BLOB NOT NULL,"

			"  PRIMARY KEY (chat_message_content_id, name),"
			"  FOREIGN KEY (chat_message_content_id)"
			"    REFERENCES chat_message_content(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_message_crypto_data ("
			"  event_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","

			"  name VARCHAR(191)," //191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4
			"  data BLOB NOT NULL,"

			"  PRIMARY KEY (event_id, name),"
			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_chat_message_event(event_id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS friends_list ("
			"  id" + primaryKeyStr("INT UNSIGNED") + ","

			"  name VARCHAR(191) UNIQUE,"  //191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4

			// /!\ Warning : if varchar columns > 255 are indexed, their size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
			"  rls_uri VARCHAR(2047),"
			"  sync_uri VARCHAR(2047),"
			"  revision INT UNSIGNED NOT NULL"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS friend ("
			"  id" + primaryKeyStr("INT UNSIGNED") + ","

			"  sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  friends_list_id" + primaryKeyRefStr("INT UNSIGNED") + " NOT NULL,"

			"  subscribe_policy TINYINT UNSIGNED NOT NULL,"
			"  send_subscribe BOOLEAN NOT NULL,"
			"  presence_received BOOLEAN NOT NULL,"

			"  v_card MEDIUMTEXT,"

			// /!\ Warning : if these varchar columns are indexed, their size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
			"  v_card_etag VARCHAR(255),"
			"  v_card_sync_uri VARCHAR(2047),"

			"  FOREIGN KEY (sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (friends_list_id)"
			"    REFERENCES friends_list(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS friend_app_data ("
			"  friend_id" + primaryKeyRefStr("INT UNSIGNED") + ","

			"  name VARCHAR(191)," //191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4
			"  data BLOB NOT NULL,"

			"  PRIMARY KEY (friend_id, name),"
			"  FOREIGN KEY (friend_id)"
			"    REFERENCES friend(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS db_module_version ("
			"  name" + varcharPrimaryKeyStr(191) + "," //191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4
			"  version INT UNSIGNED NOT NULL"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS chat_message_ephemeral_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","
			"  ephemeral_lifetime DOUBLE NOT NULL,"
			"  expired_time" + timestampType() + " NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_event(event_id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_ephemeral_message_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  lifetime DOUBLE NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES conference_event(event_id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS one_to_one_chat_room_previous_conference_id ("
			"  id" + primaryKeyStr("INT UNSIGNED") + ","

			"  sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  chat_room_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  FOREIGN KEY (sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (chat_room_id)"
			"    REFERENCES chat_room(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_info ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  organizer_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  uri_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  start_time" + timestampType() + ","
			"  duration INT UNSIGNED,"

			// /!\ Warning : if these varchar columns are indexed, their size must be set back to 191 = max indexable (KEY or UNIQUE) varchar size for mysql < 5.7 with charset utf8mb4 (both here and in migrations)
			"  subject VARCHAR(256) NOT NULL,"
			"  description VARCHAR(2048),"

			"  FOREIGN KEY (organizer_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (uri_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_info_participant ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  conference_info_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  participant_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  UNIQUE (conference_info_id, participant_sip_address_id),"

			"  FOREIGN KEY (conference_info_id)"
			"    REFERENCES conference_info(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (participant_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_info_organizer ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  conference_info_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  organizer_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  params VARCHAR(2048) DEFAULT '',"

			"  UNIQUE (conference_info_id, organizer_sip_address_id),"

			"  FOREIGN KEY (conference_info_id)"
			"    REFERENCES conference_info(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (organizer_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_call ("
			"  id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  from_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  to_sip_address_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"
			"  direction TINYINT UNSIGNED NOT NULL,"
			"  duration INT UNSIGNED,"
			"  start_time" + timestampType() + " NOT NULL,"
			"  connected_time" + timestampType() + ","
			"  status TINYINT UNSIGNED NOT NULL,"
			"  video_enabled BOOLEAN NOT NULL,"
			"  quality DOUBLE,"
			"  call_id VARCHAR(64),"
			"  refkey VARCHAR(64),"
			"  conference_info_id" + primaryKeyRefStr("BIGINT UNSIGNED") + ","

			"  FOREIGN KEY (from_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (to_sip_address_id)"
			"    REFERENCES sip_address(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (conference_info_id)"
			"    REFERENCES conference_info(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session <<
			"CREATE TABLE IF NOT EXISTS conference_call_event ("
			"  event_id" + primaryKeyStr("BIGINT UNSIGNED") + ","

			"  conference_call_id" + primaryKeyRefStr("BIGINT UNSIGNED") + " NOT NULL,"

			"  FOREIGN KEY (event_id)"
			"    REFERENCES event(id)"
			"    ON DELETE CASCADE,"
			"  FOREIGN KEY (conference_call_id)"
			"    REFERENCES conference_call(id)"
			"    ON DELETE CASCADE"
			") " + charset;

		*session << "DROP VIEW IF EXISTS conference_call_event_view";
		*session << "CREATE VIEW conference_call_event_view AS"
		"  SELECT event.id, type, creation_time, conference_call_id, from_sip_address_id, to_sip_address_id, direction, conference_call.duration AS call_duration,"
		"    conference_call.start_time AS call_start_time, connected_time, status, video_enabled, quality, call_id, refkey, conference_info_id,"
		"    organizer_sip_address_id, uri_sip_address_id, conference_info.start_time AS conf_start_time, conference_info.duration AS conf_duration,"
		"    subject, description"
		"  FROM event"
		"  LEFT JOIN conference_call_event ON conference_call_event.event_id = event.id"
		"  LEFT JOIN conference_call ON conference_call.id = conference_call_event.conference_call_id"
		"  LEFT JOIN conference_info ON conference_info.id = conference_call.conference_info_id";

		d->updateSchema();

		d->updateModuleVersion("events", ModuleVersionEvents);
		d->updateModuleVersion("friends", ModuleVersionFriends);
	}catch(const soci::soci_error &e){
		lError() << "Exception while creating or updating the database's schema : " << e.what();
		session->rollback();
		return;
	}
	session->commit();
#endif
}

bool MainDb::addEvent (const shared_ptr<EventLog> &eventLog) {
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

			case EventLog::Type::ConferenceParticipantAdded:
			case EventLog::Type::ConferenceParticipantRemoved:
			case EventLog::Type::ConferenceParticipantSetAdmin:
			case EventLog::Type::ConferenceParticipantUnsetAdmin:
				eventId = d->insertConferenceParticipantEvent(eventLog);
				break;

			case EventLog::Type::ConferenceParticipantDeviceAdded:
			case EventLog::Type::ConferenceParticipantDeviceRemoved:
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

bool MainDb::updateEvent (const shared_ptr<EventLog> &eventLog) {
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

			case EventLog::Type::ConferenceCreated:
			case EventLog::Type::ConferenceTerminated:
			case EventLog::Type::ConferenceCallStarted:
			case EventLog::Type::ConferenceCallConnected:
			case EventLog::Type::ConferenceCallEnded:
			case EventLog::Type::ConferenceParticipantAdded:
			case EventLog::Type::ConferenceParticipantRemoved:
			case EventLog::Type::ConferenceParticipantSetAdmin:
			case EventLog::Type::ConferenceParticipantUnsetAdmin:
			case EventLog::Type::ConferenceParticipantDeviceAdded:
			case EventLog::Type::ConferenceParticipantDeviceRemoved:
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
				return false;
		}

		tr.commit();

		return true;
	};
#else
	return false;
#endif
}

bool MainDb::deleteEvent (const shared_ptr<const EventLog> &eventLog) {
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
			shared_ptr<ChatMessage> chatMessage(static_pointer_cast<const ConferenceChatMessageEvent>(eventLog)->getChatMessage());
			shared_ptr<AbstractChatRoom> chatRoom(chatMessage->getChatRoom());
			const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
			*session << "UPDATE chat_room SET last_message_id = IFNULL((SELECT id FROM conference_event_simple_view WHERE chat_room_id = chat_room.id AND type = " << mapEventFilterToSql(ConferenceChatMessageFilter) << " ORDER BY id DESC LIMIT 1), 0) WHERE id = :1", soci::use(dbChatRoomId);
			// Delete chat message from cache as the event is deleted
			ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
			dChatMessage->resetStorageId();
		}

		tr.commit();

		// Reset storage ID as event is not valid anymore
		const_cast<EventLogPrivate *>(dEventLog)->resetStorageId();

		if (eventLog->getType() == EventLog::Type::ConferenceChatMessage) {
			shared_ptr<ChatMessage> chatMessage(static_pointer_cast<const ConferenceChatMessageEvent>(eventLog)->getChatMessage());
			if (chatMessage->getDirection() == ChatMessage::Direction::Incoming && !chatMessage->getPrivate()->isMarkedAsRead()) {
				int *count = d->unreadChatMessageCountCache[chatMessage->getChatRoom()->getConferenceId()];
				if (count)
					--*count;
			}
		}

		return true;
	};
#else
	return false;
#endif
}

int MainDb::getEventCount (FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	const string query = "SELECT COUNT(*) FROM event" +
		buildSqlEventFilter(
			{ ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter, ConferenceInfoNoDeviceFilter },
			mask
		);

	//DurationLogger durationLogger("Get event count with mask=" + Utils::toString(mask) + ".");

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

shared_ptr<EventLog> MainDb::getEvent (const unique_ptr<MainDb> &mainDb, const long long& storageId) {
#ifdef HAVE_DB_STORAGE
	if ((storageId < 0) || (mainDb == nullptr)) {
		lWarning() << "Unable to get event from invalid storage ID " << storageId;
		return nullptr;
	}

	MainDbPrivate *d = mainDb->getPrivate();

	shared_ptr<EventLog> event = d->getEventFromCache(storageId);
	if (event)
		return event;

	return L_DB_TRANSACTION_C(mainDb.get()) {
		// TODO: Improve. Deal with all events in the future.
		soci::row row;
		*d->dbSession.getBackendSession() << Statements::get(Statements::SelectConferenceEvent),
			soci::into(row), soci::use(storageId);

		ConferenceId conferenceId(ConferenceAddress(row.get<string>(16)), ConferenceAddress(row.get<string>(17)));
		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom)
			return shared_ptr<EventLog>();

		return d->selectGenericConferenceEvent(chatRoom, row);
	};
#else
	return nullptr;
#endif
}

shared_ptr<EventLog> MainDb::getEventFromKey (const MainDbKey &dbKey) {
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

list<shared_ptr<EventLog>> MainDb::getConferenceNotifiedEvents (
	const ConferenceId &conferenceId,
	unsigned int lastNotifyId
) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	const string query = Statements::get(Statements::SelectConferenceEvents) +
		string(" AND notify_id > :lastNotifyId");

	/*
	DurationLogger durationLogger(
		"Get conference notified events of: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() +
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

int MainDb::getChatMessageCount (const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	/*
	DurationLogger durationLogger(
		"Get chat messages count of: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() + ")."
	);
	*/

	return L_DB_TRANSACTION {
		L_D();

		int count;

		soci::session *session = d->dbSession.getBackendSession();

		string query = "SELECT COUNT(*) FROM conference_chat_message_event";
		if (!conferenceId.isValid())
			*session << query, soci::into(count);
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

int MainDb::getUnreadChatMessageCount (const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	L_D();

	if (conferenceId.isValid()) {
		const int *count = d->unreadChatMessageCountCache[conferenceId];
		if (count)
			return *count;
	}

	string query = "SELECT COUNT(*) FROM conference_chat_message_event WHERE";
	if (conferenceId.isValid())
		query += " event_id IN ("
			"  SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId"
			") AND";

	query += " marked_as_read == 0 ";

	/*
	DurationLogger durationLogger(
		"Get unread chat messages count of: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() + ")."
	);
	*/

	return L_DB_TRANSACTION {
		int count = 0;

		soci::session *session = d->dbSession.getBackendSession();

		if (!conferenceId.isValid())
			*session << query, soci::into(count);
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

void MainDb::markChatMessagesAsRead (const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	if (getUnreadChatMessageCount(conferenceId) == 0)
		return;

	static const string query = "UPDATE conference_chat_message_event"
		"  SET marked_as_read = 1"
		"  WHERE marked_as_read == 0"
		"  AND event_id IN ("
		"    SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId"
		"  )";

	/*
	DurationLogger durationLogger(
		"Mark chat messages as read of: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() + ")."
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

void MainDb::updateChatRoomEphemeralEnabled (const ConferenceId &conferenceId, bool ephemeralEnabled) const {
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

void MainDb::updateChatRoomEphemeralLifetime (const ConferenceId &conferenceId, long time) const {
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

void MainDb::updateEphemeralMessageInfos (const long long &eventId, const time_t &eTime) const {
#ifdef HAVE_DB_STORAGE
	static const string query = "UPDATE chat_message_ephemeral_event"
	"  SET expired_time = :expireTime"
	"  WHERE event_id = :eventId";

	L_DB_TRANSACTION {
		L_D();
		const tm &expireTime = Utils::getTimeTAsTm(eTime);
		*d->dbSession.getBackendSession() << query, soci::use(expireTime), soci::use(eventId);
		tr.commit();
	};
#endif
}

list<shared_ptr<ChatMessage>> MainDb::getUnreadChatMessages (const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	static const string query = Statements::get(Statements::SelectConferenceEvents)
		+ string(" AND marked_as_read == 0");

	DurationLogger durationLogger(
		"Get unread chat messages: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() + ")."
	);

	return L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();

		long long dbChatRoomId = d->selectChatRoomId(conferenceId);
		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		list<shared_ptr<ChatMessage>> chatMessages;
		if (!chatRoom)
			return chatMessages;

		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(dbChatRoomId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(
				chatRoom,
				row
			);

			if (event)
				chatMessages.push_back(static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
		}

		return chatMessages;
	};
#else
	return list<shared_ptr<ChatMessage>>();
#endif
}

list<shared_ptr<ChatMessage>> MainDb::getEphemeralMessages () const {
#ifdef HAVE_DB_STORAGE
	// Keep chat_room_id at the end of the query !!!
	string query =
		"SELECT conference_event_view.id, type, creation_time, from_sip_address.value, to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address.value, participant_sip_address.value, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value, chat_room_id"
		" FROM conference_event_view"
		" LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
		" LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
		" LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
		" LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = participant_sip_address_id"
		" LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
		" WHERE conference_event_view.id in ("
		" SELECT event_id"
		" FROM chat_message_ephemeral_event"
		" WHERE expired_time > :nullTime"
		" ORDER BY expired_time ASC";
	query += getBackend() == MainDb::Backend::Sqlite3 ? " LIMIT :maxMessages) ORDER BY expired_time ASC" : " ) ORDER BY expired_time ASC";

	return L_DB_TRANSACTION {
		L_D();
		list<shared_ptr<ChatMessage>> chatMessages;
		soci::rowset<soci::row> rows = getBackend() == MainDb::Backend::Sqlite3 ? (d->dbSession.getBackendSession()->prepare << query, soci::use(Utils::getTimeTAsTm(0)),  soci::use(EPHEMERAL_MESSAGE_TASKS_MAX_NB)) : (d->dbSession.getBackendSession()->prepare << query, soci::use(Utils::getTimeTAsTm(0)));
		for (const auto &row : rows) {
			const long long &dbChatRoomId = d->dbSession.resolveId(row, (int)row.size()-1);
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
						chatMessages.push_back(static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
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

list<MainDb::ParticipantState> MainDb::getChatMessageParticipantsByImdnState (
	const shared_ptr<EventLog> &eventLog,
	ChatMessage::State state
) const {
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
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query,
			soci::use(eventId), soci::use(stateInt)
		);

		list<MainDb::ParticipantState> result;
		for (const auto &row : rows)
			result.emplace_back(IdentityAddress(row.get<string>(0)), state, d->dbSession.getTime(row, 1));
		return result;
	};
#else
	return list<MainDb::ParticipantState>();
#endif
}

list<ChatMessage::State> MainDb::getChatMessageParticipantStates (const shared_ptr<EventLog> &eventLog) const {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		const EventLogPrivate *dEventLog = eventLog->getPrivate();
		MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
		const long long &eventId = dEventKey->storageId;

		unsigned int state;
		soci::statement statement = (
			d->dbSession.getBackendSession()->prepare << "SELECT state FROM chat_message_participant WHERE event_id = :eventId",
				soci::into(state), soci::use(eventId)
		);
		statement.execute();

		list<ChatMessage::State> states;
		while (statement.fetch()) {
			states.push_back(ChatMessage::State(state));
		}

		return states;
	};
#else
	return list<ChatMessage::State>();
#endif
}

ChatMessage::State MainDb::getChatMessageParticipantState (
	const shared_ptr<EventLog> &eventLog,
	const IdentityAddress &participantAddress
) const {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		const EventLogPrivate *dEventLog = eventLog->getPrivate();
		MainDbKeyPrivate *dEventKey = static_cast<MainDbKey &>(dEventLog->dbKey).getPrivate();
		const long long &eventId = dEventKey->storageId;
		const long long &participantSipAddressId = d->selectSipAddressId(participantAddress.asString());

		unsigned int state;
		*d->dbSession.getBackendSession() << "SELECT state FROM chat_message_participant"
			" WHERE event_id = :eventId AND participant_sip_address_id = :participantSipAddressId",
			soci::into(state), soci::use(eventId), soci::use(participantSipAddressId);

		return ChatMessage::State(state);
	};
#else
	return ChatMessage::State();
#endif
}

void MainDb::setChatMessageParticipantState (
	const shared_ptr<EventLog> &eventLog,
	const IdentityAddress &participantAddress,
	ChatMessage::State state,
	time_t stateChangeTime
) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();
		d->setChatMessageParticipantState(eventLog, participantAddress, state, stateChangeTime);
		tr.commit();
	};
#endif
}

bool MainDb::isChatRoomEmpty (const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	static const string query = "SELECT last_message_id FROM chat_room WHERE id = :1";

	return L_DB_TRANSACTION {
		L_D();

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (
			d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId)
		);

		for (const auto &row : rows) {
			return d->dbSession.resolveId(row, 0) == 0;
		}

		return true;
	};
#endif
	return true;
}

shared_ptr<ChatMessage> MainDb::getLastChatMessage (const ConferenceId &conferenceId) const {
#ifdef HAVE_DB_STORAGE
	static const string query = "SELECT conference_event_view.id AS event_id, type, conference_event_view.creation_time, from_sip_address.value, to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address.value, participant_sip_address.value, conference_event_view.subject, delivery_notification_required, display_notification_required, peer_sip_address.value, local_sip_address.value, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value"
			" FROM conference_event_view"
			" JOIN chat_room ON chat_room.id = chat_room_id"
			" JOIN sip_address AS peer_sip_address ON peer_sip_address.id = peer_sip_address_id"
			" JOIN sip_address AS local_sip_address ON local_sip_address.id = local_sip_address_id"
			" LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
			" LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
			" LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
			" LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = participant_sip_address_id"
			" LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
			" WHERE event_id = (SELECT last_message_id FROM chat_room WHERE id = :1)";

	return L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();
		shared_ptr<ChatMessage> chatMessage = nullptr;

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom)
			return chatMessage;

		long long dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(dbChatRoomId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
			if (event)
				return static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage();
		}

		return chatMessage;
	};
#else
	return nullptr;
#endif
}

list<shared_ptr<ChatMessage>> MainDb::findChatMessages (
	const ConferenceId &conferenceId,
	const string &imdnMessageId
) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	static const string query = Statements::get(Statements::SelectConferenceEvents) +
		string(" AND imdn_message_id = :imdnMessageId");

	/*
	DurationLogger durationLogger(
		"Find chat messages: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() + ")."
	);
	*/
	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		list<shared_ptr<ChatMessage>> chatMessages;
		if (!chatRoom)
			return chatMessages;

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (
			d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId), soci::use(imdnMessageId)
		);
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

list<shared_ptr<ChatMessage>> MainDb::findChatMessages (
	const ConferenceId &conferenceId,
	const list<string> &imdnMessageIds
) const {
#ifdef HAVE_DB_STORAGE
	// TODO: Optimize.
	static const string query = Statements::get(Statements::SelectConferenceEvents) +
		string(" AND ( imdn_message_id = ");

	/*
	DurationLogger durationLogger(
		"Find chat messages: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() + ")."
	);
	*/
	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		list<shared_ptr<ChatMessage>> chatMessages;
		if (!chatRoom)
			return chatMessages;

		ostringstream ostr;
		ostr << query;
		size_t index = 0;
		size_t listSize = imdnMessageIds.size();
		for (const auto& id : imdnMessageIds) {
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
		soci::rowset<soci::row> rows = (
			d->dbSession.getBackendSession()->prepare << computedQuery, soci::use(dbChatRoomId)
		);
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

list<shared_ptr<ChatMessage>> MainDb::findChatMessagesFromCallId (const std::string &callId) const {
#ifdef HAVE_DB_STORAGE
	// Keep chat_room_id at the end of the query !!!
	static const string query = "SELECT conference_event_view.id AS event_id, type, creation_time, from_sip_address.value, to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address.value, participant_sip_address.value, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value, chat_room_id"
			" FROM conference_event_view"
			" LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
			" LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
			" LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
			" LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = participant_sip_address_id"
			" LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
			" WHERE call_id = :callId";

	return L_DB_TRANSACTION {
		L_D();

		list<shared_ptr<ChatMessage>> chatMessages;
		soci::rowset<soci::row> rows = (
			d->dbSession.getBackendSession()->prepare << query, soci::use(callId)
		);

		for (const auto &row : rows) {
			const long long &dbChatRoomId = d->dbSession.resolveId(row, (int)row.size()-1);
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
						chatMessages.push_back(static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
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

list<shared_ptr<ChatMessage>> MainDb::findChatMessagesToBeNotifiedAsDelivered () const {
#ifdef HAVE_DB_STORAGE
	// Keep chat_room_id at the end of the query !!!
	static const string query = "SELECT conference_event_view.id AS event_id, type, creation_time, from_sip_address.value, to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address.value, participant_sip_address.value, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value, chat_room_id"
			" FROM conference_event_view"
			" LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id"
			" LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id"
			" LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id"
			" LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = participant_sip_address_id"
			" LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id"
			" WHERE conference_event_view.id IN (SELECT event_id FROM conference_chat_message_event WHERE delivery_notification_required <> 0 AND direction = :direction)";

	/*
	DurationLogger durationLogger(
		"Find chat messages to be notified as delivered: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() + ")."
	);
	*/

	return L_DB_TRANSACTION {
		L_D();

		list<shared_ptr<ChatMessage>> chatMessages;
		const int &direction = int(ChatMessage::Direction::Incoming);
		soci::rowset<soci::row> rows = (
			d->dbSession.getBackendSession()->prepare << query, soci::use(direction)
		);

		for (const auto &row : rows) {
			// chat_room_id is the last element of row
			const long long &dbChatRoomId = d->dbSession.resolveId(row, (int)row.size()-1);
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
						chatMessages.push_back(static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());
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

list<shared_ptr<EventLog>> MainDb::getHistory (const ConferenceId &conferenceId, int nLast, FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	return getHistoryRange(conferenceId, 0, nLast, mask);
#else
	return list<shared_ptr<EventLog>>();
#endif
}

list<shared_ptr<EventLog>> MainDb::getHistoryRange (
	const ConferenceId &conferenceId,
	int begin,
	int end,
	FilterMask mask
) const {
#ifdef HAVE_DB_STORAGE
	L_D();

	if (begin < 0)
		begin = 0;

	list<shared_ptr<EventLog>> events;
	if (end > 0 && begin > end) {
		lWarning() << "Unable to get history. Invalid range.";
		return events;
	}

	string query = Statements::get(Statements::SelectConferenceEvents) + buildSqlEventFilter({
		ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter, ConferenceInfoNoDeviceFilter, ConferenceChatMessageSecurityFilter
	}, mask, "AND");
	query += " ORDER BY event_id DESC";

	if (end > 0)
		query += " LIMIT " + Utils::toString(end - begin);
	else
		query += " LIMIT " + d->dbSession.noLimitValue();

	if (begin > 0)
		query += " OFFSET " + Utils::toString(begin);

	/*
	DurationLogger durationLogger(
		"Get history range of: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() +
		", begin=" + Utils::toString(begin) + ", end=" + Utils::toString(end) + ")."
	);
	*/

	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<AbstractChatRoom> chatRoom = d->findChatRoom(conferenceId);
		if (!chatRoom)
			return events;

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		soci::rowset<soci::row> rows = (d->dbSession.getBackendSession()->prepare << query, soci::use(dbChatRoomId));
		for (const auto &row : rows) {
			shared_ptr<EventLog> event = d->selectGenericConferenceEvent(chatRoom, row);
			if (event)
				events.push_front(event);
		}

		return events;
	};
#else
	return list<shared_ptr<EventLog>>();
#endif
}

int MainDb::getHistorySize (const ConferenceId &conferenceId, FilterMask mask) const {
#ifdef HAVE_DB_STORAGE
	const string query = "SELECT COUNT(*) FROM event, conference_event"
		"  WHERE chat_room_id = :chatRoomId"
		"  AND event_id = event.id" + buildSqlEventFilter({
			ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter, ConferenceInfoNoDeviceFilter, ConferenceChatMessageSecurityFilter
		}, mask, "AND");

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


void MainDb::cleanHistory (const ConferenceId &conferenceId, FilterMask mask) {
#ifdef HAVE_DB_STORAGE
	const string query = "SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId" +
		buildSqlEventFilter({
			ConferenceCallFilter, ConferenceChatMessageFilter, ConferenceInfoFilter, ConferenceInfoNoDeviceFilter
		}, mask);

	const string query2 = "UPDATE chat_room SET last_message_id = 0 WHERE id = :1";

	/*
	DurationLogger durationLogger(
		"Clean history of: (peer=" + conferenceId.getPeerAddress().asString() +
		", local=" + conferenceId.getLocalAddress().asString() +
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

		if (!mask || (mask & ConferenceChatMessageFilter))
			d->unreadChatMessageCountCache.insert(conferenceId, 0);
	};
#endif
}

// -----------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
template<typename T>
static void fetchContentAppData (soci::session *session, Content &content, long long contentId, T &data) {
	static const string query = "SELECT name, data FROM chat_message_content_app_data"
		" WHERE chat_message_content_id = :contentId";

	string name;
	soci::statement statement = (session->prepare << query, soci::use(contentId), soci::into(name), soci::into(data));
	statement.execute();
	while (statement.fetch())
		content.setAppData(name, blobToString(data));
}
#endif

void MainDb::loadChatMessageContents (const shared_ptr<ChatMessage> &chatMessage) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();

		bool hasFileTransferContent = false;

		ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
		const long long &eventId = chatMessage->getStorageId();

		static const string query = "SELECT chat_message_content.id, content_type.id, content_type.value, body, body_encoding_type"
			" FROM chat_message_content, content_type"
			" WHERE event_id = :eventId AND content_type_id = content_type.id";
		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(eventId));
		for (const auto &row : rows) {
			ContentType contentType(row.get<string>(2));
			const long long &contentId = d->dbSession.resolveId(row, 0);
			Content *content;
			int bodyEncodingType = row.get<int>(4);

			if (contentType == ContentType::FileTransfer) {
				hasFileTransferContent = true;
				content = new FileTransferContent();
			} else {
				// 1.1 - Fetch contents' file informations if they exist
				string name;
				int size;
				string path;
				int duration;

				*session << "SELECT name, size, path, duration FROM chat_message_file_content"
					" WHERE chat_message_content_id = :contentId",
					soci::into(name), soci::into(size), soci::into(path), soci::into(duration), soci::use(contentId);
				if (session->got_data()) {
					FileContent *fileContent = new FileContent();
					fileContent->setFileName(name);
					fileContent->setFileSize(size_t(size));
					fileContent->setFilePath(path);
					fileContent->setFileDuration(duration);
					content = fileContent;
				} else {
					content = new Content();
				}
			}

			content->setContentType(contentType);
			if(bodyEncodingType == 1)
				content->setBodyFromUtf8(row.get<string>(3));
			else
				content->setBodyFromLocale(row.get<string>(3));

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
		if (hasFileTransferContent)
			dChatMessage->loadFileTransferUrlFromBodyToContent();
	};
#endif
}

// -----------------------------------------------------------------------------

void MainDb::disableDeliveryNotificationRequired (const std::shared_ptr<const EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ChatMessage> chatMessage(static_pointer_cast<const ConferenceChatMessageEvent>(eventLog)->getChatMessage());
	const long long &eventId = static_cast<MainDbKey &>(eventLog->getPrivate()->dbKey).getPrivate()->storageId;

	L_DB_TRANSACTION {
		L_D();
		*d->dbSession.getBackendSession() << "UPDATE conference_chat_message_event SET delivery_notification_required = 0"
			" WHERE event_id = :eventId", soci::use(eventId);
		tr.commit();
	};
#endif
}

void MainDb::disableDisplayNotificationRequired (const std::shared_ptr<const EventLog> &eventLog) {
#ifdef HAVE_DB_STORAGE
	shared_ptr<ChatMessage> chatMessage(static_pointer_cast<const ConferenceChatMessageEvent>(eventLog)->getChatMessage());
	const long long &eventId = static_cast<MainDbKey &>(eventLog->getPrivate()->dbKey).getPrivate()->storageId;

	L_DB_TRANSACTION {
		L_D();
		*d->dbSession.getBackendSession() << "UPDATE conference_chat_message_event"
			" SET delivery_notification_required = 0, display_notification_required = 0"
			" WHERE event_id = :eventId", soci::use(eventId);
		tr.commit();
	};
#endif
}

// -----------------------------------------------------------------------------

list<shared_ptr<AbstractChatRoom>> MainDb::getChatRooms () const {
#ifdef HAVE_DB_STORAGE
	static const string query = "SELECT chat_room.id, peer_sip_address.value, local_sip_address.value,"
		" creation_time, last_update_time, capabilities, subject, last_notify_id, flags, last_message_id,"
		" ephemeral_enabled, ephemeral_messages_lifetime"
		" FROM chat_room, sip_address AS peer_sip_address, sip_address AS local_sip_address"
		" WHERE chat_room.peer_sip_address_id = peer_sip_address.id AND chat_room.local_sip_address_id = local_sip_address.id"
		" ORDER BY last_update_time DESC";

	DurationLogger durationLogger("Get chat rooms.");

	return L_DB_TRANSACTION {
		L_D();

		list<shared_ptr<AbstractChatRoom>> chatRooms;
		shared_ptr<Core> core = getCore();

		soci::session *session = d->dbSession.getBackendSession();

		soci::rowset<soci::row> rows = (session->prepare << query);
		for (const auto &row : rows) {
			ConferenceId conferenceId = ConferenceId(
				ConferenceAddress(row.get<string>(1)),
				ConferenceAddress(row.get<string>(2))
			);

			shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(conferenceId, false);
			if (chatRoom) {
				chatRooms.push_back(chatRoom);
				continue;
			}

			const long long &dbChatRoomId = d->dbSession.resolveId(row, 0);
			d->cache(conferenceId, dbChatRoomId);

			time_t creationTime = d->dbSession.getTime(row, 3);
			time_t lastUpdateTime = d->dbSession.getTime(row, 4);
			int capabilities = row.get<int>(5);
			string subject = row.get<string>(6, "");
			const long long &lastMessageId = d->dbSession.resolveId(row, 9);

			shared_ptr<ChatRoomParams> params = ChatRoomParams::fromCapabilities(capabilities);
			if (capabilities & ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::Basic)) {
				chatRoom = core->getPrivate()->createBasicChatRoom(conferenceId, capabilities, params);
				chatRoom->setUtf8Subject(subject);
			} else if (capabilities & ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::Conference)) {
#ifdef HAVE_ADVANCED_IM
				list<shared_ptr<Participant>> participants;

				static const string query = "SELECT chat_room_participant.id, sip_address.value, is_admin"
					" FROM sip_address, chat_room, chat_room_participant"
					" WHERE chat_room.id = :chatRoomId"
					" AND sip_address.id = chat_room_participant.participant_sip_address_id"
					" AND chat_room_participant.chat_room_id = chat_room.id";

				// Fetch participants.
				unsigned int lastNotifyId = d->dbSession.getUnsignedInt(row, 7, 0);
				soci::rowset<soci::row> rows = (session->prepare << query, soci::use(dbChatRoomId));
				shared_ptr<Participant> me;
				for (const auto &row : rows) {
					shared_ptr<Participant> participant = Participant::create(nullptr, IdentityAddress(row.get<string>(1)));
					participant->setAdmin(!!row.get<int>(2));

					// Fetch devices.
					{
						const long long &participantId = d->dbSession.resolveId(row, 0);
						static const string query = "SELECT sip_address.value, state, name FROM chat_room_participant_device, sip_address"
							" WHERE chat_room_participant_id = :participantId"
							" AND participant_device_sip_address_id = sip_address.id";

						soci::rowset<soci::row> rows = (session->prepare << query, soci::use(participantId));
						for (const auto &row : rows) {
							shared_ptr<ParticipantDevice> device = participant->addDevice(IdentityAddress(row.get<string>(0)), row.get<string>(2, ""));
							device->setState(ParticipantDevice::State(static_cast<unsigned int>(row.get<int>(1, 0))));
						}
					}

					if (participant->getAddress() == conferenceId.getLocalAddress().getAddressWithoutGruu())
						me = participant;
					else
						participants.push_back(participant);
				}

				Conference *conference = nullptr;
				if (!linphone_core_conference_server_enabled(core->getCCore())) {
					bool hasBeenLeft = !!row.get<int>(8, 0);
					if (!me) {
						lError() << "Unable to find me in: (peer=" + conferenceId.getPeerAddress().asString() +
							", local=" + conferenceId.getLocalAddress().asString() + ").";
						continue;
					}
					shared_ptr<ClientGroupChatRoom> clientGroupChatRoom(new ClientGroupChatRoom(
						core,
						conferenceId,
						me,
						capabilities,
						params,
						Utils::utf8ToLocale(subject),
						move(participants),
						lastNotifyId,
						hasBeenLeft
					));
					chatRoom = clientGroupChatRoom;
					conference = clientGroupChatRoom->getConference().get();
					chatRoom->setState(ConferenceInterface::State::Instantiated);
					chatRoom->enableEphemeral(!!row.get<int>(10, 0), false);
					chatRoom->setEphemeralLifetime((long)row.get<double>(11), false);
					chatRoom->setState(hasBeenLeft
						? ConferenceInterface::State::Terminated
						: ConferenceInterface::State::Created
					);

					if (capabilities & ChatRoom::CapabilitiesMask(ChatRoom::Capabilities::OneToOne)) {
						// TODO: load previous IDs if any
						static const string query = "SELECT sip_address.value FROM one_to_one_chat_room_previous_conference_id, sip_address"
							" WHERE chat_room_id = :chatRoomId"
							" AND sip_address_id = sip_address.id";
						soci::rowset<soci::row> rows = (session->prepare << query, soci::use(dbChatRoomId));
						for (const auto &row : rows) {
							ConferenceId previousId = ConferenceId(ConferenceAddress(row.get<string>(0)), conferenceId.getLocalAddress());
							if (previousId != conferenceId) {
								lInfo() << "Keeping around previous chat room ID [" << previousId << "] in case BYE is received for exhumed chat room [" << conferenceId << "]";
								clientGroupChatRoom->getPrivate()->addConferenceIdToPreviousList(previousId);
							}
						}
					}

				} else {
					auto serverGroupChatRoom = std::make_shared<ServerGroupChatRoom>(
						core,
						conferenceId.getPeerAddress(),
						capabilities,
						params,
						subject,
						move(participants),
						lastNotifyId
					);
					chatRoom = serverGroupChatRoom;
					conference = serverGroupChatRoom->getConference().get();
					chatRoom->setState(ConferenceInterface::State::Instantiated);
					chatRoom->enableEphemeral(!!row.get<int>(10, 0), false);
					chatRoom->setEphemeralLifetime((long)row.get<double>(11), false);
					chatRoom->setState(ConferenceInterface::State::Created);
				}
				for (auto participant : chatRoom->getParticipants())
					participant->setConference(conference);
#else
				lWarning() << "Advanced IM such as group chat is disabled!";
#endif
			}

			if (!chatRoom)
				continue; // Not fetched.

			AbstractChatRoomPrivate *dChatRoom = chatRoom->getPrivate();
			dChatRoom->setCreationTime(creationTime);
			dChatRoom->setLastUpdateTime(lastUpdateTime);
			dChatRoom->setIsEmpty(lastMessageId == 0);

			lDebug() << "Found chat room in DB: (peer=" <<
				conferenceId.getPeerAddress().asString() << ", local=" << conferenceId.getLocalAddress().asString() << ").";

			chatRooms.push_back(chatRoom);
		}

		tr.commit();

		return chatRooms;
	};
#else
	return list<shared_ptr<AbstractChatRoom>>();
#endif
}

void MainDbPrivate::insertNewPreviousConferenceId(const ConferenceId& currentConfId, const ConferenceId& previousConfId) {
#ifdef HAVE_DB_STORAGE
	const long long &previousConferenceSipAddressId = selectSipAddressId(previousConfId.getPeerAddress().asString());
	const long long &chatRoomId = selectChatRoomId(currentConfId);

	*dbSession.getBackendSession() << "INSERT INTO one_to_one_chat_room_previous_conference_id ("
	"  chat_room_id, sip_address_id"
	") VALUES ("
	"  :chatRoomId, :previousConferenceSipAddressId"
	")",
	soci::use(chatRoomId), soci::use(previousConferenceSipAddressId);
#endif
}

void MainDb::insertNewPreviousConferenceId(const ConferenceId& currentConfId, const ConferenceId& previousConfId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		lInfo() << "Inserting previous conf ID [" << previousConfId << "] in database for [" << currentConfId << "]";
		d->insertNewPreviousConferenceId(currentConfId, previousConfId);
		tr.commit();
	};
#endif
}

void MainDbPrivate::removePreviousConferenceId(const ConferenceId& previousConfId) {
#ifdef HAVE_DB_STORAGE
	const long long &previousConferenceSipAddressId = selectSipAddressId(previousConfId.getPeerAddress().asString());

	*dbSession.getBackendSession() << "DELETE FROM one_to_one_chat_room_previous_conference_id WHERE "
	"sip_address_id = :previousConferenceSipAddressId",
	soci::use(previousConferenceSipAddressId);
#endif
}

void MainDb::removePreviousConferenceId(const ConferenceId& previousConfId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		lInfo() << "Removing previous conf ID [" << previousConfId << "] from database";
		d->removePreviousConferenceId(previousConfId);
		tr.commit();
	};
#endif
}

void MainDb::insertChatRoom (const shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		d->insertChatRoom(chatRoom, notifyId);
		tr.commit();
	};
#endif
}

void MainDb::deleteChatRoom (const ConferenceId &conferenceId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);

		d->invalidConferenceEventsFromQuery(
			"SELECT event_id FROM conference_event WHERE chat_room_id = :chatRoomId",
			dbChatRoomId
		);

		*d->dbSession.getBackendSession() << "DELETE FROM chat_room WHERE id = :chatRoomId", soci::use(dbChatRoomId);

		tr.commit();
		d->unreadChatMessageCountCache.insert(conferenceId, 0);
	};
#endif
}

void MainDb::updateChatRoomConferenceId (const ConferenceId oldConferenceId, const ConferenceId &newConferenceId) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		const long long &peerSipAddressId = d->insertSipAddress(newConferenceId.getPeerAddress().asString());
		const long long &dbChatRoomId = d->selectChatRoomId(oldConferenceId);

		*d->dbSession.getBackendSession() << "UPDATE chat_room"
			" SET peer_sip_address_id = :peerSipAddressId"
			" WHERE id = :chatRoomId", soci::use(peerSipAddressId), soci::use(dbChatRoomId);

		tr.commit();

		d->cache(newConferenceId, dbChatRoomId);
	};
#endif
}



void MainDb::updateChatRoomLastUpdatedTime (const ConferenceId &conferenceId, time_t lastUpdatedTime) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();
		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);
		const tm &lastUpdateTimeTm = Utils::getTimeTAsTm(lastUpdatedTime);

		*d->dbSession.getBackendSession() << "UPDATE chat_room SET last_update_time = :lastUpdateTime"
			" WHERE id = :chatRoomId", soci::use(lastUpdateTimeTm),
			soci::use(dbChatRoomId);

		tr.commit();
	};
#endif
}

long long MainDb::addConferenceParticipantEventToDb (
	const shared_ptr<EventLog> &eventLog,
	long long *chatRoomId
) {

	L_D();
	return d->insertConferenceParticipantEvent(eventLog, chatRoomId, false);
}

void MainDb::updateNotifyId (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	const unsigned int lastNotify
) {
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

void MainDb::migrateBasicToClientGroupChatRoom (
	const shared_ptr<AbstractChatRoom> &basicChatRoom,
	const shared_ptr<AbstractChatRoom> &clientGroupChatRoom
) {
#ifdef HAVE_DB_STORAGE
	L_ASSERT(basicChatRoom->getCapabilities().isSet(ChatRoom::Capabilities::Basic));
	L_ASSERT(clientGroupChatRoom->getCapabilities().isSet(ChatRoom::Capabilities::Conference));

	L_DB_TRANSACTION {
		L_D();

		// TODO: Update events and chat messages. (Or wait signals.)
		const long long &dbChatRoomId = d->selectChatRoomId(basicChatRoom->getConferenceId());

		const ConferenceId &newConferenceId = clientGroupChatRoom->getConferenceId();
		const long long &peerSipAddressId = d->insertSipAddress(newConferenceId.getPeerAddress().asString());
		const long long &localSipAddressId = d->insertSipAddress(newConferenceId.getLocalAddress().asString());
		const int &capabilities = clientGroupChatRoom->getCapabilities();

		*d->dbSession.getBackendSession() << "UPDATE chat_room"
			" SET capabilities = :capabilities,"
			" peer_sip_address_id = :peerSipAddressId,"
			" local_sip_address_id = :localSipAddressId"
			" WHERE id = :chatRoomId", soci::use(capabilities), soci::use(peerSipAddressId),
			soci::use(localSipAddressId), soci::use(dbChatRoomId);

		shared_ptr<Participant> me = clientGroupChatRoom->getMe();
		long long meId = d->insertChatRoomParticipant(
			dbChatRoomId,
			d->insertSipAddress(me->getAddress().asString()),
			true
		);
		for (const auto &device : me->getDevices())
			d->insertChatRoomParticipantDevice(meId, d->insertSipAddress(device->getAddress().asString()), device->getName());

		for (const auto &participant : clientGroupChatRoom->getParticipants()) {
			long long participantId = d->insertChatRoomParticipant(
				dbChatRoomId,
				d->insertSipAddress(participant->getAddress().asString()),
				true
			);
			for (const auto &device : participant->getDevices())
				d->insertChatRoomParticipantDevice(participantId, d->insertSipAddress(device->getAddress().asString()), device->getName());
		}

		tr.commit();
	};
#endif
}

IdentityAddress MainDb::findMissingOneToOneConferenceChatRoomParticipantAddress (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	const IdentityAddress &presentParticipantAddr
) {
#ifdef HAVE_DB_STORAGE
	L_ASSERT(linphone_core_conference_server_enabled(chatRoom->getCore()->getCCore()));
	L_ASSERT(chatRoom->getCapabilities() & ChatRoom::Capabilities::OneToOne);
	L_ASSERT(chatRoom->getParticipantCount() == 1);

	return L_DB_TRANSACTION {
		L_D();

		string missingParticipantAddress;
		string participantASipAddress;
		string participantBSipAddress;

		const long long &chatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		L_ASSERT(chatRoomId != -1);

		*d->dbSession.getBackendSession() << "SELECT participant_a_sip_address.value, participant_b_sip_address.value"
			" FROM one_to_one_chat_room, sip_address AS participant_a_sip_address, sip_address AS participant_b_sip_address"
			" WHERE chat_room_id = :chatRoomId"
			" AND participant_a_sip_address_id = participant_a_sip_address.id"
			" AND participant_b_sip_address_id = participant_b_sip_address.id",
			soci::into(participantASipAddress), soci::into(participantBSipAddress), soci::use(chatRoomId);

		string presentParticipantAddress(presentParticipantAddr.asString());
		if (presentParticipantAddress == participantASipAddress)
			missingParticipantAddress = participantBSipAddress;
		else if (presentParticipantAddress == participantBSipAddress)
			missingParticipantAddress = participantASipAddress;

		return IdentityAddress(missingParticipantAddress);
	};
#else
	return IdentityAddress();
#endif
}

ConferenceAddress MainDb::findOneToOneConferenceChatRoomAddress (
	const IdentityAddress &participantA,
	const IdentityAddress &participantB,
	bool encrypted
) const {
#ifdef HAVE_DB_STORAGE
	return L_DB_TRANSACTION {
		L_D();

		const long long &participantASipAddressId = d->selectSipAddressId(participantA.asString());
		const long long &participantBSipAddressId = d->selectSipAddressId(participantB.asString());
		if (participantASipAddressId == -1 || participantBSipAddressId == -1)
			return ConferenceAddress();

		const long long &chatRoomId = d->selectOneToOneChatRoomId(participantASipAddressId, participantBSipAddressId, encrypted);
		if (chatRoomId == -1)
			return ConferenceAddress();

		string chatRoomAddress;
		*d->dbSession.getBackendSession() << "SELECT sip_address.value"
			" FROM chat_room, sip_address"
			" WHERE chat_room.id = :chatRoomId AND peer_sip_address_id = sip_address.id",
			soci::use(chatRoomId), soci::into(chatRoomAddress);

		return ConferenceAddress(chatRoomAddress);
	};
#else
	return ConferenceAddress();
#endif
}

void MainDb::insertOneToOneConferenceChatRoom (const shared_ptr<AbstractChatRoom> &chatRoom, bool encrypted) {
#ifdef HAVE_DB_STORAGE
	L_ASSERT(linphone_core_conference_server_enabled(chatRoom->getCore()->getCCore()));
	L_ASSERT(chatRoom->getCapabilities() & ChatRoom::Capabilities::OneToOne);

	L_DB_TRANSACTION {
		L_D();

		const list<shared_ptr<Participant>> &participants = chatRoom->getParticipants();
		const long long &participantASipAddressId = d->selectSipAddressId(participants.front()->getAddress().asString());
		const long long &participantBSipAddressId = d->selectSipAddressId(participants.back()->getAddress().asString());
		L_ASSERT(participantASipAddressId != -1);
		L_ASSERT(participantBSipAddressId != -1);

		long long chatRoomId = d->selectOneToOneChatRoomId(participantASipAddressId, participantBSipAddressId, encrypted);
		if (chatRoomId == -1) {
			chatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
			*d->dbSession.getBackendSession() << Statements::get(Statements::InsertOneToOneChatRoom, getBackend()),
				soci::use(chatRoomId), soci::use(participantASipAddressId), soci::use(participantBSipAddressId);
		}

		tr.commit();
	};
#endif
}

void MainDb::enableChatRoomMigration (const ConferenceId &conferenceId, bool enable) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();

		const long long &dbChatRoomId = d->selectChatRoomId(conferenceId);

		int capabilities = 0;
		*session << "SELECT capabilities FROM chat_room WHERE id = :chatRoomId",
			soci::use(dbChatRoomId), soci::into(capabilities);
		if (enable)
			capabilities |= int(ChatRoom::Capabilities::Migratable);
		else
			capabilities &= ~int(ChatRoom::Capabilities::Migratable);
		*session << "UPDATE chat_room SET capabilities = :capabilities WHERE id = :chatRoomId",
			soci::use(capabilities), soci::use(dbChatRoomId);

		tr.commit();
	};
#endif
}

void MainDb::updateChatRoomParticipantDevice (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	const shared_ptr<ParticipantDevice> &device
) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		L_DB_TRANSACTION {
			L_D();

			const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
			const long long &participantSipAddressId = d->selectSipAddressId(device->getParticipant()->getAddress().asString());
			const long long &participantId = d->selectChatRoomParticipantId(dbChatRoomId, participantSipAddressId);
			const long long &participantSipDeviceAddressId = d->selectSipAddressId(device->getAddress().asString());
			unsigned int state = static_cast<unsigned int>(device->getState());
			*d->dbSession.getBackendSession() << "UPDATE chat_room_participant_device SET state = :state, name = :name"
				" WHERE chat_room_participant_id = :participantId AND participant_device_sip_address_id = :participantSipDeviceAddressId",
				soci::use(state), soci::use(device->getName()), soci::use(participantId), soci::use(participantSipDeviceAddressId);

			tr.commit();
		};
	}
#endif
}

void MainDb::deleteChatRoomParticipant (
	const std::shared_ptr<AbstractChatRoom> &chatRoom,
	const IdentityAddress &participant
){
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		const long long &participantSipAddressId = d->selectSipAddressId(participant.asString());
		d->deleteChatRoomParticipant(dbChatRoomId, participantSipAddressId);
	}
#endif
}

void MainDb::deleteChatRoomParticipantDevice (
	const shared_ptr<AbstractChatRoom> &chatRoom,
	const shared_ptr<ParticipantDevice> &device
) {
#ifdef HAVE_DB_STORAGE
	L_D();
	if (isInitialized()) {
		const long long &dbChatRoomId = d->selectChatRoomId(chatRoom->getConferenceId());
		const long long &participantSipAddressId = d->selectSipAddressId(device->getParticipant()->getAddress().asString());
		const long long &participantId = d->selectChatRoomParticipantId(dbChatRoomId, participantSipAddressId);
		d->deleteChatRoomParticipantDevice(participantId, participantSipAddressId);
	}
#endif
}

// -----------------------------------------------------------------------------

std::list<std::shared_ptr<ConferenceInfo>> MainDb::getConferenceInfos (time_t afterThisTime) const {
#ifdef HAVE_DB_STORAGE
	string query = "SELECT conference_info.id, organizer_sip_address.value, uri_sip_address.value,"
		" start_time, duration, subject, description, state, ics_sequence, ics_uid"
		" FROM conference_info, sip_address AS organizer_sip_address, sip_address AS uri_sip_address"
		" WHERE conference_info.organizer_sip_address_id = organizer_sip_address.id AND conference_info.uri_sip_address_id = uri_sip_address.id";
	if (afterThisTime > -1) query += " AND start_time >= :startTime";
	query += " ORDER BY start_time";

	DurationLogger durationLogger("Get conference infos.");

	return L_DB_TRANSACTION {
		L_D();

		list<shared_ptr<ConferenceInfo>> conferenceInfos;

		soci::session *session = d->dbSession.getBackendSession();

		// We cannot create an empty rowset so each "if" will make one
		if (afterThisTime > -1) {
			const tm &startTime = Utils::getTimeTAsTm(afterThisTime);
			soci::rowset<soci::row> rows = (session->prepare << query, soci::use(startTime));

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

std::shared_ptr<ConferenceInfo> MainDb::getConferenceInfo (long long conferenceInfoId) const {
#ifdef HAVE_DB_STORAGE
	static const string query = "SELECT conference_info.id, organizer_sip_address.value, uri_sip_address.value,"
		" start_time, duration, subject, description, state, ics_sequence, ics_uid"
		" FROM conference_info, sip_address AS organizer_sip_address, sip_address AS uri_sip_address"
		" WHERE conference_info.organizer_sip_address_id = organizer_sip_address.id AND conference_info.uri_sip_address_id = uri_sip_address.id"
		"  AND conference_info.id = :conferenceInfoId";

	return L_DB_TRANSACTION {
		L_D();

		shared_ptr<ConferenceInfo> confInfo = nullptr;

		soci::session *session = d->dbSession.getBackendSession();

		soci::rowset<soci::row> rows = (session->prepare << query, soci::use(conferenceInfoId));

		const auto &row = rows.begin();
		if (row != rows.end()) {
			confInfo = d->selectConferenceInfo(*row);
		}

		tr.commit();

		return confInfo;
	};
#else
	return nullptr;
#endif
}

std::shared_ptr<ConferenceInfo> MainDb::getConferenceInfoFromURI (const ConferenceAddress &uri) const {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		string query = "SELECT conference_info.id, organizer_sip_address.value, uri_sip_address.value,"
			" start_time, duration, subject, description, state, ics_sequence, ics_uid"
			" FROM conference_info, sip_address AS organizer_sip_address, sip_address AS uri_sip_address"
			" WHERE conference_info.organizer_sip_address_id = organizer_sip_address.id AND conference_info.uri_sip_address_id = uri_sip_address.id"
			"  AND uri_sip_address.value = '" + uri.asString() + "'";

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

void MainDb::insertConferenceInfo (const std::shared_ptr<ConferenceInfo> &conferenceInfo) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		auto dbConfInfo = getConferenceInfoFromURI(conferenceInfo->getUri());
		L_DB_TRANSACTION {
			L_D();

			d->insertConferenceInfo(conferenceInfo, dbConfInfo);
			tr.commit();
		};
	}
#endif
}

void MainDb::deleteConferenceInfo (const std::shared_ptr<ConferenceInfo> &conferenceInfo) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		L_DB_TRANSACTION {
			L_D();

			const long long &uriSipAddressId = d->selectSipAddressId(conferenceInfo->getUri().asString());
			const long long &dbConferenceId = d->selectConferenceInfoId(uriSipAddressId);

			*d->dbSession.getBackendSession() << "DELETE FROM conference_info WHERE id = :conferenceId", soci::use(dbConferenceId);
			d->storageIdToConferenceInfo.erase(dbConferenceId);

			tr.commit();
		};
	}
#endif
}

// -----------------------------------------------------------------------------

long long MainDb::insertCallLog (const std::shared_ptr<CallLog> &callLog) {
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

void MainDb::deleteCallLog (const std::shared_ptr<CallLog> &callLog) {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		const long long &dbConferenceCallId = d->selectConferenceCallId(callLog->getCallId());

		*d->dbSession.getBackendSession() << "DELETE FROM conference_call WHERE id = :conferenceCallId", soci::use(dbConferenceCallId);
		d->storageIdToCallLog.erase(dbConferenceCallId),

		tr.commit();
	};
#endif
}

std::shared_ptr<CallLog> MainDb::getCallLog (const std::string &callId, int limit) {
#ifdef HAVE_DB_STORAGE
	if (isInitialized()) {
		string query = "SELECT c.id, from_sip_address.value, from_sip_address.display_name, to_sip_address.value, to_sip_address.display_name,"
			"  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, refkey, conference_info_id"
			" FROM (conference_call as c, sip_address AS from_sip_address, sip_address AS to_sip_address)";

		if (limit > 0) {
			query += " INNER JOIN (SELECT id from conference_call ORDER BY id DESC LIMIT " + std::to_string(limit) + ") as c2 ON c.id = c2.id";
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

std::list<std::shared_ptr<CallLog>> MainDb::getCallHistory (int limit) {
#ifdef HAVE_DB_STORAGE
	if( limit == 0) return list<shared_ptr<CallLog>>();
	string query = "SELECT conference_call.id, from_sip_address.value, from_sip_address.display_name, to_sip_address.value, to_sip_address.display_name,"
		"  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, refkey, conference_info_id"
		" FROM conference_call, sip_address AS from_sip_address, sip_address AS to_sip_address"
		" WHERE conference_call.from_sip_address_id = from_sip_address.id AND conference_call.to_sip_address_id = to_sip_address.id"
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
#else
	return list<shared_ptr<CallLog>>();
#endif
}

std::list<std::shared_ptr<CallLog>> MainDb::getCallHistory (const ConferenceAddress &address, int limit) {
#ifdef HAVE_DB_STORAGE
	string query = "SELECT conference_call.id, from_sip_address.value, from_sip_address.display_name, to_sip_address.value, to_sip_address.display_name,"
		"  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, refkey, conference_info_id"
		" FROM conference_call, sip_address AS from_sip_address, sip_address AS to_sip_address"
		" WHERE conference_call.from_sip_address_id = from_sip_address.id AND conference_call.to_sip_address_id = to_sip_address.id"
		"  AND (from_sip_address.value LIKE '%%" + address.asString() + "%%'"
		"  OR to_sip_address.value LIKE '%%" + address.asString() + "%%')"
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
#else
	return list<shared_ptr<CallLog>>();
#endif
}

std::list<std::shared_ptr<CallLog>> MainDb::getCallHistory (const ConferenceAddress &peer, const ConferenceAddress &local, int limit) {
#ifdef HAVE_DB_STORAGE
	string query = "SELECT conference_call.id, from_sip_address.value, from_sip_address.display_name, to_sip_address.value, to_sip_address.display_name,"
		"  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, refkey, conference_info_id"
		" FROM conference_call, sip_address AS from_sip_address, sip_address AS to_sip_address"
		" WHERE conference_call.from_sip_address_id = from_sip_address.id AND conference_call.to_sip_address_id = to_sip_address.id"
		"  AND ((from_sip_address.value LIKE '%%" + local.asString() + "%%' AND to_sip_address.value LIKE '%%" + peer.asString() + "%%' AND direction = 0) OR"
		"  (from_sip_address.value LIKE '%%" + peer.asString() + "%%' AND to_sip_address.value LIKE '%%" + local.asString() + "%%' AND DIRECTION = 1))"
		" ORDER BY conference_call.id DESC";

	if (limit > 0) query += " LIMIT " + to_string(limit);

	DurationLogger durationLogger("Get call history 2.");

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
#else
	return list<shared_ptr<CallLog>>();
#endif
}

std::shared_ptr<CallLog> MainDb::getLastOutgoingCall () {
#ifdef HAVE_DB_STORAGE
	static const string query = "SELECT conference_call.id, from_sip_address.value, from_sip_address.display_name, to_sip_address.value, to_sip_address.display_name,"
		"  direction, duration, start_time, connected_time, status, video_enabled, quality, call_id, refkey, conference_info_id"
		" FROM conference_call, sip_address AS from_sip_address, sip_address AS to_sip_address"
		" WHERE conference_call.from_sip_address_id = from_sip_address.id AND conference_call.to_sip_address_id = to_sip_address.id"
		"  AND direction = 0 AND conference_info_id IS NULL"
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
#else
	return nullptr;
#endif
}

void MainDb::deleteCallHistory () {
#ifdef HAVE_DB_STORAGE
	L_DB_TRANSACTION {
		L_D();

		soci::session *session = d->dbSession.getBackendSession();

		*session << "DELETE FROM conference_call";

		tr.commit();
	};
#endif
}

int MainDb::getCallHistorySize () {
#ifdef HAVE_DB_STORAGE
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
#else
	return -1;
#endif
}

// -----------------------------------------------------------------------------

bool MainDb::import (Backend, const string &parameters) {
#ifdef HAVE_DB_STORAGE
	L_D();

	// Backend is useless, it's sqlite3. (Only available legacy backend.)
	const string uri = "sqlite3://" + LinphonePrivate::Utils::localeToUtf8(parameters);
	DbSession inDbSession(uri);

	if (!inDbSession) {
		lWarning() << "Unable to connect to: `" << uri << "`.";
		return false;
	}

	// TODO: Remove condition after cpp migration in friends/friends list.
	if (false)
		d->importLegacyFriends(inDbSession);

	d->importLegacyHistory(inDbSession);
	d->importLegacyCallLogs(inDbSession);

	return true;
#else
	return false;
#endif
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

LINPHONE_END_NAMESPACE
