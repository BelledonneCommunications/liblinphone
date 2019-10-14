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
package org.linphone.core;

public class TunnelConfigImpl implements TunnelConfig{
	long mNativePtr;
	protected TunnelConfigImpl(long nativePtr){
		mNativePtr = nativePtr;
	}
	private native String getHost(long nativePtr);
	@Override
	public String getHost() {
		return getHost(mNativePtr);
	}
	private native void setHost(long nativePtr, String host);
	@Override
	public void setHost(String host) {
		setHost(mNativePtr, host);
	}
	private native int getPort(long nativePtr);
	@Override
	public int getPort() {
		return getPort(mNativePtr);
	}
	private native void setPort(long nativePtr, int port);
	@Override
	public void setPort(int port) {
		setPort(mNativePtr, port);
	}
	private native String getHost2(long nativePtr);
	@Override
	public String getHost2() {
		return getHost2(mNativePtr);
	}
	private native void setHost2(long nativePtr, String host);
	@Override
	public void setHost2(String host) {
		setHost2(mNativePtr, host);
	}
	private native int getPort2(long nativePtr);
	@Override
	public int getPort2() {
		return getPort2(mNativePtr);
	}
	private native void setPort2(long nativePtr, int port);
	@Override
	public void setPort2(int port) {
		setPort2(mNativePtr, port);
	}
	private native int getRemoteUdpMirrorPort(long nativePtr);
	@Override
	public int getRemoteUdpMirrorPort() {
		return getRemoteUdpMirrorPort(mNativePtr);
	}
	private native void setRemoteUdpMirrorPort(long nativePtr, int remoteUdpMirrorPort);
	@Override
	public void setRemoteUdpMirrorPort(int remoteUdpMirrorPort) {
		setRemoteUdpMirrorPort(mNativePtr, remoteUdpMirrorPort);
	}
	private native int getDelay(long nativePtr);
	@Override
	public int getDelay() {
		return getDelay(mNativePtr);
	}
	private native void setDelay(long nativePtr, int delay);
	@Override
	public void setDelay(int delay) {
		setDelay(mNativePtr, delay);
	}
	private native void enableSip(long nativePtr, boolean enabled);
	private native void destroy(long nativePtr);
	protected void finalize() throws Throwable {
		if (mNativePtr!=0) destroy(mNativePtr);
	}
}
