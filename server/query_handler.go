package main

import (
	"fmt"
	"github.com/gin-gonic/gin"
	"golang.org/x/net/context"
	"net/http"
	"os"
	"strconv"
)

type QueryRequest struct {
	Positive []string `json:"positive"`
	Negative []string `json:"negative"`
}

func getRegexDBString(defaultHost string, defaultPort int) (string, error) {
	var err error
	envHost := os.Getenv("REGEXDB_HOST")
	if len(envHost) == 0 {
		// host is not set, so use default
		envHost = defaultHost
	}

	envPortStr := os.Getenv("REGEXDB_PORT")
	var envPort int
	if len(envPortStr) == 0 {
		envPort = defaultPort
	} else {
		envPort, err = strconv.Atoi(envPortStr)
		if err != nil {
			return "", err
		}
	}

	return fmt.Sprintf("%s:%d", envHost, envPort), nil
}

func QueryHandler(netCtx context.Context) gin.HandlerFunc {
	fn := func(ctx *gin.Context) {
		connStr, err := getRegexDBString("0.0.0.0", 50051)
		if err != nil {
			ctx.JSON(http.StatusInternalServerError, gin.H{"msg": err.Error()})
			return
		}

		client, err := NewRegexDBClient(netCtx, connStr)
		if err != nil {
			ctx.JSON(http.StatusInternalServerError, gin.H{"msg": "Could not create regex client database"})
			return
		}
		defer client.Close()

		var request QueryRequest
		if err = ctx.ShouldBindJSON(&request); err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
			return
		}

		results, err := client.Query(request.Positive, request.Negative)
		if err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
			return
		}

		if results.Total == 0 {
			ctx.JSON(http.StatusOK, gin.H{"results": []string{}, "total": 0})
			return
		}

		ctx.JSON(http.StatusOK, results)
	}

	return fn
}
