
# VisionStream

**Disclaimer:** This project is not endorsed or certified by Sony Interactive Entertainment LLC.



VisionStream is a free, native, open source PS5 streaming app for visionOS that uses Chiaki. This project extends the original Chiaki codebase to provide PlayStation Remote Play functionality specifically optimized for Apple Vision Pro.

Based on the excellent [Chiaki](https://git.sr.ht/~thestr4ng3r/chiaki) project, VisionStream brings PlayStation 5 Remote Play to visionOS with native SwiftUI interfaces and spatial computing optimizations.

## Project Status

VisionStream is currently in active development for visionOS. This project extends the original Chiaki codebase to provide native PlayStation Remote Play functionality on Apple Vision Pro.

**Note:** The original Chiaki project is in maintenance mode, but VisionStream represents an active development effort to bring PlayStation streaming to visionOS.

## Installing

VisionStream is built for visionOS and requires Apple Vision Pro. You can build the project from source using Xcode.

### Building from Source

**Requirements:**
- Apple Vision Pro or visionOS Simulator
- Xcode 15.0 or later
- visionOS SDK

**Build Instructions:**
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/visionstream.git
   cd visionstream
   ```

2. Open the project in Xcode:
   ```bash
   open visionstreamswift/visionstreamswift.xcodeproj
   ```

3. Select your target device (Apple Vision Pro or Simulator)
4. Build and run the project (⌘+R)

### Original Chiaki Platforms

The original Chiaki project supports Linux, FreeBSD, OpenBSD, Android, macOS, Windows, and Nintendo Switch. You can download pre-built releases [here](https://git.sr.ht/~thestr4ng3r/chiaki/refs).

## Usage

VisionStream provides a native visionOS experience for PlayStation Remote Play. The app uses spatial computing features to create an immersive gaming environment on Apple Vision Pro.

### Getting Started

1. **Network Discovery**: If your PlayStation 5 is on your local network and in standby mode, VisionStream should automatically discover it.

2. **Manual Setup**: If automatic discovery doesn't work, you can manually add your console by entering its IP address.

3. **Registration**: You'll need to register your console with VisionStream using your PSN Account ID and a registration PIN.

### visionOS Features

- **Spatial Gaming**: Play PlayStation games in your physical space using Apple Vision Pro's spatial computing
- **Native Interface**: Built with SwiftUI for optimal visionOS integration
- **Immersive Experience**: Leverage Vision Pro's high-resolution displays for crisp gameplay

## Optimizing Performance: Ethernet + Internet Sharing Setup

For the best streaming performance with VisionStream, you can create a dedicated high-speed network using your Mac as a bridge. This setup minimizes latency and maximizes bandwidth for PlayStation Remote Play.

### Prerequisites
- Mac with Ethernet port
- PlayStation 5 with Ethernet connection
- Both devices on the same local network
- Apple Vision Pro

### Step-by-Step Setup

1. **Connect Both Devices to Ethernet**
   - Connect your PlayStation 5 to your router via Ethernet cable
   - Connect your Mac to the same router via Ethernet cable
   - Ensure both devices are on the same local area network (LAN)

2. **Enable Internet Sharing on macOS**
   - Open **System Settings** (or System Preferences on older macOS versions)
   - Navigate to **General** → **Sharing**
   - Find and enable **Internet Sharing**
   - If you don't see it, click the **+** button to add it

3. **Configure Internet Sharing**
   - **Share your connection from**: Select your Ethernet connection (usually named "Ethernet" or similar)
   - **To computers using**: Select **Wi-Fi**
   - Click **Wi-Fi Options...** to configure the hotspot

4. **Set Up Wi-Fi Hotspot**
   - **Network Name**: Create a memorable name (e.g., "PS5-VisionStream")
   - **Channel**: Choose **44** (5GHz), **11**, or **6** in order of preference
     - Channel 44 is recommended as it's a 5GHz channel with less interference
   - **Security**: Set a strong password
   - Click **OK** to save settings

5. **Connect Vision Pro**
   - On your Apple Vision Pro, go to **Settings** → **Wi-Fi**
   - Connect to the hotspot you just created
   - Enter the password when prompted

6. **Manual PS5 Connection**
   - Due to Network Address Translation (NAT), automatic discovery may not work
   - In VisionStream, manually add your PS5 using its IP address
   - You can find your PS5's IP address in: **Settings** → **Network** → **View Connection Status**

### Benefits
- **Reduced Latency**: Direct connection path minimizes network hops
- **Higher Bandwidth**: Dedicated 5GHz channel provides optimal throughput
- **Stable Connection**: Ethernet backbone ensures consistent performance
- **Lower Interference**: Dedicated network reduces congestion from other devices

### Troubleshooting
- If connection is unstable, try different Wi-Fi channels (44, 11, or 6)
- Ensure your Mac's Wi-Fi is disabled to prevent conflicts
- Check that both devices show strong signal strength
- Verify PS5 is in standby mode or powered on

### Obtaining your PSN AccountID

Starting with PS4 7.0, it is necessary to use a so-called "AccountID" as opposed to the "Online-ID" for registration (streaming itself did not change).
This ID seems to be a unique identifier for a PSN Account.

#### Easy Method (Recommended)
1. Visit [https://psn.flipscreen.games/](https://psn.flipscreen.games/)
2. Enter your PlayStation Network username (Online ID)
3. Click "Submit" to search PSN
4. Copy the **Chiaki Account ID** from the results
5. **Note:** Your privacy settings need to allow "Anyone" to find you in search. You can change this back once you have your Account ID.

#### Alternative Method (Command Line)
A Python 3 script which does this is provided in [scripts/psn-account-id.py](scripts/psn-account-id.py).
Simply run it in a terminal and follow the instructions. Once you know your ID, write it down. You will likely never have to do this process again.

### Obtaining a Registration PIN

To register a Console with a PIN, it must be put into registration mode. To do this on a PS5, simply go to:
Settings -> Remote Play -> Pair Device, or on a PS4: Settings -> System -> Remote Play -> Add Device.

You can now double-click your Console in Chiaki's main window to start Remote Play.

## Acknowledgements

This project has only been made possible because of the following Open Source projects:
[Rizin](https://rizin.re),
[Cutter](https://cutter.re),
[Frida](https://www.frida.re) and
[x64dbg](https://x64dbg.com).

[delroth](https://github.com/delroth) - registration and wakeup protocol,
[grill2010](https://github.com/grill2010) - analyzing the PSN's OAuth Login,
[FioraAeterna](https://github.com/FioraAeterna) - FEC and error correction.

## About

VisionStream is a visionOS adaptation of Chiaki, created to bring PlayStation Remote Play to Apple Vision Pro.

**Original Chiaki Project:** Created by Florian Märkl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Additional permission under GNU AGPL version 3 section 7

If you modify this program, or any covered work, by linking or
combining it with the OpenSSL project's OpenSSL library (or a
modified version of that library), containing parts covered by the
terms of the OpenSSL or SSLeay licenses, the Free Software Foundation
grants you additional permission to convey the resulting work.
Corresponding Source for a non-source form of such a combination
shall include the source code for the parts of OpenSSL used as well
as that of the covered work.
