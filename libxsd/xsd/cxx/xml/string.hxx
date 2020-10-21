// file      : xsd/cxx/xml/string.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_XML_STRING_HXX
#define XSD_CXX_XML_STRING_HXX

#include <string>
#include <cstddef> // std::size_t

#include <xercesc/util/XercesDefs.hpp> // XMLCh

#include <xsd/cxx/config.hxx> // XSD_CXX11

#ifdef XSD_CXX11
#  include <memory> // std::unique_ptr
#else
#  include <xsd/cxx/auto-array.hxx>
#endif

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      // Transcode a null-terminated string.
      //
      template <typename C>
      std::basic_string<C>
      transcode (const XMLCh* s);

      // Transcode a potentially non-null-terminated string.
      //
      template <typename C>
      std::basic_string<C>
      transcode (const XMLCh* s, std::size_t length);


      // For VC wchar_t and XMLCh are the same type so we cannot overload
      // the transcode name. You should not use these functions anyway and
      // instead use the xml::string class below.
      //
      template <typename C>
      XMLCh*
      transcode_to_xmlch (const C*);

      template <typename C>
      XMLCh*
      transcode_to_xmlch (const std::basic_string<C>& s);

      //
      //
      class string
      {
      public :
        template <typename C>
        string (const std::basic_string<C>& s)
            : s_ (transcode_to_xmlch<C> (s)) {}

        template <typename C>
        string (const C* s): s_ (transcode_to_xmlch<C> (s)) {}

        const XMLCh*
        c_str () const {return s_.get ();}

        XMLCh*
        release () {return s_.release ();}

      private:
        string (const string&);

        string&
        operator= (const string&);

      private:
#ifdef XSD_CXX11
        std::unique_ptr<XMLCh[]> s_;
#else
        auto_array<XMLCh> s_;
#endif
      };
    }
  }
}

#endif // XSD_CXX_XML_STRING_HXX

#include <xsd/cxx/xml/string.ixx>
#include <xsd/cxx/xml/string.txx>
