// file      : xsd/cxx/tree/types.txx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xercesc/util/Base64.hpp>
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
    namespace tree
    {

      // string
      //
      template <typename C, typename B>
      string<C, B>* string<C, B>::
      _clone (flags f, container* c) const
      {
        return new string (*this, f, c);
      }


      // normalized_string
      //
      template <typename C, typename B>
      normalized_string<C, B>* normalized_string<C, B>::
      _clone (flags f, container* c) const
      {
        return new normalized_string (*this, f, c);
      }


      // token
      //
      template <typename C, typename B>
      token<C, B>* token<C, B>::
      _clone (flags f, container* c) const
      {
        return new token (*this, f, c);
      }


      // nmtoken
      //
      template <typename C, typename B>
      nmtoken<C, B>* nmtoken<C, B>::
      _clone (flags f, container* c) const
      {
        return new nmtoken (*this, f, c);
      }


      // nmtokens
      //
      template <typename C, typename B, typename nmtoken>
      nmtokens<C, B, nmtoken>* nmtokens<C, B, nmtoken>::
      _clone (flags f, container* c) const
      {
        return new nmtokens (*this, f, c);
      }


      // name
      //
      template <typename C, typename B>
      name<C, B>* name<C, B>::
      _clone (flags f, container* c) const
      {
        return new name (*this, f, c);
      }


      // ncname
      //
      template <typename C, typename B>
      ncname<C, B>* ncname<C, B>::
      _clone (flags f, container* c) const
      {
        return new ncname (*this, f, c);
      }


      // language
      //
      template <typename C, typename B>
      language<C, B>* language<C, B>::
      _clone (flags f, container* c) const
      {
        return new language (*this, f, c);
      }


      // identity_impl
      //
      template <typename C, typename ncname>
      bool identity_impl<C, ncname>::
      before (const identity& y) const
      {
        return id_ < static_cast<const identity_impl&> (y).id_;
      }

      template <typename C, typename ncname>
      void identity_impl<C, ncname>::
      throw_duplicate_id () const
      {
        throw duplicate_id<C> (id_);
      }


      // id
      //
      template <typename C, typename B>
      id<C, B>* id<C, B>::
      _clone (flags f, container* c) const
      {
        return new id (*this, f, c);
      }

      template <typename C, typename B>
      id<C, B>& id<C, B>::
      operator= (C c)
      {
        unregister_id ();
        base () = c;
        register_id ();

        return *this;
      }

      template <typename C, typename B>
      id<C, B>& id<C, B>::
      operator= (const C* s)
      {
        unregister_id ();
        base () = s;
        register_id ();

        return *this;
      }

      template <typename C, typename B>
      id<C, B>& id<C, B>::
      operator= (const std::basic_string<C>& s)
      {
        unregister_id ();
        base () = s;
        register_id ();

        return *this;
      }

      template <typename C, typename B>
      id<C, B>& id<C, B>::
      operator= (const id& x)
      {
        unregister_id ();
        base () = x;
        register_id ();

        return *this;
      }

      template <typename C, typename B>
      void id<C, B>::
      _container (container* c)
      {
        B::_container (c);
        register_id ();
      }

      template <typename C, typename B>
      void id<C, B>::
      register_id ()
      {
        container* r (this->_root ());

        if (r != 0 && !this->empty ())
          r->_register_id (identity_, this->_container ());
      }

      template <typename C, typename B>
      void id<C, B>::
      unregister_id ()
      {
        container* r (this->_root ());

        if (r != 0 && !this->empty ())
          r->_unregister_id (identity_);
      }


      // idref
      //
      template <typename C, typename B, typename T>
      idref<C, B, T>* idref<C, B, T>::
      _clone (flags f, container* c) const
      {
        return new idref (*this, f, c);
      }

      template <typename C, typename B, typename T>
      const _type* idref<C, B, T>::
      get_ () const
      {
        if (!this->empty () && this->_container () != 0)
        {
          return this->_root ()->_lookup_id (identity_);
        }
        else
          return 0;
      }

      template <typename C, typename B, typename T>
      _type* idref<C, B, T>::
      get_ ()
      {
        if (!this->empty () && this->_container () != 0)
        {
          return this->_root ()->_lookup_id (identity_);
        }
        else
          return 0;
      }

      template <typename C, typename B, typename T>
      void idref<C, B, T>::
      true_ ()
      {
      }


      // idrefs
      //
      template <typename C, typename B, typename idref>
      idrefs<C, B, idref>* idrefs<C, B, idref>::
      _clone (flags f, container* c) const
      {
        return new idrefs (*this, f, c);
      }


      // uri
      //
      template <typename C, typename B>
      uri<C, B>* uri<C, B>::
      _clone (flags f, container* c) const
      {
        return new uri (*this, f, c);
      }


      // qname
      //
      template <typename C, typename B, typename uri, typename ncname>
      qname<C, B, uri, ncname>* qname<C, B, uri, ncname>::
      _clone (flags f, container* c) const
      {
        return new qname (*this, f, c);
      }


      // base64_binary
      //
      template <typename C, typename B>
      base64_binary<C, B>::
      base64_binary (size_t size)
          : buffer<C> (size)
      {
      }

      template <typename C, typename B>
      base64_binary<C, B>::
      base64_binary (size_t size, size_t capacity)
          : buffer<C> (size, capacity)
      {
      }

      template <typename C, typename B>
      base64_binary<C, B>::
      base64_binary (const void* data, size_t size)
          : buffer<C> (data, size)
      {
      }

      template <typename C, typename B>
      base64_binary<C, B>::
      base64_binary (const void* data, size_t size, size_t capacity)
          : buffer<C> (data, size, capacity)
      {
      }

      template <typename C, typename B>
      base64_binary<C, B>::
      base64_binary (void* data, size_t size, size_t capacity, bool own)
          : buffer<C> (data, size, capacity, own)
      {
      }

      template <typename C, typename B>
      base64_binary<C, B>* base64_binary<C, B>::
      _clone (flags f, container* c) const
      {
        return new base64_binary (*this, f, c);
      }

      template <typename C, typename B>
      std::basic_string<C> base64_binary<C, B>::
      encode () const
      {
        // Cannot use 'using namespace' because of MSXML conflict.
        //
        using xercesc::Base64;

        std::basic_string<C> str;

        XMLSize_t n;
        xml::std_memory_manager mm;

#ifdef XSD_CXX11
        std::unique_ptr<XMLByte[], xml::std_memory_manager&> r (
#else
        auto_array<XMLByte, xml::std_memory_manager> r (
#endif
          Base64::encode (
            reinterpret_cast<const XMLByte*> (this->data ()),
            static_cast<XMLSize_t> (this->size ()),
            &n,
            &mm),
	  mm);

        if (r)
        {
          str.reserve (n + 1);
          str.resize (n);

          for (XMLSize_t i (0); i < n; ++i)
            str[i] = C (r[i]);
        }
        else
        {
          //@@ throw
        }

        return str;
      }

      template <typename C, typename B>
      void base64_binary<C, B>::
      decode (const XMLCh* src)
      {
        // Cannot use 'using namespace' because of MSXML conflict.
        //
        using xercesc::Base64;

        xml::std_memory_manager mm;
        XMLSize_t size;

#ifdef XSD_CXX11
        std::unique_ptr<XMLByte[], xml::std_memory_manager&> data (
#else
        auto_array<XMLByte, xml::std_memory_manager> data (
#endif
          Base64::decodeToXMLByte (src, &size, &mm, Base64::Conf_RFC2045),
          mm);

        if (data)
        {
          buffer<C> tmp (data.get (), size, size, true);
          data.release ();
          this->swap (tmp); // g++ 4.1 likes it qualified, not sure why.
        }
        else
        {
          //@@ throw
        }
      }


      // hex_binary
      //
      template <typename C, typename B>
      hex_binary<C, B>::
      hex_binary (size_t size)
          : buffer<C> (size)
      {
      }

      template <typename C, typename B>
      hex_binary<C, B>::
      hex_binary (size_t size, size_t capacity)
          : buffer<C> (size, capacity)
      {
      }

      template <typename C, typename B>
      hex_binary<C, B>::
      hex_binary (const void* data, size_t size)
          : buffer<C> (data, size)
      {
      }

      template <typename C, typename B>
      hex_binary<C, B>::
      hex_binary (const void* data, size_t size, size_t capacity)
          : buffer<C> (data, size, capacity)
      {
      }

      template <typename C, typename B>
      hex_binary<C, B>::
      hex_binary (void* data, size_t size, size_t capacity, bool own)
          : buffer<C> (data, size, capacity, own)
      {
      }

      template <typename C, typename B>
      hex_binary<C, B>* hex_binary<C, B>::
      _clone (flags f, container* c) const
      {
        return new hex_binary (*this, f, c);
      }

      template <typename C, typename B>
      std::basic_string<C> hex_binary<C, B>::
      encode () const
      {
        std::basic_string<C> str;

        const char tab[] = "0123456789ABCDEF";

        if (size_t n = this->size ())
        {
          str.reserve (2 * n + 1);
          str.resize (2 * n);

          for (size_t i (0); i < n; ++i)
          {
            unsigned char byte (
	      static_cast<unsigned char> (*(this->data () + i)));
            unsigned char h (byte >> 4);
            unsigned char l (byte & 0x0F);

            str[2 * i] = C (tab[h]);
            str[2 * i + 1] = C (tab[l]);
          }
        }

        return str;
      }

      namespace bits
      {
        inline unsigned char
        hex_decode (XMLCh c)
        {
          unsigned char r (0xFF);

          if (c >= '0' && c <= '9')
            r = static_cast<unsigned char> (c - '0');
          else if (c >= 'A' && c <= 'F')
            r = static_cast<unsigned char> (10 + (c - 'A'));
          else if (c >= 'a' && c <= 'f')
            r = static_cast<unsigned char> (10 + (c - 'a'));

          return r;
        }
      }

      template <typename C, typename B>
      void hex_binary<C, B>::
      decode (const XMLCh* src)
      {
        size_t src_n (xercesc::XMLString::stringLen (src));

        if (src_n % 2 != 0)
          return; // @@ throw

        size_t n (src_n / 2);

        buffer<C> tmp (n);

        for (size_t i (0); i < n; ++i)
        {
          unsigned char h (bits::hex_decode (src[2 * i]));
          unsigned char l (bits::hex_decode (src[2 * i + 1]));

          if (h == 0xFF || l == 0xFF)
            return; //@@ throw

          tmp.data()[i] = (h << 4) | l;
        }

        this->swap (tmp); // g++ 4.1 likes it qualified, not sure why.
      }


      // entity
      //
      template <typename C, typename B>
      entity<C, B>* entity<C, B>::
      _clone (flags f, container* c) const
      {
        return new entity (*this, f, c);
      }


      // entities
      //
      template <typename C, typename B, typename entity>
      entities<C, B, entity>* entities<C, B, entity>::
      _clone (flags f, container* c) const
      {
        return new entities (*this, f, c);
      }
    }
  }
}
