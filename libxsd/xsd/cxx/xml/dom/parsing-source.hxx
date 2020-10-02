// file      : xsd/cxx/xml/dom/parsing-source.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_XML_DOM_PARSING_SOURCE_HXX
#define XSD_CXX_XML_DOM_PARSING_SOURCE_HXX

#include <string>

#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>

#include <xercesc/sax/InputSource.hpp>

#include <xsd/cxx/xml/elements.hxx>      // properies
#include <xsd/cxx/xml/error-handler.hxx>
#include <xsd/cxx/xml/dom/auto-ptr.hxx>
#include <xsd/cxx/xml/dom/elements.hxx>  // name
#include <xsd/cxx/xml/dom/parsing-header.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      namespace dom
      {
        // Parser state object. Can be used for parsing elements (and
        // optionally text), attributes, or both.
        //
        template <typename C>
        class parser
        {
        public:
          parser (const xercesc::DOMElement& e, bool ep, bool tp, bool ap);

          // Content parsing.
          //
          bool
          more_content ()
          {
            return next_content_ != 0;
          }

          const xercesc::DOMElement&
          cur_element ()
          {
            return *static_cast<const xercesc::DOMElement*> (next_content_);
          }

          const xercesc::DOMText&
          cur_text ()
          {
            return *static_cast<const xercesc::DOMText*> (next_content_);
          }

          bool
          cur_is_text ()
          {
            return next_content_->getNodeType () !=
              xercesc::DOMNode::ELEMENT_NODE;
          }

          void
          next_content (bool text);

          // Attribute parsing.
          //
          bool
          more_attributes ()
          {
            return as_ > ai_;
          }

          const xercesc::DOMAttr&
          next_attribute ()
          {
            return *static_cast<const xercesc::DOMAttr*> (a_->item (ai_++));
          }

          void
          reset_attributes ()
          {
            ai_ = 0;
          }

          const xercesc::DOMElement&
          element () const
          {
            return element_;
          }

        private:
          parser (const parser&);

          parser&
          operator= (const parser&);

        private:
          const xercesc::DOMElement& element_;
          const xercesc::DOMNode* next_content_;

          const xercesc::DOMNamedNodeMap* a_;
          XMLSize_t ai_; // Index of the next DOMAttr.
          XMLSize_t as_; // Cached size of a_.
        };


        // Parsing flags.
        //
        const unsigned long dont_validate      = 0x00000400UL;
        const unsigned long no_muliple_imports = 0x00000800UL;

        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        parse (xercesc::InputSource&,
               error_handler<C>&,
               const properties<C>&,
               unsigned long flags);

        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        parse (xercesc::InputSource&,
               xercesc::DOMErrorHandler&,
               const properties<C>&,
               unsigned long flags);

        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        parse (const std::basic_string<C>& uri,
               error_handler<C>&,
               const properties<C>&,
               unsigned long flags);

        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        parse (const std::basic_string<C>& uri,
               xercesc::DOMErrorHandler&,
               const properties<C>&,
               unsigned long flags);
      }
    }
  }
}

#include <xsd/cxx/xml/dom/parsing-source.txx>

#endif // XSD_CXX_XML_DOM_PARSING_SOURCE_HXX
