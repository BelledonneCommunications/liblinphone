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

#include "statements.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Statements {
using Backend = AbstractDb::Backend;

struct Statement {
	template <size_t N>
	constexpr Statement(Backend _backend, const char (&_sql)[N]) : backend(_backend), sql(_sql) {
	}

	Backend backend;
	const char *sql;
};

struct AbstractStatement {
public:
	template <size_t N>
	constexpr AbstractStatement(const char (&_sql)[N]) : mSql{_sql, nullptr} {
	}

	// TODO: Improve, check backends.
	constexpr AbstractStatement(const Statement &a, const Statement &b) : mSql{a.sql, b.sql} {
	}

	const char *get(Backend backend) const {
		return backend == Backend::Mysql && mSql[1] ? mSql[1] : mSql[0];
	}

private:
	const char *mSql[2];
};

// ---------------------------------------------------------------------------
// Select statements.
// ---------------------------------------------------------------------------

constexpr const char *select[SelectCount] = {
    /* SelectSipAddressIdCaseSensitive */ R"(
			SELECT id
			FROM sip_address
			WHERE value = :1
		)",

    /* SelectSipAddressIdCaseInsensitive */ R"(
			SELECT id
			FROM sip_address
			WHERE value LIKE :1
		)",

    /* SelectChatRoomId */ R"(
			SELECT id
			FROM chat_room
			WHERE peer_sip_address_id = :1 AND local_sip_address_id = :2
		)",

    /* SelectChatRoomParticipantId */ R"(
			SELECT id
			FROM chat_room_participant
			WHERE chat_room_id = :1 AND participant_sip_address_id = :2
		)",

    /* SelectOneToOneChatRoomId */ R"(
			SELECT chat_room_id
			FROM one_to_one_chat_room
			LEFT JOIN chat_room ON chat_room.id = chat_room_id
			WHERE participant_a_sip_address_id IN (:1, :2)
			AND participant_b_sip_address_id IN (:1, :2)
			AND (
				(participant_a_sip_address_id <> participant_b_sip_address_id AND :1 <> :2) OR
				(participant_a_sip_address_id = participant_b_sip_address_id AND :1 = :2)
			)
			AND (capabilities & :3) = :4
		)",

    /* SelectConferenceEvent */ R"(
			SELECT conference_event_view.id AS event_id, type, conference_event_view.creation_time, from_sip_address.value, to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address.value, participant_sip_address.value, conference_event_view.subject, delivery_notification_required, display_notification_required, peer_sip_address.value, local_sip_address.value, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value
			FROM conference_event_view
			JOIN chat_room ON chat_room.id = chat_room_id
			JOIN sip_address AS peer_sip_address ON peer_sip_address.id = peer_sip_address_id
			JOIN sip_address AS local_sip_address ON local_sip_address.id = local_sip_address_id
			LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id
			LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id
			LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id
			LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = participant_sip_address_id
			LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id
			WHERE conference_event_view.id = :1
		)",

    /* SelectConferenceEvents */ R"(
			SELECT conference_event_view.id AS event_id, type, creation_time, from_sip_address.value, to_sip_address.value, time, imdn_message_id, state, direction, is_secured, notify_id, device_sip_address.value, participant_sip_address.value, subject, delivery_notification_required, display_notification_required, security_alert, faulty_device, marked_as_read, forward_info, ephemeral_lifetime, expired_time, lifetime, reply_message_id, reply_sender_address.value, message_id
			FROM conference_event_view
			LEFT JOIN sip_address AS from_sip_address ON from_sip_address.id = from_sip_address_id
			LEFT JOIN sip_address AS to_sip_address ON to_sip_address.id = to_sip_address_id
			LEFT JOIN sip_address AS device_sip_address ON device_sip_address.id = device_sip_address_id
			LEFT JOIN sip_address AS participant_sip_address ON participant_sip_address.id = participant_sip_address_id
			LEFT JOIN sip_address AS reply_sender_address ON reply_sender_address.id = reply_sender_address_id
			WHERE chat_room_id = :1
		)",

    /* SelectConferenceInfoId */ R"(
			SELECT id
			FROM conference_info
			WHERE uri_sip_address_id = :1
		)",

    /* SelectConferenceInfoParticipantId */ R"(
			SELECT id
			FROM conference_info_participant
			WHERE conference_info_id = :1 AND participant_sip_address_id = :2
		)",

    /* SelectConferenceInfoOrganizerId */ R"(
			SELECT id
			FROM conference_info_organizer
			WHERE conference_info_id = :1
		)",

    /* SelectConferenceInfoFromId */
    R"(SELECT conference_info.id, organizer_sip_address.value, uri_sip_address.value, start_time, duration, subject, description, state, ics_sequence, ics_uid, security_level, audio, video, chat, ccmp_uri, earlier_joining_time, expiry_time FROM conference_info, sip_address AS organizer_sip_address, sip_address AS uri_sip_address WHERE conference_info.organizer_sip_address_id = organizer_sip_address.id AND conference_info.uri_sip_address_id = uri_sip_address.id AND conference_info.id = :1)",

    /* SelectConferenceCall */ R"(
			SELECT id
			FROM conference_call
			WHERE call_id = :1
		)",

    /* SelectSipAddressFromId */ R"(
			SELECT value
			FROM sip_address
			WHERE id = :1
		)"};

// ---------------------------------------------------------------------------
// Insert statements.
// ---------------------------------------------------------------------------

constexpr AbstractStatement insert[InsertCount] = {
    /* InsertOneToOneChatRoom */ R"(
			INSERT INTO one_to_one_chat_room (
				chat_room_id, participant_a_sip_address_id, participant_b_sip_address_id
			) VALUES (:1, :2, :3)
		)"};

// ---------------------------------------------------------------------------
// Getters.
// ---------------------------------------------------------------------------

const char *get(Select selectStmt) {
	return selectStmt >= Select::SelectCount ? nullptr : select[selectStmt];
}

const char *get(Insert insertStmt, AbstractDb::Backend backend) {
	return insertStmt >= Insert::InsertCount ? nullptr : insert[insertStmt].get(backend);
}
} // namespace Statements

LINPHONE_END_NAMESPACE
