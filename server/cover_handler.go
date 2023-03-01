package main

import (
	"github.com/gin-gonic/gin"
	"golang.org/x/net/context"
	"net/http"
)

type CoverRequest struct {
	Regex    string   `json:"regex"`
	Positive []string `json:"positive"`
	Negative []string `json:"negative"`
}

type CoverResponse struct {
	Total    float64 `json:"total"`
	Positive float64 `json:"positive"`
	Negative float64 `json:"negative"`
}

func CoverHandler(netCtx context.Context) gin.HandlerFunc {

	fn := func(ctx *gin.Context) {

		var request CoverRequest
		if err := ctx.ShouldBindJSON(&request); err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
			return
		}

		coverConnStr, err := GetDfaCoverageString("0.0.0.0", 50052)
		if err != nil {
			ctx.JSON(http.StatusInternalServerError, gin.H{"msg": err.Error()})
			return
		}

		// Sort by coverage metric
		// TODO sorting should still happen after inflation
		coverClient, err := NewDfaCoverageClient(netCtx, coverConnStr)
		if err != nil {
			ctx.JSON(http.StatusInternalServerError, gin.H{"msg": "Could not create regex coverage client"})
			return
		}
		defer coverClient.Close()

		coverageInfo, err := coverClient.GetSingleCoverage(request.Regex, request.Positive, request.Negative)
		if err != nil {
			ctx.JSON(http.StatusInternalServerError, gin.H{"msg": "Error while getting coverage info", "error": err.Error()})
			return
		}

		response := CoverResponse{
			Total:    float64(coverageInfo.Total),
			Positive: float64(coverageInfo.Positive),
			Negative: float64(coverageInfo.Negative),
		}

		ctx.JSON(http.StatusOK, response)
	}

	return fn
}
