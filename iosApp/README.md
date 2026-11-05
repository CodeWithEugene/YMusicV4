# `:iosApp`

The iOS product shell runs the shared Compose `App` from `:commonUi` over the AVFoundation local and MSE YouTube engines.

`iosApp/iOSApp.swift` hosts `MainViewControllerKt.MainViewController()` from the `YMusicApp` framework. The framework builds the iOS `AppGraph` and runs `App(graph)`.

## Requirements

The app requires Xcode 26 or newer. Compose Multiplatform 1.11.1 links against iOS 26 UIKit symbols, so older SDKs cannot link the framework safely.

Build the pinned sibling `ffmpeg-build` checkout before building the app.

```sh
git clone --recursive --shallow-submodules https://github.com/ymusic-io/ffmpeg-build ../ffmpeg-build
git -C ../ffmpeg-build checkout "$(sed -n 's/^ *FFMPEG_BUILD_REF: //p' .github/workflows/release.yml)"
../ffmpeg-build/scripts/build-ios.sh iphoneos
../ffmpeg-build/scripts/build-ios.sh iphonesimulator
```

The build stops with this command when the native iOS libraries are absent. A fallback native stub is available to library consumers on Apple targets, but a functional YMusic iOS application requires the real MSE host.

## Run on a device

1. Run `ruby gen_xcodeproj.rb` after changing the project generator. It requires the `xcodeproj` gem.
2. Open `iosApp.xcodeproj`.
3. Select the `iosApp` target and choose your Team under Signing and Capabilities.
4. Connect and unlock the device, trust the computer, and enable Developer Mode.
5. Select the device and run the app.
6. With a free Personal Team, trust the developer profile under Settings, General, and VPN and Device Management.

The Xcode build phase runs `:iosAppKotlin:embedAndSignAppleFrameworkForXcode` with `-PkmpTargets=all`, which configures the device and Apple silicon simulator targets.

## Simulator

Compose Multiplatform 1.11.1 does not provide an `iosX64` target. The simulator therefore requires an Apple silicon Mac and uses `iosSimulatorArm64`.

## Notes

* The iOS graph currently has no media store, so Library is empty.
* Local video does not yet use the shared iOS surface. AVFoundation local playback renders through its own player layer.
* Settings to Link devices uses the camera to scan a desktop QR code, then requires a PIN or Face ID before sending encrypted YouTube cookies.
