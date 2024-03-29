/*
tools.hh
Copyright (c) 2010-2022 Belledonne Communications SARL.

This file is part of Liblinphone
(see https://gitlab.linphone.org/BC/public/liblinphone).

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _TOOLS_HH
#define _TOOLS_HH

#include "object.hh"
#include <bctoolbox/list.h>
#include <belle-sip/object.h>
#include <list>
#include <memory>

namespace linphone {

class AbstractBctbxListWrapper {
public:
	AbstractBctbxListWrapper() = default;
	virtual ~AbstractBctbxListWrapper() = default;
	::bctbx_list_t *c_list() {
		return mCList;
	}

protected:
	::bctbx_list_t *mCList = nullptr;
};

template <class T>
class ObjectBctbxListWrapper : public AbstractBctbxListWrapper {
public:
	ObjectBctbxListWrapper(const std::list<std::shared_ptr<T>> &cppList) {
		mCList = cppListToBctbxList(cppList);
	}
	virtual ~ObjectBctbxListWrapper() {
		if (mCList != NULL) {
			bctbx_list_free_with_data(mCList, unrefData);
		}
	}
	static std::list<std::shared_ptr<T>> bctbxListToCppList(const ::bctbx_list_t *bctbxList, bool takeRef = true) {
		std::list<std::shared_ptr<T>> cppList;
		for (const ::bctbx_list_t *it = bctbxList; it != NULL; it = it->next) {
			std::shared_ptr<T> newObj = Object::cPtrToSharedPtr<T>(it->data, takeRef);
			cppList.push_back(newObj);
		}
		return cppList;
	}
	static std::list<std::shared_ptr<T>> bctbxListToCppList(::bctbx_list_t *bctbxList, bool takeRef = true) {
		std::list<std::shared_ptr<T>> cppList =
		    bctbxListToCppList(const_cast<const ::bctbx_list_t *>(bctbxList), takeRef);
		bctbx_list_free(bctbxList);
		return cppList;
	}
	static ::bctbx_list_t *cppListToBctbxList(const std::list<std::shared_ptr<T>> &cppList) {
		bctbx_list_t *cList = NULL;
		for (auto it = cppList.cbegin(); it != cppList.cend(); it++) {
			::belle_sip_object_t *cPtr =
			    (::belle_sip_object_t *)Object::sharedPtrToCPtr(std::static_pointer_cast<Object, T>(*it));
			if (cPtr != NULL) belle_sip_object_ref(cPtr);
			cList = bctbx_list_append(cList, cPtr);
		}
		return cList;
	}

private:
	static void unrefData(void *data) {
		if (data != NULL) belle_sip_object_unref(data);
	}
};

// NOTE: for optimization reason, StringBctbxListWrapper doesn't copy
// the strings while building the bctbx_list_t, but just copy their
// addresses. Please ensure the life of the C++ string list as long as
// StringBctbxListWrapper is living.
class StringBctbxListWrapper : public AbstractBctbxListWrapper {
public:
	StringBctbxListWrapper(const std::list<std::string> &cppList);
	virtual ~StringBctbxListWrapper();
	static std::list<std::string> bctbxListToCppList(const ::bctbx_list_t *bctbxList);
	static std::list<std::string> bctbxListToCppList(::bctbx_list_t *bctbxList);
};

template <class T>
class EnumBctbxListWrapper : public AbstractBctbxListWrapper {
public:
	EnumBctbxListWrapper(const std::list<T> &cppList) : AbstractBctbxListWrapper() {
		for (auto val : cppList) {
			mCList = bctbx_list_append(mCList, reinterpret_cast<void *>(static_cast<intptr_t>(val)));
		}
	}
	virtual ~EnumBctbxListWrapper() {
		bctbx_list_free(mCList);
	}
	static std::list<T> bctbxListToCppList(const ::bctbx_list_t *bctbxList) {
		std::list<T> cppList{};
		for (auto it = bctbxList; it; it = it->next) {
			cppList.push_back(static_cast<T>(static_cast<int>(reinterpret_cast<intptr_t>(it->data))));
		}
		return cppList;
	}
	static std::list<T> bctbxListToCppList(::bctbx_list_t *bctbxList) {
		auto cppList = bctbxListToCppList(const_cast<const ::bctbx_list_t *>(bctbxList));
		if (bctbxList) bctbx_list_free(bctbxList);
		return cppList;
	}
};

template <class T, class U>
class StructBctbxListWrapper : public AbstractBctbxListWrapper {
public:
	StructBctbxListWrapper(const std::list<T> &cppList) : AbstractBctbxListWrapper() {
		mCList = cppListToBctbxList(cppList);
	}
	virtual ~StructBctbxListWrapper() {
		bctbx_list_free_with_data(mCList, (bctbx_list_free_func)deleteCStruct);
	}
	static std::list<T> bctbxListToCppList(const ::bctbx_list_t *bctbxList) {
		std::list<T> cppList;
		for (const bctbx_list_t *it = bctbx_list_first_elem(bctbxList); it != NULL; it = bctbx_list_next(it)) {
			cppList.push_back(T(it->data));
		}
		return cppList;
	}
	static bctbx_list_t *cppListToBctbxList(const std::list<T> &cppList) {
		bctbx_list_t *cList = NULL;
		for (auto it = cppList.cbegin(); it != cppList.cend(); it++) {
			cList = bctbx_list_append(cList, new U(it->c_struct()));
		}
		return cList;
	}

private:
	static void deleteCStruct(U *cStruct) {
		delete cStruct;
	}
};

class StringUtilities {
public:
	static std::string cStringToCpp(const char *cstr);
	static std::string cStringToCpp(char *cstr);
	static const char *cppStringToC(const std::string &cppstr);
	static std::list<std::string> cStringArrayToCppList(const char **cArray);
};

template <class T>
class StructWrapper {
public:
	StructWrapper(const T &s) {
		mStruct = s;
	}
	const void *ptr() const {
		return &mStruct;
	}

private:
	T mStruct;
};

}; // namespace linphone

#endif // _TOOLS_HH
