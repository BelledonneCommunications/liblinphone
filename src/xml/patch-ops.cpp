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

#include "patch-ops.h"

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

#include <xsd/cxx/xml/dom/wildcard-source.hxx>

#include <xsd/cxx/xml/dom/parsing-source.hxx>

#include <xsd/cxx/tree/type-factory-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::type_factory_plate<0, char> type_factory_plate_init;
}

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

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Xpath> _xsd_Xpath_type_factory_init("xpath", "");

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

static const ::xsd::cxx::tree::type_factory_initializer<0, char, XpathAdd> _xsd_XpathAdd_type_factory_init("xpath-add",
                                                                                                           "");

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

const Pos::Value Pos::_xsd_Pos_indexes_[3] = {::Pos::after, ::Pos::before, ::Pos::prepend};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Pos> _xsd_Pos_type_factory_init("pos", "");

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

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Type> _xsd_Type_type_factory_init("type", "");

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

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Add> _xsd_Add_type_factory_init("add", "");

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

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Replace> _xsd_Replace_type_factory_init("replace", "");

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

const Ws::Value Ws::_xsd_Ws_indexes_[3] = {::Ws::after, ::Ws::before, ::Ws::both};

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Ws> _xsd_Ws_type_factory_init("ws", "");

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

static const ::xsd::cxx::tree::type_factory_initializer<0, char, Remove> _xsd_Remove_type_factory_init("remove", "");

#include <ostream>

#include <xsd/cxx/tree/std-ostream-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::std_ostream_plate<0, char> std_ostream_plate_init;
}

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

#include <istream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/sax/std-input-source.hxx>

#include <ostream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/dom/serialization-source.hxx>

#include <xsd/cxx/tree/type-serializer-map.hxx>

namespace _xsd {
static const ::xsd::cxx::tree::type_serializer_plate<0, char> type_serializer_plate_init;
}

void operator<<(::xercesc::DOMElement &e, const Xpath &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const Xpath &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const Xpath &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Xpath> _xsd_Xpath_type_serializer_init("xpath", "");

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
    _xsd_XpathAdd_type_serializer_init("xpath-add", "");

void operator<<(::xercesc::DOMElement &e, const Pos &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const Pos &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const Pos &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Pos> _xsd_Pos_type_serializer_init("pos", "");

void operator<<(::xercesc::DOMElement &e, const Type &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const Type &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const Type &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Type> _xsd_Type_type_serializer_init("type", "");

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

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Add> _xsd_Add_type_serializer_init("add", "");

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
    _xsd_Replace_type_serializer_init("replace", "");

void operator<<(::xercesc::DOMElement &e, const Ws &i) {
	e << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::xercesc::DOMAttr &a, const Ws &i) {
	a << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

void operator<<(::LinphonePrivate::Xsd::XmlSchema::ListStream &l, const Ws &i) {
	l << static_cast<const ::LinphonePrivate::Xsd::XmlSchema::String &>(i);
}

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Ws> _xsd_Ws_type_serializer_init("ws", "");

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

static const ::xsd::cxx::tree::type_serializer_initializer<0, char, Remove> _xsd_Remove_type_serializer_init("remove",
                                                                                                             "");

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
