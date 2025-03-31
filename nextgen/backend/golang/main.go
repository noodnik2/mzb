package main

import (
	"encoding/json"
	"fmt"
	"github.com/fsnotify/fsnotify"
	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
	"io/ioutil"
	"os"
	"path/filepath"
	"sync"
)

const (
	MidiDir = "/Users/martyross/repos/noodnik2/mzb/samples/src/240818-nuvi"
	DBFile  = "midi_metadata.json"
)

var (
	dataLock sync.Mutex
)

// MIDIFile represents a single MIDI file with metadata
type MIDIFile struct {
	ID       string            `json:"id"`
	Name     string            `json:"name"`
	Path     string            `json:"path"`
	Metadata map[string]string `json:"metadata"`
}

// Database structure
type Database struct {
	Files []MIDIFile `json:"files"`
}

// Load metadata from JSON file
func loadMetadata() (*Database, error) {
	dataLock.Lock()
	defer dataLock.Unlock()

	file, err := os.Open(DBFile)
	if err != nil {
		if os.IsNotExist(err) {
			return &Database{Files: []MIDIFile{}}, nil
		}
		return nil, err
	}
	defer file.Close()

	var db Database
	if err := json.NewDecoder(file).Decode(&db); err != nil {
		return nil, err
	}
	return &db, nil
}

// Save metadata to JSON file
func saveMetadata(db *Database) error {
	dataLock.Lock()
	defer dataLock.Unlock()

	file, err := json.MarshalIndent(db, "", "  ")
	if err != nil {
		return err
	}
	return ioutil.WriteFile(DBFile, file, 0644)
}

// Scan directory for new MIDI files
func scanMidiFiles() {
	db, _ := loadMetadata()
	existingPaths := make(map[string]bool)

	for _, file := range db.Files {
		existingPaths[file.Path] = true
	}

	err := filepath.Walk(MidiDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if filepath.Ext(path) == ".mid" {
			if _, exists := existingPaths[path]; !exists {
				newFile := MIDIFile{
					ID:       filepath.Base(path),
					Name:     filepath.Base(path),
					Path:     path,
					Metadata: make(map[string]string),
				}
				db.Files = append(db.Files, newFile)
			}
		}
		return nil
	})

	if err != nil {
		fmt.Println("Error scanning MIDI files:", err)
	}

	saveMetadata(db)
}

// Watch for file changes
func watchMidiDir() {
	watcher, err := fsnotify.NewWatcher()
	if err != nil {
		fmt.Println("Error creating watcher:", err)
		return
	}
	defer watcher.Close()

	err = watcher.Add(MidiDir)
	if err != nil {
		fmt.Println("Error watching directory:", err)
		return
	}

	for {
		select {
		case event, ok := <-watcher.Events:
			if !ok {
				return
			}
			if event.Op&(fsnotify.Create|fsnotify.Remove) != 0 {
				fmt.Println("MIDI files updated, rescanning...")
				scanMidiFiles()
			}
		case err, ok := <-watcher.Errors:
			if !ok {
				return
			}
			fmt.Println("Watcher error:", err)
		}
	}
}

// Get list of MIDI files
func getMidiFiles(c *gin.Context) {
	db, _ := loadMetadata()
	c.JSON(200, db.Files)
}

// Get details of a single MIDI file
func getMidiFile(c *gin.Context) {
	fileID := c.Param("id")
	db, _ := loadMetadata()

	for _, file := range db.Files {
		if file.ID == fileID {
			c.JSON(200, file)
			return
		}
	}
	c.JSON(404, gin.H{"error": "File not found"})
}

// Update metadata of a MIDI file
func updateMidiMetadata(c *gin.Context) {
	fileID := c.Param("id")
	db, _ := loadMetadata()

	var metadata map[string]string
	if err := c.ShouldBindJSON(&metadata); err != nil {
		c.JSON(400, gin.H{"error": "Invalid JSON"})
		return
	}

	for i, file := range db.Files {
		if file.ID == fileID {
			for key, value := range metadata {
				db.Files[i].Metadata[key] = value
			}
			saveMetadata(db)
			c.JSON(200, db.Files[i])
			return
		}
	}
	c.JSON(404, gin.H{"error": "File not found"})
}

// Manual scan trigger
func triggerScan(c *gin.Context) {
	scanMidiFiles()
	c.JSON(200, gin.H{"message": "Scan complete"})
}

func main() {
	// Start scanning MIDI files
	scanMidiFiles()
	go watchMidiDir()

	// Setup router
	r := gin.Default()
	//r.Use(cors.Default())

	r.Use(cors.New(cors.Config{
		AllowOrigins:     []string{"http://localhost:5173"}, // Adjust to match your frontend
		AllowMethods:     []string{"GET", "PATCH"},
		AllowHeaders:     []string{"Content-Type"},
		AllowCredentials: true,
	}))

	r.GET("/api/midi-files", getMidiFiles)
	r.GET("/api/midi-files/:id", getMidiFile)
	r.PATCH("/api/midi-files/:id", updateMidiMetadata)
	r.POST("/api/scan", triggerScan)

	fmt.Println("Server running on port 8000")
	r.Run(":8000")
}
