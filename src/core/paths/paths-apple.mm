/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#import <Foundation/Foundation.h>

#import "logger/logger.h"
#import "linphone/api/c-factory.h"

#import "paths-apple.h"

#define TEST_GROUP_ID "test group id"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE


std::string SysPaths::getDataPath (void *context) {
	NSString *fullPath;
	if(linphone_factory_is_data_dir_set(linphone_factory_get()))
		fullPath = [NSString stringWithUTF8String:linphone_factory_get_data_dir(linphone_factory_get(), context)];
	else{
		if (context && strcmp(static_cast<const char *>(context), TEST_GROUP_ID) != 0) {
			const char* appGroupId = static_cast<const char *>(context);
			NSString *objcGroupdId = [NSString stringWithCString:appGroupId encoding:[NSString defaultCStringEncoding]];
	
			NSURL *basePath = [[NSFileManager defaultManager] containerURLForSecurityApplicationGroupIdentifier:objcGroupdId];
			fullPath = [[basePath path] stringByAppendingString:@"/Library/Application Support/linphone/"];
		} else {
			NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
			NSString *writablePath = [paths objectAtIndex:0];
			fullPath = [writablePath stringByAppendingString:@"/linphone/"];
		}
	
		if (![[NSFileManager defaultManager] fileExistsAtPath:fullPath]) {
			NSError *error;
			lInfo() << "Data path " << fullPath.UTF8String << " does not exist, creating it.";
			if (![[NSFileManager defaultManager] createDirectoryAtPath:fullPath
										   withIntermediateDirectories:YES
															attributes:nil
																 error:&error]) {
				lError() << "Create data path directory error: " << error.description;
			}
		}
	}
	return fullPath.UTF8String;
}

std::string SysPaths::getConfigPath (void *context) {
	NSString *fullPath;
	if(linphone_factory_is_config_dir_set(linphone_factory_get()))
		fullPath = [NSString stringWithUTF8String:linphone_factory_get_config_dir(linphone_factory_get(), context)];
	else{
		if (context && strcmp(static_cast<const char *>(context), TEST_GROUP_ID) != 0) {
			const char* appGroupId = static_cast<const char *>(context);
			NSString *objcGroupdId = [NSString stringWithCString:appGroupId encoding:[NSString defaultCStringEncoding]];
	
			NSURL *basePath = [[NSFileManager defaultManager] containerURLForSecurityApplicationGroupIdentifier:objcGroupdId];
			fullPath = [[basePath path] stringByAppendingString:@"/Library/Preferences/linphone/"];
		} else {
			NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
			NSString *configPath = [paths objectAtIndex:0];
			fullPath = [configPath stringByAppendingString:@"/Preferences/linphone/"];
		}
	
		if (![[NSFileManager defaultManager] fileExistsAtPath:fullPath]) {
			NSError *error;
			lInfo() << "Config path " << fullPath.UTF8String << " does not exist, creating it.";
			if (![[NSFileManager defaultManager] createDirectoryAtPath:fullPath
										   withIntermediateDirectories:YES
															attributes:nil
																 error:&error]) {
				lError() << "Create config path directory error: " << error.description;
			}
		}
	}

	return fullPath.UTF8String;
}

std::string SysPaths::getDownloadPath (void *context) {
	NSString *fullPath;
	if(linphone_factory_is_download_dir_set(linphone_factory_get()))
		fullPath = [NSString stringWithUTF8String:linphone_factory_get_download_dir(linphone_factory_get(), context)];
	else{
/*
 Apple clears Cache when the disk is full. So use "Library/Images/" as the download path.
 */
		NSString *oldFullPath;
		NSFileManager *fileManager = [NSFileManager defaultManager];
		if (context && strcmp(static_cast<const char *>(context), TEST_GROUP_ID) != 0) {
			const char* appGroupId = static_cast<const char *>(context);
			NSString *objcGroupdId = [NSString stringWithCString:appGroupId encoding:[NSString defaultCStringEncoding]];
	
			NSURL *basePath = [fileManager containerURLForSecurityApplicationGroupIdentifier:objcGroupdId];
			oldFullPath = [[basePath path] stringByAppendingString:@"/Library/Caches/"];
			fullPath = [[basePath path] stringByAppendingString:@"/Library/Images/"];
		} else {
			NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
			NSString *configPath = [paths objectAtIndex:0];
			oldFullPath = [configPath stringByAppendingString:@"/"];
			fullPath = [oldFullPath stringByReplacingOccurrencesOfString:@"Caches" withString:@"Images"];
		}

		if (![fileManager fileExistsAtPath:fullPath]) {
			NSError *error;
			lInfo() << "Download path " << fullPath.UTF8String << " does not exist, creating it.";
			if (![fileManager createDirectoryAtPath:fullPath
										   withIntermediateDirectories:YES
															attributes:nil
																 error:&error]) {
				lError() << "Create download path directory error: " << error.description;
				return fullPath.UTF8String;
			}
	
			NSArray *images = [fileManager contentsOfDirectoryAtPath:oldFullPath error:NULL];
			for (NSString *image in images)
			{
				[fileManager copyItemAtPath:[oldFullPath stringByAppendingPathComponent:image] toPath:[fullPath stringByAppendingPathComponent:image] error:nil];
			}
			lInfo() << "Download path migration done.";
		}
	}

	return fullPath.UTF8String;
}

LINPHONE_END_NAMESPACE
