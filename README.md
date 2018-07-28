# roto

Roto is an embedded MIDI synthesizer that mimics a Hammond B3
organ. It's designed to run on a Teensy development board, with a
Teensy audio board as output.

As of July 2018, it's in the early stages of development. I'll be
presenting it at Strange Loop 2018 in
[Soul from Scratch: Designing a More Portable Organ](https://www.thestrangeloop.com/2018/soul-from-scratch-designing-a-more-portable-organ.html)

It currently requires a Teensy 3.6 (180MHz), though I would like to
target the Teensy 3.2 (72MHz) once its CPU requirements are
understood.

## Block Diagram

Here's where things are headed:

         ┌  Hammond B-3  ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
                                      ┌─────┐
         │                            │MIDI │                              │
                                      └─────┘
         │                            ┌─────────────┐                      │
              ┌──────────────┐        │percussion   │
         │    │MIDI keyboard │───┬───▶│oscillator   │──────────────────────┼────────┐
              └──────────────┘   │    └─────────────┘    ┌─────┐                    │
         │                       │                       │MIDI │           │        │
              ┌──────────────┐   └─┐                     └─────┘                    │
         │    │MIDI keyboard │───┐ │  ┌─────────────┐    ┌─────────────┐   │        │
              └──────────────┘   │ │  │91           │    │chorus/      │            │
         │                       ├─┴─▶│tonewheels   │───▶│vibrato      │───┼────────┤
              ┌──────────────┐   │    └─────────────┘    └─────────────┘            │
         │    │MIDI drawbars │───┘                                         │        │
              └──────────────┘                                                      │
         └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘        │
                                                                                    │
    ┌───────────────────────────────────────────────────────────────────────────────┘
    │
    │    ┌  Leslie 122 ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
    │                                                     ┌─────┐┌─────────┐
    │    │                                                │MIDI ││physics  │                   │
    │                                                     └─────┘└─────────┘
    │    │                                                ┌────────────────┐     ┌─────────┐   │
    │                                  ┌─────────────┐    │treble          │     │cabinet  │
    │    │     ┌──────────────┐   ┌───▶│HPF          │───▶│tremolo/vibrato │──┬─▶│model    │   │
    │          │tube preamp   │   │    └─────────────┘    └────────────────┘  │  └─────────┘
    └────┼────▶│model         │───┤                                           │                │
               └──────────────┘   │    ┌─────────────┐    ┌─────┐┌─────────┐  │
         │                        └───▶│LPF          │─┐  │MIDI ││physics  │  │                │
                                       └─────────────┘ │  └─────┘└─────────┘  │
         │                                             │  ┌────────────────┐  │                │
                                                       │  │bass            │  │
         │                                             └─▶│tremolo/vibrato │──┘                │
                                                          └────────────────┘
         └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘

## Getting Started

You will need:

* Teensy 3.6
* Teensy audio board

## Building

First, install the Arduino IDE and Teensyduino as described in the
[Teensy Tutorial](https://www.pjrc.com/teensy/tutorial.html).

Roto installs and builds like any Arduino sketch. Make sure you select
"USB Type: Serial + MIDI" when uploading.

## Using

Roto responds to key down & up events over USB-MIDI.

## Testing

Roto has an offline test suite that can be run with `make test`.
