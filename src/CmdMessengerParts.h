/*
  CmdMessenger - library that provides command based messaging

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  Initial Messenger Library - Thomas Ouellet Fredericks.
  CmdMessenger Version 1    - Neil Dudman.
  CmdMessenger Version 2    - Dreamcat4.
  CmdMessenger Version 3    - Thijs Elenbaas.
  3.6  - Fixes
  - Better compatibility between platforms
  - Unit tests
  3.5  - Fixes, speed improvements for Teensy
  3.4  - Internal update
  3.3  - Fixed warnings
  - Some code optimization
  3.2  - Small fixes and sending long argument support
  3.1  - Added examples
  3.0  - Bugfixes on 2.2
  - Wait for acknowlegde
  - Sending of common type arguments (float, int, char)
  - Multi-argument commands
  - Escaping of special characters
  - Sending of binary data of any type (uses escaping)
  */

#ifndef STARGATE_CMDMESSENGERPARTS_H
#define STARGATE_CMDMESSENGERPARTS_H

#include <Arduino.h>
#include <vector>

class CmdMessengerParts {
public:
    static void unescape(char *fromChar) {
        // Move unescaped characters right
        char *toChar = fromChar;
        while (*fromChar != '\0') {
            if (*fromChar == '/') {
                fromChar++;
            }
            *toChar++ = *fromChar++;
        }
        // Pad string with \0 if string was shortened
        for (; toChar < fromChar; toChar++) {
            *toChar = '\0';
        }
    }

    template < class T >
    static T readBin(char *str)
    {
        T value;
        unescape(str);
        byte *bytePointer = (byte *)(const void *)&value;
        for (unsigned int i = 0; i < sizeof(value); i++)
        {
            *bytePointer = str[i];
            bytePointer++;
        }
        return value;
    }

    template < class T >
    static std::vector<uint8_t> writeBin(const T & value)
    {
        std::vector<uint8_t> output;
        const byte *bytePointer = (const byte *)(const void *)&value;
        for (unsigned int i = 0; i < sizeof(value); i++) {
            if (*bytePointer == ',' || *bytePointer == ';' || *bytePointer == '/' || *bytePointer == '\0')
                output.emplace_back('/');
            output.emplace_back(*bytePointer);
            bytePointer++;
        }
        return output;
    }
};

#endif //STARGATE_CMDMESSENGERPARTS_H
