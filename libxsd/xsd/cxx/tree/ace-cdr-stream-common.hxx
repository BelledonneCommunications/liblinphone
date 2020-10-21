// file      : xsd/cxx/tree/ace-cdr-stream-common.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_ACE_CDR_STREAM_COMMON_HXX
#define XSD_CXX_TREE_ACE_CDR_STREAM_COMMON_HXX

#include <xsd/cxx/exceptions.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      // Base exception for ACE CDR insertion/extraction exceptions.
      //
      struct ace_cdr_stream_operation: xsd::cxx::exception
      {
      };
    }
  }
}

#endif  // XSD_CXX_TREE_ACE_CDR_STREAM_COMMON_HXX
