# Building and installing on Windows

## What you need

| | |
|---|---|
| Visual Studio 2022 | The free Community edition is fine. Install the **Desktop development with C++** workload. Build Tools alone also works. |
| CMake 3.22 or newer | `winget install Kitware.CMake`, or the installer from cmake.org. Let it add itself to PATH. |
| Git | `winget install Git.Git`. CMake uses it to fetch JUCE. |

JUCE is downloaded automatically on the first configure - you do not need to
install it yourself. Budget a few minutes and about 1 GB for that first build.

## Build

```powershell
cd bass-amp-emulator
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```

Output lands in:

```
build\BassAmpStudio_artefacts\Release\Standalone\BassAmp Studio.exe
build\BassAmpStudio_artefacts\Release\VST3\BassAmp Studio.vst3
```

The standalone `.exe` is self-contained - copy it anywhere and run it. For the
VST3, copy the `.vst3` folder to `C:\Program Files\Common Files\VST3\` and your
DAW will find it on the next scan.

Run the DSP test suite with:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

## ASIO support (do this - it matters)

ASIO is the difference between a rig you can play and one you cannot. Steinberg's
licence does not allow the SDK to be redistributed, so it is not in this
repository, but adding it takes two minutes:

1. Download the ASIO SDK from Steinberg's developer site.
2. Unzip it somewhere permanent, e.g. `C:\SDKs\asiosdk`. That folder should
   contain `common\` and `host\`.
3. Configure with the path:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DBASSAMP_ASIO_SDK_PATH="C:/SDKs/asiosdk"
cmake --build build --config Release --parallel
```

The app prints which driver types it found in **SETUP → Audio device settings**.

Without the SDK the app still runs, using WASAPI. WASAPI in **exclusive** mode is
usable - typically 5 to 10 ms round trip. Shared mode and DirectSound are not:
expect 20 to 40 ms, which you will hear as lag against a backing track.

## First run

1. Open **BassAmp Studio.exe**.
2. Go to **SETUP → Audio device settings**.
   - Device type: **ASIO** if it is listed, otherwise **Windows Audio (Exclusive Mode)**.
   - Output: your interface.
   - Active input channel: the channel your bass is plugged into. Deselect the rest.
   - Sample rate: 48000.
   - Buffer size: **128** to start with. Drop to 64 if your machine keeps up.
3. Load the **Flat Reference** preset and set your input gain so that hard
   playing peaks around -12 dBFS on the IN meter. Every other preset assumes you
   did this.
4. Now pick a preset and play.

## If the buffer will not go low

- Set Windows power plan to **High performance** (or **Ultimate performance**).
  On a laptop, plug it in - on battery, CPU throttling will cause dropouts at
  small buffers.
- In Device Manager, disable Wi-Fi briefly to test: some Wi-Fi drivers cause DPC
  latency spikes that show up as clicks at 64 samples.
- Turn off "USB selective suspend" in the power plan's advanced settings.
- LatencyMon (free) will tell you which driver is responsible if you are still
  getting dropouts.
- In the app, **SETUP → Distortion oversampling → Off** roughly halves the CPU
  cost of the amp section. You will hear a little more aliasing on heavy
  distortion and essentially nothing on clean sounds.

## Building a distributable

The CI workflow (`.github/workflows/bass-amp-emulator.yml`) builds both formats
on every push and uploads them as an artifact, so you can download a build
without having a compiler installed at all: open the Actions tab, pick the most
recent run, and download **BassAmpStudio-windows-x64**.
