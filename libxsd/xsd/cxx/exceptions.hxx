// file      : xsd/cxx/exceptions.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_EXCEPTIONS_HXX
#define XSD_CXX_EXCEPTIONS_HXX

#include <exception> // std::exception

namespace xsd
{
  namespace cxx
  {
    struct exception: std::exception
    {
    };
  }
}

#endif  // XSD_CXX_EXCEPTIONS_HXX
