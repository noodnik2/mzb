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
[NS MIDI Player](https://apps.nitinseshadri.com/midiplayer/), which I found
works pretty well.

## `cmd/player` - MIDI Source File Player

The first CLI (PoC) created is `cmd/player`, which is useful for serializing
MIDI files out to a MIDI player, for example as configured in MacOS' [Audio MIDI Setup]
app.  To run it, just give the target MIDI port where you want the MIDI events
to be sent, and the list of MIDI file(s) to play on that target; e.g.:

```shell
$ go run cmd/player/main.go 1 one.mid two.mid ...
```

The `1` value as the first parameter specifies "MIDI Port 1" for which a configured
MIDI device / player is waiting to receive / play the MIDI event stream.

[Audio MIDI Setup]: https://support.apple.com/guide/audio-midi-setup/set-up-midi-devices-ams875bae1e0/mac

