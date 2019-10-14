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

public class LinphoneXmlRpcSessionImpl implements LinphoneXmlRpcSession {
	protected long nativePtr;
	
	private native long newLinphoneXmlRpcSession(long nativePtr, String url);
	public LinphoneXmlRpcSessionImpl(LinphoneCore lc, String url) {
		nativePtr = newLinphoneXmlRpcSession(((LinphoneCoreImpl)lc).nativePtr, url);
	}
	
	public long getNativePtr() {
		return nativePtr;
	}

	private native void unref(long ptr);
	protected void finalize(){
		unref(nativePtr);
	}

	private native void sendRequest(long ptr, long requestPtr);
	@Override
	public void sendRequest(LinphoneXmlRpcRequest request) {
		sendRequest(nativePtr, ((LinphoneXmlRpcRequestImpl)request).getNativePtr());
	}
}
