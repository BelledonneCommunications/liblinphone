// file      : xsd/cxx/tree/iterator-adapter.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_ITERATOR_ADAPTER_HXX
#define XSD_CXX_TREE_ITERATOR_ADAPTER_HXX

#include <cstddef>  // std::ptrdiff_t
#include <iterator> // std::iterator_traits

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      // Sun CC's <iterator> does not have iterator_traits. To overcome
      // this, we will wrap std::iterator_traits into our own and also
      // specialize it for pointer types. Since Sun CC uses pointer
      // for vector::iterator, it will use the specialization and won't
      // notice the std::iterator_traits.
      //
#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC
      template <typename I>
      struct iterator_traits
      {
        typedef
        typename std::iterator_traits<I>::iterator_category
        iterator_category;

        typedef
        typename std::iterator_traits<I>::value_type
        value_type;

        typedef
        typename std::iterator_traits<I>::difference_type
        difference_type;
      };
#else
      // The Pointer specialization does not work for reverse and
      // set iterators. But these iterators are user-dfined types
      // and have suitable typedefs that we can use.
      //
      template <typename I>
      struct iterator_traits
      {
        typedef typename I::iterator_category iterator_category;
        typedef typename I::value_type value_type;
        typedef typename I::difference_type difference_type;
      };

      template <typename T>
      struct iterator_traits<T*>
      {
        typedef std::random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
      };
#endif

      // Iterator adapter for complex types. It expects I to point to
      // a smart pointer-like object that has operator*() that returns
      // a refernce to a type static_cast'able to T and get() that
      // returns a pointer to a type static_cast'able to T.
      //

      template <typename I, typename T>
      struct iterator_adapter
      {
        typedef T value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        typedef
        typename iterator_traits<I>::iterator_category
        iterator_category;

        typedef
        typename iterator_traits<I>::difference_type
        difference_type;


      public:
        iterator_adapter ()
            : i_ () // i_ can be of a pointer type.
        {
        }

        // Allow iterator to const_iterator conversion.
        //
        template <typename J, typename T2>
        iterator_adapter (const iterator_adapter<J, T2>& j)
            : i_ (j.base ())
        {
        }

        explicit
        iterator_adapter (const I& i)
            : i_ (i)
        {
        }

      public:
        // Forward iterator requirements.
        //
        reference
        operator* () const
        {
          return static_cast<reference> (**i_);
        }

        pointer
        operator-> () const
        {
          return static_cast<pointer> (i_->get ());
        }

        iterator_adapter&
        operator++ ()
        {
          ++i_;
          return *this;
        }

        iterator_adapter
        operator++ (int)
        {
          iterator_adapter r (*this);
          ++i_;
          return r;
        }

        // Bidirectional iterator requirements.
        //
        iterator_adapter&
        operator-- ()
        {
          --i_;
          return *this;
        }

        iterator_adapter
        operator-- (int)
        {
          iterator_adapter r (*this);
          --i_;
          return r;
        }

        // Random access iterator requirements.
        //
        reference
        operator[] (difference_type n) const
        {
          return static_cast<reference> (*(i_[n]));
        }

        iterator_adapter&
        operator+= (difference_type n)
        {
          i_ += n;
          return *this;
        }

        iterator_adapter
        operator+ (difference_type n) const
        {
          return iterator_adapter (i_ + n);
        }

        iterator_adapter&
        operator-= (difference_type n)
        {
          i_ -= n;
          return *this;
        }

        iterator_adapter
        operator- (difference_type n) const
        {
          return iterator_adapter (i_ - n);
        }

      public:
        const I&
        base () const
        {
          return i_;
        }

      private:
        I i_;
      };

      // Note: We use different types for left- and right-hand-side
      // arguments to allow comparison between iterator and const_iterator.
      //

      // Forward iterator requirements.
      //
      template <typename I, typename J, typename T1, typename T2>
      inline bool
      operator== (const iterator_adapter<I, T1>& i,
                  const iterator_adapter<J, T2>& j)
      {
        return i.base () == j.base ();
      }

      template <typename I, typename J, typename T1, typename T2>
      inline bool
      operator!= (const iterator_adapter<I, T1>& i,
                  const iterator_adapter<J, T2>& j)
      {
        return i.base () != j.base ();
      }

      // Random access iterator requirements
      //
      template <typename I, typename J, typename T1, typename T2>
      inline bool
      operator< (const iterator_adapter<I, T1>& i,
                 const iterator_adapter<J, T2>& j)
      {
        return i.base () < j.base ();
      }

      template <typename I, typename J, typename T1, typename T2>
      inline bool
      operator> (const iterator_adapter<I, T1>& i,
                 const iterator_adapter<J, T2>& j)
      {
        return i.base () > j.base ();
      }

      template <typename I, typename J, typename T1, typename T2>
      inline bool
      operator<= (const iterator_adapter<I, T1>& i,
                  const iterator_adapter<J, T2>& j)
      {
        return i.base () <= j.base ();
      }

      template <typename I, typename J, typename T1, typename T2>
      inline bool
      operator>= (const iterator_adapter<I, T1>& i,
                  const iterator_adapter<J, T2>& j)
      {
        return i.base () >= j.base ();
      }

      template <typename I, typename J, typename T1, typename T2>
      inline typename iterator_adapter<I, T1>::difference_type
      operator- (const iterator_adapter<I, T1>& i,
                 const iterator_adapter<J, T2>& j)
      {
        return i.base () - j.base ();
      }

      template <typename I, typename T>
      inline iterator_adapter<I, T>
      operator+ (typename iterator_adapter<I, T>::difference_type n,
                 const iterator_adapter<I, T>& i)
      {
        return iterator_adapter<I, T> (i.base () + n);
      }
    }
  }
}

#endif  // XSD_CXX_TREE_ITERATOR_ADAPTER_HXX
