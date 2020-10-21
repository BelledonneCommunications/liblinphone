// file      : xsd/cxx/tree/elements.ixx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      // content_order_type
      //

      inline bool
      operator== (const content_order& x, const content_order& y)
      {
        return x.id == y.id && x.index == y.index;
      }

      inline bool
      operator!= (const content_order& x, const content_order& y)
      {
        return !(x == y);
      }

      inline bool
      operator< (const content_order& x, const content_order& y)
      {
        return x.id < y.id || (x.id == y.id && x.index < y.index);
      }

      // type
      //

      inline _type::
      _type ()
          : container_ (0)
      {
      }

      template <typename C>
      inline _type::
      _type (const C*)
          : container_ (0)
      {
      }

      inline _type::
      _type (const type& x, flags f, container* c)
          : container_ (c)
      {
        if (x.content_.get () != 0)
          content_ = x.content_->clone ();

        if (x.dom_info_.get () != 0 && (f & flags::keep_dom))
        {
          dom_info_ = x.dom_info_->clone (*this, c);
        }
      }

      inline const _type::dom_content_optional& _type::
      dom_content () const
      {
        const content_type* c (content_.get ());

        if (c == 0)
        {
          content_.reset (new dom_content_type);
          c = content_.get ();
        }

        // Accessing non-DOM content via the DOM API.
        //
        assert (dynamic_cast<const dom_content_type*> (c) != 0);

        return static_cast<const dom_content_type*> (c)->dom;
      }

      inline _type::dom_content_optional& _type::
      dom_content ()
      {
        content_type* c (content_.get ());

        if (c == 0)
        {
          content_.reset (new dom_content_type);
          c = content_.get ();
        }

        // Accessing non-DOM content via the DOM API.
        //
        assert (dynamic_cast<dom_content_type*> (c) != 0);

        return static_cast<dom_content_type*> (c)->dom;
      }

      inline void _type::
      dom_content (const xercesc::DOMElement& e)
      {
        content_type* c (content_.get ());

        if (c == 0)
          content_.reset (new dom_content_type (e));
        else
        {
          // Accessing non-DOM content via the DOM API.
          //
          assert (dynamic_cast<dom_content_type*> (c) != 0);
          static_cast<dom_content_type*> (c)->dom.set (e);
        }
      }

      inline void _type::
      dom_content (xercesc::DOMElement* e)
      {
        content_type* c (content_.get ());

        if (c == 0)
          content_.reset (new dom_content_type (e));
        else
        {
          // Accessing non-DOM content via the DOM API.
          //
          assert (dynamic_cast<dom_content_type*> (c) != 0);
          static_cast<dom_content_type*> (c)->dom.set (e);
        }
      }

      inline void _type::
      dom_content (const dom_content_optional& d)
      {
        content_type* c (content_.get ());

        if (c == 0)
          content_.reset (new dom_content_type (d));
        else
        {
          // Accessing non-DOM content via the DOM API.
          //
          assert (dynamic_cast<dom_content_type*> (c) != 0);
          static_cast<dom_content_type*> (c)->dom = d;
        }
      }

      inline const xercesc::DOMDocument& _type::
      dom_content_document () const
      {
        const content_type* c (content_.get ());

        if (c == 0)
        {
          content_.reset (new dom_content_type);
          c = content_.get ();
        }

        // Accessing non-DOM content via the DOM API.
        //
        assert (dynamic_cast<const dom_content_type*> (c) != 0);

        return *static_cast<const dom_content_type*> (c)->doc;
      }

      inline xercesc::DOMDocument& _type::
      dom_content_document ()
      {
        content_type* c (content_.get ());

        if (c == 0)
        {
          content_.reset (new dom_content_type);
          c = content_.get ();
        }

        // Accessing non-DOM content via the DOM API.
        //
        assert (dynamic_cast<dom_content_type*> (c) != 0);

        return *static_cast<dom_content_type*> (c)->doc;
      }

      inline bool _type::
      null_content () const
      {
        return content_.get () == 0;
      }

      // simple_type
      //

      template <typename C, typename B>
      inline simple_type<C, B>::
      simple_type ()
      {
      }

      template <typename C, typename B>
      inline simple_type<C, B>::
      simple_type (const C* s)
      {
        this->content_.reset (new text_content_type (s));
      }

      template <typename C, typename B>
      inline simple_type<C, B>::
      simple_type (const std::basic_string<C>& s)
      {
        this->content_.reset (new text_content_type (s));
      }

      template <typename C, typename B>
      inline const std::basic_string<C>& simple_type<C, B>::
      text_content () const
      {
        const content_type* c (this->content_.get ());

        if (c == 0)
        {
          this->content_.reset (new text_content_type);
          c = this->content_.get ();
        }

        // Accessing non-text content via the text API.
        //
        assert (dynamic_cast<const text_content_type*> (c) != 0);

        return static_cast<const text_content_type*> (c)->text;
      }

      template <typename C, typename B>
      inline std::basic_string<C>& simple_type<C, B>::
      text_content ()
      {
        content_type* c (this->content_.get ());

        if (c == 0)
        {
          this->content_.reset (new text_content_type);
          c = this->content_.get ();
        }

        // Accessing non-text content via the text API.
        //
        assert (dynamic_cast<text_content_type*> (c) != 0);

        return static_cast<text_content_type*> (c)->text;
      }

      template <typename C, typename B>
      inline void simple_type<C, B>::
      text_content (const std::basic_string<C>& t)
      {
        content_type* c (this->content_.get ());

        if (c == 0)
          this->content_.reset (new text_content_type (t));
        else
        {
          // Accessing non-text content via the text API.
          //
          assert (dynamic_cast<text_content_type*> (c) != 0);
          static_cast<text_content_type*> (c)->text = t;
        }
      }
    }
  }
}
