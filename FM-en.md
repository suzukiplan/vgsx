# Design Note: FM Sound Emulation

VGS-X includes a YM2612 (OPN2) emulator and can play music using FM sound synthesis.

The FM sound emulator is based on [ymfm](./LICENSE-ymfm.txt), with the following modifications:

- Added selectable output behavior for `ym2612`, `ym3438`, and `ymf276`.
- Changed the YM2612 output approximation from the previous “simply sum six channels and average them” method to a model inspired by pin-level output behavior. This model handles the active output phase and the sign output during side-off separately.
- Added a two-stage latch state, `ch_out` / `output`, so that the averaged YM2612 output is based on latched channel values rather than the FM sum from the same cycle.
- Preserved the high-resolution mixed-DAC path for `ymf276`, while ensuring that the YM2612-specific DAC discontinuity model is not applied to `ym3438`.

These features were implemented based on ideas from the implementation details of other OPN2 emulators, such as Nuked OPN2 and YMF276-LLE.

> _Note: Nuked OPN2 and YMF276-LLE are GPL-licensed free software. For this reason, VGS-X does not reuse their source code. Instead, it implements these behaviors independently while referring to publicly known behavior and concepts._

## Analog Emulation

The mechanism by which FM sound is generated:

1. The program updates the registers of the FM sound IC.
2. The FM sound IC generates a digital waveform.
3. The DAC converts the digital waveform into an analog waveform.
4. Filters such as op-amps process the analog waveform.
5. The processed analog waveform is output to LINE OUT.
6. Speakers or other devices play back the sound received from LINE OUT.

A typical FM sound emulator is intended to emulate the FM sound IC itself.

Specifically, steps `1` and `2` above are the responsibility of the emulator.

However, VGS-X also models the subsequent stages, `3` to `4`, in a simplified form. The goal is not only to reproduce the output of the IC itself, but also to include the sound-quality changes that occur after the DAC stage.

## Comparison Targets

The same song was played back on the following four devices/emulators, and their waveform characteristics were compared:

1. Real YM2612 chip environment: Project RE:birth RE1-YM2612/3438
2. A [fork](https://github.com/suzukiplan/Nuked-OPN2) of Nuked OPN2, another OPN2 emulator
3. VGS-X without analog emulation: the `clean` preset
4. VGS-X with analog emulation: the `re1e` preset

## Comparison Results

The comparison used [./example/10_ym2612/test.vgm](./example/10_ym2612/test.vgm).

The evaluation values compare the average spectral shape across the following 11 frequency bands after RMS-normalizing each segment.

- `20-80Hz`
- `80-160Hz`
- `160-315Hz`
- `315-630Hz`
- `630Hz-1.25kHz`
- `1.25-2.5kHz`
- `2.5-5kHz`
- `5-8kHz`
- `8-12kHz`
- `12-16kHz`
- `16-22kHz`

The numbers are the RMSE of the band profile relative to `re1`.
Lower values indicate a spectral balance closer to `re1`.

| Comparison Segment | clean | Nuked OPN2 | re1e |
|---|---:|---:|---:|
| 0-2s, mainly pickup notes | 5.797 dB | **5.714 dB** | 5.914 dB |
| 0-4s | **1.554 dB** | 1.702 dB | 1.556 dB |
| 0.5-6.5s, mainly melody | 1.059377 dB | 1.321459 dB | **1.059376 dB** |
| 0-8s, overall | 1.004 dB | 1.154 dB | **1.000 dB** |

In the 0.5-6.5s melody-focused segment, `re1e` is almost identical to `clean`, but its RMSE is slightly lower than `clean`.
Across the full 0-8s segment, `re1e` also has the lowest value.
On the other hand, in the 0-2s segment, which mainly consists of pickup notes, Nuked OPN2 is the closest, while `re1e` is slightly farther from `re1` than `clean`.

The improvement rate of `re1e` across the full 0-8s segment is as follows:

| Source | RMSE | Difference from re1e | Improvement |
|---|---:|---:|---:|
| clean | 1.004 dB | -0.004 dB | 0.4% |
| Nuked OPN2 | 1.154 dB | -0.154 dB | 13.4% |

`re1e` applies only a very subtle correction compared with `clean`.
The band differences between `re1e` and `clean` across the full 0-8s segment are as follows:

| Band | re1e - clean |
|---|---:|
| 20-80Hz | +0.006 dB |
| 80-160Hz | +0.005 dB |
| 160-315Hz | +0.005 dB |
| 315-630Hz | +0.005 dB |
| 630Hz-1.25kHz | +0.005 dB |
| 1.25-2.5kHz | +0.006 dB |
| 2.5-5kHz | +0.004 dB |
| 5-8kHz | +0.001 dB |
| 8-12kHz | -0.005 dB |
| 12-16kHz | -0.012 dB |
| 16-22kHz | -0.019 dB |

In other words, `re1e` is not a preset that actively reinforces the low end. Instead, it largely preserves the spectrum of `clean` while slightly rounding off only the highest frequency bands.

## Discussion

In this comparison, `clean` was already very close to the real-chip recording `re1` in terms of spectral balance.
Because of that, applying a large amount of analog correction with `re1e` would make the sound either too bass-heavy or lacking in high frequencies, especially in the melody-focused segment, and would actually move it farther away from `re1`.

In the final tuning of `re1e`, low-end reinforcement, notches, nonlinear distortion, and bass saturation were not used. Only a very weak low-pass filter was retained.
As a result, across the full 0-8s segment and the melody-focused segment, `re1e` moved slightly closer to `re1` than `clean`.
However, the amount of improvement is small, and `clean` and `re1e` are also very close perceptually.

Nuked OPN2 was closest to `re1` in the 0-2s segment, which mainly consists of pickup notes.
On the other hand, in the melody-focused segment and across the full 0-8s segment, its RMSE was larger than `clean` / `re1e`. In terms of the average spectral balance of the whole song, VGS-X tends to be closer to `re1`.

Based on the above, it is reasonable to position `re1e` not as a preset that strongly reproduces the entire real-chip recording path, but as a preset that makes a very subtle adjustment from `clean` toward the recording characteristics of the Project RE:birth RE1-YM2612/3438.
If the goal is sound design that compensates for the lightness of real FM sound hardware, it is more consistent with both the intent and the measurement results to handle that with a separate preset such as `real`, rather than `re1e`.

## The `real` Preset / Default

`re1e` is a preset intended to make a very subtle adjustment toward the recording characteristics of the Project RE:birth RE1-YM2612/3438.
By contrast, VGS-X’s default preset, `real`, is not designed solely to match a real-chip recording. It also emphasizes how practical and pleasant the FM sound is for use in games.

In real FM sound recordings, the low end may become weak depending on the recording environment, making the whole track sound somewhat light.
For this reason, the `subtle` preset was first created to reinforce the low end slightly.
However, as a side effect of this low-end reinforcement, `subtle` tended to weaken the high end and attack slightly.

Therefore, the `real` preset was created to preserve a certain amount of low-end thickness while also maintaining the high frequencies and attack. This preset was then made the default in VGS-X.
`real` is not a strict reproduction of a real-chip recording. Rather, it is a practical preset designed to balance the character of real FM sound hardware with listenability for game use.
