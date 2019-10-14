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

public enum SubscriptionState {
	/**
	 * Initial state, should not be used.
	 */
	None(0), 
	/**
	 * An outgoing subcription was created.
	 */
	OutoingProgress(1),
	/**
	 * An incoming subcription is received.
	 */
	IncomingReceived(2), 
	/**
	 * Subscription is pending, waiting for user approval
	 */
	Pending(3),
	/**
	 * Subscription is accepted and now active.
	 */
	Active(4),
	/**
	 * Subscription is terminated normally
	 */
	Terminated(5), 
	/**
	 * Subscription encountered an error, indicated by {@link LinphoneEvent#getReason()}
	 */
	Error(6),
	
	/**
	 * Subscription is about to expire, only notified if [sip]-&lt;refresh_generic_subscribe property is set to 0
	 */
	Expiring(7);
	protected final int mValue;
	private SubscriptionState(int value){
		mValue=value;
	}
	static protected SubscriptionState fromInt(int value) throws LinphoneCoreException{
		switch(value){
		case 0: return None;
		case 1: return OutoingProgress;
		case 2: return IncomingReceived;
		case 3: return Pending;
		case 4: return Active;
		case 5: return Terminated;
		case 6: return Error;
		case 7: return Expiring;
		default:
			throw new LinphoneCoreException("Unhandled enum value "+value+" for SubscriptionState");
		}
	}
}
