package main

import (
	"github.com/gin-gonic/gin"
	"golang.org/x/net/context"
	"net/http"
)

type QueryRequest struct {
	Positive []string `json:"positive"`
	Negative []string `json:"negative"`
}

func QueryHandler(netCtx context.Context) gin.HandlerFunc {
	fn := func(ctx *gin.Context) {
		client, err := NewRegexDBClient(netCtx, "0.0.0.0:50051")
		if err != nil {
			ctx.JSON(http.StatusInternalServerError, gin.H{"msg": "Could not create regex client database"})
		}
		defer client.Close()

		var request QueryRequest
		if err = ctx.ShouldBindJSON(&request); err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
		}

		results, err := client.Query(request.Positive, request.Negative)
		if err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
		}

		ctx.JSON(http.StatusOK, results)
	}

	return fn
}
