// file      : xsd/cxx/auto-array.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_AUTO_ARRAY_HXX
#define XSD_CXX_AUTO_ARRAY_HXX

#include <xsd/cxx/config.hxx> // XSD_CXX11

#ifdef XSD_CXX11
#  error use std::unique_ptr instead of non-standard auto_array
#endif

#include <cstddef> // std::size_t

namespace xsd
{
  namespace cxx
  {
    template <typename T>
    struct std_array_deleter
    {
      void
      operator() (T* p) const
      {
        delete[] p;
      }
    };

    // Simple automatic array. The second template parameter is
    // an optional deleter type. If not specified, delete[]
    // is used.
    //
    template <typename T, typename D = std_array_deleter<T> >
    struct auto_array
    {
      auto_array (T a[])
          : a_ (a), d_ (0)
      {
      }

      auto_array (T a[], const D& d)
          : a_ (a), d_ (&d)
      {
      }

      ~auto_array ()
      {
        if (d_ != 0)
          (*d_) (a_);
        else
          delete[] a_;
      }

      T&
      operator[] (std::size_t index) const
      {
        return a_[index];
      }

      T*
      get () const
      {
        return a_;
      }

      T*
      release ()
      {
        T* tmp (a_);
        a_ = 0;
        return tmp;
      }

      void
      reset (T a[] = 0)
      {
        if (a_ != a)
        {
          if (d_ != 0)
            (*d_) (a_);
          else
            delete[] a_;

          a_ = a;
        }
      }

      typedef void (auto_array::*bool_convertible)();

      operator bool_convertible () const
      {
        return a_ ? &auto_array<T, D>::true_ : 0;
      }

    private:
      auto_array (const auto_array&);

      auto_array&
      operator= (const auto_array&);

    private:
      void
      true_ ();

    private:
      T* a_;
      const D* d_;
    };

    template <typename T, typename D>
    void auto_array<T, D>::
    true_ ()
    {
    }
  }
}

#endif  // XSD_CXX_AUTO_ARRAY_HXX
