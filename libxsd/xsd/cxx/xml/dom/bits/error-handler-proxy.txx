// file      : xsd/cxx/xml/dom/bits/error-handler-proxy.txx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd/cxx/xml/string.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      namespace dom
      {
        namespace bits
        {
          template <typename C>
          bool error_handler_proxy<C>::
          handleError (const xercesc::DOMError& e)
          {
            using xercesc::DOMError;

            if (e.getSeverity() != DOMError::DOM_SEVERITY_WARNING)
              failed_ = true;

            if (native_eh_)
              return native_eh_->handleError (e);
            else
            {
              typedef typename error_handler<C>::severity severity;

              severity s (severity::error);

              switch (e.getSeverity())
              {
              case DOMError::DOM_SEVERITY_WARNING:
                {
                  s = severity::warning;
                  break;
                }
              case DOMError::DOM_SEVERITY_ERROR:
                {
                  s = severity::error;
                  break;
                }
              case DOMError::DOM_SEVERITY_FATAL_ERROR:
                {
                  s = severity::fatal;
                  break;
                }
              }

              xercesc::DOMLocator* loc (e.getLocation ());

              return eh_->handle (
                transcode<C> (loc->getURI ()),
                static_cast<unsigned long> (loc->getLineNumber ()),
                static_cast<unsigned long> (loc->getColumnNumber ()),
                s,
                transcode<C> (e.getMessage ()));
            }
          }
        }
      }
    }
  }
}
