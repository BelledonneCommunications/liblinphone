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

#ifndef XCON_CONFERENCE_INFO_H
#define XCON_CONFERENCE_INFO_H

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
namespace XconConferenceInfo {
class Xpath;
class XpathAdd;
class Pos;
class Type;
class Add;
class Replace;
class Ws;
class Remove;
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
class ConferenceTimeType;
class TimeType;
class RoleType;
class MixingModeType;
class CodecsType;
class CodecType;
class DecisionType;
class PolicyType;
class ControlsType;
class GainType;
class VideoLayoutType;
class FloorInformationType;
class FloorRequestHandlingType;
class ConferenceFloorPolicy;
class AlgorithmType;
class UserAdmissionPolicyType;
class JoinHandlingType;
class DenyUsersListType;
class AllowedUsersListType;
class PersistentListType;
class TargetType;
class MethodType;
class ProvideAnonymityType;
class MixerType;
class MixerNameType;
class ConferenceInfoDiff;
class Entry;
class Floor;
class Target;
class User;
class Floor1;
class Add1;
class Remove1;
class Replace1;
class MixingStartOffset;
class MixingEndOffset;
} // namespace XconConferenceInfo
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

#include "conference-info.h"

#include "xml.h"

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
class Xpath : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	Xpath();

	Xpath(const char *);

	Xpath(const ::std::string &);

	Xpath(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	Xpath(const ::xercesc::DOMElement &e,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Xpath(const ::xercesc::DOMAttr &a,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Xpath(const ::std::string &s,
	      const ::xercesc::DOMElement *e,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Xpath(const Xpath &x,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Xpath *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Xpath &operator=(const Xpath &) = default;
#endif

	virtual ~Xpath();
};

class XpathAdd : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	XpathAdd();

	XpathAdd(const char *);

	XpathAdd(const ::std::string &);

	XpathAdd(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	XpathAdd(const ::xercesc::DOMElement &e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	XpathAdd(const ::xercesc::DOMAttr &a,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	XpathAdd(const ::std::string &s,
	         const ::xercesc::DOMElement *e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	XpathAdd(const XpathAdd &x,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual XpathAdd *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	XpathAdd &operator=(const XpathAdd &) = default;
#endif

	virtual ~XpathAdd();
};

class Pos : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	enum Value { before, after, prepend };

	Pos(Value v);

	Pos(const char *v);

	Pos(const ::std::string &v);

	Pos(const ::LinphonePrivate::Xsd::XmlSchema::String &v);

	Pos(const ::xercesc::DOMElement &e,
	    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Pos(const ::xercesc::DOMAttr &a,
	    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Pos(const ::std::string &s,
	    const ::xercesc::DOMElement *e,
	    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Pos(const Pos &x,
	    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

#ifdef XSD_CXX11
	Pos &operator=(const Pos &) = default;
#endif

	virtual Pos *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Pos &operator=(Value v);

	virtual operator Value() const {
		return _xsd_Pos_convert();
	}

protected:
	Value _xsd_Pos_convert() const;

public:
	static const char *const _xsd_Pos_literals_[3];
	static const Value _xsd_Pos_indexes_[3];
};

class Type : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	Type();

	Type(const char *);

	Type(const ::std::string &);

	Type(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	Type(const ::xercesc::DOMElement &e,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Type(const ::xercesc::DOMAttr &a,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Type(const ::std::string &s,
	     const ::xercesc::DOMElement *e,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Type(const Type &x,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Type *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Type &operator=(const Type &) = default;
#endif

	virtual ~Type();
};

class Add : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// sel
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::XpathAdd SelType;
	typedef ::xsd::cxx::tree::traits<SelType, char> SelTraits;

	const SelType &getSel() const;

	SelType &getSel();

	void setSel(const SelType &x);

	void setSel(::std::unique_ptr<SelType> p);

	::std::unique_ptr<SelType> setDetachSel();

	// pos
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Pos PosType;
	typedef ::xsd::cxx::tree::optional<PosType> PosOptional;
	typedef ::xsd::cxx::tree::traits<PosType, char> PosTraits;

	const PosOptional &getPos() const;

	PosOptional &getPos();

	void setPos(const PosType &x);

	void setPos(const PosOptional &x);

	void setPos(::std::unique_ptr<PosType> p);

	// type
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Type TypeType;
	typedef ::xsd::cxx::tree::optional<TypeType> TypeOptional;
	typedef ::xsd::cxx::tree::traits<TypeType, char> TypeTraits;

	const TypeOptional &getType() const;

	TypeOptional &getType();

	void setType(const TypeType &x);

	void setType(const TypeOptional &x);

	void setType(::std::unique_ptr<TypeType> p);

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	Add(const SelType &);

	Add(const ::LinphonePrivate::Xsd::XmlSchema::Type &, const SelType &);

	Add(const ::xercesc::DOMElement &e,
	    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Add(const Add &x,
	    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Add *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Add &operator=(const Add &x);

	virtual ~Add();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	AnySequence any_;
	::xsd::cxx::tree::one<SelType> sel_;
	PosOptional pos_;
	TypeOptional type_;
};

class Replace : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// any
	//
	typedef ::xsd::cxx::tree::element_optional AnyOptional;

	const AnyOptional &getAny() const;

	AnyOptional &getAny();

	void setAny(const ::xercesc::DOMElement &e);

	void setAny(::xercesc::DOMElement *p);

	void setAny(const AnyOptional &x);

	// sel
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Xpath SelType;
	typedef ::xsd::cxx::tree::traits<SelType, char> SelTraits;

	const SelType &getSel() const;

	SelType &getSel();

	void setSel(const SelType &x);

	void setSel(::std::unique_ptr<SelType> p);

	::std::unique_ptr<SelType> setDetachSel();

	// DOMDocument for wildcard content.
	//
	const ::xercesc::DOMDocument &getDomDocument() const;

	::xercesc::DOMDocument &getDomDocument();

	// Constructors.
	//
	Replace(const SelType &);

	Replace(const ::LinphonePrivate::Xsd::XmlSchema::Type &, const SelType &);

	Replace(const ::xercesc::DOMElement &e,
	        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Replace(const Replace &x,
	        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Replace *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Replace &operator=(const Replace &x);

	virtual ~Replace();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	AnyOptional any_;
	::xsd::cxx::tree::one<SelType> sel_;
};

class Ws : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	enum Value { before, after, both };

	Ws(Value v);

	Ws(const char *v);

	Ws(const ::std::string &v);

	Ws(const ::LinphonePrivate::Xsd::XmlSchema::String &v);

	Ws(const ::xercesc::DOMElement &e,
	   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Ws(const ::xercesc::DOMAttr &a,
	   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Ws(const ::std::string &s,
	   const ::xercesc::DOMElement *e,
	   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Ws(const Ws &x,
	   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

#ifdef XSD_CXX11
	Ws &operator=(const Ws &) = default;
#endif

	virtual Ws *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Ws &operator=(Value v);

	virtual operator Value() const {
		return _xsd_Ws_convert();
	}

protected:
	Value _xsd_Ws_convert() const;

public:
	static const char *const _xsd_Ws_literals_[3];
	static const Value _xsd_Ws_indexes_[3];
};

class Remove : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// sel
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Xpath SelType;
	typedef ::xsd::cxx::tree::traits<SelType, char> SelTraits;

	const SelType &getSel() const;

	SelType &getSel();

	void setSel(const SelType &x);

	void setSel(::std::unique_ptr<SelType> p);

	::std::unique_ptr<SelType> setDetachSel();

	// ws
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Ws WsType;
	typedef ::xsd::cxx::tree::optional<WsType> WsOptional;
	typedef ::xsd::cxx::tree::traits<WsType, char> WsTraits;

	const WsOptional &getWs() const;

	WsOptional &getWs();

	void setWs(const WsType &x);

	void setWs(const WsOptional &x);

	void setWs(::std::unique_ptr<WsType> p);

	// Constructors.
	//
	Remove(const SelType &);

	Remove(const ::xercesc::DOMElement &e,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Remove(const Remove &x,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Remove *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Remove &operator=(const Remove &x);

	virtual ~Remove();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<SelType> sel_;
	WsOptional ws_;
};
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
class ConferenceTimeType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// entry
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Entry EntryType;
	typedef ::xsd::cxx::tree::sequence<EntryType> EntrySequence;
	typedef EntrySequence::iterator EntryIterator;
	typedef EntrySequence::const_iterator EntryConstIterator;
	typedef ::xsd::cxx::tree::traits<EntryType, char> EntryTraits;

	const EntrySequence &getEntry() const;

	EntrySequence &getEntry();

	void setEntry(const EntrySequence &s);

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
	ConferenceTimeType();

	ConferenceTimeType(const ::xercesc::DOMElement &e,
	                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ConferenceTimeType(const ConferenceTimeType &x,
	                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ConferenceTimeType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ConferenceTimeType &operator=(const ConferenceTimeType &x);

	virtual ~ConferenceTimeType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	EntrySequence entry_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class TimeType : public ::LinphonePrivate::Xsd::XmlSchema::DateTime {
public:
	// Constructors.
	//
	TimeType(const ::LinphonePrivate::Xsd::XmlSchema::DateTime &);

	TimeType(const ::xercesc::DOMElement &e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	TimeType(const ::xercesc::DOMAttr &a,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	TimeType(const ::std::string &s,
	         const ::xercesc::DOMElement *e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	TimeType(const TimeType &x,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual TimeType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	TimeType &operator=(const TimeType &) = default;
#endif

	virtual ~TimeType();
};

class RoleType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	RoleType();

	RoleType(const char *);

	RoleType(const ::std::string &);

	RoleType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	RoleType(const ::xercesc::DOMElement &e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	RoleType(const ::xercesc::DOMAttr &a,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	RoleType(const ::std::string &s,
	         const ::xercesc::DOMElement *e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	RoleType(const RoleType &x,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual RoleType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	RoleType &operator=(const RoleType &) = default;
#endif

	virtual ~RoleType();
};

class MixingModeType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	MixingModeType();

	MixingModeType(const char *);

	MixingModeType(const ::std::string &);

	MixingModeType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	MixingModeType(const ::xercesc::DOMElement &e,
	               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixingModeType(const ::xercesc::DOMAttr &a,
	               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixingModeType(const ::std::string &s,
	               const ::xercesc::DOMElement *e,
	               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixingModeType(const MixingModeType &x,
	               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual MixingModeType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                               ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	MixingModeType &operator=(const MixingModeType &) = default;
#endif

	virtual ~MixingModeType();
};

class CodecsType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// codec
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::CodecType CodecType;
	typedef ::xsd::cxx::tree::traits<CodecType, char> CodecTraits;

	const CodecType &getCodec() const;

	CodecType &getCodec();

	void setCodec(const CodecType &x);

	void setCodec(::std::unique_ptr<CodecType> p);

	::std::unique_ptr<CodecType> setDetachCodec();

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// decision
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::DecisionType DecisionType;
	typedef ::xsd::cxx::tree::traits<DecisionType, char> DecisionTraits;

	const DecisionType &getDecision() const;

	DecisionType &getDecision();

	void setDecision(const DecisionType &x);

	void setDecision(::std::unique_ptr<DecisionType> p);

	::std::unique_ptr<DecisionType> setDetachDecision();

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
	CodecsType(const CodecType &, const DecisionType &);

	CodecsType(::std::unique_ptr<CodecType>, const DecisionType &);

	CodecsType(const ::xercesc::DOMElement &e,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CodecsType(const CodecsType &x,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CodecsType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CodecsType &operator=(const CodecsType &x);

	virtual ~CodecsType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<CodecType> codec_;
	AnySequence any_;
	::xsd::cxx::tree::one<DecisionType> decision_;
	AnyAttributeSet any_attribute_;
};

class CodecType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// subtype
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String SubtypeType;
	typedef ::xsd::cxx::tree::optional<SubtypeType> SubtypeOptional;
	typedef ::xsd::cxx::tree::traits<SubtypeType, char> SubtypeTraits;

	const SubtypeOptional &getSubtype() const;

	SubtypeOptional &getSubtype();

	void setSubtype(const SubtypeType &x);

	void setSubtype(const SubtypeOptional &x);

	void setSubtype(::std::unique_ptr<SubtypeType> p);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// name
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String NameType;
	typedef ::xsd::cxx::tree::traits<NameType, char> NameTraits;

	const NameType &getName() const;

	NameType &getName();

	void setName(const NameType &x);

	void setName(::std::unique_ptr<NameType> p);

	::std::unique_ptr<NameType> setDetachName();

	// policy
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::PolicyType PolicyType;
	typedef ::xsd::cxx::tree::traits<PolicyType, char> PolicyTraits;

	const PolicyType &getPolicy() const;

	PolicyType &getPolicy();

	void setPolicy(const PolicyType &x);

	void setPolicy(::std::unique_ptr<PolicyType> p);

	::std::unique_ptr<PolicyType> setDetachPolicy();

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
	CodecType(const NameType &, const PolicyType &);

	CodecType(const ::xercesc::DOMElement &e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CodecType(const CodecType &x,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CodecType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CodecType &operator=(const CodecType &x);

	virtual ~CodecType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	SubtypeOptional subtype_;
	AnySequence any_;
	::xsd::cxx::tree::one<NameType> name_;
	::xsd::cxx::tree::one<PolicyType> policy_;
	AnyAttributeSet any_attribute_;
};

class DecisionType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	DecisionType();

	DecisionType(const char *);

	DecisionType(const ::std::string &);

	DecisionType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	DecisionType(const ::xercesc::DOMElement &e,
	             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	DecisionType(const ::xercesc::DOMAttr &a,
	             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	DecisionType(const ::std::string &s,
	             const ::xercesc::DOMElement *e,
	             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	DecisionType(const DecisionType &x,
	             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual DecisionType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	DecisionType &operator=(const DecisionType &) = default;
#endif

	virtual ~DecisionType();
};

class PolicyType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	PolicyType();

	PolicyType(const char *);

	PolicyType(const ::std::string &);

	PolicyType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	PolicyType(const ::xercesc::DOMElement &e,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	PolicyType(const ::xercesc::DOMAttr &a,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	PolicyType(const ::std::string &s,
	           const ::xercesc::DOMElement *e,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	PolicyType(const PolicyType &x,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual PolicyType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	PolicyType &operator=(const PolicyType &) = default;
#endif

	virtual ~PolicyType();
};

class ControlsType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// mute
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Boolean MuteType;
	typedef ::xsd::cxx::tree::optional<MuteType> MuteOptional;
	typedef ::xsd::cxx::tree::traits<MuteType, char> MuteTraits;

	const MuteOptional &getMute() const;

	MuteOptional &getMute();

	void setMute(const MuteType &x);

	void setMute(const MuteOptional &x);

	// pause-video
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Boolean PauseVideoType;
	typedef ::xsd::cxx::tree::optional<PauseVideoType> PauseVideoOptional;
	typedef ::xsd::cxx::tree::traits<PauseVideoType, char> PauseVideoTraits;

	const PauseVideoOptional &getPauseVideo() const;

	PauseVideoOptional &getPauseVideo();

	void setPauseVideo(const PauseVideoType &x);

	void setPauseVideo(const PauseVideoOptional &x);

	// gain
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::GainType GainType;
	typedef ::xsd::cxx::tree::optional<GainType> GainOptional;
	typedef ::xsd::cxx::tree::traits<GainType, char> GainTraits;

	const GainOptional &getGain() const;

	GainOptional &getGain();

	void setGain(const GainType &x);

	void setGain(const GainOptional &x);

	void setGain(::std::unique_ptr<GainType> p);

	// video-layout
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::VideoLayoutType VideoLayoutType;
	typedef ::xsd::cxx::tree::optional<VideoLayoutType> VideoLayoutOptional;
	typedef ::xsd::cxx::tree::traits<VideoLayoutType, char> VideoLayoutTraits;

	const VideoLayoutOptional &getVideoLayout() const;

	VideoLayoutOptional &getVideoLayout();

	void setVideoLayout(const VideoLayoutType &x);

	void setVideoLayout(const VideoLayoutOptional &x);

	void setVideoLayout(::std::unique_ptr<VideoLayoutType> p);

	static const VideoLayoutType &getVideoLayoutDefaultValue();

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
	ControlsType();

	ControlsType(const ::xercesc::DOMElement &e,
	             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ControlsType(const ControlsType &x,
	             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ControlsType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                             ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ControlsType &operator=(const ControlsType &x);

	virtual ~ControlsType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	MuteOptional mute_;
	PauseVideoOptional pause_video_;
	GainOptional gain_;
	VideoLayoutOptional video_layout_;
	static const VideoLayoutType video_layout_default_value_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class GainType : public ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Integer,
                                                           char,
                                                           ::LinphonePrivate::Xsd::XmlSchema::SimpleType> {
public:
	// Constructors.
	//
	GainType(const ::LinphonePrivate::Xsd::XmlSchema::Integer &);

	GainType(const ::xercesc::DOMElement &e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	GainType(const ::xercesc::DOMAttr &a,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	GainType(const ::std::string &s,
	         const ::xercesc::DOMElement *e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	GainType(const GainType &x,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual GainType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	GainType &operator=(const GainType &) = default;
#endif

	virtual ~GainType();
};

class VideoLayoutType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	VideoLayoutType();

	VideoLayoutType(const char *);

	VideoLayoutType(const ::std::string &);

	VideoLayoutType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	VideoLayoutType(const ::xercesc::DOMElement &e,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	VideoLayoutType(const ::xercesc::DOMAttr &a,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	VideoLayoutType(const ::std::string &s,
	                const ::xercesc::DOMElement *e,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	VideoLayoutType(const VideoLayoutType &x,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual VideoLayoutType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	VideoLayoutType &operator=(const VideoLayoutType &) = default;
#endif

	virtual ~VideoLayoutType();
};

class FloorInformationType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// conference-ID
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::UnsignedLong ConferenceIDType;
	typedef ::xsd::cxx::tree::optional<ConferenceIDType> ConferenceIDOptional;
	typedef ::xsd::cxx::tree::traits<ConferenceIDType, char> ConferenceIDTraits;

	const ConferenceIDOptional &getConferenceID() const;

	ConferenceIDOptional &getConferenceID();

	void setConferenceID(const ConferenceIDType &x);

	void setConferenceID(const ConferenceIDOptional &x);

	// allow-floor-events
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Boolean AllowFloorEventsType;
	typedef ::xsd::cxx::tree::optional<AllowFloorEventsType> AllowFloorEventsOptional;
	typedef ::xsd::cxx::tree::traits<AllowFloorEventsType, char> AllowFloorEventsTraits;

	const AllowFloorEventsOptional &getAllowFloorEvents() const;

	AllowFloorEventsOptional &getAllowFloorEvents();

	void setAllowFloorEvents(const AllowFloorEventsType &x);

	void setAllowFloorEvents(const AllowFloorEventsOptional &x);

	static AllowFloorEventsType getAllowFloorEventsDefaultValue();

	// floor-request-handling
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::FloorRequestHandlingType FloorRequestHandlingType;
	typedef ::xsd::cxx::tree::optional<FloorRequestHandlingType> FloorRequestHandlingOptional;
	typedef ::xsd::cxx::tree::traits<FloorRequestHandlingType, char> FloorRequestHandlingTraits;

	const FloorRequestHandlingOptional &getFloorRequestHandling() const;

	FloorRequestHandlingOptional &getFloorRequestHandling();

	void setFloorRequestHandling(const FloorRequestHandlingType &x);

	void setFloorRequestHandling(const FloorRequestHandlingOptional &x);

	void setFloorRequestHandling(::std::unique_ptr<FloorRequestHandlingType> p);

	// conference-floor-policy
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceFloorPolicy ConferenceFloorPolicyType;
	typedef ::xsd::cxx::tree::optional<ConferenceFloorPolicyType> ConferenceFloorPolicyOptional;
	typedef ::xsd::cxx::tree::traits<ConferenceFloorPolicyType, char> ConferenceFloorPolicyTraits;

	const ConferenceFloorPolicyOptional &getConferenceFloorPolicy() const;

	ConferenceFloorPolicyOptional &getConferenceFloorPolicy();

	void setConferenceFloorPolicy(const ConferenceFloorPolicyType &x);

	void setConferenceFloorPolicy(const ConferenceFloorPolicyOptional &x);

	void setConferenceFloorPolicy(::std::unique_ptr<ConferenceFloorPolicyType> p);

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
	FloorInformationType();

	FloorInformationType(const ::xercesc::DOMElement &e,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	FloorInformationType(const FloorInformationType &x,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual FloorInformationType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	FloorInformationType &operator=(const FloorInformationType &x);

	virtual ~FloorInformationType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	ConferenceIDOptional conference_ID_;
	AllowFloorEventsOptional allow_floor_events_;
	FloorRequestHandlingOptional floor_request_handling_;
	ConferenceFloorPolicyOptional conference_floor_policy_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class FloorRequestHandlingType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	FloorRequestHandlingType();

	FloorRequestHandlingType(const char *);

	FloorRequestHandlingType(const ::std::string &);

	FloorRequestHandlingType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	FloorRequestHandlingType(const ::xercesc::DOMElement &e,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	FloorRequestHandlingType(const ::xercesc::DOMAttr &a,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	FloorRequestHandlingType(const ::std::string &s,
	                         const ::xercesc::DOMElement *e,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	FloorRequestHandlingType(const FloorRequestHandlingType &x,
	                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual FloorRequestHandlingType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	FloorRequestHandlingType &operator=(const FloorRequestHandlingType &) = default;
#endif

	virtual ~FloorRequestHandlingType();
};

class ConferenceFloorPolicy : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// floor
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Floor FloorType;
	typedef ::xsd::cxx::tree::sequence<FloorType> FloorSequence;
	typedef FloorSequence::iterator FloorIterator;
	typedef FloorSequence::const_iterator FloorConstIterator;
	typedef ::xsd::cxx::tree::traits<FloorType, char> FloorTraits;

	const FloorSequence &getFloor() const;

	FloorSequence &getFloor();

	void setFloor(const FloorSequence &s);

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
	ConferenceFloorPolicy();

	ConferenceFloorPolicy(const ::xercesc::DOMElement &e,
	                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ConferenceFloorPolicy(const ConferenceFloorPolicy &x,
	                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ConferenceFloorPolicy *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ConferenceFloorPolicy &operator=(const ConferenceFloorPolicy &x);

	virtual ~ConferenceFloorPolicy();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	FloorSequence floor_;
	AnyAttributeSet any_attribute_;
};

class AlgorithmType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	AlgorithmType();

	AlgorithmType(const char *);

	AlgorithmType(const ::std::string &);

	AlgorithmType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	AlgorithmType(const ::xercesc::DOMElement &e,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	AlgorithmType(const ::xercesc::DOMAttr &a,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	AlgorithmType(const ::std::string &s,
	              const ::xercesc::DOMElement *e,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	AlgorithmType(const AlgorithmType &x,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual AlgorithmType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	AlgorithmType &operator=(const AlgorithmType &) = default;
#endif

	virtual ~AlgorithmType();
};

class UserAdmissionPolicyType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	UserAdmissionPolicyType();

	UserAdmissionPolicyType(const char *);

	UserAdmissionPolicyType(const ::std::string &);

	UserAdmissionPolicyType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	UserAdmissionPolicyType(const ::xercesc::DOMElement &e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	UserAdmissionPolicyType(const ::xercesc::DOMAttr &a,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	UserAdmissionPolicyType(const ::std::string &s,
	                        const ::xercesc::DOMElement *e,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	UserAdmissionPolicyType(const UserAdmissionPolicyType &x,
	                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual UserAdmissionPolicyType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	UserAdmissionPolicyType &operator=(const UserAdmissionPolicyType &) = default;
#endif

	virtual ~UserAdmissionPolicyType();
};

class JoinHandlingType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	JoinHandlingType();

	JoinHandlingType(const char *);

	JoinHandlingType(const ::std::string &);

	JoinHandlingType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	JoinHandlingType(const ::xercesc::DOMElement &e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	JoinHandlingType(const ::xercesc::DOMAttr &a,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	JoinHandlingType(const ::std::string &s,
	                 const ::xercesc::DOMElement *e,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	JoinHandlingType(const JoinHandlingType &x,
	                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual JoinHandlingType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                 ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	JoinHandlingType &operator=(const JoinHandlingType &) = default;
#endif

	virtual ~JoinHandlingType();
};

class DenyUsersListType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// target
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Target TargetType;
	typedef ::xsd::cxx::tree::sequence<TargetType> TargetSequence;
	typedef TargetSequence::iterator TargetIterator;
	typedef TargetSequence::const_iterator TargetConstIterator;
	typedef ::xsd::cxx::tree::traits<TargetType, char> TargetTraits;

	const TargetSequence &getTarget() const;

	TargetSequence &getTarget();

	void setTarget(const TargetSequence &s);

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
	DenyUsersListType();

	DenyUsersListType(const ::xercesc::DOMElement &e,
	                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	DenyUsersListType(const DenyUsersListType &x,
	                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual DenyUsersListType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	DenyUsersListType &operator=(const DenyUsersListType &x);

	virtual ~DenyUsersListType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	TargetSequence target_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class AllowedUsersListType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// target
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::TargetType TargetType;
	typedef ::xsd::cxx::tree::sequence<TargetType> TargetSequence;
	typedef TargetSequence::iterator TargetIterator;
	typedef TargetSequence::const_iterator TargetConstIterator;
	typedef ::xsd::cxx::tree::traits<TargetType, char> TargetTraits;

	const TargetSequence &getTarget() const;

	TargetSequence &getTarget();

	void setTarget(const TargetSequence &s);

	// persistent-list
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::PersistentListType PersistentListType;
	typedef ::xsd::cxx::tree::optional<PersistentListType> PersistentListOptional;
	typedef ::xsd::cxx::tree::traits<PersistentListType, char> PersistentListTraits;

	const PersistentListOptional &getPersistentList() const;

	PersistentListOptional &getPersistentList();

	void setPersistentList(const PersistentListType &x);

	void setPersistentList(const PersistentListOptional &x);

	void setPersistentList(::std::unique_ptr<PersistentListType> p);

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
	AllowedUsersListType();

	AllowedUsersListType(const ::xercesc::DOMElement &e,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	AllowedUsersListType(const AllowedUsersListType &x,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual AllowedUsersListType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	AllowedUsersListType &operator=(const AllowedUsersListType &x);

	virtual ~AllowedUsersListType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	TargetSequence target_;
	PersistentListOptional persistent_list_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class PersistentListType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// user
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::User UserType;
	typedef ::xsd::cxx::tree::sequence<UserType> UserSequence;
	typedef UserSequence::iterator UserIterator;
	typedef UserSequence::const_iterator UserConstIterator;
	typedef ::xsd::cxx::tree::traits<UserType, char> UserTraits;

	const UserSequence &getUser() const;

	UserSequence &getUser();

	void setUser(const UserSequence &s);

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
	PersistentListType();

	PersistentListType(const ::xercesc::DOMElement &e,
	                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	PersistentListType(const PersistentListType &x,
	                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual PersistentListType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	PersistentListType &operator=(const PersistentListType &x);

	virtual ~PersistentListType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	UserSequence user_;
	AnySequence any_;
	AnyAttributeSet any_attribute_;
};

class TargetType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// uri
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Uri UriType;
	typedef ::xsd::cxx::tree::traits<UriType, char> UriTraits;

	const UriType &getUri() const;

	UriType &getUri();

	void setUri(const UriType &x);

	void setUri(::std::unique_ptr<UriType> p);

	::std::unique_ptr<UriType> setDetachUri();

	// method
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::MethodType MethodType;
	typedef ::xsd::cxx::tree::traits<MethodType, char> MethodTraits;

	const MethodType &getMethod() const;

	MethodType &getMethod();

	void setMethod(const MethodType &x);

	void setMethod(::std::unique_ptr<MethodType> p);

	::std::unique_ptr<MethodType> setDetachMethod();

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
	TargetType(const UriType &, const MethodType &);

	TargetType(const ::xercesc::DOMElement &e,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	TargetType(const TargetType &x,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual TargetType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	TargetType &operator=(const TargetType &x);

	virtual ~TargetType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<UriType> uri_;
	::xsd::cxx::tree::one<MethodType> method_;
	AnyAttributeSet any_attribute_;
};

class MethodType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	MethodType();

	MethodType(const char *);

	MethodType(const ::std::string &);

	MethodType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	MethodType(const ::xercesc::DOMElement &e,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MethodType(const ::xercesc::DOMAttr &a,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MethodType(const ::std::string &s,
	           const ::xercesc::DOMElement *e,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MethodType(const MethodType &x,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual MethodType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	MethodType &operator=(const MethodType &) = default;
#endif

	virtual ~MethodType();
};

class ProvideAnonymityType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	ProvideAnonymityType();

	ProvideAnonymityType(const char *);

	ProvideAnonymityType(const ::std::string &);

	ProvideAnonymityType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	ProvideAnonymityType(const ::xercesc::DOMElement &e,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ProvideAnonymityType(const ::xercesc::DOMAttr &a,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ProvideAnonymityType(const ::std::string &s,
	                     const ::xercesc::DOMElement *e,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ProvideAnonymityType(const ProvideAnonymityType &x,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ProvideAnonymityType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	ProvideAnonymityType &operator=(const ProvideAnonymityType &) = default;
#endif

	virtual ~ProvideAnonymityType();
};

class MixerType : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// floor
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Floor1 FloorType;
	typedef ::xsd::cxx::tree::traits<FloorType, char> FloorTraits;

	const FloorType &getFloor() const;

	FloorType &getFloor();

	void setFloor(const FloorType &x);

	void setFloor(::std::unique_ptr<FloorType> p);

	::std::unique_ptr<FloorType> setDetachFloor();

	// controls
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType ControlsType;
	typedef ::xsd::cxx::tree::sequence<ControlsType> ControlsSequence;
	typedef ControlsSequence::iterator ControlsIterator;
	typedef ControlsSequence::const_iterator ControlsConstIterator;
	typedef ::xsd::cxx::tree::traits<ControlsType, char> ControlsTraits;

	const ControlsSequence &getControls() const;

	ControlsSequence &getControls();

	void setControls(const ControlsSequence &s);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// name
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::MixerNameType NameType;
	typedef ::xsd::cxx::tree::traits<NameType, char> NameTraits;

	const NameType &getName() const;

	NameType &getName();

	void setName(const NameType &x);

	void setName(::std::unique_ptr<NameType> p);

	::std::unique_ptr<NameType> setDetachName();

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
	MixerType(const FloorType &, const NameType &);

	MixerType(::std::unique_ptr<FloorType>, const NameType &);

	MixerType(const ::xercesc::DOMElement &e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixerType(const MixerType &x,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual MixerType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	MixerType &operator=(const MixerType &x);

	virtual ~MixerType();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<FloorType> floor_;
	ControlsSequence controls_;
	AnySequence any_;
	::xsd::cxx::tree::one<NameType> name_;
	AnyAttributeSet any_attribute_;
};

class MixerNameType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	// Constructors.
	//
	MixerNameType();

	MixerNameType(const char *);

	MixerNameType(const ::std::string &);

	MixerNameType(const ::LinphonePrivate::Xsd::XmlSchema::String &);

	MixerNameType(const ::xercesc::DOMElement &e,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixerNameType(const ::xercesc::DOMAttr &a,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixerNameType(const ::std::string &s,
	              const ::xercesc::DOMElement *e,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixerNameType(const MixerNameType &x,
	              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual MixerNameType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                              ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	MixerNameType &operator=(const MixerNameType &) = default;
#endif

	virtual ~MixerNameType();
};

class ConferenceInfoDiff : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// add
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Add1 AddType;
	typedef ::xsd::cxx::tree::sequence<AddType> AddSequence;
	typedef AddSequence::iterator AddIterator;
	typedef AddSequence::const_iterator AddConstIterator;
	typedef ::xsd::cxx::tree::traits<AddType, char> AddTraits;

	const AddSequence &getAdd() const;

	AddSequence &getAdd();

	void setAdd(const AddSequence &s);

	// remove
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Remove1 RemoveType;
	typedef ::xsd::cxx::tree::sequence<RemoveType> RemoveSequence;
	typedef RemoveSequence::iterator RemoveIterator;
	typedef RemoveSequence::const_iterator RemoveConstIterator;
	typedef ::xsd::cxx::tree::traits<RemoveType, char> RemoveTraits;

	const RemoveSequence &getRemove() const;

	RemoveSequence &getRemove();

	void setRemove(const RemoveSequence &s);

	// replace
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::Replace1 ReplaceType;
	typedef ::xsd::cxx::tree::sequence<ReplaceType> ReplaceSequence;
	typedef ReplaceSequence::iterator ReplaceIterator;
	typedef ReplaceSequence::const_iterator ReplaceConstIterator;
	typedef ::xsd::cxx::tree::traits<ReplaceType, char> ReplaceTraits;

	const ReplaceSequence &getReplace() const;

	ReplaceSequence &getReplace();

	void setReplace(const ReplaceSequence &s);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// entity
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Uri EntityType;
	typedef ::xsd::cxx::tree::traits<EntityType, char> EntityTraits;

	const EntityType &getEntity() const;

	EntityType &getEntity();

	void setEntity(const EntityType &x);

	void setEntity(::std::unique_ptr<EntityType> p);

	::std::unique_ptr<EntityType> setDetachEntity();

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
	ConferenceInfoDiff(const EntityType &);

	ConferenceInfoDiff(const ::xercesc::DOMElement &e,
	                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ConferenceInfoDiff(const ConferenceInfoDiff &x,
	                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ConferenceInfoDiff *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ConferenceInfoDiff &operator=(const ConferenceInfoDiff &x);

	virtual ~ConferenceInfoDiff();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	AddSequence add_;
	RemoveSequence remove_;
	ReplaceSequence replace_;
	AnySequence any_;
	::xsd::cxx::tree::one<EntityType> entity_;
	AnyAttributeSet any_attribute_;
};

class Entry : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// base
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String BaseType;
	typedef ::xsd::cxx::tree::traits<BaseType, char> BaseTraits;

	const BaseType &getBase() const;

	BaseType &getBase();

	void setBase(const BaseType &x);

	void setBase(::std::unique_ptr<BaseType> p);

	::std::unique_ptr<BaseType> setDetachBase();

	// mixing-start-offset
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::MixingStartOffset MixingStartOffsetType;
	typedef ::xsd::cxx::tree::optional<MixingStartOffsetType> MixingStartOffsetOptional;
	typedef ::xsd::cxx::tree::traits<MixingStartOffsetType, char> MixingStartOffsetTraits;

	const MixingStartOffsetOptional &getMixingStartOffset() const;

	MixingStartOffsetOptional &getMixingStartOffset();

	void setMixingStartOffset(const MixingStartOffsetType &x);

	void setMixingStartOffset(const MixingStartOffsetOptional &x);

	void setMixingStartOffset(::std::unique_ptr<MixingStartOffsetType> p);

	// mixing-end-offset
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::MixingEndOffset MixingEndOffsetType;
	typedef ::xsd::cxx::tree::optional<MixingEndOffsetType> MixingEndOffsetOptional;
	typedef ::xsd::cxx::tree::traits<MixingEndOffsetType, char> MixingEndOffsetTraits;

	const MixingEndOffsetOptional &getMixingEndOffset() const;

	MixingEndOffsetOptional &getMixingEndOffset();

	void setMixingEndOffset(const MixingEndOffsetType &x);

	void setMixingEndOffset(const MixingEndOffsetOptional &x);

	void setMixingEndOffset(::std::unique_ptr<MixingEndOffsetType> p);

	// can-join-after-offset
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType CanJoinAfterOffsetType;
	typedef ::xsd::cxx::tree::optional<CanJoinAfterOffsetType> CanJoinAfterOffsetOptional;
	typedef ::xsd::cxx::tree::traits<CanJoinAfterOffsetType, char> CanJoinAfterOffsetTraits;

	const CanJoinAfterOffsetOptional &getCan_join_after_offset() const;

	CanJoinAfterOffsetOptional &getCan_join_after_offset();

	void setCan_join_after_offset(const CanJoinAfterOffsetType &x);

	void setCan_join_after_offset(const CanJoinAfterOffsetOptional &x);

	void setCan_join_after_offset(::std::unique_ptr<CanJoinAfterOffsetType> p);

	// must-join-before-offset
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType MustJoinBeforeOffsetType;
	typedef ::xsd::cxx::tree::optional<MustJoinBeforeOffsetType> MustJoinBeforeOffsetOptional;
	typedef ::xsd::cxx::tree::traits<MustJoinBeforeOffsetType, char> MustJoinBeforeOffsetTraits;

	const MustJoinBeforeOffsetOptional &getMust_join_before_offset() const;

	MustJoinBeforeOffsetOptional &getMust_join_before_offset();

	void setMust_join_before_offset(const MustJoinBeforeOffsetType &x);

	void setMust_join_before_offset(const MustJoinBeforeOffsetOptional &x);

	void setMust_join_before_offset(::std::unique_ptr<MustJoinBeforeOffsetType> p);

	// request-user
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType RequestUserType;
	typedef ::xsd::cxx::tree::optional<RequestUserType> RequestUserOptional;
	typedef ::xsd::cxx::tree::traits<RequestUserType, char> RequestUserTraits;

	const RequestUserOptional &getRequestUser() const;

	RequestUserOptional &getRequestUser();

	void setRequestUser(const RequestUserType &x);

	void setRequestUser(const RequestUserOptional &x);

	void setRequestUser(::std::unique_ptr<RequestUserType> p);

	// notify-end-of-conference
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::NonNegativeInteger NotifyEndOfConferenceType;
	typedef ::xsd::cxx::tree::optional<NotifyEndOfConferenceType> NotifyEndOfConferenceOptional;
	typedef ::xsd::cxx::tree::traits<NotifyEndOfConferenceType, char> NotifyEndOfConferenceTraits;

	const NotifyEndOfConferenceOptional &getNotify_end_of_conference() const;

	NotifyEndOfConferenceOptional &getNotify_end_of_conference();

	void setNotify_end_of_conference(const NotifyEndOfConferenceType &x);

	void setNotify_end_of_conference(const NotifyEndOfConferenceOptional &x);

	// allowed-extend-mixing-end-offset
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Boolean Allowed_extend_mixing_end_offsetType;
	typedef ::xsd::cxx::tree::optional<Allowed_extend_mixing_end_offsetType> Allowed_extend_mixing_end_offsetOptional;
	typedef ::xsd::cxx::tree::traits<Allowed_extend_mixing_end_offsetType, char> Allowed_extend_mixing_end_offsetTraits;

	const Allowed_extend_mixing_end_offsetOptional &getAllowed_extend_mixing_end_offset() const;

	Allowed_extend_mixing_end_offsetOptional &getAllowed_extend_mixing_end_offset();

	void setAllowed_extend_mixing_end_offset(const Allowed_extend_mixing_end_offsetType &x);

	void setAllowed_extend_mixing_end_offset(const Allowed_extend_mixing_end_offsetOptional &x);

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
	Entry(const BaseType &);

	Entry(::std::unique_ptr<BaseType>);

	Entry(const ::xercesc::DOMElement &e,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Entry(const Entry &x,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Entry *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Entry &operator=(const Entry &x);

	virtual ~Entry();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<BaseType> base_;
	MixingStartOffsetOptional mixing_start_offset_;
	MixingEndOffsetOptional mixing_end_offset_;
	CanJoinAfterOffsetOptional can_join_after_offset_;
	MustJoinBeforeOffsetOptional must_join_before_offset_;
	RequestUserOptional request_user_;
	NotifyEndOfConferenceOptional notify_end_of_conference_;
	Allowed_extend_mixing_end_offsetOptional allowed_extend_mixing_end_offset_;
	AnySequence any_;
};

class Floor : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// media-label
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String MediaLabelType;
	typedef ::xsd::cxx::tree::sequence<MediaLabelType> MediaLabelSequence;
	typedef MediaLabelSequence::iterator MediaLabelIterator;
	typedef MediaLabelSequence::const_iterator MediaLabelConstIterator;
	typedef ::xsd::cxx::tree::traits<MediaLabelType, char> MediaLabelTraits;

	const MediaLabelSequence &getMediaLabel() const;

	MediaLabelSequence &getMediaLabel();

	void setMediaLabel(const MediaLabelSequence &s);

	// algorithm
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::AlgorithmType AlgorithmType;
	typedef ::xsd::cxx::tree::optional<AlgorithmType> AlgorithmOptional;
	typedef ::xsd::cxx::tree::traits<AlgorithmType, char> AlgorithmTraits;

	const AlgorithmOptional &getAlgorithm() const;

	AlgorithmOptional &getAlgorithm();

	void setAlgorithm(const AlgorithmType &x);

	void setAlgorithm(const AlgorithmOptional &x);

	void setAlgorithm(::std::unique_ptr<AlgorithmType> p);

	// max-floor-users
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::NonNegativeInteger MaxFloorUsersType;
	typedef ::xsd::cxx::tree::optional<MaxFloorUsersType> MaxFloorUsersOptional;
	typedef ::xsd::cxx::tree::traits<MaxFloorUsersType, char> MaxFloorUsersTraits;

	const MaxFloorUsersOptional &getMaxFloorUsers() const;

	MaxFloorUsersOptional &getMaxFloorUsers();

	void setMaxFloorUsers(const MaxFloorUsersType &x);

	void setMaxFloorUsers(const MaxFloorUsersOptional &x);

	// moderator-id
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::NonNegativeInteger ModeratorIdType;
	typedef ::xsd::cxx::tree::optional<ModeratorIdType> ModeratorIdOptional;
	typedef ::xsd::cxx::tree::traits<ModeratorIdType, char> ModeratorIdTraits;

	const ModeratorIdOptional &getModeratorId() const;

	ModeratorIdOptional &getModeratorId();

	void setModeratorId(const ModeratorIdType &x);

	void setModeratorId(const ModeratorIdOptional &x);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// id
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String IdType;
	typedef ::xsd::cxx::tree::traits<IdType, char> IdTraits;

	const IdType &getId() const;

	IdType &getId();

	void setId(const IdType &x);

	void setId(::std::unique_ptr<IdType> p);

	::std::unique_ptr<IdType> setDetachId();

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
	Floor(const IdType &);

	Floor(const ::xercesc::DOMElement &e,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Floor(const Floor &x,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Floor *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Floor &operator=(const Floor &x);

	virtual ~Floor();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	MediaLabelSequence media_label_;
	AlgorithmOptional algorithm_;
	MaxFloorUsersOptional max_floor_users_;
	ModeratorIdOptional moderator_id_;
	AnySequence any_;
	::xsd::cxx::tree::one<IdType> id_;
	AnyAttributeSet any_attribute_;
};

class Target : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// uri
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Uri UriType;
	typedef ::xsd::cxx::tree::traits<UriType, char> UriTraits;

	const UriType &getUri() const;

	UriType &getUri();

	void setUri(const UriType &x);

	void setUri(::std::unique_ptr<UriType> p);

	::std::unique_ptr<UriType> setDetachUri();

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
	Target(const UriType &);

	Target(const ::xercesc::DOMElement &e,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Target(const Target &x,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Target *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Target &operator=(const Target &x);

	virtual ~Target();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<UriType> uri_;
	AnyAttributeSet any_attribute_;
};

class User : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// email
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String EmailType;
	typedef ::xsd::cxx::tree::sequence<EmailType> EmailSequence;
	typedef EmailSequence::iterator EmailIterator;
	typedef EmailSequence::const_iterator EmailConstIterator;
	typedef ::xsd::cxx::tree::traits<EmailType, char> EmailTraits;

	const EmailSequence &getEmail() const;

	EmailSequence &getEmail();

	void setEmail(const EmailSequence &s);

	// any
	//
	typedef ::xsd::cxx::tree::element_sequence AnySequence;
	typedef AnySequence::iterator AnyIterator;
	typedef AnySequence::const_iterator AnyConstIterator;

	const AnySequence &getAny() const;

	AnySequence &getAny();

	void setAny(const AnySequence &s);

	// name
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Uri NameType;
	typedef ::xsd::cxx::tree::traits<NameType, char> NameTraits;

	const NameType &getName() const;

	NameType &getName();

	void setName(const NameType &x);

	void setName(::std::unique_ptr<NameType> p);

	::std::unique_ptr<NameType> setDetachName();

	// nickname
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String NicknameType;
	typedef ::xsd::cxx::tree::traits<NicknameType, char> NicknameTraits;

	const NicknameType &getNickname() const;

	NicknameType &getNickname();

	void setNickname(const NicknameType &x);

	void setNickname(::std::unique_ptr<NicknameType> p);

	::std::unique_ptr<NicknameType> setDetachNickname();

	// id
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String IdType;
	typedef ::xsd::cxx::tree::traits<IdType, char> IdTraits;

	const IdType &getId() const;

	IdType &getId();

	void setId(const IdType &x);

	void setId(::std::unique_ptr<IdType> p);

	::std::unique_ptr<IdType> setDetachId();

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
	User(const NameType &, const NicknameType &, const IdType &);

	User(const ::xercesc::DOMElement &e,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	User(const User &x,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual User *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	User &operator=(const User &x);

	virtual ~User();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	EmailSequence email_;
	AnySequence any_;
	::xsd::cxx::tree::one<NameType> name_;
	::xsd::cxx::tree::one<NicknameType> nickname_;
	::xsd::cxx::tree::one<IdType> id_;
	AnyAttributeSet any_attribute_;
};

class Floor1 : public ::xsd::cxx::tree::fundamental_base<::LinphonePrivate::Xsd::XmlSchema::Boolean,
                                                         char,
                                                         ::LinphonePrivate::Xsd::XmlSchema::SimpleType> {
public:
	// id
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String IdType;
	typedef ::xsd::cxx::tree::traits<IdType, char> IdTraits;

	const IdType &getId() const;

	IdType &getId();

	void setId(const IdType &x);

	void setId(::std::unique_ptr<IdType> p);

	::std::unique_ptr<IdType> setDetachId();

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
	Floor1(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &, const IdType &);

	Floor1(const ::xercesc::DOMElement &e,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Floor1(const Floor1 &x,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Floor1 *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Floor1 &operator=(const Floor1 &x);

	virtual ~Floor1();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<IdType> id_;
	AnyAttributeSet any_attribute_;
};

class Add1 : public ::LinphonePrivate::Xsd::XconConferenceInfo::Add {
public:
	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// Constructors.
	//
	Add1(const SelType &);

	Add1(const ::LinphonePrivate::Xsd::XmlSchema::Type &, const SelType &);

	Add1(const ::xercesc::DOMElement &e,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Add1(const Add1 &x,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Add1 *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Add1 &operator=(const Add1 &x);

	virtual ~Add1();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	AnyAttributeSet any_attribute_;
};

class Remove1 : public ::LinphonePrivate::Xsd::XconConferenceInfo::Remove {
public:
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
	Remove1(const SelType &);

	Remove1(const ::xercesc::DOMElement &e,
	        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Remove1(const Remove1 &x,
	        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Remove1 *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Remove1 &operator=(const Remove1 &x);

	virtual ~Remove1();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	AnyAttributeSet any_attribute_;
};

class Replace1 : public ::LinphonePrivate::Xsd::XconConferenceInfo::Replace {
public:
	// any_attribute
	//
	typedef ::xsd::cxx::tree::attribute_set<char> AnyAttributeSet;
	typedef AnyAttributeSet::iterator AnyAttributeIterator;
	typedef AnyAttributeSet::const_iterator AnyAttributeConstIterator;

	const AnyAttributeSet &getAnyAttribute() const;

	AnyAttributeSet &getAnyAttribute();

	void setAnyAttribute(const AnyAttributeSet &s);

	// Constructors.
	//
	Replace1(const SelType &);

	Replace1(const ::LinphonePrivate::Xsd::XmlSchema::Type &, const SelType &);

	Replace1(const ::xercesc::DOMElement &e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Replace1(const Replace1 &x,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Replace1 *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Replace1 &operator=(const Replace1 &x);

	virtual ~Replace1();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	AnyAttributeSet any_attribute_;
};

class MixingStartOffset : public ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType {
public:
	// required-participant
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::RoleType RequiredParticipantType;
	typedef ::xsd::cxx::tree::traits<RequiredParticipantType, char> RequiredParticipantTraits;

	const RequiredParticipantType &getRequiredParticipant() const;

	RequiredParticipantType &getRequiredParticipant();

	void setRequiredParticipant(const RequiredParticipantType &x);

	void setRequiredParticipant(::std::unique_ptr<RequiredParticipantType> p);

	::std::unique_ptr<RequiredParticipantType> setDetachRequired_participant();

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
	MixingStartOffset(const ::LinphonePrivate::Xsd::XmlSchema::DateTime &, const RequiredParticipantType &);

	MixingStartOffset(const ::xercesc::DOMElement &e,
	                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixingStartOffset(const MixingStartOffset &x,
	                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual MixingStartOffset *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                  ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	MixingStartOffset &operator=(const MixingStartOffset &x);

	virtual ~MixingStartOffset();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<RequiredParticipantType> required_participant_;
	AnyAttributeSet any_attribute_;
};

class MixingEndOffset : public ::LinphonePrivate::Xsd::XconConferenceInfo::TimeType {
public:
	// required-participant
	//
	typedef ::LinphonePrivate::Xsd::XconConferenceInfo::RoleType RequiredParticipantType;
	typedef ::xsd::cxx::tree::traits<RequiredParticipantType, char> RequiredParticipantTraits;

	const RequiredParticipantType &getRequiredParticipant() const;

	RequiredParticipantType &getRequiredParticipant();

	void setRequiredParticipant(const RequiredParticipantType &x);

	void setRequiredParticipant(::std::unique_ptr<RequiredParticipantType> p);

	::std::unique_ptr<RequiredParticipantType> setDetachRequired_participant();

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
	MixingEndOffset(const ::LinphonePrivate::Xsd::XmlSchema::DateTime &, const RequiredParticipantType &);

	MixingEndOffset(const ::xercesc::DOMElement &e,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	MixingEndOffset(const MixingEndOffset &x,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual MixingEndOffset *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	MixingEndOffset &operator=(const MixingEndOffset &x);

	virtual ~MixingEndOffset();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<RequiredParticipantType> required_participant_;
	AnyAttributeSet any_attribute_;
};
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
::std::ostream &operator<<(::std::ostream &, const Xpath &);

::std::ostream &operator<<(::std::ostream &, const XpathAdd &);

::std::ostream &operator<<(::std::ostream &, Pos::Value);

::std::ostream &operator<<(::std::ostream &, const Pos &);

::std::ostream &operator<<(::std::ostream &, const Type &);

::std::ostream &operator<<(::std::ostream &, const Add &);

::std::ostream &operator<<(::std::ostream &, const Replace &);

::std::ostream &operator<<(::std::ostream &, Ws::Value);

::std::ostream &operator<<(::std::ostream &, const Ws &);

::std::ostream &operator<<(::std::ostream &, const Remove &);
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
::std::ostream &operator<<(::std::ostream &, const ConferenceTimeType &);

::std::ostream &operator<<(::std::ostream &, const TimeType &);

::std::ostream &operator<<(::std::ostream &, const RoleType &);

::std::ostream &operator<<(::std::ostream &, const MixingModeType &);

::std::ostream &operator<<(::std::ostream &, const CodecsType &);

::std::ostream &operator<<(::std::ostream &, const CodecType &);

::std::ostream &operator<<(::std::ostream &, const DecisionType &);

::std::ostream &operator<<(::std::ostream &, const PolicyType &);

::std::ostream &operator<<(::std::ostream &, const ControlsType &);

::std::ostream &operator<<(::std::ostream &, const GainType &);

::std::ostream &operator<<(::std::ostream &, const VideoLayoutType &);

::std::ostream &operator<<(::std::ostream &, const FloorInformationType &);

::std::ostream &operator<<(::std::ostream &, const FloorRequestHandlingType &);

::std::ostream &operator<<(::std::ostream &, const ConferenceFloorPolicy &);

::std::ostream &operator<<(::std::ostream &, const AlgorithmType &);

::std::ostream &operator<<(::std::ostream &, const UserAdmissionPolicyType &);

::std::ostream &operator<<(::std::ostream &, const JoinHandlingType &);

::std::ostream &operator<<(::std::ostream &, const DenyUsersListType &);

::std::ostream &operator<<(::std::ostream &, const AllowedUsersListType &);

::std::ostream &operator<<(::std::ostream &, const PersistentListType &);

::std::ostream &operator<<(::std::ostream &, const TargetType &);

::std::ostream &operator<<(::std::ostream &, const MethodType &);

::std::ostream &operator<<(::std::ostream &, const ProvideAnonymityType &);

::std::ostream &operator<<(::std::ostream &, const MixerType &);

::std::ostream &operator<<(::std::ostream &, const MixerNameType &);

::std::ostream &operator<<(::std::ostream &, const ConferenceInfoDiff &);

::std::ostream &operator<<(::std::ostream &, const Entry &);

::std::ostream &operator<<(::std::ostream &, const Floor &);

::std::ostream &operator<<(::std::ostream &, const Target &);

::std::ostream &operator<<(::std::ostream &, const User &);

::std::ostream &operator<<(::std::ostream &, const Floor1 &);

::std::ostream &operator<<(::std::ostream &, const Add1 &);

::std::ostream &operator<<(::std::ostream &, const Remove1 &);

::std::ostream &operator<<(::std::ostream &, const Replace1 &);

::std::ostream &operator<<(::std::ostream &, const MixingStartOffset &);

::std::ostream &operator<<(::std::ostream &, const MixingEndOffset &);
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/sax/InputSource.hpp>

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {}
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff> parseConferenceInfoDiff(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType> parseMixingMode(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(const ::std::string &uri,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(const ::std::string &uri,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(const ::std::string &uri,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            const ::std::string &id,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            const ::std::string &id,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::std::istream &is,
            const ::std::string &id,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::xercesc::InputSource &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::xercesc::InputSource &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::xercesc::InputSource &is,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(const ::xercesc::DOMDocument &d,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType>
parseCodecs(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String> parseConferencePassword(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(const ::std::string &uri,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(const ::std::string &uri,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(const ::std::string &uri,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              const ::std::string &id,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              const ::std::string &id,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::std::istream &is,
              const ::std::string &id,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::xercesc::InputSource &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::xercesc::InputSource &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::xercesc::InputSource &is,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(const ::xercesc::DOMDocument &d,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType>
parseControls(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(const ::std::string &uri,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(const ::std::string &uri,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(const ::std::string &uri,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              const ::std::string &id,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              const ::std::string &id,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::std::istream &is,
              const ::std::string &id,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::xercesc::InputSource &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::xercesc::InputSource &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::xercesc::InputSource &is,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(const ::xercesc::DOMDocument &d,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Language>
parseLanguage(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowSidebars(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseCloningParent(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseSidebarParent(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType> parseConferenceTime(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowConferenceEventSubscription(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(const ::std::string &uri,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(const ::std::string &uri,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(const ::std::string &uri,
             ::xercesc::DOMErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             ::xercesc::DOMErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             const ::std::string &id,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             const ::std::string &id,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::std::istream &is,
             const ::std::string &id,
             ::xercesc::DOMErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::xercesc::InputSource &is,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::xercesc::InputSource &is,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::xercesc::InputSource &is,
             ::xercesc::DOMErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(const ::xercesc::DOMDocument &d,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType>
parseToMixer(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType> parseProvideAnonymity(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowReferUsersDynamically(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowInviteUsersDynamically(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Boolean> parseAllowRemoveUsersDynamically(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::MixerType> parseFromMixer(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType> parseJoinHandling(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType> parseUserAdmissionPolicy(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType> parseAllowedUsersList(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType> parseDenyUsersList(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType> parseFloorInformation(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
void operator<<(::xercesc::DOMElement &, const Xpath &);

void operator<<(::xercesc::DOMAttr &, const Xpath &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Xpath &);

void operator<<(::xercesc::DOMElement &, const XpathAdd &);

void operator<<(::xercesc::DOMAttr &, const XpathAdd &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const XpathAdd &);

void operator<<(::xercesc::DOMElement &, const Pos &);

void operator<<(::xercesc::DOMAttr &, const Pos &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Pos &);

void operator<<(::xercesc::DOMElement &, const Type &);

void operator<<(::xercesc::DOMAttr &, const Type &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Type &);

void operator<<(::xercesc::DOMElement &, const Add &);

void operator<<(::xercesc::DOMElement &, const Replace &);

void operator<<(::xercesc::DOMElement &, const Ws &);

void operator<<(::xercesc::DOMAttr &, const Ws &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Ws &);

void operator<<(::xercesc::DOMElement &, const Remove &);
} // namespace XconConferenceInfo
} // namespace Xsd
} // namespace LinphonePrivate

namespace LinphonePrivate {
namespace Xsd {
namespace XconConferenceInfo {
// Serialize to std::ostream.
//

void serializeConferenceInfoDiff(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &x,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceInfoDiff(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceInfoDiff(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &x,
                                 ::xercesc::DOMErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeConferenceInfoDiff(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &x,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceInfoDiff(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceInfoDiff(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &x,
                                 ::xercesc::DOMErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeConferenceInfoDiff(::xercesc::DOMDocument &d,
                                 const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConferenceInfoDiff(const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceInfoDiff &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeMixingMode(::std::ostream &os,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &x,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeMixingMode(::std::ostream &os,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &x,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeMixingMode(::std::ostream &os,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &x,
                         ::xercesc::DOMErrorHandler &eh,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeMixingMode(::xercesc::XMLFormatTarget &ft,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &x,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeMixingMode(::xercesc::XMLFormatTarget &ft,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &x,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeMixingMode(::xercesc::XMLFormatTarget &ft,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &x,
                         ::xercesc::DOMErrorHandler &eh,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeMixingMode(::xercesc::DOMDocument &d,
                         const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &x,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeMixingMode(const ::LinphonePrivate::Xsd::XconConferenceInfo::MixingModeType &x,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeCodecs(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCodecs(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &x,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCodecs(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &x,
                     ::xercesc::DOMErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeCodecs(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCodecs(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &x,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCodecs(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &x,
                     ::xercesc::DOMErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeCodecs(::xercesc::DOMDocument &d,
                     const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCodecs(const ::LinphonePrivate::Xsd::XconConferenceInfo::CodecsType &x,
                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeConferencePassword(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferencePassword(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferencePassword(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                                 ::xercesc::DOMErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeConferencePassword(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferencePassword(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferencePassword(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                                 ::xercesc::DOMErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeConferencePassword(::xercesc::DOMDocument &d,
                                 const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConferencePassword(const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeControls(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeControls(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &x,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeControls(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &x,
                       ::xercesc::DOMErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeControls(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeControls(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &x,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeControls(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &x,
                       ::xercesc::DOMErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeControls(::xercesc::DOMDocument &d,
                       const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &x,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeControls(const ::LinphonePrivate::Xsd::XconConferenceInfo::ControlsType &x,
                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeLanguage(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeLanguage(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &x,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeLanguage(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &x,
                       ::xercesc::DOMErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeLanguage(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeLanguage(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &x,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeLanguage(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &x,
                       ::xercesc::DOMErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeLanguage(::xercesc::DOMDocument &d,
                       const ::LinphonePrivate::Xsd::XmlSchema::Language &x,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeLanguage(const ::LinphonePrivate::Xsd::XmlSchema::Language &x,
                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeAllowSidebars(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowSidebars(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowSidebars(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeAllowSidebars(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowSidebars(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowSidebars(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeAllowSidebars(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowSidebars(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeCloningParent(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCloningParent(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCloningParent(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeCloningParent(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCloningParent(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCloningParent(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeCloningParent(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCloningParent(const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeSidebarParent(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarParent(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarParent(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSidebarParent(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarParent(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSidebarParent(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSidebarParent(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSidebarParent(const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeConferenceTime(::std::ostream &os,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &x,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             const ::std::string &e = "UTF-8",
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceTime(::std::ostream &os,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &x,
                             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             const ::std::string &e = "UTF-8",
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceTime(::std::ostream &os,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &x,
                             ::xercesc::DOMErrorHandler &eh,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             const ::std::string &e = "UTF-8",
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeConferenceTime(::xercesc::XMLFormatTarget &ft,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &x,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             const ::std::string &e = "UTF-8",
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceTime(::xercesc::XMLFormatTarget &ft,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &x,
                             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             const ::std::string &e = "UTF-8",
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceTime(::xercesc::XMLFormatTarget &ft,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &x,
                             ::xercesc::DOMErrorHandler &eh,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             const ::std::string &e = "UTF-8",
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeConferenceTime(::xercesc::DOMDocument &d,
                             const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &x,
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConferenceTime(const ::LinphonePrivate::Xsd::XconConferenceInfo::ConferenceTimeType &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeAllowConferenceEventSubscription(::std::ostream &os,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                               const ::std::string &e = "UTF-8",
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowConferenceEventSubscription(::std::ostream &os,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                               const ::std::string &e = "UTF-8",
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowConferenceEventSubscription(::std::ostream &os,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                               ::xercesc::DOMErrorHandler &eh,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                               const ::std::string &e = "UTF-8",
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeAllowConferenceEventSubscription(::xercesc::XMLFormatTarget &ft,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                               const ::std::string &e = "UTF-8",
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowConferenceEventSubscription(::xercesc::XMLFormatTarget &ft,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                               const ::std::string &e = "UTF-8",
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowConferenceEventSubscription(::xercesc::XMLFormatTarget &ft,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                               ::xercesc::DOMErrorHandler &eh,
                                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                               const ::std::string &e = "UTF-8",
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeAllowConferenceEventSubscription(::xercesc::DOMDocument &d,
                                               const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowConferenceEventSubscription(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeToMixer(::std::ostream &os,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeToMixer(::std::ostream &os,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeToMixer(::std::ostream &os,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                      ::xercesc::DOMErrorHandler &eh,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeToMixer(::xercesc::XMLFormatTarget &ft,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeToMixer(::xercesc::XMLFormatTarget &ft,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeToMixer(::xercesc::XMLFormatTarget &ft,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                      ::xercesc::DOMErrorHandler &eh,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeToMixer(::xercesc::DOMDocument &d,
                      const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeToMixer(const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeProvideAnonymity(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProvideAnonymity(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProvideAnonymity(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeProvideAnonymity(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProvideAnonymity(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProvideAnonymity(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeProvideAnonymity(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeProvideAnonymity(const ::LinphonePrivate::Xsd::XconConferenceInfo::ProvideAnonymityType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeAllowReferUsersDynamically(::std::ostream &os,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                         const ::std::string &e = "UTF-8",
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowReferUsersDynamically(::std::ostream &os,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                         const ::std::string &e = "UTF-8",
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowReferUsersDynamically(::std::ostream &os,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                         ::xercesc::DOMErrorHandler &eh,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                         const ::std::string &e = "UTF-8",
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeAllowReferUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                         const ::std::string &e = "UTF-8",
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowReferUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                         const ::std::string &e = "UTF-8",
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowReferUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                         ::xercesc::DOMErrorHandler &eh,
                                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                         const ::std::string &e = "UTF-8",
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeAllowReferUsersDynamically(::xercesc::DOMDocument &d,
                                         const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowReferUsersDynamically(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeAllowInviteUsersDynamically(::std::ostream &os,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowInviteUsersDynamically(::std::ostream &os,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowInviteUsersDynamically(::std::ostream &os,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::xercesc::DOMErrorHandler &eh,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeAllowInviteUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowInviteUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowInviteUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::xercesc::DOMErrorHandler &eh,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeAllowInviteUsersDynamically(::xercesc::DOMDocument &d,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowInviteUsersDynamically(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeAllowRemoveUsersDynamically(::std::ostream &os,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowRemoveUsersDynamically(::std::ostream &os,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowRemoveUsersDynamically(::std::ostream &os,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::xercesc::DOMErrorHandler &eh,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeAllowRemoveUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowRemoveUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowRemoveUsersDynamically(::xercesc::XMLFormatTarget &ft,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::xercesc::DOMErrorHandler &eh,
                                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                          const ::std::string &e = "UTF-8",
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeAllowRemoveUsersDynamically(::xercesc::DOMDocument &d,
                                          const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowRemoveUsersDynamically(const ::LinphonePrivate::Xsd::XmlSchema::Boolean &x,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeFromMixer(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFromMixer(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFromMixer(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeFromMixer(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFromMixer(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFromMixer(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeFromMixer(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeFromMixer(const ::LinphonePrivate::Xsd::XconConferenceInfo::MixerType &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeJoinHandling(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeJoinHandling(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeJoinHandling(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeJoinHandling(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeJoinHandling(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeJoinHandling(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeJoinHandling(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeJoinHandling(const ::LinphonePrivate::Xsd::XconConferenceInfo::JoinHandlingType &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeUserAdmissionPolicy(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserAdmissionPolicy(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserAdmissionPolicy(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeUserAdmissionPolicy(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserAdmissionPolicy(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeUserAdmissionPolicy(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeUserAdmissionPolicy(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeUserAdmissionPolicy(const ::LinphonePrivate::Xsd::XconConferenceInfo::UserAdmissionPolicyType &x,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeAllowedUsersList(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowedUsersList(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowedUsersList(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeAllowedUsersList(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowedUsersList(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeAllowedUsersList(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeAllowedUsersList(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeAllowedUsersList(const ::LinphonePrivate::Xsd::XconConferenceInfo::AllowedUsersListType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeDenyUsersList(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDenyUsersList(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDenyUsersList(::std::ostream &os,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeDenyUsersList(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDenyUsersList(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDenyUsersList(::xercesc::XMLFormatTarget &ft,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &x,
                            ::xercesc::DOMErrorHandler &eh,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            const ::std::string &e = "UTF-8",
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeDenyUsersList(::xercesc::DOMDocument &d,
                            const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &x,
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDenyUsersList(const ::LinphonePrivate::Xsd::XconConferenceInfo::DenyUsersListType &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeFloorInformation(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFloorInformation(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFloorInformation(::std::ostream &os,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeFloorInformation(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &x,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFloorInformation(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFloorInformation(::xercesc::XMLFormatTarget &ft,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &x,
                               ::xercesc::DOMErrorHandler &eh,
                               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                               const ::std::string &e = "UTF-8",
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeFloorInformation(::xercesc::DOMDocument &d,
                               const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &x,
                               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeFloorInformation(const ::LinphonePrivate::Xsd::XconConferenceInfo::FloorInformationType &x,
                          const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                              ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const ConferenceTimeType &);

void operator<<(::xercesc::DOMElement &, const TimeType &);

void operator<<(::xercesc::DOMAttr &, const TimeType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const TimeType &);

void operator<<(::xercesc::DOMElement &, const RoleType &);

void operator<<(::xercesc::DOMAttr &, const RoleType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const RoleType &);

void operator<<(::xercesc::DOMElement &, const MixingModeType &);

void operator<<(::xercesc::DOMAttr &, const MixingModeType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const MixingModeType &);

void operator<<(::xercesc::DOMElement &, const CodecsType &);

void operator<<(::xercesc::DOMElement &, const CodecType &);

void operator<<(::xercesc::DOMElement &, const DecisionType &);

void operator<<(::xercesc::DOMAttr &, const DecisionType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const DecisionType &);

void operator<<(::xercesc::DOMElement &, const PolicyType &);

void operator<<(::xercesc::DOMAttr &, const PolicyType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const PolicyType &);

void operator<<(::xercesc::DOMElement &, const ControlsType &);

void operator<<(::xercesc::DOMElement &, const GainType &);

void operator<<(::xercesc::DOMAttr &, const GainType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const GainType &);

void operator<<(::xercesc::DOMElement &, const VideoLayoutType &);

void operator<<(::xercesc::DOMAttr &, const VideoLayoutType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const VideoLayoutType &);

void operator<<(::xercesc::DOMElement &, const FloorInformationType &);

void operator<<(::xercesc::DOMElement &, const FloorRequestHandlingType &);

void operator<<(::xercesc::DOMAttr &, const FloorRequestHandlingType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const FloorRequestHandlingType &);

void operator<<(::xercesc::DOMElement &, const ConferenceFloorPolicy &);

void operator<<(::xercesc::DOMElement &, const AlgorithmType &);

void operator<<(::xercesc::DOMAttr &, const AlgorithmType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const AlgorithmType &);

void operator<<(::xercesc::DOMElement &, const UserAdmissionPolicyType &);

void operator<<(::xercesc::DOMAttr &, const UserAdmissionPolicyType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const UserAdmissionPolicyType &);

void operator<<(::xercesc::DOMElement &, const JoinHandlingType &);

void operator<<(::xercesc::DOMAttr &, const JoinHandlingType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const JoinHandlingType &);

void operator<<(::xercesc::DOMElement &, const DenyUsersListType &);

void operator<<(::xercesc::DOMElement &, const AllowedUsersListType &);

void operator<<(::xercesc::DOMElement &, const PersistentListType &);

void operator<<(::xercesc::DOMElement &, const TargetType &);

void operator<<(::xercesc::DOMElement &, const MethodType &);

void operator<<(::xercesc::DOMAttr &, const MethodType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const MethodType &);

void operator<<(::xercesc::DOMElement &, const ProvideAnonymityType &);

void operator<<(::xercesc::DOMAttr &, const ProvideAnonymityType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const ProvideAnonymityType &);

void operator<<(::xercesc::DOMElement &, const MixerType &);

void operator<<(::xercesc::DOMElement &, const MixerNameType &);

void operator<<(::xercesc::DOMAttr &, const MixerNameType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const MixerNameType &);

void operator<<(::xercesc::DOMElement &, const ConferenceInfoDiff &);

void operator<<(::xercesc::DOMElement &, const Entry &);

void operator<<(::xercesc::DOMElement &, const Floor &);

void operator<<(::xercesc::DOMElement &, const Target &);

void operator<<(::xercesc::DOMElement &, const User &);

void operator<<(::xercesc::DOMElement &, const Floor1 &);

void operator<<(::xercesc::DOMElement &, const Add1 &);

void operator<<(::xercesc::DOMElement &, const Remove1 &);

void operator<<(::xercesc::DOMElement &, const Replace1 &);

void operator<<(::xercesc::DOMElement &, const MixingStartOffset &);

void operator<<(::xercesc::DOMElement &, const MixingEndOffset &);
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

#endif // XCON_CONFERENCE_INFO_H
