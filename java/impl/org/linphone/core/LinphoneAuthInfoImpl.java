/*
LinphoneAuthInfoImpl.java
Copyright (C) 2010  Belledonne Communications, Grenoble, France

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
package org.linphone.core;

class LinphoneAuthInfoImpl implements LinphoneAuthInfo {
	protected final long nativePtr;
	private native long newLinphoneAuthInfo();
	private native void  delete(long ptr);
	private native String getPassword(long ptr);
	private native String getRealm(long ptr);
	private native String getUsername(long ptr);
	private native String getAlgorithm(long ptr);
	private native void setPassword(long ptr, String password);
	private native void setRealm(long ptr, String realm);
	private native void setUsername(long ptr, String username);
	private native void setAlgorithm(long ptr, String algorithm);
	private native void setUserId(long ptr, String username);
	private native void setHa1(long ptr, String ha1);
	private native void setDomain(long ptr, String domain);
	private native void setTlsCertificate(long ptr, String cert);
	private native void setTlsKey(long ptr, String key);
	private native void setTlsCertificatePath(long ptr, String path);
	private native void setTlsKeyPath(long ptr, String path);
	private native String getUserId(long ptr);
	private native String getHa1(long ptr);
	private native String getDomain(long ptr);
	private native String getTlsCertificate(long ptr);
	private native String getTlsKey(long ptr);
	private native String getTlsCertificatePath(long ptr);
	private native String getTlsKeyPath(long ptr);
	
	boolean ownPtr = false;
	protected LinphoneAuthInfoImpl(String username,String password, String realm, String domain)  {
		this(username, null, password, null, realm, domain);
	}
	protected LinphoneAuthInfoImpl(String username, String userid, String passwd, String ha1, String realm, String domain)  {
		this(username, userid, passwd, ha1, realm, domain, null);
	}
	protected LinphoneAuthInfoImpl(String username, String userid, String passwd, String ha1, String realm, String domain, String algorithm)  {
		nativePtr = newLinphoneAuthInfo();
		this.setUsername(username);
		this.setUserId(userid);
		this.setPassword(passwd);
		this.setHa1(ha1);
		this.setDomain(domain);
		this.setRealm(realm);
		this.setAlgorithm(algorithm);
		ownPtr = true;
	}
	protected LinphoneAuthInfoImpl(long aNativePtr)  {
		nativePtr = aNativePtr;
		ownPtr = false;
	}
	protected void finalize() throws Throwable {
		if (ownPtr) delete(nativePtr);
	}
	public String getPassword() {
		return getPassword (nativePtr);
	}
	public String getRealm() {
		return getRealm (nativePtr);
	}
	public String getUsername() {
		return getUsername (nativePtr);
	}
	public String getAlgorithm() { return  getAlgorithm (nativePtr); }
	public void setPassword(String password) {
		setPassword(nativePtr,password);
	}
	public void setRealm(String realm) {
		setRealm(nativePtr,realm);
	}
	public void setUsername(String username) {
		setUsername(nativePtr,username);
	}
	public void setAlgorithm(String algorithm) { setAlgorithm(nativePtr,algorithm); }
	@Override
	public String getUserId() {
		return getUserId(nativePtr);
	}
	@Override
	public void setUserId(String userid) {
		setUserId(nativePtr,userid);
		
	}
	@Override
	public String getHa1() {
		return getHa1(nativePtr);
	}
	@Override
	public void setHa1(String ha1) {
		setHa1(nativePtr,ha1);
		
	}
	@Override
	public void setDomain(String domain) {
		setDomain(nativePtr, domain);
	}
	@Override
	public String getDomain() {
		return getDomain(nativePtr);
	}
	
	public LinphoneAuthInfo clone() {
		LinphoneAuthInfo clone = LinphoneCoreFactory.instance().createAuthInfo(
				getUsername(), 
				getUserId(), 
				getPassword(), 
				getHa1(), 
				getRealm(), 
				getDomain(),
				getAlgorithm());
		return clone;
	}
	
	@Override
	public String getTlsCertificate() {
		return getTlsCertificate(nativePtr);
	}
	
	@Override
	public String getTlsKey() {
		return getTlsKey(nativePtr);
	}
	
	@Override
	public String getTlsCertificatePath() {
		return getTlsCertificatePath(nativePtr);
	}
	
	@Override
	public String getTlsKeyPath() {
		return getTlsKeyPath(nativePtr);
	}
	
	@Override
	public void setTlsCertificate(String cert) {
		setTlsCertificate(nativePtr, cert);
	}
	
	@Override
	public void setTlsKey(String key) {
		setTlsKey(nativePtr, key);
	}
	
	@Override
	public void setTlsCertificatePath(String path) {
		setTlsCertificatePath(nativePtr, path);
	}
	
	@Override
	public void setTlsKeyPath(String path) {
		setTlsKeyPath(nativePtr, path);
	}
}
