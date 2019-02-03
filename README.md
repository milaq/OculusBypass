![](res/logo.png)

# OculusBypass

Most (if not all) games which support both Oculus and SteamVR use the first found HMD backend for initialization; usually, they first look for an Oculus client. Most of the time the user has no choice to make use of other backends when using an Oculus Rift.

This leads to a problem where the user wants to utilize SteamVR as a middleware layer over the Oculus client (e.g. when using a **DK2 in conjuction with Razer Hydras or PSMoveService**).
Only SteamVR allows the user to utilize third party input devices as the Oculus client only supports the Oculus Touch controllers.

This project is designed to block loading of Oculus search and init calls for a specific process (read "game executable").
The process will then fall back to SteamVR, which in turn communicates with the Oculus client on its own, enabling the use of third party controllers.

There is also a project called [Revive](https://github.com/LibreVR/Revive). This also allows you to accomplish the above but with a few drawbacks if not using the HTC Vive.
Revive completely emulates an HTC Vive for the Oculus client; this leads to the following scenario when using a DK1 or DK2: `Game executable > Revive > HTC Vice emulation > SteamVR > Oculus backend of SteamVR > Oculus client`

With OculusBypass we are able to only block the direct loading of the  Oculus client without interfering with SteamVR's native Oculus Rift support. Thus, we can use SteamVR directly without any emulation: `Game executable > SteamVR > Oculus backend of SteamVR > Oculus client`.


# How does this work?

We need to return NULL on specific Oculus library calls or library searches. We do so by hooking into XInput loading as all Oculus enabled titles use XInput in one way or another.

Because the library search call on Winblows always looks in the executable's directory first, we can use the widely used xinput1_3.dll approach.
All XInput related calls get rerouted to the system provided library (usually in `system32`) while hooking into some Oculus initialization and search calls.

# Usage

You need to compile the VS project for the correct executable architecture (usually x86-64) or just download the respective DLL from the [releases page](https://github.com/milaq/OculusBypass/releases).

1) Find the main executable of the process for which you want to block the Oculus client initialization (sometimes buried deeply in the game directories, e.g. for the Unreal engine).
2) Rename `LibOculusBypass64.dll` (or `LibOculusBypass32.dll` for x86 only games, but almost no modern game uses 32 bit anymore) to `xinput1_3.dll` and put it into the folder you found in 1).
3) Run the game.

The game will fail to communicate directly with the Oculus client and fall back to SteamVR (if the game supports it). SteamVR will then communicate with the Oculus client on its own while also enabling third party gesture input controllers.

# Credit

 * Thanks to Tsuda Kageyu for his awesome [minhook](https://github.com/TsudaKageyu/minhook)
 * Thanks to Jules Blok for some XInput related code borrowed from [Revive](https://github.com/LibreVR/Revive)
