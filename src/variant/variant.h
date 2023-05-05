/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _L_VARIANT_H_
#define _L_VARIANT_H_

#include "bctoolbox/utils.hh"
#include "logger/logger.h"
#include "variant-impl.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC Variant {
public:
	Variant() = default;
	~Variant() = default;

	template <typename T>
	Variant(T value) {
		mImplBase.reset(new VariantImpl<T>(value));
	}

	Variant(const Variant &other) {
		if (other.mImplBase) mImplBase.reset(other.mImplBase->clone());
	}

	Variant(Variant &&other) = default;

	Variant &operator=(const Variant &other) {
		if (other.mImplBase) mImplBase.reset(other.mImplBase->clone());
		return *this;
	}

	Variant &operator=(Variant &&other) = default;

	template <typename T>
	void setValue(const T &value) {
		VariantImpl<T> *vi = dynamic_cast<VariantImpl<T> *>(mImplBase.get());
		if (vi != nullptr) {
			return vi->setValue(value);
		}
	}

	template <typename T>
	const T &getValue() const {
		VariantImpl<T> *vi = dynamic_cast<VariantImpl<T> *>(mImplBase.get());
		if (vi != nullptr) {
			return vi->getValue();
		}
		return bctoolbox::Utils::getEmptyConstRefObject<T>();
	}

	bool isValid() const {
		return mImplBase != nullptr;
	}
	std::ostream &toStream(std::ostream &stream) const {
		if (mImplBase == nullptr) {
			stream << "[undefined]";
			return stream;
		}
		return mImplBase->toStream(stream);
	}

private:
	std::unique_ptr<VariantImplBase> mImplBase;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_VARIANT_H_
