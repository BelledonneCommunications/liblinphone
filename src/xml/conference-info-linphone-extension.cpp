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

#include "conference-info-linphone-extension.h"

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfoLinphoneExtension {
// ModeType
//

ModeType::ModeType(const char *s) : ::LinphonePrivate::Xsd::XmlSchema::String(s) {
}

ModeType::ModeType(const ::std::string &s) : ::LinphonePrivate::Xsd::XmlSchema::String(s) {
}

ModeType::ModeType(const ModeType &o,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(o, f, c) {
}

// ModeEnum
//

ModeEnum::ModeEnum(Value v) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_ModeEnum_literals_[v]) {
}

ModeEnum::ModeEnum(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

ModeEnum::ModeEnum(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

ModeEnum::ModeEnum(const ::LinphonePrivate::Xsd::XmlSchema::String &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

ModeEnum::ModeEnum(const ModeEnum &v,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v, f, c) {
}

ModeEnum &ModeEnum::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::String &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_ModeEnum_literals_[v]);

	return *this;
}

// Ephemeral
//

const Ephemeral::ModeType &Ephemeral::getMode() const {
	return this->mode_.get();
}

Ephemeral::ModeType &Ephemeral::getMode() {
	return this->mode_.get();
}

void Ephemeral::setMode(const ModeType &x) {
	this->mode_.set(x);
}

void Ephemeral::setMode(::std::unique_ptr<ModeType> x) {
	this->mode_.set(std::move(x));
}

::std::unique_ptr<Ephemeral::ModeType> Ephemeral::setDetachMode() {
	return this->mode_.detach();
}

const Ephemeral::LifetimeType &Ephemeral::getLifetime() const {
	return this->lifetime_.get();
}

Ephemeral::LifetimeType &Ephemeral::getLifetime() {
	return this->lifetime_.get();
}

void Ephemeral::setLifetime(const LifetimeType &x) {
	this->lifetime_.set(x);
}

void Ephemeral::setLifetime(::std::unique_ptr<LifetimeType> x) {
	this->lifetime_.set(std::move(x));
}

::std::unique_ptr<Ephemeral::LifetimeType> Ephemeral::setDetachLifetime() {
	return this->lifetime_.detach();
}

const Ephemeral::AnySequence &Ephemeral::getAny() const {
	return this->any_;
}

Ephemeral::AnySequence &Ephemeral::getAny() {
	return this->any_;
}

void Ephemeral::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &Ephemeral::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Ephemeral::getDomDocument() {
	return *this->dom_document_;
}

// ServiceDescription
//

const ServiceDescription::ServiceIdType &ServiceDescription::getServiceId() const {
	return this->service_id_.get();
}

ServiceDescription::ServiceIdType &ServiceDescription::getServiceId() {
	return this->service_id_.get();
}

void ServiceDescription::setServiceId(const ServiceIdType &x) {
	this->service_id_.set(x);
}

void ServiceDescription::setServiceId(::std::unique_ptr<ServiceIdType> x) {
	this->service_id_.set(std::move(x));
}

::std::unique_ptr<ServiceDescription::ServiceIdType> ServiceDescription::setDetachService_id() {
	return this->service_id_.detach();
}

const ServiceDescription::VersionType &ServiceDescription::getVersion() const {
	return this->version_.get();
}

ServiceDescription::VersionType &ServiceDescription::getVersion() {
	return this->version_.get();
}

void ServiceDescription::setVersion(const VersionType &x) {
	this->version_.set(x);
}

void ServiceDescription::setVersion(::std::unique_ptr<VersionType> x) {
	this->version_.set(std::move(x));
}

::std::unique_ptr<ServiceDescription::VersionType> ServiceDescription::setDetachVersion() {
	return this->version_.detach();
}

const ServiceDescription::AnySequence &ServiceDescription::getAny() const {
	return this->any_;
}

ServiceDescription::AnySequence &ServiceDescription::getAny() {
	return this->any_;
}

void ServiceDescription::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &ServiceDescription::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ServiceDescription::getDomDocument() {
	return *this->dom_document_;
}

// CryptoSecurityLevel
//

const CryptoSecurityLevel::LevelType &CryptoSecurityLevel::getLevel() const {
	return this->level_.get();
}

CryptoSecurityLevel::LevelType &CryptoSecurityLevel::getLevel() {
	return this->level_.get();
}

void CryptoSecurityLevel::setLevel(const LevelType &x) {
	this->level_.set(x);
}

void CryptoSecurityLevel::setLevel(::std::unique_ptr<LevelType> x) {
	this->level_.set(std::move(x));
}

::std::unique_ptr<CryptoSecurityLevel::LevelType> CryptoSecurityLevel::setDetachLevel() {
	return this->level_.detach();
}

const CryptoSecurityLevel::AnySequence &CryptoSecurityLevel::getAny() const {
	return this->any_;
}

CryptoSecurityLevel::AnySequence &CryptoSecurityLevel::getAny() {
	return this->any_;
}

void CryptoSecurityLevel::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &CryptoSecurityLevel::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &CryptoSecurityLevel::getDomDocument() {
	return *this->dom_document_;
}

// ConferenceTimes
//

const ConferenceTimes::StartOptional &ConferenceTimes::getStart() const {
	return this->start_;
}

ConferenceTimes::StartOptional &ConferenceTimes::getStart() {
	return this->start_;
}

void ConferenceTimes::setStart(const StartType &x) {
	this->start_.set(x);
}

void ConferenceTimes::setStart(const StartOptional &x) {
	this->start_ = x;
}

void ConferenceTimes::setStart(::std::unique_ptr<StartType> x) {
	this->start_.set(std::move(x));
}

const ConferenceTimes::EndOptional &ConferenceTimes::getEnd() const {
	return this->end_;
}

ConferenceTimes::EndOptional &ConferenceTimes::getEnd() {
	return this->end_;
}

void ConferenceTimes::setEnd(const EndType &x) {
	this->end_.set(x);
}

void ConferenceTimes::setEnd(const EndOptional &x) {
	this->end_ = x;
}

void ConferenceTimes::setEnd(::std::unique_ptr<EndType> x) {
	this->end_.set(std::move(x));
}

// StreamData
//

const StreamData::StreamContentType &StreamData::getStreamContent() const {
	return this->stream_content_.get();
}

StreamData::StreamContentType &StreamData::getStreamContent() {
	return this->stream_content_.get();
}

void StreamData::setStreamContent(const StreamContentType &x) {
	this->stream_content_.set(x);
}

void StreamData::setStreamContent(::std::unique_ptr<StreamContentType> x) {
	this->stream_content_.set(std::move(x));
}

::std::unique_ptr<StreamData::StreamContentType> StreamData::setDetachStream_content() {
	return this->stream_content_.detach();
}

const StreamData::AnySequence &StreamData::getAny() const {
	return this->any_;
}

StreamData::AnySequence &StreamData::getAny() {
	return this->any_;
}

void StreamData::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &StreamData::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &StreamData::getDomDocument() {
	return *this->dom_document_;
}
} // namespace ConferenceInfoLinphoneExtension
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
namespace ConferenceInfoLinphoneExtension {
// ModeType
//

ModeType::ModeType(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

ModeType::ModeType(const ::xercesc::DOMAttr &a,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

ModeType::ModeType(const ::std::string &s,
                   const ::xercesc::DOMElement *e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

ModeType *ModeType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ModeType(*this, f, c);
}

// ModeEnum
//

ModeEnum::ModeEnum(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
	_xsd_ModeEnum_convert();
}

ModeEnum::ModeEnum(const ::xercesc::DOMAttr &a,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
	_xsd_ModeEnum_convert();
}

ModeEnum::ModeEnum(const ::std::string &s,
                   const ::xercesc::DOMElement *e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
	_xsd_ModeEnum_convert();
}

ModeEnum *ModeEnum::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ModeEnum(*this, f, c);
}

ModeEnum::Value ModeEnum::_xsd_ModeEnum_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_ModeEnum_literals_);
	const Value *i(::std::lower_bound(_xsd_ModeEnum_indexes_, _xsd_ModeEnum_indexes_ + 2, *this, c));

	if (i == _xsd_ModeEnum_indexes_ + 2 || _xsd_ModeEnum_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const ModeEnum::_xsd_ModeEnum_literals_[2] = {"device-managed", "admin-managed"};

const ModeEnum::Value ModeEnum::_xsd_ModeEnum_indexes_[2] = {
    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ModeEnum::admin_managed,
    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ModeEnum::device_managed};

// Ephemeral
//

Ephemeral::Ephemeral(const ModeType &mode, const LifetimeType &lifetime)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      mode_(mode, this), lifetime_(lifetime, this), any_(this->getDomDocument()) {
}

Ephemeral::Ephemeral(const Ephemeral &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      mode_(x.mode_, f, this), lifetime_(x.lifetime_, f, this), any_(x.any_, this->getDomDocument()) {
}

Ephemeral::Ephemeral(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), mode_(this), lifetime_(this),
      any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void Ephemeral::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// mode
		//
		if (n.name() == "mode" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::std::unique_ptr<ModeType> r(ModeTraits::create(i, f, this));

			if (!mode_.present()) {
				this->mode_.set(::std::move(r));
				continue;
			}
		}

		// lifetime
		//
		if (n.name() == "lifetime" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::std::unique_ptr<LifetimeType> r(LifetimeTraits::create(i, f, this));

			if (!lifetime_.present()) {
				this->lifetime_.set(::std::move(r));
				continue;
			}
		}

		// any
		//
		if (n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!mode_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("mode", "linphone:xml:ns:conference-info-linphone-extension");
	}

	if (!lifetime_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("lifetime",
		                                               "linphone:xml:ns:conference-info-linphone-extension");
	}
}

Ephemeral *Ephemeral::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Ephemeral(*this, f, c);
}

Ephemeral &Ephemeral::operator=(const Ephemeral &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->mode_ = x.mode_;
		this->lifetime_ = x.lifetime_;
		this->any_ = x.any_;
	}

	return *this;
}

Ephemeral::~Ephemeral() {
}

// ServiceDescription
//

ServiceDescription::ServiceDescription(const ServiceIdType &service_id, const VersionType &version)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      service_id_(service_id, this), version_(version, this), any_(this->getDomDocument()) {
}

ServiceDescription::ServiceDescription(const ServiceDescription &x,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      service_id_(x.service_id_, f, this), version_(x.version_, f, this), any_(x.any_, this->getDomDocument()) {
}

ServiceDescription::ServiceDescription(const ::xercesc::DOMElement &e,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), service_id_(this), version_(this),
      any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void ServiceDescription::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// service-id
		//
		if (n.name() == "service-id" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::std::unique_ptr<ServiceIdType> r(ServiceIdTraits::create(i, f, this));

			if (!service_id_.present()) {
				this->service_id_.set(::std::move(r));
				continue;
			}
		}

		// version
		//
		if (n.name() == "version" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::std::unique_ptr<VersionType> r(VersionTraits::create(i, f, this));

			if (!version_.present()) {
				this->version_.set(::std::move(r));
				continue;
			}
		}

		// any
		//
		if (n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!service_id_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("service-id",
		                                               "linphone:xml:ns:conference-info-linphone-extension");
	}

	if (!version_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("version", "linphone:xml:ns:conference-info-linphone-extension");
	}
}

ServiceDescription *ServiceDescription::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ServiceDescription(*this, f, c);
}

ServiceDescription &ServiceDescription::operator=(const ServiceDescription &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->service_id_ = x.service_id_;
		this->version_ = x.version_;
		this->any_ = x.any_;
	}

	return *this;
}

ServiceDescription::~ServiceDescription() {
}

// CryptoSecurityLevel
//

CryptoSecurityLevel::CryptoSecurityLevel(const LevelType &level)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      level_(level, this), any_(this->getDomDocument()) {
}

CryptoSecurityLevel::CryptoSecurityLevel(const CryptoSecurityLevel &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      level_(x.level_, f, this), any_(x.any_, this->getDomDocument()) {
}

CryptoSecurityLevel::CryptoSecurityLevel(const ::xercesc::DOMElement &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), level_(this), any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void CryptoSecurityLevel::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// level
		//
		if (n.name() == "level" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::std::unique_ptr<LevelType> r(LevelTraits::create(i, f, this));

			if (!level_.present()) {
				this->level_.set(::std::move(r));
				continue;
			}
		}

		// any
		//
		if (n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!level_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("level", "linphone:xml:ns:conference-info-linphone-extension");
	}
}

CryptoSecurityLevel *CryptoSecurityLevel::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CryptoSecurityLevel(*this, f, c);
}

CryptoSecurityLevel &CryptoSecurityLevel::operator=(const CryptoSecurityLevel &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->level_ = x.level_;
		this->any_ = x.any_;
	}

	return *this;
}

CryptoSecurityLevel::~CryptoSecurityLevel() {
}

// ConferenceTimes
//

ConferenceTimes::ConferenceTimes() : ::LinphonePrivate::Xsd::XmlSchema::Type(), start_(this), end_(this) {
}

ConferenceTimes::ConferenceTimes(const ConferenceTimes &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), start_(x.start_, f, this), end_(x.end_, f, this) {
}

ConferenceTimes::ConferenceTimes(const ::xercesc::DOMElement &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c), start_(this),
      end_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void ConferenceTimes::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// start
		//
		if (n.name() == "start" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::std::unique_ptr<StartType> r(StartTraits::create(i, f, this));

			if (!this->start_) {
				this->start_.set(::std::move(r));
				continue;
			}
		}

		// end
		//
		if (n.name() == "end" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::std::unique_ptr<EndType> r(EndTraits::create(i, f, this));

			if (!this->end_) {
				this->end_.set(::std::move(r));
				continue;
			}
		}

		break;
	}
}

ConferenceTimes *ConferenceTimes::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceTimes(*this, f, c);
}

ConferenceTimes &ConferenceTimes::operator=(const ConferenceTimes &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->start_ = x.start_;
		this->end_ = x.end_;
	}

	return *this;
}

ConferenceTimes::~ConferenceTimes() {
}

// StreamData
//

StreamData::StreamData(const StreamContentType &stream_content)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      stream_content_(stream_content, this), any_(this->getDomDocument()) {
}

StreamData::StreamData(const StreamData &x,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      stream_content_(x.stream_content_, f, this), any_(x.any_, this->getDomDocument()) {
}

StreamData::StreamData(const ::xercesc::DOMElement &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), stream_content_(this),
      any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void StreamData::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// stream-content
		//
		if (n.name() == "stream-content" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::std::unique_ptr<StreamContentType> r(StreamContentTraits::create(i, f, this));

			if (!stream_content_.present()) {
				this->stream_content_.set(::std::move(r));
				continue;
			}
		}

		// any
		//
		if (n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!stream_content_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("stream-content",
		                                               "linphone:xml:ns:conference-info-linphone-extension");
	}
}

StreamData *StreamData::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class StreamData(*this, f, c);
}

StreamData &StreamData::operator=(const StreamData &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->stream_content_ = x.stream_content_;
		this->any_ = x.any_;
	}

	return *this;
}

StreamData::~StreamData() {
}
} // namespace ConferenceInfoLinphoneExtension
} // namespace Xsd
} // namespace LinphonePrivate

#include <ostream>

#include <xsd/cxx/tree/std-ostream-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::std_ostream_plate<0, char> std_ostream_plate_init;
}

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfoLinphoneExtension {
::std::ostream &operator<<(::std::ostream &o, const ModeType &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

::std::ostream &operator<<(::std::ostream &o, ModeEnum::Value i) {
	return o << ModeEnum::_xsd_ModeEnum_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const ModeEnum &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

::std::ostream &operator<<(::std::ostream &o, const Ephemeral &i) {
	o << ::std::endl << "mode: " << i.getMode();
	o << ::std::endl << "lifetime: " << i.getLifetime();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const ServiceDescription &i) {
	o << ::std::endl << "service-id: " << i.getServiceId();
	o << ::std::endl << "version: " << i.getVersion();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const CryptoSecurityLevel &i) {
	o << ::std::endl << "level: " << i.getLevel();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const ConferenceTimes &i) {
	if (i.getStart()) {
		o << ::std::endl << "start: " << *i.getStart();
	}

	if (i.getEnd()) {
		o << ::std::endl << "end: " << *i.getEnd();
	}

	return o;
}

::std::ostream &operator<<(::std::ostream &o, const StreamData &i) {
	o << ::std::endl << "stream-content: " << i.getStreamContent();
	return o;
}
} // namespace ConferenceInfoLinphoneExtension
} // namespace Xsd
} // namespace LinphonePrivate

#include <istream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/sax/std-input-source.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfoLinphoneExtension {
::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(const ::std::string &u,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::std::istream &is,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::std::istream &is,
               const ::std::string &sid,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::xercesc::InputSource &i,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(const ::xercesc::DOMDocument &doc,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>(
		    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseEphemeral(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "ephemeral" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
		::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral, char>::create(
		        e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ephemeral",
	                                                 "linphone:xml:ns:conference-info-linphone-extension");
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral>
parseEphemeral(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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

	if (n.name() == "ephemeral" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
		::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral, char>::create(
		        e, f, 0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ephemeral",
	                                                 "linphone:xml:ns:conference-info-linphone-extension");
}
} // namespace ConferenceInfoLinphoneExtension
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
namespace ConferenceInfoLinphoneExtension {
void operator<<(::xercesc::DOMElement &e, const ModeType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const ModeType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const ModeType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMElement &e, const ModeEnum &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const ModeEnum &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const ModeEnum &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void serializeEphemeral(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeEphemeral(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeEphemeral(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeEphemeral(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeEphemeral(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeEphemeral(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeEphemeral(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeEphemeral(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeEphemeral(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeEphemeral(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeEphemeral(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeEphemeral(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeEphemeral(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &s,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "ephemeral" && n.namespace_() == "linphone:xml:ns:conference-info-linphone-extension") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ephemeral",
		                                                 "linphone:xml:ns:conference-info-linphone-extension");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeEphemeral(const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("ephemeral", "linphone:xml:ns:conference-info-linphone-extension", m, f));

	::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeEphemeral(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const Ephemeral &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// mode
	//
	{
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("mode", "linphone:xml:ns:conference-info-linphone-extension", e));

		s << i.getMode();
	}

	// lifetime
	//
	{
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("lifetime", "linphone:xml:ns:conference-info-linphone-extension", e));

		s << i.getLifetime();
	}

	// any
	//
	for (Ephemeral::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

void operator<<(::xercesc::DOMElement &e, const ServiceDescription &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// service-id
	//
	{
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
		    "service-id", "linphone:xml:ns:conference-info-linphone-extension", e));

		s << i.getServiceId();
	}

	// version
	//
	{
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("version", "linphone:xml:ns:conference-info-linphone-extension", e));

		s << i.getVersion();
	}

	// any
	//
	for (ServiceDescription::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

void operator<<(::xercesc::DOMElement &e, const CryptoSecurityLevel &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// level
	//
	{
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("level", "linphone:xml:ns:conference-info-linphone-extension", e));

		s << i.getLevel();
	}

	// any
	//
	for (CryptoSecurityLevel::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

void operator<<(::xercesc::DOMElement &e, const ConferenceTimes &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// start
	//
	if (i.getStart()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("start", "linphone:xml:ns:conference-info-linphone-extension", e));

		s << *i.getStart();
	}

	// end
	//
	if (i.getEnd()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("end", "linphone:xml:ns:conference-info-linphone-extension", e));

		s << *i.getEnd();
	}
}

void operator<<(::xercesc::DOMElement &e, const StreamData &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// stream-content
	//
	{
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
		    "stream-content", "linphone:xml:ns:conference-info-linphone-extension", e));

		s << i.getStreamContent();
	}

	// any
	//
	for (StreamData::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}
} // namespace ConferenceInfoLinphoneExtension
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
