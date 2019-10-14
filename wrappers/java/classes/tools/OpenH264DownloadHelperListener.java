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
package org.linphone.core.tools;

public interface OpenH264DownloadHelperListener {
	/**
	 * Called at the beginning of download with current &lt; max Called
	 * at each iteration of download Called at the ending of download
	 * with current &gt; max
	 * @param current: Size of file already downloaded
	 * @param max: Size of file we want to download
	 */
	void OnProgress(int current, int max);
	
	/**
	 * Called when we failed to download codec
	 * @param error: Error message
	 */
	void OnError(String error);
}
