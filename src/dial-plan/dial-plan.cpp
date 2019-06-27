/*
 * dial-plan.cpp
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

#include <cstring>

#include "linphone/utils/utils.h"

#include "dial-plan.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

/*
 * http://en.wikipedia.org/wiki/Telephone_numbering_plan
 * http://en.wikipedia.org/wiki/Telephone_numbers_in_Europe
 * imported from https://en.wikipedia.org/wiki/List_of_mobile_phone_number_series_by_country
 */
const list<shared_ptr<DialPlan>> DialPlan::DialPlans = {
	// Country, iso country code, e164 country calling code, number length, international usual prefix
	bellesip::make_shared<DialPlan>("Afghanistan", "AF", "93", 9, "00"),
	bellesip::make_shared<DialPlan>("Albania", "AL", "355", 9, "00"),
	bellesip::make_shared<DialPlan>("Algeria", "DZ", "213", 9, "00"),
	bellesip::make_shared<DialPlan>("American Samoa", "AS", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Andorra", "AD", "376", 6, "00"),
	bellesip::make_shared<DialPlan>("Angola", "AO", "244", 9, "00"),
	bellesip::make_shared<DialPlan>("Anguilla", "AI", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Antigua and Barbuda", "AG", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Argentina", "AR", "54", 10, "00"),
	bellesip::make_shared<DialPlan>("Armenia", "AM", "374", 8, "00"),
	bellesip::make_shared<DialPlan>("Aruba", "AW", "297", 7, "011"),
	bellesip::make_shared<DialPlan>("Australia", "AU", "61", 9, "0011"),
	bellesip::make_shared<DialPlan>("Austria", "AT", "43", 10, "00"),
	bellesip::make_shared<DialPlan>("Azerbaijan", "AZ", "994", 9, "00"),
	bellesip::make_shared<DialPlan>("Bahamas", "BS", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Bahrain", "BH", "973", 8, "00"),
	bellesip::make_shared<DialPlan>("Bangladesh", "BD", "880", 10, "00"),
	bellesip::make_shared<DialPlan>("Barbados", "BB", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Belarus", "BY", "375", 9, "00"),
	bellesip::make_shared<DialPlan>("Belgium", "BE", "32", 9, "00"),
	bellesip::make_shared<DialPlan>("Belize", "BZ", "501", 7, "00"),
	bellesip::make_shared<DialPlan>("Benin", "BJ", "229", 8, "00"),
	bellesip::make_shared<DialPlan>("Bermuda", "BM", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Bhutan", "BT", "975", 8, "00"),
	bellesip::make_shared<DialPlan>("Bolivia", "BO", "591", 8, "00"),
	bellesip::make_shared<DialPlan>("Bosnia and Herzegovina", "BA", "387", 8, "00"),
	bellesip::make_shared<DialPlan>("Botswana", "BW", "267", 8, "00"),
	bellesip::make_shared<DialPlan>("Brazil", "BR", "55", 11, "00"),
	bellesip::make_shared<DialPlan>("Brunei Darussalam", "BN", "673", 7, "00"),
	bellesip::make_shared<DialPlan>("Bulgaria", "BG", "359", 9, "00"),
	bellesip::make_shared<DialPlan>("Burkina Faso", "BF", "226", 8, "00"),
	bellesip::make_shared<DialPlan>("Burundi", "BI", "257", 8, "011"),
	bellesip::make_shared<DialPlan>("Cambodia", "KH", "855", 9, "00"),
	bellesip::make_shared<DialPlan>("Cameroon", "CM", "237", 9, "00"),
	bellesip::make_shared<DialPlan>("Canada", "CA", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Cape Verde", "CV", "238", 7, "00"),
	bellesip::make_shared<DialPlan>("Cayman Islands", "KY", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Central African Republic", "CF", "236", 8, "00"),
	bellesip::make_shared<DialPlan>("Chad", "TD", "235", 8, "00"),
	bellesip::make_shared<DialPlan>("Chile", "CL", "56", 9, "00"),
	bellesip::make_shared<DialPlan>("China", "CN", "86", 11, "00"),
	bellesip::make_shared<DialPlan>("Colombia", "CO", "57", 10, "00"),
	bellesip::make_shared<DialPlan>("Comoros", "KM", "269", 7, "00"),
	bellesip::make_shared<DialPlan>("Congo", "CG", "242", 9, "00"),
	bellesip::make_shared<DialPlan>("Congo Democratic Republic", "CD", "243", 9, "00"),
	bellesip::make_shared<DialPlan>("Cook Islands", "CK", "682", 5, "00"),
	bellesip::make_shared<DialPlan>("Costa Rica", "CR", "506", 8, "00"),
	bellesip::make_shared<DialPlan>("Cote d'Ivoire", "AD", "225", 8, "00"),
	bellesip::make_shared<DialPlan>("Croatia", "HR", "385", 9, "00"),
	bellesip::make_shared<DialPlan>("Cuba", "CU", "53", 8, "119"),
	bellesip::make_shared<DialPlan>("Cyprus", "CY", "357", 8, "00"),
	bellesip::make_shared<DialPlan>("Czech Republic", "CZ", "420", 9, "00"),
	bellesip::make_shared<DialPlan>("Denmark", "DK", "45", 8, "00"),
	bellesip::make_shared<DialPlan>("Djibouti", "DJ", "253", 8, "00"),
	bellesip::make_shared<DialPlan>("Dominica", "DM", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Dominican Republic", "DO", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Ecuador", "EC", "593", 9, "00"),
	bellesip::make_shared<DialPlan>("Egypt", "EG", "20", 10, "00"),
	bellesip::make_shared<DialPlan>("El Salvador", "SV", "503", 8, "00"),
	bellesip::make_shared<DialPlan>("Equatorial Guinea", "GQ", "240", 9, "00"),
	bellesip::make_shared<DialPlan>("Eritrea", "ER", "291", 7, "00"),
	bellesip::make_shared<DialPlan>("Estonia", "EE", "372", 8, "00"),
	bellesip::make_shared<DialPlan>("Ethiopia", "ET", "251", 9, "00"),
	bellesip::make_shared<DialPlan>("Falkland Islands", "FK", "500", 5, "00"),
	bellesip::make_shared<DialPlan>("Faroe Islands", "FO", "298", 6, "00"),
	bellesip::make_shared<DialPlan>("Fiji", "FJ", "679", 7, "00"),
	bellesip::make_shared<DialPlan>("Finland", "FI", "358", 9, "00"),
	bellesip::make_shared<DialPlan>("France", "FR", "33", 9, "00"),
	bellesip::make_shared<DialPlan>("French Guiana", "GF", "594", 9, "00"),
	bellesip::make_shared<DialPlan>("French Polynesia", "PF", "689", 6, "00"),
	bellesip::make_shared<DialPlan>("Gabon", "GA", "241", 8, "00"),
	bellesip::make_shared<DialPlan>("Gambia", "GM", "220", 7, "00"),
	bellesip::make_shared<DialPlan>("Georgia", "GE", "995", 9, "00"),
	bellesip::make_shared<DialPlan>("Germany", "DE", "49", 11, "00"),
	bellesip::make_shared<DialPlan>("Ghana", "GH", "233", 9, "00"),
	bellesip::make_shared<DialPlan>("Gibraltar", "GI", "350", 8, "00"),
	bellesip::make_shared<DialPlan>("Greece", "GR", "30", 10, "00"),
	bellesip::make_shared<DialPlan>("Greenland", "GL", "299", 6, "00"),
	bellesip::make_shared<DialPlan>("Grenada", "GD", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Guadeloupe", "GP", "590", 9, "00"),
	bellesip::make_shared<DialPlan>("Guam", "GU", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Guatemala", "GT", "502", 8, "00"),
	bellesip::make_shared<DialPlan>("Guinea", "GN", "224", 8, "00"),
	bellesip::make_shared<DialPlan>("Guinea-Bissau", "GW", "245", 7, "00"),
	bellesip::make_shared<DialPlan>("Guyana", "GY", "592", 7, "001"),
	bellesip::make_shared<DialPlan>("Haiti", "HT", "509", 8, "00"),
	bellesip::make_shared<DialPlan>("Honduras", "HN", "504", 8, "00"),
	bellesip::make_shared<DialPlan>("Hong Kong", "HK", "852", 8, "001"),
	bellesip::make_shared<DialPlan>("Hungary", "HU", "36", 9, "00"),
	bellesip::make_shared<DialPlan>("Iceland", "IS", "354", 9, "00"),
	bellesip::make_shared<DialPlan>("India", "IN", "91", 10, "00"),
	bellesip::make_shared<DialPlan>("Indonesia", "ID", "62", 12, "001"),
	bellesip::make_shared<DialPlan>("Iran", "IR", "98", 10, "00"),
	bellesip::make_shared<DialPlan>("Iraq", "IQ", "964", 10, "00"),
	bellesip::make_shared<DialPlan>("Ireland", "IE", "353", 9, "00"),
	bellesip::make_shared<DialPlan>("Israel", "IL", "972", 9, "00"),
	bellesip::make_shared<DialPlan>("Italy", "IT", "39", 10, "00"),
	//	{"Jersey"                       ,"JE"		, "44"      , 10	, "00"	},
	bellesip::make_shared<DialPlan>("Jamaica", "JM", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Japan", "JP", "81", 10, "010"),
	bellesip::make_shared<DialPlan>("Jordan", "JO", "962", 9, "00"),
	//bellesip::make_shared<DialPlan>("Kazakhstan", "KZ", "7", 10, "00"), rusia
	bellesip::make_shared<DialPlan>("Kenya", "KE", "254", 9, "000"),
	bellesip::make_shared<DialPlan>("Kiribati", "KI", "686", 5, "00"),
	bellesip::make_shared<DialPlan>("Korea, North", "KP", "850", 12, "99"),
	bellesip::make_shared<DialPlan>("Korea, South", "KR", "82", 12, "001"),
	bellesip::make_shared<DialPlan>("Kuwait", "KW", "965", 8, "00"),
	bellesip::make_shared<DialPlan>("Kyrgyzstan", "KG", "996", 9, "00"),
	bellesip::make_shared<DialPlan>("Laos", "LA", "856", 10, "00"),
	bellesip::make_shared<DialPlan>("Latvia", "LV", "371", 8, "00"),
	bellesip::make_shared<DialPlan>("Lebanon", "LB", "961", 8, "00"),
	bellesip::make_shared<DialPlan>("Lesotho", "LS", "266", 8, "00"),
	bellesip::make_shared<DialPlan>("Liberia", "LR", "231", 8, "00"),
	bellesip::make_shared<DialPlan>("Libya", "LY", "218", 8, "00"),
	bellesip::make_shared<DialPlan>("Liechtenstein", "LI", "423", 7, "00"),
	bellesip::make_shared<DialPlan>("Lithuania", "LT", "370", 8, "00"),
	bellesip::make_shared<DialPlan>("Luxembourg", "LU", "352", 9, "00"),
	bellesip::make_shared<DialPlan>("Macau", "MO", "853", 8, "00"),
	bellesip::make_shared<DialPlan>("Macedonia", "MK", "389", 8, "00"),
	bellesip::make_shared<DialPlan>("Madagascar", "MG", "261", 9, "00"),
	bellesip::make_shared<DialPlan>("Malawi", "MW", "265", 9, "00"),
	bellesip::make_shared<DialPlan>("Malaysia", "MY", "60", 9, "00"),
	bellesip::make_shared<DialPlan>("Maldives", "MV", "960", 7, "00"),
	bellesip::make_shared<DialPlan>("Mali", "ML", "223", 8, "00"),
	bellesip::make_shared<DialPlan>("Malta", "MT", "356", 8, "00"),
	bellesip::make_shared<DialPlan>("Marshall Islands", "MH", "692", 7, "011"),
	bellesip::make_shared<DialPlan>("Martinique", "MQ", "596", 9, "00"),
	bellesip::make_shared<DialPlan>("Mauritania", "MR", "222", 8, "00"),
	bellesip::make_shared<DialPlan>("Mauritius", "MU", "230", 7, "00"),
	//bellesip::make_shared<DialPlan>("Mayotte Island", "YT", "262", 9, "00"), réunion
	bellesip::make_shared<DialPlan>("Mexico", "MX", "52", 11, "00"),
	//The following is a pseudo dial plan for Mexican mobile phones. See https://en.wikipedia.org/wiki/Telephone_numbers_in_Mexico
	//bellesip::make_shared<DialPlan>("Mexico", "MX", "521", 10, "00"),
	bellesip::make_shared<DialPlan>("Micronesia", "FM", "691", 7, "011"),
	bellesip::make_shared<DialPlan>("Moldova", "MD", "373", 8, "00"),
	bellesip::make_shared<DialPlan>("Monaco", "MC", "377", 8, "00"),
	bellesip::make_shared<DialPlan>("Mongolia", "MN", "976", 8, "001"),
	bellesip::make_shared<DialPlan>("Montenegro", "ME", "382", 8, "00"),
	bellesip::make_shared<DialPlan>("Montserrat", "MS", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Morocco", "MA", "212", 9, "00"),
	bellesip::make_shared<DialPlan>("Mozambique", "MZ", "258", 9, "00"),
	bellesip::make_shared<DialPlan>("Myanmar", "MM", "95", 10, "00"),
	bellesip::make_shared<DialPlan>("Namibia", "NA", "264", 9, "00"),
	bellesip::make_shared<DialPlan>("Nauru", "NR", "674", 7, "00"),
	bellesip::make_shared<DialPlan>("Nepal", "NP", "977", 8, "00"),
	bellesip::make_shared<DialPlan>("Netherlands", "NL", "31", 9, "00"),
	bellesip::make_shared<DialPlan>("New Caledonia", "NC", "687", 6, "00"),
	bellesip::make_shared<DialPlan>("New Zealand", "NZ", "64", 8, "00"),
	bellesip::make_shared<DialPlan>("Nicaragua", "NI", "505", 8, "00"),
	bellesip::make_shared<DialPlan>("Niger", "NE", "227", 8, "00"),
	bellesip::make_shared<DialPlan>("Nigeria", "NG", "234", 10, "009"),
	bellesip::make_shared<DialPlan>("Niue", "NU", "683", 4, "00"),
	bellesip::make_shared<DialPlan>("Norfolk Island", "NF", "672", 5, "00"),
	bellesip::make_shared<DialPlan>("Northern Mariana Islands", "MP", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Norway", "NO", "47", 8, "00"),
	bellesip::make_shared<DialPlan>("Oman", "OM", "968", 8, "00"),
	bellesip::make_shared<DialPlan>("Pakistan", "PK", "92", 10, "00"),
	bellesip::make_shared<DialPlan>("Palau", "PW", "680", 7, "011"),
	bellesip::make_shared<DialPlan>("Palestine", "PS", "970", 9, "00"),
	bellesip::make_shared<DialPlan>("Panama", "PA", "507", 8, "00"),
	bellesip::make_shared<DialPlan>("Papua New Guinea", "PG", "675", 8, "00"),
	bellesip::make_shared<DialPlan>("Paraguay", "PY", "595", 9, "00"),
	bellesip::make_shared<DialPlan>("Peru", "PE", "51", 9, "00"),
	bellesip::make_shared<DialPlan>("Philippines", "PH", "63", 10, "00"),
	bellesip::make_shared<DialPlan>("Poland", "PL", "48", 9, "00"),
	bellesip::make_shared<DialPlan>("Portugal", "PT", "351", 9, "00"),
	bellesip::make_shared<DialPlan>("Puerto Rico", "PR", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Qatar", "QA", "974", 8, "00"),
	bellesip::make_shared<DialPlan>("R�union Island", "RE", "262", 9, "011"),
	bellesip::make_shared<DialPlan>("Romania", "RO", "40", 9, "00"),
	bellesip::make_shared<DialPlan>("Russian Federation", "RU", "7", 10, "8"),
	bellesip::make_shared<DialPlan>("Rwanda", "RW", "250", 9, "00"),
	bellesip::make_shared<DialPlan>("Saint Helena", "SH", "290", 4, "00"),
	bellesip::make_shared<DialPlan>("Saint Kitts and Nevis", "KN", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Saint Lucia", "LC", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Saint Pierre and Miquelon", "PM", "508", 6, "00"),
	bellesip::make_shared<DialPlan>("Saint Vincent and the Grenadines", "VC", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Samoa", "WS", "685", 7, "0"),
	bellesip::make_shared<DialPlan>("San Marino", "SM", "378", 10, "00"),
	bellesip::make_shared<DialPlan>("Sao Tome and Principe", "ST", "239", 7, "00"),
	bellesip::make_shared<DialPlan>("Saudi Arabia", "SA", "966", 9, "00"),
	bellesip::make_shared<DialPlan>("Senegal", "SN", "221", 9, "00"),
	bellesip::make_shared<DialPlan>("Serbia", "RS", "381", 9, "00"),
	bellesip::make_shared<DialPlan>("Seychelles", "SC", "248", 7, "00"),
	bellesip::make_shared<DialPlan>("Sierra Leone", "SL", "232", 8, "00"),
	bellesip::make_shared<DialPlan>("Singapore", "SG", "65", 8, "001"),
	bellesip::make_shared<DialPlan>("Slovakia", "SK", "421", 9, "00"),
	bellesip::make_shared<DialPlan>("Slovenia", "SI", "386", 8, "00"),
	bellesip::make_shared<DialPlan>("Solomon Islands", "SB", "677", 7, "00"),
	bellesip::make_shared<DialPlan>("Somalia", "SO", "252", 8, "00"),
	bellesip::make_shared<DialPlan>("South Africa", "ZA", "27", 9, "00"),
	bellesip::make_shared<DialPlan>("Spain", "ES", "34", 9, "00"),
	bellesip::make_shared<DialPlan>("Sri Lanka", "LK", "94", 9, "00"),
	bellesip::make_shared<DialPlan>("Sudan", "SD", "249", 9, "00"),
	bellesip::make_shared<DialPlan>("Suriname", "SR", "597", 7, "00"),
	bellesip::make_shared<DialPlan>("Swaziland", "SZ", "268", 8, "00"),
	bellesip::make_shared<DialPlan>("Sweden", "SE", "46", 9, "00"),
	bellesip::make_shared<DialPlan>("Switzerland", "XK", "41", 9, "00"),
	bellesip::make_shared<DialPlan>("Syria", "SY", "963", 9, "00"),
	bellesip::make_shared<DialPlan>("Taiwan", "TW", "886", 9, "810"),
	bellesip::make_shared<DialPlan>("Tajikistan", "TJ", "992", 9, "002"),
	bellesip::make_shared<DialPlan>("Tanzania", "TZ", "255", 9, "000"),
	bellesip::make_shared<DialPlan>("Thailand", "TH", "66", 9, "001"),
	bellesip::make_shared<DialPlan>("Togo", "TG", "228", 8, "00"),
	bellesip::make_shared<DialPlan>("Tokelau", "TK", "690", 4, "00"),
	bellesip::make_shared<DialPlan>("Tonga", "TO", "676", 5, "00"),
	bellesip::make_shared<DialPlan>("Trinidad and Tobago", "TT", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Tunisia", "TN", "216", 8, "00"),
	bellesip::make_shared<DialPlan>("Turkey", "TR", "90", 10, "00"),
	bellesip::make_shared<DialPlan>("Turkmenistan", "TM", "993", 8, "00"),
	bellesip::make_shared<DialPlan>("Turks and Caicos Islands", "TC", "1", 7, "0"),
	bellesip::make_shared<DialPlan>("Tuvalu", "TV", "688", 5, "00"),
	bellesip::make_shared<DialPlan>("Uganda", "UG", "256", 9, "000"),
	bellesip::make_shared<DialPlan>("Ukraine", "UA", "380", 9, "00"),
	bellesip::make_shared<DialPlan>("United Arab Emirates", "AE", "971", 9, "00"),
	bellesip::make_shared<DialPlan>("United Kingdom", "GB", "44", 10, "00"),
	//	{"United Kingdom"               ,"UK"		, "44"      , 10	, "00"	},
	bellesip::make_shared<DialPlan>("United States", "US", "1", 10, "011"),
	bellesip::make_shared<DialPlan>("Uruguay", "UY", "598", 8, "00"),
	bellesip::make_shared<DialPlan>("Uzbekistan", "UZ", "998", 9, "8"),
	bellesip::make_shared<DialPlan>("Vanuatu", "VU", "678", 7, "00"),
	bellesip::make_shared<DialPlan>("Venezuela", "VE", "58", 10, "00"),
	bellesip::make_shared<DialPlan>("Vietnam", "VN", "84", 9, "00"),
	bellesip::make_shared<DialPlan>("Wallis and Futuna", "WF", "681", 5, "00"),
	bellesip::make_shared<DialPlan>("Yemen", "YE", "967", 9, "00"),
	bellesip::make_shared<DialPlan>("Zambia", "ZM", "260", 9, "00"),
	bellesip::make_shared<DialPlan>("Zimbabwe", "ZW", "263", 9, "00")
};

const shared_ptr<DialPlan> DialPlan::MostCommon = bellesip::make_shared<DialPlan>("generic", "", "", 10, "00");

DialPlan::DialPlan (
	const string &country,
	const string &isoCountryCode,
	const string &ccc,
	int nnl,
	const string &icp
) : country(country), isoCountryCode(isoCountryCode), countryCallingCode(ccc), nationalNumberLength(nnl), internationalCallPrefix(icp) { }

DialPlan::DialPlan (const DialPlan &other) : HybridObject(other) {
	country = other.getCountry();
	isoCountryCode = other.getIsoCountryCode();
	countryCallingCode = other.getCountryCallingCode();
	nationalNumberLength = other.getNationalNumberLength();
	internationalCallPrefix = other.getInternationalCallPrefix();
}

DialPlan &DialPlan::operator= (const DialPlan &other) {
	if (this != &other) {
		country = other.getCountry();
		isoCountryCode = other.getIsoCountryCode();
		countryCallingCode = other.getCountryCallingCode();
		nationalNumberLength = other.getNationalNumberLength();
		internationalCallPrefix = other.getInternationalCallPrefix();
	}

	return *this;
}

const string &DialPlan::getCountry () const {
	return country;
}

const string &DialPlan::getIsoCountryCode () const {
	return isoCountryCode;
}

const string &DialPlan::getCountryCallingCode () const {
	return countryCallingCode;
}

void DialPlan::setCountryCallingCode(const std::string &ccc) {
	countryCallingCode = ccc;
}

int DialPlan::getNationalNumberLength () const {
	return nationalNumberLength;
}

const string &DialPlan::getInternationalCallPrefix () const {
	return internationalCallPrefix;
}

bool DialPlan::isGeneric () const {
	return country == MostCommon->getCountry();
}

int DialPlan::lookupCccFromE164 (const string &e164) {
	if (e164[0] != '+')
		return -1; // Not an e164 number.

	// USA case.
	if (e164[1] == '1')
		return 1;

	shared_ptr<DialPlan> electedDialPlan;
	unsigned int found;
	unsigned int i = 0;

	do {
		found = 0;
		i++;
		for (const auto &dp : DialPlans) {
			if (strncmp(dp->getCountryCallingCode().c_str(), &e164[1], i) == 0) {
				electedDialPlan = dp;
				found++;
			}
		}
	} while ((found > 1 || found == 0) && i < e164.length() - 1);

	if (found == 1)
		return Utils::stoi(electedDialPlan->getCountryCallingCode());

	return -1;
}

int DialPlan::lookupCccFromIso (const string &iso) {
	for (const auto &dp : DialPlans) {
		if (dp->getIsoCountryCode() == iso)
			return Utils::stoi(dp->getCountryCallingCode());
	}

	return -1;
}

shared_ptr<DialPlan> DialPlan::findByCcc (int ccc) {
	return DialPlan::findByCcc(Utils::toString(ccc));
}

shared_ptr<DialPlan> DialPlan::findByCcc (const string &ccc) {
	if (ccc.empty())
		return MostCommon;

	for (const auto &dp : DialPlans) {
		if (dp->getCountryCallingCode() == ccc)
			return dp;
	}

	// Return a generic "most common" dial plan.
	return MostCommon;
}

const list<shared_ptr<DialPlan>> &DialPlan::getAllDialPlans () {
	return DialPlans;
}

LINPHONE_END_NAMESPACE
