/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#include "sal/sal_media_description.h"
#include "c-wrapper/internal/c-sal-stream-description.h"

bool SalMediaDescription::hasDir(const SalStreamDir & stream_dir) const {
	if (stream_dir==SalStreamRecvOnly){
		return containsStreamWithDir(SalStreamRecvOnly) && !(containsStreamWithDir(SalStreamSendOnly) || containsStreamWithDir(SalStreamSendRecv));
	}else if (stream_dir==SalStreamSendOnly){
		return containsStreamWithDir(SalStreamSendOnly) && !(containsStreamWithDir(SalStreamRecvOnly) || containsStreamWithDir(SalStreamSendRecv));
	}else if (stream_dir==SalStreamSendRecv){
		return containsStreamWithDir(SalStreamSendRecv);
	}else{
		/*SalStreamInactive*/
		if (containsStreamWithDir(SalStreamSendOnly) || containsStreamWithDir(SalStreamSendRecv)  || containsStreamWithDir(SalStreamRecvOnly))
			return FALSE;
		else return TRUE;
	}
	return FALSE;
}

bool SalMediaDescription::containsStreamWithDir(const SalStreamDir & stream_dir) const{
	/* we are looking for at least one stream with requested direction, inactive streams are ignored*/
	for(auto & stream : streams){
		if (!sal_stream_description_enabled(&stream)) continue;
		if (stream.dir==stream_dir) {
			return TRUE;
		}
		/*compatibility check for phones that only used the null address and no attributes */
		if (stream.dir==SalStreamSendRecv && stream_dir==SalStreamSendOnly && (isNullAddress(addr) || isNullAddress(stream.rtp_addr))){
			return TRUE;
		}
	}
	return FALSE;
}

bool SalMediaDescription::isNullAddress(const std::string & addr) const {
	return addr.compare("0.0.0.0")==0 || addr.compare("::0")==0;
}
