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

double getMinThreshold(const int &useinbandfec, const double &packetLossPercentage, const double &lossRates, const bool &stereo){
	double minT = (stereo?0.9:0.4);// When opus transmit mono, the cross correlation is around 0.6.
	if( lossRates > 0){// minT = (full received packets precision) + (lost packet precision).
		double normalPrecision = minT;
		double lostPrecision = (minT*packetLossPercentage/100.0);
		double completePackets = 100.0/lossRates;
		double lostPackets = 100.0/(100.0-lossRates);
		minT = normalPrecision/completePackets + lostPrecision/lostPackets;
	}
	return minT;
}

static void audio_call_loss_resilience() {
	bool debug = true;
	const char *codec_name = "opus";
	int clock_rate = 48000;
	int bitrate_override = 120;
	int maxRetryCount = 2;// We need to retry the call as the delta of ms_audio_diff can be high (+-0.3)
	LinphoneCoreManager *marie, *pauline;
	PayloadType *mariePt, *paulinePt;
	char *stereo_file = bc_tester_res("sounds/vrroom.wav");
	char *referenceFile = nullptr;
	char *recordPath = nullptr;
	std::string recordFileName;
	FmtpManager marieFmtp, paulineFmtp;
	std::vector<std::string> values;
	std::vector<std::string> useinbandfec = {"0"};
	std::vector<float> lossRates = {0,50};
	std::vector<std::string> packetLossPercentage = {"0","50","100"};
	std::vector<double> minThresholds;
	OrtpNetworkSimulatorParams simparams = { 0 };
	int useRef = true;	// Use a file that is already used to remove encode/decode noise for ms_audio_diff (preference for a loss rate to 0)
	int jitterBufferMs = 300;
	MSAudioDiffParams audioCmpParams = audio_cmp_params;
	int sampleLength = 6000;
	// Add jitterBufferMs to the shift
	audioCmpParams.max_shift_percent = (audioCmpParams.max_shift_percent*sampleLength+jitterBufferMs)/((double)sampleLength+jitterBufferMs);
		
	marie = linphone_core_manager_new( "marie_rc");
	pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	
	/*make sure we have opus*/
	mariePt = linphone_core_find_payload_type(marie->lc, codec_name, clock_rate, 2);
	paulinePt = linphone_core_find_payload_type(pauline->lc, codec_name, clock_rate, 2);
	if (!mariePt || !paulinePt) {
		ms_warning("%s not available, stereo with %s and fec not tested.",codec_name, codec_name);
	}else{
		marieFmtp.setFmtp("stereo", "1");
		marieFmtp.setFmtp("sprop-stereo", "1");
		
		disable_all_audio_codecs_except_one(marie->lc, codec_name, clock_rate);
		disable_all_audio_codecs_except_one(pauline->lc, codec_name, clock_rate);
		linphone_core_set_payload_type_bitrate(marie->lc, mariePt, bitrate_override);
		linphone_core_set_payload_type_bitrate(pauline->lc, paulinePt, bitrate_override);
		linphone_core_set_use_files(marie->lc, TRUE);
		linphone_core_set_play_file(marie->lc, stereo_file);
		linphone_core_set_use_files(pauline->lc, TRUE);
		/*stereo is supported only without volume control, echo canceller...*/
		linphone_config_set_string(linphone_core_get_config(marie->lc),"sound","features","REMOTE_PLAYING");
		linphone_config_set_string(linphone_core_get_config(pauline->lc),"sound","features","REMOTE_PLAYING");
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "jitter_buffer_min_size", jitterBufferMs);
		linphone_core_set_audio_jittcomp(pauline->lc, jitterBufferMs);
		linphone_core_enable_adaptive_rate_control(marie->lc, FALSE); // We don't want adaptive rate control here, in order to not interfere with loss recovery algorithms
		
		simparams.mode = OrtpNetworkSimulatorOutbound;
		simparams.enabled = TRUE;
		simparams.consecutive_loss_probability = 0.000001;// Ensure to have fec in n+1 packets

#if !defined(__arm__) && !defined(__arm64__) && !TARGET_IPHONE_SIMULATOR && !defined(__ANDROID__)
		for(size_t inbandIndex = 0 ; inbandIndex < useinbandfec.size() ; ++inbandIndex){// Loop to test the impact of useinbandfec
			marieFmtp.setFmtp("useinbandfec", useinbandfec[inbandIndex]);
			for(size_t packetLossIndex = 0 ; packetLossIndex < packetLossPercentage.size() ; ++packetLossIndex){// Loop to test the impact of packetlosspercentage
				marieFmtp.setFmtp("packetlosspercentage", packetLossPercentage[packetLossIndex]);
				paulineFmtp = marieFmtp;
				for(size_t lossRateIndex = 0 ; lossRateIndex < lossRates.size() ; ++lossRateIndex){// Loop to test the impact of loosing packets
					bool setRef = useRef &&  lossRateIndex == 0 && packetLossIndex==0;
					double minThreshold = getMinThreshold(std::stoi(useinbandfec[inbandIndex],nullptr,10)
						, std::stod(packetLossPercentage[packetLossIndex])
						, static_cast<double>(lossRates[lossRateIndex])
						, true);
					double similar = 0.0;
					int retryCount = 0;
					simparams.loss_rate = lossRates[lossRateIndex];
					linphone_core_set_network_simulator_params(marie->lc, &simparams);
					payload_type_set_recv_fmtp(mariePt, marieFmtp.toString().c_str());
					payload_type_set_recv_fmtp(paulinePt, paulineFmtp.toString().c_str());
					
					recordFileName = useinbandfec[inbandIndex]+"_"+packetLossPercentage[packetLossIndex]+"_"+std::to_string(lossRates[lossRateIndex])+"_record.wav";
					if(recordPath)
						bc_free(recordPath);
					recordPath = bc_tester_file(recordFileName.c_str());
					linphone_core_set_record_file(pauline->lc, recordPath);
					if(setRef || (!useRef && !referenceFile)){
						if(referenceFile)
							bc_free(referenceFile);
						referenceFile = bc_tester_res("sounds/vrroom.wav");
					}
					do{
						unlink(recordPath);
						if (BC_ASSERT_TRUE(call(pauline,marie))){
								wait_for_until(marie->lc, pauline->lc, NULL, 0, sampleLength+jitterBufferMs);
								end_call(pauline, marie);
								BC_ASSERT_EQUAL(ms_audio_diff(referenceFile, recordPath,&similar,&audioCmpParams,NULL,NULL), 0, int, "%d");
							}
							++retryCount;
					}while(retryCount < maxRetryCount && similar < minThreshold);
					if(debug)
						values.push_back(useinbandfec[inbandIndex]+"/"+packetLossPercentage[packetLossIndex]+"/"+std::to_string(lossRates[lossRateIndex])+"="+std::to_string(similar)+"/"+std::to_string(minThreshold)+" R="+std::to_string(retryCount-1)+(setRef?" ref":""));
					BC_ASSERT_GREATER(similar, minThreshold, double, "%g");
					if(setRef ){
						bc_free(referenceFile);
						referenceFile = bc_tester_file(recordFileName.c_str());
					}else
						unlink(recordPath);
#endif
				}
			}
		}
	}
	for(size_t i = 0 ; i < values.size() ; ++i)
		ms_message(values[i].c_str());
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	if(useRef)
		unlink(referenceFile);
	ms_free(stereo_file);
	bc_free(recordPath);
	bc_free(referenceFile);
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

test_t audio_quality_tests[] = {
	TEST_NO_TAG("Audio loss rate resilience", audio_call_loss_resilience),
	TEST_NO_TAG("Audio test diff", audio_call_test_diff),
};

test_suite_t 	audio_quality_test_suite = {
	"Audio Call quality", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(audio_quality_tests) / sizeof(audio_quality_tests[0]), audio_quality_tests
};
