/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

// Begin prologue.
//
#if __clang__ || __GNUC__ >= 4
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#if __GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#if __GNUC__ >= 7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
//
// End prologue.

#include <xsd/cxx/pre.hxx>

#include "imdn.h"

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
// Imdn
//

const Imdn::MessageIdType &Imdn::getMessageId() const {
	return this->message_id_.get();
}

Imdn::MessageIdType &Imdn::getMessageId() {
	return this->message_id_.get();
}

void Imdn::setMessageId(const MessageIdType &x) {
	this->message_id_.set(x);
}

void Imdn::setMessageId(::std::unique_ptr<MessageIdType> x) {
	this->message_id_.set(std::move(x));
}

::std::unique_ptr<Imdn::MessageIdType> Imdn::setDetachMessage_id() {
	return this->message_id_.detach();
}

const Imdn::DatetimeType &Imdn::getDatetime() const {
	return this->datetime_.get();
}

Imdn::DatetimeType &Imdn::getDatetime() {
	return this->datetime_.get();
}

void Imdn::setDatetime(const DatetimeType &x) {
	this->datetime_.set(x);
}

void Imdn::setDatetime(::std::unique_ptr<DatetimeType> x) {
	this->datetime_.set(std::move(x));
}

::std::unique_ptr<Imdn::DatetimeType> Imdn::setDetachDatetime() {
	return this->datetime_.detach();
}

const Imdn::RecipientUriOptional &Imdn::getRecipientUri() const {
	return this->recipient_uri_;
}

Imdn::RecipientUriOptional &Imdn::getRecipientUri() {
	return this->recipient_uri_;
}

void Imdn::setRecipientUri(const RecipientUriType &x) {
	this->recipient_uri_.set(x);
}

void Imdn::setRecipientUri(const RecipientUriOptional &x) {
	this->recipient_uri_ = x;
}

void Imdn::setRecipientUri(::std::unique_ptr<RecipientUriType> x) {
	this->recipient_uri_.set(std::move(x));
}

const Imdn::OriginalRecipientUriOptional &Imdn::getOriginalRecipientUri() const {
	return this->original_recipient_uri_;
}

Imdn::OriginalRecipientUriOptional &Imdn::getOriginalRecipientUri() {
	return this->original_recipient_uri_;
}

void Imdn::setOriginalRecipientUri(const OriginalRecipientUriType &x) {
	this->original_recipient_uri_.set(x);
}

void Imdn::setOriginalRecipientUri(const OriginalRecipientUriOptional &x) {
	this->original_recipient_uri_ = x;
}

void Imdn::setOriginalRecipientUri(::std::unique_ptr<OriginalRecipientUriType> x) {
	this->original_recipient_uri_.set(std::move(x));
}

const Imdn::SubjectOptional &Imdn::getSubject() const {
	return this->subject_;
}

Imdn::SubjectOptional &Imdn::getSubject() {
	return this->subject_;
}

void Imdn::setSubject(const SubjectType &x) {
	this->subject_.set(x);
}

void Imdn::setSubject(const SubjectOptional &x) {
	this->subject_ = x;
}

void Imdn::setSubject(::std::unique_ptr<SubjectType> x) {
	this->subject_.set(std::move(x));
}

const Imdn::DeliveryNotificationOptional &Imdn::getDeliveryNotification() const {
	return this->delivery_notification_;
}

Imdn::DeliveryNotificationOptional &Imdn::getDeliveryNotification() {
	return this->delivery_notification_;
}

void Imdn::setDeliveryNotification(const DeliveryNotificationType &x) {
	this->delivery_notification_.set(x);
}

void Imdn::setDeliveryNotification(const DeliveryNotificationOptional &x) {
	this->delivery_notification_ = x;
}

void Imdn::setDeliveryNotification(::std::unique_ptr<DeliveryNotificationType> x) {
	this->delivery_notification_.set(std::move(x));
}

const Imdn::DisplayNotificationOptional &Imdn::getDisplayNotification() const {
	return this->display_notification_;
}

Imdn::DisplayNotificationOptional &Imdn::getDisplayNotification() {
	return this->display_notification_;
}

void Imdn::setDisplayNotification(const DisplayNotificationType &x) {
	this->display_notification_.set(x);
}

void Imdn::setDisplayNotification(const DisplayNotificationOptional &x) {
	this->display_notification_ = x;
}

void Imdn::setDisplayNotification(::std::unique_ptr<DisplayNotificationType> x) {
	this->display_notification_.set(std::move(x));
}

const Imdn::ProcessingNotificationOptional &Imdn::getProcessingNotification() const {
	return this->processing_notification_;
}

Imdn::ProcessingNotificationOptional &Imdn::getProcessingNotification() {
	return this->processing_notification_;
}

void Imdn::setProcessingNotification(const ProcessingNotificationType &x) {
	this->processing_notification_.set(x);
}

void Imdn::setProcessingNotification(const ProcessingNotificationOptional &x) {
	this->processing_notification_ = x;
}

void Imdn::setProcessingNotification(::std::unique_ptr<ProcessingNotificationType> x) {
	this->processing_notification_.set(std::move(x));
}

const Imdn::AnySequence &Imdn::getAny() const {
	return this->any_;
}

Imdn::AnySequence &Imdn::getAny() {
	return this->any_;
}

void Imdn::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &Imdn::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Imdn::getDomDocument() {
	return *this->dom_document_;
}

// DeliveryNotification
//

const DeliveryNotification::StatusType &DeliveryNotification::getStatus() const {
	return this->status_.get();
}

DeliveryNotification::StatusType &DeliveryNotification::getStatus() {
	return this->status_.get();
}

void DeliveryNotification::setStatus(const StatusType &x) {
	this->status_.set(x);
}

void DeliveryNotification::setStatus(::std::unique_ptr<StatusType> x) {
	this->status_.set(std::move(x));
}

::std::unique_ptr<DeliveryNotification::StatusType> DeliveryNotification::setDetachStatus() {
	return this->status_.detach();
}

// Delivered
//

// Failed
//

// DisplayNotification
//

const DisplayNotification::StatusType &DisplayNotification::getStatus() const {
	return this->status_.get();
}

DisplayNotification::StatusType &DisplayNotification::getStatus() {
	return this->status_.get();
}

void DisplayNotification::setStatus(const StatusType &x) {
	this->status_.set(x);
}

void DisplayNotification::setStatus(::std::unique_ptr<StatusType> x) {
	this->status_.set(std::move(x));
}

::std::unique_ptr<DisplayNotification::StatusType> DisplayNotification::setDetachStatus() {
	return this->status_.detach();
}

// Displayed
//

// ProcessingNotification
//

const ProcessingNotification::StatusType &ProcessingNotification::getStatus() const {
	return this->status_.get();
}

ProcessingNotification::StatusType &ProcessingNotification::getStatus() {
	return this->status_.get();
}

void ProcessingNotification::setStatus(const StatusType &x) {
	this->status_.set(x);
}

void ProcessingNotification::setStatus(::std::unique_ptr<StatusType> x) {
	this->status_.set(std::move(x));
}

::std::unique_ptr<ProcessingNotification::StatusType> ProcessingNotification::setDetachStatus() {
	return this->status_.detach();
}

// Processed
//

// Stored
//

// Forbidden
//

// Error
//

// Status
//

const Status::DeliveredOptional &Status::getDelivered() const {
	return this->delivered_;
}

Status::DeliveredOptional &Status::getDelivered() {
	return this->delivered_;
}

void Status::setDelivered(const DeliveredType &x) {
	this->delivered_.set(x);
}

void Status::setDelivered(const DeliveredOptional &x) {
	this->delivered_ = x;
}

void Status::setDelivered(::std::unique_ptr<DeliveredType> x) {
	this->delivered_.set(std::move(x));
}

const Status::FailedOptional &Status::getFailed() const {
	return this->failed_;
}

Status::FailedOptional &Status::getFailed() {
	return this->failed_;
}

void Status::setFailed(const FailedType &x) {
	this->failed_.set(x);
}

void Status::setFailed(const FailedOptional &x) {
	this->failed_ = x;
}

void Status::setFailed(::std::unique_ptr<FailedType> x) {
	this->failed_.set(std::move(x));
}

const Status::ForbiddenOptional &Status::getForbidden() const {
	return this->forbidden_;
}

Status::ForbiddenOptional &Status::getForbidden() {
	return this->forbidden_;
}

void Status::setForbidden(const ForbiddenType &x) {
	this->forbidden_.set(x);
}

void Status::setForbidden(const ForbiddenOptional &x) {
	this->forbidden_ = x;
}

void Status::setForbidden(::std::unique_ptr<ForbiddenType> x) {
	this->forbidden_.set(std::move(x));
}

const Status::ErrorOptional &Status::getError() const {
	return this->error_;
}

Status::ErrorOptional &Status::getError() {
	return this->error_;
}

void Status::setError(const ErrorType &x) {
	this->error_.set(x);
}

void Status::setError(const ErrorOptional &x) {
	this->error_ = x;
}

void Status::setError(::std::unique_ptr<ErrorType> x) {
	this->error_.set(std::move(x));
}

const Status::ReasonOptional &Status::getReason() const {
	return this->reason_;
}

Status::ReasonOptional &Status::getReason() {
	return this->reason_;
}

void Status::setReason(const ReasonType &x) {
	this->reason_.set(x);
}

void Status::setReason(const ReasonOptional &x) {
	this->reason_ = x;
}

void Status::setReason(::std::unique_ptr<ReasonType> x) {
	this->reason_.set(std::move(x));
}

// Status1
//

const Status1::DisplayedOptional &Status1::getDisplayed() const {
	return this->displayed_;
}

Status1::DisplayedOptional &Status1::getDisplayed() {
	return this->displayed_;
}

void Status1::setDisplayed(const DisplayedType &x) {
	this->displayed_.set(x);
}

void Status1::setDisplayed(const DisplayedOptional &x) {
	this->displayed_ = x;
}

void Status1::setDisplayed(::std::unique_ptr<DisplayedType> x) {
	this->displayed_.set(std::move(x));
}

const Status1::ForbiddenOptional &Status1::getForbidden() const {
	return this->forbidden_;
}

Status1::ForbiddenOptional &Status1::getForbidden() {
	return this->forbidden_;
}

void Status1::setForbidden(const ForbiddenType &x) {
	this->forbidden_.set(x);
}

void Status1::setForbidden(const ForbiddenOptional &x) {
	this->forbidden_ = x;
}

void Status1::setForbidden(::std::unique_ptr<ForbiddenType> x) {
	this->forbidden_.set(std::move(x));
}

const Status1::ErrorOptional &Status1::getError() const {
	return this->error_;
}

Status1::ErrorOptional &Status1::getError() {
	return this->error_;
}

void Status1::setError(const ErrorType &x) {
	this->error_.set(x);
}

void Status1::setError(const ErrorOptional &x) {
	this->error_ = x;
}

void Status1::setError(::std::unique_ptr<ErrorType> x) {
	this->error_.set(std::move(x));
}

const Status1::AnySequence &Status1::getAny() const {
	return this->any_;
}

Status1::AnySequence &Status1::getAny() {
	return this->any_;
}

void Status1::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &Status1::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Status1::getDomDocument() {
	return *this->dom_document_;
}

// Status2
//

const Status2::ProcessedOptional &Status2::getProcessed() const {
	return this->processed_;
}

Status2::ProcessedOptional &Status2::getProcessed() {
	return this->processed_;
}

void Status2::setProcessed(const ProcessedType &x) {
	this->processed_.set(x);
}

void Status2::setProcessed(const ProcessedOptional &x) {
	this->processed_ = x;
}

void Status2::setProcessed(::std::unique_ptr<ProcessedType> x) {
	this->processed_.set(std::move(x));
}

const Status2::StoredOptional &Status2::getStored() const {
	return this->stored_;
}

Status2::StoredOptional &Status2::getStored() {
	return this->stored_;
}

void Status2::setStored(const StoredType &x) {
	this->stored_.set(x);
}

void Status2::setStored(const StoredOptional &x) {
	this->stored_ = x;
}

void Status2::setStored(::std::unique_ptr<StoredType> x) {
	this->stored_.set(std::move(x));
}

const Status2::ForbiddenOptional &Status2::getForbidden() const {
	return this->forbidden_;
}

Status2::ForbiddenOptional &Status2::getForbidden() {
	return this->forbidden_;
}

void Status2::setForbidden(const ForbiddenType &x) {
	this->forbidden_.set(x);
}

void Status2::setForbidden(const ForbiddenOptional &x) {
	this->forbidden_ = x;
}

void Status2::setForbidden(::std::unique_ptr<ForbiddenType> x) {
	this->forbidden_.set(std::move(x));
}

const Status2::ErrorOptional &Status2::getError() const {
	return this->error_;
}

Status2::ErrorOptional &Status2::getError() {
	return this->error_;
}

void Status2::setError(const ErrorType &x) {
	this->error_.set(x);
}

void Status2::setError(const ErrorOptional &x) {
	this->error_ = x;
}

void Status2::setError(::std::unique_ptr<ErrorType> x) {
	this->error_.set(std::move(x));
}

const Status2::AnySequence &Status2::getAny() const {
	return this->any_;
}

Status2::AnySequence &Status2::getAny() {
	return this->any_;
}

void Status2::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &Status2::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Status2::getDomDocument() {
	return *this->dom_document_;
}
} // namespace Imdn
} // namespace Xsd
} // namespace LinphonePrivate

#include <xsd/cxx/xml/dom/wildcard-source.hxx>

#include <xsd/cxx/xml/dom/parsing-source.hxx>

#include <xsd/cxx/tree/type-factory-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::type_factory_plate<0, char> type_factory_plate_init;
}

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
// Imdn
//

Imdn::Imdn(const MessageIdType &message_id, const DatetimeType &datetime)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      message_id_(message_id, this), datetime_(datetime, this), recipient_uri_(this), original_recipient_uri_(this),
      subject_(this), delivery_notification_(this), display_notification_(this), processing_notification_(this),
      any_(this->getDomDocument()) {
}

Imdn::Imdn(::std::unique_ptr<MessageIdType> message_id, ::std::unique_ptr<DatetimeType> datetime)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      message_id_(std::move(message_id), this), datetime_(std::move(datetime), this), recipient_uri_(this),
      original_recipient_uri_(this), subject_(this), delivery_notification_(this), display_notification_(this),
      processing_notification_(this), any_(this->getDomDocument()) {
}

Imdn::Imdn(const Imdn &x, ::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      message_id_(x.message_id_, f, this), datetime_(x.datetime_, f, this), recipient_uri_(x.recipient_uri_, f, this),
      original_recipient_uri_(x.original_recipient_uri_, f, this), subject_(x.subject_, f, this),
      delivery_notification_(x.delivery_notification_, f, this),
      display_notification_(x.display_notification_, f, this),
      processing_notification_(x.processing_notification_, f, this), any_(x.any_, this->getDomDocument()) {
}

Imdn::Imdn(const ::xercesc::DOMElement &e,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), message_id_(this), datetime_(this),
      recipient_uri_(this), original_recipient_uri_(this), subject_(this), delivery_notification_(this),
      display_notification_(this), processing_notification_(this), any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void Imdn::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// message-id
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "message-id", "urn:ietf:params:xml:ns:imdn", &::xsd::cxx::tree::factory_impl<MessageIdType>, true, true,
			    i, n, f, this));

			if (tmp.get() != 0) {
				if (!message_id_.present()) {
					::std::unique_ptr<MessageIdType> r(dynamic_cast<MessageIdType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->message_id_.set(::std::move(r));
					continue;
				}
			}
		}

		// datetime
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "datetime", "urn:ietf:params:xml:ns:imdn", &::xsd::cxx::tree::factory_impl<DatetimeType>, true, true, i,
			    n, f, this));

			if (tmp.get() != 0) {
				if (!datetime_.present()) {
					::std::unique_ptr<DatetimeType> r(dynamic_cast<DatetimeType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->datetime_.set(::std::move(r));
					continue;
				}
			}
		}

		// recipient-uri
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "recipient-uri", "urn:ietf:params:xml:ns:imdn", &::xsd::cxx::tree::factory_impl<RecipientUriType>, true,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->recipient_uri_) {
					::std::unique_ptr<RecipientUriType> r(dynamic_cast<RecipientUriType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->recipient_uri_.set(::std::move(r));
					continue;
				}
			}
		}

		// original-recipient-uri
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "original-recipient-uri", "urn:ietf:params:xml:ns:imdn",
			    &::xsd::cxx::tree::factory_impl<OriginalRecipientUriType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->original_recipient_uri_) {
					::std::unique_ptr<OriginalRecipientUriType> r(dynamic_cast<OriginalRecipientUriType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->original_recipient_uri_.set(::std::move(r));
					continue;
				}
			}
		}

		// subject
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "subject", "urn:ietf:params:xml:ns:imdn", &::xsd::cxx::tree::factory_impl<SubjectType>, true, true, i,
			    n, f, this));

			if (tmp.get() != 0) {
				if (!this->subject_) {
					::std::unique_ptr<SubjectType> r(dynamic_cast<SubjectType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->subject_.set(::std::move(r));
					continue;
				}
			}
		}

		// delivery-notification
		//
		if (n.name() == "delivery-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<DeliveryNotificationType> r(DeliveryNotificationTraits::create(i, f, this));

			if (!this->delivery_notification_) {
				this->delivery_notification_.set(::std::move(r));
				continue;
			}
		}

		// display-notification
		//
		if (n.name() == "display-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<DisplayNotificationType> r(DisplayNotificationTraits::create(i, f, this));

			if (!this->display_notification_) {
				this->display_notification_.set(::std::move(r));
				continue;
			}
		}

		// processing-notification
		//
		if (n.name() == "processing-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<ProcessingNotificationType> r(ProcessingNotificationTraits::create(i, f, this));

			if (!this->processing_notification_) {
				this->processing_notification_.set(::std::move(r));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:imdn")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!message_id_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("message-id", "urn:ietf:params:xml:ns:imdn");
	}

	if (!datetime_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("datetime", "urn:ietf:params:xml:ns:imdn");
	}
}

Imdn *Imdn::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Imdn(*this, f, c);
}

Imdn &Imdn::operator=(const Imdn &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->message_id_ = x.message_id_;
		this->datetime_ = x.datetime_;
		this->recipient_uri_ = x.recipient_uri_;
		this->original_recipient_uri_ = x.original_recipient_uri_;
		this->subject_ = x.subject_;
		this->delivery_notification_ = x.delivery_notification_;
		this->display_notification_ = x.display_notification_;
		this->processing_notification_ = x.processing_notification_;
		this->any_ = x.any_;
	}

	return *this;
}

Imdn::~Imdn() {
}

// DeliveryNotification
//

DeliveryNotification::DeliveryNotification(const StatusType &status)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), status_(status, this) {
}

DeliveryNotification::DeliveryNotification(::std::unique_ptr<StatusType> status)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), status_(std::move(status), this) {
}

DeliveryNotification::DeliveryNotification(const DeliveryNotification &x,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), status_(x.status_, f, this) {
}

DeliveryNotification::DeliveryNotification(const ::xercesc::DOMElement &e,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c), status_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void DeliveryNotification::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// status
		//
		if (n.name() == "status" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<StatusType> r(StatusTraits::create(i, f, this));

			if (!status_.present()) {
				this->status_.set(::std::move(r));
				continue;
			}
		}

		break;
	}

	if (!status_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("status", "urn:ietf:params:xml:ns:imdn");
	}
}

DeliveryNotification *DeliveryNotification::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class DeliveryNotification(*this, f, c);
}

DeliveryNotification &DeliveryNotification::operator=(const DeliveryNotification &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->status_ = x.status_;
	}

	return *this;
}

DeliveryNotification::~DeliveryNotification() {
}

// Delivered
//

Delivered::Delivered() : ::LinphonePrivate::Xsd::XmlSchema::Type() {
}

Delivered::Delivered(const Delivered &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c) {
}

Delivered::Delivered(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f, c) {
}

Delivered::Delivered(const ::xercesc::DOMAttr &a,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(a, f, c) {
}

Delivered::Delivered(const ::std::string &s,
                     const ::xercesc::DOMElement *e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(s, e, f, c) {
}

Delivered *Delivered::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Delivered(*this, f, c);
}

Delivered::~Delivered() {
}

// Failed
//

Failed::Failed() : ::LinphonePrivate::Xsd::XmlSchema::Type() {
}

Failed::Failed(const Failed &x,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c) {
}

Failed::Failed(const ::xercesc::DOMElement &e,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f, c) {
}

Failed::Failed(const ::xercesc::DOMAttr &a,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(a, f, c) {
}

Failed::Failed(const ::std::string &s,
               const ::xercesc::DOMElement *e,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(s, e, f, c) {
}

Failed *Failed::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Failed(*this, f, c);
}

Failed::~Failed() {
}

// DisplayNotification
//

DisplayNotification::DisplayNotification(const StatusType &status)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), status_(status, this) {
}

DisplayNotification::DisplayNotification(::std::unique_ptr<StatusType> status)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), status_(std::move(status), this) {
}

DisplayNotification::DisplayNotification(const DisplayNotification &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), status_(x.status_, f, this) {
}

DisplayNotification::DisplayNotification(const ::xercesc::DOMElement &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c), status_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void DisplayNotification::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// status
		//
		if (n.name() == "status" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<StatusType> r(StatusTraits::create(i, f, this));

			if (!status_.present()) {
				this->status_.set(::std::move(r));
				continue;
			}
		}

		break;
	}

	if (!status_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("status", "urn:ietf:params:xml:ns:imdn");
	}
}

DisplayNotification *DisplayNotification::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class DisplayNotification(*this, f, c);
}

DisplayNotification &DisplayNotification::operator=(const DisplayNotification &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->status_ = x.status_;
	}

	return *this;
}

DisplayNotification::~DisplayNotification() {
}

// Displayed
//

Displayed::Displayed() : ::LinphonePrivate::Xsd::XmlSchema::Type() {
}

Displayed::Displayed(const Displayed &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c) {
}

Displayed::Displayed(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f, c) {
}

Displayed::Displayed(const ::xercesc::DOMAttr &a,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(a, f, c) {
}

Displayed::Displayed(const ::std::string &s,
                     const ::xercesc::DOMElement *e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(s, e, f, c) {
}

Displayed *Displayed::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Displayed(*this, f, c);
}

Displayed::~Displayed() {
}

// ProcessingNotification
//

ProcessingNotification::ProcessingNotification(const StatusType &status)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), status_(status, this) {
}

ProcessingNotification::ProcessingNotification(::std::unique_ptr<StatusType> status)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), status_(std::move(status), this) {
}

ProcessingNotification::ProcessingNotification(const ProcessingNotification &x,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), status_(x.status_, f, this) {
}

ProcessingNotification::ProcessingNotification(const ::xercesc::DOMElement &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c), status_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void ProcessingNotification::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// status
		//
		if (n.name() == "status" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<StatusType> r(StatusTraits::create(i, f, this));

			if (!status_.present()) {
				this->status_.set(::std::move(r));
				continue;
			}
		}

		break;
	}

	if (!status_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("status", "urn:ietf:params:xml:ns:imdn");
	}
}

ProcessingNotification *ProcessingNotification::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ProcessingNotification(*this, f, c);
}

ProcessingNotification &ProcessingNotification::operator=(const ProcessingNotification &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->status_ = x.status_;
	}

	return *this;
}

ProcessingNotification::~ProcessingNotification() {
}

// Processed
//

Processed::Processed() : ::LinphonePrivate::Xsd::XmlSchema::Type() {
}

Processed::Processed(const Processed &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c) {
}

Processed::Processed(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f, c) {
}

Processed::Processed(const ::xercesc::DOMAttr &a,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(a, f, c) {
}

Processed::Processed(const ::std::string &s,
                     const ::xercesc::DOMElement *e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(s, e, f, c) {
}

Processed *Processed::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Processed(*this, f, c);
}

Processed::~Processed() {
}

// Stored
//

Stored::Stored() : ::LinphonePrivate::Xsd::XmlSchema::Type() {
}

Stored::Stored(const Stored &x,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c) {
}

Stored::Stored(const ::xercesc::DOMElement &e,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f, c) {
}

Stored::Stored(const ::xercesc::DOMAttr &a,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(a, f, c) {
}

Stored::Stored(const ::std::string &s,
               const ::xercesc::DOMElement *e,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(s, e, f, c) {
}

Stored *Stored::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Stored(*this, f, c);
}

Stored::~Stored() {
}

// Forbidden
//

Forbidden::Forbidden() : ::LinphonePrivate::Xsd::XmlSchema::Type() {
}

Forbidden::Forbidden(const Forbidden &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c) {
}

Forbidden::Forbidden(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f, c) {
}

Forbidden::Forbidden(const ::xercesc::DOMAttr &a,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(a, f, c) {
}

Forbidden::Forbidden(const ::std::string &s,
                     const ::xercesc::DOMElement *e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(s, e, f, c) {
}

Forbidden *Forbidden::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Forbidden(*this, f, c);
}

Forbidden::~Forbidden() {
}

// Error
//

Error::Error() : ::LinphonePrivate::Xsd::XmlSchema::Type() {
}

Error::Error(const Error &x,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c) {
}

Error::Error(const ::xercesc::DOMElement &e,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f, c) {
}

Error::Error(const ::xercesc::DOMAttr &a,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(a, f, c) {
}

Error::Error(const ::std::string &s,
             const ::xercesc::DOMElement *e,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(s, e, f, c) {
}

Error *Error::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Error(*this, f, c);
}

Error::~Error() {
}

// Status
//

Status::Status()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), delivered_(this), failed_(this), forbidden_(this), error_(this),
      reason_(this) {
}

Status::Status(const Status &x,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), delivered_(x.delivered_, f, this), failed_(x.failed_, f, this),
      forbidden_(x.forbidden_, f, this), error_(x.error_, f, this), reason_(x.reason_, f, this) {
}

Status::Status(const ::xercesc::DOMElement &e,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      delivered_(this), failed_(this), forbidden_(this), error_(this), reason_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void Status::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// delivered
		//
		if (n.name() == "delivered" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<DeliveredType> r(DeliveredTraits::create(i, f, this));

			if (!this->delivered_) {
				this->delivered_.set(::std::move(r));
				continue;
			}
		}

		// failed
		//
		if (n.name() == "failed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<FailedType> r(FailedTraits::create(i, f, this));

			if (!this->failed_) {
				this->failed_.set(::std::move(r));
				continue;
			}
		}

		// forbidden
		//
		if (n.name() == "forbidden" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<ForbiddenType> r(ForbiddenTraits::create(i, f, this));

			if (!this->forbidden_) {
				this->forbidden_.set(::std::move(r));
				continue;
			}
		}

		// error
		//
		if (n.name() == "error" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<ErrorType> r(ErrorTraits::create(i, f, this));

			if (!this->error_) {
				this->error_.set(::std::move(r));
				continue;
			}
		}

		// reason
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "reason", "http://www.linphone.org/xsds/imdn.xsd", &::xsd::cxx::tree::factory_impl<ReasonType>, true,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->reason_) {
					::std::unique_ptr<ReasonType> r(dynamic_cast<ReasonType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->reason_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}
}

Status *Status::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Status(*this, f, c);
}

Status &Status::operator=(const Status &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->delivered_ = x.delivered_;
		this->failed_ = x.failed_;
		this->forbidden_ = x.forbidden_;
		this->error_ = x.error_;
		this->reason_ = x.reason_;
	}

	return *this;
}

Status::~Status() {
}

// Status1
//

Status1::Status1()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      displayed_(this), forbidden_(this), error_(this), any_(this->getDomDocument()) {
}

Status1::Status1(const Status1 &x,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      displayed_(x.displayed_, f, this), forbidden_(x.forbidden_, f, this), error_(x.error_, f, this),
      any_(x.any_, this->getDomDocument()) {
}

Status1::Status1(const ::xercesc::DOMElement &e,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), displayed_(this), forbidden_(this), error_(this),
      any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void Status1::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// displayed
		//
		if (n.name() == "displayed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<DisplayedType> r(DisplayedTraits::create(i, f, this));

			if (!this->displayed_) {
				this->displayed_.set(::std::move(r));
				continue;
			}
		}

		// forbidden
		//
		if (n.name() == "forbidden" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<ForbiddenType> r(ForbiddenTraits::create(i, f, this));

			if (!this->forbidden_) {
				this->forbidden_.set(::std::move(r));
				continue;
			}
		}

		// error
		//
		if (n.name() == "error" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<ErrorType> r(ErrorTraits::create(i, f, this));

			if (!this->error_) {
				this->error_.set(::std::move(r));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:imdn")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}
}

Status1 *Status1::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Status1(*this, f, c);
}

Status1 &Status1::operator=(const Status1 &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->displayed_ = x.displayed_;
		this->forbidden_ = x.forbidden_;
		this->error_ = x.error_;
		this->any_ = x.any_;
	}

	return *this;
}

Status1::~Status1() {
}

// Status2
//

Status2::Status2()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      processed_(this), stored_(this), forbidden_(this), error_(this), any_(this->getDomDocument()) {
}

Status2::Status2(const Status2 &x,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      processed_(x.processed_, f, this), stored_(x.stored_, f, this), forbidden_(x.forbidden_, f, this),
      error_(x.error_, f, this), any_(x.any_, this->getDomDocument()) {
}

Status2::Status2(const ::xercesc::DOMElement &e,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), processed_(this), stored_(this), forbidden_(this),
      error_(this), any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void Status2::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// processed
		//
		if (n.name() == "processed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<ProcessedType> r(ProcessedTraits::create(i, f, this));

			if (!this->processed_) {
				this->processed_.set(::std::move(r));
				continue;
			}
		}

		// stored
		//
		if (n.name() == "stored" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<StoredType> r(StoredTraits::create(i, f, this));

			if (!this->stored_) {
				this->stored_.set(::std::move(r));
				continue;
			}
		}

		// forbidden
		//
		if (n.name() == "forbidden" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<ForbiddenType> r(ForbiddenTraits::create(i, f, this));

			if (!this->forbidden_) {
				this->forbidden_.set(::std::move(r));
				continue;
			}
		}

		// error
		//
		if (n.name() == "error" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			::std::unique_ptr<ErrorType> r(ErrorTraits::create(i, f, this));

			if (!this->error_) {
				this->error_.set(::std::move(r));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:imdn")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}
}

Status2 *Status2::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Status2(*this, f, c);
}

Status2 &Status2::operator=(const Status2 &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->processed_ = x.processed_;
		this->stored_ = x.stored_;
		this->forbidden_ = x.forbidden_;
		this->error_ = x.error_;
		this->any_ = x.any_;
	}

	return *this;
}

Status2::~Status2() {
}
} // namespace Imdn
} // namespace Xsd
} // namespace LinphonePrivate

#include <ostream>

#include <xsd/cxx/tree/std-ostream-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::std_ostream_plate<0, char> std_ostream_plate_init;
}

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
::std::ostream &operator<<(::std::ostream &o, const Imdn &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "message-id: ";
		om.insert(o, i.getMessageId());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "datetime: ";
		om.insert(o, i.getDatetime());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getRecipientUri()) {
			o << ::std::endl << "recipient-uri: ";
			om.insert(o, *i.getRecipientUri());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getOriginalRecipientUri()) {
			o << ::std::endl << "original-recipient-uri: ";
			om.insert(o, *i.getOriginalRecipientUri());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSubject()) {
			o << ::std::endl << "subject: ";
			om.insert(o, *i.getSubject());
		}
	}

	if (i.getDeliveryNotification()) {
		o << ::std::endl << "delivery-notification: " << *i.getDeliveryNotification();
	}

	if (i.getDisplayNotification()) {
		o << ::std::endl << "display-notification: " << *i.getDisplayNotification();
	}

	if (i.getProcessingNotification()) {
		o << ::std::endl << "processing-notification: " << *i.getProcessingNotification();
	}

	return o;
}

::std::ostream &operator<<(::std::ostream &o, const DeliveryNotification &i) {
	o << ::std::endl << "status: " << i.getStatus();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Delivered &) {
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Failed &) {
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const DisplayNotification &i) {
	o << ::std::endl << "status: " << i.getStatus();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Displayed &) {
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const ProcessingNotification &i) {
	o << ::std::endl << "status: " << i.getStatus();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Processed &) {
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Stored &) {
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Forbidden &) {
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Error &) {
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Status &i) {
	if (i.getDelivered()) {
		o << ::std::endl << "delivered: " << *i.getDelivered();
	}

	if (i.getFailed()) {
		o << ::std::endl << "failed: " << *i.getFailed();
	}

	if (i.getForbidden()) {
		o << ::std::endl << "forbidden: " << *i.getForbidden();
	}

	if (i.getError()) {
		o << ::std::endl << "error: " << *i.getError();
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getReason()) {
			o << ::std::endl << "reason: ";
			om.insert(o, *i.getReason());
		}
	}

	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Status1 &i) {
	if (i.getDisplayed()) {
		o << ::std::endl << "displayed: " << *i.getDisplayed();
	}

	if (i.getForbidden()) {
		o << ::std::endl << "forbidden: " << *i.getForbidden();
	}

	if (i.getError()) {
		o << ::std::endl << "error: " << *i.getError();
	}

	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Status2 &i) {
	if (i.getProcessed()) {
		o << ::std::endl << "processed: " << *i.getProcessed();
	}

	if (i.getStored()) {
		o << ::std::endl << "stored: " << *i.getStored();
	}

	if (i.getForbidden()) {
		o << ::std::endl << "forbidden: " << *i.getForbidden();
	}

	if (i.getError()) {
		o << ::std::endl << "error: " << *i.getError();
	}

	return o;
}
} // namespace Imdn
} // namespace Xsd
} // namespace LinphonePrivate

#include <istream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/sax/std-input-source.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(const ::std::string &u,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>(::LinphonePrivate::Xsd::Imdn::parseImdn(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(const ::std::string &u,
          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>(::LinphonePrivate::Xsd::Imdn::parseImdn(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(const ::std::string &u,
          ::xercesc::DOMErrorHandler &h,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>(::LinphonePrivate::Xsd::Imdn::parseImdn(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseImdn(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseImdn(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          ::xercesc::DOMErrorHandler &h,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseImdn(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          const ::std::string &sid,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseImdn(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          const ::std::string &sid,
          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseImdn(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          const ::std::string &sid,
          ::xercesc::DOMErrorHandler &h,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseImdn(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::xercesc::InputSource &i,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>(::LinphonePrivate::Xsd::Imdn::parseImdn(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::xercesc::InputSource &i,
          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>(::LinphonePrivate::Xsd::Imdn::parseImdn(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::xercesc::InputSource &i,
          ::xercesc::DOMErrorHandler &h,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>(::LinphonePrivate::Xsd::Imdn::parseImdn(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(const ::xercesc::DOMDocument &doc,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>(::LinphonePrivate::Xsd::Imdn::parseImdn(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "imdn" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Imdn, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "imdn", "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "imdn" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Imdn, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "imdn", "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>(::LinphonePrivate::Xsd::Imdn::parseMessageId(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>(::LinphonePrivate::Xsd::Imdn::parseMessageId(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(const ::std::string &u,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>(::LinphonePrivate::Xsd::Imdn::parseMessageId(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseMessageId(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseMessageId(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::std::istream &is,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseMessageId(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseMessageId(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseMessageId(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::std::istream &is,
               const ::std::string &sid,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseMessageId(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>(::LinphonePrivate::Xsd::Imdn::parseMessageId(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>(::LinphonePrivate::Xsd::Imdn::parseMessageId(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::xercesc::InputSource &i,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>(::LinphonePrivate::Xsd::Imdn::parseMessageId(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(const ::xercesc::DOMDocument &doc,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>(::LinphonePrivate::Xsd::Imdn::parseMessageId(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "message-id", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Token>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Token *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "message-id",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token>
parseMessageId(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "message-id", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Token>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Token *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "message-id",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(const ::std::string &u,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseDatetime(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(const ::std::string &u,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseDatetime(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(const ::std::string &u,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseDatetime(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDatetime(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDatetime(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDatetime(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              const ::std::string &sid,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDatetime(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              const ::std::string &sid,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDatetime(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              const ::std::string &sid,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDatetime(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::xercesc::InputSource &i,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseDatetime(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::xercesc::InputSource &i,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseDatetime(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::xercesc::InputSource &i,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseDatetime(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(const ::xercesc::DOMDocument &doc,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseDatetime(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "datetime", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::String>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::String *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "datetime",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "datetime", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::String>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::String *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "datetime",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(::LinphonePrivate::Xsd::Imdn::parseRecipientUri(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(::LinphonePrivate::Xsd::Imdn::parseRecipientUri(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(const ::std::string &u,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(::LinphonePrivate::Xsd::Imdn::parseRecipientUri(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseRecipientUri(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseRecipientUri(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::std::istream &is,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseRecipientUri(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseRecipientUri(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseRecipientUri(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::std::istream &is,
                  const ::std::string &sid,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseRecipientUri(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(::LinphonePrivate::Xsd::Imdn::parseRecipientUri(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(::LinphonePrivate::Xsd::Imdn::parseRecipientUri(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::xercesc::InputSource &i,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(::LinphonePrivate::Xsd::Imdn::parseRecipientUri(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(const ::xercesc::DOMDocument &doc,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
		    ::LinphonePrivate::Xsd::Imdn::parseRecipientUri(std::move(d),
		                                                    f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "recipient-uri", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Uri>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Uri *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "recipient-uri",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseRecipientUri(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "recipient-uri", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Uri>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Uri *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "recipient-uri",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(const ::std::string &u,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::std::istream &is,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::std::istream &is,
                          const ::std::string &sid,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::xercesc::InputSource &i,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(const ::xercesc::DOMDocument &doc,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
		    ::LinphonePrivate::Xsd::Imdn::parseOriginalRecipientUri(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "original-recipient-uri", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Uri>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Uri *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "original-recipient-uri",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseOriginalRecipientUri(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "original-recipient-uri", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Uri>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Uri *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "original-recipient-uri",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(const ::std::string &u,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseSubject(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(const ::std::string &u,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseSubject(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(const ::std::string &u,
             ::xercesc::DOMErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseSubject(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseSubject(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseSubject(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             ::xercesc::DOMErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseSubject(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             const ::std::string &sid,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseSubject(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             const ::std::string &sid,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseSubject(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             const ::std::string &sid,
             ::xercesc::DOMErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseSubject(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::xercesc::InputSource &i,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseSubject(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::xercesc::InputSource &i,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseSubject(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::xercesc::InputSource &i,
             ::xercesc::DOMErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseSubject(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(const ::xercesc::DOMDocument &doc,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(::LinphonePrivate::Xsd::Imdn::parseSubject(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "subject", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::String>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::String *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "subject",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "subject", "urn:ietf:params:xml:ns:imdn",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::String>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::String *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "subject",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(const ::std::string &u,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::std::istream &is,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::std::istream &is,
                          const ::std::string &sid,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::xercesc::InputSource &i,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(const ::xercesc::DOMDocument &doc,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>(
		    ::LinphonePrivate::Xsd::Imdn::parseDeliveryNotification(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "delivery-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::DeliveryNotification, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "delivery-notification",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification>
parseDeliveryNotification(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "delivery-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::DeliveryNotification, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "delivery-notification",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>(::LinphonePrivate::Xsd::Imdn::parseDelivered(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>(::LinphonePrivate::Xsd::Imdn::parseDelivered(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(const ::std::string &u,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>(::LinphonePrivate::Xsd::Imdn::parseDelivered(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDelivered(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDelivered(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::std::istream &is,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDelivered(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDelivered(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDelivered(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::std::istream &is,
               const ::std::string &sid,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDelivered(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>(::LinphonePrivate::Xsd::Imdn::parseDelivered(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>(::LinphonePrivate::Xsd::Imdn::parseDelivered(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::xercesc::InputSource &i,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>(::LinphonePrivate::Xsd::Imdn::parseDelivered(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(const ::xercesc::DOMDocument &doc,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>(::LinphonePrivate::Xsd::Imdn::parseDelivered(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "delivered" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Delivered, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "delivered",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered>
parseDelivered(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "delivered" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Delivered, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "delivered",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(const ::std::string &u,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>(::LinphonePrivate::Xsd::Imdn::parseFailed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(const ::std::string &u,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>(::LinphonePrivate::Xsd::Imdn::parseFailed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(const ::std::string &u,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>(::LinphonePrivate::Xsd::Imdn::parseFailed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseFailed(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseFailed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseFailed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            const ::std::string &sid,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseFailed(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            const ::std::string &sid,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseFailed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            const ::std::string &sid,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseFailed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::xercesc::InputSource &i,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>(::LinphonePrivate::Xsd::Imdn::parseFailed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::xercesc::InputSource &i,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>(::LinphonePrivate::Xsd::Imdn::parseFailed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::xercesc::InputSource &i,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>(::LinphonePrivate::Xsd::Imdn::parseFailed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(const ::xercesc::DOMDocument &doc,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>(::LinphonePrivate::Xsd::Imdn::parseFailed(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "failed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Failed, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "failed", "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "failed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Failed, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "failed", "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(const ::std::string &u,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(const ::std::string &u,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(const ::std::string &u,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::std::istream &is,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::std::istream &is,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::std::istream &is,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::std::istream &is,
                         const ::std::string &sid,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::std::istream &is,
                         const ::std::string &sid,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::std::istream &is,
                         const ::std::string &sid,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::xercesc::InputSource &i,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::xercesc::InputSource &i,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::xercesc::InputSource &i,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(const ::xercesc::DOMDocument &doc,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>(
		    ::LinphonePrivate::Xsd::Imdn::parseDisplayNotification(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "display-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::DisplayNotification, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "display-notification",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification>
parseDisplayNotification(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "display-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::DisplayNotification, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "display-notification",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>(::LinphonePrivate::Xsd::Imdn::parseDisplayed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>(::LinphonePrivate::Xsd::Imdn::parseDisplayed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(const ::std::string &u,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>(::LinphonePrivate::Xsd::Imdn::parseDisplayed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayed(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::std::istream &is,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayed(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::std::istream &is,
               const ::std::string &sid,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseDisplayed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>(::LinphonePrivate::Xsd::Imdn::parseDisplayed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>(::LinphonePrivate::Xsd::Imdn::parseDisplayed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::xercesc::InputSource &i,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>(::LinphonePrivate::Xsd::Imdn::parseDisplayed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(const ::xercesc::DOMDocument &doc,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>(::LinphonePrivate::Xsd::Imdn::parseDisplayed(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "displayed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Displayed, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "displayed",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed>
parseDisplayed(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "displayed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Displayed, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "displayed",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(const ::std::string &u,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(const ::std::string &u,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(const ::std::string &u,
                            ::xercesc::DOMErrorHandler &h,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::std::istream &is,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::std::istream &is,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::std::istream &is,
                            ::xercesc::DOMErrorHandler &h,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::std::istream &is,
                            const ::std::string &sid,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::std::istream &is,
                            const ::std::string &sid,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::std::istream &is,
                            const ::std::string &sid,
                            ::xercesc::DOMErrorHandler &h,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::xercesc::InputSource &i,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::xercesc::InputSource &i,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::xercesc::InputSource &i,
                            ::xercesc::DOMErrorHandler &h,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>(
	    ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(const ::xercesc::DOMDocument &doc,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>(
		    ::LinphonePrivate::Xsd::Imdn::parseProcessingNotification(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "processing-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::ProcessingNotification, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "processing-notification",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification>
parseProcessingNotification(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                            const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "processing-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::ProcessingNotification, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "processing-notification",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>(::LinphonePrivate::Xsd::Imdn::parseProcessed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>(::LinphonePrivate::Xsd::Imdn::parseProcessed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(const ::std::string &u,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>(::LinphonePrivate::Xsd::Imdn::parseProcessed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessed(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::std::istream &is,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessed(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::std::istream &is,
               const ::std::string &sid,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseProcessed(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>(::LinphonePrivate::Xsd::Imdn::parseProcessed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>(::LinphonePrivate::Xsd::Imdn::parseProcessed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::xercesc::InputSource &i,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>(::LinphonePrivate::Xsd::Imdn::parseProcessed(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(const ::xercesc::DOMDocument &doc,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>(::LinphonePrivate::Xsd::Imdn::parseProcessed(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "processed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Processed, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "processed",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed>
parseProcessed(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "processed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Processed, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "processed",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(const ::std::string &u,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>(::LinphonePrivate::Xsd::Imdn::parseStored(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(const ::std::string &u,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>(::LinphonePrivate::Xsd::Imdn::parseStored(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(const ::std::string &u,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>(::LinphonePrivate::Xsd::Imdn::parseStored(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseStored(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseStored(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseStored(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            const ::std::string &sid,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseStored(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            const ::std::string &sid,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseStored(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            const ::std::string &sid,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseStored(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::xercesc::InputSource &i,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>(::LinphonePrivate::Xsd::Imdn::parseStored(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::xercesc::InputSource &i,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>(::LinphonePrivate::Xsd::Imdn::parseStored(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::xercesc::InputSource &i,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>(::LinphonePrivate::Xsd::Imdn::parseStored(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(const ::xercesc::DOMDocument &doc,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>(::LinphonePrivate::Xsd::Imdn::parseStored(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "stored" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Stored, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "stored", "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "stored" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Stored, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "stored", "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>(::LinphonePrivate::Xsd::Imdn::parseForbidden(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>(::LinphonePrivate::Xsd::Imdn::parseForbidden(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(const ::std::string &u,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>(::LinphonePrivate::Xsd::Imdn::parseForbidden(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseForbidden(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseForbidden(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::std::istream &is,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseForbidden(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseForbidden(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseForbidden(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::std::istream &is,
               const ::std::string &sid,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseForbidden(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>(::LinphonePrivate::Xsd::Imdn::parseForbidden(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>(::LinphonePrivate::Xsd::Imdn::parseForbidden(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::xercesc::InputSource &i,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>(::LinphonePrivate::Xsd::Imdn::parseForbidden(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(const ::xercesc::DOMDocument &doc,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>(::LinphonePrivate::Xsd::Imdn::parseForbidden(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "forbidden" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Forbidden, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "forbidden",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden>
parseForbidden(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "forbidden" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Forbidden, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "forbidden",
	                                                 "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(const ::std::string &u,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>(::LinphonePrivate::Xsd::Imdn::parseError(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(const ::std::string &u,
           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>(::LinphonePrivate::Xsd::Imdn::parseError(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(const ::std::string &u,
           ::xercesc::DOMErrorHandler &h,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>(::LinphonePrivate::Xsd::Imdn::parseError(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseError(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseError(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           ::xercesc::DOMErrorHandler &h,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::Imdn::parseError(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           const ::std::string &sid,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseError(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           const ::std::string &sid,
           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseError(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           const ::std::string &sid,
           ::xercesc::DOMErrorHandler &h,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::Imdn::parseError(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::xercesc::InputSource &i,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>(::LinphonePrivate::Xsd::Imdn::parseError(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::xercesc::InputSource &i,
           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>(::LinphonePrivate::Xsd::Imdn::parseError(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::xercesc::InputSource &i,
           ::xercesc::DOMErrorHandler &h,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>(::LinphonePrivate::Xsd::Imdn::parseError(
	    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(const ::xercesc::DOMDocument &doc,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>(::LinphonePrivate::Xsd::Imdn::parseError(
		    std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "error" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Error, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "error", "urn:ietf:params:xml:ns:imdn");
}

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> c(
	    ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
	     !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
	        ? static_cast<::xercesc::DOMDocument *>(d->cloneNode(true))
	        : 0);

	::xercesc::DOMDocument &doc(c.get() ? *c : *d);
	const ::xercesc::DOMElement &e(*doc.getDocumentElement());

	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
		doc.setUserData(::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey, (c.get() ? &c : &d), 0);

	if (n.name() == "error" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::Imdn::Error, char>::create(e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "error", "urn:ietf:params:xml:ns:imdn");
}
} // namespace Imdn
} // namespace Xsd
} // namespace LinphonePrivate

#include <ostream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/dom/serialization-source.hxx>

#include <xsd/cxx/tree/type-serializer-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::type_serializer_plate<0, char> type_serializer_plate_init;
}

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
void serializeImdn(::std::ostream &o,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   const ::std::string &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeImdn(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeImdn(::std::ostream &o,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &s,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   const ::std::string &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeImdn(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeImdn(::std::ostream &o,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &s,
                   ::xercesc::DOMErrorHandler &h,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   const ::std::string &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeImdn(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeImdn(::xercesc::XMLFormatTarget &t,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   const ::std::string &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeImdn(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeImdn(::xercesc::XMLFormatTarget &t,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &s,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   const ::std::string &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeImdn(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeImdn(::xercesc::XMLFormatTarget &t,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &s,
                   ::xercesc::DOMErrorHandler &h,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   const ::std::string &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeImdn(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeImdn(::xercesc::DOMDocument &d,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &s,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "imdn" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "imdn",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeImdn(const ::LinphonePrivate::Xsd::Imdn::Imdn &s,
              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("imdn", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeImdn(*d, s, f);
	return d;
}

void serializeMessageId(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeMessageId(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeMessageId(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeMessageId(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeMessageId(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeMessageId(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeMessageId(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeMessageId(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeMessageId(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeMessageId(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeMessageId(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeMessageId(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeMessageId(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &s,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Token) == typeid(s)) {
		if (n.name() == "message-id" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "message-id",
			                                                 "urn:ietf:params:xml:ns:imdn");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("message-id", "urn:ietf:params:xml:ns:imdn",
		                                                                    e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeMessageId(const ::LinphonePrivate::Xsd::XmlSchema::Token &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Token) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("message-id", "urn:ietf:params:xml:ns:imdn", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("message-id",
		                                                                        "urn:ietf:params:xml:ns:imdn", m, s, f);
	}

	::LinphonePrivate::Xsd::Imdn::serializeMessageId(*d, s, f);
	return d;
}

void serializeDatetime(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDatetime(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDatetime(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDatetime(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDatetime(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                       ::xercesc::DOMErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDatetime(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDatetime(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDatetime(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDatetime(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDatetime(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDatetime(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                       ::xercesc::DOMErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDatetime(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDatetime(::xercesc::DOMDocument &d,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::String) == typeid(s)) {
		if (n.name() == "datetime" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "datetime",
			                                                 "urn:ietf:params:xml:ns:imdn");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("datetime", "urn:ietf:params:xml:ns:imdn",
		                                                                    e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDatetime(const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::String) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("datetime", "urn:ietf:params:xml:ns:imdn", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("datetime",
		                                                                        "urn:ietf:params:xml:ns:imdn", m, s, f);
	}

	::LinphonePrivate::Xsd::Imdn::serializeDatetime(*d, s, f);
	return d;
}

void serializeRecipientUri(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeRecipientUri(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeRecipientUri(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeRecipientUri(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeRecipientUri(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeRecipientUri(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeRecipientUri(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeRecipientUri(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeRecipientUri(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeRecipientUri(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeRecipientUri(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeRecipientUri(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeRecipientUri(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Uri) == typeid(s)) {
		if (n.name() == "recipient-uri" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "recipient-uri",
			                                                 "urn:ietf:params:xml:ns:imdn");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("recipient-uri",
		                                                                    "urn:ietf:params:xml:ns:imdn", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeRecipientUri(const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Uri) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("recipient-uri", "urn:ietf:params:xml:ns:imdn", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("recipient-uri",
		                                                                        "urn:ietf:params:xml:ns:imdn", m, s, f);
	}

	::LinphonePrivate::Xsd::Imdn::serializeRecipientUri(*d, s, f);
	return d;
}

void serializeOriginalRecipientUri(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeOriginalRecipientUri(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeOriginalRecipientUri(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeOriginalRecipientUri(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeOriginalRecipientUri(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeOriginalRecipientUri(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeOriginalRecipientUri(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeOriginalRecipientUri(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeOriginalRecipientUri(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeOriginalRecipientUri(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeOriginalRecipientUri(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeOriginalRecipientUri(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeOriginalRecipientUri(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Uri) == typeid(s)) {
		if (n.name() == "original-recipient-uri" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "original-recipient-uri",
			                                                 "urn:ietf:params:xml:ns:imdn");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("original-recipient-uri",
		                                                                    "urn:ietf:params:xml:ns:imdn", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeOriginalRecipientUri(const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Uri) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("original-recipient-uri", "urn:ietf:params:xml:ns:imdn", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("original-recipient-uri",
		                                                                        "urn:ietf:params:xml:ns:imdn", m, s, f);
	}

	::LinphonePrivate::Xsd::Imdn::serializeOriginalRecipientUri(*d, s, f);
	return d;
}

void serializeSubject(::std::ostream &o,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeSubject(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSubject(::std::ostream &o,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeSubject(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSubject(::std::ostream &o,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                      ::xercesc::DOMErrorHandler &h,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeSubject(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSubject(::xercesc::XMLFormatTarget &t,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeSubject(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSubject(::xercesc::XMLFormatTarget &t,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeSubject(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSubject(::xercesc::XMLFormatTarget &t,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                      ::xercesc::DOMErrorHandler &h,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeSubject(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSubject(::xercesc::DOMDocument &d,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::String) == typeid(s)) {
		if (n.name() == "subject" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "subject",
			                                                 "urn:ietf:params:xml:ns:imdn");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("subject", "urn:ietf:params:xml:ns:imdn", e,
		                                                                    n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSubject(const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::String) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("subject", "urn:ietf:params:xml:ns:imdn", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize("subject",
		                                                                        "urn:ietf:params:xml:ns:imdn", m, s, f);
	}

	::LinphonePrivate::Xsd::Imdn::serializeSubject(*d, s, f);
	return d;
}

void serializeDeliveryNotification(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDeliveryNotification(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDeliveryNotification(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDeliveryNotification(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDeliveryNotification(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDeliveryNotification(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDeliveryNotification(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDeliveryNotification(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDeliveryNotification(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDeliveryNotification(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDeliveryNotification(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDeliveryNotification(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDeliveryNotification(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "delivery-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "delivery-notification",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDeliveryNotification(const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("delivery-notification", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeDeliveryNotification(*d, s, f);
	return d;
}

void serializeDelivered(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDelivered(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDelivered(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDelivered(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDelivered(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDelivered(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDelivered(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDelivered(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDelivered(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDelivered(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDelivered(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDelivered(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDelivered(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &s,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "delivered" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "delivered",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDelivered(const ::LinphonePrivate::Xsd::Imdn::Delivered &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("delivered", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeDelivered(*d, s, f);
	return d;
}

void serializeFailed(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeFailed(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeFailed(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &s,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeFailed(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFailed(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &s,
                     ::xercesc::DOMErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeFailed(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFailed(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeFailed(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeFailed(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &s,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeFailed(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFailed(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &s,
                     ::xercesc::DOMErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeFailed(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFailed(::xercesc::DOMDocument &d,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &s,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "failed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "failed",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeFailed(const ::LinphonePrivate::Xsd::Imdn::Failed &s,
                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("failed", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeFailed(*d, s, f);
	return d;
}

void serializeDisplayNotification(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &s,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayNotification(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDisplayNotification(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayNotification(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDisplayNotification(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &s,
                                  ::xercesc::DOMErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayNotification(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDisplayNotification(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &s,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayNotification(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDisplayNotification(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayNotification(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDisplayNotification(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &s,
                                  ::xercesc::DOMErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayNotification(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDisplayNotification(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "display-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "display-notification",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDisplayNotification(const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &s,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("display-notification", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeDisplayNotification(*d, s, f);
	return d;
}

void serializeDisplayed(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayed(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDisplayed(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayed(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDisplayed(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayed(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDisplayed(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayed(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDisplayed(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayed(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDisplayed(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeDisplayed(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDisplayed(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &s,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "displayed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "displayed",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDisplayed(const ::LinphonePrivate::Xsd::Imdn::Displayed &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("displayed", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeDisplayed(*d, s, f);
	return d;
}

void serializeProcessingNotification(::std::ostream &o,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &s,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                     const ::std::string &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessingNotification(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeProcessingNotification(::std::ostream &o,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &s,
                                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                     const ::std::string &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessingNotification(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProcessingNotification(::std::ostream &o,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &s,
                                     ::xercesc::DOMErrorHandler &h,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                     const ::std::string &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessingNotification(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProcessingNotification(::xercesc::XMLFormatTarget &t,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &s,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                     const ::std::string &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessingNotification(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeProcessingNotification(::xercesc::XMLFormatTarget &t,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &s,
                                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                     const ::std::string &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessingNotification(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProcessingNotification(::xercesc::XMLFormatTarget &t,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &s,
                                     ::xercesc::DOMErrorHandler &h,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                     const ::std::string &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessingNotification(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProcessingNotification(::xercesc::DOMDocument &d,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &s,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "processing-notification" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "processing-notification",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeProcessingNotification(const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &s,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("processing-notification", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeProcessingNotification(*d, s, f);
	return d;
}

void serializeProcessed(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessed(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeProcessed(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessed(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProcessed(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessed(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProcessed(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessed(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeProcessed(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessed(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProcessed(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeProcessed(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProcessed(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &s,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "processed" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "processed",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeProcessed(const ::LinphonePrivate::Xsd::Imdn::Processed &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("processed", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeProcessed(*d, s, f);
	return d;
}

void serializeStored(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeStored(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeStored(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &s,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeStored(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeStored(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &s,
                     ::xercesc::DOMErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeStored(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeStored(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeStored(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeStored(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &s,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeStored(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeStored(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &s,
                     ::xercesc::DOMErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeStored(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeStored(::xercesc::DOMDocument &d,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &s,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "stored" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "stored",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeStored(const ::LinphonePrivate::Xsd::Imdn::Stored &s,
                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("stored", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeStored(*d, s, f);
	return d;
}

void serializeForbidden(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeForbidden(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeForbidden(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeForbidden(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeForbidden(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeForbidden(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeForbidden(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeForbidden(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeForbidden(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeForbidden(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeForbidden(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeForbidden(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeForbidden(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &s,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "forbidden" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "forbidden",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeForbidden(const ::LinphonePrivate::Xsd::Imdn::Forbidden &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("forbidden", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeForbidden(*d, s, f);
	return d;
}

void serializeError(::std::ostream &o,
                    const ::LinphonePrivate::Xsd::Imdn::Error &s,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                    const ::std::string &e,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeError(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeError(::std::ostream &o,
                    const ::LinphonePrivate::Xsd::Imdn::Error &s,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                    const ::std::string &e,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeError(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeError(::std::ostream &o,
                    const ::LinphonePrivate::Xsd::Imdn::Error &s,
                    ::xercesc::DOMErrorHandler &h,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                    const ::std::string &e,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeError(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeError(::xercesc::XMLFormatTarget &t,
                    const ::LinphonePrivate::Xsd::Imdn::Error &s,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                    const ::std::string &e,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeError(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeError(::xercesc::XMLFormatTarget &t,
                    const ::LinphonePrivate::Xsd::Imdn::Error &s,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                    const ::std::string &e,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeError(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeError(::xercesc::XMLFormatTarget &t,
                    const ::LinphonePrivate::Xsd::Imdn::Error &s,
                    ::xercesc::DOMErrorHandler &h,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                    const ::std::string &e,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::Imdn::serializeError(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeError(::xercesc::DOMDocument &d,
                    const ::LinphonePrivate::Xsd::Imdn::Error &s,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "error" && n.namespace_() == "urn:ietf:params:xml:ns:imdn") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "error",
		                                                 "urn:ietf:params:xml:ns:imdn");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeError(const ::LinphonePrivate::Xsd::Imdn::Error &s,
               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("error", "urn:ietf:params:xml:ns:imdn", m, f));

	::LinphonePrivate::Xsd::Imdn::serializeError(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const Imdn &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// message-id
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const Imdn::MessageIdType &x(i.getMessageId());
		if (typeid(Imdn::MessageIdType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("message-id", "urn:ietf:params:xml:ns:imdn", e));

			s << x;
		} else tsm.serialize("message-id", "urn:ietf:params:xml:ns:imdn", true, true, e, x);
	}

	// datetime
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const Imdn::DatetimeType &x(i.getDatetime());
		if (typeid(Imdn::DatetimeType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("datetime", "urn:ietf:params:xml:ns:imdn", e));

			s << x;
		} else tsm.serialize("datetime", "urn:ietf:params:xml:ns:imdn", true, true, e, x);
	}

	// recipient-uri
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getRecipientUri()) {
			const Imdn::RecipientUriType &x(*i.getRecipientUri());
			if (typeid(Imdn::RecipientUriType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("recipient-uri", "urn:ietf:params:xml:ns:imdn", e));

				s << x;
			} else tsm.serialize("recipient-uri", "urn:ietf:params:xml:ns:imdn", true, true, e, x);
		}
	}

	// original-recipient-uri
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getOriginalRecipientUri()) {
			const Imdn::OriginalRecipientUriType &x(*i.getOriginalRecipientUri());
			if (typeid(Imdn::OriginalRecipientUriType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("original-recipient-uri", "urn:ietf:params:xml:ns:imdn", e));

				s << x;
			} else tsm.serialize("original-recipient-uri", "urn:ietf:params:xml:ns:imdn", true, true, e, x);
		}
	}

	// subject
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSubject()) {
			const Imdn::SubjectType &x(*i.getSubject());
			if (typeid(Imdn::SubjectType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("subject", "urn:ietf:params:xml:ns:imdn", e));

				s << x;
			} else tsm.serialize("subject", "urn:ietf:params:xml:ns:imdn", true, true, e, x);
		}
	}

	// delivery-notification
	//
	if (i.getDeliveryNotification()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("delivery-notification", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getDeliveryNotification();
	}

	// display-notification
	//
	if (i.getDisplayNotification()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("display-notification", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getDisplayNotification();
	}

	// processing-notification
	//
	if (i.getProcessingNotification()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("processing-notification", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getProcessingNotification();
	}

	// any
	//
	for (Imdn::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

void operator<<(::xercesc::DOMElement &e, const DeliveryNotification &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// status
	//
	{
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("status", "urn:ietf:params:xml:ns:imdn", e));

		s << i.getStatus();
	}
}

void operator<<(::xercesc::DOMElement &e, const Delivered &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);
}

void operator<<(::xercesc::DOMAttr &, const Delivered &) {
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Delivered &) {
}

void operator<<(::xercesc::DOMElement &e, const Failed &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);
}

void operator<<(::xercesc::DOMAttr &, const Failed &) {
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Failed &) {
}

void operator<<(::xercesc::DOMElement &e, const DisplayNotification &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// status
	//
	{
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("status", "urn:ietf:params:xml:ns:imdn", e));

		s << i.getStatus();
	}
}

void operator<<(::xercesc::DOMElement &e, const Displayed &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);
}

void operator<<(::xercesc::DOMAttr &, const Displayed &) {
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Displayed &) {
}

void operator<<(::xercesc::DOMElement &e, const ProcessingNotification &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// status
	//
	{
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("status", "urn:ietf:params:xml:ns:imdn", e));

		s << i.getStatus();
	}
}

void operator<<(::xercesc::DOMElement &e, const Processed &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);
}

void operator<<(::xercesc::DOMAttr &, const Processed &) {
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Processed &) {
}

void operator<<(::xercesc::DOMElement &e, const Stored &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);
}

void operator<<(::xercesc::DOMAttr &, const Stored &) {
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Stored &) {
}

void operator<<(::xercesc::DOMElement &e, const Forbidden &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);
}

void operator<<(::xercesc::DOMAttr &, const Forbidden &) {
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Forbidden &) {
}

void operator<<(::xercesc::DOMElement &e, const Error &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);
}

void operator<<(::xercesc::DOMAttr &, const Error &) {
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Error &) {
}

void operator<<(::xercesc::DOMElement &e, const Status &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// delivered
	//
	if (i.getDelivered()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("delivered", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getDelivered();
	}

	// failed
	//
	if (i.getFailed()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("failed", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getFailed();
	}

	// forbidden
	//
	if (i.getForbidden()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("forbidden", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getForbidden();
	}

	// error
	//
	if (i.getError()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("error", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getError();
	}

	// reason
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getReason()) {
			const Status::ReasonType &x(*i.getReason());
			if (typeid(Status::ReasonType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("reason", "http://www.linphone.org/xsds/imdn.xsd", e));

				s << x;
			} else tsm.serialize("reason", "http://www.linphone.org/xsds/imdn.xsd", true, true, e, x);
		}
	}
}

void operator<<(::xercesc::DOMElement &e, const Status1 &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// displayed
	//
	if (i.getDisplayed()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("displayed", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getDisplayed();
	}

	// forbidden
	//
	if (i.getForbidden()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("forbidden", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getForbidden();
	}

	// error
	//
	if (i.getError()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("error", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getError();
	}

	// any
	//
	for (Status1::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

void operator<<(::xercesc::DOMElement &e, const Status2 &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// processed
	//
	if (i.getProcessed()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("processed", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getProcessed();
	}

	// stored
	//
	if (i.getStored()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("stored", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getStored();
	}

	// forbidden
	//
	if (i.getForbidden()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("forbidden", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getForbidden();
	}

	// error
	//
	if (i.getError()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("error", "urn:ietf:params:xml:ns:imdn", e));

		s << *i.getError();
	}

	// any
	//
	for (Status2::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}
} // namespace Imdn
} // namespace Xsd
} // namespace LinphonePrivate

#include <xsd/cxx/post.hxx>

// Begin epilogue.
//
#if __GNUC__ >= 7
#pragma GCC diagnostic pop
#endif
#if __GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1)
#pragma GCC diagnostic pop
#endif
#if __clang__ || __GNUC__ >= 4
#pragma GCC diagnostic pop
#endif
//
// End epilogue.
