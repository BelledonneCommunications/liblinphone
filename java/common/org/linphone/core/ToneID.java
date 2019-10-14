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

public enum ToneID {
	Undefined(0),
	Busy(1),
	CallWaiting(2),
	CallOnHold(3),
	CallLost(4);
	protected final int mValue;
	private ToneID(int value){
		mValue=value;
	}
	static protected ToneID fromInt(int value) throws LinphoneCoreException{
		switch(value){
		case 0: return Undefined;
		case 1: return Busy;
		case 2: return CallWaiting;
		case 3: return CallOnHold;
		case 4: return CallLost;
		default:
			throw new LinphoneCoreException("Unhandled enum value "+value+" for LinphoneToneID");
		}
	}
}
