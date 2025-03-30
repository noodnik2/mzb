package main

import (
	"fmt"
	"os"
	"strconv"
	"strings"

	"gitlab.com/gomidi/midi/v2/drivers"
	"gitlab.com/gomidi/midi/v2/drivers/rtmididrv"
	"gitlab.com/gomidi/midi/v2/smf"
)

func main() {
	port, errPort := parsePortArg()
	if errPort != nil {
		fatal("failed to initialize: %s\n", errPort)
		//unreached
	}

	if port == nil {
		// parseArgs has satisfied all requirements
		return
	}

	defer port.Close()

	if len(os.Args) < 3 {
		cmds := strings.Split(os.Args[0], "/")

		fatal("syntax: %s [<midiPortIdx> <midiFile1> [<midiFile2> [...]]]\n", cmds[len(cmds)-1])
		//unreached
	}

	filesToPlay := os.Args[2:]
	for _, fileToPlay := range filesToPlay {
		stdout("now playing: %s\n", fileToPlay)

		if playErr := smf.ReadTracks(fileToPlay).Play(port); playErr != nil {
			stderr("warning: %s\n", playErr)
		}
	}
}

func parsePortArg() (drivers.Out, error) {
	ports, errOpen := openMidiPorts()
	if errOpen != nil {
		return nil, fmt.Errorf("open MIDI: %w", errOpen)
	}

	nArgs := len(os.Args)
	if nArgs < 2 {
		// no port was specified by the user; just list the ones we found
		if len(ports) == 0 {
			stdout("No MIDI port(s) available\n")
		} else {
			stdout("MIDI port(s) available:\n")
			for i, port := range ports {
				stdout("%d: %s\n", i, port.String())
			}
		}

		return nil, nil
	}

	portIdxS := os.Args[1]
	portIdx, errConv := strconv.Atoi(portIdxS)
	if errConv != nil {
		return nil, fmt.Errorf("can't parse specified MIDI port %q: %w", portIdxS, errConv)
	}

	if portIdx < 0 || len(ports) <= portIdx {
		return nil, fmt.Errorf("MIDI port %d not available", portIdx)
	}

	port := ports[portIdx]
	if portErr := port.Open(); portErr != nil {
		return nil, fmt.Errorf("can't open MIDI port #%d: %w", portIdx, portErr)
	}

	return port, nil
}

func openMidiPorts() ([]drivers.Out, error) {
	drv, drvErr := rtmididrv.New()
	if drvErr != nil {
		return nil, fmt.Errorf("rtmididrv.New: %w", drvErr)
	}

	defer drv.Close()

	ports, portsErr := drv.Outs()
	if portsErr != nil {
		return nil, fmt.Errorf("rtmididrv.Outs: %w", portsErr)
	}

	return ports, nil
}

func fatal(format string, args ...any) {
	stderr(format, args...)

	os.Exit(1)
	// unreached
}

func stdout(format string, args ...any) {
	fmt.Printf(format, args...)
}

func stderr(format string, args ...any) {
	_, _ = fmt.Fprintf(os.Stderr, format, args...)
}
