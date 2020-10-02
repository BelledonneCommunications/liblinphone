// file      : xsd/cxx/tree/istream.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_ISTREAM_HXX
#define XSD_CXX_TREE_ISTREAM_HXX

#include <map>
#include <string>
#include <memory>  // std::auto_ptr/unique_ptr
#include <cstddef> // std::size_t

#include <xsd/cxx/config.hxx> // XSD_AUTO_PTR

#include <xsd/cxx/tree/istream-fwd.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      class istream_common
      {
      public:
        template <typename T>
        struct as_size
        {
          explicit as_size (T& x) : x_ (x) {}
          T& x_;
        };


        // 8-bit
        //
        template <typename T>
        struct as_int8
        {
          explicit as_int8 (T& x) : x_ (x) {}
          T& x_;
        };

        template <typename T>
        struct as_uint8
        {
          explicit as_uint8 (T& x) : x_ (x) {}
          T& x_;
        };


        // 16-bit
        //
        template <typename T>
        struct as_int16
        {
          explicit as_int16 (T& x) : x_ (x) {}
          T& x_;
        };

        template <typename T>
        struct as_uint16
        {
          explicit as_uint16 (T& x) : x_ (x) {}
          T& x_;
        };


        // 32-bit
        //
        template <typename T>
        struct as_int32
        {
          explicit as_int32 (T& x) : x_ (x) {}
          T& x_;
        };

        template <typename T>
        struct as_uint32
        {
          explicit as_uint32 (T& x) : x_ (x) {}
          T& x_;
        };


        // 64-bit
        //
        template <typename T>
        struct as_int64
        {
          explicit as_int64 (T& x) : x_ (x) {}
          T& x_;
        };

        template <typename T>
        struct as_uint64
        {
          explicit as_uint64 (T& x) : x_ (x) {}
          T& x_;
        };


        // Boolean
        //
        template <typename T>
        struct as_bool
        {
          explicit as_bool (T& x) : x_ (x) {}
          T& x_;
        };


        // Floating-point
        //
        template <typename T>
        struct as_float32
        {
          explicit as_float32 (T& x) : x_ (x) {}
          T& x_;
        };

        template <typename T>
        struct as_float64
        {
          explicit as_float64 (T& x) : x_ (x) {}
          T& x_;
        };
      };

      template<typename S>
      class istream: public istream_common
      {
      public:
        explicit
        istream (S& s)
            : s_ (s)
        {
        }

        S&
        impl ()
        {
          return s_;
        }

        // Add string to the pool. The application should add every
        // potentially pooled string to correctly re-create the pool
        // constructed during insertion.
        //
        template <typename C>
        void
        pool_add (const std::basic_string<C>& s)
        {
          typedef pool_impl<C> pool_type;

          if (pool_.get () == 0)
            pool_.reset (new pool_type);

          pool_type& p (*static_cast<pool_type*> (pool_.get ()));
          p.push_back (s);
        }

        // Get string from pool id. We return the result via an argument
        // instead of as a return type to avoid difficulties some compilers
        // (e.g., GCC) experience with calls like istream<S>::pool_string<C>.
        //
        template <typename C>
        void
        pool_string (std::size_t id, std::basic_string<C>& out)
        {
          typedef pool_impl<C> pool_type;
          pool_type& p (*static_cast<pool_type*> (pool_.get ()));
          out = p[id - 1];
        }

      public:
        // 8-bit
        //
        signed char
        read_char ();

        unsigned char
        read_uchar ();

        // 16-bit
        //
        unsigned short
        read_short ();

        unsigned short
        read_ushort ();

        // 32-bit
        //
        unsigned int
        read_int ();

        unsigned int
        read_uint ();

        // 64-bit
        //
        unsigned long long
        read_ulonglong ();

        unsigned long long
        read_longlong ();

        // Boolean
        //
        bool
        read_bool ();

        // Floating-point
        //
        float
        read_float ();

        double
        read_double ();

      private:
        istream (const istream&);
        istream&
        operator= (const istream&);

      private:
        struct pool
        {
          virtual
          ~pool () {}
        };

        template <typename C>
        struct pool_impl: pool, std::vector<std::basic_string<C> >
        {
        };

        S& s_;
        std::size_t seq_;
        XSD_AUTO_PTR<pool> pool_;
      };


      // 8-bit
      //
      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, signed char& x)
      {
        istream_common::as_int8<signed char> as_int8 (x);
        return s >> as_int8;
      }

      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, unsigned char& x)
      {
        istream_common::as_uint8<unsigned char> as_uint8 (x);
        return s >> as_uint8;
      }


      // 16-bit
      //
      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, short& x)
      {
        istream_common::as_int16<short> as_int16 (x);
        return s >> as_int16;
      }

      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, unsigned short& x)
      {
        istream_common::as_uint16<unsigned short> as_uint16 (x);
        return s >> as_uint16;
      }


      // 32-bit
      //
      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, int& x)
      {
        istream_common::as_int32<int> as_int32 (x);
        return s >> as_int32;
      }

      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, unsigned int& x)
      {
        istream_common::as_uint32<unsigned int> as_uint32 (x);
        return s >> as_uint32;
      }


      // 64-bit
      //
      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, long long& x)
      {
        istream_common::as_int64<long long> as_int64 (x);
        return s >> as_int64;
      }

      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, unsigned long long& x)
      {
        istream_common::as_uint64<unsigned long long> as_uint64 (x);
        return s >> as_uint64;
      }

      // Boolean
      //
      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, bool& x)
      {
        istream_common::as_bool<bool> as_bool (x);
        return s >> as_bool;
      }


      // Floating-point
      //
      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, float& x)
      {
        istream_common::as_float32<float> as_float32 (x);
        return s >> as_float32;
      }

      template <typename S>
      inline istream<S>&
      operator>> (istream<S>& s, double& x)
      {
        istream_common::as_float64<double> as_float64 (x);
        return s >> as_float64;
      }

      //
      // read_* functions.
      //

      template <typename S>
      inline signed char istream<S>::
      read_char ()
      {
        signed char r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline unsigned char istream<S>::
      read_uchar ()
      {
        unsigned char r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline unsigned short istream<S>::
      read_short ()
      {
        short r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline unsigned short istream<S>::
      read_ushort ()
      {
        unsigned short r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline unsigned int istream<S>::
      read_int ()
      {
        int r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline unsigned int istream<S>::
      read_uint ()
      {
        unsigned int r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline unsigned long long istream<S>::
      read_ulonglong ()
      {
        long long r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline unsigned long long istream<S>::
      read_longlong ()
      {
        unsigned long long r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline bool istream<S>::
      read_bool ()
      {
        bool r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline float istream<S>::
      read_float ()
      {
        float r;
        *this >> r;
        return r;
      }

      template <typename S>
      inline double istream<S>::
      read_double ()
      {
        double r;
        *this >> r;
        return r;
      }
    }
  }
}

#endif  // XSD_CXX_TREE_ISTREAM_HXX
