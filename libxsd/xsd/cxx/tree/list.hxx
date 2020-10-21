// file      : xsd/cxx/tree/list.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_LIST_HXX
#define XSD_CXX_TREE_LIST_HXX

#include <string>

#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMElement.hpp>

#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/istream-fwd.hxx> // tree::istream
#include <xsd/cxx/tree/containers.hxx>  // fundamental_p, sequence

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      // Class template for xsd:list mapping. Based on the sequence
      // template.
      //
      template <typename T,
                typename C,
                schema_type::value ST = schema_type::other,
                bool fund = fundamental_p<T>::r>
      class list;


      //
      //
      template <typename T, typename C, schema_type::value ST>
      class list<T, C, ST, false>: public sequence<T>
      {
      public:
        explicit
        list (container* c = 0)
            : sequence<T> (c)
        {
        }

        list (typename sequence<T>::size_type n,
              const T& x,
              container* c = 0)
            : sequence<T> (n, x, c)
        {
        }

        template<typename I>
        list (const I& b, const I& e, container* c = 0)
            : sequence<T> (b, e, c)
        {
        }

        template <typename S>
        list (istream<S>&, flags = 0, container* c = 0);

        list (const list<T, C, ST, false>& l, flags f = 0, container* c = 0)
            : sequence<T> (l, f, c)
        {
        }

      public:
        list (const xercesc::DOMElement&, flags = 0, container* c = 0);

        list (const xercesc::DOMAttr&, flags = 0, container* c = 0);

        list (const std::basic_string<C>&,
              const xercesc::DOMElement*,
              flags = 0,
              container* c = 0);

      private:
        void
        init (const std::basic_string<C>&,
              const xercesc::DOMElement*,
              flags);
      };


      //
      //
      template <typename T, typename C, schema_type::value ST>
      class list<T, C, ST, true>: public sequence<T>
      {
      public:
        explicit
        list (container* c = 0)
            : sequence<T> (c)
        {
        }

        explicit
        list (typename sequence<T>::size_type n, const T& x, container* c = 0)
            : sequence<T> (n, x, c)
        {
        }

        template<typename I>
        list (const I& b, const I& e, container* c = 0)
            : sequence<T> (b, e, c)
        {
        }

        template <typename S>
        list (istream<S>&, flags = 0, container* c = 0);

        list (const list<T, C, ST, true>& l, flags f = 0, container* c = 0)
            : sequence<T> (l, f, c)
        {
        }

      public:
        list (const xercesc::DOMElement&, flags = 0, container* c = 0);

        list (const xercesc::DOMAttr&, flags = 0, container* c = 0);

        list (const std::basic_string<C>&,
              const xercesc::DOMElement*,
              flags = 0,
              container* c = 0);

      private:
        void
        init (const std::basic_string<C>&, const xercesc::DOMElement*);
      };
    }
  }
}

#endif // XSD_CXX_TREE_LIST_HXX
