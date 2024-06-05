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

#include "conference-info.h"

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfo {
// ConferenceType
//

const ConferenceType::ConferenceDescriptionOptional &ConferenceType::getConferenceDescription() const {
	return this->conference_description_;
}

ConferenceType::ConferenceDescriptionOptional &ConferenceType::getConferenceDescription() {
	return this->conference_description_;
}

void ConferenceType::setConferenceDescription(const ConferenceDescriptionType &x) {
	this->conference_description_.set(x);
}

void ConferenceType::setConferenceDescription(const ConferenceDescriptionOptional &x) {
	this->conference_description_ = x;
}

void ConferenceType::setConferenceDescription(::std::unique_ptr<ConferenceDescriptionType> x) {
	this->conference_description_.set(std::move(x));
}

const ConferenceType::HostInfoOptional &ConferenceType::getHostInfo() const {
	return this->host_info_;
}

ConferenceType::HostInfoOptional &ConferenceType::getHostInfo() {
	return this->host_info_;
}

void ConferenceType::setHostInfo(const HostInfoType &x) {
	this->host_info_.set(x);
}

void ConferenceType::setHostInfo(const HostInfoOptional &x) {
	this->host_info_ = x;
}

void ConferenceType::setHostInfo(::std::unique_ptr<HostInfoType> x) {
	this->host_info_.set(std::move(x));
}

const ConferenceType::ConferenceStateOptional &ConferenceType::getConferenceState() const {
	return this->conference_state_;
}

ConferenceType::ConferenceStateOptional &ConferenceType::getConferenceState() {
	return this->conference_state_;
}

void ConferenceType::setConferenceState(const ConferenceStateType &x) {
	this->conference_state_.set(x);
}

void ConferenceType::setConferenceState(const ConferenceStateOptional &x) {
	this->conference_state_ = x;
}

void ConferenceType::setConferenceState(::std::unique_ptr<ConferenceStateType> x) {
	this->conference_state_.set(std::move(x));
}

const ConferenceType::UsersOptional &ConferenceType::getUsers() const {
	return this->users_;
}

ConferenceType::UsersOptional &ConferenceType::getUsers() {
	return this->users_;
}

void ConferenceType::setUsers(const UsersType &x) {
	this->users_.set(x);
}

void ConferenceType::setUsers(const UsersOptional &x) {
	this->users_ = x;
}

void ConferenceType::setUsers(::std::unique_ptr<UsersType> x) {
	this->users_.set(std::move(x));
}

const ConferenceType::SidebarsByRefOptional &ConferenceType::getSidebarsByRef() const {
	return this->sidebars_by_ref_;
}

ConferenceType::SidebarsByRefOptional &ConferenceType::getSidebarsByRef() {
	return this->sidebars_by_ref_;
}

void ConferenceType::setSidebarsByRef(const SidebarsByRefType &x) {
	this->sidebars_by_ref_.set(x);
}

void ConferenceType::setSidebarsByRef(const SidebarsByRefOptional &x) {
	this->sidebars_by_ref_ = x;
}

void ConferenceType::setSidebarsByRef(::std::unique_ptr<SidebarsByRefType> x) {
	this->sidebars_by_ref_.set(std::move(x));
}

const ConferenceType::SidebarsByValOptional &ConferenceType::getSidebarsByVal() const {
	return this->sidebars_by_val_;
}

ConferenceType::SidebarsByValOptional &ConferenceType::getSidebarsByVal() {
	return this->sidebars_by_val_;
}

void ConferenceType::setSidebarsByVal(const SidebarsByValType &x) {
	this->sidebars_by_val_.set(x);
}

void ConferenceType::setSidebarsByVal(const SidebarsByValOptional &x) {
	this->sidebars_by_val_ = x;
}

void ConferenceType::setSidebarsByVal(::std::unique_ptr<SidebarsByValType> x) {
	this->sidebars_by_val_.set(std::move(x));
}

const ConferenceType::AnySequence &ConferenceType::getAny() const {
	return this->any_;
}

ConferenceType::AnySequence &ConferenceType::getAny() {
	return this->any_;
}

void ConferenceType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConferenceType::EntityType &ConferenceType::getEntity() const {
	return this->entity_.get();
}

ConferenceType::EntityType &ConferenceType::getEntity() {
	return this->entity_.get();
}

void ConferenceType::setEntity(const EntityType &x) {
	this->entity_.set(x);
}

void ConferenceType::setEntity(::std::unique_ptr<EntityType> x) {
	this->entity_.set(std::move(x));
}

::std::unique_ptr<ConferenceType::EntityType> ConferenceType::setDetachEntity() {
	return this->entity_.detach();
}

const ConferenceType::StateType &ConferenceType::getState() const {
	return this->state_.get();
}

ConferenceType::StateType &ConferenceType::getState() {
	return this->state_.get();
}

void ConferenceType::setState(const StateType &x) {
	this->state_.set(x);
}

void ConferenceType::setState(::std::unique_ptr<StateType> x) {
	this->state_.set(std::move(x));
}

::std::unique_ptr<ConferenceType::StateType> ConferenceType::setDetachState() {
	return this->state_.detach();
}

const ConferenceType::StateType &ConferenceType::getStateDefaultValue() {
	return state_default_value_;
}

const ConferenceType::VersionOptional &ConferenceType::getVersion() const {
	return this->version_;
}

ConferenceType::VersionOptional &ConferenceType::getVersion() {
	return this->version_;
}

void ConferenceType::setVersion(const VersionType &x) {
	this->version_.set(x);
}

void ConferenceType::setVersion(const VersionOptional &x) {
	this->version_ = x;
}

const ConferenceType::AnyAttributeSet &ConferenceType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConferenceType::AnyAttributeSet &ConferenceType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConferenceType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConferenceType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConferenceType::getDomDocument() {
	return *this->dom_document_;
}

// StateType
//

StateType::StateType(Value v) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_StateType_literals_[v]) {
}

StateType::StateType(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

StateType::StateType(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

StateType::StateType(const ::LinphonePrivate::Xsd::XmlSchema::String &v)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

StateType::StateType(const StateType &v,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v, f, c) {
}

StateType &StateType::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::String &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_StateType_literals_[v]);

	return *this;
}

// ConferenceDescriptionType
//

const ConferenceDescriptionType::DisplayTextOptional &ConferenceDescriptionType::getDisplayText() const {
	return this->display_text_;
}

ConferenceDescriptionType::DisplayTextOptional &ConferenceDescriptionType::getDisplayText() {
	return this->display_text_;
}

void ConferenceDescriptionType::setDisplayText(const DisplayTextType &x) {
	this->display_text_.set(x);
}

void ConferenceDescriptionType::setDisplayText(const DisplayTextOptional &x) {
	this->display_text_ = x;
}

void ConferenceDescriptionType::setDisplayText(::std::unique_ptr<DisplayTextType> x) {
	this->display_text_.set(std::move(x));
}

const ConferenceDescriptionType::SubjectOptional &ConferenceDescriptionType::getSubject() const {
	return this->subject_;
}

ConferenceDescriptionType::SubjectOptional &ConferenceDescriptionType::getSubject() {
	return this->subject_;
}

void ConferenceDescriptionType::setSubject(const SubjectType &x) {
	this->subject_.set(x);
}

void ConferenceDescriptionType::setSubject(const SubjectOptional &x) {
	this->subject_ = x;
}

void ConferenceDescriptionType::setSubject(::std::unique_ptr<SubjectType> x) {
	this->subject_.set(std::move(x));
}

const ConferenceDescriptionType::FreeTextOptional &ConferenceDescriptionType::getFreeText() const {
	return this->free_text_;
}

ConferenceDescriptionType::FreeTextOptional &ConferenceDescriptionType::getFreeText() {
	return this->free_text_;
}

void ConferenceDescriptionType::setFreeText(const FreeTextType &x) {
	this->free_text_.set(x);
}

void ConferenceDescriptionType::setFreeText(const FreeTextOptional &x) {
	this->free_text_ = x;
}

void ConferenceDescriptionType::setFreeText(::std::unique_ptr<FreeTextType> x) {
	this->free_text_.set(std::move(x));
}

const ConferenceDescriptionType::KeywordsOptional &ConferenceDescriptionType::getKeywords() const {
	return this->keywords_;
}

ConferenceDescriptionType::KeywordsOptional &ConferenceDescriptionType::getKeywords() {
	return this->keywords_;
}

void ConferenceDescriptionType::setKeywords(const KeywordsType &x) {
	this->keywords_.set(x);
}

void ConferenceDescriptionType::setKeywords(const KeywordsOptional &x) {
	this->keywords_ = x;
}

void ConferenceDescriptionType::setKeywords(::std::unique_ptr<KeywordsType> x) {
	this->keywords_.set(std::move(x));
}

const ConferenceDescriptionType::ConfUrisOptional &ConferenceDescriptionType::getConfUris() const {
	return this->conf_uris_;
}

ConferenceDescriptionType::ConfUrisOptional &ConferenceDescriptionType::getConfUris() {
	return this->conf_uris_;
}

void ConferenceDescriptionType::setConfUris(const ConfUrisType &x) {
	this->conf_uris_.set(x);
}

void ConferenceDescriptionType::setConfUris(const ConfUrisOptional &x) {
	this->conf_uris_ = x;
}

void ConferenceDescriptionType::setConfUris(::std::unique_ptr<ConfUrisType> x) {
	this->conf_uris_.set(std::move(x));
}

const ConferenceDescriptionType::ServiceUrisOptional &ConferenceDescriptionType::getServiceUris() const {
	return this->service_uris_;
}

ConferenceDescriptionType::ServiceUrisOptional &ConferenceDescriptionType::getServiceUris() {
	return this->service_uris_;
}

void ConferenceDescriptionType::setServiceUris(const ServiceUrisType &x) {
	this->service_uris_.set(x);
}

void ConferenceDescriptionType::setServiceUris(const ServiceUrisOptional &x) {
	this->service_uris_ = x;
}

void ConferenceDescriptionType::setServiceUris(::std::unique_ptr<ServiceUrisType> x) {
	this->service_uris_.set(std::move(x));
}

const ConferenceDescriptionType::MaximumUserCountOptional &ConferenceDescriptionType::getMaximumUserCount() const {
	return this->maximum_user_count_;
}

ConferenceDescriptionType::MaximumUserCountOptional &ConferenceDescriptionType::getMaximumUserCount() {
	return this->maximum_user_count_;
}

void ConferenceDescriptionType::setMaximumUserCount(const MaximumUserCountType &x) {
	this->maximum_user_count_.set(x);
}

void ConferenceDescriptionType::setMaximumUserCount(const MaximumUserCountOptional &x) {
	this->maximum_user_count_ = x;
}

const ConferenceDescriptionType::AvailableMediaOptional &ConferenceDescriptionType::getAvailableMedia() const {
	return this->available_media_;
}

ConferenceDescriptionType::AvailableMediaOptional &ConferenceDescriptionType::getAvailableMedia() {
	return this->available_media_;
}

void ConferenceDescriptionType::setAvailableMedia(const AvailableMediaType &x) {
	this->available_media_.set(x);
}

void ConferenceDescriptionType::setAvailableMedia(const AvailableMediaOptional &x) {
	this->available_media_ = x;
}

void ConferenceDescriptionType::setAvailableMedia(::std::unique_ptr<AvailableMediaType> x) {
	this->available_media_.set(std::move(x));
}

const ConferenceDescriptionType::AnySequence &ConferenceDescriptionType::getAny() const {
	return this->any_;
}

ConferenceDescriptionType::AnySequence &ConferenceDescriptionType::getAny() {
	return this->any_;
}

void ConferenceDescriptionType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConferenceDescriptionType::AnyAttributeSet &ConferenceDescriptionType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConferenceDescriptionType::AnyAttributeSet &ConferenceDescriptionType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConferenceDescriptionType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConferenceDescriptionType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConferenceDescriptionType::getDomDocument() {
	return *this->dom_document_;
}

// HostType
//

const HostType::DisplayTextOptional &HostType::getDisplayText() const {
	return this->display_text_;
}

HostType::DisplayTextOptional &HostType::getDisplayText() {
	return this->display_text_;
}

void HostType::setDisplayText(const DisplayTextType &x) {
	this->display_text_.set(x);
}

void HostType::setDisplayText(const DisplayTextOptional &x) {
	this->display_text_ = x;
}

void HostType::setDisplayText(::std::unique_ptr<DisplayTextType> x) {
	this->display_text_.set(std::move(x));
}

const HostType::WebPageOptional &HostType::getWebPage() const {
	return this->web_page_;
}

HostType::WebPageOptional &HostType::getWebPage() {
	return this->web_page_;
}

void HostType::setWebPage(const WebPageType &x) {
	this->web_page_.set(x);
}

void HostType::setWebPage(const WebPageOptional &x) {
	this->web_page_ = x;
}

void HostType::setWebPage(::std::unique_ptr<WebPageType> x) {
	this->web_page_.set(std::move(x));
}

const HostType::UrisOptional &HostType::getUris() const {
	return this->uris_;
}

HostType::UrisOptional &HostType::getUris() {
	return this->uris_;
}

void HostType::setUris(const UrisType &x) {
	this->uris_.set(x);
}

void HostType::setUris(const UrisOptional &x) {
	this->uris_ = x;
}

void HostType::setUris(::std::unique_ptr<UrisType> x) {
	this->uris_.set(std::move(x));
}

const HostType::AnySequence &HostType::getAny() const {
	return this->any_;
}

HostType::AnySequence &HostType::getAny() {
	return this->any_;
}

void HostType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const HostType::AnyAttributeSet &HostType::getAnyAttribute() const {
	return this->any_attribute_;
}

HostType::AnyAttributeSet &HostType::getAnyAttribute() {
	return this->any_attribute_;
}

void HostType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &HostType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &HostType::getDomDocument() {
	return *this->dom_document_;
}

// ConferenceStateType
//

const ConferenceStateType::UserCountOptional &ConferenceStateType::getUserCount() const {
	return this->user_count_;
}

ConferenceStateType::UserCountOptional &ConferenceStateType::getUserCount() {
	return this->user_count_;
}

void ConferenceStateType::setUserCount(const UserCountType &x) {
	this->user_count_.set(x);
}

void ConferenceStateType::setUserCount(const UserCountOptional &x) {
	this->user_count_ = x;
}

const ConferenceStateType::ActiveOptional &ConferenceStateType::getActive() const {
	return this->active_;
}

ConferenceStateType::ActiveOptional &ConferenceStateType::getActive() {
	return this->active_;
}

void ConferenceStateType::setActive(const ActiveType &x) {
	this->active_.set(x);
}

void ConferenceStateType::setActive(const ActiveOptional &x) {
	this->active_ = x;
}

const ConferenceStateType::LockedOptional &ConferenceStateType::getLocked() const {
	return this->locked_;
}

ConferenceStateType::LockedOptional &ConferenceStateType::getLocked() {
	return this->locked_;
}

void ConferenceStateType::setLocked(const LockedType &x) {
	this->locked_.set(x);
}

void ConferenceStateType::setLocked(const LockedOptional &x) {
	this->locked_ = x;
}

const ConferenceStateType::AnySequence &ConferenceStateType::getAny() const {
	return this->any_;
}

ConferenceStateType::AnySequence &ConferenceStateType::getAny() {
	return this->any_;
}

void ConferenceStateType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConferenceStateType::AnyAttributeSet &ConferenceStateType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConferenceStateType::AnyAttributeSet &ConferenceStateType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConferenceStateType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConferenceStateType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConferenceStateType::getDomDocument() {
	return *this->dom_document_;
}

// ConferenceMediaType
//

const ConferenceMediaType::EntrySequence &ConferenceMediaType::getEntry() const {
	return this->entry_;
}

ConferenceMediaType::EntrySequence &ConferenceMediaType::getEntry() {
	return this->entry_;
}

void ConferenceMediaType::setEntry(const EntrySequence &s) {
	this->entry_ = s;
}

const ConferenceMediaType::AnyAttributeSet &ConferenceMediaType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConferenceMediaType::AnyAttributeSet &ConferenceMediaType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConferenceMediaType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConferenceMediaType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConferenceMediaType::getDomDocument() {
	return *this->dom_document_;
}

// ConferenceMediumType
//

const ConferenceMediumType::DisplayTextOptional &ConferenceMediumType::getDisplayText() const {
	return this->display_text_;
}

ConferenceMediumType::DisplayTextOptional &ConferenceMediumType::getDisplayText() {
	return this->display_text_;
}

void ConferenceMediumType::setDisplayText(const DisplayTextType &x) {
	this->display_text_.set(x);
}

void ConferenceMediumType::setDisplayText(const DisplayTextOptional &x) {
	this->display_text_ = x;
}

void ConferenceMediumType::setDisplayText(::std::unique_ptr<DisplayTextType> x) {
	this->display_text_.set(std::move(x));
}

const ConferenceMediumType::TypeType &ConferenceMediumType::getType() const {
	return this->type_.get();
}

ConferenceMediumType::TypeType &ConferenceMediumType::getType() {
	return this->type_.get();
}

void ConferenceMediumType::setType(const TypeType &x) {
	this->type_.set(x);
}

void ConferenceMediumType::setType(::std::unique_ptr<TypeType> x) {
	this->type_.set(std::move(x));
}

::std::unique_ptr<ConferenceMediumType::TypeType> ConferenceMediumType::setDetachType() {
	return this->type_.detach();
}

const ConferenceMediumType::StatusOptional &ConferenceMediumType::getStatus() const {
	return this->status_;
}

ConferenceMediumType::StatusOptional &ConferenceMediumType::getStatus() {
	return this->status_;
}

void ConferenceMediumType::setStatus(const StatusType &x) {
	this->status_.set(x);
}

void ConferenceMediumType::setStatus(const StatusOptional &x) {
	this->status_ = x;
}

void ConferenceMediumType::setStatus(::std::unique_ptr<StatusType> x) {
	this->status_.set(std::move(x));
}

const ConferenceMediumType::AnySequence &ConferenceMediumType::getAny() const {
	return this->any_;
}

ConferenceMediumType::AnySequence &ConferenceMediumType::getAny() {
	return this->any_;
}

void ConferenceMediumType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConferenceMediumType::LabelType &ConferenceMediumType::getLabel() const {
	return this->label_.get();
}

ConferenceMediumType::LabelType &ConferenceMediumType::getLabel() {
	return this->label_.get();
}

void ConferenceMediumType::setLabel(const LabelType &x) {
	this->label_.set(x);
}

void ConferenceMediumType::setLabel(::std::unique_ptr<LabelType> x) {
	this->label_.set(std::move(x));
}

::std::unique_ptr<ConferenceMediumType::LabelType> ConferenceMediumType::setDetachLabel() {
	return this->label_.detach();
}

const ConferenceMediumType::AnyAttributeSet &ConferenceMediumType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConferenceMediumType::AnyAttributeSet &ConferenceMediumType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConferenceMediumType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConferenceMediumType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConferenceMediumType::getDomDocument() {
	return *this->dom_document_;
}

// UrisType
//

const UrisType::EntrySequence &UrisType::getEntry() const {
	return this->entry_;
}

UrisType::EntrySequence &UrisType::getEntry() {
	return this->entry_;
}

void UrisType::setEntry(const EntrySequence &s) {
	this->entry_ = s;
}

const UrisType::StateType &UrisType::getState() const {
	return this->state_.get();
}

UrisType::StateType &UrisType::getState() {
	return this->state_.get();
}

void UrisType::setState(const StateType &x) {
	this->state_.set(x);
}

void UrisType::setState(::std::unique_ptr<StateType> x) {
	this->state_.set(std::move(x));
}

::std::unique_ptr<UrisType::StateType> UrisType::setDetachState() {
	return this->state_.detach();
}

const UrisType::StateType &UrisType::getStateDefaultValue() {
	return state_default_value_;
}

const UrisType::AnyAttributeSet &UrisType::getAnyAttribute() const {
	return this->any_attribute_;
}

UrisType::AnyAttributeSet &UrisType::getAnyAttribute() {
	return this->any_attribute_;
}

void UrisType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UrisType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UrisType::getDomDocument() {
	return *this->dom_document_;
}

// UriType
//

const UriType::UriType1 &UriType::getUri() const {
	return this->uri_.get();
}

UriType::UriType1 &UriType::getUri() {
	return this->uri_.get();
}

void UriType::setUri(const UriType1 &x) {
	this->uri_.set(x);
}

void UriType::setUri(::std::unique_ptr<UriType1> x) {
	this->uri_.set(std::move(x));
}

::std::unique_ptr<UriType::UriType1> UriType::setDetachUri() {
	return this->uri_.detach();
}

const UriType::DisplayTextOptional &UriType::getDisplayText() const {
	return this->display_text_;
}

UriType::DisplayTextOptional &UriType::getDisplayText() {
	return this->display_text_;
}

void UriType::setDisplayText(const DisplayTextType &x) {
	this->display_text_.set(x);
}

void UriType::setDisplayText(const DisplayTextOptional &x) {
	this->display_text_ = x;
}

void UriType::setDisplayText(::std::unique_ptr<DisplayTextType> x) {
	this->display_text_.set(std::move(x));
}

const UriType::PurposeOptional &UriType::getPurpose() const {
	return this->purpose_;
}

UriType::PurposeOptional &UriType::getPurpose() {
	return this->purpose_;
}

void UriType::setPurpose(const PurposeType &x) {
	this->purpose_.set(x);
}

void UriType::setPurpose(const PurposeOptional &x) {
	this->purpose_ = x;
}

void UriType::setPurpose(::std::unique_ptr<PurposeType> x) {
	this->purpose_.set(std::move(x));
}

const UriType::ModifiedOptional &UriType::getModified() const {
	return this->modified_;
}

UriType::ModifiedOptional &UriType::getModified() {
	return this->modified_;
}

void UriType::setModified(const ModifiedType &x) {
	this->modified_.set(x);
}

void UriType::setModified(const ModifiedOptional &x) {
	this->modified_ = x;
}

void UriType::setModified(::std::unique_ptr<ModifiedType> x) {
	this->modified_.set(std::move(x));
}

const UriType::AnySequence &UriType::getAny() const {
	return this->any_;
}

UriType::AnySequence &UriType::getAny() {
	return this->any_;
}

void UriType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const UriType::AnyAttributeSet &UriType::getAnyAttribute() const {
	return this->any_attribute_;
}

UriType::AnyAttributeSet &UriType::getAnyAttribute() {
	return this->any_attribute_;
}

void UriType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UriType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UriType::getDomDocument() {
	return *this->dom_document_;
}

// KeywordsType
//

KeywordsType::KeywordsType() : ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char>(this) {
}

KeywordsType::KeywordsType(size_type n, const ::LinphonePrivate::Xsd::XmlSchema::String &x)
    : ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char>(n, x, this) {
}

KeywordsType::KeywordsType(const KeywordsType &o,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::SimpleType(o, f, c),
      ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char>(o, f, this) {
}

// UsersType
//

const UsersType::UserSequence &UsersType::getUser() const {
	return this->user_;
}

UsersType::UserSequence &UsersType::getUser() {
	return this->user_;
}

void UsersType::setUser(const UserSequence &s) {
	this->user_ = s;
}

const UsersType::AnySequence &UsersType::getAny() const {
	return this->any_;
}

UsersType::AnySequence &UsersType::getAny() {
	return this->any_;
}

void UsersType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const UsersType::StateType &UsersType::getState() const {
	return this->state_.get();
}

UsersType::StateType &UsersType::getState() {
	return this->state_.get();
}

void UsersType::setState(const StateType &x) {
	this->state_.set(x);
}

void UsersType::setState(::std::unique_ptr<StateType> x) {
	this->state_.set(std::move(x));
}

::std::unique_ptr<UsersType::StateType> UsersType::setDetachState() {
	return this->state_.detach();
}

const UsersType::StateType &UsersType::getStateDefaultValue() {
	return state_default_value_;
}

const UsersType::AnyAttributeSet &UsersType::getAnyAttribute() const {
	return this->any_attribute_;
}

UsersType::AnyAttributeSet &UsersType::getAnyAttribute() {
	return this->any_attribute_;
}

void UsersType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UsersType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UsersType::getDomDocument() {
	return *this->dom_document_;
}

// UserType
//

const UserType::DisplayTextOptional &UserType::getDisplayText() const {
	return this->display_text_;
}

UserType::DisplayTextOptional &UserType::getDisplayText() {
	return this->display_text_;
}

void UserType::setDisplayText(const DisplayTextType &x) {
	this->display_text_.set(x);
}

void UserType::setDisplayText(const DisplayTextOptional &x) {
	this->display_text_ = x;
}

void UserType::setDisplayText(::std::unique_ptr<DisplayTextType> x) {
	this->display_text_.set(std::move(x));
}

const UserType::AssociatedAorsOptional &UserType::getAssociatedAors() const {
	return this->associated_aors_;
}

UserType::AssociatedAorsOptional &UserType::getAssociatedAors() {
	return this->associated_aors_;
}

void UserType::setAssociatedAors(const AssociatedAorsType &x) {
	this->associated_aors_.set(x);
}

void UserType::setAssociatedAors(const AssociatedAorsOptional &x) {
	this->associated_aors_ = x;
}

void UserType::setAssociatedAors(::std::unique_ptr<AssociatedAorsType> x) {
	this->associated_aors_.set(std::move(x));
}

const UserType::RolesOptional &UserType::getRoles() const {
	return this->roles_;
}

UserType::RolesOptional &UserType::getRoles() {
	return this->roles_;
}

void UserType::setRoles(const RolesType &x) {
	this->roles_.set(x);
}

void UserType::setRoles(const RolesOptional &x) {
	this->roles_ = x;
}

void UserType::setRoles(::std::unique_ptr<RolesType> x) {
	this->roles_.set(std::move(x));
}

const UserType::LanguagesOptional &UserType::getLanguages() const {
	return this->languages_;
}

UserType::LanguagesOptional &UserType::getLanguages() {
	return this->languages_;
}

void UserType::setLanguages(const LanguagesType &x) {
	this->languages_.set(x);
}

void UserType::setLanguages(const LanguagesOptional &x) {
	this->languages_ = x;
}

void UserType::setLanguages(::std::unique_ptr<LanguagesType> x) {
	this->languages_.set(std::move(x));
}

const UserType::CascadedFocusOptional &UserType::getCascadedFocus() const {
	return this->cascaded_focus_;
}

UserType::CascadedFocusOptional &UserType::getCascadedFocus() {
	return this->cascaded_focus_;
}

void UserType::setCascadedFocus(const CascadedFocusType &x) {
	this->cascaded_focus_.set(x);
}

void UserType::setCascadedFocus(const CascadedFocusOptional &x) {
	this->cascaded_focus_ = x;
}

void UserType::setCascadedFocus(::std::unique_ptr<CascadedFocusType> x) {
	this->cascaded_focus_.set(std::move(x));
}

const UserType::EndpointSequence &UserType::getEndpoint() const {
	return this->endpoint_;
}

UserType::EndpointSequence &UserType::getEndpoint() {
	return this->endpoint_;
}

void UserType::setEndpoint(const EndpointSequence &s) {
	this->endpoint_ = s;
}

const UserType::AnySequence &UserType::getAny() const {
	return this->any_;
}

UserType::AnySequence &UserType::getAny() {
	return this->any_;
}

void UserType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const UserType::EntityOptional &UserType::getEntity() const {
	return this->entity_;
}

UserType::EntityOptional &UserType::getEntity() {
	return this->entity_;
}

void UserType::setEntity(const EntityType &x) {
	this->entity_.set(x);
}

void UserType::setEntity(const EntityOptional &x) {
	this->entity_ = x;
}

void UserType::setEntity(::std::unique_ptr<EntityType> x) {
	this->entity_.set(std::move(x));
}

const UserType::StateType &UserType::getState() const {
	return this->state_.get();
}

UserType::StateType &UserType::getState() {
	return this->state_.get();
}

void UserType::setState(const StateType &x) {
	this->state_.set(x);
}

void UserType::setState(::std::unique_ptr<StateType> x) {
	this->state_.set(std::move(x));
}

::std::unique_ptr<UserType::StateType> UserType::setDetachState() {
	return this->state_.detach();
}

const UserType::StateType &UserType::getStateDefaultValue() {
	return state_default_value_;
}

const UserType::AnyAttributeSet &UserType::getAnyAttribute() const {
	return this->any_attribute_;
}

UserType::AnyAttributeSet &UserType::getAnyAttribute() {
	return this->any_attribute_;
}

void UserType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UserType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UserType::getDomDocument() {
	return *this->dom_document_;
}

// UserRolesType
//

const UserRolesType::EntrySequence &UserRolesType::getEntry() const {
	return this->entry_;
}

UserRolesType::EntrySequence &UserRolesType::getEntry() {
	return this->entry_;
}

void UserRolesType::setEntry(const EntrySequence &s) {
	this->entry_ = s;
}

const UserRolesType::AnyAttributeSet &UserRolesType::getAnyAttribute() const {
	return this->any_attribute_;
}

UserRolesType::AnyAttributeSet &UserRolesType::getAnyAttribute() {
	return this->any_attribute_;
}

void UserRolesType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UserRolesType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UserRolesType::getDomDocument() {
	return *this->dom_document_;
}

// UserLanguagesType
//

UserLanguagesType::UserLanguagesType()
    : ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char>(this) {
}

UserLanguagesType::UserLanguagesType(size_type n, const ::LinphonePrivate::Xsd::XmlSchema::Language &x)
    : ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char>(n, x, this) {
}

UserLanguagesType::UserLanguagesType(const UserLanguagesType &o,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::SimpleType(o, f, c),
      ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char>(o, f, this) {
}

// EndpointType
//

const EndpointType::DisplayTextOptional &EndpointType::getDisplayText() const {
	return this->display_text_;
}

EndpointType::DisplayTextOptional &EndpointType::getDisplayText() {
	return this->display_text_;
}

void EndpointType::setDisplayText(const DisplayTextType &x) {
	this->display_text_.set(x);
}

void EndpointType::setDisplayText(const DisplayTextOptional &x) {
	this->display_text_ = x;
}

void EndpointType::setDisplayText(::std::unique_ptr<DisplayTextType> x) {
	this->display_text_.set(std::move(x));
}

const EndpointType::ReferredOptional &EndpointType::getReferred() const {
	return this->referred_;
}

EndpointType::ReferredOptional &EndpointType::getReferred() {
	return this->referred_;
}

void EndpointType::setReferred(const ReferredType &x) {
	this->referred_.set(x);
}

void EndpointType::setReferred(const ReferredOptional &x) {
	this->referred_ = x;
}

void EndpointType::setReferred(::std::unique_ptr<ReferredType> x) {
	this->referred_.set(std::move(x));
}

const EndpointType::StatusOptional &EndpointType::getStatus() const {
	return this->status_;
}

EndpointType::StatusOptional &EndpointType::getStatus() {
	return this->status_;
}

void EndpointType::setStatus(const StatusType &x) {
	this->status_.set(x);
}

void EndpointType::setStatus(const StatusOptional &x) {
	this->status_ = x;
}

void EndpointType::setStatus(::std::unique_ptr<StatusType> x) {
	this->status_.set(std::move(x));
}

const EndpointType::JoiningMethodOptional &EndpointType::getJoiningMethod() const {
	return this->joining_method_;
}

EndpointType::JoiningMethodOptional &EndpointType::getJoiningMethod() {
	return this->joining_method_;
}

void EndpointType::setJoiningMethod(const JoiningMethodType &x) {
	this->joining_method_.set(x);
}

void EndpointType::setJoiningMethod(const JoiningMethodOptional &x) {
	this->joining_method_ = x;
}

void EndpointType::setJoiningMethod(::std::unique_ptr<JoiningMethodType> x) {
	this->joining_method_.set(std::move(x));
}

const EndpointType::JoiningInfoOptional &EndpointType::getJoiningInfo() const {
	return this->joining_info_;
}

EndpointType::JoiningInfoOptional &EndpointType::getJoiningInfo() {
	return this->joining_info_;
}

void EndpointType::setJoiningInfo(const JoiningInfoType &x) {
	this->joining_info_.set(x);
}

void EndpointType::setJoiningInfo(const JoiningInfoOptional &x) {
	this->joining_info_ = x;
}

void EndpointType::setJoiningInfo(::std::unique_ptr<JoiningInfoType> x) {
	this->joining_info_.set(std::move(x));
}

const EndpointType::DisconnectionMethodOptional &EndpointType::getDisconnectionMethod() const {
	return this->disconnection_method_;
}

EndpointType::DisconnectionMethodOptional &EndpointType::getDisconnectionMethod() {
	return this->disconnection_method_;
}

void EndpointType::setDisconnectionMethod(const DisconnectionMethodType &x) {
	this->disconnection_method_.set(x);
}

void EndpointType::setDisconnectionMethod(const DisconnectionMethodOptional &x) {
	this->disconnection_method_ = x;
}

void EndpointType::setDisconnectionMethod(::std::unique_ptr<DisconnectionMethodType> x) {
	this->disconnection_method_.set(std::move(x));
}

const EndpointType::DisconnectionInfoOptional &EndpointType::getDisconnectionInfo() const {
	return this->disconnection_info_;
}

EndpointType::DisconnectionInfoOptional &EndpointType::getDisconnectionInfo() {
	return this->disconnection_info_;
}

void EndpointType::setDisconnectionInfo(const DisconnectionInfoType &x) {
	this->disconnection_info_.set(x);
}

void EndpointType::setDisconnectionInfo(const DisconnectionInfoOptional &x) {
	this->disconnection_info_ = x;
}

void EndpointType::setDisconnectionInfo(::std::unique_ptr<DisconnectionInfoType> x) {
	this->disconnection_info_.set(std::move(x));
}

const EndpointType::MediaSequence &EndpointType::getMedia() const {
	return this->media_;
}

EndpointType::MediaSequence &EndpointType::getMedia() {
	return this->media_;
}

void EndpointType::setMedia(const MediaSequence &s) {
	this->media_ = s;
}

const EndpointType::CallInfoOptional &EndpointType::getCallInfo() const {
	return this->call_info_;
}

EndpointType::CallInfoOptional &EndpointType::getCallInfo() {
	return this->call_info_;
}

void EndpointType::setCallInfo(const CallInfoType &x) {
	this->call_info_.set(x);
}

void EndpointType::setCallInfo(const CallInfoOptional &x) {
	this->call_info_ = x;
}

void EndpointType::setCallInfo(::std::unique_ptr<CallInfoType> x) {
	this->call_info_.set(std::move(x));
}

const EndpointType::AnySequence &EndpointType::getAny() const {
	return this->any_;
}

EndpointType::AnySequence &EndpointType::getAny() {
	return this->any_;
}

void EndpointType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const EndpointType::EntityOptional &EndpointType::getEntity() const {
	return this->entity_;
}

EndpointType::EntityOptional &EndpointType::getEntity() {
	return this->entity_;
}

void EndpointType::setEntity(const EntityType &x) {
	this->entity_.set(x);
}

void EndpointType::setEntity(const EntityOptional &x) {
	this->entity_ = x;
}

void EndpointType::setEntity(::std::unique_ptr<EntityType> x) {
	this->entity_.set(std::move(x));
}

const EndpointType::StateType &EndpointType::getState() const {
	return this->state_.get();
}

EndpointType::StateType &EndpointType::getState() {
	return this->state_.get();
}

void EndpointType::setState(const StateType &x) {
	this->state_.set(x);
}

void EndpointType::setState(::std::unique_ptr<StateType> x) {
	this->state_.set(std::move(x));
}

::std::unique_ptr<EndpointType::StateType> EndpointType::setDetachState() {
	return this->state_.detach();
}

const EndpointType::StateType &EndpointType::getStateDefaultValue() {
	return state_default_value_;
}

const EndpointType::AnyAttributeSet &EndpointType::getAnyAttribute() const {
	return this->any_attribute_;
}

EndpointType::AnyAttributeSet &EndpointType::getAnyAttribute() {
	return this->any_attribute_;
}

void EndpointType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &EndpointType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &EndpointType::getDomDocument() {
	return *this->dom_document_;
}

// EndpointStatusType
//

EndpointStatusType::EndpointStatusType(Value v)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_EndpointStatusType_literals_[v]) {
}

EndpointStatusType::EndpointStatusType(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

EndpointStatusType::EndpointStatusType(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

EndpointStatusType::EndpointStatusType(const ::LinphonePrivate::Xsd::XmlSchema::String &v)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

EndpointStatusType::EndpointStatusType(const EndpointStatusType &v,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v, f, c) {
}

EndpointStatusType &EndpointStatusType::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::String &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_EndpointStatusType_literals_[v]);

	return *this;
}

// JoiningType
//

JoiningType::JoiningType(Value v) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_JoiningType_literals_[v]) {
}

JoiningType::JoiningType(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

JoiningType::JoiningType(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

JoiningType::JoiningType(const ::LinphonePrivate::Xsd::XmlSchema::String &v)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

JoiningType::JoiningType(const JoiningType &v,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v, f, c) {
}

JoiningType &JoiningType::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::String &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_JoiningType_literals_[v]);

	return *this;
}

// DisconnectionType
//

DisconnectionType::DisconnectionType(Value v)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_DisconnectionType_literals_[v]) {
}

DisconnectionType::DisconnectionType(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

DisconnectionType::DisconnectionType(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

DisconnectionType::DisconnectionType(const ::LinphonePrivate::Xsd::XmlSchema::String &v)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

DisconnectionType::DisconnectionType(const DisconnectionType &v,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v, f, c) {
}

DisconnectionType &DisconnectionType::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::String &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_DisconnectionType_literals_[v]);

	return *this;
}

// ExecutionType
//

const ExecutionType::WhenOptional &ExecutionType::getWhen() const {
	return this->when_;
}

ExecutionType::WhenOptional &ExecutionType::getWhen() {
	return this->when_;
}

void ExecutionType::setWhen(const WhenType &x) {
	this->when_.set(x);
}

void ExecutionType::setWhen(const WhenOptional &x) {
	this->when_ = x;
}

void ExecutionType::setWhen(::std::unique_ptr<WhenType> x) {
	this->when_.set(std::move(x));
}

const ExecutionType::ReasonOptional &ExecutionType::getReason() const {
	return this->reason_;
}

ExecutionType::ReasonOptional &ExecutionType::getReason() {
	return this->reason_;
}

void ExecutionType::setReason(const ReasonType &x) {
	this->reason_.set(x);
}

void ExecutionType::setReason(const ReasonOptional &x) {
	this->reason_ = x;
}

void ExecutionType::setReason(::std::unique_ptr<ReasonType> x) {
	this->reason_.set(std::move(x));
}

const ExecutionType::ByOptional &ExecutionType::getBy() const {
	return this->by_;
}

ExecutionType::ByOptional &ExecutionType::getBy() {
	return this->by_;
}

void ExecutionType::setBy(const ByType &x) {
	this->by_.set(x);
}

void ExecutionType::setBy(const ByOptional &x) {
	this->by_ = x;
}

void ExecutionType::setBy(::std::unique_ptr<ByType> x) {
	this->by_.set(std::move(x));
}

const ExecutionType::AnyAttributeSet &ExecutionType::getAnyAttribute() const {
	return this->any_attribute_;
}

ExecutionType::AnyAttributeSet &ExecutionType::getAnyAttribute() {
	return this->any_attribute_;
}

void ExecutionType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ExecutionType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ExecutionType::getDomDocument() {
	return *this->dom_document_;
}

// CallType
//

const CallType::SipOptional &CallType::getSip() const {
	return this->sip_;
}

CallType::SipOptional &CallType::getSip() {
	return this->sip_;
}

void CallType::setSip(const SipType &x) {
	this->sip_.set(x);
}

void CallType::setSip(const SipOptional &x) {
	this->sip_ = x;
}

void CallType::setSip(::std::unique_ptr<SipType> x) {
	this->sip_.set(std::move(x));
}

const CallType::AnySequence &CallType::getAny() const {
	return this->any_;
}

CallType::AnySequence &CallType::getAny() {
	return this->any_;
}

void CallType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const CallType::AnyAttributeSet &CallType::getAnyAttribute() const {
	return this->any_attribute_;
}

CallType::AnyAttributeSet &CallType::getAnyAttribute() {
	return this->any_attribute_;
}

void CallType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &CallType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &CallType::getDomDocument() {
	return *this->dom_document_;
}

// SipDialogIdType
//

const SipDialogIdType::DisplayTextOptional &SipDialogIdType::getDisplayText() const {
	return this->display_text_;
}

SipDialogIdType::DisplayTextOptional &SipDialogIdType::getDisplayText() {
	return this->display_text_;
}

void SipDialogIdType::setDisplayText(const DisplayTextType &x) {
	this->display_text_.set(x);
}

void SipDialogIdType::setDisplayText(const DisplayTextOptional &x) {
	this->display_text_ = x;
}

void SipDialogIdType::setDisplayText(::std::unique_ptr<DisplayTextType> x) {
	this->display_text_.set(std::move(x));
}

const SipDialogIdType::CallIdType &SipDialogIdType::getCallId() const {
	return this->call_id_.get();
}

SipDialogIdType::CallIdType &SipDialogIdType::getCallId() {
	return this->call_id_.get();
}

void SipDialogIdType::setCallId(const CallIdType &x) {
	this->call_id_.set(x);
}

void SipDialogIdType::setCallId(::std::unique_ptr<CallIdType> x) {
	this->call_id_.set(std::move(x));
}

::std::unique_ptr<SipDialogIdType::CallIdType> SipDialogIdType::setDetachCall_id() {
	return this->call_id_.detach();
}

const SipDialogIdType::FromTagType &SipDialogIdType::getFromTag() const {
	return this->from_tag_.get();
}

SipDialogIdType::FromTagType &SipDialogIdType::getFromTag() {
	return this->from_tag_.get();
}

void SipDialogIdType::setFromTag(const FromTagType &x) {
	this->from_tag_.set(x);
}

void SipDialogIdType::setFromTag(::std::unique_ptr<FromTagType> x) {
	this->from_tag_.set(std::move(x));
}

::std::unique_ptr<SipDialogIdType::FromTagType> SipDialogIdType::setDetachFrom_tag() {
	return this->from_tag_.detach();
}

const SipDialogIdType::ToTagType &SipDialogIdType::getToTag() const {
	return this->to_tag_.get();
}

SipDialogIdType::ToTagType &SipDialogIdType::getToTag() {
	return this->to_tag_.get();
}

void SipDialogIdType::setToTag(const ToTagType &x) {
	this->to_tag_.set(x);
}

void SipDialogIdType::setToTag(::std::unique_ptr<ToTagType> x) {
	this->to_tag_.set(std::move(x));
}

::std::unique_ptr<SipDialogIdType::ToTagType> SipDialogIdType::setDetachTo_tag() {
	return this->to_tag_.detach();
}

const SipDialogIdType::AnySequence &SipDialogIdType::getAny() const {
	return this->any_;
}

SipDialogIdType::AnySequence &SipDialogIdType::getAny() {
	return this->any_;
}

void SipDialogIdType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SipDialogIdType::AnyAttributeSet &SipDialogIdType::getAnyAttribute() const {
	return this->any_attribute_;
}

SipDialogIdType::AnyAttributeSet &SipDialogIdType::getAnyAttribute() {
	return this->any_attribute_;
}

void SipDialogIdType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SipDialogIdType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SipDialogIdType::getDomDocument() {
	return *this->dom_document_;
}

// MediaType
//

const MediaType::DisplayTextOptional &MediaType::getDisplayText() const {
	return this->display_text_;
}

MediaType::DisplayTextOptional &MediaType::getDisplayText() {
	return this->display_text_;
}

void MediaType::setDisplayText(const DisplayTextType &x) {
	this->display_text_.set(x);
}

void MediaType::setDisplayText(const DisplayTextOptional &x) {
	this->display_text_ = x;
}

void MediaType::setDisplayText(::std::unique_ptr<DisplayTextType> x) {
	this->display_text_.set(std::move(x));
}

const MediaType::TypeOptional &MediaType::getType() const {
	return this->type_;
}

MediaType::TypeOptional &MediaType::getType() {
	return this->type_;
}

void MediaType::setType(const TypeType &x) {
	this->type_.set(x);
}

void MediaType::setType(const TypeOptional &x) {
	this->type_ = x;
}

void MediaType::setType(::std::unique_ptr<TypeType> x) {
	this->type_.set(std::move(x));
}

const MediaType::LabelOptional &MediaType::getLabel() const {
	return this->label_;
}

MediaType::LabelOptional &MediaType::getLabel() {
	return this->label_;
}

void MediaType::setLabel(const LabelType &x) {
	this->label_.set(x);
}

void MediaType::setLabel(const LabelOptional &x) {
	this->label_ = x;
}

void MediaType::setLabel(::std::unique_ptr<LabelType> x) {
	this->label_.set(std::move(x));
}

const MediaType::SrcIdOptional &MediaType::getSrcId() const {
	return this->src_id_;
}

MediaType::SrcIdOptional &MediaType::getSrcId() {
	return this->src_id_;
}

void MediaType::setSrcId(const SrcIdType &x) {
	this->src_id_.set(x);
}

void MediaType::setSrcId(const SrcIdOptional &x) {
	this->src_id_ = x;
}

void MediaType::setSrcId(::std::unique_ptr<SrcIdType> x) {
	this->src_id_.set(std::move(x));
}

const MediaType::StatusOptional &MediaType::getStatus() const {
	return this->status_;
}

MediaType::StatusOptional &MediaType::getStatus() {
	return this->status_;
}

void MediaType::setStatus(const StatusType &x) {
	this->status_.set(x);
}

void MediaType::setStatus(const StatusOptional &x) {
	this->status_ = x;
}

void MediaType::setStatus(::std::unique_ptr<StatusType> x) {
	this->status_.set(std::move(x));
}

const MediaType::AnySequence &MediaType::getAny() const {
	return this->any_;
}

MediaType::AnySequence &MediaType::getAny() {
	return this->any_;
}

void MediaType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const MediaType::IdType &MediaType::getId() const {
	return this->id_.get();
}

MediaType::IdType &MediaType::getId() {
	return this->id_.get();
}

void MediaType::setId(const IdType &x) {
	this->id_.set(x);
}

void MediaType::setId(::std::unique_ptr<IdType> x) {
	this->id_.set(std::move(x));
}

::std::unique_ptr<MediaType::IdType> MediaType::setDetachId() {
	return this->id_.detach();
}

const MediaType::AnyAttributeSet &MediaType::getAnyAttribute() const {
	return this->any_attribute_;
}

MediaType::AnyAttributeSet &MediaType::getAnyAttribute() {
	return this->any_attribute_;
}

void MediaType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &MediaType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &MediaType::getDomDocument() {
	return *this->dom_document_;
}

// MediaStatusType
//

MediaStatusType::MediaStatusType(Value v)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_MediaStatusType_literals_[v]) {
}

MediaStatusType::MediaStatusType(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

MediaStatusType::MediaStatusType(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

MediaStatusType::MediaStatusType(const ::LinphonePrivate::Xsd::XmlSchema::String &v)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

MediaStatusType::MediaStatusType(const MediaStatusType &v,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v, f, c) {
}

MediaStatusType &MediaStatusType::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::String &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_MediaStatusType_literals_[v]);

	return *this;
}

// SidebarsByValType
//

const SidebarsByValType::EntrySequence &SidebarsByValType::getEntry() const {
	return this->entry_;
}

SidebarsByValType::EntrySequence &SidebarsByValType::getEntry() {
	return this->entry_;
}

void SidebarsByValType::setEntry(const EntrySequence &s) {
	this->entry_ = s;
}

const SidebarsByValType::StateType &SidebarsByValType::getState() const {
	return this->state_.get();
}

SidebarsByValType::StateType &SidebarsByValType::getState() {
	return this->state_.get();
}

void SidebarsByValType::setState(const StateType &x) {
	this->state_.set(x);
}

void SidebarsByValType::setState(::std::unique_ptr<StateType> x) {
	this->state_.set(std::move(x));
}

::std::unique_ptr<SidebarsByValType::StateType> SidebarsByValType::setDetachState() {
	return this->state_.detach();
}

const SidebarsByValType::StateType &SidebarsByValType::getStateDefaultValue() {
	return state_default_value_;
}

const SidebarsByValType::AnyAttributeSet &SidebarsByValType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarsByValType::AnyAttributeSet &SidebarsByValType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarsByValType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarsByValType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarsByValType::getDomDocument() {
	return *this->dom_document_;
}
} // namespace ConferenceInfo
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
namespace ConferenceInfo {
// ConferenceType
//

const ConferenceType::StateType ConferenceType::state_default_value_("full");

ConferenceType::ConferenceType(const EntityType &entity)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      conference_description_(this), host_info_(this), conference_state_(this), users_(this), sidebars_by_ref_(this),
      sidebars_by_val_(this), any_(this->getDomDocument()), entity_(entity, this), state_(getStateDefaultValue(), this),
      version_(this), any_attribute_(this->getDomDocument()) {
}

ConferenceType::ConferenceType(const ConferenceType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      conference_description_(x.conference_description_, f, this), host_info_(x.host_info_, f, this),
      conference_state_(x.conference_state_, f, this), users_(x.users_, f, this),
      sidebars_by_ref_(x.sidebars_by_ref_, f, this), sidebars_by_val_(x.sidebars_by_val_, f, this),
      any_(x.any_, this->getDomDocument()), entity_(x.entity_, f, this), state_(x.state_, f, this),
      version_(x.version_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConferenceType::ConferenceType(const ::xercesc::DOMElement &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), conference_description_(this), host_info_(this),
      conference_state_(this), users_(this), sidebars_by_ref_(this), sidebars_by_val_(this),
      any_(this->getDomDocument()), entity_(this), state_(this), version_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConferenceType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// conference-description
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "conference-description", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<ConferenceDescriptionType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->conference_description_) {
					::std::unique_ptr<ConferenceDescriptionType> r(
					    dynamic_cast<ConferenceDescriptionType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->conference_description_.set(::std::move(r));
					continue;
				}
			}
		}

		// host-info
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "host-info", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<HostInfoType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->host_info_) {
					::std::unique_ptr<HostInfoType> r(dynamic_cast<HostInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->host_info_.set(::std::move(r));
					continue;
				}
			}
		}

		// conference-state
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "conference-state", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<ConferenceStateType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->conference_state_) {
					::std::unique_ptr<ConferenceStateType> r(dynamic_cast<ConferenceStateType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->conference_state_.set(::std::move(r));
					continue;
				}
			}
		}

		// users
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "users", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<UsersType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->users_) {
					::std::unique_ptr<UsersType> r(dynamic_cast<UsersType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->users_.set(::std::move(r));
					continue;
				}
			}
		}

		// sidebars-by-ref
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebars-by-ref", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<SidebarsByRefType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->sidebars_by_ref_) {
					::std::unique_ptr<SidebarsByRefType> r(dynamic_cast<SidebarsByRefType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebars_by_ref_.set(::std::move(r));
					continue;
				}
			}
		}

		// sidebars-by-val
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebars-by-val", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<SidebarsByValType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->sidebars_by_val_) {
					::std::unique_ptr<SidebarsByValType> r(dynamic_cast<SidebarsByValType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebars_by_val_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "entity" && n.namespace_().empty()) {
			this->entity_.set(EntityTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "state" && n.namespace_().empty()) {
			this->state_.set(StateTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "version" && n.namespace_().empty()) {
			this->version_.set(VersionTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!entity_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("entity", "");
	}

	if (!state_.present()) {
		this->state_.set(getStateDefaultValue());
	}
}

ConferenceType *ConferenceType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceType(*this, f, c);
}

ConferenceType &ConferenceType::operator=(const ConferenceType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->conference_description_ = x.conference_description_;
		this->host_info_ = x.host_info_;
		this->conference_state_ = x.conference_state_;
		this->users_ = x.users_;
		this->sidebars_by_ref_ = x.sidebars_by_ref_;
		this->sidebars_by_val_ = x.sidebars_by_val_;
		this->any_ = x.any_;
		this->entity_ = x.entity_;
		this->state_ = x.state_;
		this->version_ = x.version_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConferenceType::~ConferenceType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConferenceType>
    _xsd_ConferenceType_type_factory_init("conference-type", "urn:ietf:params:xml:ns:conference-info");

// StateType
//

StateType::StateType(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
	_xsd_StateType_convert();
}

StateType::StateType(const ::xercesc::DOMAttr &a,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
	_xsd_StateType_convert();
}

StateType::StateType(const ::std::string &s,
                     const ::xercesc::DOMElement *e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
	_xsd_StateType_convert();
}

StateType *StateType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class StateType(*this, f, c);
}

StateType::Value StateType::_xsd_StateType_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_StateType_literals_);
	const Value *i(::std::lower_bound(_xsd_StateType_indexes_, _xsd_StateType_indexes_ + 3, *this, c));

	if (i == _xsd_StateType_indexes_ + 3 || _xsd_StateType_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const StateType::_xsd_StateType_literals_[3] = {"full", "partial", "deleted"};

const StateType::Value StateType::_xsd_StateType_indexes_[3] = {
    ::LinphonePrivate::Xsd::ConferenceInfo::StateType::deleted, ::LinphonePrivate::Xsd::ConferenceInfo::StateType::full,
    ::LinphonePrivate::Xsd::ConferenceInfo::StateType::partial};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, StateType>
    _xsd_StateType_type_factory_init("state-type", "urn:ietf:params:xml:ns:conference-info");

// ConferenceDescriptionType
//

ConferenceDescriptionType::ConferenceDescriptionType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), subject_(this), free_text_(this), keywords_(this), conf_uris_(this), service_uris_(this),
      maximum_user_count_(this), available_media_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
}

ConferenceDescriptionType::ConferenceDescriptionType(const ConferenceDescriptionType &x,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(x.display_text_, f, this), subject_(x.subject_, f, this), free_text_(x.free_text_, f, this),
      keywords_(x.keywords_, f, this), conf_uris_(x.conf_uris_, f, this), service_uris_(x.service_uris_, f, this),
      maximum_user_count_(x.maximum_user_count_, f, this), available_media_(x.available_media_, f, this),
      any_(x.any_, this->getDomDocument()), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConferenceDescriptionType::ConferenceDescriptionType(const ::xercesc::DOMElement &e,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), display_text_(this), subject_(this),
      free_text_(this), keywords_(this), conf_uris_(this), service_uris_(this), maximum_user_count_(this),
      available_media_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConferenceDescriptionType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// display-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "display-text", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisplayTextType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->display_text_) {
					::std::unique_ptr<DisplayTextType> r(dynamic_cast<DisplayTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->display_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// subject
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "subject", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<SubjectType>,
			    false, true, i, n, f, this));

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

		// free-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "free-text", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<FreeTextType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->free_text_) {
					::std::unique_ptr<FreeTextType> r(dynamic_cast<FreeTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->free_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// keywords
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "keywords", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<KeywordsType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->keywords_) {
					::std::unique_ptr<KeywordsType> r(dynamic_cast<KeywordsType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->keywords_.set(::std::move(r));
					continue;
				}
			}
		}

		// conf-uris
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "conf-uris", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<ConfUrisType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->conf_uris_) {
					::std::unique_ptr<ConfUrisType> r(dynamic_cast<ConfUrisType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->conf_uris_.set(::std::move(r));
					continue;
				}
			}
		}

		// service-uris
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "service-uris", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<ServiceUrisType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->service_uris_) {
					::std::unique_ptr<ServiceUrisType> r(dynamic_cast<ServiceUrisType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->service_uris_.set(::std::move(r));
					continue;
				}
			}
		}

		// maximum-user-count
		//
		if (n.name() == "maximum-user-count" && n.namespace_() == "urn:ietf:params:xml:ns:conference-info") {
			if (!this->maximum_user_count_) {
				this->maximum_user_count_.set(MaximumUserCountTraits::create(i, f, this));
				continue;
			}
		}

		// available-media
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "available-media", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<AvailableMediaType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->available_media_) {
					::std::unique_ptr<AvailableMediaType> r(dynamic_cast<AvailableMediaType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->available_media_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

ConferenceDescriptionType *ConferenceDescriptionType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceDescriptionType(*this, f, c);
}

ConferenceDescriptionType &ConferenceDescriptionType::operator=(const ConferenceDescriptionType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->display_text_ = x.display_text_;
		this->subject_ = x.subject_;
		this->free_text_ = x.free_text_;
		this->keywords_ = x.keywords_;
		this->conf_uris_ = x.conf_uris_;
		this->service_uris_ = x.service_uris_;
		this->maximum_user_count_ = x.maximum_user_count_;
		this->available_media_ = x.available_media_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConferenceDescriptionType::~ConferenceDescriptionType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConferenceDescriptionType>
    _xsd_ConferenceDescriptionType_type_factory_init("conference-description-type",
                                                     "urn:ietf:params:xml:ns:conference-info");

// HostType
//

HostType::HostType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), web_page_(this), uris_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
}

HostType::HostType(const HostType &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(x.display_text_, f, this), web_page_(x.web_page_, f, this), uris_(x.uris_, f, this),
      any_(x.any_, this->getDomDocument()), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

HostType::HostType(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), display_text_(this), web_page_(this), uris_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void HostType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// display-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "display-text", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisplayTextType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->display_text_) {
					::std::unique_ptr<DisplayTextType> r(dynamic_cast<DisplayTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->display_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// web-page
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "web-page", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<WebPageType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->web_page_) {
					::std::unique_ptr<WebPageType> r(dynamic_cast<WebPageType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->web_page_.set(::std::move(r));
					continue;
				}
			}
		}

		// uris
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "uris", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<UrisType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->uris_) {
					::std::unique_ptr<UrisType> r(dynamic_cast<UrisType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->uris_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

HostType *HostType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class HostType(*this, f, c);
}

HostType &HostType::operator=(const HostType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->display_text_ = x.display_text_;
		this->web_page_ = x.web_page_;
		this->uris_ = x.uris_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

HostType::~HostType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, HostType>
    _xsd_HostType_type_factory_init("host-type", "urn:ietf:params:xml:ns:conference-info");

// ConferenceStateType
//

ConferenceStateType::ConferenceStateType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      user_count_(this), active_(this), locked_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
}

ConferenceStateType::ConferenceStateType(const ConferenceStateType &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      user_count_(x.user_count_, f, this), active_(x.active_, f, this), locked_(x.locked_, f, this),
      any_(x.any_, this->getDomDocument()), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConferenceStateType::ConferenceStateType(const ::xercesc::DOMElement &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), user_count_(this), active_(this), locked_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConferenceStateType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// user-count
		//
		if (n.name() == "user-count" && n.namespace_() == "urn:ietf:params:xml:ns:conference-info") {
			if (!this->user_count_) {
				this->user_count_.set(UserCountTraits::create(i, f, this));
				continue;
			}
		}

		// active
		//
		if (n.name() == "active" && n.namespace_() == "urn:ietf:params:xml:ns:conference-info") {
			if (!this->active_) {
				this->active_.set(ActiveTraits::create(i, f, this));
				continue;
			}
		}

		// locked
		//
		if (n.name() == "locked" && n.namespace_() == "urn:ietf:params:xml:ns:conference-info") {
			if (!this->locked_) {
				this->locked_.set(LockedTraits::create(i, f, this));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

ConferenceStateType *ConferenceStateType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceStateType(*this, f, c);
}

ConferenceStateType &ConferenceStateType::operator=(const ConferenceStateType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->user_count_ = x.user_count_;
		this->active_ = x.active_;
		this->locked_ = x.locked_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConferenceStateType::~ConferenceStateType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConferenceStateType>
    _xsd_ConferenceStateType_type_factory_init("conference-state-type", "urn:ietf:params:xml:ns:conference-info");

// ConferenceMediaType
//

ConferenceMediaType::ConferenceMediaType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(this), any_attribute_(this->getDomDocument()) {
}

ConferenceMediaType::ConferenceMediaType(const ConferenceMediaType &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(x.entry_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConferenceMediaType::ConferenceMediaType(const ::xercesc::DOMElement &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), entry_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConferenceMediaType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// entry
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "entry", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<EntryType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<EntryType> r(dynamic_cast<EntryType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->entry_.push_back(::std::move(r));
				continue;
			}
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

ConferenceMediaType *ConferenceMediaType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceMediaType(*this, f, c);
}

ConferenceMediaType &ConferenceMediaType::operator=(const ConferenceMediaType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->entry_ = x.entry_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConferenceMediaType::~ConferenceMediaType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConferenceMediaType>
    _xsd_ConferenceMediaType_type_factory_init("conference-media-type", "urn:ietf:params:xml:ns:conference-info");

// ConferenceMediumType
//

ConferenceMediumType::ConferenceMediumType(const TypeType &type, const LabelType &label)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), type_(type, this), status_(this), any_(this->getDomDocument()), label_(label, this),
      any_attribute_(this->getDomDocument()) {
}

ConferenceMediumType::ConferenceMediumType(::std::unique_ptr<TypeType> type, const LabelType &label)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), type_(std::move(type), this), status_(this), any_(this->getDomDocument()),
      label_(label, this), any_attribute_(this->getDomDocument()) {
}

ConferenceMediumType::ConferenceMediumType(const ConferenceMediumType &x,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(x.display_text_, f, this), type_(x.type_, f, this), status_(x.status_, f, this),
      any_(x.any_, this->getDomDocument()), label_(x.label_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConferenceMediumType::ConferenceMediumType(const ::xercesc::DOMElement &e,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), display_text_(this), type_(this), status_(this),
      any_(this->getDomDocument()), label_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConferenceMediumType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// display-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "display-text", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisplayTextType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->display_text_) {
					::std::unique_ptr<DisplayTextType> r(dynamic_cast<DisplayTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->display_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// type
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "type", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<TypeType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!type_.present()) {
					::std::unique_ptr<TypeType> r(dynamic_cast<TypeType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->type_.set(::std::move(r));
					continue;
				}
			}
		}

		// status
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "status", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<StatusType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->status_) {
					::std::unique_ptr<StatusType> r(dynamic_cast<StatusType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->status_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!type_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("type", "urn:ietf:params:xml:ns:conference-info");
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "label" && n.namespace_().empty()) {
			this->label_.set(LabelTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!label_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("label", "");
	}
}

ConferenceMediumType *ConferenceMediumType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceMediumType(*this, f, c);
}

ConferenceMediumType &ConferenceMediumType::operator=(const ConferenceMediumType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->display_text_ = x.display_text_;
		this->type_ = x.type_;
		this->status_ = x.status_;
		this->any_ = x.any_;
		this->label_ = x.label_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConferenceMediumType::~ConferenceMediumType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConferenceMediumType>
    _xsd_ConferenceMediumType_type_factory_init("conference-medium-type", "urn:ietf:params:xml:ns:conference-info");

// UrisType
//

const UrisType::StateType UrisType::state_default_value_("full");

UrisType::UrisType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(this), state_(getStateDefaultValue(), this), any_attribute_(this->getDomDocument()) {
}

UrisType::UrisType(const UrisType &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(x.entry_, f, this), state_(x.state_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UrisType::UrisType(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), entry_(this), state_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UrisType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// entry
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "entry", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<EntryType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<EntryType> r(dynamic_cast<EntryType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->entry_.push_back(::std::move(r));
				continue;
			}
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "state" && n.namespace_().empty()) {
			this->state_.set(StateTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!state_.present()) {
		this->state_.set(getStateDefaultValue());
	}
}

UrisType *UrisType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UrisType(*this, f, c);
}

UrisType &UrisType::operator=(const UrisType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->entry_ = x.entry_;
		this->state_ = x.state_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UrisType::~UrisType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UrisType>
    _xsd_UrisType_type_factory_init("uris-type", "urn:ietf:params:xml:ns:conference-info");

// UriType
//

UriType::UriType(const UriType1 &uri)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      uri_(uri, this), display_text_(this), purpose_(this), modified_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
}

UriType::UriType(::std::unique_ptr<UriType1> uri)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      uri_(std::move(uri), this), display_text_(this), purpose_(this), modified_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
}

UriType::UriType(const UriType &x,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      uri_(x.uri_, f, this), display_text_(x.display_text_, f, this), purpose_(x.purpose_, f, this),
      modified_(x.modified_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UriType::UriType(const ::xercesc::DOMElement &e,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), uri_(this), display_text_(this), purpose_(this),
      modified_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UriType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// uri
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "uri", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<UriType1>, false, true,
			    i, n, f, this));

			if (tmp.get() != 0) {
				if (!uri_.present()) {
					::std::unique_ptr<UriType1> r(dynamic_cast<UriType1 *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->uri_.set(::std::move(r));
					continue;
				}
			}
		}

		// display-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "display-text", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisplayTextType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->display_text_) {
					::std::unique_ptr<DisplayTextType> r(dynamic_cast<DisplayTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->display_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// purpose
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "purpose", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<PurposeType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->purpose_) {
					::std::unique_ptr<PurposeType> r(dynamic_cast<PurposeType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->purpose_.set(::std::move(r));
					continue;
				}
			}
		}

		// modified
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "modified", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<ModifiedType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->modified_) {
					::std::unique_ptr<ModifiedType> r(dynamic_cast<ModifiedType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->modified_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!uri_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("uri", "urn:ietf:params:xml:ns:conference-info");
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

UriType *UriType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UriType(*this, f, c);
}

UriType &UriType::operator=(const UriType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->uri_ = x.uri_;
		this->display_text_ = x.display_text_;
		this->purpose_ = x.purpose_;
		this->modified_ = x.modified_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UriType::~UriType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UriType>
    _xsd_UriType_type_factory_init("uri-type", "urn:ietf:params:xml:ns:conference-info");

// KeywordsType
//

KeywordsType::KeywordsType(const ::xercesc::DOMElement &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::SimpleType(e, f, c),
      ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char>(e, f, this) {
}

KeywordsType::KeywordsType(const ::xercesc::DOMAttr &a,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::SimpleType(a, f, c),
      ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char>(a, f, this) {
}

KeywordsType::KeywordsType(const ::std::string &s,
                           const ::xercesc::DOMElement *e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::SimpleType(s, e, f, c),
      ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char>(s, e, f, this) {
}

KeywordsType *KeywordsType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class KeywordsType(*this, f, c);
}

KeywordsType::~KeywordsType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, KeywordsType>
    _xsd_KeywordsType_type_factory_init("keywords-type", "urn:ietf:params:xml:ns:conference-info");

// UsersType
//

const UsersType::StateType UsersType::state_default_value_("full");

UsersType::UsersType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      user_(this), any_(this->getDomDocument()), state_(getStateDefaultValue(), this),
      any_attribute_(this->getDomDocument()) {
}

UsersType::UsersType(const UsersType &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      user_(x.user_, f, this), any_(x.any_, this->getDomDocument()), state_(x.state_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UsersType::UsersType(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), user_(this), any_(this->getDomDocument()),
      state_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UsersType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// user
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "user", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<UserType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<UserType> r(dynamic_cast<UserType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->user_.push_back(::std::move(r));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "state" && n.namespace_().empty()) {
			this->state_.set(StateTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!state_.present()) {
		this->state_.set(getStateDefaultValue());
	}
}

UsersType *UsersType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UsersType(*this, f, c);
}

UsersType &UsersType::operator=(const UsersType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->user_ = x.user_;
		this->any_ = x.any_;
		this->state_ = x.state_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UsersType::~UsersType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UsersType>
    _xsd_UsersType_type_factory_init("users-type", "urn:ietf:params:xml:ns:conference-info");

// UserType
//

const UserType::StateType UserType::state_default_value_("full");

UserType::UserType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), associated_aors_(this), roles_(this), languages_(this), cascaded_focus_(this),
      endpoint_(this), any_(this->getDomDocument()), entity_(this), state_(getStateDefaultValue(), this),
      any_attribute_(this->getDomDocument()) {
}

UserType::UserType(const UserType &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(x.display_text_, f, this), associated_aors_(x.associated_aors_, f, this), roles_(x.roles_, f, this),
      languages_(x.languages_, f, this), cascaded_focus_(x.cascaded_focus_, f, this), endpoint_(x.endpoint_, f, this),
      any_(x.any_, this->getDomDocument()), entity_(x.entity_, f, this), state_(x.state_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UserType::UserType(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), display_text_(this), associated_aors_(this),
      roles_(this), languages_(this), cascaded_focus_(this), endpoint_(this), any_(this->getDomDocument()),
      entity_(this), state_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UserType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// display-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "display-text", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisplayTextType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->display_text_) {
					::std::unique_ptr<DisplayTextType> r(dynamic_cast<DisplayTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->display_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// associated-aors
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "associated-aors", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<AssociatedAorsType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->associated_aors_) {
					::std::unique_ptr<AssociatedAorsType> r(dynamic_cast<AssociatedAorsType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->associated_aors_.set(::std::move(r));
					continue;
				}
			}
		}

		// roles
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "roles", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<RolesType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->roles_) {
					::std::unique_ptr<RolesType> r(dynamic_cast<RolesType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->roles_.set(::std::move(r));
					continue;
				}
			}
		}

		// languages
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "languages", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<LanguagesType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->languages_) {
					::std::unique_ptr<LanguagesType> r(dynamic_cast<LanguagesType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->languages_.set(::std::move(r));
					continue;
				}
			}
		}

		// cascaded-focus
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "cascaded-focus", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<CascadedFocusType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->cascaded_focus_) {
					::std::unique_ptr<CascadedFocusType> r(dynamic_cast<CascadedFocusType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->cascaded_focus_.set(::std::move(r));
					continue;
				}
			}
		}

		// endpoint
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "endpoint", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<EndpointType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<EndpointType> r(dynamic_cast<EndpointType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->endpoint_.push_back(::std::move(r));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "entity" && n.namespace_().empty()) {
			this->entity_.set(EntityTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "state" && n.namespace_().empty()) {
			this->state_.set(StateTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!state_.present()) {
		this->state_.set(getStateDefaultValue());
	}
}

UserType *UserType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UserType(*this, f, c);
}

UserType &UserType::operator=(const UserType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->display_text_ = x.display_text_;
		this->associated_aors_ = x.associated_aors_;
		this->roles_ = x.roles_;
		this->languages_ = x.languages_;
		this->cascaded_focus_ = x.cascaded_focus_;
		this->endpoint_ = x.endpoint_;
		this->any_ = x.any_;
		this->entity_ = x.entity_;
		this->state_ = x.state_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UserType::~UserType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UserType>
    _xsd_UserType_type_factory_init("user-type", "urn:ietf:params:xml:ns:conference-info");

// UserRolesType
//

UserRolesType::UserRolesType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(this), any_attribute_(this->getDomDocument()) {
}

UserRolesType::UserRolesType(const UserRolesType &x,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(x.entry_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UserRolesType::UserRolesType(const ::xercesc::DOMElement &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), entry_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UserRolesType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// entry
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "entry", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<EntryType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<EntryType> r(dynamic_cast<EntryType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->entry_.push_back(::std::move(r));
				continue;
			}
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

UserRolesType *UserRolesType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UserRolesType(*this, f, c);
}

UserRolesType &UserRolesType::operator=(const UserRolesType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->entry_ = x.entry_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UserRolesType::~UserRolesType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UserRolesType>
    _xsd_UserRolesType_type_factory_init("user-roles-type", "urn:ietf:params:xml:ns:conference-info");

// UserLanguagesType
//

UserLanguagesType::UserLanguagesType(const ::xercesc::DOMElement &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::SimpleType(e, f, c),
      ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char>(e, f, this) {
}

UserLanguagesType::UserLanguagesType(const ::xercesc::DOMAttr &a,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::SimpleType(a, f, c),
      ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char>(a, f, this) {
}

UserLanguagesType::UserLanguagesType(const ::std::string &s,
                                     const ::xercesc::DOMElement *e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::SimpleType(s, e, f, c),
      ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char>(s, e, f, this) {
}

UserLanguagesType *UserLanguagesType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UserLanguagesType(*this, f, c);
}

UserLanguagesType::~UserLanguagesType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UserLanguagesType>
    _xsd_UserLanguagesType_type_factory_init("user-languages-type", "urn:ietf:params:xml:ns:conference-info");

// EndpointType
//

const EndpointType::StateType EndpointType::state_default_value_("full");

EndpointType::EndpointType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), referred_(this), status_(this), joining_method_(this), joining_info_(this),
      disconnection_method_(this), disconnection_info_(this), media_(this), call_info_(this),
      any_(this->getDomDocument()), entity_(this), state_(getStateDefaultValue(), this),
      any_attribute_(this->getDomDocument()) {
}

EndpointType::EndpointType(const EndpointType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(x.display_text_, f, this), referred_(x.referred_, f, this), status_(x.status_, f, this),
      joining_method_(x.joining_method_, f, this), joining_info_(x.joining_info_, f, this),
      disconnection_method_(x.disconnection_method_, f, this), disconnection_info_(x.disconnection_info_, f, this),
      media_(x.media_, f, this), call_info_(x.call_info_, f, this), any_(x.any_, this->getDomDocument()),
      entity_(x.entity_, f, this), state_(x.state_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

EndpointType::EndpointType(const ::xercesc::DOMElement &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), display_text_(this), referred_(this), status_(this),
      joining_method_(this), joining_info_(this), disconnection_method_(this), disconnection_info_(this), media_(this),
      call_info_(this), any_(this->getDomDocument()), entity_(this), state_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void EndpointType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// display-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "display-text", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisplayTextType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->display_text_) {
					::std::unique_ptr<DisplayTextType> r(dynamic_cast<DisplayTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->display_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// referred
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "referred", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<ReferredType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->referred_) {
					::std::unique_ptr<ReferredType> r(dynamic_cast<ReferredType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->referred_.set(::std::move(r));
					continue;
				}
			}
		}

		// status
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "status", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<StatusType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->status_) {
					::std::unique_ptr<StatusType> r(dynamic_cast<StatusType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->status_.set(::std::move(r));
					continue;
				}
			}
		}

		// joining-method
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "joining-method", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<JoiningMethodType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->joining_method_) {
					::std::unique_ptr<JoiningMethodType> r(dynamic_cast<JoiningMethodType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->joining_method_.set(::std::move(r));
					continue;
				}
			}
		}

		// joining-info
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "joining-info", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<JoiningInfoType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->joining_info_) {
					::std::unique_ptr<JoiningInfoType> r(dynamic_cast<JoiningInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->joining_info_.set(::std::move(r));
					continue;
				}
			}
		}

		// disconnection-method
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "disconnection-method", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisconnectionMethodType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->disconnection_method_) {
					::std::unique_ptr<DisconnectionMethodType> r(dynamic_cast<DisconnectionMethodType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->disconnection_method_.set(::std::move(r));
					continue;
				}
			}
		}

		// disconnection-info
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "disconnection-info", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisconnectionInfoType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->disconnection_info_) {
					::std::unique_ptr<DisconnectionInfoType> r(dynamic_cast<DisconnectionInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->disconnection_info_.set(::std::move(r));
					continue;
				}
			}
		}

		// media
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "media", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<MediaType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<MediaType> r(dynamic_cast<MediaType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->media_.push_back(::std::move(r));
				continue;
			}
		}

		// call-info
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "call-info", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<CallInfoType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->call_info_) {
					::std::unique_ptr<CallInfoType> r(dynamic_cast<CallInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->call_info_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "entity" && n.namespace_().empty()) {
			this->entity_.set(EntityTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "state" && n.namespace_().empty()) {
			this->state_.set(StateTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!state_.present()) {
		this->state_.set(getStateDefaultValue());
	}
}

EndpointType *EndpointType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class EndpointType(*this, f, c);
}

EndpointType &EndpointType::operator=(const EndpointType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->display_text_ = x.display_text_;
		this->referred_ = x.referred_;
		this->status_ = x.status_;
		this->joining_method_ = x.joining_method_;
		this->joining_info_ = x.joining_info_;
		this->disconnection_method_ = x.disconnection_method_;
		this->disconnection_info_ = x.disconnection_info_;
		this->media_ = x.media_;
		this->call_info_ = x.call_info_;
		this->any_ = x.any_;
		this->entity_ = x.entity_;
		this->state_ = x.state_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

EndpointType::~EndpointType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, EndpointType>
    _xsd_EndpointType_type_factory_init("endpoint-type", "urn:ietf:params:xml:ns:conference-info");

// EndpointStatusType
//

EndpointStatusType::EndpointStatusType(const ::xercesc::DOMElement &e,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
	_xsd_EndpointStatusType_convert();
}

EndpointStatusType::EndpointStatusType(const ::xercesc::DOMAttr &a,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
	_xsd_EndpointStatusType_convert();
}

EndpointStatusType::EndpointStatusType(const ::std::string &s,
                                       const ::xercesc::DOMElement *e,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
	_xsd_EndpointStatusType_convert();
}

EndpointStatusType *EndpointStatusType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class EndpointStatusType(*this, f, c);
}

EndpointStatusType::Value EndpointStatusType::_xsd_EndpointStatusType_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_EndpointStatusType_literals_);
	const Value *i(
	    ::std::lower_bound(_xsd_EndpointStatusType_indexes_, _xsd_EndpointStatusType_indexes_ + 9, *this, c));

	if (i == _xsd_EndpointStatusType_indexes_ + 9 || _xsd_EndpointStatusType_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const EndpointStatusType::_xsd_EndpointStatusType_literals_[9] = {
    "pending",   "dialing-out",     "dialing-in",    "alerting",    "on-hold",
    "connected", "muted-via-focus", "disconnecting", "disconnected"};

const EndpointStatusType::Value EndpointStatusType::_xsd_EndpointStatusType_indexes_[9] = {
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::alerting,
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::connected,
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::dialing_in,
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::dialing_out,
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::disconnected,
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::disconnecting,
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::muted_via_focus,
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::on_hold,
    ::LinphonePrivate::Xsd::ConferenceInfo::EndpointStatusType::pending};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, EndpointStatusType>
    _xsd_EndpointStatusType_type_factory_init("endpoint-status-type", "urn:ietf:params:xml:ns:conference-info");

// JoiningType
//

JoiningType::JoiningType(const ::xercesc::DOMElement &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
	_xsd_JoiningType_convert();
}

JoiningType::JoiningType(const ::xercesc::DOMAttr &a,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
	_xsd_JoiningType_convert();
}

JoiningType::JoiningType(const ::std::string &s,
                         const ::xercesc::DOMElement *e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
	_xsd_JoiningType_convert();
}

JoiningType *JoiningType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class JoiningType(*this, f, c);
}

JoiningType::Value JoiningType::_xsd_JoiningType_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_JoiningType_literals_);
	const Value *i(::std::lower_bound(_xsd_JoiningType_indexes_, _xsd_JoiningType_indexes_ + 3, *this, c));

	if (i == _xsd_JoiningType_indexes_ + 3 || _xsd_JoiningType_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const JoiningType::_xsd_JoiningType_literals_[3] = {"dialed-in", "dialed-out", "focus-owner"};

const JoiningType::Value JoiningType::_xsd_JoiningType_indexes_[3] = {
    ::LinphonePrivate::Xsd::ConferenceInfo::JoiningType::dialed_in,
    ::LinphonePrivate::Xsd::ConferenceInfo::JoiningType::dialed_out,
    ::LinphonePrivate::Xsd::ConferenceInfo::JoiningType::focus_owner};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, JoiningType>
    _xsd_JoiningType_type_factory_init("joining-type", "urn:ietf:params:xml:ns:conference-info");

// DisconnectionType
//

DisconnectionType::DisconnectionType(const ::xercesc::DOMElement &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
	_xsd_DisconnectionType_convert();
}

DisconnectionType::DisconnectionType(const ::xercesc::DOMAttr &a,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
	_xsd_DisconnectionType_convert();
}

DisconnectionType::DisconnectionType(const ::std::string &s,
                                     const ::xercesc::DOMElement *e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
	_xsd_DisconnectionType_convert();
}

DisconnectionType *DisconnectionType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class DisconnectionType(*this, f, c);
}

DisconnectionType::Value DisconnectionType::_xsd_DisconnectionType_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_DisconnectionType_literals_);
	const Value *i(::std::lower_bound(_xsd_DisconnectionType_indexes_, _xsd_DisconnectionType_indexes_ + 4, *this, c));

	if (i == _xsd_DisconnectionType_indexes_ + 4 || _xsd_DisconnectionType_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const DisconnectionType::_xsd_DisconnectionType_literals_[4] = {"departed", "booted", "failed", "busy"};

const DisconnectionType::Value DisconnectionType::_xsd_DisconnectionType_indexes_[4] = {
    ::LinphonePrivate::Xsd::ConferenceInfo::DisconnectionType::booted,
    ::LinphonePrivate::Xsd::ConferenceInfo::DisconnectionType::busy,
    ::LinphonePrivate::Xsd::ConferenceInfo::DisconnectionType::departed,
    ::LinphonePrivate::Xsd::ConferenceInfo::DisconnectionType::failed};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, DisconnectionType>
    _xsd_DisconnectionType_type_factory_init("disconnection-type", "urn:ietf:params:xml:ns:conference-info");

// ExecutionType
//

ExecutionType::ExecutionType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      when_(this), reason_(this), by_(this), any_attribute_(this->getDomDocument()) {
}

ExecutionType::ExecutionType(const ExecutionType &x,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      when_(x.when_, f, this), reason_(x.reason_, f, this), by_(x.by_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ExecutionType::ExecutionType(const ::xercesc::DOMElement &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), when_(this), reason_(this), by_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ExecutionType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// when
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "when", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<WhenType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->when_) {
					::std::unique_ptr<WhenType> r(dynamic_cast<WhenType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->when_.set(::std::move(r));
					continue;
				}
			}
		}

		// reason
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "reason", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<ReasonType>, false,
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

		// by
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "by", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<ByType>, false, true, i,
			    n, f, this));

			if (tmp.get() != 0) {
				if (!this->by_) {
					::std::unique_ptr<ByType> r(dynamic_cast<ByType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->by_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

ExecutionType *ExecutionType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ExecutionType(*this, f, c);
}

ExecutionType &ExecutionType::operator=(const ExecutionType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->when_ = x.when_;
		this->reason_ = x.reason_;
		this->by_ = x.by_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ExecutionType::~ExecutionType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ExecutionType>
    _xsd_ExecutionType_type_factory_init("execution-type", "urn:ietf:params:xml:ns:conference-info");

// CallType
//

CallType::CallType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sip_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

CallType::CallType(const CallType &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sip_(x.sip_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

CallType::CallType(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), sip_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CallType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sip
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sip", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<SipType>, false, true,
			    i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->sip_) {
					::std::unique_ptr<SipType> r(dynamic_cast<SipType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sip_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

CallType *CallType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CallType(*this, f, c);
}

CallType &CallType::operator=(const CallType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->sip_ = x.sip_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

CallType::~CallType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CallType>
    _xsd_CallType_type_factory_init("call-type", "urn:ietf:params:xml:ns:conference-info");

// SipDialogIdType
//

SipDialogIdType::SipDialogIdType(const CallIdType &call_id, const FromTagType &from_tag, const ToTagType &to_tag)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), call_id_(call_id, this), from_tag_(from_tag, this), to_tag_(to_tag, this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SipDialogIdType::SipDialogIdType(::std::unique_ptr<CallIdType> call_id,
                                 ::std::unique_ptr<FromTagType> from_tag,
                                 ::std::unique_ptr<ToTagType> to_tag)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), call_id_(std::move(call_id), this), from_tag_(std::move(from_tag), this),
      to_tag_(std::move(to_tag), this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SipDialogIdType::SipDialogIdType(const SipDialogIdType &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(x.display_text_, f, this), call_id_(x.call_id_, f, this), from_tag_(x.from_tag_, f, this),
      to_tag_(x.to_tag_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SipDialogIdType::SipDialogIdType(const ::xercesc::DOMElement &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), display_text_(this), call_id_(this),
      from_tag_(this), to_tag_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SipDialogIdType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// display-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "display-text", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisplayTextType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->display_text_) {
					::std::unique_ptr<DisplayTextType> r(dynamic_cast<DisplayTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->display_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// call-id
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "call-id", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<CallIdType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!call_id_.present()) {
					::std::unique_ptr<CallIdType> r(dynamic_cast<CallIdType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->call_id_.set(::std::move(r));
					continue;
				}
			}
		}

		// from-tag
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "from-tag", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<FromTagType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!from_tag_.present()) {
					::std::unique_ptr<FromTagType> r(dynamic_cast<FromTagType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->from_tag_.set(::std::move(r));
					continue;
				}
			}
		}

		// to-tag
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "to-tag", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<ToTagType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!to_tag_.present()) {
					::std::unique_ptr<ToTagType> r(dynamic_cast<ToTagType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->to_tag_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!call_id_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("call-id", "urn:ietf:params:xml:ns:conference-info");
	}

	if (!from_tag_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("from-tag", "urn:ietf:params:xml:ns:conference-info");
	}

	if (!to_tag_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("to-tag", "urn:ietf:params:xml:ns:conference-info");
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

SipDialogIdType *SipDialogIdType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SipDialogIdType(*this, f, c);
}

SipDialogIdType &SipDialogIdType::operator=(const SipDialogIdType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->display_text_ = x.display_text_;
		this->call_id_ = x.call_id_;
		this->from_tag_ = x.from_tag_;
		this->to_tag_ = x.to_tag_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SipDialogIdType::~SipDialogIdType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SipDialogIdType>
    _xsd_SipDialogIdType_type_factory_init("sip-dialog-id-type", "urn:ietf:params:xml:ns:conference-info");

// MediaType
//

MediaType::MediaType(const IdType &id)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(this), type_(this), label_(this), src_id_(this), status_(this), any_(this->getDomDocument()),
      id_(id, this), any_attribute_(this->getDomDocument()) {
}

MediaType::MediaType(const MediaType &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      display_text_(x.display_text_, f, this), type_(x.type_, f, this), label_(x.label_, f, this),
      src_id_(x.src_id_, f, this), status_(x.status_, f, this), any_(x.any_, this->getDomDocument()),
      id_(x.id_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

MediaType::MediaType(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), display_text_(this), type_(this), label_(this),
      src_id_(this), status_(this), any_(this->getDomDocument()), id_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void MediaType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// display-text
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "display-text", "urn:ietf:params:xml:ns:conference-info",
			    &::xsd::cxx::tree::factory_impl<DisplayTextType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->display_text_) {
					::std::unique_ptr<DisplayTextType> r(dynamic_cast<DisplayTextType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->display_text_.set(::std::move(r));
					continue;
				}
			}
		}

		// type
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "type", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<TypeType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->type_) {
					::std::unique_ptr<TypeType> r(dynamic_cast<TypeType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->type_.set(::std::move(r));
					continue;
				}
			}
		}

		// label
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "label", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<LabelType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->label_) {
					::std::unique_ptr<LabelType> r(dynamic_cast<LabelType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->label_.set(::std::move(r));
					continue;
				}
			}
		}

		// src-id
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "src-id", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<SrcIdType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->src_id_) {
					::std::unique_ptr<SrcIdType> r(dynamic_cast<SrcIdType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->src_id_.set(::std::move(r));
					continue;
				}
			}
		}

		// status
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "status", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<StatusType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->status_) {
					::std::unique_ptr<StatusType> r(dynamic_cast<StatusType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->status_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "id" && n.namespace_().empty()) {
			this->id_.set(IdTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!id_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("id", "");
	}
}

MediaType *MediaType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class MediaType(*this, f, c);
}

MediaType &MediaType::operator=(const MediaType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->display_text_ = x.display_text_;
		this->type_ = x.type_;
		this->label_ = x.label_;
		this->src_id_ = x.src_id_;
		this->status_ = x.status_;
		this->any_ = x.any_;
		this->id_ = x.id_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

MediaType::~MediaType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, MediaType>
    _xsd_MediaType_type_factory_init("media-type", "urn:ietf:params:xml:ns:conference-info");

// MediaStatusType
//

MediaStatusType::MediaStatusType(const ::xercesc::DOMElement &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
	_xsd_MediaStatusType_convert();
}

MediaStatusType::MediaStatusType(const ::xercesc::DOMAttr &a,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
	_xsd_MediaStatusType_convert();
}

MediaStatusType::MediaStatusType(const ::std::string &s,
                                 const ::xercesc::DOMElement *e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
	_xsd_MediaStatusType_convert();
}

MediaStatusType *MediaStatusType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class MediaStatusType(*this, f, c);
}

MediaStatusType::Value MediaStatusType::_xsd_MediaStatusType_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_MediaStatusType_literals_);
	const Value *i(::std::lower_bound(_xsd_MediaStatusType_indexes_, _xsd_MediaStatusType_indexes_ + 4, *this, c));

	if (i == _xsd_MediaStatusType_indexes_ + 4 || _xsd_MediaStatusType_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const MediaStatusType::_xsd_MediaStatusType_literals_[4] = {"recvonly", "sendonly", "sendrecv", "inactive"};

const MediaStatusType::Value MediaStatusType::_xsd_MediaStatusType_indexes_[4] = {
    ::LinphonePrivate::Xsd::ConferenceInfo::MediaStatusType::inactive,
    ::LinphonePrivate::Xsd::ConferenceInfo::MediaStatusType::recvonly,
    ::LinphonePrivate::Xsd::ConferenceInfo::MediaStatusType::sendonly,
    ::LinphonePrivate::Xsd::ConferenceInfo::MediaStatusType::sendrecv};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, MediaStatusType>
    _xsd_MediaStatusType_type_factory_init("media-status-type", "urn:ietf:params:xml:ns:conference-info");

// SidebarsByValType
//

const SidebarsByValType::StateType SidebarsByValType::state_default_value_("full");

SidebarsByValType::SidebarsByValType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(this), state_(getStateDefaultValue(), this), any_attribute_(this->getDomDocument()) {
}

SidebarsByValType::SidebarsByValType(const SidebarsByValType &x,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(x.entry_, f, this), state_(x.state_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarsByValType::SidebarsByValType(const ::xercesc::DOMElement &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), entry_(this), state_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarsByValType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// entry
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "entry", "urn:ietf:params:xml:ns:conference-info", &::xsd::cxx::tree::factory_impl<EntryType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<EntryType> r(dynamic_cast<EntryType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->entry_.push_back(::std::move(r));
				continue;
			}
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "state" && n.namespace_().empty()) {
			this->state_.set(StateTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:conference-info" &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!state_.present()) {
		this->state_.set(getStateDefaultValue());
	}
}

SidebarsByValType *SidebarsByValType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarsByValType(*this, f, c);
}

SidebarsByValType &SidebarsByValType::operator=(const SidebarsByValType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->entry_ = x.entry_;
		this->state_ = x.state_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarsByValType::~SidebarsByValType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarsByValType>
    _xsd_SidebarsByValType_type_factory_init("sidebars-by-val-type", "urn:ietf:params:xml:ns:conference-info");
} // namespace ConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

#include <ostream>

#include <xsd/cxx/tree/std-ostream-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::std_ostream_plate<0, char> std_ostream_plate_init;
}

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfo {
::std::ostream &operator<<(::std::ostream &o, const ConferenceType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConferenceDescription()) {
			o << ::std::endl << "conference-description: ";
			om.insert(o, *i.getConferenceDescription());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getHostInfo()) {
			o << ::std::endl << "host-info: ";
			om.insert(o, *i.getHostInfo());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConferenceState()) {
			o << ::std::endl << "conference-state: ";
			om.insert(o, *i.getConferenceState());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getUsers()) {
			o << ::std::endl << "users: ";
			om.insert(o, *i.getUsers());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSidebarsByRef()) {
			o << ::std::endl << "sidebars-by-ref: ";
			om.insert(o, *i.getSidebarsByRef());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSidebarsByVal()) {
			o << ::std::endl << "sidebars-by-val: ";
			om.insert(o, *i.getSidebarsByVal());
		}
	}

	o << ::std::endl << "entity: " << i.getEntity();
	o << ::std::endl << "state: " << i.getState();
	if (i.getVersion()) {
		o << ::std::endl << "version: " << *i.getVersion();
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConferenceType> _xsd_ConferenceType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, StateType::Value i) {
	return o << StateType::_xsd_StateType_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const StateType &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, StateType> _xsd_StateType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConferenceDescriptionType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisplayText()) {
			o << ::std::endl << "display-text: ";
			om.insert(o, *i.getDisplayText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSubject()) {
			o << ::std::endl << "subject: ";
			om.insert(o, *i.getSubject());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getFreeText()) {
			o << ::std::endl << "free-text: ";
			om.insert(o, *i.getFreeText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getKeywords()) {
			o << ::std::endl << "keywords: ";
			om.insert(o, *i.getKeywords());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConfUris()) {
			o << ::std::endl << "conf-uris: ";
			om.insert(o, *i.getConfUris());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getServiceUris()) {
			o << ::std::endl << "service-uris: ";
			om.insert(o, *i.getServiceUris());
		}
	}

	if (i.getMaximumUserCount()) {
		o << ::std::endl << "maximum-user-count: " << *i.getMaximumUserCount();
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getAvailableMedia()) {
			o << ::std::endl << "available-media: ";
			om.insert(o, *i.getAvailableMedia());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConferenceDescriptionType>
    _xsd_ConferenceDescriptionType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const HostType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisplayText()) {
			o << ::std::endl << "display-text: ";
			om.insert(o, *i.getDisplayText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getWebPage()) {
			o << ::std::endl << "web-page: ";
			om.insert(o, *i.getWebPage());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getUris()) {
			o << ::std::endl << "uris: ";
			om.insert(o, *i.getUris());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, HostType> _xsd_HostType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConferenceStateType &i) {
	if (i.getUserCount()) {
		o << ::std::endl << "user-count: " << *i.getUserCount();
	}

	if (i.getActive()) {
		o << ::std::endl << "active: " << *i.getActive();
	}

	if (i.getLocked()) {
		o << ::std::endl << "locked: " << *i.getLocked();
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConferenceStateType>
    _xsd_ConferenceStateType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConferenceMediaType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (ConferenceMediaType::EntryConstIterator b(i.getEntry().begin()), e(i.getEntry().end()); b != e; ++b) {
			o << ::std::endl << "entry: ";
			om.insert(o, *b);
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConferenceMediaType>
    _xsd_ConferenceMediaType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConferenceMediumType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisplayText()) {
			o << ::std::endl << "display-text: ";
			om.insert(o, *i.getDisplayText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "type: ";
		om.insert(o, i.getType());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getStatus()) {
			o << ::std::endl << "status: ";
			om.insert(o, *i.getStatus());
		}
	}

	o << ::std::endl << "label: " << i.getLabel();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConferenceMediumType>
    _xsd_ConferenceMediumType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UrisType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (UrisType::EntryConstIterator b(i.getEntry().begin()), e(i.getEntry().end()); b != e; ++b) {
			o << ::std::endl << "entry: ";
			om.insert(o, *b);
		}
	}

	o << ::std::endl << "state: " << i.getState();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UrisType> _xsd_UrisType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UriType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "uri: ";
		om.insert(o, i.getUri());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisplayText()) {
			o << ::std::endl << "display-text: ";
			om.insert(o, *i.getDisplayText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getPurpose()) {
			o << ::std::endl << "purpose: ";
			om.insert(o, *i.getPurpose());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getModified()) {
			o << ::std::endl << "modified: ";
			om.insert(o, *i.getModified());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UriType> _xsd_UriType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const KeywordsType &i) {
	return o << static_cast<const ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char> &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, KeywordsType> _xsd_KeywordsType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UsersType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (UsersType::UserConstIterator b(i.getUser().begin()), e(i.getUser().end()); b != e; ++b) {
			o << ::std::endl << "user: ";
			om.insert(o, *b);
		}
	}

	o << ::std::endl << "state: " << i.getState();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UsersType> _xsd_UsersType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UserType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisplayText()) {
			o << ::std::endl << "display-text: ";
			om.insert(o, *i.getDisplayText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getAssociatedAors()) {
			o << ::std::endl << "associated-aors: ";
			om.insert(o, *i.getAssociatedAors());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getRoles()) {
			o << ::std::endl << "roles: ";
			om.insert(o, *i.getRoles());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getLanguages()) {
			o << ::std::endl << "languages: ";
			om.insert(o, *i.getLanguages());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getCascadedFocus()) {
			o << ::std::endl << "cascaded-focus: ";
			om.insert(o, *i.getCascadedFocus());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (UserType::EndpointConstIterator b(i.getEndpoint().begin()), e(i.getEndpoint().end()); b != e; ++b) {
			o << ::std::endl << "endpoint: ";
			om.insert(o, *b);
		}
	}

	if (i.getEntity()) {
		o << ::std::endl << "entity: " << *i.getEntity();
	}

	o << ::std::endl << "state: " << i.getState();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UserType> _xsd_UserType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UserRolesType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (UserRolesType::EntryConstIterator b(i.getEntry().begin()), e(i.getEntry().end()); b != e; ++b) {
			o << ::std::endl << "entry: ";
			om.insert(o, *b);
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UserRolesType> _xsd_UserRolesType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UserLanguagesType &i) {
	return o << static_cast<const ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char> &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UserLanguagesType>
    _xsd_UserLanguagesType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const EndpointType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisplayText()) {
			o << ::std::endl << "display-text: ";
			om.insert(o, *i.getDisplayText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getReferred()) {
			o << ::std::endl << "referred: ";
			om.insert(o, *i.getReferred());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getStatus()) {
			o << ::std::endl << "status: ";
			om.insert(o, *i.getStatus());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getJoiningMethod()) {
			o << ::std::endl << "joining-method: ";
			om.insert(o, *i.getJoiningMethod());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getJoiningInfo()) {
			o << ::std::endl << "joining-info: ";
			om.insert(o, *i.getJoiningInfo());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisconnectionMethod()) {
			o << ::std::endl << "disconnection-method: ";
			om.insert(o, *i.getDisconnectionMethod());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisconnectionInfo()) {
			o << ::std::endl << "disconnection-info: ";
			om.insert(o, *i.getDisconnectionInfo());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (EndpointType::MediaConstIterator b(i.getMedia().begin()), e(i.getMedia().end()); b != e; ++b) {
			o << ::std::endl << "media: ";
			om.insert(o, *b);
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getCallInfo()) {
			o << ::std::endl << "call-info: ";
			om.insert(o, *i.getCallInfo());
		}
	}

	if (i.getEntity()) {
		o << ::std::endl << "entity: " << *i.getEntity();
	}

	o << ::std::endl << "state: " << i.getState();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, EndpointType> _xsd_EndpointType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, EndpointStatusType::Value i) {
	return o << EndpointStatusType::_xsd_EndpointStatusType_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const EndpointStatusType &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, EndpointStatusType>
    _xsd_EndpointStatusType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, JoiningType::Value i) {
	return o << JoiningType::_xsd_JoiningType_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const JoiningType &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, JoiningType> _xsd_JoiningType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, DisconnectionType::Value i) {
	return o << DisconnectionType::_xsd_DisconnectionType_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const DisconnectionType &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, DisconnectionType>
    _xsd_DisconnectionType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ExecutionType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getWhen()) {
			o << ::std::endl << "when: ";
			om.insert(o, *i.getWhen());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getReason()) {
			o << ::std::endl << "reason: ";
			om.insert(o, *i.getReason());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getBy()) {
			o << ::std::endl << "by: ";
			om.insert(o, *i.getBy());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ExecutionType> _xsd_ExecutionType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CallType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSip()) {
			o << ::std::endl << "sip: ";
			om.insert(o, *i.getSip());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CallType> _xsd_CallType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SipDialogIdType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisplayText()) {
			o << ::std::endl << "display-text: ";
			om.insert(o, *i.getDisplayText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "call-id: ";
		om.insert(o, i.getCallId());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "from-tag: ";
		om.insert(o, i.getFromTag());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "to-tag: ";
		om.insert(o, i.getToTag());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SipDialogIdType> _xsd_SipDialogIdType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const MediaType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDisplayText()) {
			o << ::std::endl << "display-text: ";
			om.insert(o, *i.getDisplayText());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getType()) {
			o << ::std::endl << "type: ";
			om.insert(o, *i.getType());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getLabel()) {
			o << ::std::endl << "label: ";
			om.insert(o, *i.getLabel());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSrcId()) {
			o << ::std::endl << "src-id: ";
			om.insert(o, *i.getSrcId());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getStatus()) {
			o << ::std::endl << "status: ";
			om.insert(o, *i.getStatus());
		}
	}

	o << ::std::endl << "id: " << i.getId();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, MediaType> _xsd_MediaType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, MediaStatusType::Value i) {
	return o << MediaStatusType::_xsd_MediaStatusType_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const MediaStatusType &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, MediaStatusType> _xsd_MediaStatusType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarsByValType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (SidebarsByValType::EntryConstIterator b(i.getEntry().begin()), e(i.getEntry().end()); b != e; ++b) {
			o << ::std::endl << "entry: ";
			om.insert(o, *b);
		}
	}

	o << ::std::endl << "state: " << i.getState();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarsByValType>
    _xsd_SidebarsByValType_std_ostream_init;
} // namespace ConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

#include <istream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/sax/std-input-source.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfo {
::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(const ::std::string &u,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>(
	    ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(const ::std::string &u,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>(
	    ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(const ::std::string &u,
                    ::xercesc::DOMErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>(
	    ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::std::istream &is,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::std::istream &is,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::std::istream &is,
                    ::xercesc::DOMErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::std::istream &is,
                    const ::std::string &sid,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::std::istream &is,
                    const ::std::string &sid,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::std::istream &is,
                    const ::std::string &sid,
                    ::xercesc::DOMErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::xercesc::InputSource &i,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>(
	    ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::xercesc::InputSource &i,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>(
	    ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::xercesc::InputSource &i,
                    ::xercesc::DOMErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>(
	    ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(const ::xercesc::DOMDocument &doc,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>(
		    ::LinphonePrivate::Xsd::ConferenceInfo::parseConferenceInfo(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "conference-info", "urn:ietf:params:xml:ns:conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-info",
	                                                 "urn:ietf:params:xml:ns:conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>
parseConferenceInfo(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "conference-info", "urn:ietf:params:xml:ns:conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-info",
	                                                 "urn:ietf:params:xml:ns:conference-info");
}
} // namespace ConferenceInfo
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
namespace ConferenceInfo {
void serializeConferenceInfo(::std::ostream &o,
                             const ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType &s,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfo::serializeConferenceInfo(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConferenceInfo(::std::ostream &o,
                             const ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType &s,
                             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfo::serializeConferenceInfo(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceInfo(::std::ostream &o,
                             const ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType &s,
                             ::xercesc::DOMErrorHandler &h,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfo::serializeConferenceInfo(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceInfo(::xercesc::XMLFormatTarget &t,
                             const ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType &s,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfo::serializeConferenceInfo(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConferenceInfo(::xercesc::XMLFormatTarget &t,
                             const ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType &s,
                             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfo::serializeConferenceInfo(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceInfo(::xercesc::XMLFormatTarget &t,
                             const ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType &s,
                             ::xercesc::DOMErrorHandler &h,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::ConferenceInfo::serializeConferenceInfo(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceInfo(::xercesc::DOMDocument &d,
                             const ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType &s,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType) == typeid(s)) {
		if (n.name() == "conference-info" && n.namespace_() == "urn:ietf:params:xml:ns:conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-info",
			                                                 "urn:ietf:params:xml:ns:conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "conference-info", "urn:ietf:params:xml:ns:conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConferenceInfo(const ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("conference-info", "urn:ietf:params:xml:ns:conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "conference-info", "urn:ietf:params:xml:ns:conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::ConferenceInfo::serializeConferenceInfo(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const ConferenceType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConferenceType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// conference-description
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConferenceDescription()) {
			const ConferenceType::ConferenceDescriptionType &x(*i.getConferenceDescription());
			if (typeid(ConferenceType::ConferenceDescriptionType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "conference-description", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("conference-description", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// host-info
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getHostInfo()) {
			const ConferenceType::HostInfoType &x(*i.getHostInfo());
			if (typeid(ConferenceType::HostInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("host-info", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("host-info", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// conference-state
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConferenceState()) {
			const ConferenceType::ConferenceStateType &x(*i.getConferenceState());
			if (typeid(ConferenceType::ConferenceStateType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "conference-state", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("conference-state", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// users
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getUsers()) {
			const ConferenceType::UsersType &x(*i.getUsers());
			if (typeid(ConferenceType::UsersType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("users", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("users", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// sidebars-by-ref
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSidebarsByRef()) {
			const ConferenceType::SidebarsByRefType &x(*i.getSidebarsByRef());
			if (typeid(ConferenceType::SidebarsByRefType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "sidebars-by-ref", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("sidebars-by-ref", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// sidebars-by-val
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSidebarsByVal()) {
			const ConferenceType::SidebarsByValType &x(*i.getSidebarsByVal());
			if (typeid(ConferenceType::SidebarsByValType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "sidebars-by-val", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("sidebars-by-val", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (ConferenceType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// entity
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("entity", e));

		a << i.getEntity();
	}

	// state
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("state", e));

		a << i.getState();
	}

	// version
	//
	if (i.getVersion()) {
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("version", e));

		a << *i.getVersion();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConferenceType>
    _xsd_ConferenceType_type_serializer_init("conference-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const StateType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const StateType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const StateType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, StateType>
    _xsd_StateType_type_serializer_init("state-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const ConferenceDescriptionType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConferenceDescriptionType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// display-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisplayText()) {
			const ConferenceDescriptionType::DisplayTextType &x(*i.getDisplayText());
			if (typeid(ConferenceDescriptionType::DisplayTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("display-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("display-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// subject
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSubject()) {
			const ConferenceDescriptionType::SubjectType &x(*i.getSubject());
			if (typeid(ConferenceDescriptionType::SubjectType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("subject", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("subject", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// free-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getFreeText()) {
			const ConferenceDescriptionType::FreeTextType &x(*i.getFreeText());
			if (typeid(ConferenceDescriptionType::FreeTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("free-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("free-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// keywords
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getKeywords()) {
			const ConferenceDescriptionType::KeywordsType &x(*i.getKeywords());
			if (typeid(ConferenceDescriptionType::KeywordsType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("keywords", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("keywords", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// conf-uris
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConfUris()) {
			const ConferenceDescriptionType::ConfUrisType &x(*i.getConfUris());
			if (typeid(ConferenceDescriptionType::ConfUrisType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("conf-uris", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("conf-uris", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// service-uris
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getServiceUris()) {
			const ConferenceDescriptionType::ServiceUrisType &x(*i.getServiceUris());
			if (typeid(ConferenceDescriptionType::ServiceUrisType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("service-uris", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("service-uris", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// maximum-user-count
	//
	if (i.getMaximumUserCount()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("maximum-user-count", "urn:ietf:params:xml:ns:conference-info", e));

		s << *i.getMaximumUserCount();
	}

	// available-media
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getAvailableMedia()) {
			const ConferenceDescriptionType::AvailableMediaType &x(*i.getAvailableMedia());
			if (typeid(ConferenceDescriptionType::AvailableMediaType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "available-media", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("available-media", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (ConferenceDescriptionType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConferenceDescriptionType>
    _xsd_ConferenceDescriptionType_type_serializer_init("conference-description-type",
                                                        "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const HostType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (HostType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// display-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisplayText()) {
			const HostType::DisplayTextType &x(*i.getDisplayText());
			if (typeid(HostType::DisplayTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("display-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("display-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// web-page
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getWebPage()) {
			const HostType::WebPageType &x(*i.getWebPage());
			if (typeid(HostType::WebPageType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("web-page", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("web-page", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// uris
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getUris()) {
			const HostType::UrisType &x(*i.getUris());
			if (typeid(HostType::UrisType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("uris", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("uris", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (HostType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, HostType>
    _xsd_HostType_type_serializer_init("host-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const ConferenceStateType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConferenceStateType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// user-count
	//
	if (i.getUserCount()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("user-count", "urn:ietf:params:xml:ns:conference-info", e));

		s << *i.getUserCount();
	}

	// active
	//
	if (i.getActive()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("active", "urn:ietf:params:xml:ns:conference-info", e));

		s << *i.getActive();
	}

	// locked
	//
	if (i.getLocked()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("locked", "urn:ietf:params:xml:ns:conference-info", e));

		s << *i.getLocked();
	}

	// any
	//
	for (ConferenceStateType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConferenceStateType>
    _xsd_ConferenceStateType_type_serializer_init("conference-state-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const ConferenceMediaType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConferenceMediaType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// entry
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (ConferenceMediaType::EntryConstIterator b(i.getEntry().begin()), n(i.getEntry().end()); b != n; ++b) {
			const ConferenceMediaType::EntryType &x(*b);

			if (typeid(ConferenceMediaType::EntryType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("entry", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("entry", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConferenceMediaType>
    _xsd_ConferenceMediaType_type_serializer_init("conference-media-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const ConferenceMediumType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConferenceMediumType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// display-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisplayText()) {
			const ConferenceMediumType::DisplayTextType &x(*i.getDisplayText());
			if (typeid(ConferenceMediumType::DisplayTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("display-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("display-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// type
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const ConferenceMediumType::TypeType &x(i.getType());
		if (typeid(ConferenceMediumType::TypeType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("type", "urn:ietf:params:xml:ns:conference-info", e));

			s << x;
		} else tsm.serialize("type", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
	}

	// status
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getStatus()) {
			const ConferenceMediumType::StatusType &x(*i.getStatus());
			if (typeid(ConferenceMediumType::StatusType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("status", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("status", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (ConferenceMediumType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// label
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("label", e));

		a << i.getLabel();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConferenceMediumType>
    _xsd_ConferenceMediumType_type_serializer_init("conference-medium-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const UrisType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UrisType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// entry
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (UrisType::EntryConstIterator b(i.getEntry().begin()), n(i.getEntry().end()); b != n; ++b) {
			const UrisType::EntryType &x(*b);

			if (typeid(UrisType::EntryType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("entry", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("entry", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// state
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("state", e));

		a << i.getState();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UrisType>
    _xsd_UrisType_type_serializer_init("uris-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const UriType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UriType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// uri
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const UriType::UriType1 &x(i.getUri());
		if (typeid(UriType::UriType1) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("uri", "urn:ietf:params:xml:ns:conference-info", e));

			s << x;
		} else tsm.serialize("uri", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
	}

	// display-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisplayText()) {
			const UriType::DisplayTextType &x(*i.getDisplayText());
			if (typeid(UriType::DisplayTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("display-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("display-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// purpose
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getPurpose()) {
			const UriType::PurposeType &x(*i.getPurpose());
			if (typeid(UriType::PurposeType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("purpose", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("purpose", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// modified
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getModified()) {
			const UriType::ModifiedType &x(*i.getModified());
			if (typeid(UriType::ModifiedType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("modified", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("modified", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (UriType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UriType>
    _xsd_UriType_type_serializer_init("uri-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const KeywordsType &i) {
	e << static_cast<const ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char> &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const KeywordsType &i) {
	a << static_cast<const ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char> &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const KeywordsType &i) {
	l << static_cast<const ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::String, char> &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, KeywordsType>
    _xsd_KeywordsType_type_serializer_init("keywords-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const UsersType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UsersType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// user
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (UsersType::UserConstIterator b(i.getUser().begin()), n(i.getUser().end()); b != n; ++b) {
			const UsersType::UserType &x(*b);

			if (typeid(UsersType::UserType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("user", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("user", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (UsersType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// state
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("state", e));

		a << i.getState();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UsersType>
    _xsd_UsersType_type_serializer_init("users-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const UserType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UserType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// display-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisplayText()) {
			const UserType::DisplayTextType &x(*i.getDisplayText());
			if (typeid(UserType::DisplayTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("display-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("display-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// associated-aors
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getAssociatedAors()) {
			const UserType::AssociatedAorsType &x(*i.getAssociatedAors());
			if (typeid(UserType::AssociatedAorsType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "associated-aors", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("associated-aors", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// roles
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getRoles()) {
			const UserType::RolesType &x(*i.getRoles());
			if (typeid(UserType::RolesType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("roles", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("roles", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// languages
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getLanguages()) {
			const UserType::LanguagesType &x(*i.getLanguages());
			if (typeid(UserType::LanguagesType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("languages", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("languages", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// cascaded-focus
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getCascadedFocus()) {
			const UserType::CascadedFocusType &x(*i.getCascadedFocus());
			if (typeid(UserType::CascadedFocusType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "cascaded-focus", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("cascaded-focus", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// endpoint
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (UserType::EndpointConstIterator b(i.getEndpoint().begin()), n(i.getEndpoint().end()); b != n; ++b) {
			const UserType::EndpointType &x(*b);

			if (typeid(UserType::EndpointType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("endpoint", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("endpoint", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (UserType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// entity
	//
	if (i.getEntity()) {
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("entity", e));

		a << *i.getEntity();
	}

	// state
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("state", e));

		a << i.getState();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UserType>
    _xsd_UserType_type_serializer_init("user-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const UserRolesType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UserRolesType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// entry
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (UserRolesType::EntryConstIterator b(i.getEntry().begin()), n(i.getEntry().end()); b != n; ++b) {
			const UserRolesType::EntryType &x(*b);

			if (typeid(UserRolesType::EntryType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("entry", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("entry", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UserRolesType>
    _xsd_UserRolesType_type_serializer_init("user-roles-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const UserLanguagesType &i) {
	e << static_cast<const ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char> &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const UserLanguagesType &i) {
	a << static_cast<const ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char> &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const UserLanguagesType &i) {
	l << static_cast<const ::xsd::cxx::tree::list<::LinphonePrivate::Xsd::XmlSchema::Language, char> &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UserLanguagesType>
    _xsd_UserLanguagesType_type_serializer_init("user-languages-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const EndpointType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (EndpointType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// display-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisplayText()) {
			const EndpointType::DisplayTextType &x(*i.getDisplayText());
			if (typeid(EndpointType::DisplayTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("display-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("display-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// referred
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getReferred()) {
			const EndpointType::ReferredType &x(*i.getReferred());
			if (typeid(EndpointType::ReferredType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("referred", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("referred", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// status
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getStatus()) {
			const EndpointType::StatusType &x(*i.getStatus());
			if (typeid(EndpointType::StatusType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("status", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("status", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// joining-method
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getJoiningMethod()) {
			const EndpointType::JoiningMethodType &x(*i.getJoiningMethod());
			if (typeid(EndpointType::JoiningMethodType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "joining-method", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("joining-method", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// joining-info
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getJoiningInfo()) {
			const EndpointType::JoiningInfoType &x(*i.getJoiningInfo());
			if (typeid(EndpointType::JoiningInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("joining-info", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("joining-info", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// disconnection-method
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisconnectionMethod()) {
			const EndpointType::DisconnectionMethodType &x(*i.getDisconnectionMethod());
			if (typeid(EndpointType::DisconnectionMethodType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "disconnection-method", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("disconnection-method", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// disconnection-info
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisconnectionInfo()) {
			const EndpointType::DisconnectionInfoType &x(*i.getDisconnectionInfo());
			if (typeid(EndpointType::DisconnectionInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "disconnection-info", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("disconnection-info", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// media
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (EndpointType::MediaConstIterator b(i.getMedia().begin()), n(i.getMedia().end()); b != n; ++b) {
			const EndpointType::MediaType &x(*b);

			if (typeid(EndpointType::MediaType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("media", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("media", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// call-info
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getCallInfo()) {
			const EndpointType::CallInfoType &x(*i.getCallInfo());
			if (typeid(EndpointType::CallInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("call-info", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("call-info", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (EndpointType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// entity
	//
	if (i.getEntity()) {
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("entity", e));

		a << *i.getEntity();
	}

	// state
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("state", e));

		a << i.getState();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, EndpointType>
    _xsd_EndpointType_type_serializer_init("endpoint-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const EndpointStatusType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const EndpointStatusType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const EndpointStatusType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, EndpointStatusType>
    _xsd_EndpointStatusType_type_serializer_init("endpoint-status-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const JoiningType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const JoiningType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const JoiningType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, JoiningType>
    _xsd_JoiningType_type_serializer_init("joining-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const DisconnectionType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const DisconnectionType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const DisconnectionType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, DisconnectionType>
    _xsd_DisconnectionType_type_serializer_init("disconnection-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const ExecutionType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ExecutionType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// when
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getWhen()) {
			const ExecutionType::WhenType &x(*i.getWhen());
			if (typeid(ExecutionType::WhenType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("when", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("when", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// reason
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getReason()) {
			const ExecutionType::ReasonType &x(*i.getReason());
			if (typeid(ExecutionType::ReasonType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("reason", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("reason", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// by
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getBy()) {
			const ExecutionType::ByType &x(*i.getBy());
			if (typeid(ExecutionType::ByType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("by", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("by", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ExecutionType>
    _xsd_ExecutionType_type_serializer_init("execution-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const CallType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (CallType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// sip
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSip()) {
			const CallType::SipType &x(*i.getSip());
			if (typeid(CallType::SipType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("sip", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("sip", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (CallType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CallType>
    _xsd_CallType_type_serializer_init("call-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const SipDialogIdType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SipDialogIdType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// display-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisplayText()) {
			const SipDialogIdType::DisplayTextType &x(*i.getDisplayText());
			if (typeid(SipDialogIdType::DisplayTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("display-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("display-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// call-id
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const SipDialogIdType::CallIdType &x(i.getCallId());
		if (typeid(SipDialogIdType::CallIdType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("call-id", "urn:ietf:params:xml:ns:conference-info", e));

			s << x;
		} else tsm.serialize("call-id", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
	}

	// from-tag
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const SipDialogIdType::FromTagType &x(i.getFromTag());
		if (typeid(SipDialogIdType::FromTagType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("from-tag", "urn:ietf:params:xml:ns:conference-info", e));

			s << x;
		} else tsm.serialize("from-tag", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
	}

	// to-tag
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const SipDialogIdType::ToTagType &x(i.getToTag());
		if (typeid(SipDialogIdType::ToTagType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("to-tag", "urn:ietf:params:xml:ns:conference-info", e));

			s << x;
		} else tsm.serialize("to-tag", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
	}

	// any
	//
	for (SipDialogIdType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SipDialogIdType>
    _xsd_SipDialogIdType_type_serializer_init("sip-dialog-id-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const MediaType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (MediaType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// display-text
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDisplayText()) {
			const MediaType::DisplayTextType &x(*i.getDisplayText());
			if (typeid(MediaType::DisplayTextType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("display-text", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("display-text", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// type
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getType()) {
			const MediaType::TypeType &x(*i.getType());
			if (typeid(MediaType::TypeType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("type", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("type", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// label
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getLabel()) {
			const MediaType::LabelType &x(*i.getLabel());
			if (typeid(MediaType::LabelType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("label", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("label", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// src-id
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSrcId()) {
			const MediaType::SrcIdType &x(*i.getSrcId());
			if (typeid(MediaType::SrcIdType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("src-id", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("src-id", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// status
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getStatus()) {
			const MediaType::StatusType &x(*i.getStatus());
			if (typeid(MediaType::StatusType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("status", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("status", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (MediaType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// id
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("id", e));

		a << i.getId();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, MediaType>
    _xsd_MediaType_type_serializer_init("media-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const MediaStatusType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const MediaStatusType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const MediaStatusType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, MediaStatusType>
    _xsd_MediaStatusType_type_serializer_init("media-status-type", "urn:ietf:params:xml:ns:conference-info");

void operator<<(::xercesc::DOMElement &e, const SidebarsByValType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarsByValType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// entry
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (SidebarsByValType::EntryConstIterator b(i.getEntry().begin()), n(i.getEntry().end()); b != n; ++b) {
			const SidebarsByValType::EntryType &x(*b);

			if (typeid(SidebarsByValType::EntryType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("entry", "urn:ietf:params:xml:ns:conference-info", e));

				s << x;
			} else tsm.serialize("entry", "urn:ietf:params:xml:ns:conference-info", false, true, e, x);
		}
	}

	// state
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("state", e));

		a << i.getState();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarsByValType>
    _xsd_SidebarsByValType_type_serializer_init("sidebars-by-val-type", "urn:ietf:params:xml:ns:conference-info");
} // namespace ConferenceInfo
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
