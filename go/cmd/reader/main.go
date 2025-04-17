package main

import (
	"encoding/json"
	"fmt"
	"os"

	"gitlab.com/gomidi/midi/v2/smf"
)

type fileMap map[string][]string

var metaIgnores = map[string]any{
	"MetaTempo":       nil,
	"MetaPort":        nil,
	"MetaEndOfTrack":  nil,
	"MetaTimeSig":     nil,
	"MetaKeySig":      nil,
	"MetaSMPTEOffset": nil,
	"MetaChannel":     nil,
	"MetaInstrument":  nil,
	"MetaSeqData":     nil,
	"MetaMarker":      nil,
}

// Looking for:
//  (C)
//  Copyright
//  @
// "written by"
//

func main() {
	meta := make(fileMap)

	for _, fileName := range os.Args[1:] {
		stderr("processing: %s\n", fileName)

		fileMeta := make([]string, 0)

		tracks, err := readTracks(fileName)
		if err != nil {
			stderr("-> error: %s\n", err)

			fileMeta = append(fileMeta, err.Error())
		} else {
			for _, track := range tracks {
				for _, event := range track {
					message := event.Message

					if message.IsMeta() {
						if _, found := metaIgnores[message.Type().String()]; !found {
							fileMeta = append(fileMeta, message.String())
						}
					}
				}
			}
		}

		meta[fileName] = fileMeta
	}

	jsonMetaBytes, err := json.Marshal(meta)
	if err != nil {
		fatal("can't marshal JSON", err)
	}

	fmt.Println(string(jsonMetaBytes))
}

func readTracks(fileName string) (tracks []smf.Track, err error) {
	defer func() {
		if r := recover(); r != nil {
			err = fmt.Errorf("smf.ReadTracks(%s): panic: %s", fileName, r)
		}
	}()

	trackReader := smf.ReadTracks(fileName)
	if trackReader.Error() != nil {
		err = fmt.Errorf("smf.ReadTracks(%s): %w", fileName, trackReader.Error())
		return
	}

	return trackReader.SMF().Tracks, nil
}

func fatal(format string, args ...any) {
	stderr(format, args...)

	os.Exit(1)
	// unreached
}

func stderr(format string, args ...any) {
	_, _ = fmt.Fprintf(os.Stderr, format, args...)
}
