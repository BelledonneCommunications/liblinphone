// file      : xsd/cxx/xml/dom/wildcard-source.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_XML_DOM_WILDCARD_SOURCE_HXX
#define XSD_CXX_XML_DOM_WILDCARD_SOURCE_HXX

#include <xercesc/dom/DOMDocument.hpp>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      namespace dom
      {
        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        create_document ();
      }
    }
  }
}

#include <xsd/cxx/xml/dom/wildcard-source.txx>

#endif // XSD_CXX_XML_DOM_WILDCARD_SOURCE_HXX
