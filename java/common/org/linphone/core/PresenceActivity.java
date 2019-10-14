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

public interface PresenceActivity {

	/**
	 * Gets the string representation of a presence activity.
	 * @return A String representing the given activity.
	 */
	String toString();

	/**
	 * Gets the activity type of a presence activity.
	 * @return The #PresenceActivityType of the activity.
	 */
	PresenceActivityType getType();

	/**
	 * Sets the type of activity of a presence activity.
	 * @param type The activity type to set for the activity.
	 * @return 0 if successful, a value &lt; 0 in case of error.
	 */
	int setType(PresenceActivityType type);

	/**
	 * Gets the description of a presence activity.
	 * @return A String containing the description of the presence activity, or null if no description is specified.
	 */
	String getDescription();

	/**
	 * Sets the description of a presence activity.
	 * @param description An additional description of the activity. Can be null if no additional description is to be added.
	 * @return 0 if successful, a value &lt; 0 in case of error.
	 */
	int setDescription(String description);

	/**
	 * Gets the native pointer for this object.
	 */
	long getNativePtr();

}
