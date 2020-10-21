// file      : xsd/cxx/tree/text.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_TEXT_HXX
#define XSD_CXX_TREE_TEXT_HXX

#include <string>

#include <xercesc/dom/DOMElement.hpp>

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      // Throws expected_text_content.
      //
      template <typename C>
      std::basic_string<C>
      text_content (const xercesc::DOMElement&);
    }
  }
}

#include <xsd/cxx/tree/text.txx>

#endif // XSD_CXX_TREE_TEXT_HXX
