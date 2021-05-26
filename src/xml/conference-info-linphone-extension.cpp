/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

//
// Furthermore, Code Synthesis Tools CC makes a special exception for
// the Free/Libre and Open Source Software (FLOSS) which is described
// in the accompanying FLOSSE file.
//

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
#if __GNUC__ >=7
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
//
// End prologue.

#include <xsd/cxx/pre.hxx>

#include "conference-info-linphone-extension.h"

namespace LinphonePrivate
{
  namespace Xsd
  {
    namespace ConferenceInfoLinphoneExtension
    {
      // ConferenceTypeLinphoneExtension
      // 

      const ConferenceTypeLinphoneExtension::EphemeralOptional& ConferenceTypeLinphoneExtension::
      getEphemeral () const
      {
        return this->ephemeral_;
      }

      ConferenceTypeLinphoneExtension::EphemeralOptional& ConferenceTypeLinphoneExtension::
      getEphemeral ()
      {
        return this->ephemeral_;
      }

      void ConferenceTypeLinphoneExtension::
      setEphemeral (const EphemeralType& x)
      {
        this->ephemeral_.set (x);
      }

      void ConferenceTypeLinphoneExtension::
      setEphemeral (const EphemeralOptional& x)
      {
        this->ephemeral_ = x;
      }

      void ConferenceTypeLinphoneExtension::
      setEphemeral (::std::unique_ptr< EphemeralType > x)
      {
        this->ephemeral_.set (std::move (x));
      }

      const ConferenceTypeLinphoneExtension::EntityType& ConferenceTypeLinphoneExtension::
      getEntity () const
      {
        return this->entity_.get ();
      }

      ConferenceTypeLinphoneExtension::EntityType& ConferenceTypeLinphoneExtension::
      getEntity ()
      {
        return this->entity_.get ();
      }

      void ConferenceTypeLinphoneExtension::
      setEntity (const EntityType& x)
      {
        this->entity_.set (x);
      }

      void ConferenceTypeLinphoneExtension::
      setEntity (::std::unique_ptr< EntityType > x)
      {
        this->entity_.set (std::move (x));
      }

      ::std::unique_ptr< ConferenceTypeLinphoneExtension::EntityType > ConferenceTypeLinphoneExtension::
      setDetachEntity ()
      {
        return this->entity_.detach ();
      }

      const ConferenceTypeLinphoneExtension::AnyAttributeSet& ConferenceTypeLinphoneExtension::
      getAnyAttribute () const
      {
        return this->any_attribute_;
      }

      ConferenceTypeLinphoneExtension::AnyAttributeSet& ConferenceTypeLinphoneExtension::
      getAnyAttribute ()
      {
        return this->any_attribute_;
      }

      void ConferenceTypeLinphoneExtension::
      setAnyAttribute (const AnyAttributeSet& s)
      {
        this->any_attribute_ = s;
      }

      const ::xercesc::DOMDocument& ConferenceTypeLinphoneExtension::
      getDomDocument () const
      {
        return *this->dom_document_;
      }

      ::xercesc::DOMDocument& ConferenceTypeLinphoneExtension::
      getDomDocument ()
      {
        return *this->dom_document_;
      }


      // EphemeralType
      // 

      const EphemeralType::LifetimeOptional& EphemeralType::
      getLifetime () const
      {
        return this->lifetime_;
      }

      EphemeralType::LifetimeOptional& EphemeralType::
      getLifetime ()
      {
        return this->lifetime_;
      }

      void EphemeralType::
      setLifetime (const LifetimeType& x)
      {
        this->lifetime_.set (x);
      }

      void EphemeralType::
      setLifetime (const LifetimeOptional& x)
      {
        this->lifetime_ = x;
      }

      void EphemeralType::
      setLifetime (::std::unique_ptr< LifetimeType > x)
      {
        this->lifetime_.set (std::move (x));
      }

      const EphemeralType::ModeOptional& EphemeralType::
      getMode () const
      {
        return this->mode_;
      }

      EphemeralType::ModeOptional& EphemeralType::
      getMode ()
      {
        return this->mode_;
      }

      void EphemeralType::
      setMode (const ModeType& x)
      {
        this->mode_.set (x);
      }

      void EphemeralType::
      setMode (const ModeOptional& x)
      {
        this->mode_ = x;
      }

      void EphemeralType::
      setMode (::std::unique_ptr< ModeType > x)
      {
        this->mode_.set (std::move (x));
      }

      const EphemeralType::AnyAttributeSet& EphemeralType::
      getAnyAttribute () const
      {
        return this->any_attribute_;
      }

      EphemeralType::AnyAttributeSet& EphemeralType::
      getAnyAttribute ()
      {
        return this->any_attribute_;
      }

      void EphemeralType::
      setAnyAttribute (const AnyAttributeSet& s)
      {
        this->any_attribute_ = s;
      }

      const ::xercesc::DOMDocument& EphemeralType::
      getDomDocument () const
      {
        return *this->dom_document_;
      }

      ::xercesc::DOMDocument& EphemeralType::
      getDomDocument ()
      {
        return *this->dom_document_;
      }
    }
  }
}

#include <xsd/cxx/xml/dom/wildcard-source.hxx>

#include <xsd/cxx/xml/dom/parsing-source.hxx>

#include <xsd/cxx/tree/type-factory-map.hxx>

namespace _xsd
{
  static
  const ::xsd::cxx::tree::type_factory_plate< 0, char >
  type_factory_plate_init;
}

namespace LinphonePrivate
{
  namespace Xsd
  {
    namespace ConferenceInfoLinphoneExtension
    {
      // ConferenceTypeLinphoneExtension
      //

      ConferenceTypeLinphoneExtension::
      ConferenceTypeLinphoneExtension (const EntityType& entity)
      : ::LinphonePrivate::Xsd::XmlSchema::Type (),
        dom_document_ (::xsd::cxx::xml::dom::create_document< char > ()),
        ephemeral_ (this),
        entity_ (entity, this),
        any_attribute_ (this->getDomDocument ())
      {
      }

      ConferenceTypeLinphoneExtension::
      ConferenceTypeLinphoneExtension (const ConferenceTypeLinphoneExtension& x,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container* c)
      : ::LinphonePrivate::Xsd::XmlSchema::Type (x, f, c),
        dom_document_ (::xsd::cxx::xml::dom::create_document< char > ()),
        ephemeral_ (x.ephemeral_, f, this),
        entity_ (x.entity_, f, this),
        any_attribute_ (x.any_attribute_, this->getDomDocument ())
      {
      }

      ConferenceTypeLinphoneExtension::
      ConferenceTypeLinphoneExtension (const ::xercesc::DOMElement& e,
                                       ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                       ::LinphonePrivate::Xsd::XmlSchema::Container* c)
      : ::LinphonePrivate::Xsd::XmlSchema::Type (e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
        dom_document_ (::xsd::cxx::xml::dom::create_document< char > ()),
        ephemeral_ (this),
        entity_ (this),
        any_attribute_ (this->getDomDocument ())
      {
        if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0)
        {
          ::xsd::cxx::xml::dom::parser< char > p (e, true, false, true);
          this->parse (p, f);
        }
      }

      void ConferenceTypeLinphoneExtension::
      parse (::xsd::cxx::xml::dom::parser< char >& p,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        for (; p.more_content (); p.next_content (false))
        {
          const ::xercesc::DOMElement& i (p.cur_element ());
          const ::xsd::cxx::xml::qualified_name< char > n (
            ::xsd::cxx::xml::dom::name< char > (i));

          // ephemeral
          //
          if (n.name () == "ephemeral" && n.namespace_ () == "linphone:xml:ns:conference-info-linphone-extension")
          {
            ::std::unique_ptr< EphemeralType > r (
              EphemeralTraits::create (i, f, this));

            if (!this->ephemeral_)
            {
              this->ephemeral_.set (::std::move (r));
              continue;
            }
          }

          break;
        }

        while (p.more_attributes ())
        {
          const ::xercesc::DOMAttr& i (p.next_attribute ());
          const ::xsd::cxx::xml::qualified_name< char > n (
            ::xsd::cxx::xml::dom::name< char > (i));

          if (n.name () == "entity" && n.namespace_ ().empty ())
          {
            this->entity_.set (EntityTraits::create (i, f, this));
            continue;
          }

          // any_attribute
          //
          if ((!n.namespace_ ().empty () &&
               n.namespace_ () != "linphone:xml:ns:conference-info-linphone-extension" &&
               n.namespace_ () != ::xsd::cxx::xml::bits::xmlns_namespace< char > () &&
               n.namespace_ () != ::xsd::cxx::xml::bits::xsi_namespace< char > ()))
          {
            ::xercesc::DOMAttr* r (
              static_cast< ::xercesc::DOMAttr* > (
                this->getDomDocument ().importNode (
                  const_cast< ::xercesc::DOMAttr* > (&i), true)));
            this->any_attribute_ .insert (r);
            continue;
          }
        }

        if (!entity_.present ())
        {
          throw ::xsd::cxx::tree::expected_attribute< char > (
            "entity",
            "");
        }
      }

      ConferenceTypeLinphoneExtension* ConferenceTypeLinphoneExtension::
      _clone (::LinphonePrivate::Xsd::XmlSchema::Flags f,
              ::LinphonePrivate::Xsd::XmlSchema::Container* c) const
      {
        return new class ConferenceTypeLinphoneExtension (*this, f, c);
      }

      ConferenceTypeLinphoneExtension& ConferenceTypeLinphoneExtension::
      operator= (const ConferenceTypeLinphoneExtension& x)
      {
        if (this != &x)
        {
          static_cast< ::LinphonePrivate::Xsd::XmlSchema::Type& > (*this) = x;
          this->ephemeral_ = x.ephemeral_;
          this->entity_ = x.entity_;
          this->any_attribute_ = x.any_attribute_;
        }

        return *this;
      }

      ConferenceTypeLinphoneExtension::
      ~ConferenceTypeLinphoneExtension ()
      {
      }

      // EphemeralType
      //

      EphemeralType::
      EphemeralType ()
      : ::LinphonePrivate::Xsd::XmlSchema::Type (),
        dom_document_ (::xsd::cxx::xml::dom::create_document< char > ()),
        lifetime_ (this),
        mode_ (this),
        any_attribute_ (this->getDomDocument ())
      {
      }

      EphemeralType::
      EphemeralType (const EphemeralType& x,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container* c)
      : ::LinphonePrivate::Xsd::XmlSchema::Type (x, f, c),
        dom_document_ (::xsd::cxx::xml::dom::create_document< char > ()),
        lifetime_ (x.lifetime_, f, this),
        mode_ (x.mode_, f, this),
        any_attribute_ (x.any_attribute_, this->getDomDocument ())
      {
      }

      EphemeralType::
      EphemeralType (const ::xercesc::DOMElement& e,
                     ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                     ::LinphonePrivate::Xsd::XmlSchema::Container* c)
      : ::LinphonePrivate::Xsd::XmlSchema::Type (e, f | ::LinphonePrivate::Xsd::XmlSchema::Flags::base, c),
        dom_document_ (::xsd::cxx::xml::dom::create_document< char > ()),
        lifetime_ (this),
        mode_ (this),
        any_attribute_ (this->getDomDocument ())
      {
        if ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::base) == 0)
        {
          ::xsd::cxx::xml::dom::parser< char > p (e, true, false, true);
          this->parse (p, f);
        }
      }

      void EphemeralType::
      parse (::xsd::cxx::xml::dom::parser< char >& p,
             ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        for (; p.more_content (); p.next_content (false))
        {
          const ::xercesc::DOMElement& i (p.cur_element ());
          const ::xsd::cxx::xml::qualified_name< char > n (
            ::xsd::cxx::xml::dom::name< char > (i));

          // lifetime
          //
          if (n.name () == "lifetime" && n.namespace_ () == "linphone:xml:ns:conference-info-linphone-extension")
          {
            ::std::unique_ptr< LifetimeType > r (
              LifetimeTraits::create (i, f, this));

            if (!this->lifetime_)
            {
              this->lifetime_.set (::std::move (r));
              continue;
            }
          }

          // mode
          //
          if (n.name () == "mode" && n.namespace_ () == "linphone:xml:ns:conference-info-linphone-extension")
          {
            ::std::unique_ptr< ModeType > r (
              ModeTraits::create (i, f, this));

            if (!this->mode_)
            {
              this->mode_.set (::std::move (r));
              continue;
            }
          }

          break;
        }

        while (p.more_attributes ())
        {
          const ::xercesc::DOMAttr& i (p.next_attribute ());
          const ::xsd::cxx::xml::qualified_name< char > n (
            ::xsd::cxx::xml::dom::name< char > (i));

          // any_attribute
          //
          if ((!n.namespace_ ().empty () &&
               n.namespace_ () != "linphone:xml:ns:conference-info-linphone-extension" &&
               n.namespace_ () != ::xsd::cxx::xml::bits::xmlns_namespace< char > () &&
               n.namespace_ () != ::xsd::cxx::xml::bits::xsi_namespace< char > ()))
          {
            ::xercesc::DOMAttr* r (
              static_cast< ::xercesc::DOMAttr* > (
                this->getDomDocument ().importNode (
                  const_cast< ::xercesc::DOMAttr* > (&i), true)));
            this->any_attribute_ .insert (r);
            continue;
          }
        }
      }

      EphemeralType* EphemeralType::
      _clone (::LinphonePrivate::Xsd::XmlSchema::Flags f,
              ::LinphonePrivate::Xsd::XmlSchema::Container* c) const
      {
        return new class EphemeralType (*this, f, c);
      }

      EphemeralType& EphemeralType::
      operator= (const EphemeralType& x)
      {
        if (this != &x)
        {
          static_cast< ::LinphonePrivate::Xsd::XmlSchema::Type& > (*this) = x;
          this->lifetime_ = x.lifetime_;
          this->mode_ = x.mode_;
          this->any_attribute_ = x.any_attribute_;
        }

        return *this;
      }

      EphemeralType::
      ~EphemeralType ()
      {
      }
    }
  }
}

#include <ostream>

#include <xsd/cxx/tree/std-ostream-map.hxx>

namespace _xsd
{
  static
  const ::xsd::cxx::tree::std_ostream_plate< 0, char >
  std_ostream_plate_init;
}

namespace LinphonePrivate
{
  namespace Xsd
  {
    namespace ConferenceInfoLinphoneExtension
    {
      ::std::ostream&
      operator<< (::std::ostream& o, const ConferenceTypeLinphoneExtension& i)
      {
        if (i.getEphemeral ())
        {
          o << ::std::endl << "ephemeral: " << *i.getEphemeral ();
        }

        o << ::std::endl << "entity: " << i.getEntity ();
        return o;
      }

      ::std::ostream&
      operator<< (::std::ostream& o, const EphemeralType& i)
      {
        if (i.getLifetime ())
        {
          o << ::std::endl << "lifetime: " << *i.getLifetime ();
        }

        if (i.getMode ())
        {
          o << ::std::endl << "mode: " << *i.getMode ();
        }

        return o;
      }
    }
  }
}

#include <istream>
#include <xsd/cxx/xml/sax/std-input-source.hxx>
#include <xsd/cxx/tree/error-handler.hxx>

namespace LinphonePrivate
{
  namespace Xsd
  {
    namespace ConferenceInfoLinphoneExtension
    {
      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (const ::std::string& u,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::xml::auto_initializer i (
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

        ::xsd::cxx::tree::error_handler< char > h;

        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::xsd::cxx::xml::dom::parse< char > (
            u, h, p, f));

        h.throw_if_failed< ::xsd::cxx::tree::parsing< char > > ();

        return ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (
            std::move (d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (const ::std::string& u,
                                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler& h,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::xml::auto_initializer i (
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::xsd::cxx::xml::dom::parse< char > (
            u, h, p, f));

        if (!d.get ())
          throw ::xsd::cxx::tree::parsing< char > ();

        return ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (
            std::move (d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (const ::std::string& u,
                                            ::xercesc::DOMErrorHandler& h,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::xsd::cxx::xml::dom::parse< char > (
            u, h, p, f));

        if (!d.get ())
          throw ::xsd::cxx::tree::parsing< char > ();

        return ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (
            std::move (d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::std::istream& is,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::xml::auto_initializer i (
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

        ::xsd::cxx::xml::sax::std_input_source isrc (is);
        return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (isrc, f, p);
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::std::istream& is,
                                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler& h,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::xml::auto_initializer i (
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

        ::xsd::cxx::xml::sax::std_input_source isrc (is);
        return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (isrc, h, f, p);
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::std::istream& is,
                                            ::xercesc::DOMErrorHandler& h,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::xml::sax::std_input_source isrc (is);
        return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (isrc, h, f, p);
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::std::istream& is,
                                            const ::std::string& sid,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::xml::auto_initializer i (
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

        ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
        return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (isrc, f, p);
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::std::istream& is,
                                            const ::std::string& sid,
                                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler& h,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::xml::auto_initializer i (
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0,
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) == 0);

        ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
        return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (isrc, h, f, p);
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::std::istream& is,
                                            const ::std::string& sid,
                                            ::xercesc::DOMErrorHandler& h,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
        return ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (isrc, h, f, p);
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::xercesc::InputSource& i,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::xsd::cxx::tree::error_handler< char > h;

        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::xsd::cxx::xml::dom::parse< char > (
            i, h, p, f));

        h.throw_if_failed< ::xsd::cxx::tree::parsing< char > > ();

        return ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (
            std::move (d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::xercesc::InputSource& i,
                                            ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler& h,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::xsd::cxx::xml::dom::parse< char > (
            i, h, p, f));

        if (!d.get ())
          throw ::xsd::cxx::tree::parsing< char > ();

        return ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (
            std::move (d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::xercesc::InputSource& i,
                                            ::xercesc::DOMErrorHandler& h,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::xsd::cxx::xml::dom::parse< char > (
            i, h, p, f));

        if (!d.get ())
          throw ::xsd::cxx::tree::parsing< char > ();

        return ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (
            std::move (d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (const ::xercesc::DOMDocument& doc,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties& p)
      {
        if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
        {
          ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
            static_cast< ::xercesc::DOMDocument* > (doc.cloneNode (true)));

          return ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > (
            ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::parseConferenceInfoLinphoneExtension (
              std::move (d), f | ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom, p));
        }

        const ::xercesc::DOMElement& e (*doc.getDocumentElement ());
        const ::xsd::cxx::xml::qualified_name< char > n (
          ::xsd::cxx::xml::dom::name< char > (e));

        if (n.name () == "conference-info-linphone-extension" &&
            n.namespace_ () == "linphone:xml:ns:conference-info-linphone-extension")
        {
          ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > r (
            ::xsd::cxx::tree::traits< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension, char >::create (
              e, f, 0));
          return r;
        }

        throw ::xsd::cxx::tree::unexpected_element < char > (
          n.name (),
          n.namespace_ (),
          "conference-info-linphone-extension",
          "linphone:xml:ns:conference-info-linphone-extension");
      }

      ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension >
      parseConferenceInfoLinphoneExtension (::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d,
                                            ::LinphonePrivate::Xsd::XmlSchema::Flags f,
                                            const ::LinphonePrivate::Xsd::XmlSchema::Properties&)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > c (
          ((f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom) &&
           !(f & ::LinphonePrivate::Xsd::XmlSchema::Flags::own_dom))
          ? static_cast< ::xercesc::DOMDocument* > (d->cloneNode (true))
          : 0);

        ::xercesc::DOMDocument& doc (c.get () ? *c : *d);
        const ::xercesc::DOMElement& e (*doc.getDocumentElement ());

        const ::xsd::cxx::xml::qualified_name< char > n (
          ::xsd::cxx::xml::dom::name< char > (e));

        if (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::keep_dom)
          doc.setUserData (::LinphonePrivate::Xsd::XmlSchema::dom::treeNodeKey,
                           (c.get () ? &c : &d),
                           0);

        if (n.name () == "conference-info-linphone-extension" &&
            n.namespace_ () == "linphone:xml:ns:conference-info-linphone-extension")
        {
          ::std::unique_ptr< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension > r (
            ::xsd::cxx::tree::traits< ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension, char >::create (
              e, f, 0));
          return r;
        }

        throw ::xsd::cxx::tree::unexpected_element < char > (
          n.name (),
          n.namespace_ (),
          "conference-info-linphone-extension",
          "linphone:xml:ns:conference-info-linphone-extension");
      }
    }
  }
}

#include <ostream>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/dom/serialization-source.hxx>

#include <xsd/cxx/tree/type-serializer-map.hxx>

namespace _xsd
{
  static
  const ::xsd::cxx::tree::type_serializer_plate< 0, char >
  type_serializer_plate_init;
}

namespace LinphonePrivate
{
  namespace Xsd
  {
    namespace ConferenceInfoLinphoneExtension
    {
      void
      serializeConferenceInfoLinphoneExtension (::std::ostream& o,
                                                const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension& s,
                                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap& m,
                                                const ::std::string& e,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        ::xsd::cxx::xml::auto_initializer i (
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeConferenceInfoLinphoneExtension (s, m, f));

        ::xsd::cxx::tree::error_handler< char > h;

        ::xsd::cxx::xml::dom::ostream_format_target t (o);
        if (!::xsd::cxx::xml::dom::serialize (t, *d, e, h, f))
        {
          h.throw_if_failed< ::xsd::cxx::tree::serialization< char > > ();
        }
      }

      void
      serializeConferenceInfoLinphoneExtension (::std::ostream& o,
                                                const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension& s,
                                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler& h,
                                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap& m,
                                                const ::std::string& e,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        ::xsd::cxx::xml::auto_initializer i (
          (f & ::LinphonePrivate::Xsd::XmlSchema::Flags::dont_initialize) == 0);

        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeConferenceInfoLinphoneExtension (s, m, f));
        ::xsd::cxx::xml::dom::ostream_format_target t (o);
        if (!::xsd::cxx::xml::dom::serialize (t, *d, e, h, f))
        {
          throw ::xsd::cxx::tree::serialization< char > ();
        }
      }

      void
      serializeConferenceInfoLinphoneExtension (::std::ostream& o,
                                                const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension& s,
                                                ::xercesc::DOMErrorHandler& h,
                                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap& m,
                                                const ::std::string& e,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeConferenceInfoLinphoneExtension (s, m, f));
        ::xsd::cxx::xml::dom::ostream_format_target t (o);
        if (!::xsd::cxx::xml::dom::serialize (t, *d, e, h, f))
        {
          throw ::xsd::cxx::tree::serialization< char > ();
        }
      }

      void
      serializeConferenceInfoLinphoneExtension (::xercesc::XMLFormatTarget& t,
                                                const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension& s,
                                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap& m,
                                                const ::std::string& e,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeConferenceInfoLinphoneExtension (s, m, f));

        ::xsd::cxx::tree::error_handler< char > h;

        if (!::xsd::cxx::xml::dom::serialize (t, *d, e, h, f))
        {
          h.throw_if_failed< ::xsd::cxx::tree::serialization< char > > ();
        }
      }

      void
      serializeConferenceInfoLinphoneExtension (::xercesc::XMLFormatTarget& t,
                                                const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension& s,
                                                ::LinphonePrivate::Xsd::XmlSchema::ErrorHandler& h,
                                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap& m,
                                                const ::std::string& e,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeConferenceInfoLinphoneExtension (s, m, f));
        if (!::xsd::cxx::xml::dom::serialize (t, *d, e, h, f))
        {
          throw ::xsd::cxx::tree::serialization< char > ();
        }
      }

      void
      serializeConferenceInfoLinphoneExtension (::xercesc::XMLFormatTarget& t,
                                                const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension& s,
                                                ::xercesc::DOMErrorHandler& h,
                                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap& m,
                                                const ::std::string& e,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeConferenceInfoLinphoneExtension (s, m, f));
        if (!::xsd::cxx::xml::dom::serialize (t, *d, e, h, f))
        {
          throw ::xsd::cxx::tree::serialization< char > ();
        }
      }

      void
      serializeConferenceInfoLinphoneExtension (::xercesc::DOMDocument& d,
                                                const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension& s,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags)
      {
        ::xercesc::DOMElement& e (*d.getDocumentElement ());
        const ::xsd::cxx::xml::qualified_name< char > n (
          ::xsd::cxx::xml::dom::name< char > (e));

        if (n.name () == "conference-info-linphone-extension" &&
            n.namespace_ () == "linphone:xml:ns:conference-info-linphone-extension")
        {
          e << s;
        }
        else
        {
          throw ::xsd::cxx::tree::unexpected_element < char > (
            n.name (),
            n.namespace_ (),
            "conference-info-linphone-extension",
            "linphone:xml:ns:conference-info-linphone-extension");
        }
      }

      ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument >
      serializeConferenceInfoLinphoneExtension (const ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::ConferenceTypeLinphoneExtension& s,
                                                const ::LinphonePrivate::Xsd::XmlSchema::NamespaceInfomap& m,
                                                ::LinphonePrivate::Xsd::XmlSchema::Flags f)
      {
        ::LinphonePrivate::Xsd::XmlSchema::dom::unique_ptr< ::xercesc::DOMDocument > d (
          ::xsd::cxx::xml::dom::serialize< char > (
            "conference-info-linphone-extension",
            "linphone:xml:ns:conference-info-linphone-extension",
            m, f));

        ::LinphonePrivate::Xsd::ConferenceInfoLinphoneExtension::serializeConferenceInfoLinphoneExtension (*d, s, f);
        return d;
      }

      void
      operator<< (::xercesc::DOMElement& e, const ConferenceTypeLinphoneExtension& i)
      {
        e << static_cast< const ::LinphonePrivate::Xsd::XmlSchema::Type& > (i);

        // any_attribute
        //
        for (ConferenceTypeLinphoneExtension::AnyAttributeConstIterator
             b (i.getAnyAttribute ().begin ()), n (i.getAnyAttribute ().end ());
             b != n; ++b)
        {
          ::xercesc::DOMAttr* a (
            static_cast< ::xercesc::DOMAttr* > (
              e.getOwnerDocument ()->importNode (
                const_cast< ::xercesc::DOMAttr* > (&(*b)), true)));

          if (a->getLocalName () == 0)
            e.setAttributeNode (a);
          else
            e.setAttributeNodeNS (a);
        }

        // ephemeral
        //
        if (i.getEphemeral ())
        {
          ::xercesc::DOMElement& s (
            ::xsd::cxx::xml::dom::create_element (
              "ephemeral",
              "linphone:xml:ns:conference-info-linphone-extension",
              e));

          s << *i.getEphemeral ();
        }

        // entity
        //
        {
          ::xercesc::DOMAttr& a (
            ::xsd::cxx::xml::dom::create_attribute (
              "entity",
              e));

          a << i.getEntity ();
        }
      }

      void
      operator<< (::xercesc::DOMElement& e, const EphemeralType& i)
      {
        e << static_cast< const ::LinphonePrivate::Xsd::XmlSchema::Type& > (i);

        // any_attribute
        //
        for (EphemeralType::AnyAttributeConstIterator
             b (i.getAnyAttribute ().begin ()), n (i.getAnyAttribute ().end ());
             b != n; ++b)
        {
          ::xercesc::DOMAttr* a (
            static_cast< ::xercesc::DOMAttr* > (
              e.getOwnerDocument ()->importNode (
                const_cast< ::xercesc::DOMAttr* > (&(*b)), true)));

          if (a->getLocalName () == 0)
            e.setAttributeNode (a);
          else
            e.setAttributeNodeNS (a);
        }

        // lifetime
        //
        if (i.getLifetime ())
        {
          ::xercesc::DOMElement& s (
            ::xsd::cxx::xml::dom::create_element (
              "lifetime",
              "linphone:xml:ns:conference-info-linphone-extension",
              e));

          s << *i.getLifetime ();
        }

        // mode
        //
        if (i.getMode ())
        {
          ::xercesc::DOMElement& s (
            ::xsd::cxx::xml::dom::create_element (
              "mode",
              "linphone:xml:ns:conference-info-linphone-extension",
              e));

          s << *i.getMode ();
        }
      }
    }
  }
}

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

