// file      : xsd/cxx/tree/serialization/unsigned-short.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_SERIALIZATION_UNSIGNED_SHORT_HXX
#define XSD_CXX_TREE_SERIALIZATION_UNSIGNED_SHORT_HXX

#include <sstream>

namespace XERCES_CPP_NAMESPACE
{
  inline void
  operator<< (xercesc::DOMElement& e, unsigned short s)
  {
    std::basic_ostringstream<char> os;
    os << s;
    e << os.str ();
  }

  inline void
  operator<< (xercesc::DOMAttr& a, unsigned short s)
  {
    std::basic_ostringstream<char> os;
    os << s;
    a << os.str ();
  }
}

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      template <typename C>
      inline void
      operator<< (list_stream<C>& ls, unsigned short s)
      {
        ls.os_ << s;
      }
    }
  }
}

#endif // XSD_CXX_TREE_SERIALIZATION_UNSIGNED_SHORT_HXX
