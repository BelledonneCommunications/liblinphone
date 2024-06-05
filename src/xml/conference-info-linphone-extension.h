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

#ifndef CONFERENCE_INFO_LINPHONE_EXTENSION_H
#define CONFERENCE_INFO_LINPHONE_EXTENSION_H

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
namespace ConferenceInfoLinphoneExtension {
class ModeType;
class ModeEnum;
class Ephemeral;
class ServiceDescription;
class CryptoSecurityLevel;
class ConferenceTimes;
class StreamData;
} // namespace ConferenceInfoLinphoneExtension
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

#include "xml.h"

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfoLinphoneExtension {
class ModeType : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	ModeType(const char *v);

	ModeType(const ::std::string &v);

	ModeType(const ::xercesc::DOMElement &e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ModeType(const ::xercesc::DOMAttr &a,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ModeType(const ::std::string &s,
	         const ::xercesc::DOMElement *e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ModeType(const ModeType &x,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

#ifdef XSD_CXX11
	ModeType &operator=(const ModeType &) = default;
#endif

	virtual ModeType *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;
};

class ModeEnum : public ::LinphonePrivate::Xsd::XmlSchema::String {
public:
	enum Value { device_managed, admin_managed };

	ModeEnum(Value v);

	ModeEnum(const char *v);

	ModeEnum(const ::std::string &v);

	ModeEnum(const ::LinphonePrivate::Xsd::XmlSchema::String &v);

	ModeEnum(const ::xercesc::DOMElement &e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ModeEnum(const ::xercesc::DOMAttr &a,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ModeEnum(const ::std::string &s,
	         const ::xercesc::DOMElement *e,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ModeEnum(const ModeEnum &x,
	         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

#ifdef XSD_CXX11
	ModeEnum &operator=(const ModeEnum &) = default;
#endif

	virtual ModeEnum *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                         ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ModeEnum &operator=(Value v);

	virtual operator Value() const {
		return _xsd_ModeEnum_convert();
	}

protected:
	Value _xsd_ModeEnum_convert() const;

public:
	static const char *const _xsd_ModeEnum_literals_[2];
	static const Value _xsd_ModeEnum_indexes_[2];
};

class Ephemeral : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// mode
	//
	typedef ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ModeType ModeType;
	typedef ::xsd::cxx::tree::traits<ModeType, char> ModeTraits;

	const ModeType &getMode() const;

	ModeType &getMode();

	void setMode(const ModeType &x);

	void setMode(::std::unique_ptr<ModeType> p);

	::std::unique_ptr<ModeType> setDetachMode();

	// lifetime
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String LifetimeType;
	typedef ::xsd::cxx::tree::traits<LifetimeType, char> LifetimeTraits;

	const LifetimeType &getLifetime() const;

	LifetimeType &getLifetime();

	void setLifetime(const LifetimeType &x);

	void setLifetime(::std::unique_ptr<LifetimeType> p);

	::std::unique_ptr<LifetimeType> setDetachLifetime();

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
	Ephemeral(const ModeType &, const LifetimeType &);

	Ephemeral(::std::unique_ptr<ModeType>, ::std::unique_ptr<LifetimeType>);

	Ephemeral(const ::xercesc::DOMElement &e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Ephemeral(const Ephemeral &x,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Ephemeral *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Ephemeral &operator=(const Ephemeral &x);

	virtual ~Ephemeral();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<ModeType> mode_;
	::xsd::cxx::tree::one<LifetimeType> lifetime_;
	AnySequence any_;
};

class ServiceDescription : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// service-id
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String ServiceIdType;
	typedef ::xsd::cxx::tree::traits<ServiceIdType, char> ServiceIdTraits;

	const ServiceIdType &getServiceId() const;

	ServiceIdType &getServiceId();

	void setServiceId(const ServiceIdType &x);

	void setServiceId(::std::unique_ptr<ServiceIdType> p);

	::std::unique_ptr<ServiceIdType> setDetachService_id();

	// version
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String VersionType;
	typedef ::xsd::cxx::tree::traits<VersionType, char> VersionTraits;

	const VersionType &getVersion() const;

	VersionType &getVersion();

	void setVersion(const VersionType &x);

	void setVersion(::std::unique_ptr<VersionType> p);

	::std::unique_ptr<VersionType> setDetachVersion();

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
	ServiceDescription(const ServiceIdType &, const VersionType &);

	ServiceDescription(::std::unique_ptr<ServiceIdType>, ::std::unique_ptr<VersionType>);

	ServiceDescription(const ::xercesc::DOMElement &e,
	                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ServiceDescription(const ServiceDescription &x,
	                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ServiceDescription *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                   ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ServiceDescription &operator=(const ServiceDescription &x);

	virtual ~ServiceDescription();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<ServiceIdType> service_id_;
	::xsd::cxx::tree::one<VersionType> version_;
	AnySequence any_;
};

class CryptoSecurityLevel : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// level
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String LevelType;
	typedef ::xsd::cxx::tree::traits<LevelType, char> LevelTraits;

	const LevelType &getLevel() const;

	LevelType &getLevel();

	void setLevel(const LevelType &x);

	void setLevel(::std::unique_ptr<LevelType> p);

	::std::unique_ptr<LevelType> setDetachLevel();

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
	CryptoSecurityLevel(const LevelType &);

	CryptoSecurityLevel(::std::unique_ptr<LevelType>);

	CryptoSecurityLevel(const ::xercesc::DOMElement &e,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	CryptoSecurityLevel(const CryptoSecurityLevel &x,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual CryptoSecurityLevel *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	CryptoSecurityLevel &operator=(const CryptoSecurityLevel &x);

	virtual ~CryptoSecurityLevel();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<LevelType> level_;
	AnySequence any_;
};

class ConferenceTimes : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// start
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::DateTime StartType;
	typedef ::xsd::cxx::tree::optional<StartType> StartOptional;
	typedef ::xsd::cxx::tree::traits<StartType, char> StartTraits;

	const StartOptional &getStart() const;

	StartOptional &getStart();

	void setStart(const StartType &x);

	void setStart(const StartOptional &x);

	void setStart(::std::unique_ptr<StartType> p);

	// end
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::DateTime EndType;
	typedef ::xsd::cxx::tree::optional<EndType> EndOptional;
	typedef ::xsd::cxx::tree::traits<EndType, char> EndTraits;

	const EndOptional &getEnd() const;

	EndOptional &getEnd();

	void setEnd(const EndType &x);

	void setEnd(const EndOptional &x);

	void setEnd(::std::unique_ptr<EndType> p);

	// Constructors.
	//
	ConferenceTimes();

	ConferenceTimes(const ::xercesc::DOMElement &e,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ConferenceTimes(const ConferenceTimes &x,
	                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ConferenceTimes *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ConferenceTimes &operator=(const ConferenceTimes &x);

	virtual ~ConferenceTimes();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	StartOptional start_;
	EndOptional end_;
};

class StreamData : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// stream-content
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String StreamContentType;
	typedef ::xsd::cxx::tree::traits<StreamContentType, char> StreamContentTraits;

	const StreamContentType &getStreamContent() const;

	StreamContentType &getStreamContent();

	void setStreamContent(const StreamContentType &x);

	void setStreamContent(::std::unique_ptr<StreamContentType> p);

	::std::unique_ptr<StreamContentType> setDetachStream_content();

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
	StreamData(const StreamContentType &);

	StreamData(::std::unique_ptr<StreamContentType>);

	StreamData(const ::xercesc::DOMElement &e,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	StreamData(const StreamData &x,
	           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual StreamData *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                           ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	StreamData &operator=(const StreamData &x);

	virtual ~StreamData();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<StreamContentType> stream_content_;
	AnySequence any_;
};
} // namespace ConferenceInfoLinphoneExtension
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfoLinphoneExtension {
::std::ostream &operator<<(::std::ostream &, const ModeType &);

::std::ostream &operator<<(::std::ostream &, ModeEnum::Value);

::std::ostream &operator<<(::std::ostream &, const ModeEnum &);

::std::ostream &operator<<(::std::ostream &, const Ephemeral &);

::std::ostream &operator<<(::std::ostream &, const ServiceDescription &);

::std::ostream &operator<<(::std::ostream &, const CryptoSecurityLevel &);

::std::ostream &operator<<(::std::ostream &, const ConferenceTimes &);

::std::ostream &operator<<(::std::ostream &, const StreamData &);
} // namespace ConferenceInfoLinphoneExtension
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/sax/InputSource.hpp>

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfoLinphoneExtension {
// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral> parseEphemeral(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription> parseServiceDescription(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel>
parseCryptoSecurityLevel(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes> parseConferenceTimes(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData> parseStreamData(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());
} // namespace ConferenceInfoLinphoneExtension
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace ConferenceInfoLinphoneExtension {
void operator<<(::xercesc::DOMElement &, const ModeType &);

void operator<<(::xercesc::DOMAttr &, const ModeType &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const ModeType &);

void operator<<(::xercesc::DOMElement &, const ModeEnum &);

void operator<<(::xercesc::DOMAttr &, const ModeEnum &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const ModeEnum &);

// Serialize to std::ostream.
//

void serializeEphemeral(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeEphemeral(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeEphemeral(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeEphemeral(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeEphemeral(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeEphemeral(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeEphemeral(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &x,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeEphemeral(const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::Ephemeral &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeServiceDescription(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription &x,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeServiceDescription(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeServiceDescription(::std::ostream &os,
                                 const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription &x,
                                 ::xercesc::DOMErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeServiceDescription(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription &x,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeServiceDescription(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeServiceDescription(::xercesc::XMLFormatTarget &ft,
                                 const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription &x,
                                 ::xercesc::DOMErrorHandler &eh,
                                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                 const ::std::string &e = "UTF-8",
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeServiceDescription(::xercesc::DOMDocument &d,
                                 const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription &x,
                                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeServiceDescription(const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ServiceDescription &x,
                            const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeCryptoSecurityLevel(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCryptoSecurityLevel(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCryptoSecurityLevel(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeCryptoSecurityLevel(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCryptoSecurityLevel(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeCryptoSecurityLevel(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeCryptoSecurityLevel(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeCryptoSecurityLevel(const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::CryptoSecurityLevel &x,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeConferenceTimes(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceTimes(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes &x,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceTimes(::std::ostream &os,
                              const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes &x,
                              ::xercesc::DOMErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeConferenceTimes(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceTimes(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes &x,
                              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeConferenceTimes(::xercesc::XMLFormatTarget &ft,
                              const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes &x,
                              ::xercesc::DOMErrorHandler &eh,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              const ::std::string &e = "UTF-8",
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeConferenceTimes(::xercesc::DOMDocument &d,
                              const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes &x,
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeConferenceTimes(const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTimes &x,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeStreamData(::std::ostream &os,
                         const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData &x,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeStreamData(::std::ostream &os,
                         const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData &x,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeStreamData(::std::ostream &os,
                         const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData &x,
                         ::xercesc::DOMErrorHandler &eh,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeStreamData(::xercesc::XMLFormatTarget &ft,
                         const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData &x,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeStreamData(::xercesc::XMLFormatTarget &ft,
                         const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData &x,
                         ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeStreamData(::xercesc::XMLFormatTarget &ft,
                         const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData &x,
                         ::xercesc::DOMErrorHandler &eh,
                         const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                             ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                         const ::std::string &e = "UTF-8",
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeStreamData(::xercesc::DOMDocument &d,
                         const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData &x,
                         ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeStreamData(const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::StreamData &x,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const Ephemeral &);

void operator<<(::xercesc::DOMElement &, const ServiceDescription &);

void operator<<(::xercesc::DOMElement &, const CryptoSecurityLevel &);

void operator<<(::xercesc::DOMElement &, const ConferenceTimes &);

void operator<<(::xercesc::DOMElement &, const StreamData &);
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

#endif // CONFERENCE_INFO_LINPHONE_EXTENSION_H
