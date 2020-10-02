// file      : xsd/cxx/tree/xdr-stream-extraction.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_XDR_STREAM_EXTRACTION_HXX
#define XSD_CXX_TREE_XDR_STREAM_EXTRACTION_HXX

#include <rpc/types.h>
#include <rpc/xdr.h>

// Of course BSD has to be different and name its functions u_intXX
// instead of uintXX. Plus it does not have XX == 8.
//
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
#  if !defined(XSD_CXX_TREE_ASSUME_SUN_XDR) && !defined(xdr_int8_t)
#    define xdr_int8_t(x, y) xdr_char(x, reinterpret_cast<char*> (y))
#    define xdr_uint8_t(x, y) xdr_u_char(x, reinterpret_cast<unsigned char*> (y))
#    define xdr_uint16_t xdr_u_int16_t
#    define xdr_uint32_t xdr_u_int32_t
#    define xdr_uint64_t xdr_u_int64_t
#  endif
#endif

#include <string>

#include <xsd/cxx/tree/buffer.hxx>
#include <xsd/cxx/tree/istream.hxx>
#include <xsd/cxx/tree/xdr-stream-common.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      struct xdr_stream_extraction: xdr_stream_operation
      {
        virtual const char*
        what () const throw ()
        {
          return "XDR stream extraction operation failed";
        }
      };


      // as_size
      //
#ifdef XSD_CXX_TREE_USE_64_BIT_SIZE
      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_size<T>& x)
      {
        uint64_t v;

        if (!xdr_uint64_t (&s.impl (), &v) || v > ~(T (0)))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }
#else
      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_size<T>& x)
      {
        uint32_t v;

        if (!xdr_uint32_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }
#endif


      // 8-bit
      //
      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_int8<T>& x)
      {
        int8_t v;

        if (!xdr_int8_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }

      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_uint8<T>& x)
      {
        uint8_t v;

        if (!xdr_uint8_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }


      // 16-bit
      //
      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_int16<T>& x)
      {
        int16_t v;

        if (!xdr_int16_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }

      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_uint16<T>& x)
      {
        uint16_t v;

        if (!xdr_uint16_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }


      // 32-bit
      //
      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_int32<T>& x)
      {
        int32_t v;

        if (!xdr_int32_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }

      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_uint32<T>& x)
      {
        uint32_t v;

        if (!xdr_uint32_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }


      // 64-bit
      //
      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_int64<T>& x)
      {
        int64_t v;

        if (!xdr_int64_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }

      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_uint64<T>& x)
      {
        uint64_t v;

        if (!xdr_uint64_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }


      // Boolean
      //
      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_bool<T>& x)
      {
        bool_t v;

        if (!xdr_bool (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }


      // Floating-point
      //
      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_float32<T>& x)
      {
        float v;

        if (!xdr_float (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }

      template <typename T>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_float64<T>& x)
      {
        double v;

        if (!xdr_double (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<T> (v);

        return s;
      }

      // Extraction of std::basic_string.
      //

      inline istream<XDR>&
      operator>> (istream<XDR>& s, std::basic_string<char>& x)
      {
        unsigned int n;

        if (!xdr_u_int (&s.impl (), &n))
          throw xdr_stream_extraction ();

        // Dangerous but fast.
        //
	x.clear ();

	if (n != 0)
	{
          x.resize (n);
          char* p (const_cast<char*> (x.c_str ()));

          if (!xdr_opaque (&s.impl (), p, n))
            throw xdr_stream_extraction ();
	}

        return s;
      }

      // Wide strings are not supported by XDR.
      //
      // inline istream<XDR>&
      // operator>> (istream<XDR>& s, std::basic_string<wchar_t>& x)
      // {
      // }


      // Extraction of a binary buffer.
      //
      template <typename C>
      istream<XDR>&
      operator>> (istream<XDR>& s, buffer<C>& x)
      {
        unsigned int n;

        if (!xdr_u_int (&s.impl (), &n))
          throw xdr_stream_extraction ();

        x.size (n);

        if (!xdr_opaque (&s.impl (), x.data (), n))
          throw xdr_stream_extraction ();

        return s;
      }
    }
  }
}

#endif  // XSD_CXX_TREE_XDR_STREAM_EXTRACTION_HXX
