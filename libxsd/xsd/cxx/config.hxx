// file      : xsd/cxx/config.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_CONFIG_HXX
#define XSD_CXX_CONFIG_HXX

#include <xsd/cxx/version.hxx>

// Available C++11 features.
//
#ifdef XSD_CXX11
#ifdef _MSC_VER
#  if _MSC_VER >= 1600
#    define XSD_CXX11_NULLPTR
#    if _MSC_VER >= 1800
#      define XSD_CXX11_TEMPLATE_ALIAS
#    endif
#  endif
#else
#  if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#    ifdef __GNUC__
#      if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
#        define XSD_CXX11_NULLPTR
#      endif
#      if (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || __GNUC__ > 4
#        define XSD_CXX11_TEMPLATE_ALIAS
#      endif
#    else
#      define XSD_CXX11_NULLPTR
#      define XSD_CXX11_TEMPLATE_ALIAS
#    endif
#  endif
#endif
#endif // XSD_CXX11

#ifdef XSD_CXX11
#  define XSD_AUTO_PTR std::unique_ptr
#else
#  define XSD_AUTO_PTR std::auto_ptr
#endif

// Macro to suppress the unused variable warning.
//
#define XSD_UNUSED(x) (void)x

#endif // XSD_CXX_CONFIG_HXX
