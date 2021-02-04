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

#include "address/address.h"
#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/basic-chat-room.h"
#include "content/content-type.h"
#include "content/content.h"
#include "content/file-content.h"
#include "core/core.h"

// TODO: Remove me later.
#include "private.h"

#include "liblinphone_tester.h"
#include "tester_utils.h"

#include <vector>
#include <map>
// =============================================================================

using namespace std;

using namespace LinphonePrivate;
class FmtpManager{
public:
	FmtpManager(){}
	void setFmtp(const std::string& key, const std::string& value){
		mFmtp[key] = value;
	}
	std::string toString(){
		return toString(mFmtp);
	}
	std::string toString(std::map<std::string, std::string> data){
		std::string merge;
		for(auto i : data){
			if(merge != "")
				merge += ';';
			if( i.second != "")
				merge += i.first+"="+i.second;
			else
				merge += i.first;
		}
		return merge;
	}
	std::map<std::string, std::string> mFmtp;
};

#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(__ANDROID__)
static void completion_cb(void *user_data, int percentage){
	fprintf(stdout,"%i %% completed\r",percentage);
	fflush(stdout);
}
#endif

static void audio_call_stereo_call(const char *codec_name, int clock_rate, int bitrate_override, bool_t stereo) {
	LinphoneCoreManager* marie;
	LinphoneCoreManager* pauline;
	PayloadType *pt;
	char *stereo_file = bc_tester_res("sounds/vrroom.wav");
	char *recordpath = bc_tester_file("stereo-record.wav");
	bool_t audio_cmp_failed = FALSE;

	unlink(recordpath);

	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	/*make sure we have opus*/
	pt = linphone_core_find_payload_type(marie->lc, codec_name, clock_rate, 2);
	if (!pt) {
		ms_warning("%s not available, stereo with %s not tested.",codec_name, codec_name);
		goto end;
	}
	if (stereo) payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	if (bitrate_override) linphone_core_set_payload_type_bitrate(marie->lc, pt, bitrate_override);
	pt = linphone_core_find_payload_type(pauline->lc, codec_name, clock_rate, 2);
	if (stereo) payload_type_set_recv_fmtp(pt, "stereo=1;sprop-stereo=1");
	if (bitrate_override) linphone_core_set_payload_type_bitrate(pauline->lc, pt, bitrate_override);

	disable_all_audio_codecs_except_one(marie->lc, codec_name, clock_rate);
	disable_all_audio_codecs_except_one(pauline->lc, codec_name, clock_rate);

	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, stereo_file);
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_record_file(pauline->lc, recordpath);

	/*stereo is supported only without volume control, echo canceller...*/
	linphone_config_set_string(linphone_core_get_config(marie->lc),"sound","features","REMOTE_PLAYING");
	linphone_config_set_string(linphone_core_get_config(pauline->lc),"sound","features","REMOTE_PLAYING");

	if (!BC_ASSERT_TRUE(call(pauline,marie))) goto end;
	wait_for_until(marie->lc, pauline->lc, NULL, 0, 6000);
	end_call(pauline, marie);


	if (clock_rate!=48000) {
		ms_warning("Similarity checking not implemented for files not having the same sampling rate");
	}else{
#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(__ANDROID__)
		double similar;
		double min_threshold = .75f; /*should be above 0.8 in best conditions*/
		double max_threshold = 1.f;
		if (!stereo){
			/*when opus doesn't transmit stereo, the cross correlation is around 0.6 : as expected, it is not as good as in full stereo mode*/
			min_threshold = .4f;
			max_threshold = .68f;
		}
		BC_ASSERT_EQUAL(ms_audio_diff(stereo_file, recordpath,&similar,&audio_cmp_params,completion_cb,NULL), 0, int, "%d");
		BC_ASSERT_GREATER(similar, min_threshold, double, "%g");
		BC_ASSERT_LOWER(similar, max_threshold, double, "%g");
		if (similar<min_threshold || similar>max_threshold){
			audio_cmp_failed = TRUE;
		}
#endif
	}
	if (!audio_cmp_failed) unlink(recordpath);
end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	ms_free(stereo_file);
	bc_free(recordpath);
}

static void audio_stereo_call_l16(void){
	audio_call_stereo_call("L16", 44100, 0, TRUE);
}

static void audio_stereo_call_opus(void){
	audio_call_stereo_call("opus", 48000, 150, TRUE);
}

static void audio_mono_call_opus(void){
	/*actually a call where input/output is made with stereo but opus transmits everything as mono*/
	audio_call_stereo_call("opus", 48000, 150, FALSE);
}

static void audio_call_loss_resilience(const char *codec_name, int clock_rate, int bitrate_override, int jitterBufferMs, bool_t stereo, std::pair<double,double> threshold ) {
#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(__ANDROID__)
	LinphoneCoreManager *marie = nullptr, *pauline = nullptr;
	char *recordPath = nullptr;
	char *referenceFile = bc_tester_res("sounds/vrroom.wav");

	OrtpNetworkSimulatorParams simparams = { 0 };
	PayloadType *mariePt, *paulinePt;
	int sampleLength = 6000;
	std::string recordFileNameRoot = "loss-record.wav", recordFileName, refRecordFileName;
	FmtpManager marieFmtp, paulineFmtp;
	std::vector<std::string> useinbandfec = {"1"};
	std::vector<float> lossRates = {50};
	std::vector<std::string> packetLossPercentage = {"50","100"};
	MSAudioDiffParams audioCmpParams = audio_cmp_params;

	// Add jitterBufferMs to the shift
        audioCmpParams.max_shift_percent = static_cast<int>(std::floor((audioCmpParams.max_shift_percent*sampleLength+jitterBufferMs)/((double)sampleLength+jitterBufferMs)));
	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	
	if (!BC_ASSERT_PTR_NOT_NULL(marie) || !BC_ASSERT_PTR_NOT_NULL(pauline)) goto end;
	
	/*make sure we have opus*/
	mariePt = linphone_core_find_payload_type(marie->lc, codec_name, clock_rate, 2);
	paulinePt = linphone_core_find_payload_type(pauline->lc, codec_name, clock_rate, 2);
	if (!BC_ASSERT_PTR_NOT_NULL(mariePt) || !BC_ASSERT_PTR_NOT_NULL(paulinePt)) {
		ms_warning("%s not available, stereo with %s and fec not tested.",codec_name, codec_name);
		goto end;
	}
	
	if (stereo){
		marieFmtp.setFmtp("stereo", "1");
		marieFmtp.setFmtp("sprop-stereo", "1");
	}
	disable_all_audio_codecs_except_one(marie->lc, codec_name, clock_rate);
	disable_all_audio_codecs_except_one(pauline->lc, codec_name, clock_rate);
	if(bitrate_override) linphone_core_set_payload_type_bitrate(marie->lc, mariePt, bitrate_override);
	if(bitrate_override) linphone_core_set_payload_type_bitrate(pauline->lc, paulinePt, bitrate_override);
	linphone_core_set_use_files(marie->lc, TRUE);
	linphone_core_set_play_file(marie->lc, referenceFile);
	linphone_core_set_use_files(pauline->lc, TRUE);
	linphone_core_set_record_file(pauline->lc, recordPath);
	/*stereo is supported only without volume control, echo canceller...*/
	linphone_config_set_string(linphone_core_get_config(marie->lc),"sound","features","REMOTE_PLAYING");
	linphone_config_set_string(linphone_core_get_config(pauline->lc),"sound","features","REMOTE_PLAYING");
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "jitter_buffer_min_size", jitterBufferMs);
	linphone_core_set_audio_jittcomp(pauline->lc, jitterBufferMs);
	linphone_core_enable_adaptive_rate_control(marie->lc, FALSE); // We don't want adaptive rate control here, in order to not interfere with loss recovery algorithms

	simparams.mode = OrtpNetworkSimulatorOutbound;
	simparams.enabled = TRUE;
        simparams.consecutive_loss_probability = 0.000001f;// Ensure to have fec in n+1 packets
	for(size_t inbandIndex = 0 ; inbandIndex < useinbandfec.size() ; ++inbandIndex){// Loop to test the impact of useinbandfec
		marieFmtp.setFmtp("useinbandfec", useinbandfec[inbandIndex]);
		for(size_t lossRateIndex = 0 ; lossRateIndex < lossRates.size() ; ++lossRateIndex){// Loop to test the impact of loosing packets
			double similarityRef = 0.0, similarity=0.0;
			bool_t noError = TRUE;
			simparams.loss_rate = lossRates[lossRateIndex];
			linphone_core_set_network_simulator_params(marie->lc, &simparams);
			
// Compute similarity reference
			marieFmtp.setFmtp("packetlosspercentage", "0");
			paulineFmtp = marieFmtp;
			payload_type_set_recv_fmtp(mariePt, marieFmtp.toString().c_str());
			payload_type_set_recv_fmtp(paulinePt, paulineFmtp.toString().c_str());
			refRecordFileName= useinbandfec[inbandIndex]+"_"+std::to_string(lossRates[lossRateIndex])+"_ref_"+recordFileNameRoot;
			bc_free(recordPath);
			recordPath = bc_tester_file(refRecordFileName.c_str());
			linphone_core_set_record_file(pauline->lc, recordPath);
			unlink(recordPath);
			if (BC_ASSERT_TRUE(call(pauline,marie))){
				wait_for_until(marie->lc, pauline->lc, NULL, 0, sampleLength+jitterBufferMs);
				end_call(pauline, marie);
				BC_ASSERT_EQUAL(ms_audio_diff(referenceFile, recordPath,&similarityRef,&audioCmpParams,NULL,NULL), 0, int, "%d");
			}
			for(size_t packetLossIndex = 0 ; packetLossIndex < packetLossPercentage.size() ; ++packetLossIndex){
// Set packetloss. Similarity must be greater than having no packetloss
				marieFmtp.setFmtp("packetlosspercentage", packetLossPercentage[packetLossIndex]);
				paulineFmtp = marieFmtp;
				payload_type_set_recv_fmtp(mariePt, marieFmtp.toString().c_str());
				payload_type_set_recv_fmtp(paulinePt, paulineFmtp.toString().c_str());
				recordFileName= useinbandfec[inbandIndex]+"_"+std::to_string(lossRates[lossRateIndex])+"_"+packetLossPercentage[packetLossIndex]+"_out_"+recordFileNameRoot;
				bc_free(recordPath);
				recordPath = bc_tester_file(recordFileName.c_str());
				linphone_core_set_record_file(pauline->lc, recordPath);
				unlink(recordPath);
				if (BC_ASSERT_TRUE(call(pauline,marie))){
					wait_for_until(marie->lc, pauline->lc, NULL, 0, sampleLength+jitterBufferMs);
					end_call(pauline, marie);
					BC_ASSERT_EQUAL(ms_audio_diff(referenceFile, recordPath,&similarity,&audioCmpParams,NULL,NULL), 0, int, "%d");
				}
				BC_ASSERT_GREATER(similarity, similarityRef, double, "%g");
				if(similarity>=similarityRef){
					unlink(refRecordFileName.c_str());
				}else
					noError = FALSE;
				similarityRef = similarity;
			}
			if(noError){
				unlink(recordFileName.c_str());
			}
		}
	}

end:

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	bc_free(recordPath);
	bc_free(referenceFile);
#endif
}

static void audio_call_loss_resilience_opus(){
	audio_call_loss_resilience("opus", 48000, 120, 300, TRUE, std::pair<double,double>(0.8,1.0));
}

static void audio_call_test_diff() {
	char *stereo_file = bc_tester_res("sounds/vrroom.wav");
	const char *recordPath = "1_50_0_0.5915.wav";
	std::vector<double> values;
	
	for(int i = 0 ; i < 20 ; ++i) {
		double similar = 1.0;
		ms_audio_diff(stereo_file, recordPath,&similar,&audio_cmp_params,NULL,NULL);
		values.push_back(similar);
	}
	for(size_t i = 0 ; i < values.size() ; ++i)
		printf("%f", values[i]);
}


static void audio_call_test_audio_diff() {
	char *stereo_file = bc_tester_res("sounds/vrroom.wav");
	std::vector<double> values;
	
	for(int i = 0 ; i < 20 ; ++i) {
		double similar = 1.0;
		ms_audio_diff(stereo_file, stereo_file,&similar,&audio_cmp_params,NULL,NULL);
		values.push_back(similar);
	}
	for(size_t i = 0 ; i < values.size() ; ++i)
		printf("%f", values[i]);
		
	bc_free(stereo_file);
}

test_t audio_quality_tests[] = {
	TEST_NO_TAG("Audio loss rate resilience opus", audio_call_loss_resilience_opus),
	TEST_NO_TAG("Simple stereo call with L16", audio_stereo_call_l16),
	TEST_NO_TAG("Simple stereo call with opus", audio_stereo_call_opus),
	TEST_NO_TAG("Simple mono call with opus", audio_mono_call_opus),
	TEST_NO_TAG("Audio test diff", audio_call_test_diff),
	TEST_NO_TAG("Audio test audio diff", audio_call_test_audio_diff),
};

test_suite_t 	audio_quality_test_suite = {
	"Audio Call quality", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(audio_quality_tests) / sizeof(audio_quality_tests[0]), audio_quality_tests
};
