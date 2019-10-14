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

/**
 * The LinphoneInfoMessage represents an informational message (INFO) to be transmitted or received by the LinphoneCore.
 * It can be created with {@link LinphoneCore#createInfoMessage()}.
 * @author smorlat
 *
 */
public interface LinphoneInfoMessage {
	/**
	 * Assign a content to the info message. This is optional.
	 * @param content
	 */
	void setContent(LinphoneContent content);
	/**
	 * Get the actual content of the info message. It may be null.
	 * @return a LinphoneContent object or null
	 */
	LinphoneContent getContent();
	/**
	 * Add a specific header to the info message
	 * @param name the header's name
	 * @param value the header's value
	 */
	void addHeader(String name, String value);
	/**
	 * Retrieve a header's value based on its name.
	 * @param name the header's name
	 * @return the header's value
	 */
	String getHeader(String name);
}
