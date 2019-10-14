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

public class LinphoneInfoMessageImpl implements LinphoneInfoMessage {
	protected long nativePtr;
	private LinphoneContent mContent;
	
	private native Object getContent(long infoptr);
	public LinphoneInfoMessageImpl(long ptr){
		nativePtr=ptr;
		mContent=(LinphoneContent)getContent(nativePtr);
	}
	
	private native void setContent(long nativePtr, String type, String subtype, String data);
	@Override
	public void setContent(LinphoneContent content) {
		mContent=content;
		setContent(nativePtr,mContent.getType(),mContent.getSubtype(),mContent.getDataAsString());
	}

	@Override
	public LinphoneContent getContent() {
		return mContent;
	}

	private native void addHeader(long nativePtr, String name, String value);
	@Override
	public void addHeader(String name, String value) {
		addHeader(nativePtr,name,value);
	}

	private native String getHeader(long nativePtr, String name);
	@Override
	public String getHeader(String name) {
		return getHeader(nativePtr,name);
	}
	
	private native void delete(long nativePtr);
	protected void finalize(){
		delete(nativePtr);
	}
}
