/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Template for easy conversion from std::list to bctbx_list_t.
 * The authority list is the C++ one, the C one being only a const view of it.
 * _T must be an HybridObject; so that conversion from C++ type to C type is done automatically.
 */

#ifndef _L_LIST_HOLDER_H_
#define _L_LIST_HOLDER_H_

#include "logger/logger.h"

LINPHONE_BEGIN_NAMESPACE

/* Utility class to convert from C++ std::list of HybridObject, to bctbx_list_t and vice versa.
 The bctbx_list_t contains the C pointer (obtained with toC())*/
template <typename _T>
class ListHolder {
public:
	ListHolder() = default;
	ListHolder(const ListHolder<_T> &other) : mList(other.mList), mCList(nullptr) {
	}
	// The STL list is a public member, directly accessible.
	std::list<std::shared_ptr<_T>> mList;
	// Return a C list from the STL list.
	const bctbx_list_t *getCList() const {
		if (mCList) bctbx_list_free(mCList);
		mCList = _T::getCListFromCppList(mList, false);
		return mCList;
	}
	// Assign a C list. This replaces the STL list.
	void setCList(const bctbx_list_t *clist) {
		mList = _T::getCppListFromCList(clist);
	}
	~ListHolder() {
		if (mCList) bctbx_list_free(mCList);
	}

private:
	mutable bctbx_list_t *mCList = nullptr;
};

/* template specialisation for std::string */
template <>
class ListHolder<std::string> {
public:
	// The STL list is a public member, directly accessible.
	std::list<std::string> mList;
	// Return a C list of <const char *> from the STL list.
	const bctbx_list_t *getCList() const {
		if (mCList) bctbx_list_free(mCList);
		bctbx_list_t *elem = nullptr, *head = nullptr;
		for (auto &str : mList) {
			if (!head) {
				head = elem = bctbx_list_new((void *)str.c_str());
			} else {
				bctbx_list_t *newElem = bctbx_list_new((void *)str.c_str());
				elem->next = newElem;
				newElem->prev = elem;
				elem = newElem;
			}
		}
		mCList = head;
		return mCList;
	}
	// Assign a C list. This replaces the STL list.
	void setCList(const bctbx_list_t *clist) {
		mList.clear();
		for (const bctbx_list_t *elem = clist; elem != nullptr; elem = elem->next) {
			mList.push_back((const char *)elem->data);
		}
	}
	~ListHolder() {
		if (mCList) bctbx_list_free(mCList);
	}

private:
	mutable bctbx_list_t *mCList = nullptr;
};

/*
 * Template class for classes that hold callbacks (such as LinphoneCallCbs, LinphoneAccountCbs etc.
 * The invocation of callbacks can be done with the LINPHONE_HYBRID_OBJECT_INVOKE_CBS() macro.
 */
template <typename _CppCbsType>
class LINPHONE_PUBLIC CallbacksHolder {
public:
	void addCallbacks(const std::shared_ptr<_CppCbsType> &callbacks) {
		if (find(mCallbacksList.mList.begin(), mCallbacksList.mList.end(), callbacks) == mCallbacksList.mList.end()) {
			mCallbacksList.mList.push_back(callbacks);
			callbacks->setActive(true);
		} else {
			lError() << "Rejected Callbacks " << typeid(_CppCbsType).name() << " [" << (void *)callbacks.get()
			         << "] added twice.";
		}
	}
	void removeCallbacks(const std::shared_ptr<_CppCbsType> &callbacks) {
		auto it = find(mCallbacksList.mList.begin(), mCallbacksList.mList.end(), callbacks);
		if (it != mCallbacksList.mList.end()) {
			mCallbacksList.mList.erase(it);
			callbacks->setActive(false);
		} else {
			lError() << "Attempt to remove " << typeid(_CppCbsType).name() << " [" << (void *)callbacks.get()
			         << "] that does not exist.";
		}
	}
	void setCurrentCallbacks(const std::shared_ptr<_CppCbsType> &callbacks) {
		mCurrentCallbacks = callbacks;
	}
	std::shared_ptr<_CppCbsType> getCurrentCallbacks() const {
		return mCurrentCallbacks;
	}
	const std::list<std::shared_ptr<_CppCbsType>> &getCallbacksList() const {
		return mCallbacksList.mList;
	}
	const bctbx_list_t *getCCallbacksList() const {
		return mCallbacksList.getCList();
	}
	void clearCallbacksList() {
		mCallbacksList.mList.clear();
		mCurrentCallbacks = nullptr;
	}

private:
	ListHolder<_CppCbsType> mCallbacksList;
	std::shared_ptr<_CppCbsType> mCurrentCallbacks;
};

LINPHONE_END_NAMESPACE

#endif // _L_LIST_HOLDER_H_