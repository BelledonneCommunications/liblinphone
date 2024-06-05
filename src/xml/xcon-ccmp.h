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

#ifndef XCON_CCMP_H
#define XCON_CCMP_H

#ifndef XSD_CXX11
#define XSD_CXX11
#endif

#ifndef XSD_USE_CHAR
#define XSD_USE_CHAR
#endif

#ifndef XSD_CXX_TREE_USE_CHAR
#define XSD_CXX_TREE_USE_CHAR
#endif

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

#include <xsd/cxx/config.hxx>

#include <xsd/cxx/pre.hxx>

#include <xsd/cxx/xml/char-utf8.hxx>

#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/types.hxx>

#include <xsd/cxx/xml/error-handler.hxx>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

#include <xsd/cxx/tree/parsing.hxx>
#include <xsd/cxx/tree/parsing/boolean.hxx>
#include <xsd/cxx/tree/parsing/byte.hxx>
#include <xsd/cxx/tree/parsing/decimal.hxx>
#include <xsd/cxx/tree/parsing/double.hxx>
#include <xsd/cxx/tree/parsing/float.hxx>
#include <xsd/cxx/tree/parsing/int.hxx>
#include <xsd/cxx/tree/parsing/long.hxx>
#include <xsd/cxx/tree/parsing/short.hxx>
#include <xsd/cxx/tree/parsing/unsigned-byte.hxx>
#include <xsd/cxx/tree/parsing/unsigned-int.hxx>
#include <xsd/cxx/tree/parsing/unsigned-long.hxx>
#include <xsd/cxx/tree/parsing/unsigned-short.hxx>

#include <xsd/cxx/tree/serialization.hxx>
#include <xsd/cxx/tree/serialization/boolean.hxx>
#include <xsd/cxx/tree/serialization/byte.hxx>
#include <xsd/cxx/tree/serialization/decimal.hxx>
#include <xsd/cxx/tree/serialization/double.hxx>
#include <xsd/cxx/tree/serialization/float.hxx>
#include <xsd/cxx/tree/serialization/int.hxx>
#include <xsd/cxx/tree/serialization/long.hxx>
#include <xsd/cxx/tree/serialization/short.hxx>
#include <xsd/cxx/tree/serialization/unsigned-byte.hxx>
#include <xsd/cxx/tree/serialization/unsigned-int.hxx>
#include <xsd/cxx/tree/serialization/unsigned-long.hxx>
#include <xsd/cxx/tree/serialization/unsigned-short.hxx>
#include <xsd/cxx/xml/dom/serialization-header.hxx>

#include <xsd/cxx/tree/std-ostream-operators.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace XmlSchema {
// anyType and anySimpleType.
//
typedef ::xsd::cxx::tree::type Type;
typedef ::xsd::cxx::tree::simple_type<char, Type> SimpleType;
typedef ::xsd::cxx::tree::type Container;

// 8-bit
//
typedef signed char Byte;
typedef unsigned char UnsignedByte;

// 16-bit
//
typedef short Short;
typedef unsigned short UnsignedShort;

// 32-bit
//
typedef int Int;
typedef unsigned int UnsignedInt;

// 64-bit
//
typedef long long Long;
typedef unsigned long long UnsignedLong;

// Supposed to be arbitrary-length integral types.
//
typedef long long Integer;
typedef long long NonPositiveInteger;
typedef unsigned long long NonNegativeInteger;
typedef unsigned long long PositiveInteger;
typedef long long NegativeInteger;

// Boolean.
//
typedef bool Boolean;

// Floating-point types.
//
typedef float Float;
typedef double Double;
typedef double Decimal;

// String types.
//
typedef ::xsd::cxx::tree::string<char, SimpleType> String;
typedef ::xsd::cxx::tree::normalized_string<char, String> NormalizedString;
typedef ::xsd::cxx::tree::token<char, NormalizedString> Token;
typedef ::xsd::cxx::tree::name<char, Token> Name;
typedef ::xsd::cxx::tree::nmtoken<char, Token> Nmtoken;
typedef ::xsd::cxx::tree::nmtokens<char, SimpleType, Nmtoken> Nmtokens;
typedef ::xsd::cxx::tree::ncname<char, Name> Ncname;
typedef ::xsd::cxx::tree::language<char, Token> Language;

// ID/IDREF.
//
typedef ::xsd::cxx::tree::id<char, Ncname> Id;
typedef ::xsd::cxx::tree::idref<char, Ncname, Type> Idref;
typedef ::xsd::cxx::tree::idrefs<char, SimpleType, Idref> Idrefs;

// URI.
//
typedef ::xsd::cxx::tree::uri<char, SimpleType> Uri;

// Qualified name.
//
typedef ::xsd::cxx::tree::qname<char, SimpleType, Uri, Ncname> Qname;

// Binary.
//
typedef ::xsd::cxx::tree::buffer<char> Buffer;
typedef ::xsd::cxx::tree::base64_binary<char, SimpleType> Base64Binary;
typedef ::xsd::cxx::tree::hex_binary<char, SimpleType> HexBinary;

// Date/time.
//
typedef ::xsd::cxx::tree::time_zone TimeZone;
typedef ::xsd::cxx::tree::date<char, SimpleType> Date;
typedef ::xsd::cxx::tree::date_time<char, SimpleType> DateTime;
typedef ::xsd::cxx::tree::duration<char, SimpleType> Duration;
typedef ::xsd::cxx::tree::gday<char, SimpleType> Gday;
typedef ::xsd::cxx::tree::gmonth<char, SimpleType> Gmonth;
typedef ::xsd::cxx::tree::gmonth_day<char, SimpleType> GmonthDay;
typedef ::xsd::cxx::tree::gyear<char, SimpleType> Gyear;
typedef ::xsd::cxx::tree::gyear_month<char, SimpleType> GyearMonth;
typedef ::xsd::cxx::tree::time<char, SimpleType> Time;

// Entity.
//
typedef ::xsd::cxx::tree::entity<char, Ncname> Entity;
typedef ::xsd::cxx::tree::entities<char, SimpleType, Entity> Entities;

typedef ::xsd::cxx::tree::content_order ContentOrder;
// Namespace information and list stream. Used in
// serialization functions.
//
typedef ::xsd::cxx::xml::dom::namespace_info<char> NamespaceInfo;
typedef ::xsd::cxx::xml::dom::namespace_infomap<char> NamespaceInfomap;
typedef ::xsd::cxx::tree::list_stream<char> ListStream;
typedef ::xsd::cxx::tree::as_double<Double> AsDouble;
typedef ::xsd::cxx::tree::as_decimal<Decimal> AsDecimal;
typedef ::xsd::cxx::tree::facet Facet;

// Flags and properties.
//
typedef ::xsd::cxx::tree::flags Flags;
typedef ::xsd::cxx::tree::properties<char> Properties;

// Parsing/serialization diagnostics.
//
typedef ::xsd::cxx::tree::severity Severity;
typedef ::xsd::cxx::tree::error<char> Error;
typedef ::xsd::cxx::tree::diagnostics<char> Diagnostics;

// Exceptions.
//
typedef ::xsd::cxx::tree::exception<char> Exception;
typedef ::xsd::cxx::tree::bounds<char> Bounds;
typedef ::xsd::cxx::tree::duplicate_id<char> DuplicateId;
typedef ::xsd::cxx::tree::parsing<char> Parsing;
typedef ::xsd::cxx::tree::expected_element<char> ExpectedElement;
typedef ::xsd::cxx::tree::unexpected_element<char> UnexpectedElement;
typedef ::xsd::cxx::tree::expected_attribute<char> ExpectedAttribute;
typedef ::xsd::cxx::tree::unexpected_enumerator<char> UnexpectedEnumerator;
typedef ::xsd::cxx::tree::expected_text_content<char> ExpectedTextContent;
typedef ::xsd::cxx::tree::no_prefix_mapping<char> NoPrefixMapping;
typedef ::xsd::cxx::tree::no_type_info<char> NoTypeInfo;
typedef ::xsd::cxx::tree::not_derived<char> NotDerived;
typedef ::xsd::cxx::tree::serialization<char> Serialization;

// Error handler callback interface.
//
typedef ::xsd::cxx::xml::error_handler<char> ErrorHandler;

// DOM interaction.
//
namespace dom {
// Automatic pointer for DOMDocument.
//
using ::xsd::cxx::xml::dom::unique_ptr;

#ifndef XSD_CXX_TREE_TREE_NODE_KEY__LINPHONEPRIVATE__XSD__XMLSCHEMA
#define XSD_CXX_TREE_TREE_NODE_KEY__LINPHONEPRIVATE__XSD__XMLSCHEMA
// DOM user data key for back pointers to tree nodes.
//
const XMLCh *const treeNodeKey = ::xsd::cxx::tree::user_data_keys::node;
#endif
} // namespace dom
} // namespace XmlSchema
} // namespace Xsd
} // namespace LinphonePrivate

// Forward declarations.
//
namespace LinphonePrivate {
namespace Xsd {
namespace XconCcmp {
class CcmpRequestType;
class CcmpRequestMessageType;
class CcmpResponseType;
class CcmpResponseMessageType;
class CcmpBlueprintsRequestMessageType;
class BlueprintsRequestType;
class CcmpBlueprintRequestMessageType;
class BlueprintRequestType;
class CcmpConfsRequestMessageType;
class ConfsRequestType;
class CcmpConfRequestMessageType;
class ConfRequestType;
class CcmpUsersRequestMessageType;
class UsersRequestType;
class CcmpUserRequestMessageType;
class UserRequestType;
class CcmpSidebarsByValRequestMessageType;
class SidebarsByValRequestType;
class CcmpSidebarsByRefRequestMessageType;
class SidebarsByRefRequestType;
class CcmpSidebarByValRequestMessageType;
class SidebarByValRequestType;
class CcmpSidebarByRefRequestMessageType;
class SidebarByRefRequestType;
class CcmpExtendedRequestMessageType;
class ExtendedRequestType;
class CcmpOptionsRequestMessageType;
class CcmpBlueprintsResponseMessageType;
class BlueprintsResponseType;
class CcmpBlueprintResponseMessageType;
class BlueprintResponseType;
class CcmpConfsResponseMessageType;
class ConfsResponseType;
class CcmpConfResponseMessageType;
class ConfResponseType;
class CcmpUsersResponseMessageType;
class UsersResponseType;
class CcmpUserResponseMessageType;
class UserResponseType;
class CcmpSidebarsByValResponseMessageType;
class SidebarsByValResponseType;
class CcmpSidebarsByRefResponseMessageType;
class SidebarsByRefResponseType;
class CcmpSidebarByValResponseMessageType;
class SidebarByValResponseType;
class CcmpSidebarByRefResponseMessageType;
class SidebarByRefResponseType;
class CcmpExtendedResponseMessageType;
class ExtendedResponseType;
class CcmpOptionsResponseMessageType;
class OptionsResponseType;
class ResponseCodeType;
class OperationType;
class SubjectType;
class OptionsType;
class StandardMessageListType;
class StandardMessageType;
class StandardMessageNameType;
class OperationsType;
class ExtendedMessageListType;
class ExtendedMessageType;
} // namespace XconCcmp
} // namespace Xsd
} // namespace LinphonePrivate

#include <algorithm> // std::binary_search
#include <limits>    // std::numeric_limits
#include <memory>    // ::std::unique_ptr
#include <utility>   // std::move

#include <xsd/cxx/xml/char-utf8.hxx>

#include <xsd/cxx/tree/containers.hxx>
#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/list.hxx>

#include <xsd/cxx/xml/dom/parsing-header.hxx>

#include <xsd/cxx/tree/containers-wildcard.hxx>

#include "xcon-conference-info.h"

#include "conference-info.h"

namespace LinphonePrivate {
namespace Xsd {
namespace XconCcmp {
class CcmpRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// ccmpRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType CcmpRequestType1;
	typedef ::xsd::cxx::tree::traits<CcmpRequestType1, char> CcmpRequestTraits;

	const CcmpRequestType1 &getCcmpRequest() const;

	CcmpRequestType1 &getCcmpRequest();

	void setCcmpRequest(const CcmpRequestType1 &x);

	void setCcmpRequest(::std::unique_ptr<CcmpRequestType1> p);

	::std::unique_ptr<CcmpRequestType1> setDetachCcmpRequest();

	// Constructors.
	//
	CcmpRequestType(const CcmpRequestType1 &);

	CcmpRequestType(::std::unique_ptr<CcmpRequestType1>);

	CcmpRequestType(const ::xercesc::DOMElement &e,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpRequestType(const CcmpRequestType &x,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpRequestType &operator=(const CcmpRequestType &x);

	virtual ~CcmpRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<CcmpRequestType1> ccmpRequest_;
};

class CcmpRequestMessageType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// subject
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SubjectType SubjectType;
	typedef ::xsd::cxx::tree::optional<SubjectType> SubjectOptional;
	typedef ::xsd::cxx::tree::traits<SubjectType, char> SubjectTraits;

	const SubjectOptional &getSubject() const;

	SubjectOptional &getSubject();

	void setSubject(const SubjectType &x);

	void setSubject(const SubjectOptional &x);

	void setSubject(::std::unique_ptr<SubjectType> p);

	// confUserID
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ConfUserIDType;
	typedef ::xsd::cxx::tree::optional<ConfUserIDType> ConfUserIDOptional;
	typedef ::xsd::cxx::tree::traits<ConfUserIDType, char> ConfUserIDTraits;

	const ConfUserIDOptional &getConfUserID() const;

	ConfUserIDOptional &getConfUserID();

	void setConfUserID(const ConfUserIDType &x);

	void setConfUserID(const ConfUserIDOptional &x);

	void setConfUserID(::std::unique_ptr<ConfUserIDType> p);

	// confObjID
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ConfObjIDType;
	typedef ::xsd::cxx::tree::optional<ConfObjIDType> ConfObjIDOptional;
	typedef ::xsd::cxx::tree::traits<ConfObjIDType, char> ConfObjIDTraits;

	const ConfObjIDOptional &getConfObjID() const;

	ConfObjIDOptional &getConfObjID();

	void setConfObjID(const ConfObjIDType &x);

	void setConfObjID(const ConfObjIDOptional &x);

	void setConfObjID(::std::unique_ptr<ConfObjIDType> p);

	// operation
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::OperationType OperationType;
	typedef ::xsd::cxx::tree::optional<OperationType> OperationOptional;
	typedef ::xsd::cxx::tree::traits<OperationType, char> OperationTraits;

	const OperationOptional &getOperation() const;

	OperationOptional &getOperation();

	void setOperation(const OperationType &x);

	void setOperation(const OperationOptional &x);

	void setOperation(::std::unique_ptr<OperationType> p);

	// conference-password
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ConferencePasswordType;
	typedef ::xsd::cxx::tree::optional<ConferencePasswordType> ConferencePasswordOptional;
	typedef ::xsd::cxx::tree::traits<ConferencePasswordType, char> ConferencePasswordTraits;

	const ConferencePasswordOptional &getConferencePassword() const;

	ConferencePasswordOptional &getConferencePassword();

	void setConferencePassword(const ConferencePasswordType &x);

	void setConferencePassword(const ConferencePasswordOptional &x);

	void setConferencePassword(::std::unique_ptr<ConferencePasswordType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	CcmpRequestMessageType();

	CcmpRequestMessageType(const ::xercesc::DOMElement &e,
	                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpRequestMessageType(const CcmpRequestMessageType &x,
	                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpRequestMessageType &operator=(const CcmpRequestMessageType &x);

	virtual ~CcmpRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	SubjectOptional subject_;
	ConfUserIDOptional confUserID_;
	ConfObjIDOptional confObjID_;
	OperationOptional operation_;
	ConferencePasswordOptional conference_password_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// ccmpResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType CcmpResponseType1;
	typedef ::xsd::cxx::tree::traits<CcmpResponseType1, char> CcmpResponseTraits;

	const CcmpResponseType1 &getCcmpResponse() const;

	CcmpResponseType1 &getCcmpResponse();

	void setCcmpResponse(const CcmpResponseType1 &x);

	void setCcmpResponse(::std::unique_ptr<CcmpResponseType1> p);

	::std::unique_ptr<CcmpResponseType1> setDetachCcmpResponse();

	// Constructors.
	//
	CcmpResponseType(const CcmpResponseType1 &);

	CcmpResponseType(::std::unique_ptr<CcmpResponseType1>);

	CcmpResponseType(const ::xercesc::DOMElement &e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpResponseType(const CcmpResponseType &x,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpResponseType &operator=(const CcmpResponseType &x);

	virtual ~CcmpResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<CcmpResponseType1> ccmpResponse_;
};

class CcmpResponseMessageType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// confUserID
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ConfUserIDType;
	typedef ::xsd::cxx::tree::traits<ConfUserIDType, char> ConfUserIDTraits;

	const ConfUserIDType &getConfUserID() const;

	ConfUserIDType &getConfUserID();

	void setConfUserID(const ConfUserIDType &x);

	void setConfUserID(::std::unique_ptr<ConfUserIDType> p);

	::std::unique_ptr<ConfUserIDType> setDetachConfUserID();

	// confObjID
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ConfObjIDType;
	typedef ::xsd::cxx::tree::optional<ConfObjIDType> ConfObjIDOptional;
	typedef ::xsd::cxx::tree::traits<ConfObjIDType, char> ConfObjIDTraits;

	const ConfObjIDOptional &getConfObjID() const;

	ConfObjIDOptional &getConfObjID();

	void setConfObjID(const ConfObjIDType &x);

	void setConfObjID(const ConfObjIDOptional &x);

	void setConfObjID(::std::unique_ptr<ConfObjIDType> p);

	// operation
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::OperationType OperationType;
	typedef ::xsd::cxx::tree::optional<OperationType> OperationOptional;
	typedef ::xsd::cxx::tree::traits<OperationType, char> OperationTraits;

	const OperationOptional &getOperation() const;

	OperationOptional &getOperation();

	void setOperation(const OperationType &x);

	void setOperation(const OperationOptional &x);

	void setOperation(::std::unique_ptr<OperationType> p);

	// response-code
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ResponseCodeType ResponseCodeType;
	typedef ::xsd::cxx::tree::traits<ResponseCodeType, char> ResponseCodeTraits;

	const ResponseCodeType &getResponseCode() const;

	ResponseCodeType &getResponseCode();

	void setResponseCode(const ResponseCodeType &x);

	void setResponseCode(::std::unique_ptr<ResponseCodeType> p);

	::std::unique_ptr<ResponseCodeType> setDetachResponse_code();

	// response-string
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ResponseStringType;
	typedef ::xsd::cxx::tree::optional<ResponseStringType> ResponseStringOptional;
	typedef ::xsd::cxx::tree::traits<ResponseStringType, char> ResponseStringTraits;

	const ResponseStringOptional &getResponseString() const;

	ResponseStringOptional &getResponseString();

	void setResponseString(const ResponseStringType &x);

	void setResponseString(const ResponseStringOptional &x);

	void setResponseString(::std::unique_ptr<ResponseStringType> p);

	// version
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::PositiveInteger VersionType;
	typedef ::xsd::cxx::tree::optional<VersionType> VersionOptional;
	typedef ::xsd::cxx::tree::traits<VersionType, char> VersionTraits;

	const VersionOptional &getVersion() const;

	VersionOptional &getVersion();

	void setVersion(const VersionType &x);

	void setVersion(const VersionOptional &x);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	CcmpResponseMessageType(const ConfUserIDType &, const ResponseCodeType &);

	CcmpResponseMessageType(::std::unique_ptr<ConfUserIDType>, ::std::unique_ptr<ResponseCodeType>);

	CcmpResponseMessageType(const ::xercesc::DOMElement &e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpResponseMessageType(const CcmpResponseMessageType &x,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpResponseMessageType &operator=(const CcmpResponseMessageType &x);

	virtual ~CcmpResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<ConfUserIDType> confUserID_;
	ConfObjIDOptional confObjID_;
	OperationOptional operation_;
	::xsd::cxx::tree::one<ResponseCodeType> response_code_;
	ResponseStringOptional response_string_;
	VersionOptional version_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpBlueprintsRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// blueprintsRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType BlueprintsRequestType;
	typedef ::xsd::cxx::tree::traits<BlueprintsRequestType, char> BlueprintsRequestTraits;

	const BlueprintsRequestType &getBlueprintsRequest() const;

	BlueprintsRequestType &getBlueprintsRequest();

	void setBlueprintsRequest(const BlueprintsRequestType &x);

	void setBlueprintsRequest(::std::unique_ptr<BlueprintsRequestType> p);

	::std::unique_ptr<BlueprintsRequestType> setDetachBlueprintsRequest();

	// Constructors.
	//
	CcmpBlueprintsRequestMessageType(const BlueprintsRequestType &);

	CcmpBlueprintsRequestMessageType(::std::unique_ptr<BlueprintsRequestType>);

	CcmpBlueprintsRequestMessageType(const ::xercesc::DOMElement &e,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpBlueprintsRequestMessageType(const CcmpBlueprintsRequestMessageType &x,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpBlueprintsRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpBlueprintsRequestMessageType &operator=(const CcmpBlueprintsRequestMessageType &x);

	virtual ~CcmpBlueprintsRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<BlueprintsRequestType> blueprintsRequest_;
};

class BlueprintsRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// xpathFilter
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String XpathFilterType;
	typedef ::xsd::cxx::tree::optional<XpathFilterType> XpathFilterOptional;
	typedef ::xsd::cxx::tree::traits<XpathFilterType, char> XpathFilterTraits;

	const XpathFilterOptional &getXpathFilter() const;

	XpathFilterOptional &getXpathFilter();

	void setXpathFilter(const XpathFilterType &x);

	void setXpathFilter(const XpathFilterOptional &x);

	void setXpathFilter(::std::unique_ptr<XpathFilterType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	BlueprintsRequestType();

	BlueprintsRequestType(const ::xercesc::DOMElement &e,
	                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	BlueprintsRequestType(const BlueprintsRequestType &x,
	                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual BlueprintsRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	BlueprintsRequestType &operator=(const BlueprintsRequestType &x);

	virtual ~BlueprintsRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	XpathFilterOptional xpathFilter_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpBlueprintRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// blueprintRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType BlueprintRequestType;
	typedef ::xsd::cxx::tree::traits<BlueprintRequestType, char> BlueprintRequestTraits;

	const BlueprintRequestType &getBlueprintRequest() const;

	BlueprintRequestType &getBlueprintRequest();

	void setBlueprintRequest(const BlueprintRequestType &x);

	void setBlueprintRequest(::std::unique_ptr<BlueprintRequestType> p);

	::std::unique_ptr<BlueprintRequestType> setDetachBlueprintRequest();

	// Constructors.
	//
	CcmpBlueprintRequestMessageType(const BlueprintRequestType &);

	CcmpBlueprintRequestMessageType(::std::unique_ptr<BlueprintRequestType>);

	CcmpBlueprintRequestMessageType(const ::xercesc::DOMElement &e,
	                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpBlueprintRequestMessageType(const CcmpBlueprintRequestMessageType &x,
	                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpBlueprintRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpBlueprintRequestMessageType &operator=(const CcmpBlueprintRequestMessageType &x);

	virtual ~CcmpBlueprintRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<BlueprintRequestType> blueprintRequest_;
};

class BlueprintRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// blueprintInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType BlueprintInfoType;
	typedef ::xsd::cxx::tree::optional<BlueprintInfoType> BlueprintInfoOptional;
	typedef ::xsd::cxx::tree::traits<BlueprintInfoType, char> BlueprintInfoTraits;

	const BlueprintInfoOptional &getBlueprintInfo() const;

	BlueprintInfoOptional &getBlueprintInfo();

	void setBlueprintInfo(const BlueprintInfoType &x);

	void setBlueprintInfo(const BlueprintInfoOptional &x);

	void setBlueprintInfo(::std::unique_ptr<BlueprintInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	BlueprintRequestType();

	BlueprintRequestType(const ::xercesc::DOMElement &e,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	BlueprintRequestType(const BlueprintRequestType &x,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual BlueprintRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	BlueprintRequestType &operator=(const BlueprintRequestType &x);

	virtual ~BlueprintRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	BlueprintInfoOptional blueprintInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpConfsRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// confsRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType ConfsRequestType;
	typedef ::xsd::cxx::tree::traits<ConfsRequestType, char> ConfsRequestTraits;

	const ConfsRequestType &getConfsRequest() const;

	ConfsRequestType &getConfsRequest();

	void setConfsRequest(const ConfsRequestType &x);

	void setConfsRequest(::std::unique_ptr<ConfsRequestType> p);

	::std::unique_ptr<ConfsRequestType> setDetachConfsRequest();

	// Constructors.
	//
	CcmpConfsRequestMessageType(const ConfsRequestType &);

	CcmpConfsRequestMessageType(::std::unique_ptr<ConfsRequestType>);

	CcmpConfsRequestMessageType(const ::xercesc::DOMElement &e,
	                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpConfsRequestMessageType(const CcmpConfsRequestMessageType &x,
	                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpConfsRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpConfsRequestMessageType &operator=(const CcmpConfsRequestMessageType &x);

	virtual ~CcmpConfsRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<ConfsRequestType> confsRequest_;
};

class ConfsRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// xpathFilter
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String XpathFilterType;
	typedef ::xsd::cxx::tree::optional<XpathFilterType> XpathFilterOptional;
	typedef ::xsd::cxx::tree::traits<XpathFilterType, char> XpathFilterTraits;

	const XpathFilterOptional &getXpathFilter() const;

	XpathFilterOptional &getXpathFilter();

	void setXpathFilter(const XpathFilterType &x);

	void setXpathFilter(const XpathFilterOptional &x);

	void setXpathFilter(::std::unique_ptr<XpathFilterType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	ConfsRequestType();

	ConfsRequestType(const ::xercesc::DOMElement &e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ConfsRequestType(const ConfsRequestType &x,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ConfsRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ConfsRequestType &operator=(const ConfsRequestType &x);

	virtual ~ConfsRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	XpathFilterOptional xpathFilter_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpConfRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// confRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType ConfRequestType;
	typedef ::xsd::cxx::tree::traits<ConfRequestType, char> ConfRequestTraits;

	const ConfRequestType &getConfRequest() const;

	ConfRequestType &getConfRequest();

	void setConfRequest(const ConfRequestType &x);

	void setConfRequest(::std::unique_ptr<ConfRequestType> p);

	::std::unique_ptr<ConfRequestType> setDetachConfRequest();

	// Constructors.
	//
	CcmpConfRequestMessageType(const ConfRequestType &);

	CcmpConfRequestMessageType(::std::unique_ptr<ConfRequestType>);

	CcmpConfRequestMessageType(const ::xercesc::DOMElement &e,
	                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpConfRequestMessageType(const CcmpConfRequestMessageType &x,
	                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpConfRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpConfRequestMessageType &operator=(const CcmpConfRequestMessageType &x);

	virtual ~CcmpConfRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<ConfRequestType> confRequest_;
};

class ConfRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// confInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType ConfInfoType;
	typedef ::xsd::cxx::tree::optional<ConfInfoType> ConfInfoOptional;
	typedef ::xsd::cxx::tree::traits<ConfInfoType, char> ConfInfoTraits;

	const ConfInfoOptional &getConfInfo() const;

	ConfInfoOptional &getConfInfo();

	void setConfInfo(const ConfInfoType &x);

	void setConfInfo(const ConfInfoOptional &x);

	void setConfInfo(::std::unique_ptr<ConfInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	ConfRequestType();

	ConfRequestType(const ::xercesc::DOMElement &e,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ConfRequestType(const ConfRequestType &x,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ConfRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ConfRequestType &operator=(const ConfRequestType &x);

	virtual ~ConfRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	ConfInfoOptional confInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpUsersRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// usersRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType UsersRequestType;
	typedef ::xsd::cxx::tree::traits<UsersRequestType, char> UsersRequestTraits;

	const UsersRequestType &getUsersRequest() const;

	UsersRequestType &getUsersRequest();

	void setUsersRequest(const UsersRequestType &x);

	void setUsersRequest(::std::unique_ptr<UsersRequestType> p);

	::std::unique_ptr<UsersRequestType> setDetachUsersRequest();

	// Constructors.
	//
	CcmpUsersRequestMessageType(const UsersRequestType &);

	CcmpUsersRequestMessageType(::std::unique_ptr<UsersRequestType>);

	CcmpUsersRequestMessageType(const ::xercesc::DOMElement &e,
	                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpUsersRequestMessageType(const CcmpUsersRequestMessageType &x,
	                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpUsersRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpUsersRequestMessageType &operator=(const CcmpUsersRequestMessageType &x);

	virtual ~CcmpUsersRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<UsersRequestType> usersRequest_;
};

class UsersRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// usersInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::UsersType UsersInfoType;
	typedef ::xsd::cxx::tree::optional<UsersInfoType> UsersInfoOptional;
	typedef ::xsd::cxx::tree::traits<UsersInfoType, char> UsersInfoTraits;

	const UsersInfoOptional &getUsersInfo() const;

	UsersInfoOptional &getUsersInfo();

	void setUsersInfo(const UsersInfoType &x);

	void setUsersInfo(const UsersInfoOptional &x);

	void setUsersInfo(::std::unique_ptr<UsersInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	UsersRequestType();

	UsersRequestType(const ::xercesc::DOMElement &e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	UsersRequestType(const UsersRequestType &x,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual UsersRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	UsersRequestType &operator=(const UsersRequestType &x);

	virtual ~UsersRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	UsersInfoOptional usersInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpUserRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// userRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::UserRequestType UserRequestType;
	typedef ::xsd::cxx::tree::traits<UserRequestType, char> UserRequestTraits;

	const UserRequestType &getUserRequest() const;

	UserRequestType &getUserRequest();

	void setUserRequest(const UserRequestType &x);

	void setUserRequest(::std::unique_ptr<UserRequestType> p);

	::std::unique_ptr<UserRequestType> setDetachUserRequest();

	// Constructors.
	//
	CcmpUserRequestMessageType(const UserRequestType &);

	CcmpUserRequestMessageType(::std::unique_ptr<UserRequestType>);

	CcmpUserRequestMessageType(const ::xercesc::DOMElement &e,
	                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpUserRequestMessageType(const CcmpUserRequestMessageType &x,
	                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpUserRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpUserRequestMessageType &operator=(const CcmpUserRequestMessageType &x);

	virtual ~CcmpUserRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<UserRequestType> userRequest_;
};

class UserRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// userInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::UserType UserInfoType;
	typedef ::xsd::cxx::tree::optional<UserInfoType> UserInfoOptional;
	typedef ::xsd::cxx::tree::traits<UserInfoType, char> UserInfoTraits;

	const UserInfoOptional &getUserInfo() const;

	UserInfoOptional &getUserInfo();

	void setUserInfo(const UserInfoType &x);

	void setUserInfo(const UserInfoOptional &x);

	void setUserInfo(::std::unique_ptr<UserInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	UserRequestType();

	UserRequestType(const ::xercesc::DOMElement &e,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	UserRequestType(const UserRequestType &x,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual UserRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	UserRequestType &operator=(const UserRequestType &x);

	virtual ~UserRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	UserInfoOptional userInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpSidebarsByValRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// sidebarsByValRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType SidebarsByValRequestType;
	typedef ::xsd::cxx::tree::traits<SidebarsByValRequestType, char> SidebarsByValRequestTraits;

	const SidebarsByValRequestType &getSidebarsByValRequest() const;

	SidebarsByValRequestType &getSidebarsByValRequest();

	void setSidebarsByValRequest(const SidebarsByValRequestType &x);

	void setSidebarsByValRequest(::std::unique_ptr<SidebarsByValRequestType> p);

	::std::unique_ptr<SidebarsByValRequestType> setDetachSidebarsByValRequest();

	// Constructors.
	//
	CcmpSidebarsByValRequestMessageType(const SidebarsByValRequestType &);

	CcmpSidebarsByValRequestMessageType(::std::unique_ptr<SidebarsByValRequestType>);

	CcmpSidebarsByValRequestMessageType(const ::xercesc::DOMElement &e,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpSidebarsByValRequestMessageType(const CcmpSidebarsByValRequestMessageType &x,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpSidebarsByValRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpSidebarsByValRequestMessageType &operator=(const CcmpSidebarsByValRequestMessageType &x);

	virtual ~CcmpSidebarsByValRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SidebarsByValRequestType> sidebarsByValRequest_;
};

class SidebarsByValRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// xpathFilter
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String XpathFilterType;
	typedef ::xsd::cxx::tree::optional<XpathFilterType> XpathFilterOptional;
	typedef ::xsd::cxx::tree::traits<XpathFilterType, char> XpathFilterTraits;

	const XpathFilterOptional &getXpathFilter() const;

	XpathFilterOptional &getXpathFilter();

	void setXpathFilter(const XpathFilterType &x);

	void setXpathFilter(const XpathFilterOptional &x);

	void setXpathFilter(::std::unique_ptr<XpathFilterType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SidebarsByValRequestType();

	SidebarsByValRequestType(const ::xercesc::DOMElement &e,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SidebarsByValRequestType(const SidebarsByValRequestType &x,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SidebarsByValRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SidebarsByValRequestType &operator=(const SidebarsByValRequestType &x);

	virtual ~SidebarsByValRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	XpathFilterOptional xpathFilter_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpSidebarsByRefRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// sidebarsByRefRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType SidebarsByRefRequestType;
	typedef ::xsd::cxx::tree::traits<SidebarsByRefRequestType, char> SidebarsByRefRequestTraits;

	const SidebarsByRefRequestType &getSidebarsByRefRequest() const;

	SidebarsByRefRequestType &getSidebarsByRefRequest();

	void setSidebarsByRefRequest(const SidebarsByRefRequestType &x);

	void setSidebarsByRefRequest(::std::unique_ptr<SidebarsByRefRequestType> p);

	::std::unique_ptr<SidebarsByRefRequestType> setDetachSidebarsByRefRequest();

	// Constructors.
	//
	CcmpSidebarsByRefRequestMessageType(const SidebarsByRefRequestType &);

	CcmpSidebarsByRefRequestMessageType(::std::unique_ptr<SidebarsByRefRequestType>);

	CcmpSidebarsByRefRequestMessageType(const ::xercesc::DOMElement &e,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpSidebarsByRefRequestMessageType(const CcmpSidebarsByRefRequestMessageType &x,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpSidebarsByRefRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpSidebarsByRefRequestMessageType &operator=(const CcmpSidebarsByRefRequestMessageType &x);

	virtual ~CcmpSidebarsByRefRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SidebarsByRefRequestType> sidebarsByRefRequest_;
};

class SidebarsByRefRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// xpathFilter
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String XpathFilterType;
	typedef ::xsd::cxx::tree::optional<XpathFilterType> XpathFilterOptional;
	typedef ::xsd::cxx::tree::traits<XpathFilterType, char> XpathFilterTraits;

	const XpathFilterOptional &getXpathFilter() const;

	XpathFilterOptional &getXpathFilter();

	void setXpathFilter(const XpathFilterType &x);

	void setXpathFilter(const XpathFilterOptional &x);

	void setXpathFilter(::std::unique_ptr<XpathFilterType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SidebarsByRefRequestType();

	SidebarsByRefRequestType(const ::xercesc::DOMElement &e,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SidebarsByRefRequestType(const SidebarsByRefRequestType &x,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SidebarsByRefRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SidebarsByRefRequestType &operator=(const SidebarsByRefRequestType &x);

	virtual ~SidebarsByRefRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	XpathFilterOptional xpathFilter_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpSidebarByValRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// sidebarByValRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType SidebarByValRequestType;
	typedef ::xsd::cxx::tree::traits<SidebarByValRequestType, char> SidebarByValRequestTraits;

	const SidebarByValRequestType &getSidebarByValRequest() const;

	SidebarByValRequestType &getSidebarByValRequest();

	void setSidebarByValRequest(const SidebarByValRequestType &x);

	void setSidebarByValRequest(::std::unique_ptr<SidebarByValRequestType> p);

	::std::unique_ptr<SidebarByValRequestType> setDetachSidebarByValRequest();

	// Constructors.
	//
	CcmpSidebarByValRequestMessageType(const SidebarByValRequestType &);

	CcmpSidebarByValRequestMessageType(::std::unique_ptr<SidebarByValRequestType>);

	CcmpSidebarByValRequestMessageType(const ::xercesc::DOMElement &e,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpSidebarByValRequestMessageType(const CcmpSidebarByValRequestMessageType &x,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpSidebarByValRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpSidebarByValRequestMessageType &operator=(const CcmpSidebarByValRequestMessageType &x);

	virtual ~CcmpSidebarByValRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SidebarByValRequestType> sidebarByValRequest_;
};

class SidebarByValRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// sidebarByValInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType SidebarByValInfoType;
	typedef ::xsd::cxx::tree::optional<SidebarByValInfoType> SidebarByValInfoOptional;
	typedef ::xsd::cxx::tree::traits<SidebarByValInfoType, char> SidebarByValInfoTraits;

	const SidebarByValInfoOptional &getSidebarByValInfo() const;

	SidebarByValInfoOptional &getSidebarByValInfo();

	void setSidebarByValInfo(const SidebarByValInfoType &x);

	void setSidebarByValInfo(const SidebarByValInfoOptional &x);

	void setSidebarByValInfo(::std::unique_ptr<SidebarByValInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SidebarByValRequestType();

	SidebarByValRequestType(const ::xercesc::DOMElement &e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SidebarByValRequestType(const SidebarByValRequestType &x,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SidebarByValRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SidebarByValRequestType &operator=(const SidebarByValRequestType &x);

	virtual ~SidebarByValRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	SidebarByValInfoOptional sidebarByValInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpSidebarByRefRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// sidebarByRefRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType SidebarByRefRequestType;
	typedef ::xsd::cxx::tree::traits<SidebarByRefRequestType, char> SidebarByRefRequestTraits;

	const SidebarByRefRequestType &getSidebarByRefRequest() const;

	SidebarByRefRequestType &getSidebarByRefRequest();

	void setSidebarByRefRequest(const SidebarByRefRequestType &x);

	void setSidebarByRefRequest(::std::unique_ptr<SidebarByRefRequestType> p);

	::std::unique_ptr<SidebarByRefRequestType> setDetachSidebarByRefRequest();

	// Constructors.
	//
	CcmpSidebarByRefRequestMessageType(const SidebarByRefRequestType &);

	CcmpSidebarByRefRequestMessageType(::std::unique_ptr<SidebarByRefRequestType>);

	CcmpSidebarByRefRequestMessageType(const ::xercesc::DOMElement &e,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpSidebarByRefRequestMessageType(const CcmpSidebarByRefRequestMessageType &x,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpSidebarByRefRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpSidebarByRefRequestMessageType &operator=(const CcmpSidebarByRefRequestMessageType &x);

	virtual ~CcmpSidebarByRefRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SidebarByRefRequestType> sidebarByRefRequest_;
};

class SidebarByRefRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// sidebarByRefInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType SidebarByRefInfoType;
	typedef ::xsd::cxx::tree::optional<SidebarByRefInfoType> SidebarByRefInfoOptional;
	typedef ::xsd::cxx::tree::traits<SidebarByRefInfoType, char> SidebarByRefInfoTraits;

	const SidebarByRefInfoOptional &getSidebarByRefInfo() const;

	SidebarByRefInfoOptional &getSidebarByRefInfo();

	void setSidebarByRefInfo(const SidebarByRefInfoType &x);

	void setSidebarByRefInfo(const SidebarByRefInfoOptional &x);

	void setSidebarByRefInfo(::std::unique_ptr<SidebarByRefInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SidebarByRefRequestType();

	SidebarByRefRequestType(const ::xercesc::DOMElement &e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SidebarByRefRequestType(const SidebarByRefRequestType &x,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SidebarByRefRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SidebarByRefRequestType &operator=(const SidebarByRefRequestType &x);

	virtual ~SidebarByRefRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	SidebarByRefInfoOptional sidebarByRefInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpExtendedRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// extendedRequest
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType ExtendedRequestType;
	typedef ::xsd::cxx::tree::traits<ExtendedRequestType, char> ExtendedRequestTraits;

	const ExtendedRequestType &getExtendedRequest() const;

	ExtendedRequestType &getExtendedRequest();

	void setExtendedRequest(const ExtendedRequestType &x);

	void setExtendedRequest(::std::unique_ptr<ExtendedRequestType> p);

	::std::unique_ptr<ExtendedRequestType> setDetachExtendedRequest();

	// Constructors.
	//
	CcmpExtendedRequestMessageType(const ExtendedRequestType &);

	CcmpExtendedRequestMessageType(::std::unique_ptr<ExtendedRequestType>);

	CcmpExtendedRequestMessageType(const ::xercesc::DOMElement &e,
	                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpExtendedRequestMessageType(const CcmpExtendedRequestMessageType &x,
	                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpExtendedRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpExtendedRequestMessageType &operator=(const CcmpExtendedRequestMessageType &x);

	virtual ~CcmpExtendedRequestMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<ExtendedRequestType> extendedRequest_;
};

class ExtendedRequestType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// extensionName
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ExtensionNameType;
	typedef ::xsd::cxx::tree::traits<ExtensionNameType, char> ExtensionNameTraits;

	const ExtensionNameType &getExtensionName() const;

	ExtensionNameType &getExtensionName();

	void setExtensionName(const ExtensionNameType &x);

	void setExtensionName(::std::unique_ptr<ExtensionNameType> p);

	::std::unique_ptr<ExtensionNameType> setDetachExtensionName();

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	ExtendedRequestType(const ExtensionNameType &);

	ExtendedRequestType(::std::unique_ptr<ExtensionNameType>);

	ExtendedRequestType(const ::xercesc::DOMElement &e,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ExtendedRequestType(const ExtendedRequestType &x,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ExtendedRequestType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ExtendedRequestType &operator=(const ExtendedRequestType &x);

	virtual ~ExtendedRequestType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<ExtensionNameType> extensionName_;
	AnySequence any_;
};

class CcmpOptionsRequestMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestMessageType {
public:
	// Constructors.
	//
	CcmpOptionsRequestMessageType();

	CcmpOptionsRequestMessageType(const ::xercesc::DOMElement &e,
	                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpOptionsRequestMessageType(const CcmpOptionsRequestMessageType &x,
	                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpOptionsRequestMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	CcmpOptionsRequestMessageType &operator=(const CcmpOptionsRequestMessageType &) = default;
#endif

	virtual ~CcmpOptionsRequestMessageType();
};

class CcmpBlueprintsResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// blueprintsResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType BlueprintsResponseType;
	typedef ::xsd::cxx::tree::traits<BlueprintsResponseType, char> BlueprintsResponseTraits;

	const BlueprintsResponseType &getBlueprintsResponse() const;

	BlueprintsResponseType &getBlueprintsResponse();

	void setBlueprintsResponse(const BlueprintsResponseType &x);

	void setBlueprintsResponse(::std::unique_ptr<BlueprintsResponseType> p);

	::std::unique_ptr<BlueprintsResponseType> setDetachBlueprintsResponse();

	// Constructors.
	//
	CcmpBlueprintsResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, const BlueprintsResponseType &);

	CcmpBlueprintsResponseMessageType(const ConfUserIDType &,
	                                  const ResponseCodeType &,
	                                  ::std::unique_ptr<BlueprintsResponseType>);

	CcmpBlueprintsResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                                  ::std::unique_ptr<ResponseCodeType>,
	                                  ::std::unique_ptr<BlueprintsResponseType>);

	CcmpBlueprintsResponseMessageType(const ::xercesc::DOMElement &e,
	                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpBlueprintsResponseMessageType(const CcmpBlueprintsResponseMessageType &x,
	                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpBlueprintsResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpBlueprintsResponseMessageType &operator=(const CcmpBlueprintsResponseMessageType &x);

	virtual ~CcmpBlueprintsResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<BlueprintsResponseType> blueprintsResponse_;
};

class BlueprintsResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// blueprintsInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::UrisType BlueprintsInfoType;
	typedef ::xsd::cxx::tree::optional<BlueprintsInfoType> BlueprintsInfoOptional;
	typedef ::xsd::cxx::tree::traits<BlueprintsInfoType, char> BlueprintsInfoTraits;

	const BlueprintsInfoOptional &getBlueprintsInfo() const;

	BlueprintsInfoOptional &getBlueprintsInfo();

	void setBlueprintsInfo(const BlueprintsInfoType &x);

	void setBlueprintsInfo(const BlueprintsInfoOptional &x);

	void setBlueprintsInfo(::std::unique_ptr<BlueprintsInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	BlueprintsResponseType();

	BlueprintsResponseType(const ::xercesc::DOMElement &e,
	                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	BlueprintsResponseType(const BlueprintsResponseType &x,
	                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual BlueprintsResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	BlueprintsResponseType &operator=(const BlueprintsResponseType &x);

	virtual ~BlueprintsResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	BlueprintsInfoOptional blueprintsInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpBlueprintResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// blueprintResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType BlueprintResponseType;
	typedef ::xsd::cxx::tree::traits<BlueprintResponseType, char> BlueprintResponseTraits;

	const BlueprintResponseType &getBlueprintResponse() const;

	BlueprintResponseType &getBlueprintResponse();

	void setBlueprintResponse(const BlueprintResponseType &x);

	void setBlueprintResponse(::std::unique_ptr<BlueprintResponseType> p);

	::std::unique_ptr<BlueprintResponseType> setDetachBlueprintResponse();

	// Constructors.
	//
	CcmpBlueprintResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, const BlueprintResponseType &);

	CcmpBlueprintResponseMessageType(const ConfUserIDType &,
	                                 const ResponseCodeType &,
	                                 ::std::unique_ptr<BlueprintResponseType>);

	CcmpBlueprintResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                                 ::std::unique_ptr<ResponseCodeType>,
	                                 ::std::unique_ptr<BlueprintResponseType>);

	CcmpBlueprintResponseMessageType(const ::xercesc::DOMElement &e,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpBlueprintResponseMessageType(const CcmpBlueprintResponseMessageType &x,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpBlueprintResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpBlueprintResponseMessageType &operator=(const CcmpBlueprintResponseMessageType &x);

	virtual ~CcmpBlueprintResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<BlueprintResponseType> blueprintResponse_;
};

class BlueprintResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// blueprintInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType BlueprintInfoType;
	typedef ::xsd::cxx::tree::optional<BlueprintInfoType> BlueprintInfoOptional;
	typedef ::xsd::cxx::tree::traits<BlueprintInfoType, char> BlueprintInfoTraits;

	const BlueprintInfoOptional &getBlueprintInfo() const;

	BlueprintInfoOptional &getBlueprintInfo();

	void setBlueprintInfo(const BlueprintInfoType &x);

	void setBlueprintInfo(const BlueprintInfoOptional &x);

	void setBlueprintInfo(::std::unique_ptr<BlueprintInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	BlueprintResponseType();

	BlueprintResponseType(const ::xercesc::DOMElement &e,
	                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	BlueprintResponseType(const BlueprintResponseType &x,
	                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual BlueprintResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	BlueprintResponseType &operator=(const BlueprintResponseType &x);

	virtual ~BlueprintResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	BlueprintInfoOptional blueprintInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpConfsResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// confsResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType ConfsResponseType;
	typedef ::xsd::cxx::tree::traits<ConfsResponseType, char> ConfsResponseTraits;

	const ConfsResponseType &getConfsResponse() const;

	ConfsResponseType &getConfsResponse();

	void setConfsResponse(const ConfsResponseType &x);

	void setConfsResponse(::std::unique_ptr<ConfsResponseType> p);

	::std::unique_ptr<ConfsResponseType> setDetachConfsResponse();

	// Constructors.
	//
	CcmpConfsResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, const ConfsResponseType &);

	CcmpConfsResponseMessageType(const ConfUserIDType &,
	                             const ResponseCodeType &,
	                             ::std::unique_ptr<ConfsResponseType>);

	CcmpConfsResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                             ::std::unique_ptr<ResponseCodeType>,
	                             ::std::unique_ptr<ConfsResponseType>);

	CcmpConfsResponseMessageType(const ::xercesc::DOMElement &e,
	                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpConfsResponseMessageType(const CcmpConfsResponseMessageType &x,
	                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpConfsResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpConfsResponseMessageType &operator=(const CcmpConfsResponseMessageType &x);

	virtual ~CcmpConfsResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<ConfsResponseType> confsResponse_;
};

class ConfsResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// confsInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::UrisType ConfsInfoType;
	typedef ::xsd::cxx::tree::optional<ConfsInfoType> ConfsInfoOptional;
	typedef ::xsd::cxx::tree::traits<ConfsInfoType, char> ConfsInfoTraits;

	const ConfsInfoOptional &getConfsInfo() const;

	ConfsInfoOptional &getConfsInfo();

	void setConfsInfo(const ConfsInfoType &x);

	void setConfsInfo(const ConfsInfoOptional &x);

	void setConfsInfo(::std::unique_ptr<ConfsInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	ConfsResponseType();

	ConfsResponseType(const ::xercesc::DOMElement &e,
	                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ConfsResponseType(const ConfsResponseType &x,
	                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ConfsResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ConfsResponseType &operator=(const ConfsResponseType &x);

	virtual ~ConfsResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	ConfsInfoOptional confsInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpConfResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// confResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType ConfResponseType;
	typedef ::xsd::cxx::tree::traits<ConfResponseType, char> ConfResponseTraits;

	const ConfResponseType &getConfResponse() const;

	ConfResponseType &getConfResponse();

	void setConfResponse(const ConfResponseType &x);

	void setConfResponse(::std::unique_ptr<ConfResponseType> p);

	::std::unique_ptr<ConfResponseType> setDetachConfResponse();

	// Constructors.
	//
	CcmpConfResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, const ConfResponseType &);

	CcmpConfResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, ::std::unique_ptr<ConfResponseType>);

	CcmpConfResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                            ::std::unique_ptr<ResponseCodeType>,
	                            ::std::unique_ptr<ConfResponseType>);

	CcmpConfResponseMessageType(const ::xercesc::DOMElement &e,
	                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpConfResponseMessageType(const CcmpConfResponseMessageType &x,
	                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpConfResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpConfResponseMessageType &operator=(const CcmpConfResponseMessageType &x);

	virtual ~CcmpConfResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<ConfResponseType> confResponse_;
};

class ConfResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// confInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType ConfInfoType;
	typedef ::xsd::cxx::tree::optional<ConfInfoType> ConfInfoOptional;
	typedef ::xsd::cxx::tree::traits<ConfInfoType, char> ConfInfoTraits;

	const ConfInfoOptional &getConfInfo() const;

	ConfInfoOptional &getConfInfo();

	void setConfInfo(const ConfInfoType &x);

	void setConfInfo(const ConfInfoOptional &x);

	void setConfInfo(::std::unique_ptr<ConfInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	ConfResponseType();

	ConfResponseType(const ::xercesc::DOMElement &e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ConfResponseType(const ConfResponseType &x,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ConfResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ConfResponseType &operator=(const ConfResponseType &x);

	virtual ~ConfResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	ConfInfoOptional confInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpUsersResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// usersResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType UsersResponseType;
	typedef ::xsd::cxx::tree::traits<UsersResponseType, char> UsersResponseTraits;

	const UsersResponseType &getUsersResponse() const;

	UsersResponseType &getUsersResponse();

	void setUsersResponse(const UsersResponseType &x);

	void setUsersResponse(::std::unique_ptr<UsersResponseType> p);

	::std::unique_ptr<UsersResponseType> setDetachUsersResponse();

	// Constructors.
	//
	CcmpUsersResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, const UsersResponseType &);

	CcmpUsersResponseMessageType(const ConfUserIDType &,
	                             const ResponseCodeType &,
	                             ::std::unique_ptr<UsersResponseType>);

	CcmpUsersResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                             ::std::unique_ptr<ResponseCodeType>,
	                             ::std::unique_ptr<UsersResponseType>);

	CcmpUsersResponseMessageType(const ::xercesc::DOMElement &e,
	                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpUsersResponseMessageType(const CcmpUsersResponseMessageType &x,
	                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpUsersResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpUsersResponseMessageType &operator=(const CcmpUsersResponseMessageType &x);

	virtual ~CcmpUsersResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<UsersResponseType> usersResponse_;
};

class UsersResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// usersInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::UsersType UsersInfoType;
	typedef ::xsd::cxx::tree::optional<UsersInfoType> UsersInfoOptional;
	typedef ::xsd::cxx::tree::traits<UsersInfoType, char> UsersInfoTraits;

	const UsersInfoOptional &getUsersInfo() const;

	UsersInfoOptional &getUsersInfo();

	void setUsersInfo(const UsersInfoType &x);

	void setUsersInfo(const UsersInfoOptional &x);

	void setUsersInfo(::std::unique_ptr<UsersInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	UsersResponseType();

	UsersResponseType(const ::xercesc::DOMElement &e,
	                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	UsersResponseType(const UsersResponseType &x,
	                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual UsersResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	UsersResponseType &operator=(const UsersResponseType &x);

	virtual ~UsersResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	UsersInfoOptional usersInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpUserResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// userResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::UserResponseType UserResponseType;
	typedef ::xsd::cxx::tree::traits<UserResponseType, char> UserResponseTraits;

	const UserResponseType &getUserResponse() const;

	UserResponseType &getUserResponse();

	void setUserResponse(const UserResponseType &x);

	void setUserResponse(::std::unique_ptr<UserResponseType> p);

	::std::unique_ptr<UserResponseType> setDetachUserResponse();

	// Constructors.
	//
	CcmpUserResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, const UserResponseType &);

	CcmpUserResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, ::std::unique_ptr<UserResponseType>);

	CcmpUserResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                            ::std::unique_ptr<ResponseCodeType>,
	                            ::std::unique_ptr<UserResponseType>);

	CcmpUserResponseMessageType(const ::xercesc::DOMElement &e,
	                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpUserResponseMessageType(const CcmpUserResponseMessageType &x,
	                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpUserResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpUserResponseMessageType &operator=(const CcmpUserResponseMessageType &x);

	virtual ~CcmpUserResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<UserResponseType> userResponse_;
};

class UserResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// userInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::UserType UserInfoType;
	typedef ::xsd::cxx::tree::optional<UserInfoType> UserInfoOptional;
	typedef ::xsd::cxx::tree::traits<UserInfoType, char> UserInfoTraits;

	const UserInfoOptional &getUserInfo() const;

	UserInfoOptional &getUserInfo();

	void setUserInfo(const UserInfoType &x);

	void setUserInfo(const UserInfoOptional &x);

	void setUserInfo(::std::unique_ptr<UserInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	UserResponseType();

	UserResponseType(const ::xercesc::DOMElement &e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	UserResponseType(const UserResponseType &x,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual UserResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	UserResponseType &operator=(const UserResponseType &x);

	virtual ~UserResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	UserInfoOptional userInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpSidebarsByValResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// sidebarsByValResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType SidebarsByValResponseType;
	typedef ::xsd::cxx::tree::traits<SidebarsByValResponseType, char> SidebarsByValResponseTraits;

	const SidebarsByValResponseType &getSidebarsByValResponse() const;

	SidebarsByValResponseType &getSidebarsByValResponse();

	void setSidebarsByValResponse(const SidebarsByValResponseType &x);

	void setSidebarsByValResponse(::std::unique_ptr<SidebarsByValResponseType> p);

	::std::unique_ptr<SidebarsByValResponseType> setDetachSidebarsByValResponse();

	// Constructors.
	//
	CcmpSidebarsByValResponseMessageType(const ConfUserIDType &,
	                                     const ResponseCodeType &,
	                                     const SidebarsByValResponseType &);

	CcmpSidebarsByValResponseMessageType(const ConfUserIDType &,
	                                     const ResponseCodeType &,
	                                     ::std::unique_ptr<SidebarsByValResponseType>);

	CcmpSidebarsByValResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                                     ::std::unique_ptr<ResponseCodeType>,
	                                     ::std::unique_ptr<SidebarsByValResponseType>);

	CcmpSidebarsByValResponseMessageType(const ::xercesc::DOMElement &e,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpSidebarsByValResponseMessageType(const CcmpSidebarsByValResponseMessageType &x,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpSidebarsByValResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpSidebarsByValResponseMessageType &operator=(const CcmpSidebarsByValResponseMessageType &x);

	virtual ~CcmpSidebarsByValResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SidebarsByValResponseType> sidebarsByValResponse_;
};

class SidebarsByValResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// sidebarsByValInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::SidebarsByValType SidebarsByValInfoType;
	typedef ::xsd::cxx::tree::optional<SidebarsByValInfoType> SidebarsByValInfoOptional;
	typedef ::xsd::cxx::tree::traits<SidebarsByValInfoType, char> SidebarsByValInfoTraits;

	const SidebarsByValInfoOptional &getSidebarsByValInfo() const;

	SidebarsByValInfoOptional &getSidebarsByValInfo();

	void setSidebarsByValInfo(const SidebarsByValInfoType &x);

	void setSidebarsByValInfo(const SidebarsByValInfoOptional &x);

	void setSidebarsByValInfo(::std::unique_ptr<SidebarsByValInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SidebarsByValResponseType();

	SidebarsByValResponseType(const ::xercesc::DOMElement &e,
	                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SidebarsByValResponseType(const SidebarsByValResponseType &x,
	                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SidebarsByValResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SidebarsByValResponseType &operator=(const SidebarsByValResponseType &x);

	virtual ~SidebarsByValResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	SidebarsByValInfoOptional sidebarsByValInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpSidebarsByRefResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// sidebarsByRefResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType SidebarsByRefResponseType;
	typedef ::xsd::cxx::tree::traits<SidebarsByRefResponseType, char> SidebarsByRefResponseTraits;

	const SidebarsByRefResponseType &getSidebarsByRefResponse() const;

	SidebarsByRefResponseType &getSidebarsByRefResponse();

	void setSidebarsByRefResponse(const SidebarsByRefResponseType &x);

	void setSidebarsByRefResponse(::std::unique_ptr<SidebarsByRefResponseType> p);

	::std::unique_ptr<SidebarsByRefResponseType> setDetachSidebarsByRefResponse();

	// Constructors.
	//
	CcmpSidebarsByRefResponseMessageType(const ConfUserIDType &,
	                                     const ResponseCodeType &,
	                                     const SidebarsByRefResponseType &);

	CcmpSidebarsByRefResponseMessageType(const ConfUserIDType &,
	                                     const ResponseCodeType &,
	                                     ::std::unique_ptr<SidebarsByRefResponseType>);

	CcmpSidebarsByRefResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                                     ::std::unique_ptr<ResponseCodeType>,
	                                     ::std::unique_ptr<SidebarsByRefResponseType>);

	CcmpSidebarsByRefResponseMessageType(const ::xercesc::DOMElement &e,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpSidebarsByRefResponseMessageType(const CcmpSidebarsByRefResponseMessageType &x,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpSidebarsByRefResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpSidebarsByRefResponseMessageType &operator=(const CcmpSidebarsByRefResponseMessageType &x);

	virtual ~CcmpSidebarsByRefResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SidebarsByRefResponseType> sidebarsByRefResponse_;
};

class SidebarsByRefResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// sidebarsByRefInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::UrisType SidebarsByRefInfoType;
	typedef ::xsd::cxx::tree::optional<SidebarsByRefInfoType> SidebarsByRefInfoOptional;
	typedef ::xsd::cxx::tree::traits<SidebarsByRefInfoType, char> SidebarsByRefInfoTraits;

	const SidebarsByRefInfoOptional &getSidebarsByRefInfo() const;

	SidebarsByRefInfoOptional &getSidebarsByRefInfo();

	void setSidebarsByRefInfo(const SidebarsByRefInfoType &x);

	void setSidebarsByRefInfo(const SidebarsByRefInfoOptional &x);

	void setSidebarsByRefInfo(::std::unique_ptr<SidebarsByRefInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SidebarsByRefResponseType();

	SidebarsByRefResponseType(const ::xercesc::DOMElement &e,
	                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SidebarsByRefResponseType(const SidebarsByRefResponseType &x,
	                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SidebarsByRefResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SidebarsByRefResponseType &operator=(const SidebarsByRefResponseType &x);

	virtual ~SidebarsByRefResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	SidebarsByRefInfoOptional sidebarsByRefInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpSidebarByValResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// sidebarByValResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType SidebarByValResponseType;
	typedef ::xsd::cxx::tree::traits<SidebarByValResponseType, char> SidebarByValResponseTraits;

	const SidebarByValResponseType &getSidebarByValResponse() const;

	SidebarByValResponseType &getSidebarByValResponse();

	void setSidebarByValResponse(const SidebarByValResponseType &x);

	void setSidebarByValResponse(::std::unique_ptr<SidebarByValResponseType> p);

	::std::unique_ptr<SidebarByValResponseType> setDetachSidebarByValResponse();

	// Constructors.
	//
	CcmpSidebarByValResponseMessageType(const ConfUserIDType &,
	                                    const ResponseCodeType &,
	                                    const SidebarByValResponseType &);

	CcmpSidebarByValResponseMessageType(const ConfUserIDType &,
	                                    const ResponseCodeType &,
	                                    ::std::unique_ptr<SidebarByValResponseType>);

	CcmpSidebarByValResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                                    ::std::unique_ptr<ResponseCodeType>,
	                                    ::std::unique_ptr<SidebarByValResponseType>);

	CcmpSidebarByValResponseMessageType(const ::xercesc::DOMElement &e,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpSidebarByValResponseMessageType(const CcmpSidebarByValResponseMessageType &x,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpSidebarByValResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpSidebarByValResponseMessageType &operator=(const CcmpSidebarByValResponseMessageType &x);

	virtual ~CcmpSidebarByValResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SidebarByValResponseType> sidebarByValResponse_;
};

class SidebarByValResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// sidebarByValInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType SidebarByValInfoType;
	typedef ::xsd::cxx::tree::optional<SidebarByValInfoType> SidebarByValInfoOptional;
	typedef ::xsd::cxx::tree::traits<SidebarByValInfoType, char> SidebarByValInfoTraits;

	const SidebarByValInfoOptional &getSidebarByValInfo() const;

	SidebarByValInfoOptional &getSidebarByValInfo();

	void setSidebarByValInfo(const SidebarByValInfoType &x);

	void setSidebarByValInfo(const SidebarByValInfoOptional &x);

	void setSidebarByValInfo(::std::unique_ptr<SidebarByValInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SidebarByValResponseType();

	SidebarByValResponseType(const ::xercesc::DOMElement &e,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SidebarByValResponseType(const SidebarByValResponseType &x,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SidebarByValResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SidebarByValResponseType &operator=(const SidebarByValResponseType &x);

	virtual ~SidebarByValResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	SidebarByValInfoOptional sidebarByValInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpSidebarByRefResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// sidebarByRefResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType SidebarByRefResponseType;
	typedef ::xsd::cxx::tree::traits<SidebarByRefResponseType, char> SidebarByRefResponseTraits;

	const SidebarByRefResponseType &getSidebarByRefResponse() const;

	SidebarByRefResponseType &getSidebarByRefResponse();

	void setSidebarByRefResponse(const SidebarByRefResponseType &x);

	void setSidebarByRefResponse(::std::unique_ptr<SidebarByRefResponseType> p);

	::std::unique_ptr<SidebarByRefResponseType> setDetachSidebarByRefResponse();

	// Constructors.
	//
	CcmpSidebarByRefResponseMessageType(const ConfUserIDType &,
	                                    const ResponseCodeType &,
	                                    const SidebarByRefResponseType &);

	CcmpSidebarByRefResponseMessageType(const ConfUserIDType &,
	                                    const ResponseCodeType &,
	                                    ::std::unique_ptr<SidebarByRefResponseType>);

	CcmpSidebarByRefResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                                    ::std::unique_ptr<ResponseCodeType>,
	                                    ::std::unique_ptr<SidebarByRefResponseType>);

	CcmpSidebarByRefResponseMessageType(const ::xercesc::DOMElement &e,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpSidebarByRefResponseMessageType(const CcmpSidebarByRefResponseMessageType &x,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpSidebarByRefResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpSidebarByRefResponseMessageType &operator=(const CcmpSidebarByRefResponseMessageType &x);

	virtual ~CcmpSidebarByRefResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SidebarByRefResponseType> sidebarByRefResponse_;
};

class SidebarByRefResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// sidebarByRefInfo
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfo::ConferenceType SidebarByRefInfoType;
	typedef ::xsd::cxx::tree::optional<SidebarByRefInfoType> SidebarByRefInfoOptional;
	typedef ::xsd::cxx::tree::traits<SidebarByRefInfoType, char> SidebarByRefInfoTraits;

	const SidebarByRefInfoOptional &getSidebarByRefInfo() const;

	SidebarByRefInfoOptional &getSidebarByRefInfo();

	void setSidebarByRefInfo(const SidebarByRefInfoType &x);

	void setSidebarByRefInfo(const SidebarByRefInfoOptional &x);

	void setSidebarByRefInfo(::std::unique_ptr<SidebarByRefInfoType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SidebarByRefResponseType();

	SidebarByRefResponseType(const ::xercesc::DOMElement &e,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SidebarByRefResponseType(const SidebarByRefResponseType &x,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SidebarByRefResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SidebarByRefResponseType &operator=(const SidebarByRefResponseType &x);

	virtual ~SidebarByRefResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	SidebarByRefInfoOptional sidebarByRefInfo_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class CcmpExtendedResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// extendedResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType ExtendedResponseType;
	typedef ::xsd::cxx::tree::traits<ExtendedResponseType, char> ExtendedResponseTraits;

	const ExtendedResponseType &getExtendedResponse() const;

	ExtendedResponseType &getExtendedResponse();

	void setExtendedResponse(const ExtendedResponseType &x);

	void setExtendedResponse(::std::unique_ptr<ExtendedResponseType> p);

	::std::unique_ptr<ExtendedResponseType> setDetachExtendedResponse();

	// Constructors.
	//
	CcmpExtendedResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, const ExtendedResponseType &);

	CcmpExtendedResponseMessageType(const ConfUserIDType &,
	                                const ResponseCodeType &,
	                                ::std::unique_ptr<ExtendedResponseType>);

	CcmpExtendedResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                                ::std::unique_ptr<ResponseCodeType>,
	                                ::std::unique_ptr<ExtendedResponseType>);

	CcmpExtendedResponseMessageType(const ::xercesc::DOMElement &e,
	                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpExtendedResponseMessageType(const CcmpExtendedResponseMessageType &x,
	                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpExtendedResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpExtendedResponseMessageType &operator=(const CcmpExtendedResponseMessageType &x);

	virtual ~CcmpExtendedResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<ExtendedResponseType> extendedResponse_;
};

class ExtendedResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// extensionName
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ExtensionNameType;
	typedef ::xsd::cxx::tree::traits<ExtensionNameType, char> ExtensionNameTraits;

	const ExtensionNameType &getExtensionName() const;

	ExtensionNameType &getExtensionName();

	void setExtensionName(const ExtensionNameType &x);

	void setExtensionName(::std::unique_ptr<ExtensionNameType> p);

	::std::unique_ptr<ExtensionNameType> setDetachExtensionName();

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	ExtendedResponseType(const ExtensionNameType &);

	ExtendedResponseType(::std::unique_ptr<ExtensionNameType>);

	ExtendedResponseType(const ::xercesc::DOMElement &e,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ExtendedResponseType(const ExtendedResponseType &x,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ExtendedResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ExtendedResponseType &operator=(const ExtendedResponseType &x);

	virtual ~ExtendedResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<ExtensionNameType> extensionName_;
	AnySequence any_;
};

class CcmpOptionsResponseMessageType : public ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseMessageType {
public:
	// optionsResponse
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType OptionsResponseType;
	typedef ::xsd::cxx::tree::traits<OptionsResponseType, char> OptionsResponseTraits;

	const OptionsResponseType &getOptionsResponse() const;

	OptionsResponseType &getOptionsResponse();

	void setOptionsResponse(const OptionsResponseType &x);

	void setOptionsResponse(::std::unique_ptr<OptionsResponseType> p);

	::std::unique_ptr<OptionsResponseType> setDetachOptionsResponse();

	// Constructors.
	//
	CcmpOptionsResponseMessageType(const ConfUserIDType &, const ResponseCodeType &, const OptionsResponseType &);

	CcmpOptionsResponseMessageType(const ConfUserIDType &,
	                               const ResponseCodeType &,
	                               ::std::unique_ptr<OptionsResponseType>);

	CcmpOptionsResponseMessageType(::std::unique_ptr<ConfUserIDType>,
	                               ::std::unique_ptr<ResponseCodeType>,
	                               ::std::unique_ptr<OptionsResponseType>);

	CcmpOptionsResponseMessageType(const ::xercesc::DOMElement &e,
	                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CcmpOptionsResponseMessageType(const CcmpOptionsResponseMessageType &x,
	                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CcmpOptionsResponseMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CcmpOptionsResponseMessageType &operator=(const CcmpOptionsResponseMessageType &x);

	virtual ~CcmpOptionsResponseMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<OptionsResponseType> optionsResponse_;
};

class OptionsResponseType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// options
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::OptionsType OptionsType;
	typedef ::xsd::cxx::tree::optional<OptionsType> OptionsOptional;
	typedef ::xsd::cxx::tree::traits<OptionsType, char> OptionsTraits;

	const OptionsOptional &getOptions() const;

	OptionsOptional &getOptions();

	void setOptions(const OptionsType &x);

	void setOptions(const OptionsOptional &x);

	void setOptions(::std::unique_ptr<OptionsType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	OptionsResponseType();

	OptionsResponseType(const ::xercesc::DOMElement &e,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	OptionsResponseType(const OptionsResponseType &x,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual OptionsResponseType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	OptionsResponseType &operator=(const OptionsResponseType &x);

	virtual ~OptionsResponseType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	OptionsOptional options_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class ResponseCodeType : public ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::PositiveInteger,
                                                                   char,
                                                                   ::LinphonePrivate::Xsd::XmlSchema::SimpleType> {
public:
	// Constructors.
	//
	ResponseCodeType(const ::LinphonePrivate::Xsd::XmlSchema::PositiveInteger &);

	ResponseCodeType(const ::xercesc::DOMElement &e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ResponseCodeType(const ::xercesc::DOMAttr &a,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ResponseCodeType(const ::std::string &s,
	                 const ::xercesc::DOMElement *e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ResponseCodeType(const ResponseCodeType &x,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ResponseCodeType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	ResponseCodeType &operator=(const ResponseCodeType &) = default;
#endif

	virtual ~ResponseCodeType();
};

class OperationType : public ::LinphonePrivate::Xsd::XmlSchema::Token {
public:
	enum Value { retrieve, create, update, delete_ };

	OperationType(Value v);

	OperationType(const char *v);

	OperationType(const ::std::string &v);

	OperationType(const ::LinphonePrivate::Xsd::XmlSchema::Token &v);

	OperationType(const ::xercesc::DOMElement &e,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	OperationType(const ::xercesc::DOMAttr &a,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	OperationType(const ::std::string &s,
	              const ::xercesc::DOMElement *e,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	OperationType(const OperationType &x,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

#ifdef XSD_CXX11
	OperationType &operator=(const OperationType &) = default;
#endif

	virtual OperationType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	OperationType &operator=(Value v);

	virtual operator Value() const {
		return _xsd_OperationType_convert();
	}

protected:
	Value _xsd_OperationType_convert() const;

public:
	static const char *const _xsd_OperationType_literals_[4];
	static const Value _xsd_OperationType_indexes_[4];
};

class SubjectType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// username
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String UsernameType;
	typedef ::xsd::cxx::tree::optional<UsernameType> UsernameOptional;
	typedef ::xsd::cxx::tree::traits<UsernameType, char> UsernameTraits;

	const UsernameOptional &getUsername() const;

	UsernameOptional &getUsername();

	void setUsername(const UsernameType &x);

	void setUsername(const UsernameOptional &x);

	void setUsername(::std::unique_ptr<UsernameType> p);

	// password
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String PasswordType;
	typedef ::xsd::cxx::tree::optional<PasswordType> PasswordOptional;
	typedef ::xsd::cxx::tree::traits<PasswordType, char> PasswordTraits;

	const PasswordOptional &getPassword() const;

	PasswordOptional &getPassword();

	void setPassword(const PasswordType &x);

	void setPassword(const PasswordOptional &x);

	void setPassword(::std::unique_ptr<PasswordType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	SubjectType();

	SubjectType(const ::xercesc::DOMElement &e,
	            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	SubjectType(const SubjectType &x,
	            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual SubjectType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	SubjectType &operator=(const SubjectType &x);

	virtual ~SubjectType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	UsernameOptional username_;
	PasswordOptional password_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class OptionsType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// standard-message-list
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::StandardMessageListType StandardMessageListType;
	typedef ::xsd::cxx::tree::traits<StandardMessageListType, char> StandardMessageListTraits;

	const StandardMessageListType &getStandardMessageList() const;

	StandardMessageListType &getStandardMessageList();

	void setStandardMessageList(const StandardMessageListType &x);

	void setStandardMessageList(::std::unique_ptr<StandardMessageListType> p);

	::std::unique_ptr<StandardMessageListType> setDetachStandard_message_list();

	// extended-message-list
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ExtendedMessageListType ExtendedMessageListType;
	typedef ::xsd::cxx::tree::optional<ExtendedMessageListType> ExtendedMessageListOptional;
	typedef ::xsd::cxx::tree::traits<ExtendedMessageListType, char> ExtendedMessageListTraits;

	const ExtendedMessageListOptional &getExtendedMessageList() const;

	ExtendedMessageListOptional &getExtendedMessageList();

	void setExtendedMessageList(const ExtendedMessageListType &x);

	void setExtendedMessageList(const ExtendedMessageListOptional &x);

	void setExtendedMessageList(::std::unique_ptr<ExtendedMessageListType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	OptionsType(const StandardMessageListType &);

	OptionsType(::std::unique_ptr<StandardMessageListType>);

	OptionsType(const ::xercesc::DOMElement &e,
	            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	OptionsType(const OptionsType &x,
	            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual OptionsType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                            ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	OptionsType &operator=(const OptionsType &x);

	virtual ~OptionsType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<StandardMessageListType> standard_message_list_;
	ExtendedMessageListOptional extended_message_list_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class StandardMessageListType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// standard-message
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::StandardMessageType StandardMessageType;
	typedef ::xsd::cxx::tree::sequence<StandardMessageType> StandardMessageSequence;
	typedef StandardMessageSequence::iterator StandardMessageIterator;
	typedef StandardMessageSequence::const_iterator StandardMessageConstIterator;
	typedef ::xsd::cxx::tree::traits<StandardMessageType, char> StandardMessageTraits;

	const StandardMessageSequence &getStandardMessage() const;

	StandardMessageSequence &getStandardMessage();

	void setStandardMessage(const StandardMessageSequence &s);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	StandardMessageListType();

	StandardMessageListType(const ::xercesc::DOMElement &e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	StandardMessageListType(const StandardMessageListType &x,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual StandardMessageListType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	StandardMessageListType &operator=(const StandardMessageListType &x);

	virtual ~StandardMessageListType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	StandardMessageSequence standard_message_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class StandardMessageType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// name
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::StandardMessageNameType NameType;
	typedef ::xsd::cxx::tree::traits<NameType, char> NameTraits;

	const NameType &getName() const;

	NameType &getName();

	void setName(const NameType &x);

	void setName(::std::unique_ptr<NameType> p);

	::std::unique_ptr<NameType> setDetachName();

	// operations
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::OperationsType OperationsType;
	typedef ::xsd::cxx::tree::optional<OperationsType> OperationsOptional;
	typedef ::xsd::cxx::tree::traits<OperationsType, char> OperationsTraits;

	const OperationsOptional &getOperations() const;

	OperationsOptional &getOperations();

	void setOperations(const OperationsType &x);

	void setOperations(const OperationsOptional &x);

	void setOperations(::std::unique_ptr<OperationsType> p);

	// schema-def
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String SchemaDefType;
	typedef ::xsd::cxx::tree::optional<SchemaDefType> SchemaDefOptional;
	typedef ::xsd::cxx::tree::traits<SchemaDefType, char> SchemaDefTraits;

	const SchemaDefOptional &getSchemaDef() const;

	SchemaDefOptional &getSchemaDef();

	void setSchemaDef(const SchemaDefType &x);

	void setSchemaDef(const SchemaDefOptional &x);

	void setSchemaDef(::std::unique_ptr<SchemaDefType> p);

	// description
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String DescriptionType;
	typedef ::xsd::cxx::tree::optional<DescriptionType> DescriptionOptional;
	typedef ::xsd::cxx::tree::traits<DescriptionType, char> DescriptionTraits;

	const DescriptionOptional &getDescription() const;

	DescriptionOptional &getDescription();

	void setDescription(const DescriptionType &x);

	void setDescription(const DescriptionOptional &x);

	void setDescription(::std::unique_ptr<DescriptionType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	StandardMessageType(const NameType &);

	StandardMessageType(::std::unique_ptr<NameType>);

	StandardMessageType(const ::xercesc::DOMElement &e,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	StandardMessageType(const StandardMessageType &x,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual StandardMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	StandardMessageType &operator=(const StandardMessageType &x);

	virtual ~StandardMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<NameType> name_;
	OperationsOptional operations_;
	SchemaDefOptional schema_def_;
	DescriptionOptional description_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class StandardMessageNameType : public ::LinphonePrivate::Xsd::XmlSchema::Token {
public:
	enum Value {
		confsRequest,
		confRequest,
		blueprintsRequest,
		blueprintRequest,
		usersRequest,
		userRequest,
		sidebarsByValRequest,
		sidebarByValRequest,
		sidebarsByRefRequest,
		sidebarByRefRequest
	};

	StandardMessageNameType(Value v);

	StandardMessageNameType(const char *v);

	StandardMessageNameType(const ::std::string &v);

	StandardMessageNameType(const ::LinphonePrivate::Xsd::XmlSchema::Token &v);

	StandardMessageNameType(const ::xercesc::DOMElement &e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	StandardMessageNameType(const ::xercesc::DOMAttr &a,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	StandardMessageNameType(const ::std::string &s,
	                        const ::xercesc::DOMElement *e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	StandardMessageNameType(const StandardMessageNameType &x,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

#ifdef XSD_CXX11
	StandardMessageNameType &operator=(const StandardMessageNameType &) = default;
#endif

	virtual StandardMessageNameType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	StandardMessageNameType &operator=(Value v);

	virtual operator Value() const {
		return _xsd_StandardMessageNameType_convert();
	}

protected:
	Value _xsd_StandardMessageNameType_convert() const;

public:
	static const char *const _xsd_StandardMessageNameType_literals_[10];
	static const Value _xsd_StandardMessageNameType_indexes_[10];
};

class OperationsType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// operation
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::OperationType OperationType;
	typedef ::xsd::cxx::tree::sequence<OperationType> OperationSequence;
	typedef OperationSequence::iterator OperationIterator;
	typedef OperationSequence::const_iterator OperationConstIterator;
	typedef ::xsd::cxx::tree::traits<OperationType, char> OperationTraits;

	const OperationSequence &getOperation() const;

	OperationSequence &getOperation();

	void setOperation(const OperationSequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	OperationsType();

	OperationsType(const ::xercesc::DOMElement &e,
	               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	OperationsType(const OperationsType &x,
	               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual OperationsType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	OperationsType &operator=(const OperationsType &x);

	virtual ~OperationsType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	OperationSequence operation_;
	AnyAttributeSet any_attribute_;
};

class ExtendedMessageListType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// extended-message
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::ExtendedMessageType ExtendedMessageType;
	typedef ::xsd::cxx::tree::optional<ExtendedMessageType> ExtendedMessageOptional;
	typedef ::xsd::cxx::tree::traits<ExtendedMessageType, char> ExtendedMessageTraits;

	const ExtendedMessageOptional &getExtendedMessage() const;

	ExtendedMessageOptional &getExtendedMessage();

	void setExtendedMessage(const ExtendedMessageType &x);

	void setExtendedMessage(const ExtendedMessageOptional &x);

	void setExtendedMessage(::std::unique_ptr<ExtendedMessageType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	ExtendedMessageListType();

	ExtendedMessageListType(const ::xercesc::DOMElement &e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ExtendedMessageListType(const ExtendedMessageListType &x,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ExtendedMessageListType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ExtendedMessageListType &operator=(const ExtendedMessageListType &x);

	virtual ~ExtendedMessageListType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	ExtendedMessageOptional extended_message_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class ExtendedMessageType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// name
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String NameType;
	typedef ::xsd::cxx::tree::traits<NameType, char> NameTraits;

	const NameType &getName() const;

	NameType &getName();

	void setName(const NameType &x);

	void setName(::std::unique_ptr<NameType> p);

	::std::unique_ptr<NameType> setDetachName();

	// operations
	//
	typedef ::LinphonePrivate::Xsd::XconCcmp::OperationsType OperationsType;
	typedef ::xsd::cxx::tree::optional<OperationsType> OperationsOptional;
	typedef ::xsd::cxx::tree::traits<OperationsType, char> OperationsTraits;

	const OperationsOptional &getOperations() const;

	OperationsOptional &getOperations();

	void setOperations(const OperationsType &x);

	void setOperations(const OperationsOptional &x);

	void setOperations(::std::unique_ptr<OperationsType> p);

	// schema-def
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String SchemaDefType;
	typedef ::xsd::cxx::tree::traits<SchemaDefType, char> SchemaDefTraits;

	const SchemaDefType &getSchemaDef() const;

	SchemaDefType &getSchemaDef();

	void setSchemaDef(const SchemaDefType &x);

	void setSchemaDef(::std::unique_ptr<SchemaDefType> p);

	::std::unique_ptr<SchemaDefType> setDetachSchema_def();

	// description
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String DescriptionType;
	typedef ::xsd::cxx::tree::optional<DescriptionType> DescriptionOptional;
	typedef ::xsd::cxx::tree::traits<DescriptionType, char> DescriptionTraits;

	const DescriptionOptional &getDescription() const;

	DescriptionOptional &getDescription();

	void setDescription(const DescriptionType &x);

	void setDescription(const DescriptionOptional &x);

	void setDescription(::std::unique_ptr<DescriptionType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	ExtendedMessageType(const NameType &, const SchemaDefType &);

	ExtendedMessageType(::std::unique_ptr<NameType>, ::std::unique_ptr<SchemaDefType>);

	ExtendedMessageType(const ::xercesc::DOMElement &e,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ExtendedMessageType(const ExtendedMessageType &x,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ExtendedMessageType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ExtendedMessageType &operator=(const ExtendedMessageType &x);

	virtual ~ExtendedMessageType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<NameType> name_;
	OperationsOptional operations_;
	::xsd::cxx::tree::one<SchemaDefType> schema_def_;
	DescriptionOptional description_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};
} // namespace XconCcmp
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

namespace LinphonePrivate {
namespace Xsd {
namespace XconCcmp {
::std::ostream &operator<<(::std::ostream &, const CcmpRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const CcmpResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const CcmpBlueprintsRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const BlueprintsRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpBlueprintRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const BlueprintRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpConfsRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const ConfsRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpConfRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const ConfRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpUsersRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const UsersRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpUserRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const UserRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpSidebarsByValRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const SidebarsByValRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpSidebarsByRefRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const SidebarsByRefRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpSidebarByValRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const SidebarByValRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpSidebarByRefRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const SidebarByRefRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpExtendedRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const ExtendedRequestType &);

::std::ostream &operator<<(::std::ostream &, const CcmpOptionsRequestMessageType &);

::std::ostream &operator<<(::std::ostream &, const CcmpBlueprintsResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const BlueprintsResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpBlueprintResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const BlueprintResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpConfsResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const ConfsResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpConfResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const ConfResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpUsersResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const UsersResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpUserResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const UserResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpSidebarsByValResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const SidebarsByValResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpSidebarsByRefResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const SidebarsByRefResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpSidebarByValResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const SidebarByValResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpSidebarByRefResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const SidebarByRefResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpExtendedResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const ExtendedResponseType &);

::std::ostream &operator<<(::std::ostream &, const CcmpOptionsResponseMessageType &);

::std::ostream &operator<<(::std::ostream &, const OptionsResponseType &);

::std::ostream &operator<<(::std::ostream &, const ResponseCodeType &);

::std::ostream &operator<<(::std::ostream &, OperationType::Value);

::std::ostream &operator<<(::std::ostream &, const OperationType &);

::std::ostream &operator<<(::std::ostream &, const SubjectType &);

::std::ostream &operator<<(::std::ostream &, const OptionsType &);

::std::ostream &operator<<(::std::ostream &, const StandardMessageListType &);

::std::ostream &operator<<(::std::ostream &, const StandardMessageType &);

::std::ostream &operator<<(::std::ostream &, StandardMessageNameType::Value);

::std::ostream &operator<<(::std::ostream &, const StandardMessageNameType &);

::std::ostream &operator<<(::std::ostream &, const OperationsType &);

::std::ostream &operator<<(::std::ostream &, const ExtendedMessageListType &);

::std::ostream &operator<<(::std::ostream &, const ExtendedMessageType &);
} // namespace XconCcmp
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/sax/InputSource.hpp>

namespace LinphonePrivate {
namespace Xsd {
namespace XconCcmp {
// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType> parseCcmpRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType> parseCcmpResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType> parseBlueprintsRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType> parseBlueprintRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType> parseConfsRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfRequestType> parseConfRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersRequestType> parseUsersRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserRequestType> parseUserRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType> parseSidebarsByValRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType> parseSidebarsByRefRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType> parseSidebarByValRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType> parseSidebarByRefRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType> parseExtendedRequest(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType> parseBlueprintsResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType> parseBlueprintResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType> parseConfsResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ConfResponseType> parseConfResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UsersResponseType> parseUsersResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::UserResponseType> parseUserResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType> parseSidebarsByValResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType> parseSidebarsByRefResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType> parseSidebarByValResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType> parseSidebarByRefResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType> parseExtendedResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType> parseOptionsResponse(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());
} // namespace XconCcmp
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace XconCcmp {
// Serialize to std::ostream.
//

void serializeCcmpRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCcmpRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCcmpRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &x,
                          ::xercesc::DOMErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeCcmpRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCcmpRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCcmpRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &x,
                          ::xercesc::DOMErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeCcmpRequest(::xercesc::DOMDocument &d,
                          const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCcmpRequest(const ::LinphonePrivate::Xsd::XconCcmp::CcmpRequestType &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeCcmpResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCcmpResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCcmpResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeCcmpResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCcmpResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCcmpResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeCcmpResponse(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCcmpResponse(const ::LinphonePrivate::Xsd::XconCcmp::CcmpResponseType &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const CcmpRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpRequestMessageType &);

void operator<<(::xercesc::DOMElement &, const CcmpResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpResponseMessageType &);

void operator<<(::xercesc::DOMElement &, const CcmpBlueprintsRequestMessageType &);

// Serialize to std::ostream.
//

void serializeBlueprintsRequest(::std::ostream &os,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &x,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintsRequest(::std::ostream &os,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &x,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintsRequest(::std::ostream &os,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &x,
                                ::xercesc::DOMErrorHandler &eh,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeBlueprintsRequest(::xercesc::XMLFormatTarget &ft,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &x,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintsRequest(::xercesc::XMLFormatTarget &ft,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &x,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintsRequest(::xercesc::XMLFormatTarget &ft,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &x,
                                ::xercesc::DOMErrorHandler &eh,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeBlueprintsRequest(::xercesc::DOMDocument &d,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &x,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeBlueprintsRequest(const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsRequestType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const BlueprintsRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpBlueprintRequestMessageType &);

// Serialize to std::ostream.
//

void serializeBlueprintRequest(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintRequest(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintRequest(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeBlueprintRequest(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintRequest(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintRequest(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeBlueprintRequest(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeBlueprintRequest(const ::LinphonePrivate::Xsd::XconCcmp::BlueprintRequestType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const BlueprintRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpConfsRequestMessageType &);

// Serialize to std::ostream.
//

void serializeConfsRequest(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfsRequest(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfsRequest(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeConfsRequest(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfsRequest(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfsRequest(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeConfsRequest(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConfsRequest(const ::LinphonePrivate::Xsd::XconCcmp::ConfsRequestType &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const ConfsRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpConfRequestMessageType &);

// Serialize to std::ostream.
//

void serializeConfRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &x,
                          ::xercesc::DOMErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeConfRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &x,
                          ::xercesc::DOMErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeConfRequest(::xercesc::DOMDocument &d,
                          const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConfRequest(const ::LinphonePrivate::Xsd::XconCcmp::ConfRequestType &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const ConfRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpUsersRequestMessageType &);

// Serialize to std::ostream.
//

void serializeUsersRequest(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUsersRequest(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUsersRequest(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeUsersRequest(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUsersRequest(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUsersRequest(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeUsersRequest(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUsersRequest(const ::LinphonePrivate::Xsd::XconCcmp::UsersRequestType &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const UsersRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpUserRequestMessageType &);

// Serialize to std::ostream.
//

void serializeUserRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserRequest(::std::ostream &os,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &x,
                          ::xercesc::DOMErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeUserRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserRequest(::xercesc::XMLFormatTarget &ft,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &x,
                          ::xercesc::DOMErrorHandler &eh,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          const ::std::string &e = "UTF-8",
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeUserRequest(::xercesc::DOMDocument &d,
                          const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &x,
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUserRequest(const ::LinphonePrivate::Xsd::XconCcmp::UserRequestType &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const UserRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpSidebarsByValRequestMessageType &);

// Serialize to std::ostream.
//

void serializeSidebarsByValRequest(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByValRequest(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByValRequest(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarsByValRequest(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByValRequest(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByValRequest(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarsByValRequest(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarsByValRequest(const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValRequestType &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const SidebarsByValRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpSidebarsByRefRequestMessageType &);

// Serialize to std::ostream.
//

void serializeSidebarsByRefRequest(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByRefRequest(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByRefRequest(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarsByRefRequest(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByRefRequest(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByRefRequest(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarsByRefRequest(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarsByRefRequest(const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefRequestType &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const SidebarsByRefRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpSidebarByValRequestMessageType &);

// Serialize to std::ostream.
//

void serializeSidebarByValRequest(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByValRequest(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByValRequest(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarByValRequest(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByValRequest(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByValRequest(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarByValRequest(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarByValRequest(const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValRequestType &x,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const SidebarByValRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpSidebarByRefRequestMessageType &);

// Serialize to std::ostream.
//

void serializeSidebarByRefRequest(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByRefRequest(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByRefRequest(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarByRefRequest(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByRefRequest(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByRefRequest(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarByRefRequest(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarByRefRequest(const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefRequestType &x,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const SidebarByRefRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpExtendedRequestMessageType &);

// Serialize to std::ostream.
//

void serializeExtendedRequest(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeExtendedRequest(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &x,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeExtendedRequest(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &x,
                              ::xercesc::DOMErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeExtendedRequest(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeExtendedRequest(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &x,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeExtendedRequest(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &x,
                              ::xercesc::DOMErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeExtendedRequest(::xercesc::DOMDocument &d,
                              const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &x,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeExtendedRequest(const ::LinphonePrivate::Xsd::XconCcmp::ExtendedRequestType &x,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const ExtendedRequestType &);

void operator<<(::xercesc::DOMElement &, const CcmpOptionsRequestMessageType &);

void operator<<(::xercesc::DOMElement &, const CcmpBlueprintsResponseMessageType &);

// Serialize to std::ostream.
//

void serializeBlueprintsResponse(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &x,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintsResponse(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintsResponse(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &x,
                                 ::xercesc::DOMErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeBlueprintsResponse(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &x,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintsResponse(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintsResponse(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &x,
                                 ::xercesc::DOMErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeBlueprintsResponse(::xercesc::DOMDocument &d,
                                 const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeBlueprintsResponse(const ::LinphonePrivate::Xsd::XconCcmp::BlueprintsResponseType &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const BlueprintsResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpBlueprintResponseMessageType &);

// Serialize to std::ostream.
//

void serializeBlueprintResponse(::std::ostream &os,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &x,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintResponse(::std::ostream &os,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &x,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintResponse(::std::ostream &os,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &x,
                                ::xercesc::DOMErrorHandler &eh,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeBlueprintResponse(::xercesc::XMLFormatTarget &ft,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &x,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintResponse(::xercesc::XMLFormatTarget &ft,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &x,
                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeBlueprintResponse(::xercesc::XMLFormatTarget &ft,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &x,
                                ::xercesc::DOMErrorHandler &eh,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                const ::std::string &e = "UTF-8",
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeBlueprintResponse(::xercesc::DOMDocument &d,
                                const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &x,
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeBlueprintResponse(const ::LinphonePrivate::Xsd::XconCcmp::BlueprintResponseType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const BlueprintResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpConfsResponseMessageType &);

// Serialize to std::ostream.
//

void serializeConfsResponse(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfsResponse(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfsResponse(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeConfsResponse(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfsResponse(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfsResponse(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeConfsResponse(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConfsResponse(const ::LinphonePrivate::Xsd::XconCcmp::ConfsResponseType &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const ConfsResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpConfResponseMessageType &);

// Serialize to std::ostream.
//

void serializeConfResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeConfResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConfResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeConfResponse(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConfResponse(const ::LinphonePrivate::Xsd::XconCcmp::ConfResponseType &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const ConfResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpUsersResponseMessageType &);

// Serialize to std::ostream.
//

void serializeUsersResponse(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUsersResponse(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUsersResponse(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeUsersResponse(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUsersResponse(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUsersResponse(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeUsersResponse(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUsersResponse(const ::LinphonePrivate::Xsd::XconCcmp::UsersResponseType &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const UsersResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpUserResponseMessageType &);

// Serialize to std::ostream.
//

void serializeUserResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserResponse(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeUserResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserResponse(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeUserResponse(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUserResponse(const ::LinphonePrivate::Xsd::XconCcmp::UserResponseType &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const UserResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpSidebarsByValResponseMessageType &);

// Serialize to std::ostream.
//

void serializeSidebarsByValResponse(::std::ostream &os,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &x,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByValResponse(::std::ostream &os,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &x,
                                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByValResponse(::std::ostream &os,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &x,
                                    ::xercesc::DOMErrorHandler &eh,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarsByValResponse(::xercesc::XMLFormatTarget &ft,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &x,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByValResponse(::xercesc::XMLFormatTarget &ft,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &x,
                                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByValResponse(::xercesc::XMLFormatTarget &ft,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &x,
                                    ::xercesc::DOMErrorHandler &eh,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarsByValResponse(::xercesc::DOMDocument &d,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &x,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarsByValResponse(const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByValResponseType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const SidebarsByValResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpSidebarsByRefResponseMessageType &);

// Serialize to std::ostream.
//

void serializeSidebarsByRefResponse(::std::ostream &os,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &x,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByRefResponse(::std::ostream &os,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &x,
                                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByRefResponse(::std::ostream &os,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &x,
                                    ::xercesc::DOMErrorHandler &eh,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarsByRefResponse(::xercesc::XMLFormatTarget &ft,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &x,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByRefResponse(::xercesc::XMLFormatTarget &ft,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &x,
                                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarsByRefResponse(::xercesc::XMLFormatTarget &ft,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &x,
                                    ::xercesc::DOMErrorHandler &eh,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    const ::std::string &e = "UTF-8",
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarsByRefResponse(::xercesc::DOMDocument &d,
                                    const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &x,
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarsByRefResponse(const ::LinphonePrivate::Xsd::XconCcmp::SidebarsByRefResponseType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const SidebarsByRefResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpSidebarByValResponseMessageType &);

// Serialize to std::ostream.
//

void serializeSidebarByValResponse(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByValResponse(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByValResponse(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarByValResponse(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByValResponse(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByValResponse(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarByValResponse(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarByValResponse(const ::LinphonePrivate::Xsd::XconCcmp::SidebarByValResponseType &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const SidebarByValResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpSidebarByRefResponseMessageType &);

// Serialize to std::ostream.
//

void serializeSidebarByRefResponse(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByRefResponse(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByRefResponse(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarByRefResponse(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByRefResponse(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarByRefResponse(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarByRefResponse(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarByRefResponse(const ::LinphonePrivate::Xsd::XconCcmp::SidebarByRefResponseType &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const SidebarByRefResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpExtendedResponseMessageType &);

// Serialize to std::ostream.
//

void serializeExtendedResponse(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeExtendedResponse(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeExtendedResponse(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeExtendedResponse(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeExtendedResponse(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeExtendedResponse(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeExtendedResponse(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeExtendedResponse(const ::LinphonePrivate::Xsd::XconCcmp::ExtendedResponseType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const ExtendedResponseType &);

void operator<<(::xercesc::DOMElement &, const CcmpOptionsResponseMessageType &);

// Serialize to std::ostream.
//

void serializeOptionsResponse(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeOptionsResponse(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &x,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeOptionsResponse(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &x,
                              ::xercesc::DOMErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeOptionsResponse(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeOptionsResponse(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &x,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeOptionsResponse(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &x,
                              ::xercesc::DOMErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeOptionsResponse(::xercesc::DOMDocument &d,
                              const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &x,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeOptionsResponse(const ::LinphonePrivate::Xsd::XconCcmp::OptionsResponseType &x,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const OptionsResponseType &);

void operator<<(::xercesc::DOMElement &, const ResponseCodeType &);

void operator<<(::xercesc::DOMAttr &, const ResponseCodeType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const ResponseCodeType &);

void operator<<(::xercesc::DOMElement &, const OperationType &);

void operator<<(::xercesc::DOMAttr &, const OperationType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const OperationType &);

void operator<<(::xercesc::DOMElement &, const SubjectType &);

void operator<<(::xercesc::DOMElement &, const OptionsType &);

void operator<<(::xercesc::DOMElement &, const StandardMessageListType &);

void operator<<(::xercesc::DOMElement &, const StandardMessageType &);

void operator<<(::xercesc::DOMElement &, const StandardMessageNameType &);

void operator<<(::xercesc::DOMAttr &, const StandardMessageNameType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const StandardMessageNameType &);

void operator<<(::xercesc::DOMElement &, const OperationsType &);

void operator<<(::xercesc::DOMElement &, const ExtendedMessageListType &);

void operator<<(::xercesc::DOMElement &, const ExtendedMessageType &);
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

#endif // XCON_CCMP_H
