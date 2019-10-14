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
package org.linphone.core;


public class DialPlanImpl implements DialPlan {
    private final String countryName;
    private final String countryCode;
    private final String countryCallingCode;
    private final int numberLength;
    private final String usualPrefix;

    public DialPlanImpl(String countryName, String countryCode, String countryCallingCode, int numberLength, String usualPrefix) {
        this.countryName = countryName;
        this.countryCode = countryCode;
        this.countryCallingCode = countryCallingCode;
        this.numberLength = numberLength;
        this.usualPrefix = usualPrefix;
    }


    @Override
    public final String getCountryCode() {
        return countryCode;
    }

    @Override
    public final String getCountryName() {
        return countryName;
    }

    @Override
    public final String getCountryCallingCode() {
        return countryCallingCode;
    }

    @Override
    public final int getNumberLength() {
        return numberLength;
    }

    @Override
    public final String getUsualPrefix() {
        return usualPrefix;
    }
}
