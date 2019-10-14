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
package org.linphone.tools;

import org.linphone.core.LpConfig;
import org.linphone.mediastream.Log;

public class Xml2Lpc {
	
    private enum LogLevel {
    	DEBUG,
    	MESSAGE,
    	WARNING,
    	ERROR
    }
    
    private static boolean mAvailable;
    
	private long internalPtr = 0;
	
	private native void init();
	private native void destroy();
	
	public Xml2Lpc() {
		init();
	}
	
	public void finalize() {
		destroy();
	}
	
	public native int setXmlFile(String filename);
	public native int setXmlString(String content);

	public native int setXsdFile(String filename);
	public native int setXsdString(String content);

	public native int validate();
	public native int convert(LpConfig config);
	
	public void printLog(int level, String message) {
		if(level > 0 && level < LogLevel.values().length) {
			switch(LogLevel.values()[level]) {
				case DEBUG:
					Log.d(message);
				break;
				case MESSAGE:
					Log.i(message);
				break;
				case WARNING:
					Log.w(message);
				break;
				case ERROR:
					Log.e(message);
				break;
			}
		}
	}
	
	public static boolean isAvailable() {
		return mAvailable;
	}
	
	// Load library
	static {
		try { 
			new Xml2Lpc();
			//System.loadLibrary("xml2");
			//System.loadLibrary("xml2lpc");
			mAvailable = true;
		} catch (Throwable e) {
			mAvailable = false;
		}
	}
}
