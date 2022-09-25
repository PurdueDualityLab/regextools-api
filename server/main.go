package main

import (
	"github.com/gin-gonic/gin"
	"golang.org/x/net/context"
	"log"
	"github.com/gin-contrib/cors"
)

func main() {
	var err error

	ctx := context.Background()

	router := gin.Default()
	router.Use(cors.Default())
	router.POST("/query", QueryHandler(ctx))

	err = router.Run()
	if err != nil {
		log.Fatalf("Error while running server: %v", err)
	}
}
