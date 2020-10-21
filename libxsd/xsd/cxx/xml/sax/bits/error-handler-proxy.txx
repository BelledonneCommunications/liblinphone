// file      : xsd/cxx/xml/sax/bits/error-handler-proxy.txx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd/cxx/xml/string.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      namespace sax
      {
        namespace bits
        {
          template <typename C>
          void error_handler_proxy<C>::
          warning (const xercesc::SAXParseException& e)
          {
            if (native_eh_)
              native_eh_->warning (e);
            else
              handle (e, severity::warning);
          }


          template <typename C>
          void error_handler_proxy<C>::
          error (const xercesc::SAXParseException& e)
          {
            failed_ = true;

            if (native_eh_)
              native_eh_->error (e);
            else
              handle (e, severity::error);
          }


          template <typename C>
          void error_handler_proxy<C>::
          fatalError (const xercesc::SAXParseException& e)
          {
            failed_ = true;

            if (native_eh_)
              native_eh_->fatalError (e);
            else
              handle (e, severity::fatal);
          }


          template <typename C>
          void error_handler_proxy<C>::
          handle (const xercesc::SAXParseException& e, severity s)
          {
            //@@ I do not honor return values from the handler. This
            //   is not too bad at the moment because I set
            //   all-errors-are-fatal flag on the parser.
            //
            const XMLCh* id (e.getPublicId ());

            if (id == 0)
              id = e.getSystemId ();

            eh_->handle (transcode<C> (id),
                         static_cast<unsigned long> (e.getLineNumber ()),
                         static_cast<unsigned long> (e.getColumnNumber ()),
                         s,
                         transcode<C> (e.getMessage ()));
          }
        }
      }
    }
  }
}
