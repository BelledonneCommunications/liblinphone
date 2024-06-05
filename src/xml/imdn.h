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

#ifndef IMDN_H
#define IMDN_H

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
namespace Imdn {
class Imdn;
class DeliveryNotification;
class Delivered;
class Failed;
class DisplayNotification;
class Displayed;
class ProcessingNotification;
class Processed;
class Stored;
class Forbidden;
class Error;
class Status;
class Status1;
class Status2;
} // namespace Imdn
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

#include "linphone-imdn.h"

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
class Imdn : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// message-id
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Token MessageIdType;
	typedef ::xsd::cxx::tree::traits<MessageIdType, char> MessageIdTraits;

	const MessageIdType &getMessageId() const;

	MessageIdType &getMessageId();

	void setMessageId(const MessageIdType &x);

	void setMessageId(::std::unique_ptr<MessageIdType> p);

	::std::unique_ptr<MessageIdType> setDetachMessage_id();

	// datetime
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String DatetimeType;
	typedef ::xsd::cxx::tree::traits<DatetimeType, char> DatetimeTraits;

	const DatetimeType &getDatetime() const;

	DatetimeType &getDatetime();

	void setDatetime(const DatetimeType &x);

	void setDatetime(::std::unique_ptr<DatetimeType> p);

	::std::unique_ptr<DatetimeType> setDetachDatetime();

	// recipient-uri
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Uri RecipientUriType;
	typedef ::xsd::cxx::tree::optional<RecipientUriType> RecipientUriOptional;
	typedef ::xsd::cxx::tree::traits<RecipientUriType, char> RecipientUriTraits;

	const RecipientUriOptional &getRecipientUri() const;

	RecipientUriOptional &getRecipientUri();

	void setRecipientUri(const RecipientUriType &x);

	void setRecipientUri(const RecipientUriOptional &x);

	void setRecipientUri(::std::unique_ptr<RecipientUriType> p);

	// original-recipient-uri
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::Uri OriginalRecipientUriType;
	typedef ::xsd::cxx::tree::optional<OriginalRecipientUriType> OriginalRecipientUriOptional;
	typedef ::xsd::cxx::tree::traits<OriginalRecipientUriType, char> OriginalRecipientUriTraits;

	const OriginalRecipientUriOptional &getOriginalRecipientUri() const;

	OriginalRecipientUriOptional &getOriginalRecipientUri();

	void setOriginalRecipientUri(const OriginalRecipientUriType &x);

	void setOriginalRecipientUri(const OriginalRecipientUriOptional &x);

	void setOriginalRecipientUri(::std::unique_ptr<OriginalRecipientUriType> p);

	// subject
	//
	typedef ::LinphonePrivate::Xsd::XmlSchema::String SubjectType;
	typedef ::xsd::cxx::tree::optional<SubjectType> SubjectOptional;
	typedef ::xsd::cxx::tree::traits<SubjectType, char> SubjectTraits;

	const SubjectOptional &getSubject() const;

	SubjectOptional &getSubject();

	void setSubject(const SubjectType &x);

	void setSubject(const SubjectOptional &x);

	void setSubject(::std::unique_ptr<SubjectType> p);

	// delivery-notification
	//
	typedef ::LinphonePrivate::Xsd::Imdn::DeliveryNotification DeliveryNotificationType;
	typedef ::xsd::cxx::tree::optional<DeliveryNotificationType> DeliveryNotificationOptional;
	typedef ::xsd::cxx::tree::traits<DeliveryNotificationType, char> DeliveryNotificationTraits;

	const DeliveryNotificationOptional &getDeliveryNotification() const;

	DeliveryNotificationOptional &getDeliveryNotification();

	void setDeliveryNotification(const DeliveryNotificationType &x);

	void setDeliveryNotification(const DeliveryNotificationOptional &x);

	void setDeliveryNotification(::std::unique_ptr<DeliveryNotificationType> p);

	// display-notification
	//
	typedef ::LinphonePrivate::Xsd::Imdn::DisplayNotification DisplayNotificationType;
	typedef ::xsd::cxx::tree::optional<DisplayNotificationType> DisplayNotificationOptional;
	typedef ::xsd::cxx::tree::traits<DisplayNotificationType, char> DisplayNotificationTraits;

	const DisplayNotificationOptional &getDisplayNotification() const;

	DisplayNotificationOptional &getDisplayNotification();

	void setDisplayNotification(const DisplayNotificationType &x);

	void setDisplayNotification(const DisplayNotificationOptional &x);

	void setDisplayNotification(::std::unique_ptr<DisplayNotificationType> p);

	// processing-notification
	//
	typedef ::LinphonePrivate::Xsd::Imdn::ProcessingNotification ProcessingNotificationType;
	typedef ::xsd::cxx::tree::optional<ProcessingNotificationType> ProcessingNotificationOptional;
	typedef ::xsd::cxx::tree::traits<ProcessingNotificationType, char> ProcessingNotificationTraits;

	const ProcessingNotificationOptional &getProcessingNotification() const;

	ProcessingNotificationOptional &getProcessingNotification();

	void setProcessingNotification(const ProcessingNotificationType &x);

	void setProcessingNotification(const ProcessingNotificationOptional &x);

	void setProcessingNotification(::std::unique_ptr<ProcessingNotificationType> p);

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
	Imdn(const MessageIdType &, const DatetimeType &);

	Imdn(::std::unique_ptr<MessageIdType>, ::std::unique_ptr<DatetimeType>);

	Imdn(const ::xercesc::DOMElement &e,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Imdn(const Imdn &x,
	     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Imdn *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Imdn &operator=(const Imdn &x);

	virtual ~Imdn();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	::xsd::cxx::tree::one<MessageIdType> message_id_;
	::xsd::cxx::tree::one<DatetimeType> datetime_;
	RecipientUriOptional recipient_uri_;
	OriginalRecipientUriOptional original_recipient_uri_;
	SubjectOptional subject_;
	DeliveryNotificationOptional delivery_notification_;
	DisplayNotificationOptional display_notification_;
	ProcessingNotificationOptional processing_notification_;
	AnySequence any_;
};

class DeliveryNotification : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// status
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Status StatusType;
	typedef ::xsd::cxx::tree::traits<StatusType, char> StatusTraits;

	const StatusType &getStatus() const;

	StatusType &getStatus();

	void setStatus(const StatusType &x);

	void setStatus(::std::unique_ptr<StatusType> p);

	::std::unique_ptr<StatusType> setDetachStatus();

	// Constructors.
	//
	DeliveryNotification(const StatusType &);

	DeliveryNotification(::std::unique_ptr<StatusType>);

	DeliveryNotification(const ::xercesc::DOMElement &e,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	DeliveryNotification(const DeliveryNotification &x,
	                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual DeliveryNotification *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                     ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	DeliveryNotification &operator=(const DeliveryNotification &x);

	virtual ~DeliveryNotification();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<StatusType> status_;
};

class Delivered : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// Constructors.
	//
	Delivered();

	Delivered(const ::xercesc::DOMElement &e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Delivered(const ::xercesc::DOMAttr &a,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Delivered(const ::std::string &s,
	          const ::xercesc::DOMElement *e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Delivered(const Delivered &x,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Delivered *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Delivered &operator=(const Delivered &) = default;
#endif

	virtual ~Delivered();
};

class Failed : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// Constructors.
	//
	Failed();

	Failed(const ::xercesc::DOMElement &e,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Failed(const ::xercesc::DOMAttr &a,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Failed(const ::std::string &s,
	       const ::xercesc::DOMElement *e,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Failed(const Failed &x,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Failed *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Failed &operator=(const Failed &) = default;
#endif

	virtual ~Failed();
};

class DisplayNotification : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// status
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Status1 StatusType;
	typedef ::xsd::cxx::tree::traits<StatusType, char> StatusTraits;

	const StatusType &getStatus() const;

	StatusType &getStatus();

	void setStatus(const StatusType &x);

	void setStatus(::std::unique_ptr<StatusType> p);

	::std::unique_ptr<StatusType> setDetachStatus();

	// Constructors.
	//
	DisplayNotification(const StatusType &);

	DisplayNotification(::std::unique_ptr<StatusType>);

	DisplayNotification(const ::xercesc::DOMElement &e,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	DisplayNotification(const DisplayNotification &x,
	                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual DisplayNotification *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                    ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	DisplayNotification &operator=(const DisplayNotification &x);

	virtual ~DisplayNotification();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<StatusType> status_;
};

class Displayed : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// Constructors.
	//
	Displayed();

	Displayed(const ::xercesc::DOMElement &e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Displayed(const ::xercesc::DOMAttr &a,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Displayed(const ::std::string &s,
	          const ::xercesc::DOMElement *e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Displayed(const Displayed &x,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Displayed *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Displayed &operator=(const Displayed &) = default;
#endif

	virtual ~Displayed();
};

class ProcessingNotification : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// status
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Status2 StatusType;
	typedef ::xsd::cxx::tree::traits<StatusType, char> StatusTraits;

	const StatusType &getStatus() const;

	StatusType &getStatus();

	void setStatus(const StatusType &x);

	void setStatus(::std::unique_ptr<StatusType> p);

	::std::unique_ptr<StatusType> setDetachStatus();

	// Constructors.
	//
	ProcessingNotification(const StatusType &);

	ProcessingNotification(::std::unique_ptr<StatusType>);

	ProcessingNotification(const ::xercesc::DOMElement &e,
	                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	ProcessingNotification(const ProcessingNotification &x,
	                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual ProcessingNotification *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	ProcessingNotification &operator=(const ProcessingNotification &x);

	virtual ~ProcessingNotification();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::xsd::cxx::tree::one<StatusType> status_;
};

class Processed : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// Constructors.
	//
	Processed();

	Processed(const ::xercesc::DOMElement &e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Processed(const ::xercesc::DOMAttr &a,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Processed(const ::std::string &s,
	          const ::xercesc::DOMElement *e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Processed(const Processed &x,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Processed *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Processed &operator=(const Processed &) = default;
#endif

	virtual ~Processed();
};

class Stored : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// Constructors.
	//
	Stored();

	Stored(const ::xercesc::DOMElement &e,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Stored(const ::xercesc::DOMAttr &a,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Stored(const ::std::string &s,
	       const ::xercesc::DOMElement *e,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Stored(const Stored &x,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Stored *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Stored &operator=(const Stored &) = default;
#endif

	virtual ~Stored();
};

class Forbidden : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// Constructors.
	//
	Forbidden();

	Forbidden(const ::xercesc::DOMElement &e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Forbidden(const ::xercesc::DOMAttr &a,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Forbidden(const ::std::string &s,
	          const ::xercesc::DOMElement *e,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Forbidden(const Forbidden &x,
	          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Forbidden *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                          ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Forbidden &operator=(const Forbidden &) = default;
#endif

	virtual ~Forbidden();
};

class Error : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// Constructors.
	//
	Error();

	Error(const ::xercesc::DOMElement &e,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Error(const ::xercesc::DOMAttr &a,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Error(const ::std::string &s,
	      const ::xercesc::DOMElement *e,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Error(const Error &x,
	      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Error *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                      ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

#ifdef XSD_CXX11
	Error &operator=(const Error &) = default;
#endif

	virtual ~Error();
};

class Status : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// delivered
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Delivered DeliveredType;
	typedef ::xsd::cxx::tree::optional<DeliveredType> DeliveredOptional;
	typedef ::xsd::cxx::tree::traits<DeliveredType, char> DeliveredTraits;

	const DeliveredOptional &getDelivered() const;

	DeliveredOptional &getDelivered();

	void setDelivered(const DeliveredType &x);

	void setDelivered(const DeliveredOptional &x);

	void setDelivered(::std::unique_ptr<DeliveredType> p);

	// failed
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Failed FailedType;
	typedef ::xsd::cxx::tree::optional<FailedType> FailedOptional;
	typedef ::xsd::cxx::tree::traits<FailedType, char> FailedTraits;

	const FailedOptional &getFailed() const;

	FailedOptional &getFailed();

	void setFailed(const FailedType &x);

	void setFailed(const FailedOptional &x);

	void setFailed(::std::unique_ptr<FailedType> p);

	// forbidden
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Forbidden ForbiddenType;
	typedef ::xsd::cxx::tree::optional<ForbiddenType> ForbiddenOptional;
	typedef ::xsd::cxx::tree::traits<ForbiddenType, char> ForbiddenTraits;

	const ForbiddenOptional &getForbidden() const;

	ForbiddenOptional &getForbidden();

	void setForbidden(const ForbiddenType &x);

	void setForbidden(const ForbiddenOptional &x);

	void setForbidden(::std::unique_ptr<ForbiddenType> p);

	// error
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Error ErrorType;
	typedef ::xsd::cxx::tree::optional<ErrorType> ErrorOptional;
	typedef ::xsd::cxx::tree::traits<ErrorType, char> ErrorTraits;

	const ErrorOptional &getError() const;

	ErrorOptional &getError();

	void setError(const ErrorType &x);

	void setError(const ErrorOptional &x);

	void setError(::std::unique_ptr<ErrorType> p);

	// reason
	//
	typedef ::LinphonePrivate::Xsd::LinphoneImdn::ImdnReason ReasonType;
	typedef ::xsd::cxx::tree::optional<ReasonType> ReasonOptional;
	typedef ::xsd::cxx::tree::traits<ReasonType, char> ReasonTraits;

	const ReasonOptional &getReason() const;

	ReasonOptional &getReason();

	void setReason(const ReasonType &x);

	void setReason(const ReasonOptional &x);

	void setReason(::std::unique_ptr<ReasonType> p);

	// Constructors.
	//
	Status();

	Status(const ::xercesc::DOMElement &e,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Status(const Status &x,
	       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Status *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                       ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Status &operator=(const Status &x);

	virtual ~Status();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	DeliveredOptional delivered_;
	FailedOptional failed_;
	ForbiddenOptional forbidden_;
	ErrorOptional error_;
	ReasonOptional reason_;
};

class Status1 : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// displayed
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Displayed DisplayedType;
	typedef ::xsd::cxx::tree::optional<DisplayedType> DisplayedOptional;
	typedef ::xsd::cxx::tree::traits<DisplayedType, char> DisplayedTraits;

	const DisplayedOptional &getDisplayed() const;

	DisplayedOptional &getDisplayed();

	void setDisplayed(const DisplayedType &x);

	void setDisplayed(const DisplayedOptional &x);

	void setDisplayed(::std::unique_ptr<DisplayedType> p);

	// forbidden
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Forbidden ForbiddenType;
	typedef ::xsd::cxx::tree::optional<ForbiddenType> ForbiddenOptional;
	typedef ::xsd::cxx::tree::traits<ForbiddenType, char> ForbiddenTraits;

	const ForbiddenOptional &getForbidden() const;

	ForbiddenOptional &getForbidden();

	void setForbidden(const ForbiddenType &x);

	void setForbidden(const ForbiddenOptional &x);

	void setForbidden(::std::unique_ptr<ForbiddenType> p);

	// error
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Error ErrorType;
	typedef ::xsd::cxx::tree::optional<ErrorType> ErrorOptional;
	typedef ::xsd::cxx::tree::traits<ErrorType, char> ErrorTraits;

	const ErrorOptional &getError() const;

	ErrorOptional &getError();

	void setError(const ErrorType &x);

	void setError(const ErrorOptional &x);

	void setError(::std::unique_ptr<ErrorType> p);

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
	Status1();

	Status1(const ::xercesc::DOMElement &e,
	        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Status1(const Status1 &x,
	        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Status1 *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Status1 &operator=(const Status1 &x);

	virtual ~Status1();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	DisplayedOptional displayed_;
	ForbiddenOptional forbidden_;
	ErrorOptional error_;
	AnySequence any_;
};

class Status2 : public ::LinphonePrivate::Xsd::XmlSchema::Type {
public:
	// processed
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Processed ProcessedType;
	typedef ::xsd::cxx::tree::optional<ProcessedType> ProcessedOptional;
	typedef ::xsd::cxx::tree::traits<ProcessedType, char> ProcessedTraits;

	const ProcessedOptional &getProcessed() const;

	ProcessedOptional &getProcessed();

	void setProcessed(const ProcessedType &x);

	void setProcessed(const ProcessedOptional &x);

	void setProcessed(::std::unique_ptr<ProcessedType> p);

	// stored
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Stored StoredType;
	typedef ::xsd::cxx::tree::optional<StoredType> StoredOptional;
	typedef ::xsd::cxx::tree::traits<StoredType, char> StoredTraits;

	const StoredOptional &getStored() const;

	StoredOptional &getStored();

	void setStored(const StoredType &x);

	void setStored(const StoredOptional &x);

	void setStored(::std::unique_ptr<StoredType> p);

	// forbidden
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Forbidden ForbiddenType;
	typedef ::xsd::cxx::tree::optional<ForbiddenType> ForbiddenOptional;
	typedef ::xsd::cxx::tree::traits<ForbiddenType, char> ForbiddenTraits;

	const ForbiddenOptional &getForbidden() const;

	ForbiddenOptional &getForbidden();

	void setForbidden(const ForbiddenType &x);

	void setForbidden(const ForbiddenOptional &x);

	void setForbidden(::std::unique_ptr<ForbiddenType> p);

	// error
	//
	typedef ::LinphonePrivate::Xsd::Imdn::Error ErrorType;
	typedef ::xsd::cxx::tree::optional<ErrorType> ErrorOptional;
	typedef ::xsd::cxx::tree::traits<ErrorType, char> ErrorTraits;

	const ErrorOptional &getError() const;

	ErrorOptional &getError();

	void setError(const ErrorType &x);

	void setError(const ErrorOptional &x);

	void setError(::std::unique_ptr<ErrorType> p);

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
	Status2();

	Status2(const ::xercesc::DOMElement &e,
	        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	Status2(const Status2 &x,
	        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0);

	virtual Status2 *_clone(::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
	                        ::LinphonePrivate::Xsd::XmlSchema::Container *c = 0) const;

	Status2 &operator=(const Status2 &x);

	virtual ~Status2();

	// Implementation.
	//
protected:
	void parse(::xsd::cxx::xml::dom::parser<char> &, ::LinphonePrivate::Xsd::XmlSchema::Flags);

protected:
	::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> dom_document_;

	ProcessedOptional processed_;
	StoredOptional stored_;
	ForbiddenOptional forbidden_;
	ErrorOptional error_;
	AnySequence any_;
};
} // namespace Imdn
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
::std::ostream &operator<<(::std::ostream &, const Imdn &);

::std::ostream &operator<<(::std::ostream &, const DeliveryNotification &);

::std::ostream &operator<<(::std::ostream &, const Delivered &);

::std::ostream &operator<<(::std::ostream &, const Failed &);

::std::ostream &operator<<(::std::ostream &, const DisplayNotification &);

::std::ostream &operator<<(::std::ostream &, const Displayed &);

::std::ostream &operator<<(::std::ostream &, const ProcessingNotification &);

::std::ostream &operator<<(::std::ostream &, const Processed &);

::std::ostream &operator<<(::std::ostream &, const Stored &);

::std::ostream &operator<<(::std::ostream &, const Forbidden &);

::std::ostream &operator<<(::std::ostream &, const Error &);

::std::ostream &operator<<(::std::ostream &, const Status &);

::std::ostream &operator<<(::std::ostream &, const Status1 &);

::std::ostream &operator<<(::std::ostream &, const Status2 &);
} // namespace Imdn
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/sax/InputSource.hpp>

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(const ::std::string &uri,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(const ::std::string &uri,
          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(const ::std::string &uri,
          ::xercesc::DOMErrorHandler &eh,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          ::xercesc::DOMErrorHandler &eh,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          const ::std::string &id,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          const ::std::string &id,
          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::std::istream &is,
          const ::std::string &id,
          ::xercesc::DOMErrorHandler &eh,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::xercesc::InputSource &is,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::xercesc::InputSource &is,
          ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::xercesc::InputSource &is,
          ::xercesc::DOMErrorHandler &eh,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(const ::xercesc::DOMDocument &d,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Imdn>
parseImdn(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
          ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
          const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Token> parseMessageId(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(const ::std::string &uri,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(const ::std::string &uri,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(const ::std::string &uri,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              const ::std::string &id,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              const ::std::string &id,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::std::istream &is,
              const ::std::string &id,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::xercesc::InputSource &is,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::xercesc::InputSource &is,
              ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::xercesc::InputSource &is,
              ::xercesc::DOMErrorHandler &eh,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(const ::xercesc::DOMDocument &d,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseDatetime(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
              const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseRecipientUri(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::Uri> parseOriginalRecipientUri(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(const ::std::string &uri,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(const ::std::string &uri,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(const ::std::string &uri,
             ::xercesc::DOMErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             ::xercesc::DOMErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             const ::std::string &id,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             const ::std::string &id,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::std::istream &is,
             const ::std::string &id,
             ::xercesc::DOMErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::xercesc::InputSource &is,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::xercesc::InputSource &is,
             ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::xercesc::InputSource &is,
             ::xercesc::DOMErrorHandler &eh,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(const ::xercesc::DOMDocument &d,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::XmlSchema::String>
parseSubject(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
             const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DeliveryNotification> parseDeliveryNotification(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Delivered> parseDelivered(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(const ::std::string &uri,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(const ::std::string &uri,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(const ::std::string &uri,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            const ::std::string &id,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            const ::std::string &id,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::std::istream &is,
            const ::std::string &id,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::xercesc::InputSource &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::xercesc::InputSource &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::xercesc::InputSource &is,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(const ::xercesc::DOMDocument &d,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Failed>
parseFailed(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::DisplayNotification> parseDisplayNotification(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Displayed> parseDisplayed(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::ProcessingNotification> parseProcessingNotification(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Processed> parseProcessed(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(const ::std::string &uri,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(const ::std::string &uri,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(const ::std::string &uri,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            const ::std::string &id,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            const ::std::string &id,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::std::istream &is,
            const ::std::string &id,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::xercesc::InputSource &is,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::xercesc::InputSource &is,
            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::xercesc::InputSource &is,
            ::xercesc::DOMErrorHandler &eh,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(const ::xercesc::DOMDocument &d,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Stored>
parseStored(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
            ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
            const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    const ::std::string &uri,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    const ::std::string &uri,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::std::istream &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::std::istream &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::std::istream &is,
    const ::std::string &id,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::std::istream &is,
    const ::std::string &id,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::xercesc::InputSource &is,
    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::xercesc::InputSource &is,
    ::xercesc::DOMErrorHandler &eh,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    const ::xercesc::DOMDocument &d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Forbidden> parseForbidden(
    ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
    const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse a URI or a local file.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(const ::std::string &uri,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(const ::std::string &uri,
           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(const ::std::string &uri,
           ::xercesc::DOMErrorHandler &eh,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse std::istream.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           ::xercesc::DOMErrorHandler &eh,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           const ::std::string &id,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           const ::std::string &id,
           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::std::istream &is,
           const ::std::string &id,
           ::xercesc::DOMErrorHandler &eh,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::InputSource.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::xercesc::InputSource &is,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::xercesc::InputSource &is,
           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::xercesc::InputSource &is,
           ::xercesc::DOMErrorHandler &eh,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

// Parse xercesc::DOMDocument.
//

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(const ::xercesc::DOMDocument &d,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());

::std::unique_ptr<::LinphonePrivate::Xsd::Imdn::Error>
parseError(::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument> d,
           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0,
           const ::LinphonePrivate::Xsd::XmlSchema::Properties &p = ::LinphonePrivate::Xsd::XmlSchema::Properties());
} // namespace Imdn
} // namespace Xsd
} // namespace LinphonePrivate

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

namespace LinphonePrivate {
namespace Xsd {
namespace Imdn {
// Serialize to std::ostream.
//

void serializeImdn(::std::ostream &os,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   const ::std::string &e = "UTF-8",
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeImdn(::std::ostream &os,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &x,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   const ::std::string &e = "UTF-8",
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeImdn(::std::ostream &os,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &x,
                   ::xercesc::DOMErrorHandler &eh,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   const ::std::string &e = "UTF-8",
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeImdn(::xercesc::XMLFormatTarget &ft,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   const ::std::string &e = "UTF-8",
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeImdn(::xercesc::XMLFormatTarget &ft,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &x,
                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   const ::std::string &e = "UTF-8",
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeImdn(::xercesc::XMLFormatTarget &ft,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &x,
                   ::xercesc::DOMErrorHandler &eh,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   const ::std::string &e = "UTF-8",
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeImdn(::xercesc::DOMDocument &d,
                   const ::LinphonePrivate::Xsd::Imdn::Imdn &x,
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeImdn(const ::LinphonePrivate::Xsd::Imdn::Imdn &x,
              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeMessageId(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeMessageId(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeMessageId(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeMessageId(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeMessageId(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeMessageId(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeMessageId(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::XmlSchema::Token &x,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeMessageId(const ::LinphonePrivate::Xsd::XmlSchema::Token &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeDatetime(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDatetime(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDatetime(::std::ostream &os,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                       ::xercesc::DOMErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeDatetime(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDatetime(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                       ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDatetime(::xercesc::XMLFormatTarget &ft,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                       ::xercesc::DOMErrorHandler &eh,
                       const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                           ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                       const ::std::string &e = "UTF-8",
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeDatetime(::xercesc::DOMDocument &d,
                       const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                       ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDatetime(const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeRecipientUri(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeRecipientUri(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeRecipientUri(::std::ostream &os,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeRecipientUri(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeRecipientUri(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                           ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeRecipientUri(::xercesc::XMLFormatTarget &ft,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                           ::xercesc::DOMErrorHandler &eh,
                           const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                               ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                           const ::std::string &e = "UTF-8",
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeRecipientUri(::xercesc::DOMDocument &d,
                           const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                           ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeRecipientUri(const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeOriginalRecipientUri(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeOriginalRecipientUri(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeOriginalRecipientUri(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeOriginalRecipientUri(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeOriginalRecipientUri(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeOriginalRecipientUri(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeOriginalRecipientUri(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeOriginalRecipientUri(const ::LinphonePrivate::Xsd::XmlSchema::Uri &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeSubject(::std::ostream &os,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSubject(::std::ostream &os,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSubject(::std::ostream &os,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                      ::xercesc::DOMErrorHandler &eh,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeSubject(::xercesc::XMLFormatTarget &ft,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSubject(::xercesc::XMLFormatTarget &ft,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                      ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeSubject(::xercesc::XMLFormatTarget &ft,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                      ::xercesc::DOMErrorHandler &eh,
                      const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                          ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                      const ::std::string &e = "UTF-8",
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeSubject(::xercesc::DOMDocument &d,
                      const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                      ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeSubject(const ::LinphonePrivate::Xsd::XmlSchema::String &x,
                 const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                     ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                 ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeDeliveryNotification(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDeliveryNotification(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDeliveryNotification(::std::ostream &os,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeDeliveryNotification(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &x,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDeliveryNotification(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDeliveryNotification(::xercesc::XMLFormatTarget &ft,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &x,
                                   ::xercesc::DOMErrorHandler &eh,
                                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                   const ::std::string &e = "UTF-8",
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeDeliveryNotification(::xercesc::DOMDocument &d,
                                   const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &x,
                                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDeliveryNotification(const ::LinphonePrivate::Xsd::Imdn::DeliveryNotification &x,
                              const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                  ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                              ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeDelivered(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDelivered(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDelivered(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeDelivered(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDelivered(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDelivered(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeDelivered(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::Imdn::Delivered &x,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDelivered(const ::LinphonePrivate::Xsd::Imdn::Delivered &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeFailed(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFailed(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &x,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFailed(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &x,
                     ::xercesc::DOMErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeFailed(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFailed(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &x,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeFailed(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &x,
                     ::xercesc::DOMErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeFailed(::xercesc::DOMDocument &d,
                     const ::LinphonePrivate::Xsd::Imdn::Failed &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeFailed(const ::LinphonePrivate::Xsd::Imdn::Failed &x,
                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeDisplayNotification(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDisplayNotification(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDisplayNotification(::std::ostream &os,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeDisplayNotification(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &x,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDisplayNotification(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDisplayNotification(::xercesc::XMLFormatTarget &ft,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &x,
                                  ::xercesc::DOMErrorHandler &eh,
                                  const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                      ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                  const ::std::string &e = "UTF-8",
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeDisplayNotification(::xercesc::DOMDocument &d,
                                  const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &x,
                                  ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDisplayNotification(const ::LinphonePrivate::Xsd::Imdn::DisplayNotification &x,
                             const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                 ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                             ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeDisplayed(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDisplayed(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDisplayed(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeDisplayed(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDisplayed(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeDisplayed(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeDisplayed(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::Imdn::Displayed &x,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeDisplayed(const ::LinphonePrivate::Xsd::Imdn::Displayed &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeProcessingNotification(::std::ostream &os,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &x,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                     const ::std::string &e = "UTF-8",
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProcessingNotification(::std::ostream &os,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &x,
                                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                     const ::std::string &e = "UTF-8",
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProcessingNotification(::std::ostream &os,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &x,
                                     ::xercesc::DOMErrorHandler &eh,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                     const ::std::string &e = "UTF-8",
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeProcessingNotification(::xercesc::XMLFormatTarget &ft,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &x,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                     const ::std::string &e = "UTF-8",
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProcessingNotification(::xercesc::XMLFormatTarget &ft,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &x,
                                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                     const ::std::string &e = "UTF-8",
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProcessingNotification(::xercesc::XMLFormatTarget &ft,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &x,
                                     ::xercesc::DOMErrorHandler &eh,
                                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                     const ::std::string &e = "UTF-8",
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeProcessingNotification(::xercesc::DOMDocument &d,
                                     const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &x,
                                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeProcessingNotification(const ::LinphonePrivate::Xsd::Imdn::ProcessingNotification &x,
                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeProcessed(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProcessed(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProcessed(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeProcessed(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProcessed(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeProcessed(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeProcessed(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::Imdn::Processed &x,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeProcessed(const ::LinphonePrivate::Xsd::Imdn::Processed &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeStored(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeStored(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &x,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeStored(::std::ostream &os,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &x,
                     ::xercesc::DOMErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeStored(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &x,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeStored(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &x,
                     ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeStored(::xercesc::XMLFormatTarget &ft,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &x,
                     ::xercesc::DOMErrorHandler &eh,
                     const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                         ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                     const ::std::string &e = "UTF-8",
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeStored(::xercesc::DOMDocument &d,
                     const ::LinphonePrivate::Xsd::Imdn::Stored &x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeStored(const ::LinphonePrivate::Xsd::Imdn::Stored &x,
                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                    ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeForbidden(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeForbidden(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeForbidden(::std::ostream &os,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeForbidden(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &x,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeForbidden(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &x,
                        ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeForbidden(::xercesc::XMLFormatTarget &ft,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &x,
                        ::xercesc::DOMErrorHandler &eh,
                        const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                            ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                        const ::std::string &e = "UTF-8",
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeForbidden(::xercesc::DOMDocument &d,
                        const ::LinphonePrivate::Xsd::Imdn::Forbidden &x,
                        ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeForbidden(const ::LinphonePrivate::Xsd::Imdn::Forbidden &x,
                   const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                       ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                   ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to std::ostream.
//

void serializeError(::std::ostream &os,
                    const ::LinphonePrivate::Xsd::Imdn::Error &x,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                    const ::std::string &e = "UTF-8",
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeError(::std::ostream &os,
                    const ::LinphonePrivate::Xsd::Imdn::Error &x,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                    const ::std::string &e = "UTF-8",
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeError(::std::ostream &os,
                    const ::LinphonePrivate::Xsd::Imdn::Error &x,
                    ::xercesc::DOMErrorHandler &eh,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                    const ::std::string &e = "UTF-8",
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to xercesc::XMLFormatTarget.
//

void serializeError(::xercesc::XMLFormatTarget &ft,
                    const ::LinphonePrivate::Xsd::Imdn::Error &x,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                    const ::std::string &e = "UTF-8",
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeError(::xercesc::XMLFormatTarget &ft,
                    const ::LinphonePrivate::Xsd::Imdn::Error &x,
                    ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler &eh,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                    const ::std::string &e = "UTF-8",
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void serializeError(::xercesc::XMLFormatTarget &ft,
                    const ::LinphonePrivate::Xsd::Imdn::Error &x,
                    ::xercesc::DOMErrorHandler &eh,
                    const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                        ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
                    const ::std::string &e = "UTF-8",
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to an existing xercesc::DOMDocument.
//

void serializeError(::xercesc::DOMDocument &d,
                    const ::LinphonePrivate::Xsd::Imdn::Error &x,
                    ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

// Serialize to a new xercesc::DOMDocument.
//

::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr<::xercesc::DOMDocument>
serializeError(const ::LinphonePrivate::Xsd::Imdn::Error &x,
               const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap &m =
                   ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap(),
               ::LinphonePrivate::Xsd::XmlSchema::Flags f = 0);

void operator<<(::xercesc::DOMElement &, const Imdn &);

void operator<<(::xercesc::DOMElement &, const DeliveryNotification &);

void operator<<(::xercesc::DOMElement &, const Delivered &);

void operator<<(::xercesc::DOMAttr &, const Delivered &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Delivered &);

void operator<<(::xercesc::DOMElement &, const Failed &);

void operator<<(::xercesc::DOMAttr &, const Failed &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Failed &);

void operator<<(::xercesc::DOMElement &, const DisplayNotification &);

void operator<<(::xercesc::DOMElement &, const Displayed &);

void operator<<(::xercesc::DOMAttr &, const Displayed &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Displayed &);

void operator<<(::xercesc::DOMElement &, const ProcessingNotification &);

void operator<<(::xercesc::DOMElement &, const Processed &);

void operator<<(::xercesc::DOMAttr &, const Processed &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Processed &);

void operator<<(::xercesc::DOMElement &, const Stored &);

void operator<<(::xercesc::DOMAttr &, const Stored &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Stored &);

void operator<<(::xercesc::DOMElement &, const Forbidden &);

void operator<<(::xercesc::DOMAttr &, const Forbidden &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Forbidden &);

void operator<<(::xercesc::DOMElement &, const Error &);

void operator<<(::xercesc::DOMAttr &, const Error &);

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &, const Error &);

void operator<<(::xercesc::DOMElement &, const Status &);

void operator<<(::xercesc::DOMElement &, const Status1 &);

void operator<<(::xercesc::DOMElement &, const Status2 &);
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

#endif // IMDN_H
