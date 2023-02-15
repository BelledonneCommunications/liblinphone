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

#ifndef LINPHONE_DAEMON_COMMAND_VIDEO_H
#define LINPHONE_DAEMON_COMMAND_VIDEO_H

#include "daemon.h"

class Video : public DaemonCommand {
public:
	Video();

	void exec(Daemon *app, const std::string &args) override;

	class Preview : public DaemonCommand {
	public:
		Preview();
		void exec(Daemon *app, const std::string &args) override;
	};
};

class VideoSource : public DaemonCommand {
public:
	VideoSource();

	void exec(Daemon *app, const std::string &args) override;
};

class AutoVideo : public DaemonCommand {
public:
	AutoVideo();

	void exec(Daemon *app, const std::string &args) override;
};

class VideoSourceGet : public DaemonCommand {
public:
	VideoSourceGet();

	void exec(Daemon *app, const std::string &args) override;
};

class VideoSourceList : public DaemonCommand {
public:
	VideoSourceList();

	void exec(Daemon *app, const std::string &args) override;
};

class VideoSourceSet : public DaemonCommand {
public:
	VideoSourceSet();

	void exec(Daemon *app, const std::string &args) override;
};

class VideoSourceReload : public DaemonCommand {
public:
	VideoSourceReload();

	void exec(Daemon *app, const std::string &args) override;
};

class VideoDisplayGet : public DaemonCommand {
public:
	VideoDisplayGet();

	void exec(Daemon *app, const std::string &args) override;
};

class VideoDisplaySet : public DaemonCommand {
public:
	VideoDisplaySet();

	void exec(Daemon *app, const std::string &args) override;
};

#endif // LINPHONE_DAEMON_COMMAND_VIDEO_H
