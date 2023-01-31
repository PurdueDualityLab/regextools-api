package main

import (
	"fmt"
	"github.com/gin-gonic/gin"
	"github.com/regextools/protos/query_service"
	"golang.org/x/net/context"
	"net/http"
	"os"
	"strconv"
)

type QueryRequest struct {
	Positive    []string     `json:"positive"`
	Negative    []string     `json:"negative"`
	PageRequest *PageRequest `json:"pageRequest,omitempty"`
}

type QueryResponse struct {
	Results   []string `json:"results"`
	Total     uint64   `json:"total"`
	CacheKey  string   `json:"cacheKey"`
	PageSize  uint64   `json:"pageSize"`
	PageNum   uint64   `json:"pageNum"`
	PageCount uint64   `json:"pageCount"`
}

func makeQueryResponse(results *query_service.QueryResults, cacheKey string, request QueryRequest, pageCount uint64) QueryResponse {
	if request.PageRequest != nil {
		return QueryResponse{
			Results:   results.Results,
			Total:     results.Total,
			PageSize:  request.PageRequest.PageSize,
			PageNum:   request.PageRequest.PageNum,
			CacheKey:  cacheKey,
			PageCount: pageCount,
		}
	} else {
		return QueryResponse{
			Results:   results.Results,
			Total:     results.Total,
			PageSize:  results.Total,
			PageNum:   0,
			CacheKey:  cacheKey,
			PageCount: 1,
		}
	}
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

func shouldTrack(ctx *gin.Context) (string, string, bool) {
	participantId, hasParticipantId := ctx.GetQuery("participantId")
	taskId, hasTaskId := ctx.GetQuery("taskId")

	return participantId, taskId, hasParticipantId && hasTaskId
}

func QueryHandler(netCtx context.Context, resultTable *ResultTable, tracker *ParticipantTracker, regexRepo *RegexEntityRepository) gin.HandlerFunc {

	fn := func(ctx *gin.Context) {

		var request QueryRequest
		if err := ctx.ShouldBindJSON(&request); err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
			return
		}

		// If a cache key is provided, we need to do some stuff
		if request.PageRequest != nil {
			if len(request.PageRequest.CacheKey) == 0 {
				// do nothing. We still have the paging stuff here so that we can still page off the bat
			} else if HashQuery(request) != request.PageRequest.CacheKey && len(request.Positive) > 0 && len(request.Negative) > 0 {
				// If the request and the key are different, we should invalidate the old result
				resultTable.InvalidateResults(request.PageRequest.CacheKey)
				// keep going -- don't return
			} else {
				// The query has not changed, so we can get these results
				results, pageCount, err := resultTable.FetchResultsWithPage(*request.PageRequest)
				if err != nil {
					// TODO error handling
					ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
					return
				}

				response := makeQueryResponse(results, request.PageRequest.CacheKey, request, pageCount)
				ctx.JSON(http.StatusOK, response)
				return
			}
		}

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

		results, err := client.Query(request.Positive, request.Negative)
		if err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
			return
		}

		// Inflate the matching regexes
		// TODO
		_, err = regexRepo.GetRegexesById([]string{})

		// Cache the results
		cacheKey := resultTable.CacheResults(request, results)

		// If there is a page request, fulfill that
		var totalPages uint64 = 1
		if request.PageRequest != nil {
			request.PageRequest.CacheKey = cacheKey

			results, totalPages, err = resultTable.FetchResultsWithPage(*request.PageRequest)
			if err != nil {
				ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
				return
			}
		}

		// cache the results
		response := makeQueryResponse(results, cacheKey, request, totalPages)

		// track the response if there are query parameters
		if participantId, taskId, areSet := shouldTrack(ctx); areSet {
			taskIdNum, err := strconv.Atoi(taskId)
			if err != nil {
				// TODO handle error
				ctx.JSON(http.StatusBadRequest, gin.H{"msg": "taskId must be a valid number"})
				return
			}
			err = tracker.StoreQuery(participantId, uint32(taskIdNum), request)
			if err != nil {
				ctx.JSON(http.StatusInternalServerError, gin.H{"msg": "Error while tracking participant info: " + err.Error()})
				return
			}
		}

		ctx.JSON(http.StatusOK, response)
	}

	return fn
}
