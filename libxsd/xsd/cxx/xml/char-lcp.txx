// file      : xsd/cxx/xml/char-lcp.txx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cstring> // std::memcpy

#include <xercesc/util/XMLString.hpp>

#include <xsd/cxx/config.hxx> // XSD_CXX11

#ifdef XSD_CXX11
#  include <memory> // std::unique_ptr
#else
#  include <xsd/cxx/auto-array.hxx>
#endif

#include <xsd/cxx/xml/std-memory-manager.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      template <typename C>
      std::basic_string<C> char_lcp_transcoder<C>::
      to (const XMLCh* s)
      {
        std_memory_manager mm;
#ifdef XSD_CXX11
        std::unique_ptr<C[], std_memory_manager&> r (
#else
        auto_array<C, std_memory_manager> r (
#endif
          xercesc::XMLString::transcode (s, &mm), mm);
        return std::basic_string<C> (r.get ());
      }

      template <typename C>
      std::basic_string<C> char_lcp_transcoder<C>::
      to (const XMLCh* s, std::size_t len)
      {
#ifdef XSD_CXX11
        std::unique_ptr<XMLCh[]> tmp (
#else
        auto_array<XMLCh> tmp (
#endif
          new XMLCh[len + 1]);
        std::memcpy (tmp.get (), s, len * sizeof (XMLCh));
        tmp[len] = XMLCh (0);

        std_memory_manager mm;
#ifdef XSD_CXX11
        std::unique_ptr<C[], std_memory_manager&> r (
#else
        auto_array<C, std_memory_manager> r (
#endif
          xercesc::XMLString::transcode (tmp.get (), &mm), mm);

        tmp.reset ();

        return std::basic_string<C> (r.get ());
      }

      template <typename C>
      XMLCh* char_lcp_transcoder<C>::
      from (const C* s)
      {
        std_memory_manager mm;
        return xercesc::XMLString::transcode (s, &mm);
      }
    }
  }
}
