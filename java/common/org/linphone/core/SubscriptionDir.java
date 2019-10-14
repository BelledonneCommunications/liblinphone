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

import java.util.Vector;

public enum SubscriptionDir {
	Incoming(0),
	Outgoing(1),
	Invalid(2);
	protected final int mValue;
	private SubscriptionDir(int value){
		mValue=value;
	}
	static protected SubscriptionDir fromInt(int value){
		switch(value){
		case 0: return Incoming;
		case 1: return Outgoing;
		}
		return Invalid;
	}
}
