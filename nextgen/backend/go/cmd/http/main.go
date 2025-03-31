package main

import (
	"fmt"
	"github.com/gin-gonic/gin"
	"github.com/noodnik2/mzb/nextgen/backend/go/internal/model"
)

type serverState struct {
	dict model.MetaDataDict
}

func main() {

	//var cfg config.Config
	ss := serverState{
		dict: model.MetaDataDict{
			"one": "ordinal",
		},
	}

	// Setup router
	r := gin.Default()
	//r.Use(cors.Default())

	//r.Use(cors.New(cors.Config{
	//	AllowOrigins:     []string{"http://localhost:5173"}, // Adjust to match your frontend
	//	AllowMethods:     []string{"GET", "PATCH"},
	//	AllowHeaders:     []string{"Content-Type"},
	//	AllowCredentials: true,
	//}))

	//r.GET("/api/midi-files", ss.getMidiFiles)
	r.GET("midi-files", ss.getMidiFiles)
	//r.GET("/api/midi-files/:id", getMidiFile)
	//r.PATCH("/api/midi-files/:id", updateMidiMetadata)
	//r.POST("/api/scan", triggerScan)

	fmt.Printf("Server running on port 8000\n")
	err := r.Run(":8000")
	if err != nil {
		fmt.Printf("error: %q from http handler\n", err)

		return
	}

	fmt.Printf("server exited normally")
}

// Get list of MIDI files
func (ss *serverState) getMidiFiles(c *gin.Context) {
	files := []model.MidiFile{
		{
			Id:   0,
			Path: "/one/two.mid",
			MetaDataRepo: model.MetaDataRepo{
				"one": 1.25,
			},
		},
	}
	c.JSON(200, files)
}
