package main

import (
	"fmt"
	"log"
	"os"

	"gitlab.com/gomidi/midi/v2/drivers"
	"gitlab.com/gomidi/midi/v2/drivers/rtmididrv"
	"gitlab.com/gomidi/midi/v2/smf"
)

func main() {
	drv, port, midiErr := openMidi()
	if midiErr != nil {
		log.Fatalf("error: %s\n", midiErr)
		//not reached
	}

	defer func() { _ = drv.Close() }()
	defer func() { _ = port.Close() }()

	for _, fileToPlay := range os.Args[1:] {
		if playErr := smf.ReadTracks(fileToPlay).Play(port); playErr != nil {
			log.Printf("warning: %s\n", playErr)
		}
	}
}

func openMidi() (drivers.Driver, drivers.Out, error) {
	drv, drvErr := rtmididrv.New()
	if drvErr != nil {
		return nil, nil, fmt.Errorf("failed to initialize MIDI driver: %w", drvErr)
	}

	ports, portsErr := drv.Outs()
	if portsErr != nil || len(ports) == 0 {
		return nil, nil, fmt.Errorf("failed to get output ports: %w", portsErr)
	}

	port := ports[0]
	if portErr := port.Open(); portErr != nil {
		return nil, nil, fmt.Errorf("failed to open output port: %w", portErr)
	}

	return drv, port, nil
}
