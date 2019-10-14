/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/**
 *
 */
package org.linphone.core;

/**
 * @author François Grisez
 *
 */
public class LinphonePlayerImpl implements LinphonePlayer {
	private long nativePtr = 0;

	private native void init(long nativePtr);
	LinphonePlayerImpl(long nativePtr) {
		this.nativePtr = nativePtr;
		init(nativePtr);
	}

	private native int open(long nativePtr, String filename);
	@Override
	public synchronized int open(String filename) {
		return open(nativePtr, filename);
	}

	private native int start(long nativePtr);
	@Override
	public synchronized int start() {
		return start(nativePtr);
	}

	private native int pause(long nativePtr);
	@Override
	public synchronized int pause() {
		return pause(nativePtr);
	}

	private native int seek(long nativePtr, int timeMs);
	@Override
	public synchronized int seek(int timeMs) {
		return seek(nativePtr, timeMs);
	}

	private native int getState(long nativePtr);
	@Override
	public synchronized State getState() {
		return LinphonePlayer.State.fromValue(getState(nativePtr));
	}

	private native int getDuration(long nativePtr);
	@Override
	public synchronized int getDuration() {
		return getDuration(nativePtr);
	}

	private native int getCurrentPosition(long nativePtr);
	@Override
	public synchronized int getCurrentPosition() {
		return getCurrentPosition(nativePtr);
	}

	private native void close(long nativePtr);
	@Override
	public synchronized void close() {
		close(nativePtr);
	}

	private native void destroy(long nativePtr);
	@Override
	protected void finalize() {
		destroy(nativePtr);
	}
}
