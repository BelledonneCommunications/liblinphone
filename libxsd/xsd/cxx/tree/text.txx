// file      : xsd/cxx/tree/text.txx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xercesc/dom/DOMText.hpp>

#include <xsd/cxx/xml/string.hxx>

#include <xsd/cxx/tree/exceptions.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      template <typename C>
      std::basic_string<C>
      text_content (const xercesc::DOMElement& e)
      {
        using xercesc::DOMNode;
        using xercesc::DOMText;

        DOMNode* n (e.getFirstChild ());

        // Fast path.
        //
        if (n != 0 &&
            n->getNodeType () == DOMNode::TEXT_NODE &&
            n->getNextSibling () == 0)
        {
          DOMText* t (static_cast<DOMText*> (n));
          return xml::transcode<C> (t->getData (), t->getLength ());
        }

        std::basic_string<C> r;

        for (; n != 0; n = n->getNextSibling ())
        {
          switch (n->getNodeType ())
          {
          case DOMNode::TEXT_NODE:
          case DOMNode::CDATA_SECTION_NODE:
            {
              DOMText* t (static_cast<DOMText*> (n));
              r += xml::transcode<C> (t->getData (), t->getLength ());
              break;
            }
          case DOMNode::ELEMENT_NODE:
            {
              throw expected_text_content<C> ();
            }
          default:
            break; // ignore
          }
        }

        return r;
      }
    }
  }
}
