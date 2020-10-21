// file      : xsd/cxx/xml/std-memory-manager.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_XML_STD_MEMORY_MANAGER_HXX
#define XSD_CXX_XML_STD_MEMORY_MANAGER_HXX

#include <new> // operator new, delete
#include <xercesc/framework/MemoryManager.hpp>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      class std_memory_manager: public xercesc::MemoryManager
      {
      public:
        // Xerces-C++ MemoryManager interface.
        //
        virtual void*
        allocate(XMLSize_t size)
        {
          return operator new (size);
        }

        virtual void
        deallocate(void* p)
        {
          if (p)
	    operator delete (p);
        }

        virtual xercesc::MemoryManager*
        getExceptionMemoryManager()
        {
          return xercesc::XMLPlatformUtils::fgMemoryManager;
        }

        // Standard deleter interface.
        //
        void
        operator() (void* p) const
        {
          if (p)
	    operator delete (p);
        }
      };
    }
  }
}

#endif  // XSD_CXX_XML_STD_MEMORY_MANAGER_HXX
