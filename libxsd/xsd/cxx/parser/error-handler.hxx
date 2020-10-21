// file      : xsd/cxx/parser/error-handler.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_PARSER_ERROR_HANDLER_HXX
#define XSD_CXX_PARSER_ERROR_HANDLER_HXX

#include <xsd/cxx/xml/error-handler.hxx>

#include <xsd/cxx/parser/exceptions.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace parser
    {
      template <typename C>
      class error_handler: public xml::error_handler<C>
      {
      public:
        typedef typename xml::error_handler<C>::severity severity;

        error_handler ()
            : failed_ (false)
        {
        }

        virtual bool
        handle (const std::basic_string<C>& id,
                unsigned long line,
                unsigned long column,
                severity s,
                const std::basic_string<C>& message);

        void
        throw_if_failed () const;

        void
        reset ()
        {
          failed_ = false;
          diagnostics_.clear ();
        }

      private:
        bool failed_;
        diagnostics<C> diagnostics_;
      };
    }
  }
}

#include <xsd/cxx/parser/error-handler.txx>

#endif  // XSD_CXX_PARSER_ERROR_HANDLER_HXX
