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

#ifndef _L_C_ENUMS_H_
#define _L_C_ENUMS_H_

/*
 * Global enums, not bound to any object.
 */

/**
 * @brief All kinds of alerts
 * @ingroup alert
 */
typedef enum _LinphoneAlertTypes {

	/** Camera is not working. No other information
	 * @note Use the key "camera_misfunction_interval" in the section "alerts::camera" to set the interval
	 * at which the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSCameraMisfunction,
	/** Camera is capturing low framerate. Information supplied : float framerate;
	 *  @note Use the key "low_framerate_interval" in the section "alerts::camera" to set or get the interval at which
	 * the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSCameraLowFramerate,
	/** Video decoding has stopped for a given period (10 s by default). No other information.
	 *  @note Use the key "video_stalled_interval" in the section "alerts::camera" to set or get the interval at which
	 * the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSVideoStalled,
	/** A received media stream suffers from high loss or late rate. Information provided is:
	 * - loss-rate (float)
	 * - late-rate (float)
	 * - media-type (string) with values {audio, video, text}
	 *  @note Use the key "loss_rate_interval" in the section "alerts::network" to set or get the interval at which
	the problem is checked in a #LinphoneConfig.
	*/
	LinphoneAlertQoSHighLossLateRate,
	/** A report of high loss rate is received from remote party. Information provided: loss-rate (float).
	 *  @note Use the key "remote_loss_rate_interval" in the section "alerts::network" to set or get the interval at
	 * which the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSHighRemoteLossRate,
	/** Packet Burst phenomenon
	 *  @note Use the key "burst_occured_interval" in the section "alerts::network" to set or get the interval at which
	 * the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSBurstOccured,
	/** Loss rate is significant but retransmissions fail to arrive on time.
	 * Information provided: nack-performance (float) the fraction of lost packets recovered thanks to nack-triggered
	 * retransmissions.
	 *  @note Use the key "nack_check_interval" in the section "alerts::network" to set or get the interval at which the
	 * problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSRetransmissionFailures,
	/** Low bandwidth detected. Information provided: bandwidth (float) in kbit/s.
	 *  @note Use the key "download_bandwidth_interval" in the section "alerts::video" to set or get the interval at
	 * which the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSLowDownloadBandwidthEstimation,
	/** Low quality (bitrate) video received. Information provided: bitrate (float) in kbit/s, width (integer), int
	 * height (integer).
	 *  @note Use the key "low_quality_received_interval" in the section "alerts::video" to set or get the interval at
	 * which the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSLowQualityReceivedVideo,
	/** Low quality video is being sent. Information provided: bitrate (float)in kbit/s, width (integer), height
	 * (integer).
	 * @note Use the key "quality_sent_interval" in the section "alerts::camera" to set or get the interval at which
	 * the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSLowQualitySentVideo,
	/** The operating system reports a low radio signal (wifi or mobile)
	 * @note Use the key "low_signal_interval" in the section "alerts::network" to set or get the interval at which the
	 * problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSLowSignal,
	/** The operating system reports a loss of radio signal (wifi or mobile).
	 * Information provided: rssi-value (float), signal-type (string) with values {"wifi", "mobile", "other"}.
	 * @note Use the key "lost_signal_interval" in the section "alerts::network" to set or get the interval at which
	 * the problem is checked in a #LinphoneConfig.
	 */
	LinphoneAlertQoSLostSignal

} LinphoneAlertType;

/**
 * All signal types that a device can use.
 * @ingroup signalInformation
 */
typedef enum _LinphoneSignalType {
	LinphoneSignalTypeWifi = 0,
	LinphoneSignalTypeMobile = 1,
	LinphoneSignalTypeOther = 2
} LinphoneSignalType;
/**
 * All signal units that a device can use.
 * @ingroup signalInformation
 */
typedef enum _LinphoneSignalStrengthUnit {
	LinphoneSignalStrengthUnitRssi = 0,
	LinphoneSignalStrengthUnitDbm = 1,
} LinphoneSignalStrengthUnit;

/**
 * Codec priority policies.
 * This enum represents different policies for managing offered codec lists during calls, as well as the offer-answer
 * logic. Currently, policies can be applied only for video codecs.
 * @ingroup media_parameters
 */
typedef enum _LinphoneCodecPriorityPolicy {
	LinphoneCodecPriorityPolicyBasic =
	    0, /**< In this mode, codecs have initial default ordering, that can be changed by the application
	         The answerer of a call accepts codecs with the order given in the offer. */
	LinphoneCodecPriorityPolicyAuto =
	    1 /**< In this mode, the codec list is managed by the #LinphoneCore according to hardware capabilities
	          in the goal of optimizing video quality and user experience. The answerer of call
	          may re-order the offerer's list in its answer in order to give preference to certain codecs.*/
} LinphoneCodecPriorityPolicy;

/**
 * Security level determined by type of encryption (point-to-point, end-to-end, etc...) and whether or not a SAS
 * validation was made with the remote(s) end(s). A #LinphoneSecurityLevelEndToEndEncryptedAndVerified level means it's
 * end-to-end encrypted and SAS validation was made. An #LinphoneSecurityLevelUnsafe level means end-to-end-encrypted
 * but it's likely a man-in-the-middle exists between you and one device.
 * @ingroup misc
 */
typedef enum _LinphoneSecurityLevel {
	LinphoneSecurityLevelUnsafe = 0,                       /**< Security failure */
	LinphoneSecurityLevelNone = 1,                         /**< No encryption */
	LinphoneSecurityLevelEndToEndEncrypted = 2,            /**< End-to-end encrypted */
	LinphoneSecurityLevelEndToEndEncryptedAndVerified = 3, /**< End-to-end encrypted and verified */
	LinphoneSecurityLevelPointToPointEncrypted = 4         /**< Point-to-point encrypted */
} LinphoneSecurityLevel;

/**
 * List of all supported #LinphoneAccountManagerServicesRequest requests,
 * allowing to know which one triggered either a callback from the #LinphoneAccountManagerServicesRequestCbs.
 * @ingroup account_creator
 */
typedef enum _LinphoneAccountManagerServicesRequestType {
	LinphoneAccountManagerServicesRequestTypeSendAccountCreationTokenByPush =
	    0, /**< Asks the account manager to send us an account creation token by push notification */
	LinphoneAccountManagerServicesRequestTypeAccountCreationRequestToken =
	    1, /**< Asks the account manager to create a request token to be validated in a web browser (captcha) and that
	          can later be used to create an account creation token */
	LinphoneAccountManagerServicesRequestTypeAccountCreationTokenFromAccountCreationRequestToken =
	    2, /**< Asks the account manager to consume the account creation request token to create an account creation
	          token */
	LinphoneAccountManagerServicesRequestTypeCreateAccountUsingToken =
	    3, /**< Uses an account creation token to create the account */
	LinphoneAccountManagerServicesRequestTypeSendPhoneNumberLinkingCodeBySms =
	    4, /**< Asks for a code to be sent by SMS to a given phone number to link that phone number with an account */
	LinphoneAccountManagerServicesRequestTypeLinkPhoneNumberUsingCode =
	    5, /**< Uses the code received by SMS to confirm the link between an account and a phone number */
	LinphoneAccountManagerServicesRequestTypeSendEmailLinkingCodeByEmail =
	    6, /**< Asks for a code to be sent by email to a given email address to link that address with an account */
	LinphoneAccountManagerServicesRequestTypeLinkEmailUsingCode =
	    7, /**< Uses the code received by email to confirm the link between an account and an email address */
	LinphoneAccountManagerServicesRequestTypeGetDevicesList = 8, /**< Gets the list of devices for account */
	LinphoneAccountManagerServicesRequestTypeDeleteDevice = 9,   /**< Removes an account device */
	LinphoneAccountManagerServicesRequestTypeSendAccountRecoveryTokenByPush =
	    10, /**< Asks the account manager to send us an account recovery token by push notification */

	LinphoneAccountManagerServicesRequestTypeGetCreationTokenAsAdmin = 100,
	LinphoneAccountManagerServicesRequestTypeGetAccountInfoAsAdmin = 101,
	LinphoneAccountManagerServicesRequestTypeDeleteAccountAsAdmin = 102
} LinphoneAccountManagerServicesRequestType;

/*
 * WARNING: the LinphoneRecorderFileFormat enum must match the MSFileFormat enum defined in mediastreamer2.
 * See dangerous cast in src/recorder/recorder.cpp.
 */

/**
 * @brief Enum representing the file format of a recording.
 * @ingroup call_control
 **/
typedef enum _LinphoneMediaFileFormat {
	LinphoneMediaFileFormatUnknown,
	LinphoneMediaFileFormatWav, /** < WAVE file format, .wav file extension. */
	LinphoneMediaFileFormatMkv, /** < Standard Matroska file format, supports video, .mkv or .mka file extension. */
	LinphoneMediaFileFormatSmff /** < Simple Multimedia File Format, a proprietary format that supports video, .smff
	                                  file extension. */
} LinphoneMediaFileFormat;

#endif
