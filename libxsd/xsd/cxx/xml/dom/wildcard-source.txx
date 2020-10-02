// file      : xsd/cxx/xml/dom/wildcard-source.txx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xercesc/util/XMLUniDefs.hpp> // chLatin_L, etc

#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>

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
        create_document ()
        {
          const XMLCh ls[] = {xercesc::chLatin_L,
                              xercesc::chLatin_S,
                              xercesc::chNull};

          // Get an implementation of the Load-Store (LS) interface.
          //
          xercesc::DOMImplementation* impl (
            xercesc::DOMImplementationRegistry::getDOMImplementation (ls));

          return XSD_DOM_AUTO_PTR<xercesc::DOMDocument> (
            impl->createDocument ());
        }
      }
    }
  }
}
