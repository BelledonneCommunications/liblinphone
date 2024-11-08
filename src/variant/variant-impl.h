/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef _L_VARIANT_IMPL_H_
#define _L_VARIANT_IMPL_H_

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC VariantImplBase {
public:
	virtual VariantImplBase *clone() = 0;
	virtual ~VariantImplBase(){};
	virtual std::ostream &toStream(std::ostream &stream) const = 0;
};

template <typename T>
class LINPHONE_PUBLIC VariantImpl : public VariantImplBase {
public:
	VariantImpl() = default;

	VariantImpl(const T &value) {
		mValue = value;
	}

	VariantImpl(const VariantImpl<T> &other) = default;

	~VariantImpl() = default;

	VariantImplBase *clone() override {
		return new VariantImpl<T>(*this);
	}

	const T &getValue() const {
		return mValue;
	}

	void setValue(const T &value) {
		mValue = value;
	}

	std::ostream &toStream(std::ostream &stream) const override {
		stream << mValue;
		return stream;
	}

private:
	T mValue;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_VARIANT_IMPL_H_