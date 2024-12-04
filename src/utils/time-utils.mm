/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __APPLE__
#import <Foundation/Foundation.h>

#import "utils/time-utils.h"

LINPHONE_BEGIN_NAMESPACE

time_t iso8601ToTimeApple(std::string iso8601DateTime) {
	NSString* iso8601DateTimeString = [NSString stringWithCString:iso8601DateTime.c_str() encoding:[NSString defaultCStringEncoding]];
	NSISO8601DateFormatter* dateFormatter = [[NSISO8601DateFormatter alloc] init];
	NSDate* dateString = [dateFormatter dateFromString:iso8601DateTimeString];
	return (time_t)[dateString timeIntervalSince1970];
}

std::string timeToIso8601Apple(time_t t) {
	NSDate* nsDateTime = [NSDate dateWithTimeIntervalSince1970:t];
	NSISO8601DateFormatter* dateFormatter = [[NSISO8601DateFormatter alloc] init];
	NSString* dateString = [dateFormatter stringFromDate:nsDateTime];
	return std::string([dateString UTF8String]);
}

LINPHONE_END_NAMESPACE

#endif // __APPLE__
