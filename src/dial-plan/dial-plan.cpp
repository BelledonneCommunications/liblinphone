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
	DialPlan::create("Afghanistan", "AF", "93", 9, "00"),
	DialPlan::create("Albania", "AL", "355", 9, "00"),
	DialPlan::create("Algeria", "DZ", "213", 9, "00"),
	DialPlan::create("American Samoa", "AS", "1", 10, "011"),
	DialPlan::create("Andorra", "AD", "376", 6, "00"),
	DialPlan::create("Angola", "AO", "244", 9, "00"),
	DialPlan::create("Anguilla", "AI", "1", 10, "011"),
	DialPlan::create("Antigua and Barbuda", "AG", "1", 10, "011"),
	DialPlan::create("Argentina", "AR", "54", 10, "00"),
	DialPlan::create("Armenia", "AM", "374", 8, "00"),
	DialPlan::create("Aruba", "AW", "297", 7, "011"),
	DialPlan::create("Australia", "AU", "61", 9, "0011"),
	DialPlan::create("Austria", "AT", "43", 13, "00"),
	DialPlan::create("Azerbaijan", "AZ", "994", 9, "00"),
	DialPlan::create("Bahamas", "BS", "1", 10, "011"),
	DialPlan::create("Bahrain", "BH", "973", 8, "00"),
	DialPlan::create("Bangladesh", "BD", "880", 10, "00"),
	DialPlan::create("Barbados", "BB", "1", 10, "011"),
	DialPlan::create("Belarus", "BY", "375", 9, "00"),
	DialPlan::create("Belgium", "BE", "32", 9, "00"),
	DialPlan::create("Belize", "BZ", "501", 7, "00"),
	DialPlan::create("Benin", "BJ", "229", 8, "00"),
	DialPlan::create("Bermuda", "BM", "1", 10, "011"),
	DialPlan::create("Bhutan", "BT", "975", 8, "00"),
	DialPlan::create("Bolivia", "BO", "591", 8, "00"),
	DialPlan::create("Bosnia and Herzegovina", "BA", "387", 8, "00"),
	DialPlan::create("Botswana", "BW", "267", 8, "00"),
	DialPlan::create("Brazil", "BR", "55", 11, "00"),
	DialPlan::create("Brunei Darussalam", "BN", "673", 7, "00"),
	DialPlan::create("Bulgaria", "BG", "359", 9, "00"),
	DialPlan::create("Burkina Faso", "BF", "226", 8, "00"),
	DialPlan::create("Burundi", "BI", "257", 8, "011"),
	DialPlan::create("Cambodia", "KH", "855", 9, "00"),
	DialPlan::create("Cameroon", "CM", "237", 9, "00"),
	DialPlan::create("Canada", "CA", "1", 10, "011"),
	DialPlan::create("Cape Verde", "CV", "238", 7, "00"),
	DialPlan::create("Cayman Islands", "KY", "1", 10, "011"),
	DialPlan::create("Central African Republic", "CF", "236", 8, "00"),
	DialPlan::create("Chad", "TD", "235", 8, "00"),
	DialPlan::create("Chile", "CL", "56", 9, "00"),
	DialPlan::create("China", "CN", "86", 11, "00"),
	DialPlan::create("Colombia", "CO", "57", 10, "00"),
	DialPlan::create("Comoros", "KM", "269", 7, "00"),
	DialPlan::create("Congo", "CG", "242", 9, "00"),
	DialPlan::create("Congo Democratic Republic", "CD", "243", 9, "00"),
	DialPlan::create("Cook Islands", "CK", "682", 5, "00"),
	DialPlan::create("Costa Rica", "CR", "506", 8, "00"),
	DialPlan::create("Cote d'Ivoire", "AD", "225", 8, "00"),
	DialPlan::create("Croatia", "HR", "385", 9, "00"),
	DialPlan::create("Cuba", "CU", "53", 8, "119"),
	DialPlan::create("Cyprus", "CY", "357", 8, "00"),
	DialPlan::create("Czech Republic", "CZ", "420", 9, "00"),
	DialPlan::create("Denmark", "DK", "45", 8, "00"),
	DialPlan::create("Djibouti", "DJ", "253", 8, "00"),
	DialPlan::create("Dominica", "DM", "1", 10, "011"),
	DialPlan::create("Dominican Republic", "DO", "1", 10, "011"),
	DialPlan::create("Ecuador", "EC", "593", 9, "00"),
	DialPlan::create("Egypt", "EG", "20", 10, "00"),
	DialPlan::create("El Salvador", "SV", "503", 8, "00"),
	DialPlan::create("Equatorial Guinea", "GQ", "240", 9, "00"),
	DialPlan::create("Eritrea", "ER", "291", 7, "00"),
	DialPlan::create("Estonia", "EE", "372", 8, "00"),
	DialPlan::create("Ethiopia", "ET", "251", 9, "00"),
	DialPlan::create("Falkland Islands", "FK", "500", 5, "00"),
	DialPlan::create("Faroe Islands", "FO", "298", 6, "00"),
	DialPlan::create("Fiji", "FJ", "679", 7, "00"),
	DialPlan::create("Finland", "FI", "358", 9, "00"),
	DialPlan::create("France", "FR", "33", 9, "00"),
	DialPlan::create("French Guiana", "GF", "594", 9, "00"),
	DialPlan::create("French Polynesia", "PF", "689", 6, "00"),
	DialPlan::create("Gabon", "GA", "241", 8, "00"),
	DialPlan::create("Gambia", "GM", "220", 7, "00"),
	DialPlan::create("Georgia", "GE", "995", 9, "00"),
	DialPlan::create("Germany", "DE", "49", 12, "00"),
	DialPlan::create("Ghana", "GH", "233", 9, "00"),
	DialPlan::create("Gibraltar", "GI", "350", 8, "00"),
	DialPlan::create("Greece", "GR", "30", 10, "00"),
	DialPlan::create("Greenland", "GL", "299", 6, "00"),
	DialPlan::create("Grenada", "GD", "1", 10, "011"),
	DialPlan::create("Guadeloupe", "GP", "590", 9, "00"),
	DialPlan::create("Guam", "GU", "1", 10, "011"),
	DialPlan::create("Guatemala", "GT", "502", 8, "00"),
	DialPlan::create("Guinea", "GN", "224", 8, "00"),
	DialPlan::create("Guinea-Bissau", "GW", "245", 7, "00"),
	DialPlan::create("Guyana", "GY", "592", 7, "001"),
	DialPlan::create("Haiti", "HT", "509", 8, "00"),
	DialPlan::create("Honduras", "HN", "504", 8, "00"),
	DialPlan::create("Hong Kong", "HK", "852", 8, "001"),
	DialPlan::create("Hungary", "HU", "36", 9, "00"),
	DialPlan::create("Iceland", "IS", "354", 9, "00"),
	DialPlan::create("India", "IN", "91", 10, "00"),
	DialPlan::create("Indonesia", "ID", "62", 12, "001"),
	DialPlan::create("Iran", "IR", "98", 10, "00"),
	DialPlan::create("Iraq", "IQ", "964", 10, "00"),
	DialPlan::create("Ireland", "IE", "353", 9, "00"),
	DialPlan::create("Israel", "IL", "972", 9, "00"),
	DialPlan::create("Italy", "IT", "39", 10, "00"),
	//	{"Jersey"                       ,"JE"		, "44"      , 10	, "00"	},
	DialPlan::create("Jamaica", "JM", "1", 10, "011"),
	DialPlan::create("Japan", "JP", "81", 10, "010"),
	DialPlan::create("Jordan", "JO", "962", 9, "00"),
	//DialPlan::create("Kazakhstan", "KZ", "7", 10, "00"), rusia
	DialPlan::create("Kenya", "KE", "254", 9, "000"),
	DialPlan::create("Kiribati", "KI", "686", 5, "00"),
	DialPlan::create("Korea, North", "KP", "850", 12, "99"),
	DialPlan::create("Korea, South", "KR", "82", 12, "001"),
	DialPlan::create("Kuwait", "KW", "965", 8, "00"),
	DialPlan::create("Kyrgyzstan", "KG", "996", 9, "00"),
	DialPlan::create("Laos", "LA", "856", 10, "00"),
	DialPlan::create("Latvia", "LV", "371", 8, "00"),
	DialPlan::create("Lebanon", "LB", "961", 8, "00"),
	DialPlan::create("Lesotho", "LS", "266", 8, "00"),
	DialPlan::create("Liberia", "LR", "231", 8, "00"),
	DialPlan::create("Libya", "LY", "218", 8, "00"),
	DialPlan::create("Liechtenstein", "LI", "423", 7, "00"),
	DialPlan::create("Lithuania", "LT", "370", 8, "00"),
	DialPlan::create("Luxembourg", "LU", "352", 9, "00"),
	DialPlan::create("Macau", "MO", "853", 8, "00"),
	DialPlan::create("Macedonia", "MK", "389", 8, "00"),
	DialPlan::create("Madagascar", "MG", "261", 9, "00"),
	DialPlan::create("Malawi", "MW", "265", 9, "00"),
	DialPlan::create("Malaysia", "MY", "60", 9, "00"),
	DialPlan::create("Maldives", "MV", "960", 7, "00"),
	DialPlan::create("Mali", "ML", "223", 8, "00"),
	DialPlan::create("Malta", "MT", "356", 8, "00"),
	DialPlan::create("Marshall Islands", "MH", "692", 7, "011"),
	DialPlan::create("Martinique", "MQ", "596", 9, "00"),
	DialPlan::create("Mauritania", "MR", "222", 8, "00"),
	DialPlan::create("Mauritius", "MU", "230", 7, "00"),
	//DialPlan::create("Mayotte Island", "YT", "262", 9, "00"), réunion
	DialPlan::create("Mexico", "MX", "52", 11, "00"),
	//The following is a pseudo dial plan for Mexican mobile phones. See https://en.wikipedia.org/wiki/Telephone_numbers_in_Mexico
	//DialPlan::create("Mexico", "MX", "521", 10, "00"),
	DialPlan::create("Micronesia", "FM", "691", 7, "011"),
	DialPlan::create("Moldova", "MD", "373", 8, "00"),
	DialPlan::create("Monaco", "MC", "377", 8, "00"),
	DialPlan::create("Mongolia", "MN", "976", 8, "001"),
	DialPlan::create("Montenegro", "ME", "382", 8, "00"),
	DialPlan::create("Montserrat", "MS", "1", 10, "011"),
	DialPlan::create("Morocco", "MA", "212", 9, "00"),
	DialPlan::create("Mozambique", "MZ", "258", 9, "00"),
	DialPlan::create("Myanmar", "MM", "95", 10, "00"),
	DialPlan::create("Namibia", "NA", "264", 9, "00"),
	DialPlan::create("Nauru", "NR", "674", 7, "00"),
	DialPlan::create("Nepal", "NP", "977", 8, "00"),
	DialPlan::create("Netherlands", "NL", "31", 9, "00"),
	DialPlan::create("New Caledonia", "NC", "687", 6, "00"),
	DialPlan::create("New Zealand", "NZ", "64", 9, "00"),
	DialPlan::create("Nicaragua", "NI", "505", 8, "00"),
	DialPlan::create("Niger", "NE", "227", 8, "00"),
	DialPlan::create("Nigeria", "NG", "234", 10, "009"),
	DialPlan::create("Niue", "NU", "683", 4, "00"),
	DialPlan::create("Norfolk Island", "NF", "672", 5, "00"),
	DialPlan::create("Northern Mariana Islands", "MP", "1", 10, "011"),
	DialPlan::create("Norway", "NO", "47", 8, "00"),
	DialPlan::create("Oman", "OM", "968", 8, "00"),
	DialPlan::create("Pakistan", "PK", "92", 10, "00"),
	DialPlan::create("Palau", "PW", "680", 7, "011"),
	DialPlan::create("Palestine", "PS", "970", 9, "00"),
	DialPlan::create("Panama", "PA", "507", 8, "00"),
	DialPlan::create("Papua New Guinea", "PG", "675", 8, "00"),
	DialPlan::create("Paraguay", "PY", "595", 9, "00"),
	DialPlan::create("Peru", "PE", "51", 9, "00"),
	DialPlan::create("Philippines", "PH", "63", 10, "00"),
	DialPlan::create("Poland", "PL", "48", 9, "00"),
	DialPlan::create("Portugal", "PT", "351", 9, "00"),
	DialPlan::create("Puerto Rico", "PR", "1", 10, "011"),
	DialPlan::create("Qatar", "QA", "974", 8, "00"),
	DialPlan::create("Reunion Island", "RE", "262", 9, "011"),
	DialPlan::create("Romania", "RO", "40", 9, "00"),
	DialPlan::create("Russian Federation", "RU", "7", 10, "8"),
	DialPlan::create("Rwanda", "RW", "250", 9, "00"),
	DialPlan::create("Saint Helena", "SH", "290", 4, "00"),
	DialPlan::create("Saint Kitts and Nevis", "KN", "1", 10, "011"),
	DialPlan::create("Saint Lucia", "LC", "1", 10, "011"),
	DialPlan::create("Saint Pierre and Miquelon", "PM", "508", 6, "00"),
	DialPlan::create("Saint Vincent and the Grenadines", "VC", "1", 10, "011"),
	DialPlan::create("Samoa", "WS", "685", 7, "0"),
	DialPlan::create("San Marino", "SM", "378", 10, "00"),
	DialPlan::create("Sao Tome and Principe", "ST", "239", 7, "00"),
	DialPlan::create("Saudi Arabia", "SA", "966", 9, "00"),
	DialPlan::create("Senegal", "SN", "221", 9, "00"),
	DialPlan::create("Serbia", "RS", "381", 9, "00"),
	DialPlan::create("Seychelles", "SC", "248", 7, "00"),
	DialPlan::create("Sierra Leone", "SL", "232", 8, "00"),
	DialPlan::create("Singapore", "SG", "65", 8, "001"),
	DialPlan::create("Slovakia", "SK", "421", 9, "00"),
	DialPlan::create("Slovenia", "SI", "386", 8, "00"),
	DialPlan::create("Solomon Islands", "SB", "677", 7, "00"),
	DialPlan::create("Somalia", "SO", "252", 8, "00"),
	DialPlan::create("South Africa", "ZA", "27", 9, "00"),
	DialPlan::create("Spain", "ES", "34", 9, "00"),
	DialPlan::create("Sri Lanka", "LK", "94", 9, "00"),
	DialPlan::create("Sudan", "SD", "249", 9, "00"),
	DialPlan::create("Suriname", "SR", "597", 7, "00"),
	DialPlan::create("Swaziland", "SZ", "268", 8, "00"),
	DialPlan::create("Sweden", "SE", "46", 9, "00"),
	DialPlan::create("Switzerland", "XK", "41", 9, "00"),
	DialPlan::create("Syria", "SY", "963", 9, "00"),
	DialPlan::create("Taiwan", "TW", "886", 9, "810"),
	DialPlan::create("Tajikistan", "TJ", "992", 9, "002"),
	DialPlan::create("Tanzania", "TZ", "255", 9, "000"),
	DialPlan::create("Thailand", "TH", "66", 9, "001"),
	DialPlan::create("Togo", "TG", "228", 8, "00"),
	DialPlan::create("Tokelau", "TK", "690", 4, "00"),
	DialPlan::create("Tonga", "TO", "676", 5, "00"),
	DialPlan::create("Trinidad and Tobago", "TT", "1", 10, "011"),
	DialPlan::create("Tunisia", "TN", "216", 8, "00"),
	DialPlan::create("Turkey", "TR", "90", 10, "00"),
	DialPlan::create("Turkmenistan", "TM", "993", 8, "00"),
	DialPlan::create("Turks and Caicos Islands", "TC", "1", 7, "0"),
	DialPlan::create("Tuvalu", "TV", "688", 5, "00"),
	DialPlan::create("Uganda", "UG", "256", 9, "000"),
	DialPlan::create("Ukraine", "UA", "380", 9, "00"),
	DialPlan::create("United Arab Emirates", "AE", "971", 9, "00"),
	DialPlan::create("United Kingdom", "GB", "44", 10, "00"),
	//	{"United Kingdom"               ,"UK"		, "44"      , 10	, "00"	},
	DialPlan::create("United States", "US", "1", 10, "011"),
	DialPlan::create("Uruguay", "UY", "598", 8, "00"),
	DialPlan::create("Uzbekistan", "UZ", "998", 9, "8"),
	DialPlan::create("Vanuatu", "VU", "678", 7, "00"),
	DialPlan::create("Venezuela", "VE", "58", 10, "00"),
	DialPlan::create("Vietnam", "VN", "84", 9, "00"),
	DialPlan::create("Wallis and Futuna", "WF", "681", 5, "00"),
	DialPlan::create("Yemen", "YE", "967", 9, "00"),
	DialPlan::create("Zambia", "ZM", "260", 9, "00"),
	DialPlan::create("Zimbabwe", "ZW", "263", 9, "00")
};

const shared_ptr<DialPlan> DialPlan::MostCommon = DialPlan::create("generic", "", "", 10, "00");

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
