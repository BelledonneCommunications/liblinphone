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

#include "xcon-ccmp.h"

namespace LinphonePrivate {
namespace Xsd {
namespace XconCcmp {
// CcmpRequestType
//

const CcmpRequestType::CcmpRequestType1 &CcmpRequestType::getCcmpRequest() const {
	return this->ccmpRequest_.get();
}

CcmpRequestType::CcmpRequestType1 &CcmpRequestType::getCcmpRequest() {
	return this->ccmpRequest_.get();
}

void CcmpRequestType::setCcmpRequest(const CcmpRequestType1 &x) {
	this->ccmpRequest_.set(x);
}

void CcmpRequestType::setCcmpRequest(::std::unique_ptr<CcmpRequestType1> x) {
	this->ccmpRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpRequestType::CcmpRequestType1> CcmpRequestType::setDetachCcmpRequest() {
	return this->ccmpRequest_.detach();
}

// CcmpRequestMessageType
//

const CcmpRequestMessageType::SubjectOptional &CcmpRequestMessageType::getSubject() const {
	return this->subject_;
}

CcmpRequestMessageType::SubjectOptional &CcmpRequestMessageType::getSubject() {
	return this->subject_;
}

void CcmpRequestMessageType::setSubject(const SubjectType &x) {
	this->subject_.set(x);
}

void CcmpRequestMessageType::setSubject(const SubjectOptional &x) {
	this->subject_ = x;
}

void CcmpRequestMessageType::setSubject(::std::unique_ptr<SubjectType> x) {
	this->subject_.set(std::move(x));
}

const CcmpRequestMessageType::ConfUserIDOptional &CcmpRequestMessageType::getConfUserID() const {
	return this->confUserID_;
}

CcmpRequestMessageType::ConfUserIDOptional &CcmpRequestMessageType::getConfUserID() {
	return this->confUserID_;
}

void CcmpRequestMessageType::setConfUserID(const ConfUserIDType &x) {
	this->confUserID_.set(x);
}

void CcmpRequestMessageType::setConfUserID(const ConfUserIDOptional &x) {
	this->confUserID_ = x;
}

void CcmpRequestMessageType::setConfUserID(::std::unique_ptr<ConfUserIDType> x) {
	this->confUserID_.set(std::move(x));
}

const CcmpRequestMessageType::ConfObjIDOptional &CcmpRequestMessageType::getConfObjID() const {
	return this->confObjID_;
}

CcmpRequestMessageType::ConfObjIDOptional &CcmpRequestMessageType::getConfObjID() {
	return this->confObjID_;
}

void CcmpRequestMessageType::setConfObjID(const ConfObjIDType &x) {
	this->confObjID_.set(x);
}

void CcmpRequestMessageType::setConfObjID(const ConfObjIDOptional &x) {
	this->confObjID_ = x;
}

void CcmpRequestMessageType::setConfObjID(::std::unique_ptr<ConfObjIDType> x) {
	this->confObjID_.set(std::move(x));
}

const CcmpRequestMessageType::OperationOptional &CcmpRequestMessageType::getOperation() const {
	return this->operation_;
}

CcmpRequestMessageType::OperationOptional &CcmpRequestMessageType::getOperation() {
	return this->operation_;
}

void CcmpRequestMessageType::setOperation(const OperationType &x) {
	this->operation_.set(x);
}

void CcmpRequestMessageType::setOperation(const OperationOptional &x) {
	this->operation_ = x;
}

void CcmpRequestMessageType::setOperation(::std::unique_ptr<OperationType> x) {
	this->operation_.set(std::move(x));
}

const CcmpRequestMessageType::ConferencePasswordOptional &CcmpRequestMessageType::getConferencePassword() const {
	return this->conference_password_;
}

CcmpRequestMessageType::ConferencePasswordOptional &CcmpRequestMessageType::getConferencePassword() {
	return this->conference_password_;
}

void CcmpRequestMessageType::setConferencePassword(const ConferencePasswordType &x) {
	this->conference_password_.set(x);
}

void CcmpRequestMessageType::setConferencePassword(const ConferencePasswordOptional &x) {
	this->conference_password_ = x;
}

void CcmpRequestMessageType::setConferencePassword(::std::unique_ptr<ConferencePasswordType> x) {
	this->conference_password_.set(std::move(x));
}

const CcmpRequestMessageType::AnySequence &CcmpRequestMessageType::getAny() const {
	return this->any_;
}

CcmpRequestMessageType::AnySequence &CcmpRequestMessageType::getAny() {
	return this->any_;
}

void CcmpRequestMessageType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const CcmpRequestMessageType::AnyAttributeSet &CcmpRequestMessageType::getAnyAttribute() const {
	return this->any_attribute_;
}

CcmpRequestMessageType::AnyAttributeSet &CcmpRequestMessageType::getAnyAttribute() {
	return this->any_attribute_;
}

void CcmpRequestMessageType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &CcmpRequestMessageType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &CcmpRequestMessageType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpResponseType
//

const CcmpResponseType::CcmpResponseType1 &CcmpResponseType::getCcmpResponse() const {
	return this->ccmpResponse_.get();
}

CcmpResponseType::CcmpResponseType1 &CcmpResponseType::getCcmpResponse() {
	return this->ccmpResponse_.get();
}

void CcmpResponseType::setCcmpResponse(const CcmpResponseType1 &x) {
	this->ccmpResponse_.set(x);
}

void CcmpResponseType::setCcmpResponse(::std::unique_ptr<CcmpResponseType1> x) {
	this->ccmpResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpResponseType::CcmpResponseType1> CcmpResponseType::setDetachCcmpResponse() {
	return this->ccmpResponse_.detach();
}

// CcmpResponseMessageType
//

const CcmpResponseMessageType::ConfUserIDType &CcmpResponseMessageType::getConfUserID() const {
	return this->confUserID_.get();
}

CcmpResponseMessageType::ConfUserIDType &CcmpResponseMessageType::getConfUserID() {
	return this->confUserID_.get();
}

void CcmpResponseMessageType::setConfUserID(const ConfUserIDType &x) {
	this->confUserID_.set(x);
}

void CcmpResponseMessageType::setConfUserID(::std::unique_ptr<ConfUserIDType> x) {
	this->confUserID_.set(std::move(x));
}

::std::unique_ptr<CcmpResponseMessageType::ConfUserIDType> CcmpResponseMessageType::setDetachConfUserID() {
	return this->confUserID_.detach();
}

const CcmpResponseMessageType::ConfObjIDOptional &CcmpResponseMessageType::getConfObjID() const {
	return this->confObjID_;
}

CcmpResponseMessageType::ConfObjIDOptional &CcmpResponseMessageType::getConfObjID() {
	return this->confObjID_;
}

void CcmpResponseMessageType::setConfObjID(const ConfObjIDType &x) {
	this->confObjID_.set(x);
}

void CcmpResponseMessageType::setConfObjID(const ConfObjIDOptional &x) {
	this->confObjID_ = x;
}

void CcmpResponseMessageType::setConfObjID(::std::unique_ptr<ConfObjIDType> x) {
	this->confObjID_.set(std::move(x));
}

const CcmpResponseMessageType::OperationOptional &CcmpResponseMessageType::getOperation() const {
	return this->operation_;
}

CcmpResponseMessageType::OperationOptional &CcmpResponseMessageType::getOperation() {
	return this->operation_;
}

void CcmpResponseMessageType::setOperation(const OperationType &x) {
	this->operation_.set(x);
}

void CcmpResponseMessageType::setOperation(const OperationOptional &x) {
	this->operation_ = x;
}

void CcmpResponseMessageType::setOperation(::std::unique_ptr<OperationType> x) {
	this->operation_.set(std::move(x));
}

const CcmpResponseMessageType::ResponseCodeType &CcmpResponseMessageType::getResponseCode() const {
	return this->response_code_.get();
}

CcmpResponseMessageType::ResponseCodeType &CcmpResponseMessageType::getResponseCode() {
	return this->response_code_.get();
}

void CcmpResponseMessageType::setResponseCode(const ResponseCodeType &x) {
	this->response_code_.set(x);
}

void CcmpResponseMessageType::setResponseCode(::std::unique_ptr<ResponseCodeType> x) {
	this->response_code_.set(std::move(x));
}

::std::unique_ptr<CcmpResponseMessageType::ResponseCodeType> CcmpResponseMessageType::setDetachResponse_code() {
	return this->response_code_.detach();
}

const CcmpResponseMessageType::ResponseStringOptional &CcmpResponseMessageType::getResponseString() const {
	return this->response_string_;
}

CcmpResponseMessageType::ResponseStringOptional &CcmpResponseMessageType::getResponseString() {
	return this->response_string_;
}

void CcmpResponseMessageType::setResponseString(const ResponseStringType &x) {
	this->response_string_.set(x);
}

void CcmpResponseMessageType::setResponseString(const ResponseStringOptional &x) {
	this->response_string_ = x;
}

void CcmpResponseMessageType::setResponseString(::std::unique_ptr<ResponseStringType> x) {
	this->response_string_.set(std::move(x));
}

const CcmpResponseMessageType::VersionOptional &CcmpResponseMessageType::getVersion() const {
	return this->version_;
}

CcmpResponseMessageType::VersionOptional &CcmpResponseMessageType::getVersion() {
	return this->version_;
}

void CcmpResponseMessageType::setVersion(const VersionType &x) {
	this->version_.set(x);
}

void CcmpResponseMessageType::setVersion(const VersionOptional &x) {
	this->version_ = x;
}

const CcmpResponseMessageType::AnySequence &CcmpResponseMessageType::getAny() const {
	return this->any_;
}

CcmpResponseMessageType::AnySequence &CcmpResponseMessageType::getAny() {
	return this->any_;
}

void CcmpResponseMessageType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const CcmpResponseMessageType::AnyAttributeSet &CcmpResponseMessageType::getAnyAttribute() const {
	return this->any_attribute_;
}

CcmpResponseMessageType::AnyAttributeSet &CcmpResponseMessageType::getAnyAttribute() {
	return this->any_attribute_;
}

void CcmpResponseMessageType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &CcmpResponseMessageType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &CcmpResponseMessageType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpBlueprintsRequestMessageType
//

const CcmpBlueprintsRequestMessageType::BlueprintsRequestType &
CcmpBlueprintsRequestMessageType::getBlueprintsRequest() const {
	return this->blueprintsRequest_.get();
}

CcmpBlueprintsRequestMessageType::BlueprintsRequestType &CcmpBlueprintsRequestMessageType::getBlueprintsRequest() {
	return this->blueprintsRequest_.get();
}

void CcmpBlueprintsRequestMessageType::setBlueprintsRequest(const BlueprintsRequestType &x) {
	this->blueprintsRequest_.set(x);
}

void CcmpBlueprintsRequestMessageType::setBlueprintsRequest(::std::unique_ptr<BlueprintsRequestType> x) {
	this->blueprintsRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpBlueprintsRequestMessageType::BlueprintsRequestType>
CcmpBlueprintsRequestMessageType::setDetachBlueprintsRequest() {
	return this->blueprintsRequest_.detach();
}

// BlueprintsRequestType
//

const BlueprintsRequestType::XpathFilterOptional &BlueprintsRequestType::getXpathFilter() const {
	return this->xpathFilter_;
}

BlueprintsRequestType::XpathFilterOptional &BlueprintsRequestType::getXpathFilter() {
	return this->xpathFilter_;
}

void BlueprintsRequestType::setXpathFilter(const XpathFilterType &x) {
	this->xpathFilter_.set(x);
}

void BlueprintsRequestType::setXpathFilter(const XpathFilterOptional &x) {
	this->xpathFilter_ = x;
}

void BlueprintsRequestType::setXpathFilter(::std::unique_ptr<XpathFilterType> x) {
	this->xpathFilter_.set(std::move(x));
}

const BlueprintsRequestType::AnySequence &BlueprintsRequestType::getAny() const {
	return this->any_;
}

BlueprintsRequestType::AnySequence &BlueprintsRequestType::getAny() {
	return this->any_;
}

void BlueprintsRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const BlueprintsRequestType::AnyAttributeSet &BlueprintsRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

BlueprintsRequestType::AnyAttributeSet &BlueprintsRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void BlueprintsRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &BlueprintsRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &BlueprintsRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpBlueprintRequestMessageType
//

const CcmpBlueprintRequestMessageType::BlueprintRequestType &
CcmpBlueprintRequestMessageType::getBlueprintRequest() const {
	return this->blueprintRequest_.get();
}

CcmpBlueprintRequestMessageType::BlueprintRequestType &CcmpBlueprintRequestMessageType::getBlueprintRequest() {
	return this->blueprintRequest_.get();
}

void CcmpBlueprintRequestMessageType::setBlueprintRequest(const BlueprintRequestType &x) {
	this->blueprintRequest_.set(x);
}

void CcmpBlueprintRequestMessageType::setBlueprintRequest(::std::unique_ptr<BlueprintRequestType> x) {
	this->blueprintRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpBlueprintRequestMessageType::BlueprintRequestType>
CcmpBlueprintRequestMessageType::setDetachBlueprintRequest() {
	return this->blueprintRequest_.detach();
}

// BlueprintRequestType
//

const BlueprintRequestType::BlueprintInfoOptional &BlueprintRequestType::getBlueprintInfo() const {
	return this->blueprintInfo_;
}

BlueprintRequestType::BlueprintInfoOptional &BlueprintRequestType::getBlueprintInfo() {
	return this->blueprintInfo_;
}

void BlueprintRequestType::setBlueprintInfo(const BlueprintInfoType &x) {
	this->blueprintInfo_.set(x);
}

void BlueprintRequestType::setBlueprintInfo(const BlueprintInfoOptional &x) {
	this->blueprintInfo_ = x;
}

void BlueprintRequestType::setBlueprintInfo(::std::unique_ptr<BlueprintInfoType> x) {
	this->blueprintInfo_.set(std::move(x));
}

const BlueprintRequestType::AnySequence &BlueprintRequestType::getAny() const {
	return this->any_;
}

BlueprintRequestType::AnySequence &BlueprintRequestType::getAny() {
	return this->any_;
}

void BlueprintRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const BlueprintRequestType::AnyAttributeSet &BlueprintRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

BlueprintRequestType::AnyAttributeSet &BlueprintRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void BlueprintRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &BlueprintRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &BlueprintRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpConfsRequestMessageType
//

const CcmpConfsRequestMessageType::ConfsRequestType &CcmpConfsRequestMessageType::getConfsRequest() const {
	return this->confsRequest_.get();
}

CcmpConfsRequestMessageType::ConfsRequestType &CcmpConfsRequestMessageType::getConfsRequest() {
	return this->confsRequest_.get();
}

void CcmpConfsRequestMessageType::setConfsRequest(const ConfsRequestType &x) {
	this->confsRequest_.set(x);
}

void CcmpConfsRequestMessageType::setConfsRequest(::std::unique_ptr<ConfsRequestType> x) {
	this->confsRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpConfsRequestMessageType::ConfsRequestType> CcmpConfsRequestMessageType::setDetachConfsRequest() {
	return this->confsRequest_.detach();
}

// ConfsRequestType
//

const ConfsRequestType::XpathFilterOptional &ConfsRequestType::getXpathFilter() const {
	return this->xpathFilter_;
}

ConfsRequestType::XpathFilterOptional &ConfsRequestType::getXpathFilter() {
	return this->xpathFilter_;
}

void ConfsRequestType::setXpathFilter(const XpathFilterType &x) {
	this->xpathFilter_.set(x);
}

void ConfsRequestType::setXpathFilter(const XpathFilterOptional &x) {
	this->xpathFilter_ = x;
}

void ConfsRequestType::setXpathFilter(::std::unique_ptr<XpathFilterType> x) {
	this->xpathFilter_.set(std::move(x));
}

const ConfsRequestType::AnySequence &ConfsRequestType::getAny() const {
	return this->any_;
}

ConfsRequestType::AnySequence &ConfsRequestType::getAny() {
	return this->any_;
}

void ConfsRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConfsRequestType::AnyAttributeSet &ConfsRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConfsRequestType::AnyAttributeSet &ConfsRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConfsRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConfsRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConfsRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpConfRequestMessageType
//

const CcmpConfRequestMessageType::ConfRequestType &CcmpConfRequestMessageType::getConfRequest() const {
	return this->confRequest_.get();
}

CcmpConfRequestMessageType::ConfRequestType &CcmpConfRequestMessageType::getConfRequest() {
	return this->confRequest_.get();
}

void CcmpConfRequestMessageType::setConfRequest(const ConfRequestType &x) {
	this->confRequest_.set(x);
}

void CcmpConfRequestMessageType::setConfRequest(::std::unique_ptr<ConfRequestType> x) {
	this->confRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpConfRequestMessageType::ConfRequestType> CcmpConfRequestMessageType::setDetachConfRequest() {
	return this->confRequest_.detach();
}

// ConfRequestType
//

const ConfRequestType::ConfInfoOptional &ConfRequestType::getConfInfo() const {
	return this->confInfo_;
}

ConfRequestType::ConfInfoOptional &ConfRequestType::getConfInfo() {
	return this->confInfo_;
}

void ConfRequestType::setConfInfo(const ConfInfoType &x) {
	this->confInfo_.set(x);
}

void ConfRequestType::setConfInfo(const ConfInfoOptional &x) {
	this->confInfo_ = x;
}

void ConfRequestType::setConfInfo(::std::unique_ptr<ConfInfoType> x) {
	this->confInfo_.set(std::move(x));
}

const ConfRequestType::AnySequence &ConfRequestType::getAny() const {
	return this->any_;
}

ConfRequestType::AnySequence &ConfRequestType::getAny() {
	return this->any_;
}

void ConfRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConfRequestType::AnyAttributeSet &ConfRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConfRequestType::AnyAttributeSet &ConfRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConfRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConfRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConfRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpUsersRequestMessageType
//

const CcmpUsersRequestMessageType::UsersRequestType &CcmpUsersRequestMessageType::getUsersRequest() const {
	return this->usersRequest_.get();
}

CcmpUsersRequestMessageType::UsersRequestType &CcmpUsersRequestMessageType::getUsersRequest() {
	return this->usersRequest_.get();
}

void CcmpUsersRequestMessageType::setUsersRequest(const UsersRequestType &x) {
	this->usersRequest_.set(x);
}

void CcmpUsersRequestMessageType::setUsersRequest(::std::unique_ptr<UsersRequestType> x) {
	this->usersRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpUsersRequestMessageType::UsersRequestType> CcmpUsersRequestMessageType::setDetachUsersRequest() {
	return this->usersRequest_.detach();
}

// UsersRequestType
//

const UsersRequestType::UsersInfoOptional &UsersRequestType::getUsersInfo() const {
	return this->usersInfo_;
}

UsersRequestType::UsersInfoOptional &UsersRequestType::getUsersInfo() {
	return this->usersInfo_;
}

void UsersRequestType::setUsersInfo(const UsersInfoType &x) {
	this->usersInfo_.set(x);
}

void UsersRequestType::setUsersInfo(const UsersInfoOptional &x) {
	this->usersInfo_ = x;
}

void UsersRequestType::setUsersInfo(::std::unique_ptr<UsersInfoType> x) {
	this->usersInfo_.set(std::move(x));
}

const UsersRequestType::AnySequence &UsersRequestType::getAny() const {
	return this->any_;
}

UsersRequestType::AnySequence &UsersRequestType::getAny() {
	return this->any_;
}

void UsersRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const UsersRequestType::AnyAttributeSet &UsersRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

UsersRequestType::AnyAttributeSet &UsersRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void UsersRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UsersRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UsersRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpUserRequestMessageType
//

const CcmpUserRequestMessageType::UserRequestType &CcmpUserRequestMessageType::getUserRequest() const {
	return this->userRequest_.get();
}

CcmpUserRequestMessageType::UserRequestType &CcmpUserRequestMessageType::getUserRequest() {
	return this->userRequest_.get();
}

void CcmpUserRequestMessageType::setUserRequest(const UserRequestType &x) {
	this->userRequest_.set(x);
}

void CcmpUserRequestMessageType::setUserRequest(::std::unique_ptr<UserRequestType> x) {
	this->userRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpUserRequestMessageType::UserRequestType> CcmpUserRequestMessageType::setDetachUserRequest() {
	return this->userRequest_.detach();
}

// UserRequestType
//

const UserRequestType::UserInfoOptional &UserRequestType::getUserInfo() const {
	return this->userInfo_;
}

UserRequestType::UserInfoOptional &UserRequestType::getUserInfo() {
	return this->userInfo_;
}

void UserRequestType::setUserInfo(const UserInfoType &x) {
	this->userInfo_.set(x);
}

void UserRequestType::setUserInfo(const UserInfoOptional &x) {
	this->userInfo_ = x;
}

void UserRequestType::setUserInfo(::std::unique_ptr<UserInfoType> x) {
	this->userInfo_.set(std::move(x));
}

const UserRequestType::AnySequence &UserRequestType::getAny() const {
	return this->any_;
}

UserRequestType::AnySequence &UserRequestType::getAny() {
	return this->any_;
}

void UserRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const UserRequestType::AnyAttributeSet &UserRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

UserRequestType::AnyAttributeSet &UserRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void UserRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UserRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UserRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpSidebarsByValRequestMessageType
//

const CcmpSidebarsByValRequestMessageType::SidebarsByValRequestType &
CcmpSidebarsByValRequestMessageType::getSidebarsByValRequest() const {
	return this->sidebarsByValRequest_.get();
}

CcmpSidebarsByValRequestMessageType::SidebarsByValRequestType &
CcmpSidebarsByValRequestMessageType::getSidebarsByValRequest() {
	return this->sidebarsByValRequest_.get();
}

void CcmpSidebarsByValRequestMessageType::setSidebarsByValRequest(const SidebarsByValRequestType &x) {
	this->sidebarsByValRequest_.set(x);
}

void CcmpSidebarsByValRequestMessageType::setSidebarsByValRequest(::std::unique_ptr<SidebarsByValRequestType> x) {
	this->sidebarsByValRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpSidebarsByValRequestMessageType::SidebarsByValRequestType>
CcmpSidebarsByValRequestMessageType::setDetachSidebarsByValRequest() {
	return this->sidebarsByValRequest_.detach();
}

// SidebarsByValRequestType
//

const SidebarsByValRequestType::XpathFilterOptional &SidebarsByValRequestType::getXpathFilter() const {
	return this->xpathFilter_;
}

SidebarsByValRequestType::XpathFilterOptional &SidebarsByValRequestType::getXpathFilter() {
	return this->xpathFilter_;
}

void SidebarsByValRequestType::setXpathFilter(const XpathFilterType &x) {
	this->xpathFilter_.set(x);
}

void SidebarsByValRequestType::setXpathFilter(const XpathFilterOptional &x) {
	this->xpathFilter_ = x;
}

void SidebarsByValRequestType::setXpathFilter(::std::unique_ptr<XpathFilterType> x) {
	this->xpathFilter_.set(std::move(x));
}

const SidebarsByValRequestType::AnySequence &SidebarsByValRequestType::getAny() const {
	return this->any_;
}

SidebarsByValRequestType::AnySequence &SidebarsByValRequestType::getAny() {
	return this->any_;
}

void SidebarsByValRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SidebarsByValRequestType::AnyAttributeSet &SidebarsByValRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarsByValRequestType::AnyAttributeSet &SidebarsByValRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarsByValRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarsByValRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarsByValRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpSidebarsByRefRequestMessageType
//

const CcmpSidebarsByRefRequestMessageType::SidebarsByRefRequestType &
CcmpSidebarsByRefRequestMessageType::getSidebarsByRefRequest() const {
	return this->sidebarsByRefRequest_.get();
}

CcmpSidebarsByRefRequestMessageType::SidebarsByRefRequestType &
CcmpSidebarsByRefRequestMessageType::getSidebarsByRefRequest() {
	return this->sidebarsByRefRequest_.get();
}

void CcmpSidebarsByRefRequestMessageType::setSidebarsByRefRequest(const SidebarsByRefRequestType &x) {
	this->sidebarsByRefRequest_.set(x);
}

void CcmpSidebarsByRefRequestMessageType::setSidebarsByRefRequest(::std::unique_ptr<SidebarsByRefRequestType> x) {
	this->sidebarsByRefRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpSidebarsByRefRequestMessageType::SidebarsByRefRequestType>
CcmpSidebarsByRefRequestMessageType::setDetachSidebarsByRefRequest() {
	return this->sidebarsByRefRequest_.detach();
}

// SidebarsByRefRequestType
//

const SidebarsByRefRequestType::XpathFilterOptional &SidebarsByRefRequestType::getXpathFilter() const {
	return this->xpathFilter_;
}

SidebarsByRefRequestType::XpathFilterOptional &SidebarsByRefRequestType::getXpathFilter() {
	return this->xpathFilter_;
}

void SidebarsByRefRequestType::setXpathFilter(const XpathFilterType &x) {
	this->xpathFilter_.set(x);
}

void SidebarsByRefRequestType::setXpathFilter(const XpathFilterOptional &x) {
	this->xpathFilter_ = x;
}

void SidebarsByRefRequestType::setXpathFilter(::std::unique_ptr<XpathFilterType> x) {
	this->xpathFilter_.set(std::move(x));
}

const SidebarsByRefRequestType::AnySequence &SidebarsByRefRequestType::getAny() const {
	return this->any_;
}

SidebarsByRefRequestType::AnySequence &SidebarsByRefRequestType::getAny() {
	return this->any_;
}

void SidebarsByRefRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SidebarsByRefRequestType::AnyAttributeSet &SidebarsByRefRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarsByRefRequestType::AnyAttributeSet &SidebarsByRefRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarsByRefRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarsByRefRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarsByRefRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpSidebarByValRequestMessageType
//

const CcmpSidebarByValRequestMessageType::SidebarByValRequestType &
CcmpSidebarByValRequestMessageType::getSidebarByValRequest() const {
	return this->sidebarByValRequest_.get();
}

CcmpSidebarByValRequestMessageType::SidebarByValRequestType &
CcmpSidebarByValRequestMessageType::getSidebarByValRequest() {
	return this->sidebarByValRequest_.get();
}

void CcmpSidebarByValRequestMessageType::setSidebarByValRequest(const SidebarByValRequestType &x) {
	this->sidebarByValRequest_.set(x);
}

void CcmpSidebarByValRequestMessageType::setSidebarByValRequest(::std::unique_ptr<SidebarByValRequestType> x) {
	this->sidebarByValRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpSidebarByValRequestMessageType::SidebarByValRequestType>
CcmpSidebarByValRequestMessageType::setDetachSidebarByValRequest() {
	return this->sidebarByValRequest_.detach();
}

// SidebarByValRequestType
//

const SidebarByValRequestType::SidebarByValInfoOptional &SidebarByValRequestType::getSidebarByValInfo() const {
	return this->sidebarByValInfo_;
}

SidebarByValRequestType::SidebarByValInfoOptional &SidebarByValRequestType::getSidebarByValInfo() {
	return this->sidebarByValInfo_;
}

void SidebarByValRequestType::setSidebarByValInfo(const SidebarByValInfoType &x) {
	this->sidebarByValInfo_.set(x);
}

void SidebarByValRequestType::setSidebarByValInfo(const SidebarByValInfoOptional &x) {
	this->sidebarByValInfo_ = x;
}

void SidebarByValRequestType::setSidebarByValInfo(::std::unique_ptr<SidebarByValInfoType> x) {
	this->sidebarByValInfo_.set(std::move(x));
}

const SidebarByValRequestType::AnySequence &SidebarByValRequestType::getAny() const {
	return this->any_;
}

SidebarByValRequestType::AnySequence &SidebarByValRequestType::getAny() {
	return this->any_;
}

void SidebarByValRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SidebarByValRequestType::AnyAttributeSet &SidebarByValRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarByValRequestType::AnyAttributeSet &SidebarByValRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarByValRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarByValRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarByValRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpSidebarByRefRequestMessageType
//

const CcmpSidebarByRefRequestMessageType::SidebarByRefRequestType &
CcmpSidebarByRefRequestMessageType::getSidebarByRefRequest() const {
	return this->sidebarByRefRequest_.get();
}

CcmpSidebarByRefRequestMessageType::SidebarByRefRequestType &
CcmpSidebarByRefRequestMessageType::getSidebarByRefRequest() {
	return this->sidebarByRefRequest_.get();
}

void CcmpSidebarByRefRequestMessageType::setSidebarByRefRequest(const SidebarByRefRequestType &x) {
	this->sidebarByRefRequest_.set(x);
}

void CcmpSidebarByRefRequestMessageType::setSidebarByRefRequest(::std::unique_ptr<SidebarByRefRequestType> x) {
	this->sidebarByRefRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpSidebarByRefRequestMessageType::SidebarByRefRequestType>
CcmpSidebarByRefRequestMessageType::setDetachSidebarByRefRequest() {
	return this->sidebarByRefRequest_.detach();
}

// SidebarByRefRequestType
//

const SidebarByRefRequestType::SidebarByRefInfoOptional &SidebarByRefRequestType::getSidebarByRefInfo() const {
	return this->sidebarByRefInfo_;
}

SidebarByRefRequestType::SidebarByRefInfoOptional &SidebarByRefRequestType::getSidebarByRefInfo() {
	return this->sidebarByRefInfo_;
}

void SidebarByRefRequestType::setSidebarByRefInfo(const SidebarByRefInfoType &x) {
	this->sidebarByRefInfo_.set(x);
}

void SidebarByRefRequestType::setSidebarByRefInfo(const SidebarByRefInfoOptional &x) {
	this->sidebarByRefInfo_ = x;
}

void SidebarByRefRequestType::setSidebarByRefInfo(::std::unique_ptr<SidebarByRefInfoType> x) {
	this->sidebarByRefInfo_.set(std::move(x));
}

const SidebarByRefRequestType::AnySequence &SidebarByRefRequestType::getAny() const {
	return this->any_;
}

SidebarByRefRequestType::AnySequence &SidebarByRefRequestType::getAny() {
	return this->any_;
}

void SidebarByRefRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SidebarByRefRequestType::AnyAttributeSet &SidebarByRefRequestType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarByRefRequestType::AnyAttributeSet &SidebarByRefRequestType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarByRefRequestType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarByRefRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarByRefRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpExtendedRequestMessageType
//

const CcmpExtendedRequestMessageType::ExtendedRequestType &CcmpExtendedRequestMessageType::getExtendedRequest() const {
	return this->extendedRequest_.get();
}

CcmpExtendedRequestMessageType::ExtendedRequestType &CcmpExtendedRequestMessageType::getExtendedRequest() {
	return this->extendedRequest_.get();
}

void CcmpExtendedRequestMessageType::setExtendedRequest(const ExtendedRequestType &x) {
	this->extendedRequest_.set(x);
}

void CcmpExtendedRequestMessageType::setExtendedRequest(::std::unique_ptr<ExtendedRequestType> x) {
	this->extendedRequest_.set(std::move(x));
}

::std::unique_ptr<CcmpExtendedRequestMessageType::ExtendedRequestType>
CcmpExtendedRequestMessageType::setDetachExtendedRequest() {
	return this->extendedRequest_.detach();
}

// ExtendedRequestType
//

const ExtendedRequestType::ExtensionNameType &ExtendedRequestType::getExtensionName() const {
	return this->extensionName_.get();
}

ExtendedRequestType::ExtensionNameType &ExtendedRequestType::getExtensionName() {
	return this->extensionName_.get();
}

void ExtendedRequestType::setExtensionName(const ExtensionNameType &x) {
	this->extensionName_.set(x);
}

void ExtendedRequestType::setExtensionName(::std::unique_ptr<ExtensionNameType> x) {
	this->extensionName_.set(std::move(x));
}

::std::unique_ptr<ExtendedRequestType::ExtensionNameType> ExtendedRequestType::setDetachExtensionName() {
	return this->extensionName_.detach();
}

const ExtendedRequestType::AnySequence &ExtendedRequestType::getAny() const {
	return this->any_;
}

ExtendedRequestType::AnySequence &ExtendedRequestType::getAny() {
	return this->any_;
}

void ExtendedRequestType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &ExtendedRequestType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ExtendedRequestType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpOptionsRequestMessageType
//

// CcmpBlueprintsResponseMessageType
//

const CcmpBlueprintsResponseMessageType::BlueprintsResponseType &
CcmpBlueprintsResponseMessageType::getBlueprintsResponse() const {
	return this->blueprintsResponse_.get();
}

CcmpBlueprintsResponseMessageType::BlueprintsResponseType &CcmpBlueprintsResponseMessageType::getBlueprintsResponse() {
	return this->blueprintsResponse_.get();
}

void CcmpBlueprintsResponseMessageType::setBlueprintsResponse(const BlueprintsResponseType &x) {
	this->blueprintsResponse_.set(x);
}

void CcmpBlueprintsResponseMessageType::setBlueprintsResponse(::std::unique_ptr<BlueprintsResponseType> x) {
	this->blueprintsResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpBlueprintsResponseMessageType::BlueprintsResponseType>
CcmpBlueprintsResponseMessageType::setDetachBlueprintsResponse() {
	return this->blueprintsResponse_.detach();
}

// BlueprintsResponseType
//

const BlueprintsResponseType::BlueprintsInfoOptional &BlueprintsResponseType::getBlueprintsInfo() const {
	return this->blueprintsInfo_;
}

BlueprintsResponseType::BlueprintsInfoOptional &BlueprintsResponseType::getBlueprintsInfo() {
	return this->blueprintsInfo_;
}

void BlueprintsResponseType::setBlueprintsInfo(const BlueprintsInfoType &x) {
	this->blueprintsInfo_.set(x);
}

void BlueprintsResponseType::setBlueprintsInfo(const BlueprintsInfoOptional &x) {
	this->blueprintsInfo_ = x;
}

void BlueprintsResponseType::setBlueprintsInfo(::std::unique_ptr<BlueprintsInfoType> x) {
	this->blueprintsInfo_.set(std::move(x));
}

const BlueprintsResponseType::AnySequence &BlueprintsResponseType::getAny() const {
	return this->any_;
}

BlueprintsResponseType::AnySequence &BlueprintsResponseType::getAny() {
	return this->any_;
}

void BlueprintsResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const BlueprintsResponseType::AnyAttributeSet &BlueprintsResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

BlueprintsResponseType::AnyAttributeSet &BlueprintsResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void BlueprintsResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &BlueprintsResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &BlueprintsResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpBlueprintResponseMessageType
//

const CcmpBlueprintResponseMessageType::BlueprintResponseType &
CcmpBlueprintResponseMessageType::getBlueprintResponse() const {
	return this->blueprintResponse_.get();
}

CcmpBlueprintResponseMessageType::BlueprintResponseType &CcmpBlueprintResponseMessageType::getBlueprintResponse() {
	return this->blueprintResponse_.get();
}

void CcmpBlueprintResponseMessageType::setBlueprintResponse(const BlueprintResponseType &x) {
	this->blueprintResponse_.set(x);
}

void CcmpBlueprintResponseMessageType::setBlueprintResponse(::std::unique_ptr<BlueprintResponseType> x) {
	this->blueprintResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpBlueprintResponseMessageType::BlueprintResponseType>
CcmpBlueprintResponseMessageType::setDetachBlueprintResponse() {
	return this->blueprintResponse_.detach();
}

// BlueprintResponseType
//

const BlueprintResponseType::BlueprintInfoOptional &BlueprintResponseType::getBlueprintInfo() const {
	return this->blueprintInfo_;
}

BlueprintResponseType::BlueprintInfoOptional &BlueprintResponseType::getBlueprintInfo() {
	return this->blueprintInfo_;
}

void BlueprintResponseType::setBlueprintInfo(const BlueprintInfoType &x) {
	this->blueprintInfo_.set(x);
}

void BlueprintResponseType::setBlueprintInfo(const BlueprintInfoOptional &x) {
	this->blueprintInfo_ = x;
}

void BlueprintResponseType::setBlueprintInfo(::std::unique_ptr<BlueprintInfoType> x) {
	this->blueprintInfo_.set(std::move(x));
}

const BlueprintResponseType::AnySequence &BlueprintResponseType::getAny() const {
	return this->any_;
}

BlueprintResponseType::AnySequence &BlueprintResponseType::getAny() {
	return this->any_;
}

void BlueprintResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const BlueprintResponseType::AnyAttributeSet &BlueprintResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

BlueprintResponseType::AnyAttributeSet &BlueprintResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void BlueprintResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &BlueprintResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &BlueprintResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpConfsResponseMessageType
//

const CcmpConfsResponseMessageType::ConfsResponseType &CcmpConfsResponseMessageType::getConfsResponse() const {
	return this->confsResponse_.get();
}

CcmpConfsResponseMessageType::ConfsResponseType &CcmpConfsResponseMessageType::getConfsResponse() {
	return this->confsResponse_.get();
}

void CcmpConfsResponseMessageType::setConfsResponse(const ConfsResponseType &x) {
	this->confsResponse_.set(x);
}

void CcmpConfsResponseMessageType::setConfsResponse(::std::unique_ptr<ConfsResponseType> x) {
	this->confsResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpConfsResponseMessageType::ConfsResponseType>
CcmpConfsResponseMessageType::setDetachConfsResponse() {
	return this->confsResponse_.detach();
}

// ConfsResponseType
//

const ConfsResponseType::ConfsInfoOptional &ConfsResponseType::getConfsInfo() const {
	return this->confsInfo_;
}

ConfsResponseType::ConfsInfoOptional &ConfsResponseType::getConfsInfo() {
	return this->confsInfo_;
}

void ConfsResponseType::setConfsInfo(const ConfsInfoType &x) {
	this->confsInfo_.set(x);
}

void ConfsResponseType::setConfsInfo(const ConfsInfoOptional &x) {
	this->confsInfo_ = x;
}

void ConfsResponseType::setConfsInfo(::std::unique_ptr<ConfsInfoType> x) {
	this->confsInfo_.set(std::move(x));
}

const ConfsResponseType::AnySequence &ConfsResponseType::getAny() const {
	return this->any_;
}

ConfsResponseType::AnySequence &ConfsResponseType::getAny() {
	return this->any_;
}

void ConfsResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConfsResponseType::AnyAttributeSet &ConfsResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConfsResponseType::AnyAttributeSet &ConfsResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConfsResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConfsResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConfsResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpConfResponseMessageType
//

const CcmpConfResponseMessageType::ConfResponseType &CcmpConfResponseMessageType::getConfResponse() const {
	return this->confResponse_.get();
}

CcmpConfResponseMessageType::ConfResponseType &CcmpConfResponseMessageType::getConfResponse() {
	return this->confResponse_.get();
}

void CcmpConfResponseMessageType::setConfResponse(const ConfResponseType &x) {
	this->confResponse_.set(x);
}

void CcmpConfResponseMessageType::setConfResponse(::std::unique_ptr<ConfResponseType> x) {
	this->confResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpConfResponseMessageType::ConfResponseType> CcmpConfResponseMessageType::setDetachConfResponse() {
	return this->confResponse_.detach();
}

// ConfResponseType
//

const ConfResponseType::ConfInfoOptional &ConfResponseType::getConfInfo() const {
	return this->confInfo_;
}

ConfResponseType::ConfInfoOptional &ConfResponseType::getConfInfo() {
	return this->confInfo_;
}

void ConfResponseType::setConfInfo(const ConfInfoType &x) {
	this->confInfo_.set(x);
}

void ConfResponseType::setConfInfo(const ConfInfoOptional &x) {
	this->confInfo_ = x;
}

void ConfResponseType::setConfInfo(::std::unique_ptr<ConfInfoType> x) {
	this->confInfo_.set(std::move(x));
}

const ConfResponseType::AnySequence &ConfResponseType::getAny() const {
	return this->any_;
}

ConfResponseType::AnySequence &ConfResponseType::getAny() {
	return this->any_;
}

void ConfResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ConfResponseType::AnyAttributeSet &ConfResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

ConfResponseType::AnyAttributeSet &ConfResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void ConfResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ConfResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ConfResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpUsersResponseMessageType
//

const CcmpUsersResponseMessageType::UsersResponseType &CcmpUsersResponseMessageType::getUsersResponse() const {
	return this->usersResponse_.get();
}

CcmpUsersResponseMessageType::UsersResponseType &CcmpUsersResponseMessageType::getUsersResponse() {
	return this->usersResponse_.get();
}

void CcmpUsersResponseMessageType::setUsersResponse(const UsersResponseType &x) {
	this->usersResponse_.set(x);
}

void CcmpUsersResponseMessageType::setUsersResponse(::std::unique_ptr<UsersResponseType> x) {
	this->usersResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpUsersResponseMessageType::UsersResponseType>
CcmpUsersResponseMessageType::setDetachUsersResponse() {
	return this->usersResponse_.detach();
}

// UsersResponseType
//

const UsersResponseType::UsersInfoOptional &UsersResponseType::getUsersInfo() const {
	return this->usersInfo_;
}

UsersResponseType::UsersInfoOptional &UsersResponseType::getUsersInfo() {
	return this->usersInfo_;
}

void UsersResponseType::setUsersInfo(const UsersInfoType &x) {
	this->usersInfo_.set(x);
}

void UsersResponseType::setUsersInfo(const UsersInfoOptional &x) {
	this->usersInfo_ = x;
}

void UsersResponseType::setUsersInfo(::std::unique_ptr<UsersInfoType> x) {
	this->usersInfo_.set(std::move(x));
}

const UsersResponseType::AnySequence &UsersResponseType::getAny() const {
	return this->any_;
}

UsersResponseType::AnySequence &UsersResponseType::getAny() {
	return this->any_;
}

void UsersResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const UsersResponseType::AnyAttributeSet &UsersResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

UsersResponseType::AnyAttributeSet &UsersResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void UsersResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UsersResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UsersResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpUserResponseMessageType
//

const CcmpUserResponseMessageType::UserResponseType &CcmpUserResponseMessageType::getUserResponse() const {
	return this->userResponse_.get();
}

CcmpUserResponseMessageType::UserResponseType &CcmpUserResponseMessageType::getUserResponse() {
	return this->userResponse_.get();
}

void CcmpUserResponseMessageType::setUserResponse(const UserResponseType &x) {
	this->userResponse_.set(x);
}

void CcmpUserResponseMessageType::setUserResponse(::std::unique_ptr<UserResponseType> x) {
	this->userResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpUserResponseMessageType::UserResponseType> CcmpUserResponseMessageType::setDetachUserResponse() {
	return this->userResponse_.detach();
}

// UserResponseType
//

const UserResponseType::UserInfoOptional &UserResponseType::getUserInfo() const {
	return this->userInfo_;
}

UserResponseType::UserInfoOptional &UserResponseType::getUserInfo() {
	return this->userInfo_;
}

void UserResponseType::setUserInfo(const UserInfoType &x) {
	this->userInfo_.set(x);
}

void UserResponseType::setUserInfo(const UserInfoOptional &x) {
	this->userInfo_ = x;
}

void UserResponseType::setUserInfo(::std::unique_ptr<UserInfoType> x) {
	this->userInfo_.set(std::move(x));
}

const UserResponseType::AnySequence &UserResponseType::getAny() const {
	return this->any_;
}

UserResponseType::AnySequence &UserResponseType::getAny() {
	return this->any_;
}

void UserResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const UserResponseType::AnyAttributeSet &UserResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

UserResponseType::AnyAttributeSet &UserResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void UserResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &UserResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &UserResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpSidebarsByValResponseMessageType
//

const CcmpSidebarsByValResponseMessageType::SidebarsByValResponseType &
CcmpSidebarsByValResponseMessageType::getSidebarsByValResponse() const {
	return this->sidebarsByValResponse_.get();
}

CcmpSidebarsByValResponseMessageType::SidebarsByValResponseType &
CcmpSidebarsByValResponseMessageType::getSidebarsByValResponse() {
	return this->sidebarsByValResponse_.get();
}

void CcmpSidebarsByValResponseMessageType::setSidebarsByValResponse(const SidebarsByValResponseType &x) {
	this->sidebarsByValResponse_.set(x);
}

void CcmpSidebarsByValResponseMessageType::setSidebarsByValResponse(::std::unique_ptr<SidebarsByValResponseType> x) {
	this->sidebarsByValResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpSidebarsByValResponseMessageType::SidebarsByValResponseType>
CcmpSidebarsByValResponseMessageType::setDetachSidebarsByValResponse() {
	return this->sidebarsByValResponse_.detach();
}

// SidebarsByValResponseType
//

const SidebarsByValResponseType::SidebarsByValInfoOptional &SidebarsByValResponseType::getSidebarsByValInfo() const {
	return this->sidebarsByValInfo_;
}

SidebarsByValResponseType::SidebarsByValInfoOptional &SidebarsByValResponseType::getSidebarsByValInfo() {
	return this->sidebarsByValInfo_;
}

void SidebarsByValResponseType::setSidebarsByValInfo(const SidebarsByValInfoType &x) {
	this->sidebarsByValInfo_.set(x);
}

void SidebarsByValResponseType::setSidebarsByValInfo(const SidebarsByValInfoOptional &x) {
	this->sidebarsByValInfo_ = x;
}

void SidebarsByValResponseType::setSidebarsByValInfo(::std::unique_ptr<SidebarsByValInfoType> x) {
	this->sidebarsByValInfo_.set(std::move(x));
}

const SidebarsByValResponseType::AnySequence &SidebarsByValResponseType::getAny() const {
	return this->any_;
}

SidebarsByValResponseType::AnySequence &SidebarsByValResponseType::getAny() {
	return this->any_;
}

void SidebarsByValResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SidebarsByValResponseType::AnyAttributeSet &SidebarsByValResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarsByValResponseType::AnyAttributeSet &SidebarsByValResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarsByValResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarsByValResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarsByValResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpSidebarsByRefResponseMessageType
//

const CcmpSidebarsByRefResponseMessageType::SidebarsByRefResponseType &
CcmpSidebarsByRefResponseMessageType::getSidebarsByRefResponse() const {
	return this->sidebarsByRefResponse_.get();
}

CcmpSidebarsByRefResponseMessageType::SidebarsByRefResponseType &
CcmpSidebarsByRefResponseMessageType::getSidebarsByRefResponse() {
	return this->sidebarsByRefResponse_.get();
}

void CcmpSidebarsByRefResponseMessageType::setSidebarsByRefResponse(const SidebarsByRefResponseType &x) {
	this->sidebarsByRefResponse_.set(x);
}

void CcmpSidebarsByRefResponseMessageType::setSidebarsByRefResponse(::std::unique_ptr<SidebarsByRefResponseType> x) {
	this->sidebarsByRefResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpSidebarsByRefResponseMessageType::SidebarsByRefResponseType>
CcmpSidebarsByRefResponseMessageType::setDetachSidebarsByRefResponse() {
	return this->sidebarsByRefResponse_.detach();
}

// SidebarsByRefResponseType
//

const SidebarsByRefResponseType::SidebarsByRefInfoOptional &SidebarsByRefResponseType::getSidebarsByRefInfo() const {
	return this->sidebarsByRefInfo_;
}

SidebarsByRefResponseType::SidebarsByRefInfoOptional &SidebarsByRefResponseType::getSidebarsByRefInfo() {
	return this->sidebarsByRefInfo_;
}

void SidebarsByRefResponseType::setSidebarsByRefInfo(const SidebarsByRefInfoType &x) {
	this->sidebarsByRefInfo_.set(x);
}

void SidebarsByRefResponseType::setSidebarsByRefInfo(const SidebarsByRefInfoOptional &x) {
	this->sidebarsByRefInfo_ = x;
}

void SidebarsByRefResponseType::setSidebarsByRefInfo(::std::unique_ptr<SidebarsByRefInfoType> x) {
	this->sidebarsByRefInfo_.set(std::move(x));
}

const SidebarsByRefResponseType::AnySequence &SidebarsByRefResponseType::getAny() const {
	return this->any_;
}

SidebarsByRefResponseType::AnySequence &SidebarsByRefResponseType::getAny() {
	return this->any_;
}

void SidebarsByRefResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SidebarsByRefResponseType::AnyAttributeSet &SidebarsByRefResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarsByRefResponseType::AnyAttributeSet &SidebarsByRefResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarsByRefResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarsByRefResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarsByRefResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpSidebarByValResponseMessageType
//

const CcmpSidebarByValResponseMessageType::SidebarByValResponseType &
CcmpSidebarByValResponseMessageType::getSidebarByValResponse() const {
	return this->sidebarByValResponse_.get();
}

CcmpSidebarByValResponseMessageType::SidebarByValResponseType &
CcmpSidebarByValResponseMessageType::getSidebarByValResponse() {
	return this->sidebarByValResponse_.get();
}

void CcmpSidebarByValResponseMessageType::setSidebarByValResponse(const SidebarByValResponseType &x) {
	this->sidebarByValResponse_.set(x);
}

void CcmpSidebarByValResponseMessageType::setSidebarByValResponse(::std::unique_ptr<SidebarByValResponseType> x) {
	this->sidebarByValResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpSidebarByValResponseMessageType::SidebarByValResponseType>
CcmpSidebarByValResponseMessageType::setDetachSidebarByValResponse() {
	return this->sidebarByValResponse_.detach();
}

// SidebarByValResponseType
//

const SidebarByValResponseType::SidebarByValInfoOptional &SidebarByValResponseType::getSidebarByValInfo() const {
	return this->sidebarByValInfo_;
}

SidebarByValResponseType::SidebarByValInfoOptional &SidebarByValResponseType::getSidebarByValInfo() {
	return this->sidebarByValInfo_;
}

void SidebarByValResponseType::setSidebarByValInfo(const SidebarByValInfoType &x) {
	this->sidebarByValInfo_.set(x);
}

void SidebarByValResponseType::setSidebarByValInfo(const SidebarByValInfoOptional &x) {
	this->sidebarByValInfo_ = x;
}

void SidebarByValResponseType::setSidebarByValInfo(::std::unique_ptr<SidebarByValInfoType> x) {
	this->sidebarByValInfo_.set(std::move(x));
}

const SidebarByValResponseType::AnySequence &SidebarByValResponseType::getAny() const {
	return this->any_;
}

SidebarByValResponseType::AnySequence &SidebarByValResponseType::getAny() {
	return this->any_;
}

void SidebarByValResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SidebarByValResponseType::AnyAttributeSet &SidebarByValResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarByValResponseType::AnyAttributeSet &SidebarByValResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarByValResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarByValResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarByValResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpSidebarByRefResponseMessageType
//

const CcmpSidebarByRefResponseMessageType::SidebarByRefResponseType &
CcmpSidebarByRefResponseMessageType::getSidebarByRefResponse() const {
	return this->sidebarByRefResponse_.get();
}

CcmpSidebarByRefResponseMessageType::SidebarByRefResponseType &
CcmpSidebarByRefResponseMessageType::getSidebarByRefResponse() {
	return this->sidebarByRefResponse_.get();
}

void CcmpSidebarByRefResponseMessageType::setSidebarByRefResponse(const SidebarByRefResponseType &x) {
	this->sidebarByRefResponse_.set(x);
}

void CcmpSidebarByRefResponseMessageType::setSidebarByRefResponse(::std::unique_ptr<SidebarByRefResponseType> x) {
	this->sidebarByRefResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpSidebarByRefResponseMessageType::SidebarByRefResponseType>
CcmpSidebarByRefResponseMessageType::setDetachSidebarByRefResponse() {
	return this->sidebarByRefResponse_.detach();
}

// SidebarByRefResponseType
//

const SidebarByRefResponseType::SidebarByRefInfoOptional &SidebarByRefResponseType::getSidebarByRefInfo() const {
	return this->sidebarByRefInfo_;
}

SidebarByRefResponseType::SidebarByRefInfoOptional &SidebarByRefResponseType::getSidebarByRefInfo() {
	return this->sidebarByRefInfo_;
}

void SidebarByRefResponseType::setSidebarByRefInfo(const SidebarByRefInfoType &x) {
	this->sidebarByRefInfo_.set(x);
}

void SidebarByRefResponseType::setSidebarByRefInfo(const SidebarByRefInfoOptional &x) {
	this->sidebarByRefInfo_ = x;
}

void SidebarByRefResponseType::setSidebarByRefInfo(::std::unique_ptr<SidebarByRefInfoType> x) {
	this->sidebarByRefInfo_.set(std::move(x));
}

const SidebarByRefResponseType::AnySequence &SidebarByRefResponseType::getAny() const {
	return this->any_;
}

SidebarByRefResponseType::AnySequence &SidebarByRefResponseType::getAny() {
	return this->any_;
}

void SidebarByRefResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SidebarByRefResponseType::AnyAttributeSet &SidebarByRefResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

SidebarByRefResponseType::AnyAttributeSet &SidebarByRefResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void SidebarByRefResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SidebarByRefResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SidebarByRefResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpExtendedResponseMessageType
//

const CcmpExtendedResponseMessageType::ExtendedResponseType &
CcmpExtendedResponseMessageType::getExtendedResponse() const {
	return this->extendedResponse_.get();
}

CcmpExtendedResponseMessageType::ExtendedResponseType &CcmpExtendedResponseMessageType::getExtendedResponse() {
	return this->extendedResponse_.get();
}

void CcmpExtendedResponseMessageType::setExtendedResponse(const ExtendedResponseType &x) {
	this->extendedResponse_.set(x);
}

void CcmpExtendedResponseMessageType::setExtendedResponse(::std::unique_ptr<ExtendedResponseType> x) {
	this->extendedResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpExtendedResponseMessageType::ExtendedResponseType>
CcmpExtendedResponseMessageType::setDetachExtendedResponse() {
	return this->extendedResponse_.detach();
}

// ExtendedResponseType
//

const ExtendedResponseType::ExtensionNameType &ExtendedResponseType::getExtensionName() const {
	return this->extensionName_.get();
}

ExtendedResponseType::ExtensionNameType &ExtendedResponseType::getExtensionName() {
	return this->extensionName_.get();
}

void ExtendedResponseType::setExtensionName(const ExtensionNameType &x) {
	this->extensionName_.set(x);
}

void ExtendedResponseType::setExtensionName(::std::unique_ptr<ExtensionNameType> x) {
	this->extensionName_.set(std::move(x));
}

::std::unique_ptr<ExtendedResponseType::ExtensionNameType> ExtendedResponseType::setDetachExtensionName() {
	return this->extensionName_.detach();
}

const ExtendedResponseType::AnySequence &ExtendedResponseType::getAny() const {
	return this->any_;
}

ExtendedResponseType::AnySequence &ExtendedResponseType::getAny() {
	return this->any_;
}

void ExtendedResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ::xercesc::DOMDocument &ExtendedResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ExtendedResponseType::getDomDocument() {
	return *this->dom_document_;
}

// CcmpOptionsResponseMessageType
//

const CcmpOptionsResponseMessageType::OptionsResponseType &CcmpOptionsResponseMessageType::getOptionsResponse() const {
	return this->optionsResponse_.get();
}

CcmpOptionsResponseMessageType::OptionsResponseType &CcmpOptionsResponseMessageType::getOptionsResponse() {
	return this->optionsResponse_.get();
}

void CcmpOptionsResponseMessageType::setOptionsResponse(const OptionsResponseType &x) {
	this->optionsResponse_.set(x);
}

void CcmpOptionsResponseMessageType::setOptionsResponse(::std::unique_ptr<OptionsResponseType> x) {
	this->optionsResponse_.set(std::move(x));
}

::std::unique_ptr<CcmpOptionsResponseMessageType::OptionsResponseType>
CcmpOptionsResponseMessageType::setDetachOptionsResponse() {
	return this->optionsResponse_.detach();
}

// OptionsResponseType
//

const OptionsResponseType::OptionsOptional &OptionsResponseType::getOptions() const {
	return this->options_;
}

OptionsResponseType::OptionsOptional &OptionsResponseType::getOptions() {
	return this->options_;
}

void OptionsResponseType::setOptions(const OptionsType &x) {
	this->options_.set(x);
}

void OptionsResponseType::setOptions(const OptionsOptional &x) {
	this->options_ = x;
}

void OptionsResponseType::setOptions(::std::unique_ptr<OptionsType> x) {
	this->options_.set(std::move(x));
}

const OptionsResponseType::AnySequence &OptionsResponseType::getAny() const {
	return this->any_;
}

OptionsResponseType::AnySequence &OptionsResponseType::getAny() {
	return this->any_;
}

void OptionsResponseType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const OptionsResponseType::AnyAttributeSet &OptionsResponseType::getAnyAttribute() const {
	return this->any_attribute_;
}

OptionsResponseType::AnyAttributeSet &OptionsResponseType::getAnyAttribute() {
	return this->any_attribute_;
}

void OptionsResponseType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &OptionsResponseType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &OptionsResponseType::getDomDocument() {
	return *this->dom_document_;
}

// ResponseCodeType
//

// OperationType
//

OperationType::OperationType(Value v) : ::LinphonePrivate::Xsd::XmlSchema::Token(_xsd_OperationType_literals_[v]) {
}

OperationType::OperationType(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::Token(v) {
}

OperationType::OperationType(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::Token(v) {
}

OperationType::OperationType(const ::LinphonePrivate::Xsd::XmlSchema::Token &v)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(v) {
}

OperationType::OperationType(const OperationType &v,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(v, f, c) {
}

OperationType &OperationType::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::Token &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::Token(_xsd_OperationType_literals_[v]);

	return *this;
}

// SubjectType
//

const SubjectType::UsernameOptional &SubjectType::getUsername() const {
	return this->username_;
}

SubjectType::UsernameOptional &SubjectType::getUsername() {
	return this->username_;
}

void SubjectType::setUsername(const UsernameType &x) {
	this->username_.set(x);
}

void SubjectType::setUsername(const UsernameOptional &x) {
	this->username_ = x;
}

void SubjectType::setUsername(::std::unique_ptr<UsernameType> x) {
	this->username_.set(std::move(x));
}

const SubjectType::PasswordOptional &SubjectType::getPassword() const {
	return this->password_;
}

SubjectType::PasswordOptional &SubjectType::getPassword() {
	return this->password_;
}

void SubjectType::setPassword(const PasswordType &x) {
	this->password_.set(x);
}

void SubjectType::setPassword(const PasswordOptional &x) {
	this->password_ = x;
}

void SubjectType::setPassword(::std::unique_ptr<PasswordType> x) {
	this->password_.set(std::move(x));
}

const SubjectType::AnySequence &SubjectType::getAny() const {
	return this->any_;
}

SubjectType::AnySequence &SubjectType::getAny() {
	return this->any_;
}

void SubjectType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const SubjectType::AnyAttributeSet &SubjectType::getAnyAttribute() const {
	return this->any_attribute_;
}

SubjectType::AnyAttributeSet &SubjectType::getAnyAttribute() {
	return this->any_attribute_;
}

void SubjectType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &SubjectType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &SubjectType::getDomDocument() {
	return *this->dom_document_;
}

// OptionsType
//

const OptionsType::StandardMessageListType &OptionsType::getStandardMessageList() const {
	return this->standard_message_list_.get();
}

OptionsType::StandardMessageListType &OptionsType::getStandardMessageList() {
	return this->standard_message_list_.get();
}

void OptionsType::setStandardMessageList(const StandardMessageListType &x) {
	this->standard_message_list_.set(x);
}

void OptionsType::setStandardMessageList(::std::unique_ptr<StandardMessageListType> x) {
	this->standard_message_list_.set(std::move(x));
}

::std::unique_ptr<OptionsType::StandardMessageListType> OptionsType::setDetachStandard_message_list() {
	return this->standard_message_list_.detach();
}

const OptionsType::ExtendedMessageListOptional &OptionsType::getExtendedMessageList() const {
	return this->extended_message_list_;
}

OptionsType::ExtendedMessageListOptional &OptionsType::getExtendedMessageList() {
	return this->extended_message_list_;
}

void OptionsType::setExtendedMessageList(const ExtendedMessageListType &x) {
	this->extended_message_list_.set(x);
}

void OptionsType::setExtendedMessageList(const ExtendedMessageListOptional &x) {
	this->extended_message_list_ = x;
}

void OptionsType::setExtendedMessageList(::std::unique_ptr<ExtendedMessageListType> x) {
	this->extended_message_list_.set(std::move(x));
}

const OptionsType::AnySequence &OptionsType::getAny() const {
	return this->any_;
}

OptionsType::AnySequence &OptionsType::getAny() {
	return this->any_;
}

void OptionsType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const OptionsType::AnyAttributeSet &OptionsType::getAnyAttribute() const {
	return this->any_attribute_;
}

OptionsType::AnyAttributeSet &OptionsType::getAnyAttribute() {
	return this->any_attribute_;
}

void OptionsType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &OptionsType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &OptionsType::getDomDocument() {
	return *this->dom_document_;
}

// StandardMessageListType
//

const StandardMessageListType::StandardMessageSequence &StandardMessageListType::getStandardMessage() const {
	return this->standard_message_;
}

StandardMessageListType::StandardMessageSequence &StandardMessageListType::getStandardMessage() {
	return this->standard_message_;
}

void StandardMessageListType::setStandardMessage(const StandardMessageSequence &s) {
	this->standard_message_ = s;
}

const StandardMessageListType::AnySequence &StandardMessageListType::getAny() const {
	return this->any_;
}

StandardMessageListType::AnySequence &StandardMessageListType::getAny() {
	return this->any_;
}

void StandardMessageListType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const StandardMessageListType::AnyAttributeSet &StandardMessageListType::getAnyAttribute() const {
	return this->any_attribute_;
}

StandardMessageListType::AnyAttributeSet &StandardMessageListType::getAnyAttribute() {
	return this->any_attribute_;
}

void StandardMessageListType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &StandardMessageListType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &StandardMessageListType::getDomDocument() {
	return *this->dom_document_;
}

// StandardMessageType
//

const StandardMessageType::NameType &StandardMessageType::getName() const {
	return this->name_.get();
}

StandardMessageType::NameType &StandardMessageType::getName() {
	return this->name_.get();
}

void StandardMessageType::setName(const NameType &x) {
	this->name_.set(x);
}

void StandardMessageType::setName(::std::unique_ptr<NameType> x) {
	this->name_.set(std::move(x));
}

::std::unique_ptr<StandardMessageType::NameType> StandardMessageType::setDetachName() {
	return this->name_.detach();
}

const StandardMessageType::OperationsOptional &StandardMessageType::getOperations() const {
	return this->operations_;
}

StandardMessageType::OperationsOptional &StandardMessageType::getOperations() {
	return this->operations_;
}

void StandardMessageType::setOperations(const OperationsType &x) {
	this->operations_.set(x);
}

void StandardMessageType::setOperations(const OperationsOptional &x) {
	this->operations_ = x;
}

void StandardMessageType::setOperations(::std::unique_ptr<OperationsType> x) {
	this->operations_.set(std::move(x));
}

const StandardMessageType::SchemaDefOptional &StandardMessageType::getSchemaDef() const {
	return this->schema_def_;
}

StandardMessageType::SchemaDefOptional &StandardMessageType::getSchemaDef() {
	return this->schema_def_;
}

void StandardMessageType::setSchemaDef(const SchemaDefType &x) {
	this->schema_def_.set(x);
}

void StandardMessageType::setSchemaDef(const SchemaDefOptional &x) {
	this->schema_def_ = x;
}

void StandardMessageType::setSchemaDef(::std::unique_ptr<SchemaDefType> x) {
	this->schema_def_.set(std::move(x));
}

const StandardMessageType::DescriptionOptional &StandardMessageType::getDescription() const {
	return this->description_;
}

StandardMessageType::DescriptionOptional &StandardMessageType::getDescription() {
	return this->description_;
}

void StandardMessageType::setDescription(const DescriptionType &x) {
	this->description_.set(x);
}

void StandardMessageType::setDescription(const DescriptionOptional &x) {
	this->description_ = x;
}

void StandardMessageType::setDescription(::std::unique_ptr<DescriptionType> x) {
	this->description_.set(std::move(x));
}

const StandardMessageType::AnySequence &StandardMessageType::getAny() const {
	return this->any_;
}

StandardMessageType::AnySequence &StandardMessageType::getAny() {
	return this->any_;
}

void StandardMessageType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const StandardMessageType::AnyAttributeSet &StandardMessageType::getAnyAttribute() const {
	return this->any_attribute_;
}

StandardMessageType::AnyAttributeSet &StandardMessageType::getAnyAttribute() {
	return this->any_attribute_;
}

void StandardMessageType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &StandardMessageType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &StandardMessageType::getDomDocument() {
	return *this->dom_document_;
}

// StandardMessageNameType
//

StandardMessageNameType::StandardMessageNameType(Value v)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(_xsd_StandardMessageNameType_literals_[v]) {
}

StandardMessageNameType::StandardMessageNameType(const char *v) : ::LinphonePrivate::Xsd::XmlSchema::Token(v) {
}

StandardMessageNameType::StandardMessageNameType(const ::std::string &v) : ::LinphonePrivate::Xsd::XmlSchema::Token(v) {
}

StandardMessageNameType::StandardMessageNameType(const ::LinphonePrivate::Xsd::XmlSchema::Token &v)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(v) {
}

StandardMessageNameType::StandardMessageNameType(const StandardMessageNameType &v,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(v, f, c) {
}

StandardMessageNameType &StandardMessageNameType::operator=(Value v) {
	static_cast<::LinphonePrivate::Xsd::XmlSchema::Token &>(*this) =
	    ::LinphonePrivate::Xsd::XmlSchema::Token(_xsd_StandardMessageNameType_literals_[v]);

	return *this;
}

// OperationsType
//

const OperationsType::OperationSequence &OperationsType::getOperation() const {
	return this->operation_;
}

OperationsType::OperationSequence &OperationsType::getOperation() {
	return this->operation_;
}

void OperationsType::setOperation(const OperationSequence &s) {
	this->operation_ = s;
}

const OperationsType::AnyAttributeSet &OperationsType::getAnyAttribute() const {
	return this->any_attribute_;
}

OperationsType::AnyAttributeSet &OperationsType::getAnyAttribute() {
	return this->any_attribute_;
}

void OperationsType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &OperationsType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &OperationsType::getDomDocument() {
	return *this->dom_document_;
}

// ExtendedMessageListType
//

const ExtendedMessageListType::ExtendedMessageOptional &ExtendedMessageListType::getExtendedMessage() const {
	return this->extended_message_;
}

ExtendedMessageListType::ExtendedMessageOptional &ExtendedMessageListType::getExtendedMessage() {
	return this->extended_message_;
}

void ExtendedMessageListType::setExtendedMessage(const ExtendedMessageType &x) {
	this->extended_message_.set(x);
}

void ExtendedMessageListType::setExtendedMessage(const ExtendedMessageOptional &x) {
	this->extended_message_ = x;
}

void ExtendedMessageListType::setExtendedMessage(::std::unique_ptr<ExtendedMessageType> x) {
	this->extended_message_.set(std::move(x));
}

const ExtendedMessageListType::AnySequence &ExtendedMessageListType::getAny() const {
	return this->any_;
}

ExtendedMessageListType::AnySequence &ExtendedMessageListType::getAny() {
	return this->any_;
}

void ExtendedMessageListType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ExtendedMessageListType::AnyAttributeSet &ExtendedMessageListType::getAnyAttribute() const {
	return this->any_attribute_;
}

ExtendedMessageListType::AnyAttributeSet &ExtendedMessageListType::getAnyAttribute() {
	return this->any_attribute_;
}

void ExtendedMessageListType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ExtendedMessageListType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ExtendedMessageListType::getDomDocument() {
	return *this->dom_document_;
}

// ExtendedMessageType
//

const ExtendedMessageType::NameType &ExtendedMessageType::getName() const {
	return this->name_.get();
}

ExtendedMessageType::NameType &ExtendedMessageType::getName() {
	return this->name_.get();
}

void ExtendedMessageType::setName(const NameType &x) {
	this->name_.set(x);
}

void ExtendedMessageType::setName(::std::unique_ptr<NameType> x) {
	this->name_.set(std::move(x));
}

::std::unique_ptr<ExtendedMessageType::NameType> ExtendedMessageType::setDetachName() {
	return this->name_.detach();
}

const ExtendedMessageType::OperationsOptional &ExtendedMessageType::getOperations() const {
	return this->operations_;
}

ExtendedMessageType::OperationsOptional &ExtendedMessageType::getOperations() {
	return this->operations_;
}

void ExtendedMessageType::setOperations(const OperationsType &x) {
	this->operations_.set(x);
}

void ExtendedMessageType::setOperations(const OperationsOptional &x) {
	this->operations_ = x;
}

void ExtendedMessageType::setOperations(::std::unique_ptr<OperationsType> x) {
	this->operations_.set(std::move(x));
}

const ExtendedMessageType::SchemaDefType &ExtendedMessageType::getSchemaDef() const {
	return this->schema_def_.get();
}

ExtendedMessageType::SchemaDefType &ExtendedMessageType::getSchemaDef() {
	return this->schema_def_.get();
}

void ExtendedMessageType::setSchemaDef(const SchemaDefType &x) {
	this->schema_def_.set(x);
}

void ExtendedMessageType::setSchemaDef(::std::unique_ptr<SchemaDefType> x) {
	this->schema_def_.set(std::move(x));
}

::std::unique_ptr<ExtendedMessageType::SchemaDefType> ExtendedMessageType::setDetachSchema_def() {
	return this->schema_def_.detach();
}

const ExtendedMessageType::DescriptionOptional &ExtendedMessageType::getDescription() const {
	return this->description_;
}

ExtendedMessageType::DescriptionOptional &ExtendedMessageType::getDescription() {
	return this->description_;
}

void ExtendedMessageType::setDescription(const DescriptionType &x) {
	this->description_.set(x);
}

void ExtendedMessageType::setDescription(const DescriptionOptional &x) {
	this->description_ = x;
}

void ExtendedMessageType::setDescription(::std::unique_ptr<DescriptionType> x) {
	this->description_.set(std::move(x));
}

const ExtendedMessageType::AnySequence &ExtendedMessageType::getAny() const {
	return this->any_;
}

ExtendedMessageType::AnySequence &ExtendedMessageType::getAny() {
	return this->any_;
}

void ExtendedMessageType::setAny(const AnySequence &s) {
	this->any_ = s;
}

const ExtendedMessageType::AnyAttributeSet &ExtendedMessageType::getAnyAttribute() const {
	return this->any_attribute_;
}

ExtendedMessageType::AnyAttributeSet &ExtendedMessageType::getAnyAttribute() {
	return this->any_attribute_;
}

void ExtendedMessageType::setAnyAttribute(const AnyAttributeSet &s) {
	this->any_attribute_ = s;
}

const ::xercesc::DOMDocument &ExtendedMessageType::getDomDocument() const {
	return *this->dom_document_;
}

::xercesc::DOMDocument &ExtendedMessageType::getDomDocument() {
	return *this->dom_document_;
}
} // namespace XconCcmp
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
namespace XconCcmp {
// CcmpRequestType
//

CcmpRequestType::CcmpRequestType(const CcmpRequestType1 &ccmpRequest)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), ccmpRequest_(ccmpRequest, this) {
}

CcmpRequestType::CcmpRequestType(::std::unique_ptr<CcmpRequestType1> ccmpRequest)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), ccmpRequest_(std::move(ccmpRequest), this) {
}

CcmpRequestType::CcmpRequestType(const CcmpRequestType &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), ccmpRequest_(x.ccmpRequest_, f, this) {
}

CcmpRequestType::CcmpRequestType(const ::xercesc::DOMElement &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      ccmpRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void CcmpRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// ccmpRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "ccmpRequest", "", &::xsd::cxx::tree::factory_impl<CcmpRequestType1>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!ccmpRequest_.present()) {
					::std::unique_ptr<CcmpRequestType1> r(dynamic_cast<CcmpRequestType1 *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->ccmpRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!ccmpRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("ccmpRequest", "");
	}
}

CcmpRequestType *CcmpRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpRequestType(*this, f, c);
}

CcmpRequestType &CcmpRequestType::operator=(const CcmpRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->ccmpRequest_ = x.ccmpRequest_;
	}

	return *this;
}

CcmpRequestType::~CcmpRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpRequestType>
    _xsd_CcmpRequestType_type_factory_init("ccmp-request-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpRequestMessageType
//

CcmpRequestMessageType::CcmpRequestMessageType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      subject_(this), confUserID_(this), confObjID_(this), operation_(this), conference_password_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

CcmpRequestMessageType::CcmpRequestMessageType(const CcmpRequestMessageType &x,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      subject_(x.subject_, f, this), confUserID_(x.confUserID_, f, this), confObjID_(x.confObjID_, f, this),
      operation_(x.operation_, f, this), conference_password_(x.conference_password_, f, this),
      any_(x.any_, this->getDomDocument()), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

CcmpRequestMessageType::CcmpRequestMessageType(const ::xercesc::DOMElement &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), subject_(this), confUserID_(this), confObjID_(this),
      operation_(this), conference_password_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// subject
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "subject", "", &::xsd::cxx::tree::factory_impl<SubjectType>, false, false, i, n, f, this));

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

		// confUserID
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confUserID", "", &::xsd::cxx::tree::factory_impl<ConfUserIDType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->confUserID_) {
					::std::unique_ptr<ConfUserIDType> r(dynamic_cast<ConfUserIDType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confUserID_.set(::std::move(r));
					continue;
				}
			}
		}

		// confObjID
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confObjID", "", &::xsd::cxx::tree::factory_impl<ConfObjIDType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->confObjID_) {
					::std::unique_ptr<ConfObjIDType> r(dynamic_cast<ConfObjIDType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confObjID_.set(::std::move(r));
					continue;
				}
			}
		}

		// operation
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "operation", "", &::xsd::cxx::tree::factory_impl<OperationType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->operation_) {
					::std::unique_ptr<OperationType> r(dynamic_cast<OperationType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->operation_.set(::std::move(r));
					continue;
				}
			}
		}

		// conference-password
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "conference-password", "", &::xsd::cxx::tree::factory_impl<ConferencePasswordType>, false, false, i, n,
			    f, this));

			if (tmp.get() != 0) {
				if (!this->conference_password_) {
					::std::unique_ptr<ConferencePasswordType> r(dynamic_cast<ConferencePasswordType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->conference_password_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

CcmpRequestMessageType *CcmpRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpRequestMessageType(*this, f, c);
}

CcmpRequestMessageType &CcmpRequestMessageType::operator=(const CcmpRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->subject_ = x.subject_;
		this->confUserID_ = x.confUserID_;
		this->confObjID_ = x.confObjID_;
		this->operation_ = x.operation_;
		this->conference_password_ = x.conference_password_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

CcmpRequestMessageType::~CcmpRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpRequestMessageType>
    _xsd_CcmpRequestMessageType_type_factory_init("ccmp-request-message-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpResponseType
//

CcmpResponseType::CcmpResponseType(const CcmpResponseType1 &ccmpResponse)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), ccmpResponse_(ccmpResponse, this) {
}

CcmpResponseType::CcmpResponseType(::std::unique_ptr<CcmpResponseType1> ccmpResponse)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), ccmpResponse_(std::move(ccmpResponse), this) {
}

CcmpResponseType::CcmpResponseType(const CcmpResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), ccmpResponse_(x.ccmpResponse_, f, this) {
}

CcmpResponseType::CcmpResponseType(const ::xercesc::DOMElement &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      ccmpResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void CcmpResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// ccmpResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "ccmpResponse", "", &::xsd::cxx::tree::factory_impl<CcmpResponseType1>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!ccmpResponse_.present()) {
					::std::unique_ptr<CcmpResponseType1> r(dynamic_cast<CcmpResponseType1 *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->ccmpResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!ccmpResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("ccmpResponse", "");
	}
}

CcmpResponseType *CcmpResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpResponseType(*this, f, c);
}

CcmpResponseType &CcmpResponseType::operator=(const CcmpResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->ccmpResponse_ = x.ccmpResponse_;
	}

	return *this;
}

CcmpResponseType::~CcmpResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpResponseType>
    _xsd_CcmpResponseType_type_factory_init("ccmp-response-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpResponseMessageType
//

CcmpResponseMessageType::CcmpResponseMessageType(const ConfUserIDType &confUserID,
                                                 const ResponseCodeType &response_code)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confUserID_(confUserID, this), confObjID_(this), operation_(this), response_code_(response_code, this),
      response_string_(this), version_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

CcmpResponseMessageType::CcmpResponseMessageType(::std::unique_ptr<ConfUserIDType> confUserID,
                                                 ::std::unique_ptr<ResponseCodeType> response_code)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confUserID_(std::move(confUserID), this), confObjID_(this), operation_(this),
      response_code_(std::move(response_code), this), response_string_(this), version_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

CcmpResponseMessageType::CcmpResponseMessageType(const CcmpResponseMessageType &x,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confUserID_(x.confUserID_, f, this), confObjID_(x.confObjID_, f, this), operation_(x.operation_, f, this),
      response_code_(x.response_code_, f, this), response_string_(x.response_string_, f, this),
      version_(x.version_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

CcmpResponseMessageType::CcmpResponseMessageType(const ::xercesc::DOMElement &e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), confUserID_(this), confObjID_(this),
      operation_(this), response_code_(this), response_string_(this), version_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// confUserID
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confUserID", "", &::xsd::cxx::tree::factory_impl<ConfUserIDType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!confUserID_.present()) {
					::std::unique_ptr<ConfUserIDType> r(dynamic_cast<ConfUserIDType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confUserID_.set(::std::move(r));
					continue;
				}
			}
		}

		// confObjID
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confObjID", "", &::xsd::cxx::tree::factory_impl<ConfObjIDType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->confObjID_) {
					::std::unique_ptr<ConfObjIDType> r(dynamic_cast<ConfObjIDType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confObjID_.set(::std::move(r));
					continue;
				}
			}
		}

		// operation
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "operation", "", &::xsd::cxx::tree::factory_impl<OperationType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->operation_) {
					::std::unique_ptr<OperationType> r(dynamic_cast<OperationType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->operation_.set(::std::move(r));
					continue;
				}
			}
		}

		// response-code
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "response-code", "", &::xsd::cxx::tree::factory_impl<ResponseCodeType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!response_code_.present()) {
					::std::unique_ptr<ResponseCodeType> r(dynamic_cast<ResponseCodeType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->response_code_.set(::std::move(r));
					continue;
				}
			}
		}

		// response-string
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "response-string", "", &::xsd::cxx::tree::factory_impl<ResponseStringType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->response_string_) {
					::std::unique_ptr<ResponseStringType> r(dynamic_cast<ResponseStringType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->response_string_.set(::std::move(r));
					continue;
				}
			}
		}

		// version
		//
		if (n.name() == "version" && n.namespace_().empty()) {
			if (!this->version_) {
				this->version_.set(VersionTraits::create(i, f, this));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!confUserID_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("confUserID", "");
	}

	if (!response_code_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("response-code", "");
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

CcmpResponseMessageType *CcmpResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpResponseMessageType(*this, f, c);
}

CcmpResponseMessageType &CcmpResponseMessageType::operator=(const CcmpResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->confUserID_ = x.confUserID_;
		this->confObjID_ = x.confObjID_;
		this->operation_ = x.operation_;
		this->response_code_ = x.response_code_;
		this->response_string_ = x.response_string_;
		this->version_ = x.version_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

CcmpResponseMessageType::~CcmpResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpResponseMessageType>
    _xsd_CcmpResponseMessageType_type_factory_init("ccmp-response-message-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpBlueprintsRequestMessageType
//

CcmpBlueprintsRequestMessageType::CcmpBlueprintsRequestMessageType(const BlueprintsRequestType &blueprintsRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), blueprintsRequest_(blueprintsRequest, this) {
}

CcmpBlueprintsRequestMessageType::CcmpBlueprintsRequestMessageType(
    ::std::unique_ptr<BlueprintsRequestType> blueprintsRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(),
      blueprintsRequest_(std::move(blueprintsRequest), this) {
}

CcmpBlueprintsRequestMessageType::CcmpBlueprintsRequestMessageType(const CcmpBlueprintsRequestMessageType &x,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c),
      blueprintsRequest_(x.blueprintsRequest_, f, this) {
}

CcmpBlueprintsRequestMessageType::CcmpBlueprintsRequestMessageType(const ::xercesc::DOMElement &e,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      blueprintsRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpBlueprintsRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// blueprintsRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<BlueprintsRequestType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!blueprintsRequest_.present()) {
					::std::unique_ptr<BlueprintsRequestType> r(dynamic_cast<BlueprintsRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->blueprintsRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!blueprintsRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpBlueprintsRequestMessageType *
CcmpBlueprintsRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpBlueprintsRequestMessageType(*this, f, c);
}

CcmpBlueprintsRequestMessageType &
CcmpBlueprintsRequestMessageType::operator=(const CcmpBlueprintsRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->blueprintsRequest_ = x.blueprintsRequest_;
	}

	return *this;
}

CcmpBlueprintsRequestMessageType::~CcmpBlueprintsRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpBlueprintsRequestMessageType>
    _xsd_CcmpBlueprintsRequestMessageType_type_factory_init("ccmp-blueprints-request-message-type",
                                                            "urn:ietf:params:xml:ns:xcon-ccmp");

// BlueprintsRequestType
//

BlueprintsRequestType::BlueprintsRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      xpathFilter_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

BlueprintsRequestType::BlueprintsRequestType(const BlueprintsRequestType &x,
                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      xpathFilter_(x.xpathFilter_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

BlueprintsRequestType::BlueprintsRequestType(const ::xercesc::DOMElement &e,
                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), xpathFilter_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void BlueprintsRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// xpathFilter
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "xpathFilter", "", &::xsd::cxx::tree::factory_impl<XpathFilterType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->xpathFilter_) {
					::std::unique_ptr<XpathFilterType> r(dynamic_cast<XpathFilterType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->xpathFilter_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

BlueprintsRequestType *BlueprintsRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class BlueprintsRequestType(*this, f, c);
}

BlueprintsRequestType &BlueprintsRequestType::operator=(const BlueprintsRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->xpathFilter_ = x.xpathFilter_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

BlueprintsRequestType::~BlueprintsRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, BlueprintsRequestType>
    _xsd_BlueprintsRequestType_type_factory_init("blueprintsRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpBlueprintRequestMessageType
//

CcmpBlueprintRequestMessageType::CcmpBlueprintRequestMessageType(const BlueprintRequestType &blueprintRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), blueprintRequest_(blueprintRequest, this) {
}

CcmpBlueprintRequestMessageType::CcmpBlueprintRequestMessageType(
    ::std::unique_ptr<BlueprintRequestType> blueprintRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), blueprintRequest_(std::move(blueprintRequest), this) {
}

CcmpBlueprintRequestMessageType::CcmpBlueprintRequestMessageType(const CcmpBlueprintRequestMessageType &x,
                                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c),
      blueprintRequest_(x.blueprintRequest_, f, this) {
}

CcmpBlueprintRequestMessageType::CcmpBlueprintRequestMessageType(const ::xercesc::DOMElement &e,
                                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      blueprintRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpBlueprintRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// blueprintRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<BlueprintRequestType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!blueprintRequest_.present()) {
					::std::unique_ptr<BlueprintRequestType> r(dynamic_cast<BlueprintRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->blueprintRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!blueprintRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpBlueprintRequestMessageType *
CcmpBlueprintRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpBlueprintRequestMessageType(*this, f, c);
}

CcmpBlueprintRequestMessageType &CcmpBlueprintRequestMessageType::operator=(const CcmpBlueprintRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->blueprintRequest_ = x.blueprintRequest_;
	}

	return *this;
}

CcmpBlueprintRequestMessageType::~CcmpBlueprintRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpBlueprintRequestMessageType>
    _xsd_CcmpBlueprintRequestMessageType_type_factory_init("ccmp-blueprint-request-message-type",
                                                           "urn:ietf:params:xml:ns:xcon-ccmp");

// BlueprintRequestType
//

BlueprintRequestType::BlueprintRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      blueprintInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

BlueprintRequestType::BlueprintRequestType(const BlueprintRequestType &x,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      blueprintInfo_(x.blueprintInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

BlueprintRequestType::BlueprintRequestType(const ::xercesc::DOMElement &e,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), blueprintInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void BlueprintRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// blueprintInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "blueprintInfo", "", &::xsd::cxx::tree::factory_impl<BlueprintInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->blueprintInfo_) {
					::std::unique_ptr<BlueprintInfoType> r(dynamic_cast<BlueprintInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->blueprintInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

BlueprintRequestType *BlueprintRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class BlueprintRequestType(*this, f, c);
}

BlueprintRequestType &BlueprintRequestType::operator=(const BlueprintRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->blueprintInfo_ = x.blueprintInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

BlueprintRequestType::~BlueprintRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, BlueprintRequestType>
    _xsd_BlueprintRequestType_type_factory_init("blueprintRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpConfsRequestMessageType
//

CcmpConfsRequestMessageType::CcmpConfsRequestMessageType(const ConfsRequestType &confsRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), confsRequest_(confsRequest, this) {
}

CcmpConfsRequestMessageType::CcmpConfsRequestMessageType(::std::unique_ptr<ConfsRequestType> confsRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), confsRequest_(std::move(confsRequest), this) {
}

CcmpConfsRequestMessageType::CcmpConfsRequestMessageType(const CcmpConfsRequestMessageType &x,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c), confsRequest_(x.confsRequest_, f, this) {
}

CcmpConfsRequestMessageType::CcmpConfsRequestMessageType(const ::xercesc::DOMElement &e,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      confsRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpConfsRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// confsRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", &::xsd::cxx::tree::factory_impl<ConfsRequestType>,
			    true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!confsRequest_.present()) {
					::std::unique_ptr<ConfsRequestType> r(dynamic_cast<ConfsRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confsRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!confsRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpConfsRequestMessageType *
CcmpConfsRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpConfsRequestMessageType(*this, f, c);
}

CcmpConfsRequestMessageType &CcmpConfsRequestMessageType::operator=(const CcmpConfsRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->confsRequest_ = x.confsRequest_;
	}

	return *this;
}

CcmpConfsRequestMessageType::~CcmpConfsRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpConfsRequestMessageType>
    _xsd_CcmpConfsRequestMessageType_type_factory_init("ccmp-confs-request-message-type",
                                                       "urn:ietf:params:xml:ns:xcon-ccmp");

// ConfsRequestType
//

ConfsRequestType::ConfsRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      xpathFilter_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

ConfsRequestType::ConfsRequestType(const ConfsRequestType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      xpathFilter_(x.xpathFilter_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConfsRequestType::ConfsRequestType(const ::xercesc::DOMElement &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), xpathFilter_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConfsRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// xpathFilter
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "xpathFilter", "", &::xsd::cxx::tree::factory_impl<XpathFilterType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->xpathFilter_) {
					::std::unique_ptr<XpathFilterType> r(dynamic_cast<XpathFilterType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->xpathFilter_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

ConfsRequestType *ConfsRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConfsRequestType(*this, f, c);
}

ConfsRequestType &ConfsRequestType::operator=(const ConfsRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->xpathFilter_ = x.xpathFilter_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConfsRequestType::~ConfsRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConfsRequestType>
    _xsd_ConfsRequestType_type_factory_init("confsRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpConfRequestMessageType
//

CcmpConfRequestMessageType::CcmpConfRequestMessageType(const ConfRequestType &confRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), confRequest_(confRequest, this) {
}

CcmpConfRequestMessageType::CcmpConfRequestMessageType(::std::unique_ptr<ConfRequestType> confRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), confRequest_(std::move(confRequest), this) {
}

CcmpConfRequestMessageType::CcmpConfRequestMessageType(const CcmpConfRequestMessageType &x,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c), confRequest_(x.confRequest_, f, this) {
}

CcmpConfRequestMessageType::CcmpConfRequestMessageType(const ::xercesc::DOMElement &e,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      confRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpConfRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// confRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confRequest", "urn:ietf:params:xml:ns:xcon-ccmp", &::xsd::cxx::tree::factory_impl<ConfRequestType>,
			    true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!confRequest_.present()) {
					::std::unique_ptr<ConfRequestType> r(dynamic_cast<ConfRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!confRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("confRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpConfRequestMessageType *CcmpConfRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpConfRequestMessageType(*this, f, c);
}

CcmpConfRequestMessageType &CcmpConfRequestMessageType::operator=(const CcmpConfRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->confRequest_ = x.confRequest_;
	}

	return *this;
}

CcmpConfRequestMessageType::~CcmpConfRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpConfRequestMessageType>
    _xsd_CcmpConfRequestMessageType_type_factory_init("ccmp-conf-request-message-type",
                                                      "urn:ietf:params:xml:ns:xcon-ccmp");

// ConfRequestType
//

ConfRequestType::ConfRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

ConfRequestType::ConfRequestType(const ConfRequestType &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confInfo_(x.confInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConfRequestType::ConfRequestType(const ::xercesc::DOMElement &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), confInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConfRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// confInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confInfo", "", &::xsd::cxx::tree::factory_impl<ConfInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->confInfo_) {
					::std::unique_ptr<ConfInfoType> r(dynamic_cast<ConfInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

ConfRequestType *ConfRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConfRequestType(*this, f, c);
}

ConfRequestType &ConfRequestType::operator=(const ConfRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->confInfo_ = x.confInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConfRequestType::~ConfRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConfRequestType>
    _xsd_ConfRequestType_type_factory_init("confRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpUsersRequestMessageType
//

CcmpUsersRequestMessageType::CcmpUsersRequestMessageType(const UsersRequestType &usersRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), usersRequest_(usersRequest, this) {
}

CcmpUsersRequestMessageType::CcmpUsersRequestMessageType(::std::unique_ptr<UsersRequestType> usersRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), usersRequest_(std::move(usersRequest), this) {
}

CcmpUsersRequestMessageType::CcmpUsersRequestMessageType(const CcmpUsersRequestMessageType &x,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c), usersRequest_(x.usersRequest_, f, this) {
}

CcmpUsersRequestMessageType::CcmpUsersRequestMessageType(const ::xercesc::DOMElement &e,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      usersRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpUsersRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// usersRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp", &::xsd::cxx::tree::factory_impl<UsersRequestType>,
			    true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!usersRequest_.present()) {
					::std::unique_ptr<UsersRequestType> r(dynamic_cast<UsersRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->usersRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!usersRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpUsersRequestMessageType *
CcmpUsersRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpUsersRequestMessageType(*this, f, c);
}

CcmpUsersRequestMessageType &CcmpUsersRequestMessageType::operator=(const CcmpUsersRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->usersRequest_ = x.usersRequest_;
	}

	return *this;
}

CcmpUsersRequestMessageType::~CcmpUsersRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpUsersRequestMessageType>
    _xsd_CcmpUsersRequestMessageType_type_factory_init("ccmp-users-request-message-type",
                                                       "urn:ietf:params:xml:ns:xcon-ccmp");

// UsersRequestType
//

UsersRequestType::UsersRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      usersInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

UsersRequestType::UsersRequestType(const UsersRequestType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      usersInfo_(x.usersInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UsersRequestType::UsersRequestType(const ::xercesc::DOMElement &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), usersInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UsersRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// usersInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "usersInfo", "", &::xsd::cxx::tree::factory_impl<UsersInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->usersInfo_) {
					::std::unique_ptr<UsersInfoType> r(dynamic_cast<UsersInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->usersInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

UsersRequestType *UsersRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UsersRequestType(*this, f, c);
}

UsersRequestType &UsersRequestType::operator=(const UsersRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->usersInfo_ = x.usersInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UsersRequestType::~UsersRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UsersRequestType>
    _xsd_UsersRequestType_type_factory_init("usersRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpUserRequestMessageType
//

CcmpUserRequestMessageType::CcmpUserRequestMessageType(const UserRequestType &userRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), userRequest_(userRequest, this) {
}

CcmpUserRequestMessageType::CcmpUserRequestMessageType(::std::unique_ptr<UserRequestType> userRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), userRequest_(std::move(userRequest), this) {
}

CcmpUserRequestMessageType::CcmpUserRequestMessageType(const CcmpUserRequestMessageType &x,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c), userRequest_(x.userRequest_, f, this) {
}

CcmpUserRequestMessageType::CcmpUserRequestMessageType(const ::xercesc::DOMElement &e,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      userRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpUserRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// userRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "userRequest", "urn:ietf:params:xml:ns:xcon-ccmp", &::xsd::cxx::tree::factory_impl<UserRequestType>,
			    true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!userRequest_.present()) {
					::std::unique_ptr<UserRequestType> r(dynamic_cast<UserRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->userRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!userRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("userRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpUserRequestMessageType *CcmpUserRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpUserRequestMessageType(*this, f, c);
}

CcmpUserRequestMessageType &CcmpUserRequestMessageType::operator=(const CcmpUserRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->userRequest_ = x.userRequest_;
	}

	return *this;
}

CcmpUserRequestMessageType::~CcmpUserRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpUserRequestMessageType>
    _xsd_CcmpUserRequestMessageType_type_factory_init("ccmp-user-request-message-type",
                                                      "urn:ietf:params:xml:ns:xcon-ccmp");

// UserRequestType
//

UserRequestType::UserRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      userInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

UserRequestType::UserRequestType(const UserRequestType &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      userInfo_(x.userInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UserRequestType::UserRequestType(const ::xercesc::DOMElement &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), userInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UserRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// userInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "userInfo", "", &::xsd::cxx::tree::factory_impl<UserInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->userInfo_) {
					::std::unique_ptr<UserInfoType> r(dynamic_cast<UserInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->userInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

UserRequestType *UserRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UserRequestType(*this, f, c);
}

UserRequestType &UserRequestType::operator=(const UserRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->userInfo_ = x.userInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UserRequestType::~UserRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UserRequestType>
    _xsd_UserRequestType_type_factory_init("userRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpSidebarsByValRequestMessageType
//

CcmpSidebarsByValRequestMessageType::CcmpSidebarsByValRequestMessageType(
    const SidebarsByValRequestType &sidebarsByValRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), sidebarsByValRequest_(sidebarsByValRequest, this) {
}

CcmpSidebarsByValRequestMessageType::CcmpSidebarsByValRequestMessageType(
    ::std::unique_ptr<SidebarsByValRequestType> sidebarsByValRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(),
      sidebarsByValRequest_(std::move(sidebarsByValRequest), this) {
}

CcmpSidebarsByValRequestMessageType::CcmpSidebarsByValRequestMessageType(
    const CcmpSidebarsByValRequestMessageType &x,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c),
      sidebarsByValRequest_(x.sidebarsByValRequest_, f, this) {
}

CcmpSidebarsByValRequestMessageType::CcmpSidebarsByValRequestMessageType(
    const ::xercesc::DOMElement &e,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      sidebarsByValRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpSidebarsByValRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarsByValRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<SidebarsByValRequestType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!sidebarsByValRequest_.present()) {
					::std::unique_ptr<SidebarsByValRequestType> r(dynamic_cast<SidebarsByValRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarsByValRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!sidebarsByValRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpSidebarsByValRequestMessageType *
CcmpSidebarsByValRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpSidebarsByValRequestMessageType(*this, f, c);
}

CcmpSidebarsByValRequestMessageType &
CcmpSidebarsByValRequestMessageType::operator=(const CcmpSidebarsByValRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->sidebarsByValRequest_ = x.sidebarsByValRequest_;
	}

	return *this;
}

CcmpSidebarsByValRequestMessageType::~CcmpSidebarsByValRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpSidebarsByValRequestMessageType>
    _xsd_CcmpSidebarsByValRequestMessageType_type_factory_init("ccmp-sidebarsByVal-request-message-type",
                                                               "urn:ietf:params:xml:ns:xcon-ccmp");

// SidebarsByValRequestType
//

SidebarsByValRequestType::SidebarsByValRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      xpathFilter_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SidebarsByValRequestType::SidebarsByValRequestType(const SidebarsByValRequestType &x,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      xpathFilter_(x.xpathFilter_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarsByValRequestType::SidebarsByValRequestType(const ::xercesc::DOMElement &e,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), xpathFilter_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarsByValRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// xpathFilter
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "xpathFilter", "", &::xsd::cxx::tree::factory_impl<XpathFilterType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->xpathFilter_) {
					::std::unique_ptr<XpathFilterType> r(dynamic_cast<XpathFilterType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->xpathFilter_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SidebarsByValRequestType *SidebarsByValRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarsByValRequestType(*this, f, c);
}

SidebarsByValRequestType &SidebarsByValRequestType::operator=(const SidebarsByValRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->xpathFilter_ = x.xpathFilter_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarsByValRequestType::~SidebarsByValRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarsByValRequestType>
    _xsd_SidebarsByValRequestType_type_factory_init("sidebarsByValRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpSidebarsByRefRequestMessageType
//

CcmpSidebarsByRefRequestMessageType::CcmpSidebarsByRefRequestMessageType(
    const SidebarsByRefRequestType &sidebarsByRefRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), sidebarsByRefRequest_(sidebarsByRefRequest, this) {
}

CcmpSidebarsByRefRequestMessageType::CcmpSidebarsByRefRequestMessageType(
    ::std::unique_ptr<SidebarsByRefRequestType> sidebarsByRefRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(),
      sidebarsByRefRequest_(std::move(sidebarsByRefRequest), this) {
}

CcmpSidebarsByRefRequestMessageType::CcmpSidebarsByRefRequestMessageType(
    const CcmpSidebarsByRefRequestMessageType &x,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c),
      sidebarsByRefRequest_(x.sidebarsByRefRequest_, f, this) {
}

CcmpSidebarsByRefRequestMessageType::CcmpSidebarsByRefRequestMessageType(
    const ::xercesc::DOMElement &e,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      sidebarsByRefRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpSidebarsByRefRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarsByRefRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<SidebarsByRefRequestType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!sidebarsByRefRequest_.present()) {
					::std::unique_ptr<SidebarsByRefRequestType> r(dynamic_cast<SidebarsByRefRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarsByRefRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!sidebarsByRefRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpSidebarsByRefRequestMessageType *
CcmpSidebarsByRefRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpSidebarsByRefRequestMessageType(*this, f, c);
}

CcmpSidebarsByRefRequestMessageType &
CcmpSidebarsByRefRequestMessageType::operator=(const CcmpSidebarsByRefRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->sidebarsByRefRequest_ = x.sidebarsByRefRequest_;
	}

	return *this;
}

CcmpSidebarsByRefRequestMessageType::~CcmpSidebarsByRefRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpSidebarsByRefRequestMessageType>
    _xsd_CcmpSidebarsByRefRequestMessageType_type_factory_init("ccmp-sidebarsByRef-request-message-type",
                                                               "urn:ietf:params:xml:ns:xcon-ccmp");

// SidebarsByRefRequestType
//

SidebarsByRefRequestType::SidebarsByRefRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      xpathFilter_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SidebarsByRefRequestType::SidebarsByRefRequestType(const SidebarsByRefRequestType &x,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      xpathFilter_(x.xpathFilter_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarsByRefRequestType::SidebarsByRefRequestType(const ::xercesc::DOMElement &e,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), xpathFilter_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarsByRefRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// xpathFilter
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "xpathFilter", "", &::xsd::cxx::tree::factory_impl<XpathFilterType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->xpathFilter_) {
					::std::unique_ptr<XpathFilterType> r(dynamic_cast<XpathFilterType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->xpathFilter_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SidebarsByRefRequestType *SidebarsByRefRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarsByRefRequestType(*this, f, c);
}

SidebarsByRefRequestType &SidebarsByRefRequestType::operator=(const SidebarsByRefRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->xpathFilter_ = x.xpathFilter_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarsByRefRequestType::~SidebarsByRefRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarsByRefRequestType>
    _xsd_SidebarsByRefRequestType_type_factory_init("sidebarsByRefRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpSidebarByValRequestMessageType
//

CcmpSidebarByValRequestMessageType::CcmpSidebarByValRequestMessageType(
    const SidebarByValRequestType &sidebarByValRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), sidebarByValRequest_(sidebarByValRequest, this) {
}

CcmpSidebarByValRequestMessageType::CcmpSidebarByValRequestMessageType(
    ::std::unique_ptr<SidebarByValRequestType> sidebarByValRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(),
      sidebarByValRequest_(std::move(sidebarByValRequest), this) {
}

CcmpSidebarByValRequestMessageType::CcmpSidebarByValRequestMessageType(const CcmpSidebarByValRequestMessageType &x,
                                                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c),
      sidebarByValRequest_(x.sidebarByValRequest_, f, this) {
}

CcmpSidebarByValRequestMessageType::CcmpSidebarByValRequestMessageType(const ::xercesc::DOMElement &e,
                                                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      sidebarByValRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpSidebarByValRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarByValRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<SidebarByValRequestType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!sidebarByValRequest_.present()) {
					::std::unique_ptr<SidebarByValRequestType> r(dynamic_cast<SidebarByValRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarByValRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!sidebarByValRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpSidebarByValRequestMessageType *
CcmpSidebarByValRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpSidebarByValRequestMessageType(*this, f, c);
}

CcmpSidebarByValRequestMessageType &
CcmpSidebarByValRequestMessageType::operator=(const CcmpSidebarByValRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->sidebarByValRequest_ = x.sidebarByValRequest_;
	}

	return *this;
}

CcmpSidebarByValRequestMessageType::~CcmpSidebarByValRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpSidebarByValRequestMessageType>
    _xsd_CcmpSidebarByValRequestMessageType_type_factory_init("ccmp-sidebarByVal-request-message-type",
                                                              "urn:ietf:params:xml:ns:xcon-ccmp");

// SidebarByValRequestType
//

SidebarByValRequestType::SidebarByValRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarByValInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SidebarByValRequestType::SidebarByValRequestType(const SidebarByValRequestType &x,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarByValInfo_(x.sidebarByValInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarByValRequestType::SidebarByValRequestType(const ::xercesc::DOMElement &e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), sidebarByValInfo_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarByValRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarByValInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarByValInfo", "", &::xsd::cxx::tree::factory_impl<SidebarByValInfoType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->sidebarByValInfo_) {
					::std::unique_ptr<SidebarByValInfoType> r(dynamic_cast<SidebarByValInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarByValInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SidebarByValRequestType *SidebarByValRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarByValRequestType(*this, f, c);
}

SidebarByValRequestType &SidebarByValRequestType::operator=(const SidebarByValRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->sidebarByValInfo_ = x.sidebarByValInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarByValRequestType::~SidebarByValRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarByValRequestType>
    _xsd_SidebarByValRequestType_type_factory_init("sidebarByValRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpSidebarByRefRequestMessageType
//

CcmpSidebarByRefRequestMessageType::CcmpSidebarByRefRequestMessageType(
    const SidebarByRefRequestType &sidebarByRefRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), sidebarByRefRequest_(sidebarByRefRequest, this) {
}

CcmpSidebarByRefRequestMessageType::CcmpSidebarByRefRequestMessageType(
    ::std::unique_ptr<SidebarByRefRequestType> sidebarByRefRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(),
      sidebarByRefRequest_(std::move(sidebarByRefRequest), this) {
}

CcmpSidebarByRefRequestMessageType::CcmpSidebarByRefRequestMessageType(const CcmpSidebarByRefRequestMessageType &x,
                                                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c),
      sidebarByRefRequest_(x.sidebarByRefRequest_, f, this) {
}

CcmpSidebarByRefRequestMessageType::CcmpSidebarByRefRequestMessageType(const ::xercesc::DOMElement &e,
                                                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      sidebarByRefRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpSidebarByRefRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarByRefRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<SidebarByRefRequestType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!sidebarByRefRequest_.present()) {
					::std::unique_ptr<SidebarByRefRequestType> r(dynamic_cast<SidebarByRefRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarByRefRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!sidebarByRefRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpSidebarByRefRequestMessageType *
CcmpSidebarByRefRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpSidebarByRefRequestMessageType(*this, f, c);
}

CcmpSidebarByRefRequestMessageType &
CcmpSidebarByRefRequestMessageType::operator=(const CcmpSidebarByRefRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->sidebarByRefRequest_ = x.sidebarByRefRequest_;
	}

	return *this;
}

CcmpSidebarByRefRequestMessageType::~CcmpSidebarByRefRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpSidebarByRefRequestMessageType>
    _xsd_CcmpSidebarByRefRequestMessageType_type_factory_init("ccmp-sidebarByRef-request-message-type",
                                                              "urn:ietf:params:xml:ns:xcon-ccmp");

// SidebarByRefRequestType
//

SidebarByRefRequestType::SidebarByRefRequestType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarByRefInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SidebarByRefRequestType::SidebarByRefRequestType(const SidebarByRefRequestType &x,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarByRefInfo_(x.sidebarByRefInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarByRefRequestType::SidebarByRefRequestType(const ::xercesc::DOMElement &e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), sidebarByRefInfo_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarByRefRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarByRefInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarByRefInfo", "", &::xsd::cxx::tree::factory_impl<SidebarByRefInfoType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->sidebarByRefInfo_) {
					::std::unique_ptr<SidebarByRefInfoType> r(dynamic_cast<SidebarByRefInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarByRefInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SidebarByRefRequestType *SidebarByRefRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarByRefRequestType(*this, f, c);
}

SidebarByRefRequestType &SidebarByRefRequestType::operator=(const SidebarByRefRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->sidebarByRefInfo_ = x.sidebarByRefInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarByRefRequestType::~SidebarByRefRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarByRefRequestType>
    _xsd_SidebarByRefRequestType_type_factory_init("sidebarByRefRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpExtendedRequestMessageType
//

CcmpExtendedRequestMessageType::CcmpExtendedRequestMessageType(const ExtendedRequestType &extendedRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), extendedRequest_(extendedRequest, this) {
}

CcmpExtendedRequestMessageType::CcmpExtendedRequestMessageType(::std::unique_ptr<ExtendedRequestType> extendedRequest)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(), extendedRequest_(std::move(extendedRequest), this) {
}

CcmpExtendedRequestMessageType::CcmpExtendedRequestMessageType(const CcmpExtendedRequestMessageType &x,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c), extendedRequest_(x.extendedRequest_, f, this) {
}

CcmpExtendedRequestMessageType::CcmpExtendedRequestMessageType(const ::xercesc::DOMElement &e,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      extendedRequest_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpExtendedRequestMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// extendedRequest
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<ExtendedRequestType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!extendedRequest_.present()) {
					::std::unique_ptr<ExtendedRequestType> r(dynamic_cast<ExtendedRequestType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->extendedRequest_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!extendedRequest_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpExtendedRequestMessageType *
CcmpExtendedRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpExtendedRequestMessageType(*this, f, c);
}

CcmpExtendedRequestMessageType &CcmpExtendedRequestMessageType::operator=(const CcmpExtendedRequestMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(*this) = x;
		this->extendedRequest_ = x.extendedRequest_;
	}

	return *this;
}

CcmpExtendedRequestMessageType::~CcmpExtendedRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpExtendedRequestMessageType>
    _xsd_CcmpExtendedRequestMessageType_type_factory_init("ccmp-extended-request-message-type",
                                                          "urn:ietf:params:xml:ns:xcon-ccmp");

// ExtendedRequestType
//

ExtendedRequestType::ExtendedRequestType(const ExtensionNameType &extensionName)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      extensionName_(extensionName, this), any_(this->getDomDocument()) {
}

ExtendedRequestType::ExtendedRequestType(::std::unique_ptr<ExtensionNameType> extensionName)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      extensionName_(std::move(extensionName), this), any_(this->getDomDocument()) {
}

ExtendedRequestType::ExtendedRequestType(const ExtendedRequestType &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      extensionName_(x.extensionName_, f, this), any_(x.any_, this->getDomDocument()) {
}

ExtendedRequestType::ExtendedRequestType(const ::xercesc::DOMElement &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), extensionName_(this), any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void ExtendedRequestType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// extensionName
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "extensionName", "", &::xsd::cxx::tree::factory_impl<ExtensionNameType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!extensionName_.present()) {
					::std::unique_ptr<ExtensionNameType> r(dynamic_cast<ExtensionNameType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->extensionName_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!extensionName_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("extensionName", "");
	}
}

ExtendedRequestType *ExtendedRequestType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ExtendedRequestType(*this, f, c);
}

ExtendedRequestType &ExtendedRequestType::operator=(const ExtendedRequestType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->extensionName_ = x.extensionName_;
		this->any_ = x.any_;
	}

	return *this;
}

ExtendedRequestType::~ExtendedRequestType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ExtendedRequestType>
    _xsd_ExtendedRequestType_type_factory_init("extendedRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpOptionsRequestMessageType
//

CcmpOptionsRequestMessageType::CcmpOptionsRequestMessageType()
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType() {
}

CcmpOptionsRequestMessageType::CcmpOptionsRequestMessageType(const CcmpOptionsRequestMessageType &x,
                                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(x, f, c) {
}

CcmpOptionsRequestMessageType::CcmpOptionsRequestMessageType(const ::xercesc::DOMElement &e,
                                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType(e, f, c) {
}

CcmpOptionsRequestMessageType *
CcmpOptionsRequestMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                      ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpOptionsRequestMessageType(*this, f, c);
}

CcmpOptionsRequestMessageType::~CcmpOptionsRequestMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpOptionsRequestMessageType>
    _xsd_CcmpOptionsRequestMessageType_type_factory_init("ccmp-options-request-message-type",
                                                         "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpBlueprintsResponseMessageType
//

CcmpBlueprintsResponseMessageType::CcmpBlueprintsResponseMessageType(const ConfUserIDType &confUserID,
                                                                     const ResponseCodeType &response_code,
                                                                     const BlueprintsResponseType &blueprintsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      blueprintsResponse_(blueprintsResponse, this) {
}

CcmpBlueprintsResponseMessageType::CcmpBlueprintsResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    ::std::unique_ptr<BlueprintsResponseType> blueprintsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      blueprintsResponse_(std::move(blueprintsResponse), this) {
}

CcmpBlueprintsResponseMessageType::CcmpBlueprintsResponseMessageType(
    ::std::unique_ptr<ConfUserIDType> confUserID,
    ::std::unique_ptr<ResponseCodeType> response_code,
    ::std::unique_ptr<BlueprintsResponseType> blueprintsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      blueprintsResponse_(std::move(blueprintsResponse), this) {
}

CcmpBlueprintsResponseMessageType::CcmpBlueprintsResponseMessageType(const CcmpBlueprintsResponseMessageType &x,
                                                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c),
      blueprintsResponse_(x.blueprintsResponse_, f, this) {
}

CcmpBlueprintsResponseMessageType::CcmpBlueprintsResponseMessageType(const ::xercesc::DOMElement &e,
                                                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      blueprintsResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpBlueprintsResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// blueprintsResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<BlueprintsResponseType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!blueprintsResponse_.present()) {
					::std::unique_ptr<BlueprintsResponseType> r(dynamic_cast<BlueprintsResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->blueprintsResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!blueprintsResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpBlueprintsResponseMessageType *
CcmpBlueprintsResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                          ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpBlueprintsResponseMessageType(*this, f, c);
}

CcmpBlueprintsResponseMessageType &
CcmpBlueprintsResponseMessageType::operator=(const CcmpBlueprintsResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->blueprintsResponse_ = x.blueprintsResponse_;
	}

	return *this;
}

CcmpBlueprintsResponseMessageType::~CcmpBlueprintsResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpBlueprintsResponseMessageType>
    _xsd_CcmpBlueprintsResponseMessageType_type_factory_init("ccmp-blueprints-response-message-type",
                                                             "urn:ietf:params:xml:ns:xcon-ccmp");

// BlueprintsResponseType
//

BlueprintsResponseType::BlueprintsResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      blueprintsInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

BlueprintsResponseType::BlueprintsResponseType(const BlueprintsResponseType &x,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      blueprintsInfo_(x.blueprintsInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

BlueprintsResponseType::BlueprintsResponseType(const ::xercesc::DOMElement &e,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), blueprintsInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void BlueprintsResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// blueprintsInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "blueprintsInfo", "", &::xsd::cxx::tree::factory_impl<BlueprintsInfoType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->blueprintsInfo_) {
					::std::unique_ptr<BlueprintsInfoType> r(dynamic_cast<BlueprintsInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->blueprintsInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

BlueprintsResponseType *BlueprintsResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class BlueprintsResponseType(*this, f, c);
}

BlueprintsResponseType &BlueprintsResponseType::operator=(const BlueprintsResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->blueprintsInfo_ = x.blueprintsInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

BlueprintsResponseType::~BlueprintsResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, BlueprintsResponseType>
    _xsd_BlueprintsResponseType_type_factory_init("blueprintsResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpBlueprintResponseMessageType
//

CcmpBlueprintResponseMessageType::CcmpBlueprintResponseMessageType(const ConfUserIDType &confUserID,
                                                                   const ResponseCodeType &response_code,
                                                                   const BlueprintResponseType &blueprintResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      blueprintResponse_(blueprintResponse, this) {
}

CcmpBlueprintResponseMessageType::CcmpBlueprintResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    ::std::unique_ptr<BlueprintResponseType> blueprintResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      blueprintResponse_(std::move(blueprintResponse), this) {
}

CcmpBlueprintResponseMessageType::CcmpBlueprintResponseMessageType(
    ::std::unique_ptr<ConfUserIDType> confUserID,
    ::std::unique_ptr<ResponseCodeType> response_code,
    ::std::unique_ptr<BlueprintResponseType> blueprintResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      blueprintResponse_(std::move(blueprintResponse), this) {
}

CcmpBlueprintResponseMessageType::CcmpBlueprintResponseMessageType(const CcmpBlueprintResponseMessageType &x,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c),
      blueprintResponse_(x.blueprintResponse_, f, this) {
}

CcmpBlueprintResponseMessageType::CcmpBlueprintResponseMessageType(const ::xercesc::DOMElement &e,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      blueprintResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpBlueprintResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// blueprintResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<BlueprintResponseType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!blueprintResponse_.present()) {
					::std::unique_ptr<BlueprintResponseType> r(dynamic_cast<BlueprintResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->blueprintResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!blueprintResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpBlueprintResponseMessageType *
CcmpBlueprintResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpBlueprintResponseMessageType(*this, f, c);
}

CcmpBlueprintResponseMessageType &
CcmpBlueprintResponseMessageType::operator=(const CcmpBlueprintResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->blueprintResponse_ = x.blueprintResponse_;
	}

	return *this;
}

CcmpBlueprintResponseMessageType::~CcmpBlueprintResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpBlueprintResponseMessageType>
    _xsd_CcmpBlueprintResponseMessageType_type_factory_init("ccmp-blueprint-response-message-type",
                                                            "urn:ietf:params:xml:ns:xcon-ccmp");

// BlueprintResponseType
//

BlueprintResponseType::BlueprintResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      blueprintInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

BlueprintResponseType::BlueprintResponseType(const BlueprintResponseType &x,
                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      blueprintInfo_(x.blueprintInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

BlueprintResponseType::BlueprintResponseType(const ::xercesc::DOMElement &e,
                                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), blueprintInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void BlueprintResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// blueprintInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "blueprintInfo", "", &::xsd::cxx::tree::factory_impl<BlueprintInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->blueprintInfo_) {
					::std::unique_ptr<BlueprintInfoType> r(dynamic_cast<BlueprintInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->blueprintInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

BlueprintResponseType *BlueprintResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class BlueprintResponseType(*this, f, c);
}

BlueprintResponseType &BlueprintResponseType::operator=(const BlueprintResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->blueprintInfo_ = x.blueprintInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

BlueprintResponseType::~BlueprintResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, BlueprintResponseType>
    _xsd_BlueprintResponseType_type_factory_init("blueprintResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpConfsResponseMessageType
//

CcmpConfsResponseMessageType::CcmpConfsResponseMessageType(const ConfUserIDType &confUserID,
                                                           const ResponseCodeType &response_code,
                                                           const ConfsResponseType &confsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      confsResponse_(confsResponse, this) {
}

CcmpConfsResponseMessageType::CcmpConfsResponseMessageType(const ConfUserIDType &confUserID,
                                                           const ResponseCodeType &response_code,
                                                           ::std::unique_ptr<ConfsResponseType> confsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      confsResponse_(std::move(confsResponse), this) {
}

CcmpConfsResponseMessageType::CcmpConfsResponseMessageType(::std::unique_ptr<ConfUserIDType> confUserID,
                                                           ::std::unique_ptr<ResponseCodeType> response_code,
                                                           ::std::unique_ptr<ConfsResponseType> confsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      confsResponse_(std::move(confsResponse), this) {
}

CcmpConfsResponseMessageType::CcmpConfsResponseMessageType(const CcmpConfsResponseMessageType &x,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c), confsResponse_(x.confsResponse_, f, this) {
}

CcmpConfsResponseMessageType::CcmpConfsResponseMessageType(const ::xercesc::DOMElement &e,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      confsResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpConfsResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// confsResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", &::xsd::cxx::tree::factory_impl<ConfsResponseType>,
			    true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!confsResponse_.present()) {
					::std::unique_ptr<ConfsResponseType> r(dynamic_cast<ConfsResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confsResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!confsResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpConfsResponseMessageType *
CcmpConfsResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpConfsResponseMessageType(*this, f, c);
}

CcmpConfsResponseMessageType &CcmpConfsResponseMessageType::operator=(const CcmpConfsResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->confsResponse_ = x.confsResponse_;
	}

	return *this;
}

CcmpConfsResponseMessageType::~CcmpConfsResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpConfsResponseMessageType>
    _xsd_CcmpConfsResponseMessageType_type_factory_init("ccmp-confs-response-message-type",
                                                        "urn:ietf:params:xml:ns:xcon-ccmp");

// ConfsResponseType
//

ConfsResponseType::ConfsResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confsInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

ConfsResponseType::ConfsResponseType(const ConfsResponseType &x,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confsInfo_(x.confsInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConfsResponseType::ConfsResponseType(const ::xercesc::DOMElement &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), confsInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConfsResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// confsInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confsInfo", "", &::xsd::cxx::tree::factory_impl<ConfsInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->confsInfo_) {
					::std::unique_ptr<ConfsInfoType> r(dynamic_cast<ConfsInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confsInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

ConfsResponseType *ConfsResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConfsResponseType(*this, f, c);
}

ConfsResponseType &ConfsResponseType::operator=(const ConfsResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->confsInfo_ = x.confsInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConfsResponseType::~ConfsResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConfsResponseType>
    _xsd_ConfsResponseType_type_factory_init("confsResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpConfResponseMessageType
//

CcmpConfResponseMessageType::CcmpConfResponseMessageType(const ConfUserIDType &confUserID,
                                                         const ResponseCodeType &response_code,
                                                         const ConfResponseType &confResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      confResponse_(confResponse, this) {
}

CcmpConfResponseMessageType::CcmpConfResponseMessageType(const ConfUserIDType &confUserID,
                                                         const ResponseCodeType &response_code,
                                                         ::std::unique_ptr<ConfResponseType> confResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      confResponse_(std::move(confResponse), this) {
}

CcmpConfResponseMessageType::CcmpConfResponseMessageType(::std::unique_ptr<ConfUserIDType> confUserID,
                                                         ::std::unique_ptr<ResponseCodeType> response_code,
                                                         ::std::unique_ptr<ConfResponseType> confResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      confResponse_(std::move(confResponse), this) {
}

CcmpConfResponseMessageType::CcmpConfResponseMessageType(const CcmpConfResponseMessageType &x,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c), confResponse_(x.confResponse_, f, this) {
}

CcmpConfResponseMessageType::CcmpConfResponseMessageType(const ::xercesc::DOMElement &e,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      confResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpConfResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// confResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confResponse", "urn:ietf:params:xml:ns:xcon-ccmp", &::xsd::cxx::tree::factory_impl<ConfResponseType>,
			    true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!confResponse_.present()) {
					::std::unique_ptr<ConfResponseType> r(dynamic_cast<ConfResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!confResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("confResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpConfResponseMessageType *
CcmpConfResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpConfResponseMessageType(*this, f, c);
}

CcmpConfResponseMessageType &CcmpConfResponseMessageType::operator=(const CcmpConfResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->confResponse_ = x.confResponse_;
	}

	return *this;
}

CcmpConfResponseMessageType::~CcmpConfResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpConfResponseMessageType>
    _xsd_CcmpConfResponseMessageType_type_factory_init("ccmp-conf-response-message-type",
                                                       "urn:ietf:params:xml:ns:xcon-ccmp");

// ConfResponseType
//

ConfResponseType::ConfResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

ConfResponseType::ConfResponseType(const ConfResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      confInfo_(x.confInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ConfResponseType::ConfResponseType(const ::xercesc::DOMElement &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), confInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ConfResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// confInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "confInfo", "", &::xsd::cxx::tree::factory_impl<ConfInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->confInfo_) {
					::std::unique_ptr<ConfInfoType> r(dynamic_cast<ConfInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->confInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

ConfResponseType *ConfResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ConfResponseType(*this, f, c);
}

ConfResponseType &ConfResponseType::operator=(const ConfResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->confInfo_ = x.confInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ConfResponseType::~ConfResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ConfResponseType>
    _xsd_ConfResponseType_type_factory_init("confResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpUsersResponseMessageType
//

CcmpUsersResponseMessageType::CcmpUsersResponseMessageType(const ConfUserIDType &confUserID,
                                                           const ResponseCodeType &response_code,
                                                           const UsersResponseType &usersResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      usersResponse_(usersResponse, this) {
}

CcmpUsersResponseMessageType::CcmpUsersResponseMessageType(const ConfUserIDType &confUserID,
                                                           const ResponseCodeType &response_code,
                                                           ::std::unique_ptr<UsersResponseType> usersResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      usersResponse_(std::move(usersResponse), this) {
}

CcmpUsersResponseMessageType::CcmpUsersResponseMessageType(::std::unique_ptr<ConfUserIDType> confUserID,
                                                           ::std::unique_ptr<ResponseCodeType> response_code,
                                                           ::std::unique_ptr<UsersResponseType> usersResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      usersResponse_(std::move(usersResponse), this) {
}

CcmpUsersResponseMessageType::CcmpUsersResponseMessageType(const CcmpUsersResponseMessageType &x,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c), usersResponse_(x.usersResponse_, f, this) {
}

CcmpUsersResponseMessageType::CcmpUsersResponseMessageType(const ::xercesc::DOMElement &e,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      usersResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpUsersResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// usersResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp", &::xsd::cxx::tree::factory_impl<UsersResponseType>,
			    true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!usersResponse_.present()) {
					::std::unique_ptr<UsersResponseType> r(dynamic_cast<UsersResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->usersResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!usersResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpUsersResponseMessageType *
CcmpUsersResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpUsersResponseMessageType(*this, f, c);
}

CcmpUsersResponseMessageType &CcmpUsersResponseMessageType::operator=(const CcmpUsersResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->usersResponse_ = x.usersResponse_;
	}

	return *this;
}

CcmpUsersResponseMessageType::~CcmpUsersResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpUsersResponseMessageType>
    _xsd_CcmpUsersResponseMessageType_type_factory_init("ccmp-users-response-message-type",
                                                        "urn:ietf:params:xml:ns:xcon-ccmp");

// UsersResponseType
//

UsersResponseType::UsersResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      usersInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

UsersResponseType::UsersResponseType(const UsersResponseType &x,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      usersInfo_(x.usersInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UsersResponseType::UsersResponseType(const ::xercesc::DOMElement &e,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), usersInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UsersResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// usersInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "usersInfo", "", &::xsd::cxx::tree::factory_impl<UsersInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->usersInfo_) {
					::std::unique_ptr<UsersInfoType> r(dynamic_cast<UsersInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->usersInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

UsersResponseType *UsersResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UsersResponseType(*this, f, c);
}

UsersResponseType &UsersResponseType::operator=(const UsersResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->usersInfo_ = x.usersInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UsersResponseType::~UsersResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UsersResponseType>
    _xsd_UsersResponseType_type_factory_init("usersResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpUserResponseMessageType
//

CcmpUserResponseMessageType::CcmpUserResponseMessageType(const ConfUserIDType &confUserID,
                                                         const ResponseCodeType &response_code,
                                                         const UserResponseType &userResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      userResponse_(userResponse, this) {
}

CcmpUserResponseMessageType::CcmpUserResponseMessageType(const ConfUserIDType &confUserID,
                                                         const ResponseCodeType &response_code,
                                                         ::std::unique_ptr<UserResponseType> userResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      userResponse_(std::move(userResponse), this) {
}

CcmpUserResponseMessageType::CcmpUserResponseMessageType(::std::unique_ptr<ConfUserIDType> confUserID,
                                                         ::std::unique_ptr<ResponseCodeType> response_code,
                                                         ::std::unique_ptr<UserResponseType> userResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      userResponse_(std::move(userResponse), this) {
}

CcmpUserResponseMessageType::CcmpUserResponseMessageType(const CcmpUserResponseMessageType &x,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c), userResponse_(x.userResponse_, f, this) {
}

CcmpUserResponseMessageType::CcmpUserResponseMessageType(const ::xercesc::DOMElement &e,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      userResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpUserResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                        ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// userResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "userResponse", "urn:ietf:params:xml:ns:xcon-ccmp", &::xsd::cxx::tree::factory_impl<UserResponseType>,
			    true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!userResponse_.present()) {
					::std::unique_ptr<UserResponseType> r(dynamic_cast<UserResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->userResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!userResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("userResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpUserResponseMessageType *
CcmpUserResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpUserResponseMessageType(*this, f, c);
}

CcmpUserResponseMessageType &CcmpUserResponseMessageType::operator=(const CcmpUserResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->userResponse_ = x.userResponse_;
	}

	return *this;
}

CcmpUserResponseMessageType::~CcmpUserResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpUserResponseMessageType>
    _xsd_CcmpUserResponseMessageType_type_factory_init("ccmp-user-response-message-type",
                                                       "urn:ietf:params:xml:ns:xcon-ccmp");

// UserResponseType
//

UserResponseType::UserResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      userInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

UserResponseType::UserResponseType(const UserResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      userInfo_(x.userInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

UserResponseType::UserResponseType(const ::xercesc::DOMElement &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), userInfo_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void UserResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// userInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "userInfo", "", &::xsd::cxx::tree::factory_impl<UserInfoType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->userInfo_) {
					::std::unique_ptr<UserInfoType> r(dynamic_cast<UserInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->userInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

UserResponseType *UserResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class UserResponseType(*this, f, c);
}

UserResponseType &UserResponseType::operator=(const UserResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->userInfo_ = x.userInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

UserResponseType::~UserResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, UserResponseType>
    _xsd_UserResponseType_type_factory_init("userResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpSidebarsByValResponseMessageType
//

CcmpSidebarsByValResponseMessageType::CcmpSidebarsByValResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    const SidebarsByValResponseType &sidebarsByValResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      sidebarsByValResponse_(sidebarsByValResponse, this) {
}

CcmpSidebarsByValResponseMessageType::CcmpSidebarsByValResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    ::std::unique_ptr<SidebarsByValResponseType> sidebarsByValResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      sidebarsByValResponse_(std::move(sidebarsByValResponse), this) {
}

CcmpSidebarsByValResponseMessageType::CcmpSidebarsByValResponseMessageType(
    ::std::unique_ptr<ConfUserIDType> confUserID,
    ::std::unique_ptr<ResponseCodeType> response_code,
    ::std::unique_ptr<SidebarsByValResponseType> sidebarsByValResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      sidebarsByValResponse_(std::move(sidebarsByValResponse), this) {
}

CcmpSidebarsByValResponseMessageType::CcmpSidebarsByValResponseMessageType(
    const CcmpSidebarsByValResponseMessageType &x,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c),
      sidebarsByValResponse_(x.sidebarsByValResponse_, f, this) {
}

CcmpSidebarsByValResponseMessageType::CcmpSidebarsByValResponseMessageType(
    const ::xercesc::DOMElement &e,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      sidebarsByValResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpSidebarsByValResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarsByValResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<SidebarsByValResponseType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!sidebarsByValResponse_.present()) {
					::std::unique_ptr<SidebarsByValResponseType> r(
					    dynamic_cast<SidebarsByValResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarsByValResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!sidebarsByValResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpSidebarsByValResponseMessageType *
CcmpSidebarsByValResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpSidebarsByValResponseMessageType(*this, f, c);
}

CcmpSidebarsByValResponseMessageType &
CcmpSidebarsByValResponseMessageType::operator=(const CcmpSidebarsByValResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->sidebarsByValResponse_ = x.sidebarsByValResponse_;
	}

	return *this;
}

CcmpSidebarsByValResponseMessageType::~CcmpSidebarsByValResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpSidebarsByValResponseMessageType>
    _xsd_CcmpSidebarsByValResponseMessageType_type_factory_init("ccmp-sidebarsByVal-response-message-type",
                                                                "urn:ietf:params:xml:ns:xcon-ccmp");

// SidebarsByValResponseType
//

SidebarsByValResponseType::SidebarsByValResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarsByValInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SidebarsByValResponseType::SidebarsByValResponseType(const SidebarsByValResponseType &x,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarsByValInfo_(x.sidebarsByValInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarsByValResponseType::SidebarsByValResponseType(const ::xercesc::DOMElement &e,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), sidebarsByValInfo_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarsByValResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarsByValInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarsByValInfo", "", &::xsd::cxx::tree::factory_impl<SidebarsByValInfoType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->sidebarsByValInfo_) {
					::std::unique_ptr<SidebarsByValInfoType> r(dynamic_cast<SidebarsByValInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarsByValInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SidebarsByValResponseType *SidebarsByValResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarsByValResponseType(*this, f, c);
}

SidebarsByValResponseType &SidebarsByValResponseType::operator=(const SidebarsByValResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->sidebarsByValInfo_ = x.sidebarsByValInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarsByValResponseType::~SidebarsByValResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarsByValResponseType>
    _xsd_SidebarsByValResponseType_type_factory_init("sidebarsByValResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpSidebarsByRefResponseMessageType
//

CcmpSidebarsByRefResponseMessageType::CcmpSidebarsByRefResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    const SidebarsByRefResponseType &sidebarsByRefResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      sidebarsByRefResponse_(sidebarsByRefResponse, this) {
}

CcmpSidebarsByRefResponseMessageType::CcmpSidebarsByRefResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    ::std::unique_ptr<SidebarsByRefResponseType> sidebarsByRefResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      sidebarsByRefResponse_(std::move(sidebarsByRefResponse), this) {
}

CcmpSidebarsByRefResponseMessageType::CcmpSidebarsByRefResponseMessageType(
    ::std::unique_ptr<ConfUserIDType> confUserID,
    ::std::unique_ptr<ResponseCodeType> response_code,
    ::std::unique_ptr<SidebarsByRefResponseType> sidebarsByRefResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      sidebarsByRefResponse_(std::move(sidebarsByRefResponse), this) {
}

CcmpSidebarsByRefResponseMessageType::CcmpSidebarsByRefResponseMessageType(
    const CcmpSidebarsByRefResponseMessageType &x,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c),
      sidebarsByRefResponse_(x.sidebarsByRefResponse_, f, this) {
}

CcmpSidebarsByRefResponseMessageType::CcmpSidebarsByRefResponseMessageType(
    const ::xercesc::DOMElement &e,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      sidebarsByRefResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpSidebarsByRefResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarsByRefResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<SidebarsByRefResponseType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!sidebarsByRefResponse_.present()) {
					::std::unique_ptr<SidebarsByRefResponseType> r(
					    dynamic_cast<SidebarsByRefResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarsByRefResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!sidebarsByRefResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpSidebarsByRefResponseMessageType *
CcmpSidebarsByRefResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpSidebarsByRefResponseMessageType(*this, f, c);
}

CcmpSidebarsByRefResponseMessageType &
CcmpSidebarsByRefResponseMessageType::operator=(const CcmpSidebarsByRefResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->sidebarsByRefResponse_ = x.sidebarsByRefResponse_;
	}

	return *this;
}

CcmpSidebarsByRefResponseMessageType::~CcmpSidebarsByRefResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpSidebarsByRefResponseMessageType>
    _xsd_CcmpSidebarsByRefResponseMessageType_type_factory_init("ccmp-sidebarsByRef-response-message-type",
                                                                "urn:ietf:params:xml:ns:xcon-ccmp");

// SidebarsByRefResponseType
//

SidebarsByRefResponseType::SidebarsByRefResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarsByRefInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SidebarsByRefResponseType::SidebarsByRefResponseType(const SidebarsByRefResponseType &x,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarsByRefInfo_(x.sidebarsByRefInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarsByRefResponseType::SidebarsByRefResponseType(const ::xercesc::DOMElement &e,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), sidebarsByRefInfo_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarsByRefResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarsByRefInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarsByRefInfo", "", &::xsd::cxx::tree::factory_impl<SidebarsByRefInfoType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->sidebarsByRefInfo_) {
					::std::unique_ptr<SidebarsByRefInfoType> r(dynamic_cast<SidebarsByRefInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarsByRefInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SidebarsByRefResponseType *SidebarsByRefResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarsByRefResponseType(*this, f, c);
}

SidebarsByRefResponseType &SidebarsByRefResponseType::operator=(const SidebarsByRefResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->sidebarsByRefInfo_ = x.sidebarsByRefInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarsByRefResponseType::~SidebarsByRefResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarsByRefResponseType>
    _xsd_SidebarsByRefResponseType_type_factory_init("sidebarsByRefResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpSidebarByValResponseMessageType
//

CcmpSidebarByValResponseMessageType::CcmpSidebarByValResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    const SidebarByValResponseType &sidebarByValResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      sidebarByValResponse_(sidebarByValResponse, this) {
}

CcmpSidebarByValResponseMessageType::CcmpSidebarByValResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    ::std::unique_ptr<SidebarByValResponseType> sidebarByValResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      sidebarByValResponse_(std::move(sidebarByValResponse), this) {
}

CcmpSidebarByValResponseMessageType::CcmpSidebarByValResponseMessageType(
    ::std::unique_ptr<ConfUserIDType> confUserID,
    ::std::unique_ptr<ResponseCodeType> response_code,
    ::std::unique_ptr<SidebarByValResponseType> sidebarByValResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      sidebarByValResponse_(std::move(sidebarByValResponse), this) {
}

CcmpSidebarByValResponseMessageType::CcmpSidebarByValResponseMessageType(
    const CcmpSidebarByValResponseMessageType &x,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c),
      sidebarByValResponse_(x.sidebarByValResponse_, f, this) {
}

CcmpSidebarByValResponseMessageType::CcmpSidebarByValResponseMessageType(
    const ::xercesc::DOMElement &e,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      sidebarByValResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpSidebarByValResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarByValResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<SidebarByValResponseType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!sidebarByValResponse_.present()) {
					::std::unique_ptr<SidebarByValResponseType> r(dynamic_cast<SidebarByValResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarByValResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!sidebarByValResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpSidebarByValResponseMessageType *
CcmpSidebarByValResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpSidebarByValResponseMessageType(*this, f, c);
}

CcmpSidebarByValResponseMessageType &
CcmpSidebarByValResponseMessageType::operator=(const CcmpSidebarByValResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->sidebarByValResponse_ = x.sidebarByValResponse_;
	}

	return *this;
}

CcmpSidebarByValResponseMessageType::~CcmpSidebarByValResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpSidebarByValResponseMessageType>
    _xsd_CcmpSidebarByValResponseMessageType_type_factory_init("ccmp-sidebarByVal-response-message-type",
                                                               "urn:ietf:params:xml:ns:xcon-ccmp");

// SidebarByValResponseType
//

SidebarByValResponseType::SidebarByValResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarByValInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SidebarByValResponseType::SidebarByValResponseType(const SidebarByValResponseType &x,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarByValInfo_(x.sidebarByValInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarByValResponseType::SidebarByValResponseType(const ::xercesc::DOMElement &e,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), sidebarByValInfo_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarByValResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarByValInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarByValInfo", "", &::xsd::cxx::tree::factory_impl<SidebarByValInfoType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->sidebarByValInfo_) {
					::std::unique_ptr<SidebarByValInfoType> r(dynamic_cast<SidebarByValInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarByValInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SidebarByValResponseType *SidebarByValResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarByValResponseType(*this, f, c);
}

SidebarByValResponseType &SidebarByValResponseType::operator=(const SidebarByValResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->sidebarByValInfo_ = x.sidebarByValInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarByValResponseType::~SidebarByValResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarByValResponseType>
    _xsd_SidebarByValResponseType_type_factory_init("sidebarByValResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpSidebarByRefResponseMessageType
//

CcmpSidebarByRefResponseMessageType::CcmpSidebarByRefResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    const SidebarByRefResponseType &sidebarByRefResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      sidebarByRefResponse_(sidebarByRefResponse, this) {
}

CcmpSidebarByRefResponseMessageType::CcmpSidebarByRefResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    ::std::unique_ptr<SidebarByRefResponseType> sidebarByRefResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      sidebarByRefResponse_(std::move(sidebarByRefResponse), this) {
}

CcmpSidebarByRefResponseMessageType::CcmpSidebarByRefResponseMessageType(
    ::std::unique_ptr<ConfUserIDType> confUserID,
    ::std::unique_ptr<ResponseCodeType> response_code,
    ::std::unique_ptr<SidebarByRefResponseType> sidebarByRefResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      sidebarByRefResponse_(std::move(sidebarByRefResponse), this) {
}

CcmpSidebarByRefResponseMessageType::CcmpSidebarByRefResponseMessageType(
    const CcmpSidebarByRefResponseMessageType &x,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c),
      sidebarByRefResponse_(x.sidebarByRefResponse_, f, this) {
}

CcmpSidebarByRefResponseMessageType::CcmpSidebarByRefResponseMessageType(
    const ::xercesc::DOMElement &e,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f,
    ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      sidebarByRefResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpSidebarByRefResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarByRefResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<SidebarByRefResponseType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!sidebarByRefResponse_.present()) {
					::std::unique_ptr<SidebarByRefResponseType> r(dynamic_cast<SidebarByRefResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarByRefResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!sidebarByRefResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpSidebarByRefResponseMessageType *
CcmpSidebarByRefResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpSidebarByRefResponseMessageType(*this, f, c);
}

CcmpSidebarByRefResponseMessageType &
CcmpSidebarByRefResponseMessageType::operator=(const CcmpSidebarByRefResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->sidebarByRefResponse_ = x.sidebarByRefResponse_;
	}

	return *this;
}

CcmpSidebarByRefResponseMessageType::~CcmpSidebarByRefResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpSidebarByRefResponseMessageType>
    _xsd_CcmpSidebarByRefResponseMessageType_type_factory_init("ccmp-sidebarByRef-response-message-type",
                                                               "urn:ietf:params:xml:ns:xcon-ccmp");

// SidebarByRefResponseType
//

SidebarByRefResponseType::SidebarByRefResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarByRefInfo_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SidebarByRefResponseType::SidebarByRefResponseType(const SidebarByRefResponseType &x,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      sidebarByRefInfo_(x.sidebarByRefInfo_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SidebarByRefResponseType::SidebarByRefResponseType(const ::xercesc::DOMElement &e,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), sidebarByRefInfo_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SidebarByRefResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// sidebarByRefInfo
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "sidebarByRefInfo", "", &::xsd::cxx::tree::factory_impl<SidebarByRefInfoType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->sidebarByRefInfo_) {
					::std::unique_ptr<SidebarByRefInfoType> r(dynamic_cast<SidebarByRefInfoType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->sidebarByRefInfo_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SidebarByRefResponseType *SidebarByRefResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SidebarByRefResponseType(*this, f, c);
}

SidebarByRefResponseType &SidebarByRefResponseType::operator=(const SidebarByRefResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->sidebarByRefInfo_ = x.sidebarByRefInfo_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SidebarByRefResponseType::~SidebarByRefResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SidebarByRefResponseType>
    _xsd_SidebarByRefResponseType_type_factory_init("sidebarByRefResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpExtendedResponseMessageType
//

CcmpExtendedResponseMessageType::CcmpExtendedResponseMessageType(const ConfUserIDType &confUserID,
                                                                 const ResponseCodeType &response_code,
                                                                 const ExtendedResponseType &extendedResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      extendedResponse_(extendedResponse, this) {
}

CcmpExtendedResponseMessageType::CcmpExtendedResponseMessageType(
    const ConfUserIDType &confUserID,
    const ResponseCodeType &response_code,
    ::std::unique_ptr<ExtendedResponseType> extendedResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      extendedResponse_(std::move(extendedResponse), this) {
}

CcmpExtendedResponseMessageType::CcmpExtendedResponseMessageType(
    ::std::unique_ptr<ConfUserIDType> confUserID,
    ::std::unique_ptr<ResponseCodeType> response_code,
    ::std::unique_ptr<ExtendedResponseType> extendedResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      extendedResponse_(std::move(extendedResponse), this) {
}

CcmpExtendedResponseMessageType::CcmpExtendedResponseMessageType(const CcmpExtendedResponseMessageType &x,
                                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c),
      extendedResponse_(x.extendedResponse_, f, this) {
}

CcmpExtendedResponseMessageType::CcmpExtendedResponseMessageType(const ::xercesc::DOMElement &e,
                                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      extendedResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpExtendedResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// extendedResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<ExtendedResponseType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!extendedResponse_.present()) {
					::std::unique_ptr<ExtendedResponseType> r(dynamic_cast<ExtendedResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->extendedResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!extendedResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpExtendedResponseMessageType *
CcmpExtendedResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpExtendedResponseMessageType(*this, f, c);
}

CcmpExtendedResponseMessageType &CcmpExtendedResponseMessageType::operator=(const CcmpExtendedResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->extendedResponse_ = x.extendedResponse_;
	}

	return *this;
}

CcmpExtendedResponseMessageType::~CcmpExtendedResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpExtendedResponseMessageType>
    _xsd_CcmpExtendedResponseMessageType_type_factory_init("ccmp-extended-response-message-type",
                                                           "urn:ietf:params:xml:ns:xcon-ccmp");

// ExtendedResponseType
//

ExtendedResponseType::ExtendedResponseType(const ExtensionNameType &extensionName)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      extensionName_(extensionName, this), any_(this->getDomDocument()) {
}

ExtendedResponseType::ExtendedResponseType(::std::unique_ptr<ExtensionNameType> extensionName)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      extensionName_(std::move(extensionName), this), any_(this->getDomDocument()) {
}

ExtendedResponseType::ExtendedResponseType(const ExtendedResponseType &x,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      extensionName_(x.extensionName_, f, this), any_(x.any_, this->getDomDocument()) {
}

ExtendedResponseType::ExtendedResponseType(const ::xercesc::DOMElement &e,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), extensionName_(this), any_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, false);
		this->parse(p, f);
	}
}

void ExtendedResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// extensionName
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "extensionName", "", &::xsd::cxx::tree::factory_impl<ExtensionNameType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!extensionName_.present()) {
					::std::unique_ptr<ExtensionNameType> r(dynamic_cast<ExtensionNameType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->extensionName_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!extensionName_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("extensionName", "");
	}
}

ExtendedResponseType *ExtendedResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ExtendedResponseType(*this, f, c);
}

ExtendedResponseType &ExtendedResponseType::operator=(const ExtendedResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->extensionName_ = x.extensionName_;
		this->any_ = x.any_;
	}

	return *this;
}

ExtendedResponseType::~ExtendedResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ExtendedResponseType>
    _xsd_ExtendedResponseType_type_factory_init("extendedResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// CcmpOptionsResponseMessageType
//

CcmpOptionsResponseMessageType::CcmpOptionsResponseMessageType(const ConfUserIDType &confUserID,
                                                               const ResponseCodeType &response_code,
                                                               const OptionsResponseType &optionsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      optionsResponse_(optionsResponse, this) {
}

CcmpOptionsResponseMessageType::CcmpOptionsResponseMessageType(const ConfUserIDType &confUserID,
                                                               const ResponseCodeType &response_code,
                                                               ::std::unique_ptr<OptionsResponseType> optionsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(confUserID, response_code),
      optionsResponse_(std::move(optionsResponse), this) {
}

CcmpOptionsResponseMessageType::CcmpOptionsResponseMessageType(::std::unique_ptr<ConfUserIDType> confUserID,
                                                               ::std::unique_ptr<ResponseCodeType> response_code,
                                                               ::std::unique_ptr<OptionsResponseType> optionsResponse)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(std::move(confUserID), std::move(response_code)),
      optionsResponse_(std::move(optionsResponse), this) {
}

CcmpOptionsResponseMessageType::CcmpOptionsResponseMessageType(const CcmpOptionsResponseMessageType &x,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(x, f, c),
      optionsResponse_(x.optionsResponse_, f, this) {
}

CcmpOptionsResponseMessageType::CcmpOptionsResponseMessageType(const ::xercesc::DOMElement &e,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType(
          e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      optionsResponse_(this) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void CcmpOptionsResponseMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p,
                                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	this->::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType::parse(p, f);

	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// optionsResponse
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
			    &::xsd::cxx::tree::factory_impl<OptionsResponseType>, true, true, i, n, f, this));

			if (tmp.get() != 0) {
				if (!optionsResponse_.present()) {
					::std::unique_ptr<OptionsResponseType> r(dynamic_cast<OptionsResponseType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->optionsResponse_.set(::std::move(r));
					continue;
				}
			}
		}

		break;
	}

	if (!optionsResponse_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp");
	}
}

CcmpOptionsResponseMessageType *
CcmpOptionsResponseMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class CcmpOptionsResponseMessageType(*this, f, c);
}

CcmpOptionsResponseMessageType &CcmpOptionsResponseMessageType::operator=(const CcmpOptionsResponseMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(*this) = x;
		this->optionsResponse_ = x.optionsResponse_;
	}

	return *this;
}

CcmpOptionsResponseMessageType::~CcmpOptionsResponseMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, CcmpOptionsResponseMessageType>
    _xsd_CcmpOptionsResponseMessageType_type_factory_init("ccmp-options-response-message-type",
                                                          "urn:ietf:params:xml:ns:xcon-ccmp");

// OptionsResponseType
//

OptionsResponseType::OptionsResponseType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      options_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

OptionsResponseType::OptionsResponseType(const OptionsResponseType &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      options_(x.options_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

OptionsResponseType::OptionsResponseType(const ::xercesc::DOMElement &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), options_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void OptionsResponseType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// options
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "options", "", &::xsd::cxx::tree::factory_impl<OptionsType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->options_) {
					::std::unique_ptr<OptionsType> r(dynamic_cast<OptionsType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->options_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

OptionsResponseType *OptionsResponseType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class OptionsResponseType(*this, f, c);
}

OptionsResponseType &OptionsResponseType::operator=(const OptionsResponseType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->options_ = x.options_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

OptionsResponseType::~OptionsResponseType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, OptionsResponseType>
    _xsd_OptionsResponseType_type_factory_init("optionsResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

// ResponseCodeType
//

ResponseCodeType::ResponseCodeType(const ::LinphonePrivate::Xsd::XmlSchema::PositiveInteger &_xsd_PositiveInteger_base)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(_xsd_PositiveInteger_base) {
}

ResponseCodeType::ResponseCodeType(const ResponseCodeType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(x, f, c) {
}

ResponseCodeType::ResponseCodeType(const ::xercesc::DOMElement &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(e, f, c) {
}

ResponseCodeType::ResponseCodeType(const ::xercesc::DOMAttr &a,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(a, f, c) {
}

ResponseCodeType::ResponseCodeType(const ::std::string &s,
                                   const ::xercesc::DOMElement *e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger,
                                         char,
                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType>(s, e, f, c) {
}

ResponseCodeType *ResponseCodeType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ResponseCodeType(*this, f, c);
}

ResponseCodeType::~ResponseCodeType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ResponseCodeType>
    _xsd_ResponseCodeType_type_factory_init("response-codeType", "urn:ietf:params:xml:ns:xcon-ccmp");

// OperationType
//

OperationType::OperationType(const ::xercesc::DOMElement &e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(e, f, c) {
	_xsd_OperationType_convert();
}

OperationType::OperationType(const ::xercesc::DOMAttr &a,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(a, f, c) {
	_xsd_OperationType_convert();
}

OperationType::OperationType(const ::std::string &s,
                             const ::xercesc::DOMElement *e,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                             ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(s, e, f, c) {
	_xsd_OperationType_convert();
}

OperationType *OperationType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class OperationType(*this, f, c);
}

OperationType::Value OperationType::_xsd_OperationType_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_OperationType_literals_);
	const Value *i(::std::lower_bound(_xsd_OperationType_indexes_, _xsd_OperationType_indexes_ + 4, *this, c));

	if (i == _xsd_OperationType_indexes_ + 4 || _xsd_OperationType_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const OperationType::_xsd_OperationType_literals_[4] = {"retrieve", "create", "update", "delete"};

const OperationType::Value OperationType::_xsd_OperationType_indexes_[4] = {
    ::LinphonePrivate::Xsd::XconCcmp::OperationType::create, ::LinphonePrivate::Xsd::XconCcmp::OperationType::delete_,
    ::LinphonePrivate::Xsd::XconCcmp::OperationType::retrieve, ::LinphonePrivate::Xsd::XconCcmp::OperationType::update};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, OperationType>
    _xsd_OperationType_type_factory_init("operationType", "urn:ietf:params:xml:ns:xcon-ccmp");

// SubjectType
//

SubjectType::SubjectType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      username_(this), password_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

SubjectType::SubjectType(const SubjectType &x,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      username_(x.username_, f, this), password_(x.password_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

SubjectType::SubjectType(const ::xercesc::DOMElement &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), username_(this), password_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void SubjectType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// username
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "username", "", &::xsd::cxx::tree::factory_impl<UsernameType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->username_) {
					::std::unique_ptr<UsernameType> r(dynamic_cast<UsernameType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->username_.set(::std::move(r));
					continue;
				}
			}
		}

		// password
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "password", "", &::xsd::cxx::tree::factory_impl<PasswordType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->password_) {
					::std::unique_ptr<PasswordType> r(dynamic_cast<PasswordType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->password_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

SubjectType *SubjectType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class SubjectType(*this, f, c);
}

SubjectType &SubjectType::operator=(const SubjectType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->username_ = x.username_;
		this->password_ = x.password_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

SubjectType::~SubjectType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, SubjectType>
    _xsd_SubjectType_type_factory_init("subject-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// OptionsType
//

OptionsType::OptionsType(const StandardMessageListType &standard_message_list)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      standard_message_list_(standard_message_list, this), extended_message_list_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
}

OptionsType::OptionsType(::std::unique_ptr<StandardMessageListType> standard_message_list)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      standard_message_list_(std::move(standard_message_list), this), extended_message_list_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

OptionsType::OptionsType(const OptionsType &x,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      standard_message_list_(x.standard_message_list_, f, this),
      extended_message_list_(x.extended_message_list_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

OptionsType::OptionsType(const ::xercesc::DOMElement &e,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), standard_message_list_(this),
      extended_message_list_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void OptionsType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// standard-message-list
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "standard-message-list", "", &::xsd::cxx::tree::factory_impl<StandardMessageListType>, false, false, i,
			    n, f, this));

			if (tmp.get() != 0) {
				if (!standard_message_list_.present()) {
					::std::unique_ptr<StandardMessageListType> r(dynamic_cast<StandardMessageListType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->standard_message_list_.set(::std::move(r));
					continue;
				}
			}
		}

		// extended-message-list
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "extended-message-list", "", &::xsd::cxx::tree::factory_impl<ExtendedMessageListType>, false, false, i,
			    n, f, this));

			if (tmp.get() != 0) {
				if (!this->extended_message_list_) {
					::std::unique_ptr<ExtendedMessageListType> r(dynamic_cast<ExtendedMessageListType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->extended_message_list_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!standard_message_list_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("standard-message-list", "");
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

OptionsType *OptionsType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class OptionsType(*this, f, c);
}

OptionsType &OptionsType::operator=(const OptionsType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->standard_message_list_ = x.standard_message_list_;
		this->extended_message_list_ = x.extended_message_list_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

OptionsType::~OptionsType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, OptionsType>
    _xsd_OptionsType_type_factory_init("options-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// StandardMessageListType
//

StandardMessageListType::StandardMessageListType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      standard_message_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

StandardMessageListType::StandardMessageListType(const StandardMessageListType &x,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      standard_message_(x.standard_message_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

StandardMessageListType::StandardMessageListType(const ::xercesc::DOMElement &e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), standard_message_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void StandardMessageListType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// standard-message
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "standard-message", "", &::xsd::cxx::tree::factory_impl<StandardMessageType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				::std::unique_ptr<StandardMessageType> r(dynamic_cast<StandardMessageType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->standard_message_.push_back(::std::move(r));
				continue;
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

StandardMessageListType *StandardMessageListType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class StandardMessageListType(*this, f, c);
}

StandardMessageListType &StandardMessageListType::operator=(const StandardMessageListType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->standard_message_ = x.standard_message_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

StandardMessageListType::~StandardMessageListType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, StandardMessageListType>
    _xsd_StandardMessageListType_type_factory_init("standard-message-list-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// StandardMessageType
//

StandardMessageType::StandardMessageType(const NameType &name)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      name_(name, this), operations_(this), schema_def_(this), description_(this), any_(this->getDomDocument()),
      any_attribute_(this->getDomDocument()) {
}

StandardMessageType::StandardMessageType(::std::unique_ptr<NameType> name)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      name_(std::move(name), this), operations_(this), schema_def_(this), description_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

StandardMessageType::StandardMessageType(const StandardMessageType &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      name_(x.name_, f, this), operations_(x.operations_, f, this), schema_def_(x.schema_def_, f, this),
      description_(x.description_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

StandardMessageType::StandardMessageType(const ::xercesc::DOMElement &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), name_(this), operations_(this), schema_def_(this),
      description_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void StandardMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// name
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "name", "", &::xsd::cxx::tree::factory_impl<NameType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!name_.present()) {
					::std::unique_ptr<NameType> r(dynamic_cast<NameType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->name_.set(::std::move(r));
					continue;
				}
			}
		}

		// operations
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "operations", "", &::xsd::cxx::tree::factory_impl<OperationsType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->operations_) {
					::std::unique_ptr<OperationsType> r(dynamic_cast<OperationsType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->operations_.set(::std::move(r));
					continue;
				}
			}
		}

		// schema-def
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "schema-def", "", &::xsd::cxx::tree::factory_impl<SchemaDefType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->schema_def_) {
					::std::unique_ptr<SchemaDefType> r(dynamic_cast<SchemaDefType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->schema_def_.set(::std::move(r));
					continue;
				}
			}
		}

		// description
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "description", "", &::xsd::cxx::tree::factory_impl<DescriptionType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->description_) {
					::std::unique_ptr<DescriptionType> r(dynamic_cast<DescriptionType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->description_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!name_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("name", "");
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

StandardMessageType *StandardMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class StandardMessageType(*this, f, c);
}

StandardMessageType &StandardMessageType::operator=(const StandardMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->name_ = x.name_;
		this->operations_ = x.operations_;
		this->schema_def_ = x.schema_def_;
		this->description_ = x.description_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

StandardMessageType::~StandardMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, StandardMessageType>
    _xsd_StandardMessageType_type_factory_init("standard-message-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// StandardMessageNameType
//

StandardMessageNameType::StandardMessageNameType(const ::xercesc::DOMElement &e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(e, f, c) {
	_xsd_StandardMessageNameType_convert();
}

StandardMessageNameType::StandardMessageNameType(const ::xercesc::DOMAttr &a,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(a, f, c) {
	_xsd_StandardMessageNameType_convert();
}

StandardMessageNameType::StandardMessageNameType(const ::std::string &s,
                                                 const ::xercesc::DOMElement *e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Token(s, e, f, c) {
	_xsd_StandardMessageNameType_convert();
}

StandardMessageNameType *StandardMessageNameType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class StandardMessageNameType(*this, f, c);
}

StandardMessageNameType::Value StandardMessageNameType::_xsd_StandardMessageNameType_convert() const {
	::xsd::cxx::tree::enum_comparator<char> c(_xsd_StandardMessageNameType_literals_);
	const Value *i(::std::lower_bound(_xsd_StandardMessageNameType_indexes_, _xsd_StandardMessageNameType_indexes_ + 10,
	                                  *this, c));

	if (i == _xsd_StandardMessageNameType_indexes_ + 10 || _xsd_StandardMessageNameType_literals_[*i] != *this) {
		throw ::xsd::cxx::tree::unexpected_enumerator<char>(*this);
	}

	return *i;
}

const char *const StandardMessageNameType::_xsd_StandardMessageNameType_literals_[10] = {
    "confsRequest", "confRequest",          "blueprintsRequest",   "blueprintRequest",     "usersRequest",
    "userRequest",  "sidebarsByValRequest", "sidebarByValRequest", "sidebarsByRefRequest", "sidebarByRefRequest"};

const StandardMessageNameType::Value StandardMessageNameType::_xsd_StandardMessageNameType_indexes_[10] = {
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::blueprintRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::blueprintsRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::confRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::confsRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::sidebarByRefRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::sidebarByValRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::sidebarsByRefRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::sidebarsByValRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::userRequest,
    ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType::usersRequest};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, StandardMessageNameType>
    _xsd_StandardMessageNameType_type_factory_init("standard-message-name-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// OperationsType
//

OperationsType::OperationsType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      operation_(this), any_attribute_(this->getDomDocument()) {
}

OperationsType::OperationsType(const OperationsType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      operation_(x.operation_, f, this), any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

OperationsType::OperationsType(const ::xercesc::DOMElement &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                               ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), operation_(this),
      any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void OperationsType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// operation
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "operation", "", &::xsd::cxx::tree::factory_impl<OperationType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				::std::unique_ptr<OperationType> r(dynamic_cast<OperationType *>(tmp.get()));

				if (r.get()) tmp.release();
				else throw ::xsd::cxx::tree::not_derived<char>();

				this->operation_.push_back(::std::move(r));
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
		if ((n.namespace_() != ::xsd::cxx::xml::bits::xmlns_namespace<char>() &&
		     n.namespace_() != ::xsd::cxx::xml::bits::xsi_namespace<char>())) {
			::xercesc::DOMAttr *r(static_cast<::xercesc::DOMAttr *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMAttr *>(&i), true)));
			this->any_attribute_.insert(r);
			continue;
		}
	}
}

OperationsType *OperationsType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class OperationsType(*this, f, c);
}

OperationsType &OperationsType::operator=(const OperationsType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->operation_ = x.operation_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

OperationsType::~OperationsType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, OperationsType>
    _xsd_OperationsType_type_factory_init("operations-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// ExtendedMessageListType
//

ExtendedMessageListType::ExtendedMessageListType()
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      extended_message_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

ExtendedMessageListType::ExtendedMessageListType(const ExtendedMessageListType &x,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      extended_message_(x.extended_message_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ExtendedMessageListType::ExtendedMessageListType(const ::xercesc::DOMElement &e,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), extended_message_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ExtendedMessageListType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// extended-message
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "extended-message", "", &::xsd::cxx::tree::factory_impl<ExtendedMessageType>, false, false, i, n, f,
			    this));

			if (tmp.get() != 0) {
				if (!this->extended_message_) {
					::std::unique_ptr<ExtendedMessageType> r(dynamic_cast<ExtendedMessageType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->extended_message_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
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

ExtendedMessageListType *ExtendedMessageListType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ExtendedMessageListType(*this, f, c);
}

ExtendedMessageListType &ExtendedMessageListType::operator=(const ExtendedMessageListType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->extended_message_ = x.extended_message_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ExtendedMessageListType::~ExtendedMessageListType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ExtendedMessageListType>
    _xsd_ExtendedMessageListType_type_factory_init("extended-message-list-type", "urn:ietf:params:xml:ns:xcon-ccmp");

// ExtendedMessageType
//

ExtendedMessageType::ExtendedMessageType(const NameType &name, const SchemaDefType &schema_def)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      name_(name, this), operations_(this), schema_def_(schema_def, this), description_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

ExtendedMessageType::ExtendedMessageType(::std::unique_ptr<NameType> name, ::std::unique_ptr<SchemaDefType> schema_def)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      name_(std::move(name), this), operations_(this), schema_def_(std::move(schema_def), this), description_(this),
      any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
}

ExtendedMessageType::ExtendedMessageType(const ExtendedMessageType &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(x, f, c), dom_document_(::xsd::cxx::xml::dom::create_document<char>()),
      name_(x.name_, f, this), operations_(x.operations_, f, this), schema_def_(x.schema_def_, f, this),
      description_(x.description_, f, this), any_(x.any_, this->getDomDocument()),
      any_attribute_(x.any_attribute_, this->getDomDocument()) {
}

ExtendedMessageType::ExtendedMessageType(const ::xercesc::DOMElement &e,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c)
    : ::LinphonePrivate::Xsd::XmlSchema::Type(e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
      dom_document_(::xsd::cxx::xml::dom::create_document<char>()), name_(this), operations_(this), schema_def_(this),
      description_(this), any_(this->getDomDocument()), any_attribute_(this->getDomDocument()) {
	if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0) {
		::xsd::cxx::xml::dom::parser<char> p(e, true, false, true);
		this->parse(p, f);
	}
}

void ExtendedMessageType::parse(::xsd::cxx::xml::dom::parser<char> &p, ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	for (; p.more_content(); p.next_content(false)) {
		const ::xercesc::DOMElement &i(p.cur_element());
		const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(i));

		// name
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "name", "", &::xsd::cxx::tree::factory_impl<NameType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!name_.present()) {
					::std::unique_ptr<NameType> r(dynamic_cast<NameType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->name_.set(::std::move(r));
					continue;
				}
			}
		}

		// operations
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "operations", "", &::xsd::cxx::tree::factory_impl<OperationsType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->operations_) {
					::std::unique_ptr<OperationsType> r(dynamic_cast<OperationsType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->operations_.set(::std::move(r));
					continue;
				}
			}
		}

		// schema-def
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "schema-def", "", &::xsd::cxx::tree::factory_impl<SchemaDefType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!schema_def_.present()) {
					::std::unique_ptr<SchemaDefType> r(dynamic_cast<SchemaDefType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->schema_def_.set(::std::move(r));
					continue;
				}
			}
		}

		// description
		//
		{
			::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
			    "description", "", &::xsd::cxx::tree::factory_impl<DescriptionType>, false, false, i, n, f, this));

			if (tmp.get() != 0) {
				if (!this->description_) {
					::std::unique_ptr<DescriptionType> r(dynamic_cast<DescriptionType *>(tmp.get()));

					if (r.get()) tmp.release();
					else throw ::xsd::cxx::tree::not_derived<char>();

					this->description_.set(::std::move(r));
					continue;
				}
			}
		}

		// any
		//
		if ((!n.namespace_().empty() && n.namespace_() != "urn:ietf:params:xml:ns:xcon-ccmp")) {
			::xercesc::DOMElement *r(static_cast<::xercesc::DOMElement *>(
			    this->getDomDocument().importNode(const_cast<::xercesc::DOMElement *>(&i), true)));
			this->any_.push_back(r);
			continue;
		}

		break;
	}

	if (!name_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("name", "");
	}

	if (!schema_def_.present()) {
		throw ::xsd::cxx::tree::expected_element<char>("schema-def", "");
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

ExtendedMessageType *ExtendedMessageType::_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c) const {
	return new class ExtendedMessageType(*this, f, c);
}

ExtendedMessageType &ExtendedMessageType::operator=(const ExtendedMessageType &x) {
	if (this != &x) {
		static_cast<::LinphonePrivate::Xsd::XmlSchema::Type &>(*this) = x;
		this->name_ = x.name_;
		this->operations_ = x.operations_;
		this->schema_def_ = x.schema_def_;
		this->description_ = x.description_;
		this->any_ = x.any_;
		this->any_attribute_ = x.any_attribute_;
	}

	return *this;
}

ExtendedMessageType::~ExtendedMessageType() {
}

static const ::xsd::cxx::tree::type_factory_initializer<0, char, ExtendedMessageType>
    _xsd_ExtendedMessageType_type_factory_init("extended-message-type", "urn:ietf:params:xml:ns:xcon-ccmp");
} // namespace XconCcmp
} // namespace Xsd
} // namespace LinphonePrivate

#include <ostream>

#include <xsd/cxx/tree/std-ostream-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::std_ostream_plate<0, char> std_ostream_plate_init;
}

namespace LinphonePrivate {
namespace Xsd {
namespace XconCcmp {
::std::ostream &operator<<(::std::ostream &o, const CcmpRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "ccmpRequest: ";
		om.insert(o, i.getCcmpRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpRequestType> _xsd_CcmpRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpRequestMessageType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSubject()) {
			o << ::std::endl << "subject: ";
			om.insert(o, *i.getSubject());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConfUserID()) {
			o << ::std::endl << "confUserID: ";
			om.insert(o, *i.getConfUserID());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConfObjID()) {
			o << ::std::endl << "confObjID: ";
			om.insert(o, *i.getConfObjID());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getOperation()) {
			o << ::std::endl << "operation: ";
			om.insert(o, *i.getOperation());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConferencePassword()) {
			o << ::std::endl << "conference-password: ";
			om.insert(o, *i.getConferencePassword());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpRequestMessageType>
    _xsd_CcmpRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "ccmpResponse: ";
		om.insert(o, i.getCcmpResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpResponseType>
    _xsd_CcmpResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpResponseMessageType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "confUserID: ";
		om.insert(o, i.getConfUserID());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConfObjID()) {
			o << ::std::endl << "confObjID: ";
			om.insert(o, *i.getConfObjID());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getOperation()) {
			o << ::std::endl << "operation: ";
			om.insert(o, *i.getOperation());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "response-code: ";
		om.insert(o, i.getResponseCode());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getResponseString()) {
			o << ::std::endl << "response-string: ";
			om.insert(o, *i.getResponseString());
		}
	}

	if (i.getVersion()) {
		o << ::std::endl << "version: " << *i.getVersion();
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpResponseMessageType>
    _xsd_CcmpResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpBlueprintsRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "blueprintsRequest: ";
		om.insert(o, i.getBlueprintsRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpBlueprintsRequestMessageType>
    _xsd_CcmpBlueprintsRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const BlueprintsRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getXpathFilter()) {
			o << ::std::endl << "xpathFilter: ";
			om.insert(o, *i.getXpathFilter());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, BlueprintsRequestType>
    _xsd_BlueprintsRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpBlueprintRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "blueprintRequest: ";
		om.insert(o, i.getBlueprintRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpBlueprintRequestMessageType>
    _xsd_CcmpBlueprintRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const BlueprintRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getBlueprintInfo()) {
			o << ::std::endl << "blueprintInfo: ";
			om.insert(o, *i.getBlueprintInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, BlueprintRequestType>
    _xsd_BlueprintRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpConfsRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "confsRequest: ";
		om.insert(o, i.getConfsRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpConfsRequestMessageType>
    _xsd_CcmpConfsRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConfsRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getXpathFilter()) {
			o << ::std::endl << "xpathFilter: ";
			om.insert(o, *i.getXpathFilter());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConfsRequestType>
    _xsd_ConfsRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpConfRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "confRequest: ";
		om.insert(o, i.getConfRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpConfRequestMessageType>
    _xsd_CcmpConfRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConfRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConfInfo()) {
			o << ::std::endl << "confInfo: ";
			om.insert(o, *i.getConfInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConfRequestType> _xsd_ConfRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpUsersRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "usersRequest: ";
		om.insert(o, i.getUsersRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpUsersRequestMessageType>
    _xsd_CcmpUsersRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UsersRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getUsersInfo()) {
			o << ::std::endl << "usersInfo: ";
			om.insert(o, *i.getUsersInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UsersRequestType>
    _xsd_UsersRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpUserRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "userRequest: ";
		om.insert(o, i.getUserRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpUserRequestMessageType>
    _xsd_CcmpUserRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UserRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getUserInfo()) {
			o << ::std::endl << "userInfo: ";
			om.insert(o, *i.getUserInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UserRequestType> _xsd_UserRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpSidebarsByValRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "sidebarsByValRequest: ";
		om.insert(o, i.getSidebarsByValRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpSidebarsByValRequestMessageType>
    _xsd_CcmpSidebarsByValRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarsByValRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getXpathFilter()) {
			o << ::std::endl << "xpathFilter: ";
			om.insert(o, *i.getXpathFilter());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarsByValRequestType>
    _xsd_SidebarsByValRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpSidebarsByRefRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "sidebarsByRefRequest: ";
		om.insert(o, i.getSidebarsByRefRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpSidebarsByRefRequestMessageType>
    _xsd_CcmpSidebarsByRefRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarsByRefRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getXpathFilter()) {
			o << ::std::endl << "xpathFilter: ";
			om.insert(o, *i.getXpathFilter());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarsByRefRequestType>
    _xsd_SidebarsByRefRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpSidebarByValRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "sidebarByValRequest: ";
		om.insert(o, i.getSidebarByValRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpSidebarByValRequestMessageType>
    _xsd_CcmpSidebarByValRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarByValRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSidebarByValInfo()) {
			o << ::std::endl << "sidebarByValInfo: ";
			om.insert(o, *i.getSidebarByValInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarByValRequestType>
    _xsd_SidebarByValRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpSidebarByRefRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "sidebarByRefRequest: ";
		om.insert(o, i.getSidebarByRefRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpSidebarByRefRequestMessageType>
    _xsd_CcmpSidebarByRefRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarByRefRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSidebarByRefInfo()) {
			o << ::std::endl << "sidebarByRefInfo: ";
			om.insert(o, *i.getSidebarByRefInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarByRefRequestType>
    _xsd_SidebarByRefRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpExtendedRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "extendedRequest: ";
		om.insert(o, i.getExtendedRequest());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpExtendedRequestMessageType>
    _xsd_CcmpExtendedRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ExtendedRequestType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "extensionName: ";
		om.insert(o, i.getExtensionName());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ExtendedRequestType>
    _xsd_ExtendedRequestType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpOptionsRequestMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpOptionsRequestMessageType>
    _xsd_CcmpOptionsRequestMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpBlueprintsResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "blueprintsResponse: ";
		om.insert(o, i.getBlueprintsResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpBlueprintsResponseMessageType>
    _xsd_CcmpBlueprintsResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const BlueprintsResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getBlueprintsInfo()) {
			o << ::std::endl << "blueprintsInfo: ";
			om.insert(o, *i.getBlueprintsInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, BlueprintsResponseType>
    _xsd_BlueprintsResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpBlueprintResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "blueprintResponse: ";
		om.insert(o, i.getBlueprintResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpBlueprintResponseMessageType>
    _xsd_CcmpBlueprintResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const BlueprintResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getBlueprintInfo()) {
			o << ::std::endl << "blueprintInfo: ";
			om.insert(o, *i.getBlueprintInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, BlueprintResponseType>
    _xsd_BlueprintResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpConfsResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "confsResponse: ";
		om.insert(o, i.getConfsResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpConfsResponseMessageType>
    _xsd_CcmpConfsResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConfsResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConfsInfo()) {
			o << ::std::endl << "confsInfo: ";
			om.insert(o, *i.getConfsInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConfsResponseType>
    _xsd_ConfsResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpConfResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "confResponse: ";
		om.insert(o, i.getConfResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpConfResponseMessageType>
    _xsd_CcmpConfResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ConfResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getConfInfo()) {
			o << ::std::endl << "confInfo: ";
			om.insert(o, *i.getConfInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ConfResponseType>
    _xsd_ConfResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpUsersResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "usersResponse: ";
		om.insert(o, i.getUsersResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpUsersResponseMessageType>
    _xsd_CcmpUsersResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UsersResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getUsersInfo()) {
			o << ::std::endl << "usersInfo: ";
			om.insert(o, *i.getUsersInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UsersResponseType>
    _xsd_UsersResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpUserResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "userResponse: ";
		om.insert(o, i.getUserResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpUserResponseMessageType>
    _xsd_CcmpUserResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const UserResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getUserInfo()) {
			o << ::std::endl << "userInfo: ";
			om.insert(o, *i.getUserInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, UserResponseType>
    _xsd_UserResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpSidebarsByValResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "sidebarsByValResponse: ";
		om.insert(o, i.getSidebarsByValResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpSidebarsByValResponseMessageType>
    _xsd_CcmpSidebarsByValResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarsByValResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSidebarsByValInfo()) {
			o << ::std::endl << "sidebarsByValInfo: ";
			om.insert(o, *i.getSidebarsByValInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarsByValResponseType>
    _xsd_SidebarsByValResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpSidebarsByRefResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "sidebarsByRefResponse: ";
		om.insert(o, i.getSidebarsByRefResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpSidebarsByRefResponseMessageType>
    _xsd_CcmpSidebarsByRefResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarsByRefResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSidebarsByRefInfo()) {
			o << ::std::endl << "sidebarsByRefInfo: ";
			om.insert(o, *i.getSidebarsByRefInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarsByRefResponseType>
    _xsd_SidebarsByRefResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpSidebarByValResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "sidebarByValResponse: ";
		om.insert(o, i.getSidebarByValResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpSidebarByValResponseMessageType>
    _xsd_CcmpSidebarByValResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarByValResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSidebarByValInfo()) {
			o << ::std::endl << "sidebarByValInfo: ";
			om.insert(o, *i.getSidebarByValInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarByValResponseType>
    _xsd_SidebarByValResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpSidebarByRefResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "sidebarByRefResponse: ";
		om.insert(o, i.getSidebarByRefResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpSidebarByRefResponseMessageType>
    _xsd_CcmpSidebarByRefResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SidebarByRefResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSidebarByRefInfo()) {
			o << ::std::endl << "sidebarByRefInfo: ";
			om.insert(o, *i.getSidebarByRefInfo());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SidebarByRefResponseType>
    _xsd_SidebarByRefResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpExtendedResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "extendedResponse: ";
		om.insert(o, i.getExtendedResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpExtendedResponseMessageType>
    _xsd_CcmpExtendedResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ExtendedResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "extensionName: ";
		om.insert(o, i.getExtensionName());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ExtendedResponseType>
    _xsd_ExtendedResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const CcmpOptionsResponseMessageType &i) {
	o << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "optionsResponse: ";
		om.insert(o, i.getOptionsResponse());
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, CcmpOptionsResponseMessageType>
    _xsd_CcmpOptionsResponseMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const OptionsResponseType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getOptions()) {
			o << ::std::endl << "options: ";
			om.insert(o, *i.getOptions());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, OptionsResponseType>
    _xsd_OptionsResponseType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ResponseCodeType &i) {
	o << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ResponseCodeType>
    _xsd_ResponseCodeType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, OperationType::Value i) {
	return o << OperationType::_xsd_OperationType_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const OperationType &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Token &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, OperationType> _xsd_OperationType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const SubjectType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getUsername()) {
			o << ::std::endl << "username: ";
			om.insert(o, *i.getUsername());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getPassword()) {
			o << ::std::endl << "password: ";
			om.insert(o, *i.getPassword());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, SubjectType> _xsd_SubjectType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const OptionsType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "standard-message-list: ";
		om.insert(o, i.getStandardMessageList());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getExtendedMessageList()) {
			o << ::std::endl << "extended-message-list: ";
			om.insert(o, *i.getExtendedMessageList());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, OptionsType> _xsd_OptionsType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const StandardMessageListType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (StandardMessageListType::StandardMessageConstIterator b(i.getStandardMessage().begin()),
		     e(i.getStandardMessage().end());
		     b != e; ++b) {
			o << ::std::endl << "standard-message: ";
			om.insert(o, *b);
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, StandardMessageListType>
    _xsd_StandardMessageListType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const StandardMessageType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "name: ";
		om.insert(o, i.getName());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getOperations()) {
			o << ::std::endl << "operations: ";
			om.insert(o, *i.getOperations());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getSchemaDef()) {
			o << ::std::endl << "schema-def: ";
			om.insert(o, *i.getSchemaDef());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDescription()) {
			o << ::std::endl << "description: ";
			om.insert(o, *i.getDescription());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, StandardMessageType>
    _xsd_StandardMessageType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, StandardMessageNameType::Value i) {
	return o << StandardMessageNameType::_xsd_StandardMessageNameType_literals_[i];
}

::std::ostream &operator<<(::std::ostream &o, const StandardMessageNameType &i) {
	return o << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Token &>(i);
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, StandardMessageNameType>
    _xsd_StandardMessageNameType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const OperationsType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		for (OperationsType::OperationConstIterator b(i.getOperation().begin()), e(i.getOperation().end()); b != e;
		     ++b) {
			o << ::std::endl << "operation: ";
			om.insert(o, *b);
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, OperationsType> _xsd_OperationsType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ExtendedMessageListType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getExtendedMessage()) {
			o << ::std::endl << "extended-message: ";
			om.insert(o, *i.getExtendedMessage());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ExtendedMessageListType>
    _xsd_ExtendedMessageListType_std_ostream_init;

::std::ostream &operator<<(::std::ostream &o, const ExtendedMessageType &i) {
	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "name: ";
		om.insert(o, i.getName());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getOperations()) {
			o << ::std::endl << "operations: ";
			om.insert(o, *i.getOperations());
		}
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		o << ::std::endl << "schema-def: ";
		om.insert(o, i.getSchemaDef());
	}

	{
		::xsd::cxx::tree::std_ostream_map<char> &om(::xsd::cxx::tree::std_ostream_map_instance<0, char>());

		if (i.getDescription()) {
			o << ::std::endl << "description: ";
			om.insert(o, *i.getDescription());
		}
	}

	return o;
}

static const ::xsd::cxx::tree::std_ostream_initializer<0, char, ExtendedMessageType>
    _xsd_ExtendedMessageType_std_ostream_init;
} // namespace XconCcmp
} // namespace Xsd
} // namespace LinphonePrivate

#include <istream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/sax/std-input-source.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace XconCcmp {
::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(const ::std::string &u,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(const ::std::string &u,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(const ::std::string &u,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::std::istream &is,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::std::istream &is,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::std::istream &is,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::xercesc::InputSource &i,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::xercesc::InputSource &i,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::xercesc::InputSource &i,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(const ::xercesc::DOMDocument &doc,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "ccmpRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ccmpRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>
parseCcmpRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "ccmpRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ccmpRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(const ::std::string &u,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::std::istream &is,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::xercesc::InputSource &i,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(const ::xercesc::DOMDocument &doc,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseCcmpResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "ccmpResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ccmpResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>
parseCcmpResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "ccmpResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ccmpResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(const ::std::string &u,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(const ::std::string &u,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(const ::std::string &u,
                       ::xercesc::DOMErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::std::istream &is,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::std::istream &is,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::std::istream &is,
                       ::xercesc::DOMErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::std::istream &is,
                       const ::std::string &sid,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::std::istream &is,
                       const ::std::string &sid,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::std::istream &is,
                       const ::std::string &sid,
                       ::xercesc::DOMErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::xercesc::InputSource &i,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::xercesc::InputSource &i,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::xercesc::InputSource &i,
                       ::xercesc::DOMErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(const ::xercesc::DOMDocument &doc,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintsRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>
parseBlueprintsRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintsRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(const ::std::string &u,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::std::istream &is,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::std::istream &is,
                      const ::std::string &sid,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::xercesc::InputSource &i,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(const ::xercesc::DOMDocument &doc,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>
parseBlueprintRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(const ::std::string &u,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::std::istream &is,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::std::istream &is,
                  const ::std::string &sid,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::xercesc::InputSource &i,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(const ::xercesc::DOMDocument &doc,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseConfsRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confsRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>
parseConfsRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confsRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(const ::std::string &u,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(const ::std::string &u,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(const ::std::string &u,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::std::istream &is,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::std::istream &is,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::std::istream &is,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::xercesc::InputSource &i,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::xercesc::InputSource &i,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::xercesc::InputSource &i,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(const ::xercesc::DOMDocument &doc,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseConfRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "confRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>
parseConfRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "confRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(const ::std::string &u,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::std::istream &is,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::std::istream &is,
                  const ::std::string &sid,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::xercesc::InputSource &i,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(const ::xercesc::DOMDocument &doc,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseUsersRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "usersRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>
parseUsersRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "usersRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(const ::std::string &u,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(const ::std::string &u,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(const ::std::string &u,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::std::istream &is,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::std::istream &is,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::std::istream &is,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::std::istream &is,
                 const ::std::string &sid,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::xercesc::InputSource &i,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::xercesc::InputSource &i,
                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::xercesc::InputSource &i,
                 ::xercesc::DOMErrorHandler &h,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(std::move(d),
	                                                       f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(const ::xercesc::DOMDocument &doc,
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                 const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseUserRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "userRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::UserRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "userRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>
parseUserRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "userRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::UserRequestType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::UserRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "userRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(const ::std::string &u,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::std::istream &is,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::std::istream &is,
                          const ::std::string &sid,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::xercesc::InputSource &i,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(const ::xercesc::DOMDocument &doc,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByValRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>
parseSidebarsByValRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByValRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(const ::std::string &u,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::std::istream &is,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::std::istream &is,
                          const ::std::string &sid,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::xercesc::InputSource &i,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(const ::xercesc::DOMDocument &doc,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByRefRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>
parseSidebarsByRefRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByRefRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(const ::std::string &u,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(const ::std::string &u,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(const ::std::string &u,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::std::istream &is,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::std::istream &is,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::std::istream &is,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::std::istream &is,
                         const ::std::string &sid,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::std::istream &is,
                         const ::std::string &sid,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::std::istream &is,
                         const ::std::string &sid,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::xercesc::InputSource &i,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::xercesc::InputSource &i,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::xercesc::InputSource &i,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(const ::xercesc::DOMDocument &doc,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByValRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>
parseSidebarByValRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByValRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(const ::std::string &u,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(const ::std::string &u,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(const ::std::string &u,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::std::istream &is,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::std::istream &is,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::std::istream &is,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::std::istream &is,
                         const ::std::string &sid,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::std::istream &is,
                         const ::std::string &sid,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::std::istream &is,
                         const ::std::string &sid,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::xercesc::InputSource &i,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::xercesc::InputSource &i,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::xercesc::InputSource &i,
                         ::xercesc::DOMErrorHandler &h,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(const ::xercesc::DOMDocument &doc,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                         const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByRefRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>
parseSidebarByRefRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByRefRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(const ::std::string &u,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(const ::std::string &u,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(const ::std::string &u,
                     ::xercesc::DOMErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::std::istream &is,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::std::istream &is,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::std::istream &is,
                     ::xercesc::DOMErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::std::istream &is,
                     const ::std::string &sid,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::std::istream &is,
                     const ::std::string &sid,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::std::istream &is,
                     const ::std::string &sid,
                     ::xercesc::DOMErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::xercesc::InputSource &i,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::xercesc::InputSource &i,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::xercesc::InputSource &i,
                     ::xercesc::DOMErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(const ::xercesc::DOMDocument &doc,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedRequest(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "extendedRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>
parseExtendedRequest(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "extendedRequest",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(const ::std::string &u,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(const ::std::string &u,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(const ::std::string &u,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::std::istream &is,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::std::istream &is,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::std::istream &is,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::std::istream &is,
                        const ::std::string &sid,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::std::istream &is,
                        const ::std::string &sid,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::std::istream &is,
                        const ::std::string &sid,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::xercesc::InputSource &i,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::xercesc::InputSource &i,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::xercesc::InputSource &i,
                        ::xercesc::DOMErrorHandler &h,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(const ::xercesc::DOMDocument &doc,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                        const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintsResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintsResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>
parseBlueprintsResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintsResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(const ::std::string &u,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(const ::std::string &u,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(const ::std::string &u,
                       ::xercesc::DOMErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::std::istream &is,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::std::istream &is,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::std::istream &is,
                       ::xercesc::DOMErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::std::istream &is,
                       const ::std::string &sid,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::std::istream &is,
                       const ::std::string &sid,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::std::istream &is,
                       const ::std::string &sid,
                       ::xercesc::DOMErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::xercesc::InputSource &i,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::xercesc::InputSource &i,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::xercesc::InputSource &i,
                       ::xercesc::DOMErrorHandler &h,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(const ::xercesc::DOMDocument &doc,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                       const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseBlueprintResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>
parseBlueprintResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(const ::std::string &u,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::std::istream &is,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::std::istream &is,
                   const ::std::string &sid,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::xercesc::InputSource &i,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(const ::xercesc::DOMDocument &doc,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseConfsResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confsResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>
parseConfsResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confsResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(const ::std::string &u,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::std::istream &is,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::xercesc::InputSource &i,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(const ::xercesc::DOMDocument &doc,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseConfResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "confResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>
parseConfResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "confResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(const ::std::string &u,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(const ::std::string &u,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::std::istream &is,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::std::istream &is,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::std::istream &is,
                   const ::std::string &sid,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::std::istream &is,
                   const ::std::string &sid,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::xercesc::InputSource &i,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::xercesc::InputSource &i,
                   ::xercesc::DOMErrorHandler &h,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(std::move(d),
	                                                         f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(const ::xercesc::DOMDocument &doc,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                   const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseUsersResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "usersResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>
parseUsersResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "usersResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(const ::std::string &u,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(const ::std::string &u,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::std::istream &is,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::std::istream &is,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::std::istream &is,
                  const ::std::string &sid,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::xercesc::InputSource &i,
                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::xercesc::InputSource &i,
                  ::xercesc::DOMErrorHandler &h,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(std::move(d),
	                                                        f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(const ::xercesc::DOMDocument &doc,
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                  const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseUserResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "userResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::UserResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "userResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>
parseUserResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "userResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::UserResponseType>, true, true, e, n, f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::UserResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "userResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(const ::std::string &u,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(const ::std::string &u,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(const ::std::string &u,
                           ::xercesc::DOMErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::std::istream &is,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::std::istream &is,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::std::istream &is,
                           ::xercesc::DOMErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::std::istream &is,
                           const ::std::string &sid,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::std::istream &is,
                           const ::std::string &sid,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::std::istream &is,
                           const ::std::string &sid,
                           ::xercesc::DOMErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::xercesc::InputSource &i,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::xercesc::InputSource &i,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::xercesc::InputSource &i,
                           ::xercesc::DOMErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(const ::xercesc::DOMDocument &doc,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByValResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByValResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>
parseSidebarsByValResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByValResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(const ::std::string &u,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(const ::std::string &u,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(const ::std::string &u,
                           ::xercesc::DOMErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::std::istream &is,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::std::istream &is,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::std::istream &is,
                           ::xercesc::DOMErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::std::istream &is,
                           const ::std::string &sid,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::std::istream &is,
                           const ::std::string &sid,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::std::istream &is,
                           const ::std::string &sid,
                           ::xercesc::DOMErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::xercesc::InputSource &i,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::xercesc::InputSource &i,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::xercesc::InputSource &i,
                           ::xercesc::DOMErrorHandler &h,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(const ::xercesc::DOMDocument &doc,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarsByRefResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByRefResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>
parseSidebarsByRefResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByRefResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(const ::std::string &u,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::std::istream &is,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::std::istream &is,
                          const ::std::string &sid,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::xercesc::InputSource &i,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(const ::xercesc::DOMDocument &doc,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByValResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByValResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>
parseSidebarByValResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByValResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(const ::std::string &u,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(const ::std::string &u,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::std::istream &is,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::std::istream &is,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::std::istream &is,
                          const ::std::string &sid,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::std::istream &is,
                          const ::std::string &sid,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::xercesc::InputSource &i,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::xercesc::InputSource &i,
                          ::xercesc::DOMErrorHandler &h,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(const ::xercesc::DOMDocument &doc,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseSidebarByRefResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByRefResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>
parseSidebarByRefResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType>, true, true, e, n,
	    f, 0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByRefResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(const ::std::string &u,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(const ::std::string &u,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::std::istream &is,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::std::istream &is,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::std::istream &is,
                      const ::std::string &sid,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::std::istream &is,
                      const ::std::string &sid,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::xercesc::InputSource &i,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::xercesc::InputSource &i,
                      ::xercesc::DOMErrorHandler &h,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(const ::xercesc::DOMDocument &doc,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                      const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseExtendedResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "extendedResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>
parseExtendedResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "extendedResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(const ::std::string &u,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(const ::std::string &u,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(const ::std::string &u,
                     ::xercesc::DOMErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(u, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::std::istream &is,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::std::istream &is,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::std::istream &is,
                     ::xercesc::DOMErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is);
	return ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::std::istream &is,
                     const ::std::string &sid,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(isrc, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::std::istream &is,
                     const ::std::string &sid,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
	                                    (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::std::istream &is,
                     const ::std::string &sid,
                     ::xercesc::DOMErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::xml::sax::std_input_source isrc(is, sid);
	return ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(isrc, h, f, p);
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::xercesc::InputSource &i,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::xsd::cxx::tree::error_handler<char> h;

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	h.throw_if_failed<::xsd::cxx::tree::parsing<char>>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::xercesc::InputSource &i,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::xercesc::InputSource &i,
                     ::xercesc::DOMErrorHandler &h,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::xsd::cxx::xml::dom::parse<char>(i, h, p, f));

	if (!d.get()) throw ::xsd::cxx::tree::parsing<char>();

	return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>(
	    ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(
	        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(const ::xercesc::DOMDocument &doc,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     const ::LinphonePrivate::Xsd::XmlSchema::Properties &p) {
	if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) {
		::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
		    static_cast<::xercesc::DOMDocument *>(doc.cloneNode(true)));

		return ::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>(
		    ::LinphonePrivate::Xsd::XconCcmp::parseOptionsResponse(
		        std::move(d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
	}

	const ::xercesc::DOMElement &e(*doc.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	::std::unique_ptr<::xsd::cxx::tree::type> tmp(::xsd::cxx::tree::type_factory_map_instance<0, char>().create(
	    "optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {
		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "optionsResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>
parseOptionsResponse(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
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
	    "optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp",
	    &::xsd::cxx::tree::factory_impl<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType>, true, true, e, n, f,
	    0));

	if (tmp.get() != 0) {

		::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> r(
		    dynamic_cast<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType *>(tmp.get()));

		if (r.get()) tmp.release();
		else throw ::xsd::cxx::tree::not_derived<char>();

		return r;
	}

	throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "optionsResponse",
	                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
}
} // namespace XconCcmp
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
namespace XconCcmp {
void serializeCcmpRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeCcmpRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCcmpRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &s,
                          ::xercesc::DOMErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCcmpRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeCcmpRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCcmpRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &s,
                          ::xercesc::DOMErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCcmpRequest(::xercesc::DOMDocument &d,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType) == typeid(s)) {
		if (n.name() == "ccmpRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ccmpRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "ccmpRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCcmpRequest(const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("ccmpRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "ccmpRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeCcmpRequest(*d, s, f);
	return d;
}

void serializeCcmpResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeCcmpResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCcmpResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCcmpResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeCcmpResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCcmpResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeCcmpResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeCcmpResponse(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType) == typeid(s)) {
		if (n.name() == "ccmpResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "ccmpResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "ccmpResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCcmpResponse(const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("ccmpResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "ccmpResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeCcmpResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const CcmpRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// ccmpRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpRequestType::CcmpRequestType1 &x(i.getCcmpRequest());
		if (typeid(CcmpRequestType::CcmpRequestType1) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("ccmpRequest", e));

			s << x;
		} else tsm.serialize("ccmpRequest", "", false, false, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpRequestType>
    _xsd_CcmpRequestType_type_serializer_init("ccmp-request-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (CcmpRequestMessageType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// subject
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSubject()) {
			const CcmpRequestMessageType::SubjectType &x(*i.getSubject());
			if (typeid(CcmpRequestMessageType::SubjectType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("subject", e));

				s << x;
			} else tsm.serialize("subject", "", false, false, e, x);
		}
	}

	// confUserID
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConfUserID()) {
			const CcmpRequestMessageType::ConfUserIDType &x(*i.getConfUserID());
			if (typeid(CcmpRequestMessageType::ConfUserIDType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("confUserID", e));

				s << x;
			} else tsm.serialize("confUserID", "", false, false, e, x);
		}
	}

	// confObjID
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConfObjID()) {
			const CcmpRequestMessageType::ConfObjIDType &x(*i.getConfObjID());
			if (typeid(CcmpRequestMessageType::ConfObjIDType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("confObjID", e));

				s << x;
			} else tsm.serialize("confObjID", "", false, false, e, x);
		}
	}

	// operation
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getOperation()) {
			const CcmpRequestMessageType::OperationType &x(*i.getOperation());
			if (typeid(CcmpRequestMessageType::OperationType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("operation", e));

				s << x;
			} else tsm.serialize("operation", "", false, false, e, x);
		}
	}

	// conference-password
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConferencePassword()) {
			const CcmpRequestMessageType::ConferencePasswordType &x(*i.getConferencePassword());
			if (typeid(CcmpRequestMessageType::ConferencePasswordType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("conference-password", e));

				s << x;
			} else tsm.serialize("conference-password", "", false, false, e, x);
		}
	}

	// any
	//
	for (CcmpRequestMessageType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpRequestMessageType>
    _xsd_CcmpRequestMessageType_type_serializer_init("ccmp-request-message-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// ccmpResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpResponseType::CcmpResponseType1 &x(i.getCcmpResponse());
		if (typeid(CcmpResponseType::CcmpResponseType1) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("ccmpResponse", e));

			s << x;
		} else tsm.serialize("ccmpResponse", "", false, false, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpResponseType>
    _xsd_CcmpResponseType_type_serializer_init("ccmp-response-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (CcmpResponseMessageType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// confUserID
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpResponseMessageType::ConfUserIDType &x(i.getConfUserID());
		if (typeid(CcmpResponseMessageType::ConfUserIDType) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("confUserID", e));

			s << x;
		} else tsm.serialize("confUserID", "", false, false, e, x);
	}

	// confObjID
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConfObjID()) {
			const CcmpResponseMessageType::ConfObjIDType &x(*i.getConfObjID());
			if (typeid(CcmpResponseMessageType::ConfObjIDType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("confObjID", e));

				s << x;
			} else tsm.serialize("confObjID", "", false, false, e, x);
		}
	}

	// operation
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getOperation()) {
			const CcmpResponseMessageType::OperationType &x(*i.getOperation());
			if (typeid(CcmpResponseMessageType::OperationType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("operation", e));

				s << x;
			} else tsm.serialize("operation", "", false, false, e, x);
		}
	}

	// response-code
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpResponseMessageType::ResponseCodeType &x(i.getResponseCode());
		if (typeid(CcmpResponseMessageType::ResponseCodeType) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("response-code", e));

			s << x;
		} else tsm.serialize("response-code", "", false, false, e, x);
	}

	// response-string
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getResponseString()) {
			const CcmpResponseMessageType::ResponseStringType &x(*i.getResponseString());
			if (typeid(CcmpResponseMessageType::ResponseStringType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("response-string", e));

				s << x;
			} else tsm.serialize("response-string", "", false, false, e, x);
		}
	}

	// version
	//
	if (i.getVersion()) {
		::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("version", e));

		s << *i.getVersion();
	}

	// any
	//
	for (CcmpResponseMessageType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpResponseMessageType>
    _xsd_CcmpResponseMessageType_type_serializer_init("ccmp-response-message-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpBlueprintsRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// blueprintsRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpBlueprintsRequestMessageType::BlueprintsRequestType &x(i.getBlueprintsRequest());
		if (typeid(CcmpBlueprintsRequestMessageType::BlueprintsRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpBlueprintsRequestMessageType>
    _xsd_CcmpBlueprintsRequestMessageType_type_serializer_init("ccmp-blueprints-request-message-type",
                                                               "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeBlueprintsRequest(::std::ostream &o,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &s,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeBlueprintsRequest(::std::ostream &o,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &s,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintsRequest(::std::ostream &o,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &s,
                                ::xercesc::DOMErrorHandler &h,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintsRequest(::xercesc::XMLFormatTarget &t,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &s,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeBlueprintsRequest(::xercesc::XMLFormatTarget &t,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &s,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintsRequest(::xercesc::XMLFormatTarget &t,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &s,
                                ::xercesc::DOMErrorHandler &h,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintsRequest(::xercesc::DOMDocument &d,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &s,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType) == typeid(s)) {
		if (n.name() == "blueprintsRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintsRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeBlueprintsRequest(const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "blueprintsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const BlueprintsRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (BlueprintsRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// xpathFilter
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getXpathFilter()) {
			const BlueprintsRequestType::XpathFilterType &x(*i.getXpathFilter());
			if (typeid(BlueprintsRequestType::XpathFilterType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("xpathFilter", e));

				s << x;
			} else tsm.serialize("xpathFilter", "", false, false, e, x);
		}
	}

	// any
	//
	for (BlueprintsRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, BlueprintsRequestType>
    _xsd_BlueprintsRequestType_type_serializer_init("blueprintsRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpBlueprintRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// blueprintRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpBlueprintRequestMessageType::BlueprintRequestType &x(i.getBlueprintRequest());
		if (typeid(CcmpBlueprintRequestMessageType::BlueprintRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpBlueprintRequestMessageType>
    _xsd_CcmpBlueprintRequestMessageType_type_serializer_init("ccmp-blueprint-request-message-type",
                                                              "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeBlueprintRequest(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeBlueprintRequest(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintRequest(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintRequest(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeBlueprintRequest(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintRequest(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintRequest(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType) == typeid(s)) {
		if (n.name() == "blueprintRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeBlueprintRequest(const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "blueprintRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const BlueprintRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (BlueprintRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// blueprintInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getBlueprintInfo()) {
			const BlueprintRequestType::BlueprintInfoType &x(*i.getBlueprintInfo());
			if (typeid(BlueprintRequestType::BlueprintInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("blueprintInfo", e));

				s << x;
			} else tsm.serialize("blueprintInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (BlueprintRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, BlueprintRequestType>
    _xsd_BlueprintRequestType_type_serializer_init("blueprintRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpConfsRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// confsRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpConfsRequestMessageType::ConfsRequestType &x(i.getConfsRequest());
		if (typeid(CcmpConfsRequestMessageType::ConfsRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpConfsRequestMessageType>
    _xsd_CcmpConfsRequestMessageType_type_serializer_init("ccmp-confs-request-message-type",
                                                          "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeConfsRequest(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConfsRequest(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfsRequest(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfsRequest(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConfsRequest(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfsRequest(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfsRequest(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType) == typeid(s)) {
		if (n.name() == "confsRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confsRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConfsRequest(const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "confsRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeConfsRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const ConfsRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConfsRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// xpathFilter
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getXpathFilter()) {
			const ConfsRequestType::XpathFilterType &x(*i.getXpathFilter());
			if (typeid(ConfsRequestType::XpathFilterType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("xpathFilter", e));

				s << x;
			} else tsm.serialize("xpathFilter", "", false, false, e, x);
		}
	}

	// any
	//
	for (ConfsRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConfsRequestType>
    _xsd_ConfsRequestType_type_serializer_init("confsRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpConfRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// confRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpConfRequestMessageType::ConfRequestType &x(i.getConfRequest());
		if (typeid(CcmpConfRequestMessageType::ConfRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("confRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("confRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpConfRequestMessageType>
    _xsd_CcmpConfRequestMessageType_type_serializer_init("ccmp-conf-request-message-type",
                                                         "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeConfRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConfRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &s,
                          ::xercesc::DOMErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConfRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &s,
                          ::xercesc::DOMErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfRequest(::xercesc::DOMDocument &d,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ConfRequestType) == typeid(s)) {
		if (n.name() == "confRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "confRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConfRequest(const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ConfRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("confRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "confRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeConfRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const ConfRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConfRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// confInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConfInfo()) {
			const ConfRequestType::ConfInfoType &x(*i.getConfInfo());
			if (typeid(ConfRequestType::ConfInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("confInfo", e));

				s << x;
			} else tsm.serialize("confInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (ConfRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConfRequestType>
    _xsd_ConfRequestType_type_serializer_init("confRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpUsersRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// usersRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpUsersRequestMessageType::UsersRequestType &x(i.getUsersRequest());
		if (typeid(CcmpUsersRequestMessageType::UsersRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpUsersRequestMessageType>
    _xsd_CcmpUsersRequestMessageType_type_serializer_init("ccmp-users-request-message-type",
                                                          "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeUsersRequest(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUsersRequest(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUsersRequest(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUsersRequest(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUsersRequest(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUsersRequest(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUsersRequest(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::UsersRequestType) == typeid(s)) {
		if (n.name() == "usersRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "usersRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUsersRequest(const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::UsersRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "usersRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeUsersRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const UsersRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UsersRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// usersInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getUsersInfo()) {
			const UsersRequestType::UsersInfoType &x(*i.getUsersInfo());
			if (typeid(UsersRequestType::UsersInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("usersInfo", e));

				s << x;
			} else tsm.serialize("usersInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (UsersRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UsersRequestType>
    _xsd_UsersRequestType_type_serializer_init("usersRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpUserRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// userRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpUserRequestMessageType::UserRequestType &x(i.getUserRequest());
		if (typeid(CcmpUserRequestMessageType::UserRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("userRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("userRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpUserRequestMessageType>
    _xsd_CcmpUserRequestMessageType_type_serializer_init("ccmp-user-request-message-type",
                                                         "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeUserRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUserRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserRequest(::std::ostream &o,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &s,
                          ::xercesc::DOMErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUserRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserRequest(::xercesc::XMLFormatTarget &t,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &s,
                          ::xercesc::DOMErrorHandler &h,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          const ::std::string &e,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserRequest(::xercesc::DOMDocument &d,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &s,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::UserRequestType) == typeid(s)) {
		if (n.name() == "userRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "userRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "userRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUserRequest(const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &s,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::UserRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("userRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "userRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeUserRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const UserRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UserRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// userInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getUserInfo()) {
			const UserRequestType::UserInfoType &x(*i.getUserInfo());
			if (typeid(UserRequestType::UserInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("userInfo", e));

				s << x;
			} else tsm.serialize("userInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (UserRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UserRequestType>
    _xsd_UserRequestType_type_serializer_init("userRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpSidebarsByValRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// sidebarsByValRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpSidebarsByValRequestMessageType::SidebarsByValRequestType &x(i.getSidebarsByValRequest());
		if (typeid(CcmpSidebarsByValRequestMessageType::SidebarsByValRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpSidebarsByValRequestMessageType>
    _xsd_CcmpSidebarsByValRequestMessageType_type_serializer_init("ccmp-sidebarsByVal-request-message-type",
                                                                  "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeSidebarsByValRequest(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarsByValRequest(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByValRequest(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByValRequest(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarsByValRequest(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByValRequest(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByValRequest(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType) == typeid(s)) {
		if (n.name() == "sidebarsByValRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByValRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarsByValRequest(const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarsByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const SidebarsByValRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarsByValRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// xpathFilter
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getXpathFilter()) {
			const SidebarsByValRequestType::XpathFilterType &x(*i.getXpathFilter());
			if (typeid(SidebarsByValRequestType::XpathFilterType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("xpathFilter", e));

				s << x;
			} else tsm.serialize("xpathFilter", "", false, false, e, x);
		}
	}

	// any
	//
	for (SidebarsByValRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarsByValRequestType>
    _xsd_SidebarsByValRequestType_type_serializer_init("sidebarsByValRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpSidebarsByRefRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// sidebarsByRefRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpSidebarsByRefRequestMessageType::SidebarsByRefRequestType &x(i.getSidebarsByRefRequest());
		if (typeid(CcmpSidebarsByRefRequestMessageType::SidebarsByRefRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpSidebarsByRefRequestMessageType>
    _xsd_CcmpSidebarsByRefRequestMessageType_type_serializer_init("ccmp-sidebarsByRef-request-message-type",
                                                                  "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeSidebarsByRefRequest(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarsByRefRequest(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByRefRequest(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByRefRequest(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarsByRefRequest(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByRefRequest(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByRefRequest(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType) == typeid(s)) {
		if (n.name() == "sidebarsByRefRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByRefRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarsByRefRequest(const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarsByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const SidebarsByRefRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarsByRefRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// xpathFilter
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getXpathFilter()) {
			const SidebarsByRefRequestType::XpathFilterType &x(*i.getXpathFilter());
			if (typeid(SidebarsByRefRequestType::XpathFilterType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("xpathFilter", e));

				s << x;
			} else tsm.serialize("xpathFilter", "", false, false, e, x);
		}
	}

	// any
	//
	for (SidebarsByRefRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarsByRefRequestType>
    _xsd_SidebarsByRefRequestType_type_serializer_init("sidebarsByRefRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpSidebarByValRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// sidebarByValRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpSidebarByValRequestMessageType::SidebarByValRequestType &x(i.getSidebarByValRequest());
		if (typeid(CcmpSidebarByValRequestMessageType::SidebarByValRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpSidebarByValRequestMessageType>
    _xsd_CcmpSidebarByValRequestMessageType_type_serializer_init("ccmp-sidebarByVal-request-message-type",
                                                                 "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeSidebarByValRequest(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &s,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarByValRequest(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByValRequest(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &s,
                                  ::xercesc::DOMErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByValRequest(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &s,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarByValRequest(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByValRequest(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &s,
                                  ::xercesc::DOMErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByValRequest(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType) == typeid(s)) {
		if (n.name() == "sidebarByValRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByValRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarByValRequest(const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &s,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarByValRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const SidebarByValRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarByValRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// sidebarByValInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSidebarByValInfo()) {
			const SidebarByValRequestType::SidebarByValInfoType &x(*i.getSidebarByValInfo());
			if (typeid(SidebarByValRequestType::SidebarByValInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("sidebarByValInfo", e));

				s << x;
			} else tsm.serialize("sidebarByValInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (SidebarByValRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarByValRequestType>
    _xsd_SidebarByValRequestType_type_serializer_init("sidebarByValRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpSidebarByRefRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// sidebarByRefRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpSidebarByRefRequestMessageType::SidebarByRefRequestType &x(i.getSidebarByRefRequest());
		if (typeid(CcmpSidebarByRefRequestMessageType::SidebarByRefRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpSidebarByRefRequestMessageType>
    _xsd_CcmpSidebarByRefRequestMessageType_type_serializer_init("ccmp-sidebarByRef-request-message-type",
                                                                 "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeSidebarByRefRequest(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &s,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarByRefRequest(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByRefRequest(::std::ostream &o,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &s,
                                  ::xercesc::DOMErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByRefRequest(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &s,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarByRefRequest(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByRefRequest(::xercesc::XMLFormatTarget &t,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &s,
                                  ::xercesc::DOMErrorHandler &h,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                  const ::std::string &e,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByRefRequest(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &s,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType) == typeid(s)) {
		if (n.name() == "sidebarByRefRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByRefRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarByRefRequest(const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &s,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarByRefRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const SidebarByRefRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarByRefRequestType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// sidebarByRefInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSidebarByRefInfo()) {
			const SidebarByRefRequestType::SidebarByRefInfoType &x(*i.getSidebarByRefInfo());
			if (typeid(SidebarByRefRequestType::SidebarByRefInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("sidebarByRefInfo", e));

				s << x;
			} else tsm.serialize("sidebarByRefInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (SidebarByRefRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarByRefRequestType>
    _xsd_SidebarByRefRequestType_type_serializer_init("sidebarByRefRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpExtendedRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);

	// extendedRequest
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpExtendedRequestMessageType::ExtendedRequestType &x(i.getExtendedRequest());
		if (typeid(CcmpExtendedRequestMessageType::ExtendedRequestType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpExtendedRequestMessageType>
    _xsd_CcmpExtendedRequestMessageType_type_serializer_init("ccmp-extended-request-message-type",
                                                             "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeExtendedRequest(::std::ostream &o,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeExtendedRequest(::std::ostream &o,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &s,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeExtendedRequest(::std::ostream &o,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &s,
                              ::xercesc::DOMErrorHandler &h,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedRequest(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeExtendedRequest(::xercesc::XMLFormatTarget &t,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedRequest(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeExtendedRequest(::xercesc::XMLFormatTarget &t,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &s,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeExtendedRequest(::xercesc::XMLFormatTarget &t,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &s,
                              ::xercesc::DOMErrorHandler &h,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedRequest(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeExtendedRequest(::xercesc::DOMDocument &d,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &s,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType) == typeid(s)) {
		if (n.name() == "extendedRequest" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "extendedRequest",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeExtendedRequest(const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &s,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "extendedRequest", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeExtendedRequest(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const ExtendedRequestType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// extensionName
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const ExtendedRequestType::ExtensionNameType &x(i.getExtensionName());
		if (typeid(ExtendedRequestType::ExtensionNameType) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("extensionName", e));

			s << x;
		} else tsm.serialize("extensionName", "", false, false, e, x);
	}

	// any
	//
	for (ExtendedRequestType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ExtendedRequestType>
    _xsd_ExtendedRequestType_type_serializer_init("extendedRequestType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpOptionsRequestMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpOptionsRequestMessageType>
    _xsd_CcmpOptionsRequestMessageType_type_serializer_init("ccmp-options-request-message-type",
                                                            "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpBlueprintsResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// blueprintsResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpBlueprintsResponseMessageType::BlueprintsResponseType &x(i.getBlueprintsResponse());
		if (typeid(CcmpBlueprintsResponseMessageType::BlueprintsResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpBlueprintsResponseMessageType>
    _xsd_CcmpBlueprintsResponseMessageType_type_serializer_init("ccmp-blueprints-response-message-type",
                                                                "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeBlueprintsResponse(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &s,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeBlueprintsResponse(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintsResponse(::std::ostream &o,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &s,
                                 ::xercesc::DOMErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintsResponse(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &s,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeBlueprintsResponse(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintsResponse(::xercesc::XMLFormatTarget &t,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &s,
                                 ::xercesc::DOMErrorHandler &h,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                 const ::std::string &e,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintsResponse(::xercesc::DOMDocument &d,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &s,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType) == typeid(s)) {
		if (n.name() == "blueprintsResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintsResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeBlueprintsResponse(const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "blueprintsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintsResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const BlueprintsResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (BlueprintsResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// blueprintsInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getBlueprintsInfo()) {
			const BlueprintsResponseType::BlueprintsInfoType &x(*i.getBlueprintsInfo());
			if (typeid(BlueprintsResponseType::BlueprintsInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("blueprintsInfo", e));

				s << x;
			} else tsm.serialize("blueprintsInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (BlueprintsResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, BlueprintsResponseType>
    _xsd_BlueprintsResponseType_type_serializer_init("blueprintsResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpBlueprintResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// blueprintResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpBlueprintResponseMessageType::BlueprintResponseType &x(i.getBlueprintResponse());
		if (typeid(CcmpBlueprintResponseMessageType::BlueprintResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpBlueprintResponseMessageType>
    _xsd_CcmpBlueprintResponseMessageType_type_serializer_init("ccmp-blueprint-response-message-type",
                                                               "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeBlueprintResponse(::std::ostream &o,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &s,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeBlueprintResponse(::std::ostream &o,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &s,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintResponse(::std::ostream &o,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &s,
                                ::xercesc::DOMErrorHandler &h,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintResponse(::xercesc::XMLFormatTarget &t,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &s,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeBlueprintResponse(::xercesc::XMLFormatTarget &t,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &s,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintResponse(::xercesc::XMLFormatTarget &t,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &s,
                                ::xercesc::DOMErrorHandler &h,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                const ::std::string &e,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeBlueprintResponse(::xercesc::DOMDocument &d,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &s,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType) == typeid(s)) {
		if (n.name() == "blueprintResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "blueprintResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeBlueprintResponse(const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "blueprintResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeBlueprintResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const BlueprintResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (BlueprintResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// blueprintInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getBlueprintInfo()) {
			const BlueprintResponseType::BlueprintInfoType &x(*i.getBlueprintInfo());
			if (typeid(BlueprintResponseType::BlueprintInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("blueprintInfo", e));

				s << x;
			} else tsm.serialize("blueprintInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (BlueprintResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, BlueprintResponseType>
    _xsd_BlueprintResponseType_type_serializer_init("blueprintResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpConfsResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// confsResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpConfsResponseMessageType::ConfsResponseType &x(i.getConfsResponse());
		if (typeid(CcmpConfsResponseMessageType::ConfsResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpConfsResponseMessageType>
    _xsd_CcmpConfsResponseMessageType_type_serializer_init("ccmp-confs-response-message-type",
                                                           "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeConfsResponse(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConfsResponse(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfsResponse(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfsResponse(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConfsResponse(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfsResponse(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfsResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfsResponse(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType) == typeid(s)) {
		if (n.name() == "confsResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confsResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConfsResponse(const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "confsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeConfsResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const ConfsResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConfsResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// confsInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConfsInfo()) {
			const ConfsResponseType::ConfsInfoType &x(*i.getConfsInfo());
			if (typeid(ConfsResponseType::ConfsInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("confsInfo", e));

				s << x;
			} else tsm.serialize("confsInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (ConfsResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConfsResponseType>
    _xsd_ConfsResponseType_type_serializer_init("confsResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpConfResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// confResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpConfResponseMessageType::ConfResponseType &x(i.getConfResponse());
		if (typeid(CcmpConfResponseMessageType::ConfResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("confResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("confResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpConfResponseMessageType>
    _xsd_CcmpConfResponseMessageType_type_serializer_init("ccmp-conf-response-message-type",
                                                          "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeConfResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConfResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeConfResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeConfResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeConfResponse(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ConfResponseType) == typeid(s)) {
		if (n.name() == "confResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "confResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "confResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConfResponse(const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ConfResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("confResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "confResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeConfResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const ConfResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ConfResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// confInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getConfInfo()) {
			const ConfResponseType::ConfInfoType &x(*i.getConfInfo());
			if (typeid(ConfResponseType::ConfInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("confInfo", e));

				s << x;
			} else tsm.serialize("confInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (ConfResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ConfResponseType>
    _xsd_ConfResponseType_type_serializer_init("confResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpUsersResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// usersResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpUsersResponseMessageType::UsersResponseType &x(i.getUsersResponse());
		if (typeid(CcmpUsersResponseMessageType::UsersResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpUsersResponseMessageType>
    _xsd_CcmpUsersResponseMessageType_type_serializer_init("ccmp-users-response-message-type",
                                                           "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeUsersResponse(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUsersResponse(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUsersResponse(::std::ostream &o,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUsersResponse(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &s,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUsersResponse(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUsersResponse(::xercesc::XMLFormatTarget &t,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &s,
                            ::xercesc::DOMErrorHandler &h,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                            const ::std::string &e,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUsersResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUsersResponse(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &s,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::UsersResponseType) == typeid(s)) {
		if (n.name() == "usersResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "usersResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUsersResponse(const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &s,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::UsersResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "usersResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeUsersResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const UsersResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UsersResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// usersInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getUsersInfo()) {
			const UsersResponseType::UsersInfoType &x(*i.getUsersInfo());
			if (typeid(UsersResponseType::UsersInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("usersInfo", e));

				s << x;
			} else tsm.serialize("usersInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (UsersResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UsersResponseType>
    _xsd_UsersResponseType_type_serializer_init("usersResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpUserResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// userResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpUserResponseMessageType::UserResponseType &x(i.getUserResponse());
		if (typeid(CcmpUserResponseMessageType::UserResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("userResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("userResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpUserResponseMessageType>
    _xsd_CcmpUserResponseMessageType_type_serializer_init("ccmp-user-response-message-type",
                                                          "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeUserResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUserResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserResponse(::std::ostream &o,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &s,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeUserResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserResponse(::xercesc::XMLFormatTarget &t,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &s,
                           ::xercesc::DOMErrorHandler &h,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                           const ::std::string &e,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeUserResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeUserResponse(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &s,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::UserResponseType) == typeid(s)) {
		if (n.name() == "userResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "userResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "userResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUserResponse(const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &s,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::UserResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("userResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "userResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeUserResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const UserResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (UserResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// userInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getUserInfo()) {
			const UserResponseType::UserInfoType &x(*i.getUserInfo());
			if (typeid(UserResponseType::UserInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("userInfo", e));

				s << x;
			} else tsm.serialize("userInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (UserResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, UserResponseType>
    _xsd_UserResponseType_type_serializer_init("userResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpSidebarsByValResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// sidebarsByValResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpSidebarsByValResponseMessageType::SidebarsByValResponseType &x(i.getSidebarsByValResponse());
		if (typeid(CcmpSidebarsByValResponseMessageType::SidebarsByValResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpSidebarsByValResponseMessageType>
    _xsd_CcmpSidebarsByValResponseMessageType_type_serializer_init("ccmp-sidebarsByVal-response-message-type",
                                                                   "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeSidebarsByValResponse(::std::ostream &o,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &s,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarsByValResponse(::std::ostream &o,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &s,
                                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByValResponse(::std::ostream &o,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &s,
                                    ::xercesc::DOMErrorHandler &h,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByValResponse(::xercesc::XMLFormatTarget &t,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &s,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarsByValResponse(::xercesc::XMLFormatTarget &t,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &s,
                                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByValResponse(::xercesc::XMLFormatTarget &t,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &s,
                                    ::xercesc::DOMErrorHandler &h,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByValResponse(::xercesc::DOMDocument &d,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &s,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType) == typeid(s)) {
		if (n.name() == "sidebarsByValResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByValResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarsByValResponse(const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarsByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByValResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const SidebarsByValResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarsByValResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// sidebarsByValInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSidebarsByValInfo()) {
			const SidebarsByValResponseType::SidebarsByValInfoType &x(*i.getSidebarsByValInfo());
			if (typeid(SidebarsByValResponseType::SidebarsByValInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("sidebarsByValInfo", e));

				s << x;
			} else tsm.serialize("sidebarsByValInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (SidebarsByValResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarsByValResponseType>
    _xsd_SidebarsByValResponseType_type_serializer_init("sidebarsByValResponseType",
                                                        "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpSidebarsByRefResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// sidebarsByRefResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpSidebarsByRefResponseMessageType::SidebarsByRefResponseType &x(i.getSidebarsByRefResponse());
		if (typeid(CcmpSidebarsByRefResponseMessageType::SidebarsByRefResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpSidebarsByRefResponseMessageType>
    _xsd_CcmpSidebarsByRefResponseMessageType_type_serializer_init("ccmp-sidebarsByRef-response-message-type",
                                                                   "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeSidebarsByRefResponse(::std::ostream &o,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &s,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarsByRefResponse(::std::ostream &o,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &s,
                                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByRefResponse(::std::ostream &o,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &s,
                                    ::xercesc::DOMErrorHandler &h,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByRefResponse(::xercesc::XMLFormatTarget &t,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &s,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarsByRefResponse(::xercesc::XMLFormatTarget &t,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &s,
                                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByRefResponse(::xercesc::XMLFormatTarget &t,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &s,
                                    ::xercesc::DOMErrorHandler &h,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                    const ::std::string &e,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarsByRefResponse(::xercesc::DOMDocument &d,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &s,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType) == typeid(s)) {
		if (n.name() == "sidebarsByRefResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarsByRefResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarsByRefResponse(const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarsByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeSidebarsByRefResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const SidebarsByRefResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarsByRefResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// sidebarsByRefInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSidebarsByRefInfo()) {
			const SidebarsByRefResponseType::SidebarsByRefInfoType &x(*i.getSidebarsByRefInfo());
			if (typeid(SidebarsByRefResponseType::SidebarsByRefInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("sidebarsByRefInfo", e));

				s << x;
			} else tsm.serialize("sidebarsByRefInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (SidebarsByRefResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarsByRefResponseType>
    _xsd_SidebarsByRefResponseType_type_serializer_init("sidebarsByRefResponseType",
                                                        "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpSidebarByValResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// sidebarByValResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpSidebarByValResponseMessageType::SidebarByValResponseType &x(i.getSidebarByValResponse());
		if (typeid(CcmpSidebarByValResponseMessageType::SidebarByValResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpSidebarByValResponseMessageType>
    _xsd_CcmpSidebarByValResponseMessageType_type_serializer_init("ccmp-sidebarByVal-response-message-type",
                                                                  "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeSidebarByValResponse(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarByValResponse(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByValResponse(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByValResponse(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarByValResponse(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByValResponse(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByValResponse(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType) == typeid(s)) {
		if (n.name() == "sidebarByValResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByValResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarByValResponse(const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarByValResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByValResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const SidebarByValResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarByValResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// sidebarByValInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSidebarByValInfo()) {
			const SidebarByValResponseType::SidebarByValInfoType &x(*i.getSidebarByValInfo());
			if (typeid(SidebarByValResponseType::SidebarByValInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("sidebarByValInfo", e));

				s << x;
			} else tsm.serialize("sidebarByValInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (SidebarByValResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarByValResponseType>
    _xsd_SidebarByValResponseType_type_serializer_init("sidebarByValResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpSidebarByRefResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// sidebarByRefResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpSidebarByRefResponseMessageType::SidebarByRefResponseType &x(i.getSidebarByRefResponse());
		if (typeid(CcmpSidebarByRefResponseMessageType::SidebarByRefResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpSidebarByRefResponseMessageType>
    _xsd_CcmpSidebarByRefResponseMessageType_type_serializer_init("ccmp-sidebarByRef-response-message-type",
                                                                  "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeSidebarByRefResponse(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarByRefResponse(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByRefResponse(::std::ostream &o,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByRefResponse(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &s,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeSidebarByRefResponse(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByRefResponse(::xercesc::XMLFormatTarget &t,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &s,
                                   ::xercesc::DOMErrorHandler &h,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                                   const ::std::string &e,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeSidebarByRefResponse(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &s,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType) == typeid(s)) {
		if (n.name() == "sidebarByRefResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "sidebarByRefResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarByRefResponse(const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "sidebarByRefResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeSidebarByRefResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const SidebarByRefResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SidebarByRefResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// sidebarByRefInfo
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSidebarByRefInfo()) {
			const SidebarByRefResponseType::SidebarByRefInfoType &x(*i.getSidebarByRefInfo());
			if (typeid(SidebarByRefResponseType::SidebarByRefInfoType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("sidebarByRefInfo", e));

				s << x;
			} else tsm.serialize("sidebarByRefInfo", "", false, false, e, x);
		}
	}

	// any
	//
	for (SidebarByRefResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SidebarByRefResponseType>
    _xsd_SidebarByRefResponseType_type_serializer_init("sidebarByRefResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpExtendedResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// extendedResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpExtendedResponseMessageType::ExtendedResponseType &x(i.getExtendedResponse());
		if (typeid(CcmpExtendedResponseMessageType::ExtendedResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpExtendedResponseMessageType>
    _xsd_CcmpExtendedResponseMessageType_type_serializer_init("ccmp-extended-response-message-type",
                                                              "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeExtendedResponse(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeExtendedResponse(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeExtendedResponse(::std::ostream &o,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeExtendedResponse(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &s,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeExtendedResponse(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeExtendedResponse(::xercesc::XMLFormatTarget &t,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &s,
                               ::xercesc::DOMErrorHandler &h,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                               const ::std::string &e,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeExtendedResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeExtendedResponse(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &s,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType) == typeid(s)) {
		if (n.name() == "extendedResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "extendedResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeExtendedResponse(const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &s,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "extendedResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeExtendedResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const ExtendedResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// extensionName
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const ExtendedResponseType::ExtensionNameType &x(i.getExtensionName());
		if (typeid(ExtendedResponseType::ExtensionNameType) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("extensionName", e));

			s << x;
		} else tsm.serialize("extensionName", "", false, false, e, x);
	}

	// any
	//
	for (ExtendedResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ExtendedResponseType>
    _xsd_ExtendedResponseType_type_serializer_init("extendedResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const CcmpOptionsResponseMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType &>(i);

	// optionsResponse
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const CcmpOptionsResponseMessageType::OptionsResponseType &x(i.getOptionsResponse());
		if (typeid(CcmpOptionsResponseMessageType::OptionsResponseType) == typeid(x)) {
			::xercesc::DOMElement &s(
			    ::xsd::cxx::xml::dom::create_element("optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e));

			s << x;
		} else tsm.serialize("optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", true, true, e, x);
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, CcmpOptionsResponseMessageType>
    _xsd_CcmpOptionsResponseMessageType_type_serializer_init("ccmp-options-response-message-type",
                                                             "urn:ietf:params:xml:ns:xcon-ccmp");

void serializeOptionsResponse(::std::ostream &o,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeOptionsResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeOptionsResponse(::std::ostream &o,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &s,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::xsd::cxx::xml::auto_initializer i((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeOptionsResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeOptionsResponse(::std::ostream &o,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &s,
                              ::xercesc::DOMErrorHandler &h,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeOptionsResponse(s, m, f));
	::xsd::cxx::xml::dom::ostream_format_target t(o);
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeOptionsResponse(::xercesc::XMLFormatTarget &t,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &s,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeOptionsResponse(s, m, f));

	::xsd::cxx::tree::error_handler<char> h;

	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		h.throw_if_failed<::xsd::cxx::tree::serialization<char>>();
	}
}

void serializeOptionsResponse(::xercesc::XMLFormatTarget &t,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &s,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &h,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeOptionsResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeOptionsResponse(::xercesc::XMLFormatTarget &t,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &s,
                              ::xercesc::DOMErrorHandler &h,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                              const ::std::string &e,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d(
	    ::LinphonePrivate::Xsd::XconCcmp::serializeOptionsResponse(s, m, f));
	if (!::xsd::cxx::xml::dom::serialize(t, *d, e, h, f)) {
		throw ::xsd::cxx::tree::serialization<char>();
	}
}

void serializeOptionsResponse(::xercesc::DOMDocument &d,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &s,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags) {
	::xercesc::DOMElement &e(*d.getDocumentElement());
	const ::xsd::cxx::xml::qualified_name<char> n(::xsd::cxx::xml::dom::name<char>(e));

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType) == typeid(s)) {
		if (n.name() == "optionsResponse" && n.namespace_() == "urn:ietf:params:xml:ns:xcon-ccmp") {
			e << s;
		} else {
			throw ::xsd::cxx::tree::unexpected_element<char>(n.name(), n.namespace_(), "optionsResponse",
			                                                 "urn:ietf:params:xml:ns:xcon-ccmp");
		}
	} else {
		::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", e, n, s);
	}
}

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeOptionsResponse(const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &s,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f) {
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d;

	if (typeid(::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType) == typeid(s)) {
		d = ::xsd::cxx::xml::dom::serialize<char>("optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, f);
	} else {
		d = ::xsd::cxx::tree::type_serializer_map_instance<0, char>().serialize(
		    "optionsResponse", "urn:ietf:params:xml:ns:xcon-ccmp", m, s, f);
	}

	::LinphonePrivate::Xsd::XconCcmp::serializeOptionsResponse(*d, s, f);
	return d;
}

void operator<<(::xercesc::DOMElement &e, const OptionsResponseType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (OptionsResponseType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// options
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getOptions()) {
			const OptionsResponseType::OptionsType &x(*i.getOptions());
			if (typeid(OptionsResponseType::OptionsType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("options", e));

				s << x;
			} else tsm.serialize("options", "", false, false, e, x);
		}
	}

	// any
	//
	for (OptionsResponseType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, OptionsResponseType>
    _xsd_OptionsResponseType_type_serializer_init("optionsResponseType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const ResponseCodeType &i) {
	e << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const ResponseCodeType &i) {
	a << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const ResponseCodeType &i) {
	l << static_cast<const ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger, char,
	                                                          ::LinphonePrivate::Xsd::XmlSchema::SimpleType> &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ResponseCodeType>
    _xsd_ResponseCodeType_type_serializer_init("response-codeType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const OperationType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Token &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const OperationType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Token &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const OperationType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Token &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, OperationType>
    _xsd_OperationType_type_serializer_init("operationType", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const SubjectType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (SubjectType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// username
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getUsername()) {
			const SubjectType::UsernameType &x(*i.getUsername());
			if (typeid(SubjectType::UsernameType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("username", e));

				s << x;
			} else tsm.serialize("username", "", false, false, e, x);
		}
	}

	// password
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getPassword()) {
			const SubjectType::PasswordType &x(*i.getPassword());
			if (typeid(SubjectType::PasswordType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("password", e));

				s << x;
			} else tsm.serialize("password", "", false, false, e, x);
		}
	}

	// any
	//
	for (SubjectType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, SubjectType>
    _xsd_SubjectType_type_serializer_init("subject-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const OptionsType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (OptionsType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// standard-message-list
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const OptionsType::StandardMessageListType &x(i.getStandardMessageList());
		if (typeid(OptionsType::StandardMessageListType) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("standard-message-list", e));

			s << x;
		} else tsm.serialize("standard-message-list", "", false, false, e, x);
	}

	// extended-message-list
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getExtendedMessageList()) {
			const OptionsType::ExtendedMessageListType &x(*i.getExtendedMessageList());
			if (typeid(OptionsType::ExtendedMessageListType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("extended-message-list", e));

				s << x;
			} else tsm.serialize("extended-message-list", "", false, false, e, x);
		}
	}

	// any
	//
	for (OptionsType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, OptionsType>
    _xsd_OptionsType_type_serializer_init("options-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const StandardMessageListType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (StandardMessageListType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// standard-message
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (StandardMessageListType::StandardMessageConstIterator b(i.getStandardMessage().begin()),
		     n(i.getStandardMessage().end());
		     b != n; ++b) {
			const StandardMessageListType::StandardMessageType &x(*b);

			if (typeid(StandardMessageListType::StandardMessageType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("standard-message", e));

				s << x;
			} else tsm.serialize("standard-message", "", false, false, e, x);
		}
	}

	// any
	//
	for (StandardMessageListType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, StandardMessageListType>
    _xsd_StandardMessageListType_type_serializer_init("standard-message-list-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const StandardMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (StandardMessageType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// name
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const StandardMessageType::NameType &x(i.getName());
		if (typeid(StandardMessageType::NameType) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("name", e));

			s << x;
		} else tsm.serialize("name", "", false, false, e, x);
	}

	// operations
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getOperations()) {
			const StandardMessageType::OperationsType &x(*i.getOperations());
			if (typeid(StandardMessageType::OperationsType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("operations", e));

				s << x;
			} else tsm.serialize("operations", "", false, false, e, x);
		}
	}

	// schema-def
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getSchemaDef()) {
			const StandardMessageType::SchemaDefType &x(*i.getSchemaDef());
			if (typeid(StandardMessageType::SchemaDefType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("schema-def", e));

				s << x;
			} else tsm.serialize("schema-def", "", false, false, e, x);
		}
	}

	// description
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDescription()) {
			const StandardMessageType::DescriptionType &x(*i.getDescription());
			if (typeid(StandardMessageType::DescriptionType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("description", e));

				s << x;
			} else tsm.serialize("description", "", false, false, e, x);
		}
	}

	// any
	//
	for (StandardMessageType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, StandardMessageType>
    _xsd_StandardMessageType_type_serializer_init("standard-message-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const StandardMessageNameType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Token &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const StandardMessageNameType &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Token &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const StandardMessageNameType &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Token &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, StandardMessageNameType>
    _xsd_StandardMessageNameType_type_serializer_init("standard-message-name-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const OperationsType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (OperationsType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end()); b != n;
	     ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// operation
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		for (OperationsType::OperationConstIterator b(i.getOperation().begin()), n(i.getOperation().end()); b != n;
		     ++b) {
			const OperationsType::OperationType &x(*b);

			if (typeid(OperationsType::OperationType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("operation", e));

				s << x;
			} else tsm.serialize("operation", "", false, false, e, x);
		}
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, OperationsType>
    _xsd_OperationsType_type_serializer_init("operations-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const ExtendedMessageListType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ExtendedMessageListType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()),
	     n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// extended-message
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getExtendedMessage()) {
			const ExtendedMessageListType::ExtendedMessageType &x(*i.getExtendedMessage());
			if (typeid(ExtendedMessageListType::ExtendedMessageType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("extended-message", e));

				s << x;
			} else tsm.serialize("extended-message", "", false, false, e, x);
		}
	}

	// any
	//
	for (ExtendedMessageListType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ExtendedMessageListType>
    _xsd_ExtendedMessageListType_type_serializer_init("extended-message-list-type", "urn:ietf:params:xml:ns:xcon-ccmp");

void operator<<(::xercesc::DOMElement &e, const ExtendedMessageType &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::Type &>(i);

	// any_attribute
	//
	for (ExtendedMessageType::AnyAttributeConstIterator b(i.getAnyAttribute().begin()), n(i.getAnyAttribute().end());
	     b != n; ++b) {
		::xercesc::DOMAttr *a(static_cast<::xercesc::DOMAttr *>(
		    e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMAttr *>(&(*b)), true)));

		if (a->getLocalName() == 0) e.setAttributeNode(a);
		else e.setAttributeNodeNS(a);
	}

	// name
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const ExtendedMessageType::NameType &x(i.getName());
		if (typeid(ExtendedMessageType::NameType) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("name", e));

			s << x;
		} else tsm.serialize("name", "", false, false, e, x);
	}

	// operations
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getOperations()) {
			const ExtendedMessageType::OperationsType &x(*i.getOperations());
			if (typeid(ExtendedMessageType::OperationsType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("operations", e));

				s << x;
			} else tsm.serialize("operations", "", false, false, e, x);
		}
	}

	// schema-def
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		const ExtendedMessageType::SchemaDefType &x(i.getSchemaDef());
		if (typeid(ExtendedMessageType::SchemaDefType) == typeid(x)) {
			::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("schema-def", e));

			s << x;
		} else tsm.serialize("schema-def", "", false, false, e, x);
	}

	// description
	//
	{
		::xsd::cxx::tree::type_serializer_map<char> &tsm(::xsd::cxx::tree::type_serializer_map_instance<0, char>());

		if (i.getDescription()) {
			const ExtendedMessageType::DescriptionType &x(*i.getDescription());
			if (typeid(ExtendedMessageType::DescriptionType) == typeid(x)) {
				::xercesc::DOMElement &s(::xsd::cxx::xml::dom::create_element("description", e));

				s << x;
			} else tsm.serialize("description", "", false, false, e, x);
		}
	}

	// any
	//
	for (ExtendedMessageType::AnyConstIterator b(i.getAny().begin()), n(i.getAny().end()); b != n; ++b) {
		e.appendChild(e.getOwnerDocument()->importNode(const_cast<::xercesc::DOMElement *>(&(*b)), true));
	}
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, ExtendedMessageType>
    _xsd_ExtendedMessageType_type_serializer_init("extended-message-type", "urn:ietf:params:xml:ns:xcon-ccmp");
} // namespace XconCcmp
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
