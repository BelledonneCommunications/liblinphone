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

import android.content.Context;
import android.os.Build;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.linphone.mediastream.Log;
import org.linphone.mediastream.MediastreamerAndroidContext;
import org.linphone.mediastream.Version;
import org.linphone.tools.OpenH264DownloadHelper;

public class LinphoneCoreFactoryImpl extends LinphoneCoreFactory {
	private static boolean loadOptionalLibrary(String s) {
		try {
			System.loadLibrary(s);
			return true;
		} catch (Throwable e) {
			android.util.Log.w("LinphoneCoreFactoryImpl", "Unable to load optional library " + s + ": " +e.getMessage());
		}
		return false;
	}

	static {
		System.loadLibrary("c++_shared");
		loadOptionalLibrary("ffmpeg-linphone");
		System.loadLibrary("bctoolbox");
		System.loadLibrary("ortp");
		System.loadLibrary("mediastreamer");
		System.loadLibrary("linphone");
		Version.dumpCapabilities();
	}
	@Override
	public LinphoneAuthInfo createAuthInfo(String username, String password,
			String realm, String domain) {
		return new LinphoneAuthInfoImpl(username, password, realm, domain);
	}

	@Override
	public LinphoneAddress createLinphoneAddress(String username,
			String domain, String displayName) {
		return new LinphoneAddressImpl(username,domain,displayName);
	}

	@Override
	public LinphoneAddress createLinphoneAddress(String identity) throws LinphoneCoreException {
		return new LinphoneAddressImpl(identity);
	}

	@Override
	public LpConfig createLpConfig(String file) {
		return LpConfigImpl.fromFile(file);
	}

	public LpConfig createLpConfigFromString(String buffer) {
		return LpConfigImpl.fromBuffer(buffer);
	}

	private boolean loadingDownloadedOpenH264(Context context) {
		File file = new File(context.getApplicationInfo().nativeLibraryDir+"/libmsopenh264.so");

		if (!file.exists()) {
			Log.i("LinphoneCoreFactoryImpl"," libmsopenh264 not found, we disable the download of Openh264");
			return false;
		}

		OpenH264DownloadHelper downloadHelper = new OpenH264DownloadHelper(context);
		if (downloadHelper.isCodecFound()) {
			Log.i("LinphoneCoreFactoryImpl"," Loading OpenH264 downloaded plugin:" + downloadHelper.getFullPathLib());
			System.load(downloadHelper.getFullPathLib());
		} else {
			Log.i("LinphoneCoreFactoryImpl"," Cannot load OpenH264 downloaded plugin");
		}
		return true;
	}

	private void setSpeaker(LinphoneCore lc) {
		boolean value = MediastreamerAndroidContext.getSpeakerphoneAlwaysOn(lc.getMSFactory());
		lc.setSpeakerphoneAlwaysOn(value);
		lc.enableSpeaker(value);
	}

	@Override
	public LinphoneCore createLinphoneCore(LinphoneCoreListener listener,
			String userConfig, String factoryConfig, Object userdata, Object context)
			throws LinphoneCoreException {
		try {
			fcontext = (Context)context;
			boolean openh264DownloadEnabled = false;
			if (context != null) openh264DownloadEnabled = loadingDownloadedOpenH264(fcontext);
			MediastreamerAndroidContext.setContext(context);
			File user = userConfig == null ? null : new File(userConfig);
			File factory = factoryConfig == null ? null : new File(factoryConfig);
			LinphoneCore lc = new LinphoneCoreImpl(listener, user, factory, userdata, context);
			setSpeaker(lc);
			lc.enableDownloadOpenH264(openh264DownloadEnabled);
			return lc;
		} catch (IOException e) {
			throw new LinphoneCoreException("Cannot create LinphoneCore",e);
		}
	}

	@Override
	public LinphoneCore createLinphoneCore(LinphoneCoreListener listener, Object context) throws LinphoneCoreException {
		try {
			fcontext = (Context)context;
			boolean openh264DownloadEnabled = false;
			if (context != null) openh264DownloadEnabled = loadingDownloadedOpenH264(fcontext);
			MediastreamerAndroidContext.setContext(context);
			LinphoneCore lc = new LinphoneCoreImpl(listener, context);
			setSpeaker(lc);
			lc.enableDownloadOpenH264(openh264DownloadEnabled);
			return lc;
		} catch (IOException e) {
			throw new LinphoneCoreException("Cannot create LinphoneCore",e);
		}
	}

	@Override
	public native void setDebugMode(boolean enable, String tag);


	private native void _setLogHandler(Object handler);
	@Override
	public void setLogHandler(LinphoneLogHandler handler) {
		_setLogHandler(handler);
	}

	@Override
	public OpenH264DownloadHelper createOpenH264DownloadHelper() {
		if (fcontext == null) {
			new LinphoneCoreException("Cannot create OpenH264DownloadHelper");
			return null;//exception
		}
		return new OpenH264DownloadHelper(fcontext);
	}

	@Override
	@Deprecated
	public LinphoneFriend createLinphoneFriend(String friendUri) {
		return new LinphoneFriendImpl(friendUri);
	}

	@Override
	@Deprecated
	public LinphoneFriend createLinphoneFriend() {
		return createLinphoneFriend(null);
	}

	@Override
	public native void enableLogCollection(boolean enable);

	@Override
	public native void setLogCollectionPath(String path);

	public static boolean isArmv7()
	{
		return System.getProperty("os.arch").contains("armv7");
	}

	@Override
	public LinphoneAuthInfo createAuthInfo(String username, String userid,
			String passwd, String ha1, String realm, String domain) {
		return new LinphoneAuthInfoImpl(username, userid, passwd, ha1, realm, domain);
	}

	@Override
	public LinphoneAuthInfo createAuthInfo(String username, String userid,
										   String passwd, String ha1, String realm, String domain,
										   String algorithm) {
		return new LinphoneAuthInfoImpl(username, userid, passwd, ha1, realm, domain, algorithm);
	}

	@Override
	public LinphoneContent createLinphoneContent(String type, String subType,
			byte [] data, String encoding) {
		return new LinphoneContentImpl(type,subType,data,encoding);
	}

	@Override
	public LinphoneContent createLinphoneContent(String type, String subType,
			String data) {
		return new LinphoneContentImpl(type,subType,data == null ? null : data.getBytes(), null);
	}

	@Override
	public PresenceActivity createPresenceActivity(PresenceActivityType type, String description) {
		return new PresenceActivityImpl(type, description);
	}

	@Override
	public PresenceService createPresenceService(String id, PresenceBasicStatus status, String contact) {
		return new PresenceServiceImpl(id, status, contact);
	}

	@Override
	public PresenceModel createPresenceModel() {
		return new PresenceModelImpl();
	}

	@Override
	public PresenceModel createPresenceModel(PresenceActivityType type, String description) {
		return new PresenceModelImpl(type, description);
	}

	@Override
	public PresenceModel createPresenceModel(PresenceActivityType type, String description, String note, String lang) {
		return new PresenceModelImpl(type, description, note, lang);
	}

	private native Object _createTunnelConfig();
	@Override
	public TunnelConfig createTunnelConfig() {
		return (TunnelConfig)_createTunnelConfig();
	}

	@Override
	public LinphoneAccountCreator createAccountCreator(LinphoneCore lc, String url) {
		return new LinphoneAccountCreatorImpl(lc, url);
	}

	private native DialPlan[] getAllDialPlanNative();
	@Override
	public DialPlan[] getAllDialPlan(){
		return getAllDialPlanNative();
	}
	
	private native long createErrorInfoNative();
	@Override
	public ErrorInfo createErrorInfo(){
		return new ErrorInfoImpl(createErrorInfoNative(), true);
	}
}
