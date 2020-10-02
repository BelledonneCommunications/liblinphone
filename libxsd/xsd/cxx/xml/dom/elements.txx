// file      : xsd/cxx/xml/dom/elements.txx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd/cxx/xml/string.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      namespace dom
      {
        template <typename C>
        qualified_name<C>
        name (const xercesc::DOMAttr& a)
        {
          const XMLCh* n (a.getLocalName ());

          // If this DOM doesn't support namespaces then use getName.
          //
          if (n != 0)
          {
            if (const XMLCh* ns = a.getNamespaceURI ())
              return qualified_name<C> (transcode<C> (n), transcode<C> (ns));
            else
              return qualified_name<C> (transcode<C> (n));
          }
          else
            return qualified_name<C> (transcode<C> (a.getName ()));
        }


        template <typename C>
        qualified_name<C>
        name (const xercesc::DOMElement& e)
        {
          const XMLCh* n (e.getLocalName ());

          // If this DOM doesn't support namespaces then use getTagName.
          //
          if (n != 0)
          {
            if (const XMLCh* ns = e.getNamespaceURI ())
              return qualified_name<C> (transcode<C> (n), transcode<C> (ns));
            else
              return qualified_name<C> (transcode<C> (n));
          }
          else
            return qualified_name<C> (transcode<C> (e.getTagName ()));
        }
      }
    }
  }
}
