package model

type MidiFile struct {
	Id           uint64 `json:"id"`
	Path         string `json:"path"`
	MetaDataRepo `json:"metaData"`
}
