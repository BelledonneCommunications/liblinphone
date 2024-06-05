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

#include "xcon-conference-info.h"

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
// Xpath
//

// XpathAdd
//

// Pos
//

Pos::Pos(Value v) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_Pos_literals_[v]) {
}

Pos::Pos(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

Pos::Pos(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

Pos::Pos(const ::LinphonePrivate::Xsd::XmlSchema::String &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

Pos::Pos(const Pos &v, ::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v, f, c) {
}

Pos &Pos::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::String &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_Pos_literals_[v]);

	return *this;
}

// Type
//

// Add
//

const Add::AnySequence &Add::getAny() const {
	return this->any_;
}

Add::AnySequence &Add::getAny() {
	return this->any_;
}

void Add::setAny(const AnySequence &s) {
	this->any_ = s;
}

const Add::SelType &Add::getSel() const {
	return this->sel_.get();
}

Add::SelType &Add::getSel() {
	return this->sel_.get();
}

void Add::setSel(const SelType &x) {
	this->sel_.set(x);
}

void Add::setSel(::std::unique_ptr<SelType> x) {
	this->sel_.set(std::move(x));
}

::std::unique_ptr<Add::SelType> Add::setDetachSel() {
	return this->sel_.detach();
}

const Add::PosOptional &Add::getPos() const {
	return this->pos_;
}

Add::PosOptional &Add::getPos() {
	return this->pos_;
}

void Add::setPos(const PosType &x) {
	this->pos_.set(x);
}

void Add::setPos(const PosOptional &x) {
	this->pos_ = x;
}

void Add::setPos(::std::unique_ptr<PosType> x) {
	this->pos_.set(std::move(x));
}

const Add::TypeOptional &Add::getType() const {
	return this->type_;
}

Add::TypeOptional &Add::getType() {
	return this->type_;
}

void Add::setType(const TypeType &x) {
	this->type_.set(x);
}

void Add::setType(const TypeOptional &x) {
	this->type_ = x;
}

void Add::setType(::std::unique_ptr<TypeType> x) {
	this->type_.set(std::move(x));
}

const ::xercesc::DOMDocument &Add::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Add::getDomDocument() {
	return *this->dom_document_;
}

// Replace
//

const Replace::AnyOptional &Replace::getAny() const {
	return this->any_;
}

Replace::AnyOptional &Replace::getAny() {
	return this->any_;
}

void Replace::setAny(const ::xercesc::DOMElement &e) {
	this->any_.set(e);
}

void Replace::setAny(::xercesc::DOMElement *e) {
	this->any_.set(e);
}

void Replace::setAny(const AnyOptional &x) {
	this->any_ = x;
}

const Replace::SelType &Replace::getSel() const {
	return this->sel_.get();
}

Replace::SelType &Replace::getSel() {
	return this->sel_.get();
}

void Replace::setSel(const SelType &x) {
	this->sel_.set(x);
}

void Replace::setSel(::std::unique_ptr<SelType> x) {
	this->sel_.set(std::move(x));
}

::std::unique_ptr<Replace::SelType> Replace::setDetachSel() {
	return this->sel_.detach();
}

const ::xercesc::DOMDocument &Replace::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Replace::getDomDocument() {
	return *this->dom_document_;
}

// Ws
//

Ws::Ws(Value v) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_Ws_literals_[v]) {
}

Ws::Ws(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

Ws::Ws(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

Ws::Ws(const ::LinphonePrivate::Xsd::XmlSchema::String &v) : ::LinphonePrivate::Xsd::XmlSchema::String(v) {
}

Ws::Ws(const Ws &v, ::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(v, f, c) {
}

Ws &Ws::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::String &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_Ws_literals_[v]);

	return *this;
}

// Remove
//

const Remove::SelType &Remove::getSel() const {
	return this->sel_.get();
}

Remove::SelType &Remove::getSel() {
	return this->sel_.get();
}

void Remove::setSel(const SelType &x) {
	this->sel_.set(x);
}

void Remove::setSel(::std::unique_ptr<SelType> x) {
	this->sel_.set(std::move(x));
}

::std::unique_ptr<Remove::SelType> Remove::setDetachSel() {
	return this->sel_.detach();
}

const Remove::WsOptional &Remove::getWs() const {
	return this->ws_;
}

Remove::WsOptional &Remove::getWs() {
	return this->ws_;
}

void Remove::setWs(const WsType &x) {
	this->ws_.set(x);
}

void Remove::setWs(const WsOptional &x) {
	this->ws_ = x;
}

void Remove::setWs(::std::unique_ptr<WsType> x) {
	this->ws_.set(std::move(x));
}
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
// ConferenceTimeType
//

const ConferenceTimeType::EntrySequence &ConferenceTimeType::getEntry() const {
	return this->entry_;
}

ConferenceTimeType::EntrySequence &ConferenceTimeType::getEntry() {
	return this->entry_;
}

void ConferenceTimeType::setEntry(const EntrySequence &s) {
	this->entry_ = s;
}

const ConferenceTimeType::AnySequence &ConferenceTimeType::getAny() const {
	return this->any_;
}

ConferenceTimeType::AnySequence &ConferenceTimeType::getAny() {
	return this->any_;
}

void ConferenceTimeType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConferenceTimeType::AnyAttributeSet &ConferenceTimeType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConferenceTimeType::AnyAttributeSet &ConferenceTimeType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConferenceTimeType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConferenceTimeType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConferenceTimeType::getDomDocument() {
	return *this->dom_document_;
}

// TimeType
//

// RoleType
//

// MixingModeType
//

// CodecsType
//

const CodecsType::CodecType &CodecsType::getCodec() const {
	return this->codec_.get();
}

CodecsType::CodecType &CodecsType::getCodec() {
	return this->codec_.get();
}

void CodecsType::setCodec(const CodecType &x) {
	this->codec_.set(x);
}

void CodecsType::setCodec(::std::unique_ptr<CodecType> x) {
	this->codec_.set(std::move(x));
}

::std::unique_ptr<CodecsType::CodecType> CodecsType::setDetachCodec() {
	return this->codec_.detach();
}

const CodecsType::AnySequence &CodecsType::getAny() const {
	return this->any_;
}

CodecsType::AnySequence &CodecsType::getAny() {
	return this->any_;
}

void CodecsType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const CodecsType::DecisionType &CodecsType::getDecision() const {
	return this->decision_.get();
}

CodecsType::DecisionType &CodecsType::getDecision() {
	return this->decision_.get();
}

void CodecsType::setDecision(const DecisionType &x) {
	this->decision_.set(x);
}

void CodecsType::setDecision(::std::unique_ptr<DecisionType> x) {
	this->decision_.set(std::move(x));
}

::std::unique_ptr<CodecsType::DecisionType> CodecsType::setDetachDecision() {
	return this->decision_.detach();
}

const CodecsType::AnyAttributeSet &CodecsType::getAnyAttribute() const {
	return this->any_attribute_;
}

CodecsType::AnyAttributeSet &CodecsType::getAnyAttribute() {
	return this->any_attribute_;
}

void CodecsType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &CodecsType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &CodecsType::getDomDocument() {
	return *this->dom_document_;
}

// CodecType
//

const CodecType::SubtypeOptional &CodecType::getSubtype() const {
	return this->subtype_;
}

CodecType::SubtypeOptional &CodecType::getSubtype() {
	return this->subtype_;
}

void CodecType::setSubtype(const SubtypeType &x) {
	this->subtype_.set(x);
}

void CodecType::setSubtype(const SubtypeOptional &x) {
	this->subtype_ = x;
}

void CodecType::setSubtype(::std::unique_ptr<SubtypeType> x) {
	this->subtype_.set(std::move(x));
}

const CodecType::AnySequence &CodecType::getAny() const {
	return this->any_;
}

CodecType::AnySequence &CodecType::getAny() {
	return this->any_;
}

void CodecType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const CodecType::NameType &CodecType::getName() const {
	return this->name_.get();
}

CodecType::NameType &CodecType::getName() {
	return this->name_.get();
}

void CodecType::setName(const NameType &x) {
	this->name_.set(x);
}

void CodecType::setName(::std::unique_ptr<NameType> x) {
	this->name_.set(std::move(x));
}

::std::unique_ptr<CodecType::NameType> CodecType::setDetachName() {
	return this->name_.detach();
}

const CodecType::PolicyType &CodecType::getPolicy() const {
	return this->policy_.get();
}

CodecType::PolicyType &CodecType::getPolicy() {
	return this->policy_.get();
}

void CodecType::setPolicy(const PolicyType &x) {
	this->policy_.set(x);
}

void CodecType::setPolicy(::std::unique_ptr<PolicyType> x) {
	this->policy_.set(std::move(x));
}

::std::unique_ptr<CodecType::PolicyType> CodecType::setDetachPolicy() {
	return this->policy_.detach();
}

const CodecType::AnyAttributeSet &CodecType::getAnyAttribute() const {
	return this->any_attribute_;
}

CodecType::AnyAttributeSet &CodecType::getAnyAttribute() {
	return this->any_attribute_;
}

void CodecType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &CodecType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &CodecType::getDomDocument() {
	return *this->dom_document_;
}

// DecisionType
//

// PolicyType
//

// ControlsType
//

const ControlsType::MuteOptional &ControlsType::getMute() const {
	return this->mute_;
}

ControlsType::MuteOptional &ControlsType::getMute() {
	return this->mute_;
}

void ControlsType::setMute(const MuteType &x) {
	this->mute_.set(x);
}

void ControlsType::setMute(const MuteOptional &x) {
	this->mute_ = x;
}

const ControlsType::PauseVideoOptional &ControlsType::getPauseVideo() const {
	return this->pause_video_;
}

ControlsType::PauseVideoOptional &ControlsType::getPauseVideo() {
	return this->pause_video_;
}

void ControlsType::setPauseVideo(const PauseVideoType &x) {
	this->pause_video_.set(x);
}

void ControlsType::setPauseVideo(const PauseVideoOptional &x) {
	this->pause_video_ = x;
}

const ControlsType::GainOptional &ControlsType::getGain() const {
	return this->gain_;
}

ControlsType::GainOptional &ControlsType::getGain() {
	return this->gain_;
}

void ControlsType::setGain(const GainType &x) {
	this->gain_.set(x);
}

void ControlsType::setGain(const GainOptional &x) {
	this->gain_ = x;
}

void ControlsType::setGain(::std::unique_ptr<GainType> x) {
	this->gain_.set(std::move(x));
}

const ControlsType::VideoLayoutOptional &ControlsType::getVideoLayout() const {
	return this->video_layout_;
}

ControlsType::VideoLayoutOptional &ControlsType::getVideoLayout() {
	return this->video_layout_;
}

void ControlsType::setVideoLayout(const VideoLayoutType &x) {
	this->video_layout_.set(x);
}

void ControlsType::setVideoLayout(const VideoLayoutOptional &x) {
	this->video_layout_ = x;
}

void ControlsType::setVideoLayout(::std::unique_ptr<VideoLayoutType> x) {
	this->video_layout_.set(std::move(x));
}

const ControlsType::VideoLayoutType &ControlsType::getVideoLayoutDefaultValue() {
	return video_layout_default_value_;
}

const ControlsType::AnySequence &ControlsType::getAny() const {
	return this->any_;
}

ControlsType::AnySequence &ControlsType::getAny() {
	return this->any_;
}

void ControlsType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ControlsType::AnyAttributeSet &ControlsType::getAnyAttribute() const {
	return this->any_attribute_;
}

ControlsType::AnyAttributeSet &ControlsType::getAnyAttribute() {
	return this->any_attribute_;
}

void ControlsType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ControlsType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ControlsType::getDomDocument() {
	return *this->dom_document_;
}

// GainType
//

// VideoLayoutType
//

// FloorInformationType
//

const FloorInformationType::ConferenceIDOptional &FloorInformationType::getConferenceID() const {
	return this->conference_ID_;
}

FloorInformationType::ConferenceIDOptional &FloorInformationType::getConferenceID() {
	return this->conference_ID_;
}

void FloorInformationType::setConferenceID(const ConferenceIDType &x) {
	this->conference_ID_.set(x);
}

void FloorInformationType::setConferenceID(const ConferenceIDOptional &x) {
	this->conference_ID_ = x;
}

const FloorInformationType::AllowFloorEventsOptional &FloorInformationType::getAllowFloorEvents() const {
	return this->allow_floor_events_;
}

FloorInformationType::AllowFloorEventsOptional &FloorInformationType::getAllowFloorEvents() {
	return this->allow_floor_events_;
}

void FloorInformationType::setAllowFloorEvents(const AllowFloorEventsType &x) {
	this->allow_floor_events_.set(x);
}

void FloorInformationType::setAllowFloorEvents(const AllowFloorEventsOptional &x) {
	this->allow_floor_events_ = x;
}

FloorInformationType::AllowFloorEventsType FloorInformationType::getAllowFloorEventsDefaultValue() {
	return AllowFloorEventsType(false);
}

const FloorInformationType::FloorRequestHandlingOptional &FloorInformationType::getFloorRequestHandling() const {
	return this->floor_request_handling_;
}

FloorInformationType::FloorRequestHandlingOptional &FloorInformationType::getFloorRequestHandling() {
	return this->floor_request_handling_;
}

void FloorInformationType::setFloorRequestHandling(const FloorRequestHandlingType &x) {
	this->floor_request_handling_.set(x);
}

void FloorInformationType::setFloorRequestHandling(const FloorRequestHandlingOptional &x) {
	this->floor_request_handling_ = x;
}

void FloorInformationType::setFloorRequestHandling(::std::unique_ptr<FloorRequestHandlingType> x) {
	this->floor_request_handling_.set(std::move(x));
}

const FloorInformationType::ConferenceFloorPolicyOptional &FloorInformationType::getConferenceFloorPolicy() const {
	return this->conference_floor_policy_;
}

FloorInformationType::ConferenceFloorPolicyOptional &FloorInformationType::getConferenceFloorPolicy() {
	return this->conference_floor_policy_;
}

void FloorInformationType::setConferenceFloorPolicy(const ConferenceFloorPolicyType &x) {
	this->conference_floor_policy_.set(x);
}

void FloorInformationType::setConferenceFloorPolicy(const ConferenceFloorPolicyOptional &x) {
	this->conference_floor_policy_ = x;
}

void FloorInformationType::setConferenceFloorPolicy(::std::unique_ptr<ConferenceFloorPolicyType> x) {
	this->conference_floor_policy_.set(std::move(x));
}

const FloorInformationType::AnySequence &FloorInformationType::getAny() const {
	return this->any_;
}

FloorInformationType::AnySequence &FloorInformationType::getAny() {
	return this->any_;
}

void FloorInformationType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const FloorInformationType::AnyAttributeSet &FloorInformationType::getAnyAttribute() const {
	return this->any_attribute_;
}

FloorInformationType::AnyAttributeSet &FloorInformationType::getAnyAttribute() {
	return this->any_attribute_;
}

void FloorInformationType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &FloorInformationType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &FloorInformationType::getDomDocument() {
	return *this->dom_document_;
}

// FloorRequestHandlingType
//

// ConferenceFloorPolicy
//

const ConferenceFloorPolicy::FloorSequence &ConferenceFloorPolicy::getFloor() const {
	return this->floor_;
}

ConferenceFloorPolicy::FloorSequence &ConferenceFloorPolicy::getFloor() {
	return this->floor_;
}

void ConferenceFloorPolicy::setFloor(const FloorSequence &s) {
	this->floor_ = s;
}

const ConferenceFloorPolicy::AnyAttributeSet &ConferenceFloorPolicy::getAnyAttribute() const {
	return this->any_attribute_;
}

ConferenceFloorPolicy::AnyAttributeSet &ConferenceFloorPolicy::getAnyAttribute() {
	return this->any_attribute_;
}

void ConferenceFloorPolicy::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConferenceFloorPolicy::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConferenceFloorPolicy::getDomDocument() {
	return *this->dom_document_;
}

// AlgorithmType
//

// UserAdmissionPolicyType
//

// JoinHandlingType
//

// DenyUsersListType
//

const DenyUsersListType::TargetSequence &DenyUsersListType::getTarget() const {
	return this->target_;
}

DenyUsersListType::TargetSequence &DenyUsersListType::getTarget() {
	return this->target_;
}

void DenyUsersListType::setTarget(const TargetSequence &s) {
	this->target_ = s;
}

const DenyUsersListType::AnySequence &DenyUsersListType::getAny() const {
	return this->any_;
}

DenyUsersListType::AnySequence &DenyUsersListType::getAny() {
	return this->any_;
}

void DenyUsersListType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const DenyUsersListType::AnyAttributeSet &DenyUsersListType::getAnyAttribute() const {
	return this->any_attribute_;
}

DenyUsersListType::AnyAttributeSet &DenyUsersListType::getAnyAttribute() {
	return this->any_attribute_;
}

void DenyUsersListType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &DenyUsersListType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &DenyUsersListType::getDomDocument() {
	return *this->dom_document_;
}

// AllowedUsersListType
//

const AllowedUsersListType::TargetSequence &AllowedUsersListType::getTarget() const {
	return this->target_;
}

AllowedUsersListType::TargetSequence &AllowedUsersListType::getTarget() {
	return this->target_;
}

void AllowedUsersListType::setTarget(const TargetSequence &s) {
	this->target_ = s;
}

const AllowedUsersListType::PersistentListOptional &AllowedUsersListType::getPersistentList() const {
	return this->persistent_list_;
}

AllowedUsersListType::PersistentListOptional &AllowedUsersListType::getPersistentList() {
	return this->persistent_list_;
}

void AllowedUsersListType::setPersistentList(const PersistentListType &x) {
	this->persistent_list_.set(x);
}

void AllowedUsersListType::setPersistentList(const PersistentListOptional &x) {
	this->persistent_list_ = x;
}

void AllowedUsersListType::setPersistentList(::std::unique_ptr<PersistentListType> x) {
	this->persistent_list_.set(std::move(x));
}

const AllowedUsersListType::AnySequence &AllowedUsersListType::getAny() const {
	return this->any_;
}

AllowedUsersListType::AnySequence &AllowedUsersListType::getAny() {
	return this->any_;
}

void AllowedUsersListType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const AllowedUsersListType::AnyAttributeSet &AllowedUsersListType::getAnyAttribute() const {
	return this->any_attribute_;
}

AllowedUsersListType::AnyAttributeSet &AllowedUsersListType::getAnyAttribute() {
	return this->any_attribute_;
}

void AllowedUsersListType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &AllowedUsersListType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &AllowedUsersListType::getDomDocument() {
	return *this->dom_document_;
}

// PersistentListType
//

const PersistentListType::UserSequence &PersistentListType::getUser() const {
	return this->user_;
}

PersistentListType::UserSequence &PersistentListType::getUser() {
	return this->user_;
}

void PersistentListType::setUser(const UserSequence &s) {
	this->user_ = s;
}

const PersistentListType::AnySequence &PersistentListType::getAny() const {
	return this->any_;
}

PersistentListType::AnySequence &PersistentListType::getAny() {
	return this->any_;
}

void PersistentListType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const PersistentListType::AnyAttributeSet &PersistentListType::getAnyAttribute() const {
	return this->any_attribute_;
}

PersistentListType::AnyAttributeSet &PersistentListType::getAnyAttribute() {
	return this->any_attribute_;
}

void PersistentListType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &PersistentListType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &PersistentListType::getDomDocument() {
	return *this->dom_document_;
}

// TargetType
//

const TargetType::UriType &TargetType::getUri() const {
	return this->uri_.get();
}

TargetType::UriType &TargetType::getUri() {
	return this->uri_.get();
}

void TargetType::setUri(const UriType &x) {
	this->uri_.set(x);
}

void TargetType::setUri(::std::unique_ptr<UriType> x) {
	this->uri_.set(std::move(x));
}

::std::unique_ptr<TargetType::UriType> TargetType::setDetachUri() {
	return this->uri_.detach();
}

const TargetType::MethodType &TargetType::getMethod() const {
	return this->method_.get();
}

TargetType::MethodType &TargetType::getMethod() {
	return this->method_.get();
}

void TargetType::setMethod(const MethodType &x) {
	this->method_.set(x);
}

void TargetType::setMethod(::std::unique_ptr<MethodType> x) {
	this->method_.set(std::move(x));
}

::std::unique_ptr<TargetType::MethodType> TargetType::setDetachMethod() {
	return this->method_.detach();
}

const TargetType::AnyAttributeSet &TargetType::getAnyAttribute() const {
	return this->any_attribute_;
}

TargetType::AnyAttributeSet &TargetType::getAnyAttribute() {
	return this->any_attribute_;
}

void TargetType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &TargetType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &TargetType::getDomDocument() {
	return *this->dom_document_;
}

// MethodType
//

// ProvideAnonymityType
//

// MixerType
//

const MixerType::FloorType &MixerType::getFloor() const {
	return this->floor_.get();
}

MixerType::FloorType &MixerType::getFloor() {
	return this->floor_.get();
}

void MixerType::setFloor(const FloorType &x) {
	this->floor_.set(x);
}

void MixerType::setFloor(::std::unique_ptr<FloorType> x) {
	this->floor_.set(std::move(x));
}

::std::unique_ptr<MixerType::FloorType> MixerType::setDetachFloor() {
	return this->floor_.detach();
}

const MixerType::ControlsSequence &MixerType::getControls() const {
	return this->controls_;
}

MixerType::ControlsSequence &MixerType::getControls() {
	return this->controls_;
}

void MixerType::setControls(const ControlsSequence &s) {
	this->controls_ = s;
}

const MixerType::AnySequence &MixerType::getAny() const {
	return this->any_;
}

MixerType::AnySequence &MixerType::getAny() {
	return this->any_;
}

void MixerType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const MixerType::NameType &MixerType::getName() const {
	return this->name_.get();
}

MixerType::NameType &MixerType::getName() {
	return this->name_.get();
}

void MixerType::setName(const NameType &x) {
	this->name_.set(x);
}

void MixerType::setName(::std::unique_ptr<NameType> x) {
	this->name_.set(std::move(x));
}

::std::unique_ptr<MixerType::NameType> MixerType::setDetachName() {
	return this->name_.detach();
}

const MixerType::AnyAttributeSet &MixerType::getAnyAttribute() const {
	return this->any_attribute_;
}

MixerType::AnyAttributeSet &MixerType::getAnyAttribute() {
	return this->any_attribute_;
}

void MixerType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &MixerType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &MixerType::getDomDocument() {
	return *this->dom_document_;
}

// MixerNameType
//

// ConferenceInfoDiff
//

const ConferenceInfoDiff::AddSequence &ConferenceInfoDiff::getAdd() const {
	return this->add_;
}

ConferenceInfoDiff::AddSequence &ConferenceInfoDiff::getAdd() {
	return this->add_;
}

void ConferenceInfoDiff::setAdd(const AddSequence &s) {
	this->add_ = s;
}

const ConferenceInfoDiff::RemoveSequence &ConferenceInfoDiff::getRemove() const {
	return this->remove_;
}

ConferenceInfoDiff::RemoveSequence &ConferenceInfoDiff::getRemove() {
	return this->remove_;
}

void ConferenceInfoDiff::setRemove(const RemoveSequence &s) {
	this->remove_ = s;
}

const ConferenceInfoDiff::ReplaceSequence &ConferenceInfoDiff::getReplace() const {
	return this->replace_;
}

ConferenceInfoDiff::ReplaceSequence &ConferenceInfoDiff::getReplace() {
	return this->replace_;
}

void ConferenceInfoDiff::setReplace(const ReplaceSequence &s) {
	this->replace_ = s;
}

const ConferenceInfoDiff::AnySequence &ConferenceInfoDiff::getAny() const {
	return this->any_;
}

ConferenceInfoDiff::AnySequence &ConferenceInfoDiff::getAny() {
	return this->any_;
}

void ConferenceInfoDiff::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConferenceInfoDiff::EntityType &ConferenceInfoDiff::getEntity() const {
	return this->entity_.get();
}

ConferenceInfoDiff::EntityType &ConferenceInfoDiff::getEntity() {
	return this->entity_.get();
}

void ConferenceInfoDiff::setEntity(const EntityType &x) {
	this->entity_.set(x);
}

void ConferenceInfoDiff::setEntity(::std::unique_ptr<EntityType> x) {
	this->entity_.set(std::move(x));
}

::std::unique_ptr<ConferenceInfoDiff::EntityType> ConferenceInfoDiff::setDetachEntity() {
	return this->entity_.detach();
}

const ConferenceInfoDiff::AnyAttributeSet &ConferenceInfoDiff::getAnyAttribute() const {
	return this->any_attribute_;
}

ConferenceInfoDiff::AnyAttributeSet &ConferenceInfoDiff::getAnyAttribute() {
	return this->any_attribute_;
}

void ConferenceInfoDiff::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConferenceInfoDiff::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConferenceInfoDiff::getDomDocument() {
	return *this->dom_document_;
}

// Entry
//

const Entry::BaseType &Entry::getBase() const {
	return this->base_.get();
}

Entry::BaseType &Entry::getBase() {
	return this->base_.get();
}

void Entry::setBase(const BaseType &x) {
	this->base_.set(x);
}

void Entry::setBase(::std::unique_ptr<BaseType> x) {
	this->base_.set(std::move(x));
}

::std::unique_ptr<Entry::BaseType> Entry::setDetachBase() {
	return this->base_.detach();
}

const Entry::MixingStartOffsetOptional &Entry::getMixingStartOffset() const {
	return this->mixing_start_offset_;
}

Entry::MixingStartOffsetOptional &Entry::getMixingStartOffset() {
	return this->mixing_start_offset_;
}

void Entry::setMixingStartOffset(const MixingStartOffsetType &x) {
	this->mixing_start_offset_.set(x);
}

void Entry::setMixingStartOffset(const MixingStartOffsetOptional &x) {
	this->mixing_start_offset_ = x;
}

void Entry::setMixingStartOffset(::std::unique_ptr<MixingStartOffsetType> x) {
	this->mixing_start_offset_.set(std::move(x));
}

const Entry::MixingEndOffsetOptional &Entry::getMixingEndOffset() const {
	return this->mixing_end_offset_;
}

Entry::MixingEndOffsetOptional &Entry::getMixingEndOffset() {
	return this->mixing_end_offset_;
}

void Entry::setMixingEndOffset(const MixingEndOffsetType &x) {
	this->mixing_end_offset_.set(x);
}

void Entry::setMixingEndOffset(const MixingEndOffsetOptional &x) {
	this->mixing_end_offset_ = x;
}

void Entry::setMixingEndOffset(::std::unique_ptr<MixingEndOffsetType> x) {
	this->mixing_end_offset_.set(std::move(x));
}

const Entry::CanJoinAfterOffsetOptional &Entry::getCan_join_after_offset() const {
	return this->can_join_after_offset_;
}

Entry::CanJoinAfterOffsetOptional &Entry::getCan_join_after_offset() {
	return this->can_join_after_offset_;
}

void Entry::setCan_join_after_offset(const CanJoinAfterOffsetType &x) {
	this->can_join_after_offset_.set(x);
}

void Entry::setCan_join_after_offset(const CanJoinAfterOffsetOptional &x) {
	this->can_join_after_offset_ = x;
}

void Entry::setCan_join_after_offset(::std::unique_ptr<CanJoinAfterOffsetType> x) {
	this->can_join_after_offset_.set(std::move(x));
}

const Entry::MustJoinBeforeOffsetOptional &Entry::getMust_join_before_offset() const {
	return this->must_join_before_offset_;
}

Entry::MustJoinBeforeOffsetOptional &Entry::getMust_join_before_offset() {
	return this->must_join_before_offset_;
}

void Entry::setMust_join_before_offset(const MustJoinBeforeOffsetType &x) {
	this->must_join_before_offset_.set(x);
}

void Entry::setMust_join_before_offset(const MustJoinBeforeOffsetOptional &x) {
	this->must_join_before_offset_ = x;
}

void Entry::setMust_join_before_offset(::std::unique_ptr<MustJoinBeforeOffsetType> x) {
	this->must_join_before_offset_.set(std::move(x));
}

const Entry::RequestUserOptional &Entry::getRequestUser() const {
	return this->request_user_;
}

Entry::RequestUserOptional &Entry::getRequestUser() {
	return this->request_user_;
}

void Entry::setRequestUser(const RequestUserType &x) {
	this->request_user_.set(x);
}

void Entry::setRequestUser(const RequestUserOptional &x) {
	this->request_user_ = x;
}

void Entry::setRequestUser(::std::unique_ptr<RequestUserType> x) {
	this->request_user_.set(std::move(x));
}

const Entry::NotifyEndOfConferenceOptional &Entry::getNotify_end_of_conference() const {
	return this->notify_end_of_conference_;
}

Entry::NotifyEndOfConferenceOptional &Entry::getNotify_end_of_conference() {
	return this->notify_end_of_conference_;
}

void Entry::setNotify_end_of_conference(const NotifyEndOfConferenceType &x) {
	this->notify_end_of_conference_.set(x);
}

void Entry::setNotify_end_of_conference(const NotifyEndOfConferenceOptional &x) {
	this->notify_end_of_conference_ = x;
}

const Entry::Allowed_extend_mixing_end_offsetOptional &Entry::getAllowed_extend_mixing_end_offset() const {
	return this->allowed_extend_mixing_end_offset_;
}

Entry::Allowed_extend_mixing_end_offsetOptional &Entry::getAllowed_extend_mixing_end_offset() {
	return this->allowed_extend_mixing_end_offset_;
}

void Entry::setAllowed_extend_mixing_end_offset(const Allowed_extend_mixing_end_offsetType &x) {
	this->allowed_extend_mixing_end_offset_.set(x);
}

void Entry::setAllowed_extend_mixing_end_offset(const Allowed_extend_mixing_end_offsetOptional &x) {
	this->allowed_extend_mixing_end_offset_ = x;
}

const Entry::AnySequence &Entry::getAny() const {
	return this->any_;
}

Entry::AnySequence &Entry::getAny() {
	return this->any_;
}

void Entry::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &Entry::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Entry::getDomDocument() {
	return *this->dom_document_;
}

// Floor
//

const Floor::MediaLabelSequence &Floor::getMediaLabel() const {
	return this->media_label_;
}

Floor::MediaLabelSequence &Floor::getMediaLabel() {
	return this->media_label_;
}

void Floor::setMediaLabel(const MediaLabelSequence &s) {
	this->media_label_ = s;
}

const Floor::AlgorithmOptional &Floor::getAlgorithm() const {
	return this->algorithm_;
}

Floor::AlgorithmOptional &Floor::getAlgorithm() {
	return this->algorithm_;
}

void Floor::setAlgorithm(const AlgorithmType &x) {
	this->algorithm_.set(x);
}

void Floor::setAlgorithm(const AlgorithmOptional &x) {
	this->algorithm_ = x;
}

void Floor::setAlgorithm(::std::unique_ptr<AlgorithmType> x) {
	this->algorithm_.set(std::move(x));
}

const Floor::MaxFloorUsersOptional &Floor::getMaxFloorUsers() const {
	return this->max_floor_users_;
}

Floor::MaxFloorUsersOptional &Floor::getMaxFloorUsers() {
	return this->max_floor_users_;
}

void Floor::setMaxFloorUsers(const MaxFloorUsersType &x) {
	this->max_floor_users_.set(x);
}

void Floor::setMaxFloorUsers(const MaxFloorUsersOptional &x) {
	this->max_floor_users_ = x;
}

const Floor::ModeratorIdOptional &Floor::getModeratorId() const {
	return this->moderator_id_;
}

Floor::ModeratorIdOptional &Floor::getModeratorId() {
	return this->moderator_id_;
}

void Floor::setModeratorId(const ModeratorIdType &x) {
	this->moderator_id_.set(x);
}

void Floor::setModeratorId(const ModeratorIdOptional &x) {
	this->moderator_id_ = x;
}

const Floor::AnySequence &Floor::getAny() const {
	return this->any_;
}

Floor::AnySequence &Floor::getAny() {
	return this->any_;
}

void Floor::setAny(const AnySequence &s) {
	this->any_ = s;
}

const Floor::IdType &Floor::getId() const {
	return this->id_.get();
}

Floor::IdType &Floor::getId() {
	return this->id_.get();
}

void Floor::setId(const IdType &x) {
	this->id_.set(x);
}

void Floor::setId(::std::unique_ptr<IdType> x) {
	this->id_.set(std::move(x));
}

::std::unique_ptr<Floor::IdType> Floor::setDetachId() {
	return this->id_.detach();
}

const Floor::AnyAttributeSet &Floor::getAnyAttribute() const {
	return this->any_attribute_;
}

Floor::AnyAttributeSet &Floor::getAnyAttribute() {
	return this->any_attribute_;
}

void Floor::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &Floor::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Floor::getDomDocument() {
	return *this->dom_document_;
}

// Target
//

const Target::UriType &Target::getUri() const {
	return this->uri_.get();
}

Target::UriType &Target::getUri() {
	return this->uri_.get();
}

void Target::setUri(const UriType &x) {
	this->uri_.set(x);
}

void Target::setUri(::std::unique_ptr<UriType> x) {
	this->uri_.set(std::move(x));
}

::std::unique_ptr<Target::UriType> Target::setDetachUri() {
	return this->uri_.detach();
}

const Target::AnyAttributeSet &Target::getAnyAttribute() const {
	return this->any_attribute_;
}

Target::AnyAttributeSet &Target::getAnyAttribute() {
	return this->any_attribute_;
}

void Target::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &Target::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Target::getDomDocument() {
	return *this->dom_document_;
}

// User
//

const User::EmailSequence &User::getEmail() const {
	return this->email_;
}

User::EmailSequence &User::getEmail() {
	return this->email_;
}

void User::setEmail(const EmailSequence &s) {
	this->email_ = s;
}

const User::AnySequence &User::getAny() const {
	return this->any_;
}

User::AnySequence &User::getAny() {
	return this->any_;
}

void User::setAny(const AnySequence &s) {
	this->any_ = s;
}

const User::NameType &User::getName() const {
	return this->name_.get();
}

User::NameType &User::getName() {
	return this->name_.get();
}

void User::setName(const NameType &x) {
	this->name_.set(x);
}

void User::setName(::std::unique_ptr<NameType> x) {
	this->name_.set(std::move(x));
}

::std::unique_ptr<User::NameType> User::setDetachName() {
	return this->name_.detach();
}

const User::NicknameType &User::getNickname() const {
	return this->nickname_.get();
}

User::NicknameType &User::getNickname() {
	return this->nickname_.get();
}

void User::setNickname(const NicknameType &x) {
	this->nickname_.set(x);
}

void User::setNickname(::std::unique_ptr<NicknameType> x) {
	this->nickname_.set(std::move(x));
}

::std::unique_ptr<User::NicknameType> User::setDetachNickname() {
	return this->nickname_.detach();
}

const User::IdType &User::getId() const {
	return this->id_.get();
}

User::IdType &User::getId() {
	return this->id_.get();
}

void User::setId(const IdType &x) {
	this->id_.set(x);
}

void User::setId(::std::unique_ptr<IdType> x) {
	this->id_.set(std::move(x));
}

::std::unique_ptr<User::IdType> User::setDetachId() {
	return this->id_.detach();
}

const User::AnyAttributeSet &User::getAnyAttribute() const {
	return this->any_attribute_;
}

User::AnyAttributeSet &User::getAnyAttribute() {
	return this->any_attribute_;
}

void User::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &User::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &User::getDomDocument() {
	return *this->dom_document_;
}

// Floor1
//

const Floor1::IdType &Floor1::getId() const {
	return this->id_.get();
}

Floor1::IdType &Floor1::getId() {
	return this->id_.get();
}

void Floor1::setId(const IdType &x) {
	this->id_.set(x);
}

void Floor1::setId(::std::unique_ptr<IdType> x) {
	this->id_.set(std::move(x));
}

::std::unique_ptr<Floor1::IdType> Floor1::setDetachId() {
	return this->id_.detach();
}

const Floor1::AnyAttributeSet &Floor1::getAnyAttribute() const {
	return this->any_attribute_;
}

Floor1::AnyAttributeSet &Floor1::getAnyAttribute() {
	return this->any_attribute_;
}

void Floor1::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &Floor1::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Floor1::getDomDocument() {
	return *this->dom_document_;
}

// Add1
//

const Add1::AnyAttributeSet &Add1::getAnyAttribute() const {
	return this->any_attribute_;
}

Add1::AnyAttributeSet &Add1::getAnyAttribute() {
	return this->any_attribute_;
}

void Add1::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

// Remove1
//

const Remove1::AnyAttributeSet &Remove1::getAnyAttribute() const {
	return this->any_attribute_;
}

Remove1::AnyAttributeSet &Remove1::getAnyAttribute() {
	return this->any_attribute_;
}

void Remove1::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &Remove1::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &Remove1::getDomDocument() {
	return *this->dom_document_;
}

// Replace1
//

const Replace1::AnyAttributeSet &Replace1::getAnyAttribute() const {
	return this->any_attribute_;
}

Replace1::AnyAttributeSet &Replace1::getAnyAttribute() {
	return this->any_attribute_;
}

void Replace1::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

// MixingStartOffset
//

const MixingStartOffset::RequiredParticipantType &MixingStartOffset::getRequiredParticipant() const {
	return this->required_participant_.get();
}

MixingStartOffset::RequiredParticipantType &MixingStartOffset::getRequiredParticipant() {
	return this->required_participant_.get();
}

void MixingStartOffset::setRequiredParticipant(const RequiredParticipantType &x) {
	this->required_participant_.set(x);
}

void MixingStartOffset::setRequiredParticipant(::std::unique_ptr<RequiredParticipantType> x) {
	this->required_participant_.set(std::move(x));
}

::std::unique_ptr<MixingStartOffset::RequiredParticipantType> MixingStartOffset::setDetachRequired_participant() {
	return this->required_participant_.detach();
}

const MixingStartOffset::AnyAttributeSet &MixingStartOffset::getAnyAttribute() const {
	return this->any_attribute_;
}

MixingStartOffset::AnyAttributeSet &MixingStartOffset::getAnyAttribute() {
	return this->any_attribute_;
}

void MixingStartOffset::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &MixingStartOffset::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &MixingStartOffset::getDomDocument() {
	return *this->dom_document_;
}

// MixingEndOffset
//

const MixingEndOffset::RequiredParticipantType &MixingEndOffset::getRequiredParticipant() const {
	return this->required_participant_.get();
}

MixingEndOffset::RequiredParticipantType &MixingEndOffset::getRequiredParticipant() {
	return this->required_participant_.get();
}

void MixingEndOffset::setRequiredParticipant(const RequiredParticipantType &x) {
	this->required_participant_.set(x);
}

void MixingEndOffset::setRequiredParticipant(::std::unique_ptr<RequiredParticipantType> x) {
	this->required_participant_.set(std::move(x));
}

::std::unique_ptr<MixingEndOffset::RequiredParticipantType> MixingEndOffset::setDetachRequired_participant() {
	return this->required_participant_.detach();
}

const MixingEndOffset::AnyAttributeSet &MixingEndOffset::getAnyAttribute() const {
	return this->any_attribute_;
}

MixingEndOffset::AnyAttributeSet &MixingEndOffset::getAnyAttribute() {
	return this->any_attribute_;
}

void MixingEndOffset::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &MixingEndOffset::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &MixingEndOffset::getDomDocument() {
	return *this->dom_document_;
}
} // namespace XconConferenceInfo
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
namespace XconConferenceInfo {
// Xpath
//

Xpath::Xpath() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

Xpath::Xpath(const char *_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

Xpath::Xpath(const ::std::string &_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

Xpath::Xpath(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

Xpath::Xpath(const Xpath &x,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

Xpath::Xpath(const ::xercesc::DOMElement &e,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

Xpath::Xpath(const ::xercesc::DOMAttr &a,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

Xpath::Xpath(const ::std::string &s,
             const ::xercesc::DOMElement *e,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

Xpath *Xpath::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Xpath(*this, f, c);
}

Xpath::~Xpath() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Xpath>
    _xsd_Xpath_type_factory_init("xpath", "urn:ietf:params:xml:ns:xcon-conference-info");

// XpathAdd
//

XpathAdd::XpathAdd() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

XpathAdd::XpathAdd(const char *_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

XpathAdd::XpathAdd(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

XpathAdd::XpathAdd(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

XpathAdd::XpathAdd(const XpathAdd &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

XpathAdd::XpathAdd(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

XpathAdd::XpathAdd(const ::xercesc::DOMAttr &a,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

XpathAdd::XpathAdd(const ::std::string &s,
                   const ::xercesc::DOMElement *e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

XpathAdd *XpathAdd::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class XpathAdd(*this, f, c);
}

XpathAdd::~XpathAdd() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, XpathAdd>
    _xsd_XpathAdd_type_factory_init("xpath-add", "urn:ietf:params:xml:ns:xcon-conference-info");

// Pos
//

Pos::Pos(const ::xercesc::DOMElement &e,
         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
	_xsd_Pos_convert();
}

Pos::Pos(const ::xercesc::DOMAttr &a,
         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
	_xsd_Pos_convert();
}

Pos::Pos(const ::std::string &s,
         const ::xercesc::DOMElement *e,
         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
	_xsd_Pos_convert();
}

Pos *Pos::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Pos(*this, f, c);
}

Pos::Value Pos::_xsd_Pos_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_Pos_literals_);
	const Value *i(::std::lower_bound(_xsd_Pos_indexes_, _xsd_Pos_indexes_ + 3, *this, c));

	if (i == _xsd_Pos_indexes_ + 3 || _xsd_Pos_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const Pos::_xsd_Pos_literals_[3] = {"before", "after", "prepend"};

const Pos::Value Pos::_xsd_Pos_indexes_[3] = {::LinphonePrivate::Xsd::XconConferenceInfo::Pos::after,
                                              ::LinphonePrivate::Xsd::XconConferenceInfo::Pos::before,
                                              ::LinphonePrivate::Xsd::XconConferenceInfo::Pos::prepend};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Pos>
    _xsd_Pos_type_factory_init("pos", "urn:ietf:params:xml:ns:xcon-conference-info");

// Type
//

Type::Type() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

Type::Type(const char *_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

Type::Type(const ::std::string &_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

Type::Type(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

Type::Type(const Type &x, ::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

Type::Type(const ::xercesc::DOMElement &e,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

Type::Type(const ::xercesc::DOMAttr &a,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

Type::Type(const ::std::string &s,
           const ::xercesc::DOMElement *e,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

Type *Type::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Type(*this, f, c);
}

Type::~Type() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Type>
    _xsd_Type_type_factory_init("type", "urn:ietf:params:xml:ns:xcon-conference-info");

// Add
//

Add::Add(const SelType &sel)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      any_(this->getDomDocument()), sel_(sel, this), pos_(this), type_(this) {
}

Add::Add(const ::LinphonePrivate::Xsd::XmlSchema::Type &_xsd_Type_base, const SelType &sel)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(_xsd_Type_base),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), any_(this->getDomDocument()), sel_(sel, this),
      pos_(this), type_(this) {
}

Add::Add(const Add &x, ::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      any_(x.any_, this->getDomDocument()), sel_(x.sel_, f, this), pos_(x.pos_, f, this), type_(x.type_, f, this) {
}

Add::Add(const ::xercesc::DOMElement &e,
         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), any_(this->getDomDocument()), sel_(this),
      pos_(this), type_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void Add::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any
		//
		if (true) {
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

		if (n.name() == "sel" && n.namespace_().empty()) {
			this->sel_.set(SelTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "pos" && n.namespace_().empty()) {
			this->pos_.set(PosTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "type" && n.namespace_().empty()) {
			this->type_.set(TypeTraits::create(i, f, this));
			continue;
		}
	}

	if (!sel_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("sel", "");
	}
}

Add *Add::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Add(*this, f, c);
}

Add &Add::operator=(const Add &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->any_ = x.any_;
		this->sel_ = x.sel_;
		this->pos_ = x.pos_;
		this->type_ = x.type_;
	}

	return *this;
}

Add::~Add() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Add>
    _xsd_Add_type_factory_init("add", "urn:ietf:params:xml:ns:xcon-conference-info");

// Replace
//

Replace::Replace(const SelType &sel)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      any_(this->getDomDocument()), sel_(sel, this) {
}

Replace::Replace(const ::LinphonePrivate::Xsd::XmlSchema::Type &_xsd_Type_base, const SelType &sel)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(_xsd_Type_base),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), any_(this->getDomDocument()), sel_(sel, this) {
}

Replace::Replace(const Replace &x,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      any_(x.any_, this->getDomDocument()), sel_(x.sel_, f, this) {
}

Replace::Replace(const ::xercesc::DOMElement &e,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), any_(this->getDomDocument()), sel_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void Replace::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any
		//
		if (true) {
			if (!this->any_) {
				::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
				    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
				this->any_.set(r);
				continue;
			}
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "sel" && n.namespace_().empty()) {
			this->sel_.set(SelTraits::create(i, f, this));
			continue;
		}
	}

	if (!sel_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("sel", "");
	}
}

Replace *Replace::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Replace(*this, f, c);
}

Replace &Replace::operator=(const Replace &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->any_ = x.any_;
		this->sel_ = x.sel_;
	}

	return *this;
}

Replace::~Replace() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Replace>
    _xsd_Replace_type_factory_init("replace", "urn:ietf:params:xml:ns:xcon-conference-info");

// Ws
//

Ws::Ws(const ::xercesc::DOMElement &e,
       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
	_xsd_Ws_convert();
}

Ws::Ws(const ::xercesc::DOMAttr &a,
       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
	_xsd_Ws_convert();
}

Ws::Ws(const ::std::string &s,
       const ::xercesc::DOMElement *e,
       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
	_xsd_Ws_convert();
}

Ws *Ws::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Ws(*this, f, c);
}

Ws::Value Ws::_xsd_Ws_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_Ws_literals_);
	const Value *i(::std::lower_bound(_xsd_Ws_indexes_, _xsd_Ws_indexes_ + 3, *this, c));

	if (i == _xsd_Ws_indexes_ + 3 || _xsd_Ws_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const Ws::_xsd_Ws_literals_[3] = {"before", "after", "both"};

const Ws::Value Ws::_xsd_Ws_indexes_[3] = {::LinphonePrivate::Xsd::XconConferenceInfo::Ws::after,
                                           ::LinphonePrivate::Xsd::XconConferenceInfo::Ws::before,
                                           ::LinphonePrivate::Xsd::XconConferenceInfo::Ws::both};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Ws>
    _xsd_Ws_type_factory_init("ws", "urn:ietf:params:xml:ns:xcon-conference-info");

// Remove
//

Remove::Remove(const SelType &sel) : ::LinphonePrivate::Xsd::XmlSchema::Type(), sel_(sel, this), ws_(this) {
}

Remove::Remove(const Remove &x,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), sel_(x.sel_, f, this), ws_(x.ws_, f, this) {
}

Remove::Remove(const ::xercesc::DOMElement &e,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c), sel_(this),
      ws_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, false, false, true);
		this->parse(p, f);
	}
}

void Remove::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "sel" && n.namespace_().empty()) {
			this->sel_.set(SelTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "ws" && n.namespace_().empty()) {
			this->ws_.set(WsTraits::create(i, f, this));
			continue;
		}
	}

	if (!sel_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("sel", "");
	}
}

Remove *Remove::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Remove(*this, f, c);
}

Remove &Remove::operator=(const Remove &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->sel_ = x.sel_;
		this->ws_ = x.ws_;
	}

	return *this;
}

Remove::~Remove() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Remove>
    _xsd_Remove_type_factory_init("remove", "urn:ietf:params:xml:ns:xcon-conference-info");
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
// ConferenceTimeType
//

ConferenceTimeType::ConferenceTimeType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

ConferenceTimeType::ConferenceTimeType(const ConferenceTimeType &x,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      entry_(x.entry_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConferenceTimeType::ConferenceTimeType(const ::xercesc::DOMElement &e,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), entry_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConferenceTimeType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// entry
		//
		if (n.name() == "entry" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<EntryType> r(EntryTraits::create(i, f, this));

			this->entry_.push_back(::std::move(r));
			continue;
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

ConferenceTimeType *ConferenceTimeType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceTimeType(*this, f, c);
}

ConferenceTimeType &ConferenceTimeType::operator=(const ConferenceTimeType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->entry_ = x.entry_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConferenceTimeType::~ConferenceTimeType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConferenceTimeType>
    _xsd_ConferenceTimeType_type_factory_init("conference-time-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// TimeType
//

TimeType::TimeType(const ::LinphonePrivate::Xsd::XmlSchema::DateTime &_xsd_DateTime_base)
    : ::LinphonePrivate::Xsd::XmlSchema::DateTime(_xsd_DateTime_base) {
}

TimeType::TimeType(const TimeType &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::DateTime(x, f, c) {
}

TimeType::TimeType(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::DateTime(e, f, c) {
}

TimeType::TimeType(const ::xercesc::DOMAttr &a,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::DateTime(a, f, c) {
}

TimeType::TimeType(const ::std::string &s,
                   const ::xercesc::DOMElement *e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::DateTime(s, e, f, c) {
}

TimeType *TimeType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class TimeType(*this, f, c);
}

TimeType::~TimeType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, TimeType>
    _xsd_TimeType_type_factory_init("time-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// RoleType
//

RoleType::RoleType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

RoleType::RoleType(const char *_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

RoleType::RoleType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

RoleType::RoleType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

RoleType::RoleType(const RoleType &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

RoleType::RoleType(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

RoleType::RoleType(const ::xercesc::DOMAttr &a,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

RoleType::RoleType(const ::std::string &s,
                   const ::xercesc::DOMElement *e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

RoleType *RoleType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class RoleType(*this, f, c);
}

RoleType::~RoleType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, RoleType>
    _xsd_RoleType_type_factory_init("role-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// MixingModeType
//

MixingModeType::MixingModeType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

MixingModeType::MixingModeType(const char *_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MixingModeType::MixingModeType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MixingModeType::MixingModeType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MixingModeType::MixingModeType(const MixingModeType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

MixingModeType::MixingModeType(const ::xercesc::DOMElement &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

MixingModeType::MixingModeType(const ::xercesc::DOMAttr &a,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

MixingModeType::MixingModeType(const ::std::string &s,
                               const ::xercesc::DOMElement *e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

MixingModeType *MixingModeType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class MixingModeType(*this, f, c);
}

MixingModeType::~MixingModeType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, MixingModeType>
    _xsd_MixingModeType_type_factory_init("mixing-mode-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// CodecsType
//

CodecsType::CodecsType(const CodecType &codec, const DecisionType &decision)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      codec_(codec, this), any_(this->getDomDocument()), decision_(decision, this),
      any_attribute_(this->getDomDocument()) {
}

CodecsType::CodecsType(::std::unique_ptr<CodecType> codec, const DecisionType &decision)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      codec_(std::move(codec), this), any_(this->getDomDocument()), decision_(decision, this),
      any_attribute_(this->getDomDocument()) {
}

CodecsType::CodecsType(const CodecsType &x,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      codec_(x.codec_, f, this), any_(x.any_, this->getDomDocument()), decision_(x.decision_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

CodecsType::CodecsType(const ::xercesc::DOMElement &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), codec_(this), any_(this->getDomDocument()),
      decision_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CodecsType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// codec
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "codec", "urn:ietf:params:xml:ns:xcon-conference-info", &::xsd::cxx::tree::factory_impl<CodecType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!codec_.present()) {
					::std::unique_ptr<CodecType> r(dynamic_cast<CodecType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->codec_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!codec_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("codec", "urn:ietf:params:xml:ns:xcon-conference-info");
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "decision" && n.namespace_().empty()) {
			this->decision_.set(DecisionTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!decision_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("decision", "");
	}
}

CodecsType *CodecsType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CodecsType(*this, f, c);
}

CodecsType &CodecsType::operator=(const CodecsType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->codec_ = x.codec_;
		this->any_ = x.any_;
		this->decision_ = x.decision_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

CodecsType::~CodecsType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CodecsType>
    _xsd_CodecsType_type_factory_init("codecs-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// CodecType
//

CodecType::CodecType(const NameType &name, const PolicyType &policy)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      subtype_(this), any_(this->getDomDocument()), name_(name, this), policy_(policy, this),
      any_attribute_(this->getDomDocument()) {
}

CodecType::CodecType(const CodecType &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      subtype_(x.subtype_, f, this), any_(x.any_, this->getDomDocument()), name_(x.name_, f, this),
      policy_(x.policy_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

CodecType::CodecType(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), subtype_(this), any_(this->getDomDocument()),
      name_(this), policy_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CodecType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// subtype
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "subtype", "urn:ietf:params:xml:ns:xcon-conference-info", &::xsd::cxx::tree::factory_impl<SubtypeType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->subtype_) {
					::std::unique_ptr<SubtypeType> r(dynamic_cast<SubtypeType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->subtype_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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

		if (n.name() == "name" && n.namespace_().empty()) {
			this->name_.set(NameTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "policy" && n.namespace_().empty()) {
			this->policy_.set(PolicyTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!name_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("name", "");
	}

	if (!policy_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("policy", "");
	}
}

CodecType *CodecType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CodecType(*this, f, c);
}

CodecType &CodecType::operator=(const CodecType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->subtype_ = x.subtype_;
		this->any_ = x.any_;
		this->name_ = x.name_;
		this->policy_ = x.policy_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

CodecType::~CodecType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CodecType>
    _xsd_CodecType_type_factory_init("codec-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// DecisionType
//

DecisionType::DecisionType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

DecisionType::DecisionType(const char *_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

DecisionType::DecisionType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

DecisionType::DecisionType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

DecisionType::DecisionType(const DecisionType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

DecisionType::DecisionType(const ::xercesc::DOMElement &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

DecisionType::DecisionType(const ::xercesc::DOMAttr &a,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

DecisionType::DecisionType(const ::std::string &s,
                           const ::xercesc::DOMElement *e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

DecisionType *DecisionType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class DecisionType(*this, f, c);
}

DecisionType::~DecisionType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, DecisionType>
    _xsd_DecisionType_type_factory_init("decision-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// PolicyType
//

PolicyType::PolicyType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

PolicyType::PolicyType(const char *_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

PolicyType::PolicyType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

PolicyType::PolicyType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

PolicyType::PolicyType(const PolicyType &x,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

PolicyType::PolicyType(const ::xercesc::DOMElement &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

PolicyType::PolicyType(const ::xercesc::DOMAttr &a,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

PolicyType::PolicyType(const ::std::string &s,
                       const ::xercesc::DOMElement *e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

PolicyType *PolicyType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class PolicyType(*this, f, c);
}

PolicyType::~PolicyType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, PolicyType>
    _xsd_PolicyType_type_factory_init("policy-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// ControlsType
//

const ControlsType::VideoLayoutType ControlsType::video_layout_default_value_("single-view");

ControlsType::ControlsType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      mute_(this), pause_video_(this), gain_(this), video_layout_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
}

ControlsType::ControlsType(const ControlsType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      mute_(x.mute_, f, this), pause_video_(x.pause_video_, f, this), gain_(x.gain_, f, this),
      video_layout_(x.video_layout_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ControlsType::ControlsType(const ::xercesc::DOMElement &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), mute_(this), pause_video_(this), gain_(this),
      video_layout_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ControlsType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// mute
		//
		if (n.name() == "mute" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			if (!this->mute_) {
				this->mute_.set(MuteTraits::create(i, f, this));
				continue;
			}
		}

		// pause-video
		//
		if (n.name() == "pause-video" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			if (!this->pause_video_) {
				this->pause_video_.set(PauseVideoTraits::create(i, f, this));
				continue;
			}
		}

		// gain
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "gain", "urn:ietf:params:xml:ns:xcon-conference-info", &::xsd::cxx::tree::factory_impl<GainType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->gain_) {
					::std::unique_ptr<GainType> r(dynamic_cast<GainType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->gain_.set(::std::move(r));
					continue;
				}
			}
		}

		// video-layout
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "video-layout", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<VideoLayoutType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->video_layout_) {
					::std::unique_ptr<VideoLayoutType> r(dynamic_cast<VideoLayoutType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->video_layout_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

ControlsType *ControlsType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ControlsType(*this, f, c);
}

ControlsType &ControlsType::operator=(const ControlsType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->mute_ = x.mute_;
		this->pause_video_ = x.pause_video_;
		this->gain_ = x.gain_;
		this->video_layout_ = x.video_layout_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ControlsType::~ControlsType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ControlsType>
    _xsd_ControlsType_type_factory_init("controls-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// GainType
//

GainType::GainType(const ::LinphonePrivate::Xsd::XmlSchema::Integer &_xsd_Integer_base)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(_xsd_Integer_base) {
}

GainType::GainType(const GainType &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(x, f, c) {
}

GainType::GainType(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(e, f, c) {
}

GainType::GainType(const ::xercesc::DOMAttr &a,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(a, f, c) {
}

GainType::GainType(const ::std::string &s,
                   const ::xercesc::DOMElement *e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(s, e, f, c) {
}

GainType *GainType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class GainType(*this, f, c);
}

GainType::~GainType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, GainType>
    _xsd_GainType_type_factory_init("gain-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// VideoLayoutType
//

VideoLayoutType::VideoLayoutType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

VideoLayoutType::VideoLayoutType(const char *_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

VideoLayoutType::VideoLayoutType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

VideoLayoutType::VideoLayoutType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

VideoLayoutType::VideoLayoutType(const VideoLayoutType &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

VideoLayoutType::VideoLayoutType(const ::xercesc::DOMElement &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

VideoLayoutType::VideoLayoutType(const ::xercesc::DOMAttr &a,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

VideoLayoutType::VideoLayoutType(const ::std::string &s,
                                 const ::xercesc::DOMElement *e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

VideoLayoutType *VideoLayoutType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class VideoLayoutType(*this, f, c);
}

VideoLayoutType::~VideoLayoutType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, VideoLayoutType>
    _xsd_VideoLayoutType_type_factory_init("video-layout-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// FloorInformationType
//

FloorInformationType::FloorInformationType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      conference_ID_(this), allow_floor_events_(this), floor_request_handling_(this), conference_floor_policy_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

FloorInformationType::FloorInformationType(const FloorInformationType &x,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      conference_ID_(x.conference_ID_, f, this), allow_floor_events_(x.allow_floor_events_, f, this),
      floor_request_handling_(x.floor_request_handling_, f, this),
      conference_floor_policy_(x.conference_floor_policy_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

FloorInformationType::FloorInformationType(const ::xercesc::DOMElement &e,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), conference_ID_(this), allow_floor_events_(this),
      floor_request_handling_(this), conference_floor_policy_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void FloorInformationType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// conference-ID
		//
		if (n.name() == "conference-ID" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			if (!this->conference_ID_) {
				this->conference_ID_.set(ConferenceIDTraits::create(i, f, this));
				continue;
			}
		}

		// allow-floor-events
		//
		if (n.name() == "allow-floor-events" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			if (!this->allow_floor_events_) {
				this->allow_floor_events_.set(AllowFloorEventsTraits::create(i, f, this));
				continue;
			}
		}

		// floor-request-handling
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "floor-request-handling", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<FloorRequestHandlingType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->floor_request_handling_) {
					::std::unique_ptr<FloorRequestHandlingType> r(dynamic_cast<FloorRequestHandlingType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->floor_request_handling_.set(::std::move(r));
					continue;
				}
			}
		}

		// conference-floor-policy
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "conference-floor-policy", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<ConferenceFloorPolicyType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->conference_floor_policy_) {
					::std::unique_ptr<ConferenceFloorPolicyType> r(
					    dynamic_cast<ConferenceFloorPolicyType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->conference_floor_policy_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

FloorInformationType *FloorInformationType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class FloorInformationType(*this, f, c);
}

FloorInformationType &FloorInformationType::operator=(const FloorInformationType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->conference_ID_ = x.conference_ID_;
		this->allow_floor_events_ = x.allow_floor_events_;
		this->floor_request_handling_ = x.floor_request_handling_;
		this->conference_floor_policy_ = x.conference_floor_policy_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

FloorInformationType::~FloorInformationType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, FloorInformationType>
    _xsd_FloorInformationType_type_factory_init("floor-information-type",
                                                "urn:ietf:params:xml:ns:xcon-conference-info");

// FloorRequestHandlingType
//

FloorRequestHandlingType::FloorRequestHandlingType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

FloorRequestHandlingType::FloorRequestHandlingType(const char *_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

FloorRequestHandlingType::FloorRequestHandlingType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

FloorRequestHandlingType::FloorRequestHandlingType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

FloorRequestHandlingType::FloorRequestHandlingType(const FloorRequestHandlingType &x,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

FloorRequestHandlingType::FloorRequestHandlingType(const ::xercesc::DOMElement &e,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

FloorRequestHandlingType::FloorRequestHandlingType(const ::xercesc::DOMAttr &a,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

FloorRequestHandlingType::FloorRequestHandlingType(const ::std::string &s,
                                                   const ::xercesc::DOMElement *e,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

FloorRequestHandlingType *FloorRequestHandlingType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class FloorRequestHandlingType(*this, f, c);
}

FloorRequestHandlingType::~FloorRequestHandlingType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, FloorRequestHandlingType>
    _xsd_FloorRequestHandlingType_type_factory_init("floor-request-handling-type",
                                                    "urn:ietf:params:xml:ns:xcon-conference-info");

// ConferenceFloorPolicy
//

ConferenceFloorPolicy::ConferenceFloorPolicy()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      floor_(this), any_attribute_(this->getDomDocument()) {
}

ConferenceFloorPolicy::ConferenceFloorPolicy(const ConferenceFloorPolicy &x,
                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      floor_(x.floor_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConferenceFloorPolicy::ConferenceFloorPolicy(const ::xercesc::DOMElement &e,
                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), floor_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConferenceFloorPolicy::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// floor
		//
		if (n.name() == "floor" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<FloorType> r(FloorTraits::create(i, f, this));

			this->floor_.push_back(::std::move(r));
			continue;
		}

		break;
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

ConferenceFloorPolicy *ConferenceFloorPolicy::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceFloorPolicy(*this, f, c);
}

ConferenceFloorPolicy &ConferenceFloorPolicy::operator=(const ConferenceFloorPolicy &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->floor_ = x.floor_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConferenceFloorPolicy::~ConferenceFloorPolicy() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConferenceFloorPolicy>
    _xsd_ConferenceFloorPolicy_type_factory_init("conference-floor-policy",
                                                 "urn:ietf:params:xml:ns:xcon-conference-info");

// AlgorithmType
//

AlgorithmType::AlgorithmType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

AlgorithmType::AlgorithmType(const char *_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

AlgorithmType::AlgorithmType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

AlgorithmType::AlgorithmType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

AlgorithmType::AlgorithmType(const AlgorithmType &x,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

AlgorithmType::AlgorithmType(const ::xercesc::DOMElement &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

AlgorithmType::AlgorithmType(const ::xercesc::DOMAttr &a,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

AlgorithmType::AlgorithmType(const ::std::string &s,
                             const ::xercesc::DOMElement *e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

AlgorithmType *AlgorithmType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class AlgorithmType(*this, f, c);
}

AlgorithmType::~AlgorithmType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, AlgorithmType>
    _xsd_AlgorithmType_type_factory_init("algorithm-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// UserAdmissionPolicyType
//

UserAdmissionPolicyType::UserAdmissionPolicyType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

UserAdmissionPolicyType::UserAdmissionPolicyType(const char *_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

UserAdmissionPolicyType::UserAdmissionPolicyType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

UserAdmissionPolicyType::UserAdmissionPolicyType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

UserAdmissionPolicyType::UserAdmissionPolicyType(const UserAdmissionPolicyType &x,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

UserAdmissionPolicyType::UserAdmissionPolicyType(const ::xercesc::DOMElement &e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

UserAdmissionPolicyType::UserAdmissionPolicyType(const ::xercesc::DOMAttr &a,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

UserAdmissionPolicyType::UserAdmissionPolicyType(const ::std::string &s,
                                                 const ::xercesc::DOMElement *e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

UserAdmissionPolicyType *UserAdmissionPolicyType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UserAdmissionPolicyType(*this, f, c);
}

UserAdmissionPolicyType::~UserAdmissionPolicyType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UserAdmissionPolicyType>
    _xsd_UserAdmissionPolicyType_type_factory_init("user-admission-policy-type",
                                                   "urn:ietf:params:xml:ns:xcon-conference-info");

// JoinHandlingType
//

JoinHandlingType::JoinHandlingType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

JoinHandlingType::JoinHandlingType(const char *_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

JoinHandlingType::JoinHandlingType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

JoinHandlingType::JoinHandlingType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

JoinHandlingType::JoinHandlingType(const JoinHandlingType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

JoinHandlingType::JoinHandlingType(const ::xercesc::DOMElement &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

JoinHandlingType::JoinHandlingType(const ::xercesc::DOMAttr &a,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

JoinHandlingType::JoinHandlingType(const ::std::string &s,
                                   const ::xercesc::DOMElement *e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

JoinHandlingType *JoinHandlingType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class JoinHandlingType(*this, f, c);
}

JoinHandlingType::~JoinHandlingType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, JoinHandlingType>
    _xsd_JoinHandlingType_type_factory_init("join-handling-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// DenyUsersListType
//

DenyUsersListType::DenyUsersListType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      target_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

DenyUsersListType::DenyUsersListType(const DenyUsersListType &x,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      target_(x.target_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

DenyUsersListType::DenyUsersListType(const ::xercesc::DOMElement &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), target_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void DenyUsersListType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// target
		//
		if (n.name() == "target" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<TargetType> r(TargetTraits::create(i, f, this));

			this->target_.push_back(::std::move(r));
			continue;
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

DenyUsersListType *DenyUsersListType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class DenyUsersListType(*this, f, c);
}

DenyUsersListType &DenyUsersListType::operator=(const DenyUsersListType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->target_ = x.target_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

DenyUsersListType::~DenyUsersListType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, DenyUsersListType>
    _xsd_DenyUsersListType_type_factory_init("deny-users-list-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// AllowedUsersListType
//

AllowedUsersListType::AllowedUsersListType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      target_(this), persistent_list_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

AllowedUsersListType::AllowedUsersListType(const AllowedUsersListType &x,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      target_(x.target_, f, this), persistent_list_(x.persistent_list_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

AllowedUsersListType::AllowedUsersListType(const ::xercesc::DOMElement &e,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), target_(this), persistent_list_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void AllowedUsersListType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// target
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "target", "urn:ietf:params:xml:ns:xcon-conference-info", &::xsd::cxx::tree::factory_impl<TargetType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<TargetType> r(dynamic_cast<TargetType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->target_.push_back(::std::move(r));
				continue;
			}
		}

		// persistent-list
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "persistent-list", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<PersistentListType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->persistent_list_) {
					::std::unique_ptr<PersistentListType> r(dynamic_cast<PersistentListType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->persistent_list_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

AllowedUsersListType *AllowedUsersListType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class AllowedUsersListType(*this, f, c);
}

AllowedUsersListType &AllowedUsersListType::operator=(const AllowedUsersListType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->target_ = x.target_;
		this->persistent_list_ = x.persistent_list_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

AllowedUsersListType::~AllowedUsersListType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, AllowedUsersListType>
    _xsd_AllowedUsersListType_type_factory_init("allowed-users-list-type",
                                                "urn:ietf:params:xml:ns:xcon-conference-info");

// PersistentListType
//

PersistentListType::PersistentListType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      user_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

PersistentListType::PersistentListType(const PersistentListType &x,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      user_(x.user_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

PersistentListType::PersistentListType(const ::xercesc::DOMElement &e,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), user_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void PersistentListType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// user
		//
		if (n.name() == "user" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<UserType> r(UserTraits::create(i, f, this));

			this->user_.push_back(::std::move(r));
			continue;
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

PersistentListType *PersistentListType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class PersistentListType(*this, f, c);
}

PersistentListType &PersistentListType::operator=(const PersistentListType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->user_ = x.user_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

PersistentListType::~PersistentListType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, PersistentListType>
    _xsd_PersistentListType_type_factory_init("persistent-list-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// TargetType
//

TargetType::TargetType(const UriType &uri, const MethodType &method)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      uri_(uri, this), method_(method, this), any_attribute_(this->getDomDocument()) {
}

TargetType::TargetType(const TargetType &x,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      uri_(x.uri_, f, this), method_(x.method_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

TargetType::TargetType(const ::xercesc::DOMElement &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), uri_(this), method_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, false, false, true);
		this->parse(p, f);
	}
}

void TargetType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "uri" && n.namespace_().empty()) {
			this->uri_.set(UriTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "method" && n.namespace_().empty()) {
			this->method_.set(MethodTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!uri_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("uri", "");
	}

	if (!method_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("method", "");
	}
}

TargetType *TargetType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class TargetType(*this, f, c);
}

TargetType &TargetType::operator=(const TargetType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->uri_ = x.uri_;
		this->method_ = x.method_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

TargetType::~TargetType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, TargetType>
    _xsd_TargetType_type_factory_init("target-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// MethodType
//

MethodType::MethodType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

MethodType::MethodType(const char *_xsd_String_base) : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MethodType::MethodType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MethodType::MethodType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MethodType::MethodType(const MethodType &x,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

MethodType::MethodType(const ::xercesc::DOMElement &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

MethodType::MethodType(const ::xercesc::DOMAttr &a,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

MethodType::MethodType(const ::std::string &s,
                       const ::xercesc::DOMElement *e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

MethodType *MethodType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class MethodType(*this, f, c);
}

MethodType::~MethodType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, MethodType>
    _xsd_MethodType_type_factory_init("method-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// ProvideAnonymityType
//

ProvideAnonymityType::ProvideAnonymityType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

ProvideAnonymityType::ProvideAnonymityType(const char *_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

ProvideAnonymityType::ProvideAnonymityType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

ProvideAnonymityType::ProvideAnonymityType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

ProvideAnonymityType::ProvideAnonymityType(const ProvideAnonymityType &x,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

ProvideAnonymityType::ProvideAnonymityType(const ::xercesc::DOMElement &e,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

ProvideAnonymityType::ProvideAnonymityType(const ::xercesc::DOMAttr &a,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

ProvideAnonymityType::ProvideAnonymityType(const ::std::string &s,
                                           const ::xercesc::DOMElement *e,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

ProvideAnonymityType *ProvideAnonymityType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ProvideAnonymityType(*this, f, c);
}

ProvideAnonymityType::~ProvideAnonymityType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ProvideAnonymityType>
    _xsd_ProvideAnonymityType_type_factory_init("provide-anonymity-type",
                                                "urn:ietf:params:xml:ns:xcon-conference-info");

// MixerType
//

MixerType::MixerType(const FloorType &floor, const NameType &name)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      floor_(floor, this), controls_(this), any_(this->getDomDocument()), name_(name, this),
      any_attribute_(this->getDomDocument()) {
}

MixerType::MixerType(::std::unique_ptr<FloorType> floor, const NameType &name)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      floor_(std::move(floor), this), controls_(this), any_(this->getDomDocument()), name_(name, this),
      any_attribute_(this->getDomDocument()) {
}

MixerType::MixerType(const MixerType &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      floor_(x.floor_, f, this), controls_(x.controls_, f, this), any_(x.any_, this->getDomDocument()),
      name_(x.name_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

MixerType::MixerType(const ::xercesc::DOMElement &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), floor_(this), controls_(this),
      any_(this->getDomDocument()), name_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void MixerType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// floor
		//
		if (n.name() == "floor" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<FloorType> r(FloorTraits::create(i, f, this));

			if (!floor_.present()) {
				this->floor_.set(::std::move(r));
				continue;
			}
		}

		// controls
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "controls", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<ControlsType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<ControlsType> r(dynamic_cast<ControlsType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->controls_.push_back(::std::move(r));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!floor_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("floor", "urn:ietf:params:xml:ns:xcon-conference-info");
	}

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "name" && n.namespace_().empty()) {
			this->name_.set(NameTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!name_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("name", "");
	}
}

MixerType *MixerType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class MixerType(*this, f, c);
}

MixerType &MixerType::operator=(const MixerType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->floor_ = x.floor_;
		this->controls_ = x.controls_;
		this->any_ = x.any_;
		this->name_ = x.name_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

MixerType::~MixerType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, MixerType>
    _xsd_MixerType_type_factory_init("mixer-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// MixerNameType
//

MixerNameType::MixerNameType() : ::LinphonePrivate::Xsd::XmlSchema::String() {
}

MixerNameType::MixerNameType(const char *_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MixerNameType::MixerNameType(const ::std::string &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MixerNameType::MixerNameType(const ::LinphonePrivate::Xsd::XmlSchema::String &_xsd_String_base)
    : ::LinphonePrivate::Xsd::XmlSchema::String(_xsd_String_base) {
}

MixerNameType::MixerNameType(const MixerNameType &x,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(x, f, c) {
}

MixerNameType::MixerNameType(const ::xercesc::DOMElement &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(e, f, c) {
}

MixerNameType::MixerNameType(const ::xercesc::DOMAttr &a,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(a, f, c) {
}

MixerNameType::MixerNameType(const ::std::string &s,
                             const ::xercesc::DOMElement *e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::String(s, e, f, c) {
}

MixerNameType *MixerNameType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class MixerNameType(*this, f, c);
}

MixerNameType::~MixerNameType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, MixerNameType>
    _xsd_MixerNameType_type_factory_init("mixer-name-type", "urn:ietf:params:xml:ns:xcon-conference-info");

// ConferenceInfoDiff
//

ConferenceInfoDiff::ConferenceInfoDiff(const EntityType &entity)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      add_(this), remove_(this), replace_(this), any_(this->getDomDocument()), entity_(entity, this),
      any_attribute_(this->getDomDocument()) {
}

ConferenceInfoDiff::ConferenceInfoDiff(const ConferenceInfoDiff &x,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      add_(x.add_, f, this), remove_(x.remove_, f, this), replace_(x.replace_, f, this),
      any_(x.any_, this->getDomDocument()), entity_(x.entity_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConferenceInfoDiff::ConferenceInfoDiff(const ::xercesc::DOMElement &e,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), add_(this), remove_(this), replace_(this),
      any_(this->getDomDocument()), entity_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConferenceInfoDiff::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// add
		//
		if (n.name() == "add" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<AddType> r(AddTraits::create(i, f, this));

			this->add_.push_back(::std::move(r));
			continue;
		}

		// remove
		//
		if (n.name() == "remove" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<RemoveType> r(RemoveTraits::create(i, f, this));

			this->remove_.push_back(::std::move(r));
			continue;
		}

		// replace
		//
		if (n.name() == "replace" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<ReplaceType> r(ReplaceTraits::create(i, f, this));

			this->replace_.push_back(::std::move(r));
			continue;
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
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
}

ConferenceInfoDiff *ConferenceInfoDiff::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConferenceInfoDiff(*this, f, c);
}

ConferenceInfoDiff &ConferenceInfoDiff::operator=(const ConferenceInfoDiff &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->add_ = x.add_;
		this->remove_ = x.remove_;
		this->replace_ = x.replace_;
		this->any_ = x.any_;
		this->entity_ = x.entity_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConferenceInfoDiff::~ConferenceInfoDiff() {
}

// Entry
//

Entry::Entry(const BaseType &base)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      base_(base, this), mixing_start_offset_(this), mixing_end_offset_(this), can_join_after_offset_(this),
      must_join_before_offset_(this), request_user_(this), notify_end_of_conference_(this),
      allowed_extend_mixing_end_offset_(this), any_(this->getDomDocument()) {
}

Entry::Entry(::std::unique_ptr<BaseType> base)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      base_(std::move(base), this), mixing_start_offset_(this), mixing_end_offset_(this), can_join_after_offset_(this),
      must_join_before_offset_(this), request_user_(this), notify_end_of_conference_(this),
      allowed_extend_mixing_end_offset_(this), any_(this->getDomDocument()) {
}

Entry::Entry(const Entry &x,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      base_(x.base_, f, this), mixing_start_offset_(x.mixing_start_offset_, f, this),
      mixing_end_offset_(x.mixing_end_offset_, f, this), can_join_after_offset_(x.can_join_after_offset_, f, this),
      must_join_before_offset_(x.must_join_before_offset_, f, this), request_user_(x.request_user_, f, this),
      notify_end_of_conference_(x.notify_end_of_conference_, f, this),
      allowed_extend_mixing_end_offset_(x.allowed_extend_mixing_end_offset_, f, this),
      any_(x.any_, this->getDomDocument()) {
}

Entry::Entry(const ::xercesc::DOMElement &e,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), base_(this), mixing_start_offset_(this),
      mixing_end_offset_(this), can_join_after_offset_(this), must_join_before_offset_(this), request_user_(this),
      notify_end_of_conference_(this), allowed_extend_mixing_end_offset_(this), any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void Entry::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// base
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "base", "urn:ietf:params:xml:ns:xcon-conference-info", &::xsd::cxx::tree::factory_impl<BaseType>, false,
			    true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!base_.present()) {
					::std::unique_ptr<BaseType> r(dynamic_cast<BaseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->base_.set(::std::move(r));
					continue;
				}
			}
		}

		// mixing-start-offset
		//
		if (n.name() == "mixing-start-offset" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<MixingStartOffsetType> r(MixingStartOffsetTraits::create(i, f, this));

			if (!this->mixing_start_offset_) {
				this->mixing_start_offset_.set(::std::move(r));
				continue;
			}
		}

		// mixing-end-offset
		//
		if (n.name() == "mixing-end-offset" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			::std::unique_ptr<MixingEndOffsetType> r(MixingEndOffsetTraits::create(i, f, this));

			if (!this->mixing_end_offset_) {
				this->mixing_end_offset_.set(::std::move(r));
				continue;
			}
		}

		// can-join-after-offset
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "can-join-after-offset", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<CanJoinAfterOffsetType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->can_join_after_offset_) {
					::std::unique_ptr<CanJoinAfterOffsetType> r(dynamic_cast<CanJoinAfterOffsetType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->can_join_after_offset_.set(::std::move(r));
					continue;
				}
			}
		}

		// must-join-before-offset
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "must-join-before-offset", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<MustJoinBeforeOffsetType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->must_join_before_offset_) {
					::std::unique_ptr<MustJoinBeforeOffsetType> r(dynamic_cast<MustJoinBeforeOffsetType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->must_join_before_offset_.set(::std::move(r));
					continue;
				}
			}
		}

		// request-user
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "request-user", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<RequestUserType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->request_user_) {
					::std::unique_ptr<RequestUserType> r(dynamic_cast<RequestUserType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->request_user_.set(::std::move(r));
					continue;
				}
			}
		}

		// notify-end-of-conference
		//
		if (n.name() == "notify-end-of-conference" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			if (!this->notify_end_of_conference_) {
				this->notify_end_of_conference_.set(NotifyEndOfConferenceTraits::create(i, f, this));
				continue;
			}
		}

		// allowed-extend-mixing-end-offset
		//
		if (n.name() == "allowed-extend-mixing-end-offset" &&
		    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			if (!this->allowed_extend_mixing_end_offset_) {
				this->allowed_extend_mixing_end_offset_.set(Allowed_extend_mixing_end_offsetTraits::create(i, f, this));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!base_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("base", "urn:ietf:params:xml:ns:xcon-conference-info");
	}
}

Entry *Entry::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Entry(*this, f, c);
}

Entry &Entry::operator=(const Entry &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->base_ = x.base_;
		this->mixing_start_offset_ = x.mixing_start_offset_;
		this->mixing_end_offset_ = x.mixing_end_offset_;
		this->can_join_after_offset_ = x.can_join_after_offset_;
		this->must_join_before_offset_ = x.must_join_before_offset_;
		this->request_user_ = x.request_user_;
		this->notify_end_of_conference_ = x.notify_end_of_conference_;
		this->allowed_extend_mixing_end_offset_ = x.allowed_extend_mixing_end_offset_;
		this->any_ = x.any_;
	}

	return *this;
}

Entry::~Entry() {
}

// Floor
//

Floor::Floor(const IdType &id)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      media_label_(this), algorithm_(this), max_floor_users_(this), moderator_id_(this), any_(this->getDomDocument()),
      id_(id, this), any_attribute_(this->getDomDocument()) {
}

Floor::Floor(const Floor &x,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      media_label_(x.media_label_, f, this), algorithm_(x.algorithm_, f, this),
      max_floor_users_(x.max_floor_users_, f, this), moderator_id_(x.moderator_id_, f, this),
      any_(x.any_, this->getDomDocument()), id_(x.id_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

Floor::Floor(const ::xercesc::DOMElement &e,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), media_label_(this), algorithm_(this),
      max_floor_users_(this), moderator_id_(this), any_(this->getDomDocument()), id_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void Floor::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// media-label
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "media-label", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<MediaLabelType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<MediaLabelType> r(dynamic_cast<MediaLabelType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->media_label_.push_back(::std::move(r));
				continue;
			}
		}

		// algorithm
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "algorithm", "urn:ietf:params:xml:ns:xcon-conference-info",
			    &::xsd::cxx::tree::factory_impl<AlgorithmType>, false, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->algorithm_) {
					::std::unique_ptr<AlgorithmType> r(dynamic_cast<AlgorithmType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->algorithm_.set(::std::move(r));
					continue;
				}
			}
		}

		// max-floor-users
		//
		if (n.name() == "max-floor-users" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			if (!this->max_floor_users_) {
				this->max_floor_users_.set(MaxFloorUsersTraits::create(i, f, this));
				continue;
			}
		}

		// moderator-id
		//
		if (n.name() == "moderator-id" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			if (!this->moderator_id_) {
				this->moderator_id_.set(ModeratorIdTraits::create(i, f, this));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
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

Floor *Floor::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Floor(*this, f, c);
}

Floor &Floor::operator=(const Floor &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->media_label_ = x.media_label_;
		this->algorithm_ = x.algorithm_;
		this->max_floor_users_ = x.max_floor_users_;
		this->moderator_id_ = x.moderator_id_;
		this->any_ = x.any_;
		this->id_ = x.id_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

Floor::~Floor() {
}

// Target
//

Target::Target(const UriType &uri)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      uri_(uri, this), any_attribute_(this->getDomDocument()) {
}

Target::Target(const Target &x,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      uri_(x.uri_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

Target::Target(const ::xercesc::DOMElement &e,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), uri_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, false, false, true);
		this->parse(p, f);
	}
}

void Target::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "uri" && n.namespace_().empty()) {
			this->uri_.set(UriTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!uri_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("uri", "");
	}
}

Target *Target::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Target(*this, f, c);
}

Target &Target::operator=(const Target &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->uri_ = x.uri_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

Target::~Target() {
}

// User
//

User::User(const NameType &name, const NicknameType &nickname, const IdType &id)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      email_(this), any_(this->getDomDocument()), name_(name, this), nickname_(nickname, this), id_(id, this),
      any_attribute_(this->getDomDocument()) {
}

User::User(const User &x, ::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      email_(x.email_, f, this), any_(x.any_, this->getDomDocument()), name_(x.name_, f, this),
      nickname_(x.nickname_, f, this), id_(x.id_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

User::User(const ::xercesc::DOMElement &e,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), email_(this), any_(this->getDomDocument()),
      name_(this), nickname_(this), id_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void User::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// email
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "email", "urn:ietf:params:xml:ns:xcon-conference-info", &::xsd::cxx::tree::factory_impl<EmailType>,
			    false, true, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<EmailType> r(dynamic_cast<EmailType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->email_.push_back(::std::move(r));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-conference-info")) {
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

		if (n.name() == "name" && n.namespace_().empty()) {
			this->name_.set(NameTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "nickname" && n.namespace_().empty()) {
			this->nickname_.set(NicknameTraits::create(i, f, this));
			continue;
		}

		if (n.name() == "id" && n.namespace_().empty()) {
			this->id_.set(IdTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!name_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("name", "");
	}

	if (!nickname_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("nickname", "");
	}

	if (!id_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("id", "");
	}
}

User *User::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class User(*this, f, c);
}

User &User::operator=(const User &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->email_ = x.email_;
		this->any_ = x.any_;
		this->name_ = x.name_;
		this->nickname_ = x.nickname_;
		this->id_ = x.id_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

User::~User() {
}

// Floor1
//

Floor1::Floor1(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &_xsd_Boolean_base, const IdType &id)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Boolean,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(_xsd_Boolean_base),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), id_(id, this),
      any_attribute_(this->getDomDocument()) {
}

Floor1::Floor1(const Floor1 &x,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Boolean,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(x, f, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), id_(x.id_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

Floor1::Floor1(const ::xercesc::DOMElement &e,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Boolean,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), id_(this), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, false, false, true);
		this->parse(p, f);
	}
}

void Floor1::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "id" && n.namespace_().empty()) {
			this->id_.set(IdTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
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

Floor1 *Floor1::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Floor1(*this, f, c);
}

Floor1 &Floor1::operator=(const Floor1 &x) {
	if (this != &x) {
		static_cast<::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Boolean, char,
		                                               ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(*this) = x;
		this->id_ = x.id_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

Floor1::~Floor1() {
}

// Add1
//

Add1::Add1(const SelType &sel)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Add(sel), any_attribute_(this->getDomDocument()) {
}

Add1::Add1(const ::LinphonePrivate::Xsd::XmlSchema::Type &_xsd_Type_base, const SelType &sel)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Add(_xsd_Type_base, sel), any_attribute_(this->getDomDocument()) {
}

Add1::Add1(const Add1 &x, ::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Add(x, f, c),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

Add1::Add1(const ::xercesc::DOMElement &e,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Add(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void Add1::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconConferenceInfo::Add::parse(p, f);

	p.reset_attributes();

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

Add1 *Add1::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f, ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Add1(*this, f, c);
}

Add1 &Add1::operator=(const Add1 &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconConferenceInfo::Add &>(*this) = x;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

Add1::~Add1() {
}

// Remove1
//

Remove1::Remove1(const SelType &sel)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Remove(sel),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), any_attribute_(this->getDomDocument()) {
}

Remove1::Remove1(const Remove1 &x,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Remove(x, f, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

Remove1::Remove1(const ::xercesc::DOMElement &e,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Remove(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, false, false, true);
		this->parse(p, f);
	}
}

void Remove1::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconConferenceInfo::Remove::parse(p, f);

	p.reset_attributes();

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

Remove1 *Remove1::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Remove1(*this, f, c);
}

Remove1 &Remove1::operator=(const Remove1 &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconConferenceInfo::Remove &>(*this) = x;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

Remove1::~Remove1() {
}

// Replace1
//

Replace1::Replace1(const SelType &sel)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Replace(sel), any_attribute_(this->getDomDocument()) {
}

Replace1::Replace1(const ::LinphonePrivate::Xsd::XmlSchema::Type &_xsd_Type_base, const SelType &sel)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Replace(_xsd_Type_base, sel), any_attribute_(this->getDomDocument()) {
}

Replace1::Replace1(const Replace1 &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Replace(x, f, c),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

Replace1::Replace1(const ::xercesc::DOMElement &e,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::Replace(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void Replace1::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconConferenceInfo::Replace::parse(p, f);

	p.reset_attributes();

	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

Replace1 *Replace1::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class Replace1(*this, f, c);
}

Replace1 &Replace1::operator=(const Replace1 &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconConferenceInfo::Replace &>(*this) = x;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

Replace1::~Replace1() {
}

// MixingStartOffset
//

MixingStartOffset::MixingStartOffset(const ::LinphonePrivate::Xsd::XmlSchema::DateTime &_xsd_DateTime_base,
                                     const RequiredParticipantType &required_participant)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType(_xsd_DateTime_base),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), required_participant_(required_participant, this),
      any_attribute_(this->getDomDocument()) {
}

MixingStartOffset::MixingStartOffset(const MixingStartOffset &x,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType(x, f, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      required_participant_(x.required_participant_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

MixingStartOffset::MixingStartOffset(const ::xercesc::DOMElement &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), required_participant_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, false, false, true);
		this->parse(p, f);
	}
}

void MixingStartOffset::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "required-participant" && n.namespace_().empty()) {
			this->required_participant_.set(RequiredParticipantTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!required_participant_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("required-participant", "");
	}
}

MixingStartOffset *MixingStartOffset::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class MixingStartOffset(*this, f, c);
}

MixingStartOffset &MixingStartOffset::operator=(const MixingStartOffset &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconConferenceInfo::TimeType &>(*this) = x;
		this->required_participant_ = x.required_participant_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

MixingStartOffset::~MixingStartOffset() {
}

// MixingEndOffset
//

MixingEndOffset::MixingEndOffset(const ::LinphonePrivate::Xsd::XmlSchema::DateTime &_xsd_DateTime_base,
                                 const RequiredParticipantType &required_participant)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType(_xsd_DateTime_base),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), required_participant_(required_participant, this),
      any_attribute_(this->getDomDocument()) {
}

MixingEndOffset::MixingEndOffset(const MixingEndOffset &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType(x, f, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      required_participant_(x.required_participant_, f, this),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

MixingEndOffset::MixingEndOffset(const ::xercesc::DOMElement &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), required_participant_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, false, false, true);
		this->parse(p, f);
	}
}

void MixingEndOffset::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	while (p.more_attributes()) {
		const ::xercesc::DOMAttr &i(p.next_attribute());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		if (n.name() == "required-participant" && n.namespace_().empty()) {
			this->required_participant_.set(RequiredParticipantTraits::create(i, f, this));
			continue;
		}

		// any_attribute
		//
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}

	if (!required_participant_.present()) {
		throw ::xsd::cxx::tree::expected_attribute<char>("required-participant", "");
	}
}

MixingEndOffset *MixingEndOffset::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class MixingEndOffset(*this, f, c);
}

MixingEndOffset &MixingEndOffset::operator=(const MixingEndOffset &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconConferenceInfo::TimeType &>(*this) = x;
		this->required_participant_ = x.required_participant_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

MixingEndOffset::~MixingEndOffset() {
}
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

#include <ostream>

#include <xsd/cxx/tree/std-ostream-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::std_ostream_plate<0, char> std_ostream_plate_init;
}

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
::std::ostream &operator<<(::std::ostream &o, const Xpath &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, Xpath> _xsd_Xpath_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const XpathAdd &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, XpathAdd> _xsd_XpathAdd_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, Pos::Value i) {
	return o << Pos::_xsd_Pos_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const Pos &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, Pos> _xsd_Pos_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const Type &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, Type> _xsd_Type_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const Add &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	o << ::std::endl << "sel: " << i.getSel();
	if (i.getPos()) {
		o << ::std::endl << "pos: " << *i.getPos();
	}

	if (i.getType()) {
		o << ::std::endl << "type: " << *i.getType();
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, Add> _xsd_Add_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const Replace &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	o << ::std::endl << "sel: " << i.getSel();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, Replace> _xsd_Replace_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, Ws::Value i) {
	return o << Ws::_xsd_Ws_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const Ws &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, Ws> _xsd_Ws_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const Remove &i) {
	o << ::std::endl << "sel: " << i.getSel();
	if (i.getWs()) {
		o << ::std::endl << "ws: " << *i.getWs();
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, Remove> _xsd_Remove_std_ostream_init;
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
::std::ostream &operator<<(::std::ostream &o, const ConferenceTimeType &i) {
	for (ConferenceTimeType::EntryConstIterator b(i.getEntry().begin()), e(i.getEntry().end()); b != e; ++b) {
		o << ::std::endl << "entry: " << *b;
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConferenceTimeType>
    _xsd_ConferenceTimeType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const TimeType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::DateTime &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, TimeType> _xsd_TimeType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const RoleType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, RoleType> _xsd_RoleType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const MixingModeType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, MixingModeType> _xsd_MixingModeType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CodecsType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "codec: ";
		om.insert(o, i.getCodec());
	}

	o << ::std::endl << "decision: " << i.getDecision();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CodecsType> _xsd_CodecsType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CodecType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSubtype()) {
			o << ::std::endl << "subtype: ";
			om.insert(o, *i.getSubtype());
		}
	}

	o << ::std::endl << "name: " << i.getName();
	o << ::std::endl << "policy: " << i.getPolicy();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CodecType> _xsd_CodecType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const DecisionType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, DecisionType> _xsd_DecisionType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const PolicyType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, PolicyType> _xsd_PolicyType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ControlsType &i) {
	if (i.getMute()) {
		o << ::std::endl << "mute: " << *i.getMute();
	}

	if (i.getPauseVideo()) {
		o << ::std::endl << "pause-video: " << *i.getPauseVideo();
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getGain()) {
			o << ::std::endl << "gain: ";
			om.insert(o, *i.getGain());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getVideoLayout()) {
			o << ::std::endl << "video-layout: ";
			om.insert(o, *i.getVideoLayout());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ControlsType> _xsd_ControlsType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const GainType &i) {
	o << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, GainType> _xsd_GainType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const VideoLayoutType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, VideoLayoutType> _xsd_VideoLayoutType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const FloorInformationType &i) {
	if (i.getConferenceID()) {
		o << ::std::endl << "conference-ID: " << *i.getConferenceID();
	}

	if (i.getAllowFloorEvents()) {
		o << ::std::endl << "allow-floor-events: " << *i.getAllowFloorEvents();
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getFloorRequestHandling()) {
			o << ::std::endl << "floor-request-handling: ";
			om.insert(o, *i.getFloorRequestHandling());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConferenceFloorPolicy()) {
			o << ::std::endl << "conference-floor-policy: ";
			om.insert(o, *i.getConferenceFloorPolicy());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, FloorInformationType>
    _xsd_FloorInformationType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const FloorRequestHandlingType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, FloorRequestHandlingType>
    _xsd_FloorRequestHandlingType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConferenceFloorPolicy &i) {
	for (ConferenceFloorPolicy::FloorConstIterator b(i.getFloor().begin()), e(i.getFloor().end()); b != e; ++b) {
		o << ::std::endl << "floor: " << *b;
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConferenceFloorPolicy>
    _xsd_ConferenceFloorPolicy_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const AlgorithmType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, AlgorithmType> _xsd_AlgorithmType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UserAdmissionPolicyType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UserAdmissionPolicyType>
    _xsd_UserAdmissionPolicyType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const JoinHandlingType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, JoinHandlingType>
    _xsd_JoinHandlingType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const DenyUsersListType &i) {
	for (DenyUsersListType::TargetConstIterator b(i.getTarget().begin()), e(i.getTarget().end()); b != e; ++b) {
		o << ::std::endl << "target: " << *b;
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, DenyUsersListType>
    _xsd_DenyUsersListType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const AllowedUsersListType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (AllowedUsersListType::TargetConstIterator b(i.getTarget().begin()), e(i.getTarget().end()); b != e; ++b) {
			o << ::std::endl << "target: ";
			om.insert(o, *b);
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getPersistentList()) {
			o << ::std::endl << "persistent-list: ";
			om.insert(o, *i.getPersistentList());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, AllowedUsersListType>
    _xsd_AllowedUsersListType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const PersistentListType &i) {
	for (PersistentListType::UserConstIterator b(i.getUser().begin()), e(i.getUser().end()); b != e; ++b) {
		o << ::std::endl << "user: " << *b;
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, PersistentListType>
    _xsd_PersistentListType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const TargetType &i) {
	o << ::std::endl << "uri: " << i.getUri();
	o << ::std::endl << "method: " << i.getMethod();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, TargetType> _xsd_TargetType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const MethodType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, MethodType> _xsd_MethodType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ProvideAnonymityType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ProvideAnonymityType>
    _xsd_ProvideAnonymityType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const MixerType &i) {
	o << ::std::endl << "floor: " << i.getFloor();
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (MixerType::ControlsConstIterator b(i.getControls().begin()), e(i.getControls().end()); b != e; ++b) {
			o << ::std::endl << "controls: ";
			om.insert(o, *b);
		}
	}

	o << ::std::endl << "name: " << i.getName();
	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, MixerType> _xsd_MixerType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const MixerNameType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, MixerNameType> _xsd_MixerNameType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConferenceInfoDiff &i) {
	for (ConferenceInfoDiff::AddConstIterator b(i.getAdd().begin()), e(i.getAdd().end()); b != e; ++b) {
		o << ::std::endl << "add: " << *b;
	}

	for (ConferenceInfoDiff::RemoveConstIterator b(i.getRemove().begin()), e(i.getRemove().end()); b != e; ++b) {
		o << ::std::endl << "remove: " << *b;
	}

	for (ConferenceInfoDiff::ReplaceConstIterator b(i.getReplace().begin()), e(i.getReplace().end()); b != e; ++b) {
		o << ::std::endl << "replace: " << *b;
	}

	o << ::std::endl << "entity: " << i.getEntity();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Entry &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "base: ";
		om.insert(o, i.getBase());
	}

	if (i.getMixingStartOffset()) {
		o << ::std::endl << "mixing-start-offset: " << *i.getMixingStartOffset();
	}

	if (i.getMixingEndOffset()) {
		o << ::std::endl << "mixing-end-offset: " << *i.getMixingEndOffset();
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getCan_join_after_offset()) {
			o << ::std::endl << "can-join-after-offset: ";
			om.insert(o, *i.getCan_join_after_offset());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getMust_join_before_offset()) {
			o << ::std::endl << "must-join-before-offset: ";
			om.insert(o, *i.getMust_join_before_offset());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getRequestUser()) {
			o << ::std::endl << "request-user: ";
			om.insert(o, *i.getRequestUser());
		}
	}

	if (i.getNotify_end_of_conference()) {
		o << ::std::endl << "notify-end-of-conference: " << *i.getNotify_end_of_conference();
	}

	if (i.getAllowed_extend_mixing_end_offset()) {
		o << ::std::endl << "allowed-extend-mixing-end-offset: " << *i.getAllowed_extend_mixing_end_offset();
	}

	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Floor &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (Floor::MediaLabelConstIterator b(i.getMediaLabel().begin()), e(i.getMediaLabel().end()); b != e; ++b) {
			o << ::std::endl << "media-label: ";
			om.insert(o, *b);
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getAlgorithm()) {
			o << ::std::endl << "algorithm: ";
			om.insert(o, *i.getAlgorithm());
		}
	}

	if (i.getMaxFloorUsers()) {
		o << ::std::endl << "max-floor-users: " << *i.getMaxFloorUsers();
	}

	if (i.getModeratorId()) {
		o << ::std::endl << "moderator-id: " << *i.getModeratorId();
	}

	o << ::std::endl << "id: " << i.getId();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Target &i) {
	o << ::std::endl << "uri: " << i.getUri();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const User &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (User::EmailConstIterator b(i.getEmail().begin()), e(i.getEmail().end()); b != e; ++b) {
			o << ::std::endl << "email: ";
			om.insert(o, *b);
		}
	}

	o << ::std::endl << "name: " << i.getName();
	o << ::std::endl << "nickname: " << i.getNickname();
	o << ::std::endl << "id: " << i.getId();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Floor1 &i) {
	o << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Boolean, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);

	o << ::std::endl << "id: " << i.getId();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Add1 &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::Add &>(i);

	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Remove1 &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::Remove &>(i);

	return o;
}

::std::ostream &operator<<(::std::ostream &o, const Replace1 &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::Replace &>(i);

	return o;
}

::std::ostream &operator<<(::std::ostream &o, const MixingStartOffset &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType &>(i);

	o << ::std::endl << "required-participant: " << i.getRequiredParticipant();
	return o;
}

::std::ostream &operator<<(::std::ostream &o, const MixingEndOffset &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType &>(i);

	o << ::std::endl << "required-participant: " << i.getRequiredParticipant();
	return o;
}
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

#include <istream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/sax/std-input-source.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {}
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(const ::std::string &u,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(const ::std::string &u,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(const ::std::string &u,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::std::istream &is,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::std::istream &is,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::std::istream &is,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::std::istream &is,
                        const ::std::string &sid,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::std::istream &is,
                        const ::std::string &sid,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::std::istream &is,
                        const ::std::string &sid,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::xercesc::InputSource &i,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::xercesc::InputSource &i,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::xercesc::InputSource &i,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(const ::xercesc::DOMDocument &doc,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceInfoDiff(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "conference-info-diff" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff, char>::create(e, f,
		                                                                                                           0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-info-diff",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff>
parseConferenceInfoDiff(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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

	if (n.name() == "conference-info-diff" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> r(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff, char>::create(e, f,
		                                                                                                           0));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-info-diff",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(const ::std::string &u,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(const ::std::string &u,
                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(const ::std::string &u,
                ::xercesc::DOMErrorHandler &h,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::std::istream &is,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::std::istream &is,
                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::std::istream &is,
                ::xercesc::DOMErrorHandler &h,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::std::istream &is,
                const ::std::string &sid,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::std::istream &is,
                const ::std::string &sid,
                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::std::istream &is,
                const ::std::string &sid,
                ::xercesc::DOMErrorHandler &h,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::xercesc::InputSource &i,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::xercesc::InputSource &i,
                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::xercesc::InputSource &i,
                ::xercesc::DOMErrorHandler &h,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(const ::xercesc::DOMDocument &doc,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseMixingMode(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "mixing-mode", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "mixing-mode",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>
parseMixingMode(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "mixing-mode", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "mixing-mode",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(const ::std::string &u,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(const ::std::string &u,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(const ::std::string &u,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            const ::std::string &sid,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            const ::std::string &sid,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            const ::std::string &sid,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::xercesc::InputSource &i,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::xercesc::InputSource &i,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::xercesc::InputSource &i,
            ::xercesc::DOMErrorHandler &h,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(const ::xercesc::DOMDocument &doc,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCodecs(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "codecs", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "codecs",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "codecs", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "codecs",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(const ::std::string &u,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(const ::std::string &u,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(const ::std::string &u,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::std::istream &is,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::std::istream &is,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::std::istream &is,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::std::istream &is,
                        const ::std::string &sid,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::std::istream &is,
                        const ::std::string &sid,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::std::istream &is,
                        const ::std::string &sid,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::xercesc::InputSource &i,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::xercesc::InputSource &i,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::xercesc::InputSource &i,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(const ::xercesc::DOMDocument &doc,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferencePassword(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "conference-password", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::String>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::String *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-password",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseConferencePassword(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "conference-password", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::String>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::String *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-password",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(const ::std::string &u,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(const ::std::string &u,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(const ::std::string &u,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              const ::std::string &sid,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              const ::std::string &sid,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              const ::std::string &sid,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::xercesc::InputSource &i,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::xercesc::InputSource &i,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::xercesc::InputSource &i,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(const ::xercesc::DOMDocument &doc,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseControls(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "controls", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "controls",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "controls", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "controls",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(const ::std::string &u,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(const ::std::string &u,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(const ::std::string &u,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              const ::std::string &sid,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              const ::std::string &sid,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              const ::std::string &sid,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::xercesc::InputSource &i,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::xercesc::InputSource &i,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::xercesc::InputSource &i,
              ::xercesc::DOMErrorHandler &h,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(const ::xercesc::DOMDocument &doc,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseLanguage(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "language", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Language>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Language *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "language",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "language", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Language>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Language *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "language",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(const ::std::string &u,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::std::istream &is,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::std::istream &is,
                   const ::std::string &sid,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::xercesc::InputSource &i,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(const ::xercesc::DOMDocument &doc,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowSidebars(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-sidebars" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-sidebars",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowSidebars(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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

	if (n.name() == "allow-sidebars" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-sidebars",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(const ::std::string &u,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::std::istream &is,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::std::istream &is,
                   const ::std::string &sid,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::xercesc::InputSource &i,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(const ::xercesc::DOMDocument &doc,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseCloningParent(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "cloning-parent", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Uri>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Uri *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "cloning-parent",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseCloningParent(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "cloning-parent", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Uri>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Uri *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "cloning-parent",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(const ::std::string &u,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::std::istream &is,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::std::istream &is,
                   const ::std::string &sid,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::xercesc::InputSource &i,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(const ::xercesc::DOMDocument &doc,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseSidebarParent(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebar-parent", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Uri>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Uri *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebar-parent",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri>
parseSidebarParent(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebar-parent", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XmlSchema::Uri>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XmlSchema::Uri *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebar-parent",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(const ::std::string &u,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(const ::std::string &u,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(const ::std::string &u,
                    ::xercesc::DOMErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::std::istream &is,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::std::istream &is,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::std::istream &is,
                    ::xercesc::DOMErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::std::istream &is,
                    const ::std::string &sid,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::std::istream &is,
                    const ::std::string &sid,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::std::istream &is,
                    const ::std::string &sid,
                    ::xercesc::DOMErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::xercesc::InputSource &i,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::xercesc::InputSource &i,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::xercesc::InputSource &i,
                    ::xercesc::DOMErrorHandler &h,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(const ::xercesc::DOMDocument &doc,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseConferenceTime(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "conference-time", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>, true, true, e,
	    n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-time",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>
parseConferenceTime(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "conference-time", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType>, true, true, e,
	    n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-time",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(const ::std::string &u,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(const ::std::string &u,
                                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(const ::std::string &u,
                                      ::xercesc::DOMErrorHandler &h,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::std::istream &is,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::std::istream &is,
                                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::std::istream &is,
                                      ::xercesc::DOMErrorHandler &h,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::std::istream &is,
                                      const ::std::string &sid,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::std::istream &is,
                                      const ::std::string &sid,
                                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::std::istream &is,
                                      const ::std::string &sid,
                                      ::xercesc::DOMErrorHandler &h,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::xercesc::InputSource &i,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::xercesc::InputSource &i,
                                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::xercesc::InputSource &i,
                                      ::xercesc::DOMErrorHandler &h,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(const ::xercesc::DOMDocument &doc,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowConferenceEventSubscription(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-conference-event-subscription" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-conference-event-subscription",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowConferenceEventSubscription(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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

	if (n.name() == "allow-conference-event-subscription" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-conference-event-subscription",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(const ::std::string &u,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(const ::std::string &u,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(const ::std::string &u,
             ::xercesc::DOMErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             ::xercesc::DOMErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             const ::std::string &sid,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             const ::std::string &sid,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             const ::std::string &sid,
             ::xercesc::DOMErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::xercesc::InputSource &i,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::xercesc::InputSource &i,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::xercesc::InputSource &i,
             ::xercesc::DOMErrorHandler &h,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(const ::xercesc::DOMDocument &doc,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseToMixer(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "to-mixer", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "to-mixer",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "to-mixer", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "to-mixer",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(const ::std::string &u,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::std::istream &is,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::std::istream &is,
                      const ::std::string &sid,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::xercesc::InputSource &i,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(const ::xercesc::DOMDocument &doc,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseProvideAnonymity(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "provide-anonymity", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>, true, true,
	    e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "provide-anonymity",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>
parseProvideAnonymity(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "provide-anonymity", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType>, true, true,
	    e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "provide-anonymity",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(const ::std::string &u,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(const ::std::string &u,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(const ::std::string &u,
                                ::xercesc::DOMErrorHandler &h,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::std::istream &is,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::std::istream &is,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::std::istream &is,
                                ::xercesc::DOMErrorHandler &h,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::std::istream &is,
                                const ::std::string &sid,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::std::istream &is,
                                const ::std::string &sid,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::std::istream &is,
                                const ::std::string &sid,
                                ::xercesc::DOMErrorHandler &h,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::xercesc::InputSource &i,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::xercesc::InputSource &i,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::xercesc::InputSource &i,
                                ::xercesc::DOMErrorHandler &h,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(const ::xercesc::DOMDocument &doc,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowReferUsersDynamically(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-refer-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-refer-users-dynamically",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowReferUsersDynamically(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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

	if (n.name() == "allow-refer-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-refer-users-dynamically",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(const ::std::string &u,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(const ::std::string &u,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(const ::std::string &u,
                                 ::xercesc::DOMErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::std::istream &is,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::std::istream &is,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::std::istream &is,
                                 ::xercesc::DOMErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::std::istream &is,
                                 const ::std::string &sid,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::std::istream &is,
                                 const ::std::string &sid,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::std::istream &is,
                                 const ::std::string &sid,
                                 ::xercesc::DOMErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::xercesc::InputSource &i,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::xercesc::InputSource &i,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::xercesc::InputSource &i,
                                 ::xercesc::DOMErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(const ::xercesc::DOMDocument &doc,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowInviteUsersDynamically(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-invite-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-invite-users-dynamically",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowInviteUsersDynamically(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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

	if (n.name() == "allow-invite-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-invite-users-dynamically",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(const ::std::string &u,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(const ::std::string &u,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(const ::std::string &u,
                                 ::xercesc::DOMErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::std::istream &is,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::std::istream &is,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::std::istream &is,
                                 ::xercesc::DOMErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::std::istream &is,
                                 const ::std::string &sid,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::std::istream &is,
                                 const ::std::string &sid,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::std::istream &is,
                                 const ::std::string &sid,
                                 ::xercesc::DOMErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::xercesc::InputSource &i,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::xercesc::InputSource &i,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::xercesc::InputSource &i,
                                 ::xercesc::DOMErrorHandler &h,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(const ::xercesc::DOMDocument &doc,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowRemoveUsersDynamically(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-remove-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-remove-users-dynamically",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean>
parseAllowRemoveUsersDynamically(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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

	if (n.name() == "allow-remove-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> r(new ::LinphonePrivate::Xsd::XmlSchema::Boolean(
		    ::xsd::cxx::tree::traits<::LinphonePrivate::Xsd::XmlSchema::Boolean, char>::create(e, f, 0)));
		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-remove-users-dynamically",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(const ::std::string &u,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(const ::std::string &u,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::std::istream &is,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::std::istream &is,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::std::istream &is,
               const ::std::string &sid,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::std::istream &is,
               const ::std::string &sid,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::xercesc::InputSource &i,
               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::xercesc::InputSource &i,
               ::xercesc::DOMErrorHandler &h,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(const ::xercesc::DOMDocument &doc,
               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
               const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFromMixer(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "from-mixer", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "from-mixer",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseFromMixer(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "from-mixer", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "from-mixer",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(const ::std::string &u,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::std::istream &is,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::std::istream &is,
                  const ::std::string &sid,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::xercesc::InputSource &i,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(const ::xercesc::DOMDocument &doc,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseJoinHandling(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "join-handling", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "join-handling",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>
parseJoinHandling(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "join-handling", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "join-handling",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(const ::std::string &u,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(const ::std::string &u,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(const ::std::string &u,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::std::istream &is,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::std::istream &is,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::std::istream &is,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::std::istream &is,
                         const ::std::string &sid,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::std::istream &is,
                         const ::std::string &sid,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::std::istream &is,
                         const ::std::string &sid,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::xercesc::InputSource &i,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::xercesc::InputSource &i,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::xercesc::InputSource &i,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(const ::xercesc::DOMDocument &doc,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseUserAdmissionPolicy(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "user-admission-policy", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>, true,
	    true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "user-admission-policy",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>
parseUserAdmissionPolicy(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "user-admission-policy", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType>, true,
	    true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "user-admission-policy",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(const ::std::string &u,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::std::istream &is,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::std::istream &is,
                      const ::std::string &sid,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::xercesc::InputSource &i,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(const ::xercesc::DOMDocument &doc,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseAllowedUsersList(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "allowed-users-list", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>, true, true,
	    e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allowed-users-list",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>
parseAllowedUsersList(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "allowed-users-list", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType>, true, true,
	    e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allowed-users-list",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(const ::std::string &u,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::std::istream &is,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::std::istream &is,
                   const ::std::string &sid,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::xercesc::InputSource &i,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(const ::xercesc::DOMDocument &doc,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseDenyUsersList(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "deny-users-list", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>, true, true, e,
	    n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "deny-users-list",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>
parseDenyUsersList(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "deny-users-list", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType>, true, true, e,
	    n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "deny-users-list",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(const ::std::string &u,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::std::istream &is,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::std::istream &is,
                      const ::std::string &sid,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::xercesc::InputSource &i,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(const ::xercesc::DOMDocument &doc,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>(
		    ::LinphonePrivate::Xsd::XconConferenceInfo::parseFloorInformation(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "floor-information", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>, true, true,
	    e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "floor-information",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>
parseFloorInformation(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "floor-information", "urn:ietf:params:xml:ns:xcon-conference-info",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType>, true, true,
	    e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "floor-information",
	                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
}
} // namespace XconConferenceInfo
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
namespace XconConferenceInfo {
void operator<<(::xercesc::DOMElement &e, const Xpath &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const Xpath &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const Xpath &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Xpath>
    _xsd_Xpath_type_serializer_init("xpath", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const XpathAdd &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const XpathAdd &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const XpathAdd &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, XpathAdd>
    _xsd_XpathAdd_type_serializer_init("xpath-add", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const Pos &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const Pos &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const Pos &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Pos>
    _xsd_Pos_type_serializer_init("pos", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const Type &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const Type &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const Type &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Type>
    _xsd_Type_type_serializer_init("type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const Add &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any
	//
	for (Add::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// sel
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("sel", e));

		a << i.getSel();
	}

	// pos
	//
	if (i.getPos()) {
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("pos", e));

		a << *i.getPos();
	}

	// type
	//
	if (i.getType()) {
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("type", e));

		a << *i.getType();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Add>
    _xsd_Add_type_serializer_init("add", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const Replace &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any
	//
	if (i.getAny()) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*i.getAny())), true));
	}

	// sel
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("sel", e));

		a << i.getSel();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Replace>
    _xsd_Replace_type_serializer_init("replace", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const Ws &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const Ws &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const Ws &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Ws>
    _xsd_Ws_type_serializer_init("ws", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const Remove &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// sel
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("sel", e));

		a << i.getSel();
	}

	// ws
	//
	if (i.getWs()) {
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("ws", e));

		a << *i.getWs();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Remove>
    _xsd_Remove_type_serializer_init("remove", "urn:ietf:params:xml:ns:xcon-conference-info");
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
void serializeConferenceInfoDiff(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &s,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceInfoDiff(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConferenceInfoDiff(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceInfoDiff(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceInfoDiff(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &s,
                                 ::xercesc::DOMErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceInfoDiff(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceInfoDiff(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &s,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceInfoDiff(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConferenceInfoDiff(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceInfoDiff(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceInfoDiff(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &s,
                                 ::xercesc::DOMErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceInfoDiff(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceInfoDiff(::xercesc::DOMDocument &d,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "conference-info-diff" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-info-diff",
		                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConferenceInfoDiff(const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(::xsd::cxx::xml::dom::serialize<char>(
	    "conference-info-diff", "urn:ietf:params:xml:ns:xcon-conference-info", m, f));

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceInfoDiff(*d, s, f);
	return d;
}

void serializeMixingMode(::std::ostream &o,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &s,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                         const ::std::string &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeMixingMode(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeMixingMode(::std::ostream &o,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &s,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                         const ::std::string &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeMixingMode(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeMixingMode(::std::ostream &o,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &s,
                         ::xercesc::DOMErrorHandler &h,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                         const ::std::string &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeMixingMode(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeMixingMode(::xercesc::XMLFormatTarget &t,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &s,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                         const ::std::string &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeMixingMode(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeMixingMode(::xercesc::XMLFormatTarget &t,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &s,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                         const ::std::string &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeMixingMode(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeMixingMode(::xercesc::XMLFormatTarget &t,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &s,
                         ::xercesc::DOMErrorHandler &h,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                         const ::std::string &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeMixingMode(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeMixingMode(::xercesc::DOMDocument &d,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &s,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType) == typeid(s)) {
		if (n.name() == "mixing-mode" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "mixing-mode",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "mixing-mode", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeMixingMode(const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &s,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("mixing-mode", "urn:ietf:params:xml:ns:xcon-conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "mixing-mode", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeMixingMode(*d, s, f);
	return d;
}

void serializeCodecs(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCodecs(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeCodecs(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &s,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCodecs(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCodecs(::std::ostream &o,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &s,
                     ::xercesc::DOMErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCodecs(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCodecs(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCodecs(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeCodecs(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &s,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCodecs(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCodecs(::xercesc::XMLFormatTarget &t,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &s,
                     ::xercesc::DOMErrorHandler &h,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     const ::std::string &e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCodecs(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCodecs(::xercesc::DOMDocument &d,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &s,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType) == typeid(s)) {
		if (n.name() == "codecs" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "codecs",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "codecs", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCodecs(const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &s,
                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("codecs", "urn:ietf:params:xml:ns:xcon-conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "codecs", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeCodecs(*d, s, f);
	return d;
}

void serializeConferencePassword(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferencePassword(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConferencePassword(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferencePassword(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferencePassword(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                                 ::xercesc::DOMErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferencePassword(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferencePassword(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferencePassword(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConferencePassword(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferencePassword(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferencePassword(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                                 ::xercesc::DOMErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferencePassword(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferencePassword(::xercesc::DOMDocument &d,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::String) == typeid(s)) {
		if (n.name() == "conference-password" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-password",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "conference-password", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConferencePassword(const ::LinphonePrivate::Xsd::XmlSchema::String &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::String) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("conference-password", "urn:ietf:params:xml:ns:xcon-conference-info",
		                                          m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "conference-password", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferencePassword(*d, s, f);
	return d;
}

void serializeControls(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeControls(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeControls(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &s,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeControls(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeControls(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &s,
                       ::xercesc::DOMErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeControls(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeControls(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeControls(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeControls(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &s,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeControls(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeControls(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &s,
                       ::xercesc::DOMErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeControls(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeControls(::xercesc::DOMDocument &d,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &s,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType) == typeid(s)) {
		if (n.name() == "controls" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "controls",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "controls", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeControls(const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &s,
                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("controls", "urn:ietf:params:xml:ns:xcon-conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "controls", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeControls(*d, s, f);
	return d;
}

void serializeLanguage(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeLanguage(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeLanguage(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &s,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeLanguage(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeLanguage(::std::ostream &o,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &s,
                       ::xercesc::DOMErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeLanguage(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeLanguage(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeLanguage(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeLanguage(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &s,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeLanguage(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeLanguage(::xercesc::XMLFormatTarget &t,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &s,
                       ::xercesc::DOMErrorHandler &h,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       const ::std::string &e,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeLanguage(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeLanguage(::xercesc::DOMDocument &d,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &s,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Language) == typeid(s)) {
		if (n.name() == "language" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "language",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "language", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeLanguage(const ::LinphonePrivate::Xsd::XmlSchema::Language &s,
                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Language) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("language", "urn:ietf:params:xml:ns:xcon-conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "language", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeLanguage(*d, s, f);
	return d;
}

void serializeAllowSidebars(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowSidebars(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowSidebars(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowSidebars(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowSidebars(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowSidebars(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowSidebars(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowSidebars(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowSidebars(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowSidebars(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowSidebars(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowSidebars(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowSidebars(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-sidebars" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-sidebars",
		                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowSidebars(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::serialize<char>("allow-sidebars", "urn:ietf:params:xml:ns:xcon-conference-info", m, f));

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowSidebars(*d, s, f);
	return d;
}

void serializeCloningParent(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCloningParent(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeCloningParent(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCloningParent(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCloningParent(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCloningParent(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCloningParent(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCloningParent(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeCloningParent(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCloningParent(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCloningParent(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeCloningParent(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCloningParent(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Uri) == typeid(s)) {
		if (n.name() == "cloning-parent" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "cloning-parent",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "cloning-parent", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCloningParent(const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Uri) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("cloning-parent", "urn:ietf:params:xml:ns:xcon-conference-info", m,
		                                          f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "cloning-parent", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeCloningParent(*d, s, f);
	return d;
}

void serializeSidebarParent(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeSidebarParent(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarParent(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeSidebarParent(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarParent(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeSidebarParent(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarParent(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeSidebarParent(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarParent(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeSidebarParent(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarParent(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeSidebarParent(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarParent(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Uri) == typeid(s)) {
		if (n.name() == "sidebar-parent" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebar-parent",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebar-parent", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarParent(const ::LinphonePrivate::Xsd::XmlSchema::Uri &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XmlSchema::Uri) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebar-parent", "urn:ietf:params:xml:ns:xcon-conference-info", m,
		                                          f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebar-parent", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeSidebarParent(*d, s, f);
	return d;
}

void serializeConferenceTime(::std::ostream &o,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &s,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceTime(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConferenceTime(::std::ostream &o,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &s,
                             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceTime(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceTime(::std::ostream &o,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &s,
                             ::xercesc::DOMErrorHandler &h,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceTime(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceTime(::xercesc::XMLFormatTarget &t,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &s,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceTime(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConferenceTime(::xercesc::XMLFormatTarget &t,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &s,
                             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceTime(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceTime(::xercesc::XMLFormatTarget &t,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &s,
                             ::xercesc::DOMErrorHandler &h,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             const ::std::string &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceTime(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConferenceTime(::xercesc::DOMDocument &d,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &s,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType) == typeid(s)) {
		if (n.name() == "conference-time" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "conference-time",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "conference-time", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConferenceTime(const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("conference-time", "urn:ietf:params:xml:ns:xcon-conference-info", m,
		                                          f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "conference-time", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeConferenceTime(*d, s, f);
	return d;
}

void serializeAllowConferenceEventSubscription(::std::ostream &o,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                               const ::std::string &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowConferenceEventSubscription(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowConferenceEventSubscription(::std::ostream &o,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                               const ::std::string &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowConferenceEventSubscription(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowConferenceEventSubscription(::std::ostream &o,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                               ::xercesc::DOMErrorHandler &h,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                               const ::std::string &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowConferenceEventSubscription(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowConferenceEventSubscription(::xercesc::XMLFormatTarget &t,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                               const ::std::string &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowConferenceEventSubscription(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowConferenceEventSubscription(::xercesc::XMLFormatTarget &t,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                               const ::std::string &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowConferenceEventSubscription(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowConferenceEventSubscription(::xercesc::XMLFormatTarget &t,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                               ::xercesc::DOMErrorHandler &h,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                               const ::std::string &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowConferenceEventSubscription(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowConferenceEventSubscription(::xercesc::DOMDocument &d,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-conference-event-subscription" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(),
		                                                 "allow-conference-event-subscription",
		                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowConferenceEventSubscription(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(::xsd::cxx::xml::dom::serialize<char>(
	    "allow-conference-event-subscription", "urn:ietf:params:xml:ns:xcon-conference-info", m, f));

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowConferenceEventSubscription(*d, s, f);
	return d;
}

void serializeToMixer(::std::ostream &o,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeToMixer(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeToMixer(::std::ostream &o,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeToMixer(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeToMixer(::std::ostream &o,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                      ::xercesc::DOMErrorHandler &h,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeToMixer(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeToMixer(::xercesc::XMLFormatTarget &t,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeToMixer(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeToMixer(::xercesc::XMLFormatTarget &t,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeToMixer(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeToMixer(::xercesc::XMLFormatTarget &t,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                      ::xercesc::DOMErrorHandler &h,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      const ::std::string &e,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeToMixer(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeToMixer(::xercesc::DOMDocument &d,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::MixerType) == typeid(s)) {
		if (n.name() == "to-mixer" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "to-mixer",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "to-mixer", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeToMixer(const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::MixerType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("to-mixer", "urn:ietf:params:xml:ns:xcon-conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "to-mixer", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeToMixer(*d, s, f);
	return d;
}

void serializeProvideAnonymity(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeProvideAnonymity(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeProvideAnonymity(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeProvideAnonymity(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProvideAnonymity(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeProvideAnonymity(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProvideAnonymity(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeProvideAnonymity(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeProvideAnonymity(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeProvideAnonymity(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProvideAnonymity(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeProvideAnonymity(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeProvideAnonymity(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType) == typeid(s)) {
		if (n.name() == "provide-anonymity" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "provide-anonymity",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "provide-anonymity", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeProvideAnonymity(const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("provide-anonymity", "urn:ietf:params:xml:ns:xcon-conference-info", m,
		                                          f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "provide-anonymity", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeProvideAnonymity(*d, s, f);
	return d;
}

void serializeAllowReferUsersDynamically(::std::ostream &o,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                         const ::std::string &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowReferUsersDynamically(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowReferUsersDynamically(::std::ostream &o,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                         const ::std::string &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowReferUsersDynamically(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowReferUsersDynamically(::std::ostream &o,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                         ::xercesc::DOMErrorHandler &h,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                         const ::std::string &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowReferUsersDynamically(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowReferUsersDynamically(::xercesc::XMLFormatTarget &t,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                         const ::std::string &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowReferUsersDynamically(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowReferUsersDynamically(::xercesc::XMLFormatTarget &t,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                         const ::std::string &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowReferUsersDynamically(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowReferUsersDynamically(::xercesc::XMLFormatTarget &t,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                         ::xercesc::DOMErrorHandler &h,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                         const ::std::string &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowReferUsersDynamically(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowReferUsersDynamically(::xercesc::DOMDocument &d,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-refer-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-refer-users-dynamically",
		                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowReferUsersDynamically(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(::xsd::cxx::xml::dom::serialize<char>(
	    "allow-refer-users-dynamically", "urn:ietf:params:xml:ns:xcon-conference-info", m, f));

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowReferUsersDynamically(*d, s, f);
	return d;
}

void serializeAllowInviteUsersDynamically(::std::ostream &o,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowInviteUsersDynamically(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowInviteUsersDynamically(::std::ostream &o,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowInviteUsersDynamically(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowInviteUsersDynamically(::std::ostream &o,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::xercesc::DOMErrorHandler &h,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowInviteUsersDynamically(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowInviteUsersDynamically(::xercesc::XMLFormatTarget &t,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowInviteUsersDynamically(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowInviteUsersDynamically(::xercesc::XMLFormatTarget &t,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowInviteUsersDynamically(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowInviteUsersDynamically(::xercesc::XMLFormatTarget &t,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::xercesc::DOMErrorHandler &h,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowInviteUsersDynamically(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowInviteUsersDynamically(::xercesc::DOMDocument &d,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-invite-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-invite-users-dynamically",
		                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowInviteUsersDynamically(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(::xsd::cxx::xml::dom::serialize<char>(
	    "allow-invite-users-dynamically", "urn:ietf:params:xml:ns:xcon-conference-info", m, f));

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowInviteUsersDynamically(*d, s, f);
	return d;
}

void serializeAllowRemoveUsersDynamically(::std::ostream &o,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowRemoveUsersDynamically(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowRemoveUsersDynamically(::std::ostream &o,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowRemoveUsersDynamically(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowRemoveUsersDynamically(::std::ostream &o,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::xercesc::DOMErrorHandler &h,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowRemoveUsersDynamically(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowRemoveUsersDynamically(::xercesc::XMLFormatTarget &t,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowRemoveUsersDynamically(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowRemoveUsersDynamically(::xercesc::XMLFormatTarget &t,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowRemoveUsersDynamically(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowRemoveUsersDynamically(::xercesc::XMLFormatTarget &t,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::xercesc::DOMErrorHandler &h,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                          const ::std::string &e,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowRemoveUsersDynamically(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowRemoveUsersDynamically(::xercesc::DOMDocument &d,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (n.name() == "allow-remove-users-dynamically" &&
	    n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
		e << s;
	} else {
		throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allow-remove-users-dynamically",
		                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowRemoveUsersDynamically(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &s,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(::xsd::cxx::xml::dom::serialize<char>(
	    "allow-remove-users-dynamically", "urn:ietf:params:xml:ns:xcon-conference-info", m, f));

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowRemoveUsersDynamically(*d, s, f);
	return d;
}

void serializeFromMixer(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFromMixer(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeFromMixer(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFromMixer(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFromMixer(::std::ostream &o,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFromMixer(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFromMixer(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFromMixer(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeFromMixer(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFromMixer(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFromMixer(::xercesc::XMLFormatTarget &t,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                        ::xercesc::DOMErrorHandler &h,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                        const ::std::string &e,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFromMixer(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFromMixer(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::MixerType) == typeid(s)) {
		if (n.name() == "from-mixer" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "from-mixer",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "from-mixer", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeFromMixer(const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &s,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::MixerType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("from-mixer", "urn:ietf:params:xml:ns:xcon-conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "from-mixer", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeFromMixer(*d, s, f);
	return d;
}

void serializeJoinHandling(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeJoinHandling(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeJoinHandling(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeJoinHandling(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeJoinHandling(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeJoinHandling(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeJoinHandling(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeJoinHandling(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeJoinHandling(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeJoinHandling(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeJoinHandling(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeJoinHandling(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeJoinHandling(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType) == typeid(s)) {
		if (n.name() == "join-handling" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "join-handling",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "join-handling", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeJoinHandling(const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("join-handling", "urn:ietf:params:xml:ns:xcon-conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "join-handling", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeJoinHandling(*d, s, f);
	return d;
}

void serializeUserAdmissionPolicy(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &s,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeUserAdmissionPolicy(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUserAdmissionPolicy(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeUserAdmissionPolicy(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserAdmissionPolicy(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &s,
                                  ::xercesc::DOMErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeUserAdmissionPolicy(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserAdmissionPolicy(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &s,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeUserAdmissionPolicy(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUserAdmissionPolicy(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeUserAdmissionPolicy(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserAdmissionPolicy(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &s,
                                  ::xercesc::DOMErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeUserAdmissionPolicy(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserAdmissionPolicy(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType) == typeid(s)) {
		if (n.name() == "user-admission-policy" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "user-admission-policy",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "user-admission-policy", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUserAdmissionPolicy(const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &s,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("user-admission-policy",
		                                          "urn:ietf:params:xml:ns:xcon-conference-info", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "user-admission-policy", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeUserAdmissionPolicy(*d, s, f);
	return d;
}

void serializeAllowedUsersList(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowedUsersList(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowedUsersList(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowedUsersList(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowedUsersList(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowedUsersList(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowedUsersList(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowedUsersList(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeAllowedUsersList(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowedUsersList(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowedUsersList(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowedUsersList(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeAllowedUsersList(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType) == typeid(s)) {
		if (n.name() == "allowed-users-list" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "allowed-users-list",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "allowed-users-list", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowedUsersList(const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("allowed-users-list", "urn:ietf:params:xml:ns:xcon-conference-info",
		                                          m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "allowed-users-list", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeAllowedUsersList(*d, s, f);
	return d;
}

void serializeDenyUsersList(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeDenyUsersList(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDenyUsersList(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeDenyUsersList(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDenyUsersList(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeDenyUsersList(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDenyUsersList(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeDenyUsersList(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeDenyUsersList(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeDenyUsersList(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDenyUsersList(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeDenyUsersList(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeDenyUsersList(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType) == typeid(s)) {
		if (n.name() == "deny-users-list" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "deny-users-list",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "deny-users-list", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDenyUsersList(const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("deny-users-list", "urn:ietf:params:xml:ns:xcon-conference-info", m,
		                                          f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "deny-users-list", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeDenyUsersList(*d, s, f);
	return d;
}

void serializeFloorInformation(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFloorInformation(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeFloorInformation(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFloorInformation(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFloorInformation(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFloorInformation(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFloorInformation(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFloorInformation(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeFloorInformation(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFloorInformation(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFloorInformation(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconConferenceInfo::serializeFloorInformation(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeFloorInformation(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType) == typeid(s)) {
		if (n.name() == "floor-information" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-conference-info") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "floor-information",
			                                                 "urn:ietf:params:xml:ns:xcon-conference-info");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "floor-information", "urn:ietf:params:xml:ns:xcon-conference-info", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeFloorInformation(const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("floor-information", "urn:ietf:params:xml:ns:xcon-conference-info", m,
		                                          f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "floor-information", "urn:ietf:params:xml:ns:xcon-conference-info", m, s, f);
	}

	::LinphonePrivate::Xsd::XconConferenceInfo::serializeFloorInformation(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const ConferenceTimeType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConferenceTimeType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// entry
	//
	for (ConferenceTimeType::EntryConstIterator b(i.getEntry().begin()), n(i.getEntry().end()); b != n; ++b) {
		const ConferenceTimeType::EntryType &x(*b);

		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("entry", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << x;
	}

	// any
	//
	for (ConferenceTimeType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConferenceTimeType>
    _xsd_ConferenceTimeType_type_serializer_init("conference-time-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const TimeType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::DateTime &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const TimeType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::DateTime &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const TimeType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::DateTime &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, TimeType>
    _xsd_TimeType_type_serializer_init("time-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const RoleType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const RoleType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const RoleType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, RoleType>
    _xsd_RoleType_type_serializer_init("role-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const MixingModeType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const MixingModeType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const MixingModeType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, MixingModeType>
    _xsd_MixingModeType_type_serializer_init("mixing-mode-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const CodecsType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (CodecsType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// codec
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CodecsType::CodecType &x(i.getCodec());
		if (typeid(CodecsType::CodecType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("codec", "urn:ietf:params:xml:ns:xcon-conference-info", e));

			s << x;
		} else tsm.serialize("codec", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
	}

	// any
	//
	for (CodecsType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// decision
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("decision", e));

		a << i.getDecision();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CodecsType>
    _xsd_CodecsType_type_serializer_init("codecs-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const CodecType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (CodecType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// subtype
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSubtype()) {
			const CodecType::SubtypeType &x(*i.getSubtype());
			if (typeid(CodecType::SubtypeType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("subtype", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("subtype", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (CodecType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// name
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("name", e));

		a << i.getName();
	}

	// policy
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("policy", e));

		a << i.getPolicy();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CodecType>
    _xsd_CodecType_type_serializer_init("codec-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const DecisionType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const DecisionType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const DecisionType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, DecisionType>
    _xsd_DecisionType_type_serializer_init("decision-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const PolicyType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const PolicyType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const PolicyType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, PolicyType>
    _xsd_PolicyType_type_serializer_init("policy-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const ControlsType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ControlsType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// mute
	//
	if (i.getMute()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("mute", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getMute();
	}

	// pause-video
	//
	if (i.getPauseVideo()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("pause-video", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getPauseVideo();
	}

	// gain
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getGain()) {
			const ControlsType::GainType &x(*i.getGain());
			if (typeid(ControlsType::GainType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("gain", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("gain", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// video-layout
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getVideoLayout()) {
			const ControlsType::VideoLayoutType &x(*i.getVideoLayout());
			if (typeid(ControlsType::VideoLayoutType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "video-layout", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("video-layout", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (ControlsType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ControlsType>
    _xsd_ControlsType_type_serializer_init("controls-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const GainType &i) {
	e << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const GainType &i) {
	a << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const GainType &i) {
	l << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, GainType>
    _xsd_GainType_type_serializer_init("gain-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const VideoLayoutType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const VideoLayoutType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const VideoLayoutType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, VideoLayoutType>
    _xsd_VideoLayoutType_type_serializer_init("video-layout-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const FloorInformationType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (FloorInformationType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// conference-ID
	//
	if (i.getConferenceID()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("conference-ID", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getConferenceID();
	}

	// allow-floor-events
	//
	if (i.getAllowFloorEvents()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
		    "allow-floor-events", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getAllowFloorEvents();
	}

	// floor-request-handling
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getFloorRequestHandling()) {
			const FloorInformationType::FloorRequestHandlingType &x(*i.getFloorRequestHandling());
			if (typeid(FloorInformationType::FloorRequestHandlingType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "floor-request-handling", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else
				tsm.serialize("floor-request-handling", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e,
				              x);
		}
	}

	// conference-floor-policy
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConferenceFloorPolicy()) {
			const FloorInformationType::ConferenceFloorPolicyType &x(*i.getConferenceFloorPolicy());
			if (typeid(FloorInformationType::ConferenceFloorPolicyType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "conference-floor-policy", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else
				tsm.serialize("conference-floor-policy", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e,
				              x);
		}
	}

	// any
	//
	for (FloorInformationType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, FloorInformationType>
    _xsd_FloorInformationType_type_serializer_init("floor-information-type",
                                                   "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const FloorRequestHandlingType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const FloorRequestHandlingType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const FloorRequestHandlingType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, FloorRequestHandlingType>
    _xsd_FloorRequestHandlingType_type_serializer_init("floor-request-handling-type",
                                                       "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const ConferenceFloorPolicy &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConferenceFloorPolicy::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// floor
	//
	for (ConferenceFloorPolicy::FloorConstIterator b(i.getFloor().begin()), n(i.getFloor().end()); b != n; ++b) {
		const ConferenceFloorPolicy::FloorType &x(*b);

		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("floor", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << x;
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConferenceFloorPolicy>
    _xsd_ConferenceFloorPolicy_type_serializer_init("conference-floor-policy",
                                                    "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const AlgorithmType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const AlgorithmType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const AlgorithmType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, AlgorithmType>
    _xsd_AlgorithmType_type_serializer_init("algorithm-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const UserAdmissionPolicyType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const UserAdmissionPolicyType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const UserAdmissionPolicyType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UserAdmissionPolicyType>
    _xsd_UserAdmissionPolicyType_type_serializer_init("user-admission-policy-type",
                                                      "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const JoinHandlingType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const JoinHandlingType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const JoinHandlingType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, JoinHandlingType>
    _xsd_JoinHandlingType_type_serializer_init("join-handling-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const DenyUsersListType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (DenyUsersListType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// target
	//
	for (DenyUsersListType::TargetConstIterator b(i.getTarget().begin()), n(i.getTarget().end()); b != n; ++b) {
		const DenyUsersListType::TargetType &x(*b);

		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("target", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << x;
	}

	// any
	//
	for (DenyUsersListType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, DenyUsersListType>
    _xsd_DenyUsersListType_type_serializer_init("deny-users-list-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const AllowedUsersListType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (AllowedUsersListType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// target
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (AllowedUsersListType::TargetConstIterator b(i.getTarget().begin()), n(i.getTarget().end()); b != n; ++b) {
			const AllowedUsersListType::TargetType &x(*b);

			if (typeid(AllowedUsersListType::TargetType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("target", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("target", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// persistent-list
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getPersistentList()) {
			const AllowedUsersListType::PersistentListType &x(*i.getPersistentList());
			if (typeid(AllowedUsersListType::PersistentListType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "persistent-list", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("persistent-list", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (AllowedUsersListType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, AllowedUsersListType>
    _xsd_AllowedUsersListType_type_serializer_init("allowed-users-list-type",
                                                   "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const PersistentListType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (PersistentListType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// user
	//
	for (PersistentListType::UserConstIterator b(i.getUser().begin()), n(i.getUser().end()); b != n; ++b) {
		const PersistentListType::UserType &x(*b);

		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("user", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << x;
	}

	// any
	//
	for (PersistentListType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, PersistentListType>
    _xsd_PersistentListType_type_serializer_init("persistent-list-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const TargetType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (TargetType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// uri
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("uri", e));

		a << i.getUri();
	}

	// method
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("method", e));

		a << i.getMethod();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, TargetType>
    _xsd_TargetType_type_serializer_init("target-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const MethodType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const MethodType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const MethodType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, MethodType>
    _xsd_MethodType_type_serializer_init("method-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const ProvideAnonymityType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const ProvideAnonymityType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const ProvideAnonymityType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ProvideAnonymityType>
    _xsd_ProvideAnonymityType_type_serializer_init("provide-anonymity-type",
                                                   "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const MixerType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (MixerType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// floor
	//
	{
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("floor", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << i.getFloor();
	}

	// controls
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (MixerType::ControlsConstIterator b(i.getControls().begin()), n(i.getControls().end()); b != n; ++b) {
			const MixerType::ControlsType &x(*b);

			if (typeid(MixerType::ControlsType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("controls", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("controls", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (MixerType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// name
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("name", e));

		a << i.getName();
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, MixerType>
    _xsd_MixerType_type_serializer_init("mixer-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const MixerNameType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const MixerNameType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const MixerNameType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, MixerNameType>
    _xsd_MixerNameType_type_serializer_init("mixer-name-type", "urn:ietf:params:xml:ns:xcon-conference-info");

void operator<<(::xercesc::DOMElement &e, const ConferenceInfoDiff &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConferenceInfoDiff::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// add
	//
	for (ConferenceInfoDiff::AddConstIterator b(i.getAdd().begin()), n(i.getAdd().end()); b != n; ++b) {
		const ConferenceInfoDiff::AddType &x(*b);

		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("add", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << x;
	}

	// remove
	//
	for (ConferenceInfoDiff::RemoveConstIterator b(i.getRemove().begin()), n(i.getRemove().end()); b != n; ++b) {
		const ConferenceInfoDiff::RemoveType &x(*b);

		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("remove", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << x;
	}

	// replace
	//
	for (ConferenceInfoDiff::ReplaceConstIterator b(i.getReplace().begin()), n(i.getReplace().end()); b != n; ++b) {
		const ConferenceInfoDiff::ReplaceType &x(*b);

		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("replace", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << x;
	}

	// any
	//
	for (ConferenceInfoDiff::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// entity
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("entity", e));

		a << i.getEntity();
	}
}

void operator<<(::xercesc::DOMElement &e, const Entry &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// base
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const Entry::BaseType &x(i.getBase());
		if (typeid(Entry::BaseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("base", "urn:ietf:params:xml:ns:xcon-conference-info", e));

			s << x;
		} else tsm.serialize("base", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
	}

	// mixing-start-offset
	//
	if (i.getMixingStartOffset()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
		    "mixing-start-offset", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getMixingStartOffset();
	}

	// mixing-end-offset
	//
	if (i.getMixingEndOffset()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
		    "mixing-end-offset", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getMixingEndOffset();
	}

	// can-join-after-offset
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getCan_join_after_offset()) {
			const Entry::CanJoinAfterOffsetType &x(*i.getCan_join_after_offset());
			if (typeid(Entry::CanJoinAfterOffsetType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "can-join-after-offset", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else
				tsm.serialize("can-join-after-offset", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e,
				              x);
		}
	}

	// must-join-before-offset
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getMust_join_before_offset()) {
			const Entry::MustJoinBeforeOffsetType &x(*i.getMust_join_before_offset());
			if (typeid(Entry::MustJoinBeforeOffsetType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "must-join-before-offset", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else
				tsm.serialize("must-join-before-offset", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e,
				              x);
		}
	}

	// request-user
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getRequestUser()) {
			const Entry::RequestUserType &x(*i.getRequestUser());
			if (typeid(Entry::RequestUserType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "request-user", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("request-user", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// notify-end-of-conference
	//
	if (i.getNotify_end_of_conference()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
		    "notify-end-of-conference", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getNotify_end_of_conference();
	}

	// allowed-extend-mixing-end-offset
	//
	if (i.getAllowed_extend_mixing_end_offset()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
		    "allowed-extend-mixing-end-offset", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getAllowed_extend_mixing_end_offset();
	}

	// any
	//
	for (Entry::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

void operator<<(::xercesc::DOMElement &e, const Floor &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (Floor::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// media-label
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (Floor::MediaLabelConstIterator b(i.getMediaLabel().begin()), n(i.getMediaLabel().end()); b != n; ++b) {
			const Floor::MediaLabelType &x(*b);

			if (typeid(Floor::MediaLabelType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "media-label", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("media-label", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// algorithm
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getAlgorithm()) {
			const Floor::AlgorithmType &x(*i.getAlgorithm());
			if (typeid(Floor::AlgorithmType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element(
				    "algorithm", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("algorithm", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// max-floor-users
	//
	if (i.getMaxFloorUsers()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("max-floor-users", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getMaxFloorUsers();
	}

	// moderator-id
	//
	if (i.getModeratorId()) {
		::xercesc::DOMElement &s(
		    ::xsd::cxx::xml::dom::create_element("moderator-id", "urn:ietf:params:xml:ns:xcon-conference-info", e));

		s << *i.getModeratorId();
	}

	// any
	//
	for (Floor::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// id
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("id", e));

		a << i.getId();
	}
}

void operator<<(::xercesc::DOMElement &e, const Target &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (Target::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// uri
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("uri", e));

		a << i.getUri();
	}
}

void operator<<(::xercesc::DOMElement &e, const User &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (User::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// email
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (User::EmailConstIterator b(i.getEmail().begin()), n(i.getEmail().end()); b != n; ++b) {
			const User::EmailType &x(*b);

			if (typeid(User::EmailType) == typeid(x)) {
				::xercesc::DOMElement &s(
				    ::xsd::cxx::xml::dom::create_element("email", "urn:ietf:params:xml:ns:xcon-conference-info", e));

				s << x;
			} else tsm.serialize("email", "urn:ietf:params:xml:ns:xcon-conference-info", false, true, e, x);
		}
	}

	// any
	//
	for (User::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}

	// name
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("name", e));

		a << i.getName();
	}

	// nickname
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("nickname", e));

		a << i.getNickname();
	}

	// id
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("id", e));

		a << i.getId();
	}
}

void operator<<(::xercesc::DOMElement &e, const Floor1 &i) {
	e << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Boolean, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);

	// any_attribute
	//
	for (Floor1::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// id
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("id", e));

		a << i.getId();
	}
}

void operator<<(::xercesc::DOMElement &e, const Add1 &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::Add &>(i);

	// any_attribute
	//
	for (Add1::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}
}

void operator<<(::xercesc::DOMElement &e, const Remove1 &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::Remove &>(i);

	// any_attribute
	//
	for (Remove1::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}
}

void operator<<(::xercesc::DOMElement &e, const Replace1 &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::Replace &>(i);

	// any_attribute
	//
	for (Replace1::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}
}

void operator<<(::xercesc::DOMElement &e, const MixingStartOffset &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType &>(i);

	// any_attribute
	//
	for (MixingStartOffset::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// required-participant
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("required-participant", e));

		a << i.getRequiredParticipant();
	}
}

void operator<<(::xercesc::DOMElement &e, const MixingEndOffset &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType &>(i);

	// any_attribute
	//
	for (MixingEndOffset::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// required-participant
	//
	{
		::xercesc::DOMAttr &a(::xsd::cxx::xml::dom::create_attribute("required-participant", e));

		a << i.getRequiredParticipant();
	}
}
} // namespace XconConferenceInfo
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
