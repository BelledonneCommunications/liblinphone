// file      : xsd/cxx/xml/dom/parsing-source.txx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xercesc/dom/DOMLSParser.hpp>
#include <xercesc/dom/DOMLSException.hpp>

#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>

#include <xercesc/util/XMLUni.hpp>     // xercesc::fg*
#include <xercesc/util/XMLUniDefs.hpp> // chLatin_L, etc

#include <xercesc/framework/Wrapper4InputSource.hpp>

#include <xsd/cxx/xml/string.hxx>
#include <xsd/cxx/xml/dom/bits/error-handler-proxy.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      namespace dom
      {
        // parser
        //
        template <typename C>
        parser<C>::
        parser (const xercesc::DOMElement& e, bool ep, bool tp, bool ap)
            : element_ (e),
              next_content_ (0),
              a_ (0),
              ai_ (0)
        {
          using xercesc::DOMNode;

          if (ep)
          {
            for (next_content_ = e.getFirstChild ();;
                 next_content_ = next_content_->getNextSibling ())
            {
              if (next_content_ == 0)
                break;

              DOMNode::NodeType t (next_content_->getNodeType ());

              if (t == DOMNode::ELEMENT_NODE)
                break;

              if (tp && (t == DOMNode::TEXT_NODE ||
                         t == DOMNode::CDATA_SECTION_NODE))
                break;
            }
          }

          if (ap)
          {
            a_ = e.getAttributes ();
            as_ = a_->getLength ();
          }
        }

        template <typename C>
        void parser<C>::
        next_content (bool tp)
        {
          using xercesc::DOMNode;

          for (next_content_ = next_content_->getNextSibling ();;
               next_content_ = next_content_->getNextSibling ())
          {
            if (next_content_ == 0)
              break;

            DOMNode::NodeType t (next_content_->getNodeType ());

            if (t == DOMNode::ELEMENT_NODE)
              break;

            if (tp && (t == DOMNode::TEXT_NODE ||
                       t == DOMNode::CDATA_SECTION_NODE))
              break;
          }
        }

        // parse()
        //
        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        parse (xercesc::InputSource& is,
               error_handler<C>& eh,
               const properties<C>& prop,
               unsigned long flags)
        {
          bits::error_handler_proxy<C> ehp (eh);
          return xml::dom::parse (is, ehp, prop, flags);
        }

        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        parse (xercesc::InputSource& is,
               xercesc::DOMErrorHandler& eh,
               const properties<C>& prop,
               unsigned long flags)
        {
          using namespace xercesc;

          // Instantiate the DOM parser.
          //
          const XMLCh ls_id[] = {xercesc::chLatin_L,
                                 xercesc::chLatin_S,
                                 xercesc::chNull};

          // Get an implementation of the Load-Store (LS) interface.
          //
          DOMImplementation* impl (
            DOMImplementationRegistry::getDOMImplementation (ls_id));

          XSD_DOM_AUTO_PTR<DOMLSParser> parser (
            impl->createLSParser (DOMImplementationLS::MODE_SYNCHRONOUS, 0));

          DOMConfiguration* conf (parser->getDomConfig ());

          // Discard comment nodes in the document.
          //
          conf->setParameter (XMLUni::fgDOMComments, false);

          // Enable datatype normalization.
          //
          conf->setParameter (XMLUni::fgDOMDatatypeNormalization, true);

          // Do not create EntityReference nodes in the DOM tree. No
          // EntityReference nodes will be created, only the nodes
          // corresponding to their fully expanded substitution text
          // will be created.
          //
          conf->setParameter (XMLUni::fgDOMEntities, false);

          // Perform namespace processing.
          //
          conf->setParameter (XMLUni::fgDOMNamespaces, true);

          // Do not include ignorable whitespace in the DOM tree.
          //
          conf->setParameter (XMLUni::fgDOMElementContentWhitespace, false);

          if (flags & dont_validate)
          {
            conf->setParameter (XMLUni::fgDOMValidate, false);
            conf->setParameter (XMLUni::fgXercesSchema, false);
            conf->setParameter (XMLUni::fgXercesSchemaFullChecking, false);
          }
          else
          {
            conf->setParameter (XMLUni::fgDOMValidate, true);
            conf->setParameter (XMLUni::fgXercesSchema, true);

            // Xerces-C++ 3.1.0 is the first version with working multi import
            // support.
            //
#if _XERCES_VERSION >= 30100
            if (!(flags & no_muliple_imports))
              conf->setParameter (XMLUni::fgXercesHandleMultipleImports, true);
#endif
            // This feature checks the schema grammar for additional
            // errors. We most likely do not need it when validating
            // instances (assuming the schema is valid).
            //
            conf->setParameter (XMLUni::fgXercesSchemaFullChecking, false);
          }

          // We will release DOM ourselves.
          //
          conf->setParameter (XMLUni::fgXercesUserAdoptsDOMDocument, true);


          // Transfer properies if any.
          //

          if (!prop.schema_location ().empty ())
          {
            xml::string sl (prop.schema_location ());
            const void* v (sl.c_str ());

            conf->setParameter (
              XMLUni::fgXercesSchemaExternalSchemaLocation,
              const_cast<void*> (v));
          }

          if (!prop.no_namespace_schema_location ().empty ())
          {
            xml::string sl (prop.no_namespace_schema_location ());
            const void* v (sl.c_str ());

            conf->setParameter (
              XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
              const_cast<void*> (v));
          }

          // If external schema location was specified, disable loading
          // schemas via the schema location attributes in the document.
          //
#if _XERCES_VERSION >= 30100
          if (!prop.schema_location ().empty () ||
              !prop.no_namespace_schema_location ().empty ())
          {
            conf->setParameter (XMLUni::fgXercesLoadSchema, false);
          }
#endif
          // Set error handler.
          //
          bits::error_handler_proxy<C> ehp (eh);
          conf->setParameter (XMLUni::fgDOMErrorHandler, &ehp);

          xercesc::Wrapper4InputSource wrap (&is, false);

          XSD_DOM_AUTO_PTR<DOMDocument> doc;
          try
          {
            doc.reset (parser->parse (&wrap));
          }
          catch (const xercesc::DOMLSException&)
          {
          }

          if (ehp.failed ())
            doc.reset ();

          return doc;
        }

        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        parse (const std::basic_string<C>& uri,
               error_handler<C>& eh,
               const properties<C>& prop,
               unsigned long flags)
        {
          bits::error_handler_proxy<C> ehp (eh);
          return xml::dom::parse (uri, ehp, prop, flags);
        }

        template <typename C>
        XSD_DOM_AUTO_PTR<xercesc::DOMDocument>
        parse (const std::basic_string<C>& uri,
               xercesc::DOMErrorHandler& eh,
               const properties<C>& prop,
               unsigned long flags)
        {
          using namespace xercesc;

          // Instantiate the DOM parser.
          //
          const XMLCh ls_id[] = {xercesc::chLatin_L,
                                 xercesc::chLatin_S,
                                 xercesc::chNull};

          // Get an implementation of the Load-Store (LS) interface.
          //
          DOMImplementation* impl (
            DOMImplementationRegistry::getDOMImplementation (ls_id));

          XSD_DOM_AUTO_PTR<DOMLSParser> parser (
            impl->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0));

          DOMConfiguration* conf (parser->getDomConfig ());

          // Discard comment nodes in the document.
          //
          conf->setParameter (XMLUni::fgDOMComments, false);

          // Enable datatype normalization.
          //
          conf->setParameter (XMLUni::fgDOMDatatypeNormalization, true);

          // Do not create EntityReference nodes in the DOM tree. No
          // EntityReference nodes will be created, only the nodes
          // corresponding to their fully expanded substitution text
          // will be created.
          //
          conf->setParameter (XMLUni::fgDOMEntities, false);

          // Perform namespace processing.
          //
          conf->setParameter (XMLUni::fgDOMNamespaces, true);

          // Do not include ignorable whitespace in the DOM tree.
          //
          conf->setParameter (XMLUni::fgDOMElementContentWhitespace, false);

          if (flags & dont_validate)
          {
            conf->setParameter (XMLUni::fgDOMValidate, false);
            conf->setParameter (XMLUni::fgXercesSchema, false);
            conf->setParameter (XMLUni::fgXercesSchemaFullChecking, false);
          }
          else
          {
            conf->setParameter (XMLUni::fgDOMValidate, true);
            conf->setParameter (XMLUni::fgXercesSchema, true);

            // Xerces-C++ 3.1.0 is the first version with working multi import
            // support.
            //
#if _XERCES_VERSION >= 30100
            if (!(flags & no_muliple_imports))
              conf->setParameter (XMLUni::fgXercesHandleMultipleImports, true);
#endif

            // This feature checks the schema grammar for additional
            // errors. We most likely do not need it when validating
            // instances (assuming the schema is valid).
            //
            conf->setParameter (XMLUni::fgXercesSchemaFullChecking, false);
          }

          // We will release DOM ourselves.
          //
          conf->setParameter (XMLUni::fgXercesUserAdoptsDOMDocument, true);


          // Transfer properies if any.
          //

          if (!prop.schema_location ().empty ())
          {
            xml::string sl (prop.schema_location ());
            const void* v (sl.c_str ());

            conf->setParameter (
              XMLUni::fgXercesSchemaExternalSchemaLocation,
              const_cast<void*> (v));
          }

          if (!prop.no_namespace_schema_location ().empty ())
          {
            xml::string sl (prop.no_namespace_schema_location ());
            const void* v (sl.c_str ());

            conf->setParameter (
              XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
              const_cast<void*> (v));
          }

          // If external schema location was specified, disable loading
          // schemas via the schema location attributes in the document.
          //
#if _XERCES_VERSION >= 30100
          if (!prop.schema_location ().empty () ||
              !prop.no_namespace_schema_location ().empty ())
          {
            conf->setParameter (XMLUni::fgXercesLoadSchema, false);
          }
#endif
          // Set error handler.
          //
          bits::error_handler_proxy<C> ehp (eh);
          conf->setParameter (XMLUni::fgDOMErrorHandler, &ehp);

          XSD_DOM_AUTO_PTR<DOMDocument> doc;
          try
          {
            doc.reset (parser->parseURI (string (uri).c_str ()));
          }
          catch (const xercesc::DOMLSException&)
          {
          }

          if (ehp.failed ())
            doc.reset ();

          return doc;
        }
      }
    }
  }
}
