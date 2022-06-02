/** SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Pointer wrappers for explicit ownership semantics.
 * Those are NOT smart pointers (they do not manage memory)
 *
 * They can all be constructed from raw pointers to interop with existing code, but the conversion is explicit to make
 * sure the intent is clear.
 */

#pragma once

namespace ownership {

/* A pointer that is NOT responsible for the destruction of its pointee.
 *
 * Same as `Borrowed<T>` but mutable (non-const)
 */
template <class T> class BorrowedMut {
	T *mPointer;

  public:
	/* State explicitly that this pointer represents a mutable borrow
	 */
	explicit BorrowedMut(T *pointer) : mPointer(pointer) {
	}

	operator T *() {
		return mPointer;
	};
};

/* A pointer that IS responsible for the destruction of its pointee.
 *
 * When reference counting, having an Owned<T> means the reference count has been incremented.
 *
 * Use .take() to move the pointer out of the wrapper (typically before deleting the pointee).
 * Use .borrow() to use mutating operations without incrementing the reference count.
 */
template <class T> class Owned {
	T *mPointer;

	void assertFreed() {
		if (mPointer != nullptr) {
			throw std::logic_error("Owned pointer lost. If you did free it, maybe you forgot to call .take() before");
		}
	}

  public:
	Owned(std::nullptr_t null) : mPointer(null) {
	}
	/* State explicitly that this pointer owns its pointee
	 */
	explicit Owned(T *pointer) : mPointer(pointer) {
	}
	~Owned() {
		assertFreed();
	}

	// Move only semantics to ensure ownership is always transferred
	Owned(Owned<T> &&other) : mPointer(other.take()) {
	}
	Owned &operator=(Owned<T> &&other) {
		assertFreed();
		mPointer = other.take();
		return *this;
	}
	Owned(const Owned<T> &other) = delete;
	Owned &operator=(const Owned<T> &other) = delete;

	/* Move the raw pointer out of the wrapper (leaving nullptr in its place).
	 * Use it to pass ownership to the appropriate destructor
	 */
	T *take() {
		auto moved = mPointer;
		mPointer = nullptr;
		return moved;
	}

	/* Take a mutable borrow.
	 * Use it to call mutating operations that don't need ownership
	 */
	BorrowedMut<T> borrow() {
		return BorrowedMut<T>(mPointer);
	}

	/* Automatically degrade into const pointer because that is always safe
	 */
	operator const T *() const {
		return mPointer;
	};
};

/* A const pointer that is NOT responsible for the destruction of its pointee.
 *
 * Care must be taken to ensure appropiate lifetimes: Since the borrow is not responsible for the lifetime of the
 * pointee, the latter might be destroyed before the borrow is used, leaving it dangling.
 */
template <class T> class Borrowed {
	const T *mPointer;

  public:
	/* State explicitly that this pointer represents an immutable borrow
	 */
	explicit Borrowed(const T *pointer) : mPointer(pointer) {
	}
	Borrowed(const Owned<T> &owned) : Borrowed(static_cast<const T *>(owned)) {
	}
	Borrowed(const BorrowedMut<T> &mutBorrow) : Borrowed(static_cast<const T *>(mutBorrow)) {
	}

	operator const T *() const {
		return mPointer;
	};
};

template <class T> auto owned(T *pointer) {
	return Owned<T>(pointer);
}

template <class T> auto borrowed(const T *pointer) {
	return Borrowed<T>(pointer);
}

template <class T> auto borrowed_mut(T *pointer) {
	return BorrowedMut<T>(pointer);
}

} // namespace ownership
