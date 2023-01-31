package main

import (
	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
	"golang.org/x/net/context"
	"log"
)

func main() {
	var err error

	ctx := context.Background()
	resultTable := NewResultTable()
	tracker := NewParticipantTracker()

	repo := NewRegexEntityRepository()

	router := gin.Default()
	router.Use(cors.Default())
	router.POST("/query", QueryHandler(ctx, resultTable, tracker, repo))

	err = router.Run()
	if err != nil {
		log.Fatalf("Error while running server: %v", err)
	}
}
