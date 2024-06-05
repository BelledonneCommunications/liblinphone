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

#ifndef PATCH_OPS_H
#define PATCH_OPS_H

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
class Xpath;
class XpathAdd;
class Pos;
class Type;
class Add;
class Replace;
class Ws;
class Remove;

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
	typedef ::XpathAdd SelType;
	typedef ::xsd::cxx::tree::traits<SelType, char> SelTraits;

	const SelType &getSel() const;

	SelType &getSel();

	void setSel(const SelType &x);

	void setSel(::std::unique_ptr<SelType> p);

	::std::unique_ptr<SelType> setDetachSel();

	// pos
	//
	typedef ::Pos PosType;
	typedef ::xsd::cxx::tree::optional<PosType> PosOptional;
	typedef ::xsd::cxx::tree::traits<PosType, char> PosTraits;

	const PosOptional &getPos() const;

	PosOptional &getPos();

	void setPos(const PosType &x);

	void setPos(const PosOptional &x);

	void setPos(::std::unique_ptr<PosType> p);

	// type
	//
	typedef ::Type TypeType;
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
	typedef ::Xpath SelType;
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
	typedef ::Xpath SelType;
	typedef ::xsd::cxx::tree::traits<SelType, char> SelTraits;

	const SelType &getSel() const;

	SelType &getSel();

	void setSel(const SelType &x);

	void setSel(::std::unique_ptr<SelType> p);

	::std::unique_ptr<SelType> setDetachSel();

	// ws
	//
	typedef ::Ws WsType;
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

#include <iosfwd>

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

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/sax/InputSource.hpp>

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

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

#endif // PATCH_OPS_H
