/*
 * media-device-watcher-linux.h
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _L_MEDIA_DEVICE_WATCHER_LINUX_H_
#define _L_MEDIA_DEVICE_WATCHER_LINUX_H_

#include <libudev.h>

#include "logger/logger.h"
#include "media-device-watcher.h"
#include "object/object-p.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class MediaDeviceWatcherPrivate : public ObjectPrivate {
public:
	udev *udevContext;
	udev_monitor *udevMonitor;

	void clean ();
};

void MediaDeviceWatcherPrivate::clean () {
	if (udevMonitor)
		udev_monitor_unref(udevMonitor);
	if (udevContext)
		udev_unref(udevContext);
}

MediaDeviceWatcher::MediaDeviceWatcher () : Object(*new MediaDeviceWatcherPrivate) {
	L_D();

	if (!(d->udevContext = udev_new())) {
		lError() << "Unable to create udev context.";
		return;
	}

	if (!(d->udevMonitor = udev_monitor_new_from_netlink(d->udevContext, "udev"))) {
		lError() << "Unable to create udev monitor.";
		d->clean();
		return;
	}

	for (const char *subsystem : { "sound", "video4linux" })
		if (udev_monitor_filter_add_match_subsystem_devtype(d->udevMonitor, subsystem, nullptr) < 0) {
			lError() << "Unable to monitor: " << subsystem;
			d->clean();
			return;
		}

	int fd = udev_monitor_get_fd(d->udevMonitor);
	if (fd < 0) {
		d->clean();
		return;
	}

	// TODO: Deal with fd. Add a socket notifier in core?
}

MediaDeviceWatcher::~MediaDeviceWatcher () {
	L_D();
	d->clean();
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MEDIA_DEVICE_WATCHER_LINUX_H_
