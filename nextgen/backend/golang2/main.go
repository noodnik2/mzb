package main

import (
	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
	"github.com/jinzhu/gorm"
	_ "github.com/jinzhu/gorm/dialects/sqlite"
	"net/http"
	"strconv"
)

type Item struct {
	ID       uint       `json:"id"`
	Name     string     `json:"name"`
	Metadata []Metadata `json:"metadata"`
}

type Metadata struct {
	ID     uint   `json:"id"`
	Key    string `json:"key"`
	Value  string `json:"value"`
	Type   string `json:"type"` // 'ordinal' or 'categorical'
	ItemID uint   `json:"item_id"`
}

var db *gorm.DB
var err error

func main() {
	// Setup CORS
	router := gin.Default()
	router.Use(cors.Default())

	// Initialize database
	db, err = gorm.Open("sqlite3", "./items.db")
	if err != nil {
		panic("failed to connect to database")
	}
	defer db.Close()

	// Migrate schema
	db.AutoMigrate(&Item{}, &Metadata{})

	// Routes
	router.GET("/items", getItems)
	router.GET("/item/:id", getItem)
	router.PATCH("/item/:id", updateItem)
	router.POST("/item", createItem)
	router.POST("/metadata/:item_id", addMetadata)

	router.Run(":8080")
}

// Get all items
func getItems(c *gin.Context) {
	var items []Item
	db.Preload("Metadata").Find(&items)
	c.JSON(http.StatusOK, items)
}

// Get a specific item by ID
func getItem(c *gin.Context) {
	id := c.Param("id")
	var item Item
	if err := db.Preload("Metadata").First(&item, id).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "Item not found"})
		return
	}
	c.JSON(http.StatusOK, item)
}

// Create a new item
func createItem(c *gin.Context) {
	var item Item
	if err := c.ShouldBindJSON(&item); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	db.Create(&item)
	c.JSON(http.StatusCreated, item)
}

// Update an item
func updateItem(c *gin.Context) {
	id := c.Param("id")
	var item Item
	if err := db.First(&item, id).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "Item not found"})
		return
	}
	if err := c.ShouldBindJSON(&item); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	db.Save(&item)
	c.JSON(http.StatusOK, item)
}

// Add metadata to an item
func addMetadata(c *gin.Context) {
	itemID := c.Param("item_id")
	var metadata Metadata
	if err := c.ShouldBindJSON(&metadata); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	atoi, err2 := strconv.Atoi(itemID)
	if err2 != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	metadata.ItemID = uint(atoi)
	db.Create(&metadata)
	c.JSON(http.StatusCreated, metadata)
}
