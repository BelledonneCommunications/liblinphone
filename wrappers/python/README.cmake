<!--
Copyright (c) 2010-2024 Belledonne Communications SARL.

This file is part of Liblinphone 
(see https://gitlab.linphone.org/BC/public/liblinphone).

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
-->

# Python API reference documentation

Liblinphone is a high-level open source library that integrates all the SIP voice/video and instant messaging features into a single easy-to-use API. This is the VoIP SDK engine on which Linphone applications are based.

Liblinphone combines our media processing and streaming toolkit (Mediastreamer2) with our user-agent library for SIP signaling (belle-sip).

Liblinphone is distributed under GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html). Please understand the licencing details before using it!
For any use of this library beyond the rights granted to you by the GPLv3 license, please [contact Belledonne Communications](https://www.linphone.org/contact).

## Other supported languages

 Liblinphone has support for a variety of languages, each one has its own reference documentation:

 - C (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/c)
 - C++ (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/c++)
 - Swift (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/swift)
 - Java (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/java)
 - C# (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/cs)
 - Python (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/python)

## See also
http://www.linphone.org

# Getting started

Here's how to import our module and print the liblinphone version that you are using:
```python
from pylinphone import linphone
print(linphone.Core.get_version())
```

## Troubleshooting

If you encounter an issue about grammar not being found, and that you installed our module using the wheel, here's how to fix it:
```python
import os
from pylinphone import linphone

linphone.Factory.get().top_resources_dir = os.path.dirname(os.path.abspath(linphone.__file__)) + "/share/" 
```