// file      : xsd/cxx/xml/dom/auto-ptr.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_XML_DOM_AUTO_PTR_HXX
#define XSD_CXX_XML_DOM_AUTO_PTR_HXX

#include <xsd/cxx/config.hxx> // XSD_CXX11_*

#ifdef XSD_CXX11
#  include <memory>      // std::unique_ptr
#  include <utility>     // std::move
#  include <type_traits> // std::remove_const
#endif

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      namespace dom
      {
#ifdef XSD_CXX11
        template <typename T>
        struct deleter
        {
          void
          operator() (T* p) const
          {
            if (p != 0)
              const_cast<typename std::remove_const<T>::type*> (p)->release ();
          }
        };

#ifdef XSD_CXX11_TEMPLATE_ALIAS
        template <typename T>
        using unique_ptr = std::unique_ptr<T, deleter<T>>;
#else
        template <typename T>
        class unique_ptr: public std::unique_ptr<T, deleter<T>>
        {
        public:
          typedef std::unique_ptr<T, deleter<T>> base;

          typedef typename base::pointer pointer;
          typedef T element_type;
          typedef deleter<T> deleter_type;

          unique_ptr (): base () {}
          explicit unique_ptr (pointer p): base (p) {}
          unique_ptr (pointer p, const deleter_type& d): base (p, d) {}
          unique_ptr (pointer p, deleter_type&& d): base (p, std::move (d)) {}
          unique_ptr (unique_ptr&& p): base (std::move (p)) {}
          template <class T1>
          unique_ptr (unique_ptr<T1>&& p): base (std::move (p)) {}
          template <class T1>
          unique_ptr (std::auto_ptr<T1>&& p): base (std::move (p)) {}

          unique_ptr& operator= (unique_ptr&& p)
          {
            static_cast<base&> (*this) = std::move (p);
            return *this;
          }

          template <class T1>
          unique_ptr& operator= (unique_ptr<T1>&& p)
          {
            static_cast<base&> (*this) = std::move (p);
            return *this;
          }

#ifdef XSD_CXX11_NULLPTR
          unique_ptr (std::nullptr_t p): base (p) {}

          unique_ptr& operator= (std::nullptr_t p)
          {
            static_cast<base&> (*this) = p;
            return *this;
          }
#endif
        };
#endif // XSD_CXX11_TEMPLATE_ALIAS

#define XSD_DOM_AUTO_PTR xsd::cxx::xml::dom::unique_ptr

#else
        // Simple auto_ptr version for C++98 that calls release() instead
        // of delete.
        //
        template <typename T>
        struct remove_c
        {
          typedef T r;
        };

        template <typename T>
        struct remove_c<const T>
        {
          typedef T r;
        };

        template <typename T>
        struct auto_ptr_ref
        {
          T* x_;

          explicit
          auto_ptr_ref (T* x)
              : x_ (x)
          {
          }
        };

        template <typename T>
        struct auto_ptr
        {
          ~auto_ptr ()
          {
            reset ();
          }

          explicit
          auto_ptr (T* x = 0)
              : x_ (x)
          {
          }

          auto_ptr (auto_ptr& y)
              : x_ (y.release ())
          {
          }

          template <typename T2>
          auto_ptr (auto_ptr<T2>& y)
              : x_ (y.release ())
          {
          }

          auto_ptr (auto_ptr_ref<T> r)
              : x_ (r.x_)
          {
          }

          auto_ptr&
          operator= (auto_ptr& y)
          {
            if (x_ != y.x_)
              reset (y.release ());

            return *this;
          }

          template <typename T2>
          auto_ptr&
          operator= (auto_ptr<T2>& y)
          {
            if (x_ != y.x_)
              reset (y.release ());

            return *this;
          }

          auto_ptr&
          operator= (auto_ptr_ref<T> r)
          {
            if (r.x_ != x_)
              reset (r.x_);

            return *this;
          }

          template <typename T2>
          operator auto_ptr_ref<T2> ()
          {
            return auto_ptr_ref<T2> (release ());
          }

          template <typename T2>
          operator auto_ptr<T2> ()
          {
            return auto_ptr<T2> (release ());
          }

        public:
          T&
          operator* () const
          {
            return *x_;
          }

          T*
          operator-> () const
          {
            return x_;
          }

          T*
          get () const
          {
            return x_;
          }

          T*
          release ()
          {
            T* x (x_);
            x_ = 0;
            return x;
          }

          void
          reset (T* x = 0)
          {
            if (x_)
              const_cast<typename remove_c<T>::r*> (x_)->release ();

            x_ = x;
          }

        private:
          T* x_;
        };

#define XSD_DOM_AUTO_PTR xsd::cxx::xml::dom::auto_ptr

#endif // XSD_CXX11
      }
    }
  }
}

#endif // XSD_CXX_XML_DOM_AUTO_PTR_HXX
