/*
object.hh
Copyright (C) 2017 Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _LINPHONE_OBJECT_HH
#define _LINPHONE_OBJECT_HH

#include <memory>
#include <list>
#include <map>
#include <string>
#include <stdexcept>

#ifndef LINPHONECXX_PUBLIC
#if defined(_MSC_VER)
#ifdef LINPHONECXX_EXPORTS
#define LINPHONECXX_PUBLIC	__declspec(dllexport)
#else
#define LINPHONECXX_PUBLIC	__declspec(dllimport)
#endif
#else
#define LINPHONECXX_PUBLIC
#endif
#endif

#ifndef LINPHONECXX_DEPRECATED
#if defined(_MSC_VER)
#define LINPHONECXX_DEPRECATED __declspec(deprecated)
#else
#define LINPHONECXX_DEPRECATED __attribute__ ((deprecated))
#endif
#endif

namespace linphone {

	class Object: public std::enable_shared_from_this<Object> {
	public:
		Object(void *ptr, bool takeRef=true);
		virtual ~Object();

	public:
		template <class T>
		void setData(const std::string &key, T &data) {
			std::map<std::string,void *> &userData = getUserData();
			userData[key] = &data;
		}
		template <class T>
		T &getData(const std::string &key) {
			std::map<std::string,void *> &userData = getUserData();
			void *ptr = userData[key];
			if (ptr == NULL) {
				throw std::out_of_range("unknown data key [" + key + "]");
			} else {
				return *(T *)ptr;
			}
		}
		LINPHONECXX_PUBLIC void unsetData(const std::string &key);
		LINPHONECXX_PUBLIC bool dataExists(const std::string &key);

	public:
		template <class T>
		static std::shared_ptr<T> cPtrToSharedPtr(void *ptr, bool takeRef=true) {
			if (ptr == NULL) {
				return nullptr;
			} else {
				Object *cppPtr = getBackPtrFromCPtr(ptr);
				if (cppPtr == NULL) {
					return std::make_shared<T>(ptr, takeRef);
				} else {
					if (!takeRef) unrefCPtr(ptr);
					return std::static_pointer_cast<T,Object>(cppPtr->shared_from_this());
				}
			}
		}
		template <class T>
		static std::shared_ptr<const T> cPtrToSharedPtr(const void *ptr, bool takeRef=true) {
			return cPtrToSharedPtr<T>(const_cast<void *>(ptr), takeRef);
		}
		static void *sharedPtrToCPtr(const std::shared_ptr<const Object> &sharedPtr);

		static void unrefCPtr(void *ptr);

	private:
		LINPHONECXX_PUBLIC std::map<std::string,void *> &getUserData() const;
		static Object *getBackPtrFromCPtr(const void *ptr);
		template <class T> static void deleteSharedPtr(std::shared_ptr<T> *ptr) {if (ptr != NULL) delete ptr;}
		static void deleteString(std::string *str) {if (str != NULL) delete str;}

	protected:
		void *mPrivPtr;

	private:
		static const std::string sUserDataKey;
	};


	class Listener {
	public:
		LINPHONECXX_PUBLIC virtual ~Listener() = 0;
	};


	class ListenableObject: public Object {
	protected:
		ListenableObject(void *ptr, bool takeRef=true);
		void setListener(const std::shared_ptr<Listener> &listener);

	public:
		static std::shared_ptr<Listener> & getListenerFromObject(void *object);

	private:
		static void deleteListenerPtr(std::shared_ptr<Listener> *ptr) {delete ptr;}

	private:
		static std::string sListenerDataName;
	};

	class MultiListenableObject: public Object {
	public:
		static const char *sListenerListName;
		static const char *sCbsPtrName;

	protected:
		MultiListenableObject(void *ptr, bool takeRef=true);
		virtual ~MultiListenableObject() = default;

		void addListener(const std::shared_ptr<Listener> &listener);
		void removeListener(const std::shared_ptr<Listener> &listener);
		std::list<std::shared_ptr<Listener> > &getListeners() const;

		virtual void *createCallbacks() = 0;

		static void deleteListenerList(std::list<std::shared_ptr<Listener> > *listeners) {delete listeners;}

		friend class Factory;
	};

};

#endif // _LINPHONE_OBJECT_HH
