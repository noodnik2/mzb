# Midi Zuke Box (MZB) - `go` Version!

My interest in MIDI was recently revived due to the arrival of a guest from 
Japan - I wanted to play for them some of the old Anime MIDI files I collected
years ago!

Since MS-DOS and `C` is out of style these days 🥹, I decided to see what I
could find to build on to revive `mzb` using a more modern toolset.  Since my
"lingua franca" these days is `go`, it was the natural choice.

I found [gomidi/midi](https://gitlab.com/gomidi/midi) and decided to give
it a try.

## Roadmap

Ultimately, I'm looking to revive the utility of MZB: an easy-to-use
"playlist" sort of app for playing MIDI files.  

Of course, since MZB's "heyday" (several decades ago!), lots of other great,
similar apps have spawned.  One notable one I found is
[NS MIDI Player](https://apps.nitinseshadri.com/midiplayer/)
which I was able to try since it still supports older versions of MacOS
(which I was using when I was looking around) via its
[Legacy repo](https://github.com/nitinseshadri/NSMIDIPlayer-Legacy).

## `cmd/player`

The first CLI (PoC) created is `cmd/player`.  To run it:

```shell
$ go run cmd/player/main.go one.mid two.mid ...
```

Just give it a list of MIDI file(s) to play.

As of this writing, it supports (only) the
[`rtmididrv` driver](https://pkg.go.dev/gitlab.com/gomidi/midi/v2/drivers/rtmididrv),
since that one seems to work out of the box with MacOS' "CoreMIDI" feature.

### TODOs

The initial version sounds _really_ bad compared to other players.

- Maybe it's not sending through _sysex_?
- Maybe it's not selecting the best patches?
- ???

### Dependencies

Of course a working MIDI playback environment is needed to hear the MIDI files play.  

The only setup currently tested is on MacOS by enabling the "IAC Driver" using the
["Audio MIDI Setup"](https://support.apple.com/guide/audio-midi-setup/set-up-midi-devices-ams875bae1e0/mac)
using the "MIDI Studio".  Once that driver is enabled, and with
a loaded set of patches (such as those installed by GarageBand or perhaps FluidSynth),
you should be able to play MIDI files using this CLI utility.


