package main

import (
	"fmt"
	"github.com/gin-gonic/gin"
	"golang.org/x/net/context"
	"log"
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
	Results    []RegexEntity `json:"results"`
	SortScores []float64     `json:"sortScores"`
	Total      uint64        `json:"total"`
	CacheKey   string        `json:"cacheKey"`
	PageSize   uint64        `json:"pageSize"`
	PageNum    uint64        `json:"pageNum"`
	PageCount  uint64        `json:"pageCount"`
}

// TODO wrap some of the args into a struct
func makeQueryResponse(results []RegexEntity, scores []float64, totalElements uint64, cacheKey string, request QueryRequest, pageCount uint64) QueryResponse {
	if request.PageRequest != nil {
		return QueryResponse{
			Results:    results,
			SortScores: scores,
			Total:      totalElements,
			PageSize:   request.PageRequest.PageSize,
			PageNum:    request.PageRequest.PageNum,
			CacheKey:   cacheKey,
			PageCount:  pageCount,
		}
	} else {
		return QueryResponse{
			Results:    results,
			SortScores: scores,
			Total:      totalElements,
			PageSize:   uint64(len(results)),
			PageNum:    0,
			CacheKey:   cacheKey,
			PageCount:  1,
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

// TODO return a struct instead...
func tryCachedEntry(request QueryRequest, resultTable *ResultTable) ([]RegexEntity, []float64, uint64, uint64, bool, error) {
	if request.PageRequest != nil {
		if len(request.PageRequest.CacheKey) == 0 {
			// do nothing. We still have the paging stuff here so that we can still page off the bat
			return []RegexEntity{}, nil, 0, 0, false, nil
		} else if HashQuery(request) != request.PageRequest.CacheKey && len(request.Positive) > 0 && len(request.Negative) > 0 {
			// If the request and the key are different, we should invalidate the old result
			resultTable.InvalidateResults(request.PageRequest.CacheKey)
			// keep going -- don't return
			return []RegexEntity{}, nil, 0, 0, false, nil
		} else {
			// The query has not changed, so we can get these results
			results, resultScores, pageCount, totalElements, err := resultTable.FetchResultsWithPage(*request.PageRequest)
			if err != nil {
				// TODO error handling
				return []RegexEntity{}, nil, 0, 0, false, err
			}

			return results, resultScores, pageCount, totalElements, true, nil
		}
	} else {
		// we don't even want to page...
		return []RegexEntity{}, nil, 0, 0, false, nil
	}
}

func QueryHandler(netCtx context.Context, resultTable *ResultTable, tracker *ParticipantTracker, regexRepo *RegexEntityRepository) gin.HandlerFunc {

	fn := func(ctx *gin.Context) {

		var request QueryRequest
		if err := ctx.ShouldBindJSON(&request); err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
			return
		}

		/*
			First, try finding a cached entry
		*/
		cachedResults, cachedScores, cachedTotalPageCount, cachedTotalElements, didFindEntry, err := tryCachedEntry(request, resultTable)
		if err != nil {
			ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
			return
		}

		if didFindEntry {
			response := makeQueryResponse(cachedResults, cachedScores, cachedTotalElements, request.PageRequest.CacheKey, request, cachedTotalPageCount)
			ctx.JSON(http.StatusOK, response)
			return
		}

		/*
			We couldn't find a cached entry, so actually perform the query
		*/

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

		// Get the ids for the results
		var resultIds []string
		for _, result := range results.Results {
			resultIds = append(resultIds, result.Id)
		}

		log.Printf("query_handler: got %d results from regexdb\n", len(resultIds))

		// Inflate the matching regexes
		// TODO only inflate the page, not the whole payload
		inflatedResults, err := regexRepo.GetRegexesById(resultIds)
		if err != nil {
			ctx.JSON(http.StatusInternalServerError, gin.H{"msg": "Failed to inflate regex entities"})
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

		var sortingScores []float64
		inflatedResults, sortingScores, err = coverClient.SortByCoverage(inflatedResults, request.Positive, request.Negative)
		if err != nil {
			ctx.JSON(http.StatusInternalServerError, gin.H{"msg": fmt.Sprintf("Error while sorting by coverage: %s", err.Error())})
			return
		}

		// Cache the results
		cacheKey := resultTable.CacheResults(request, inflatedResults, sortingScores)

		// If there is a page request, fulfill that
		var totalPages uint64 = 1
		var totalElements uint64 = 0
		if request.PageRequest != nil {
			request.PageRequest.CacheKey = cacheKey

			inflatedResults, sortingScores, totalPages, totalElements, err = resultTable.FetchResultsWithPage(*request.PageRequest)
			if err != nil {
				ctx.JSON(http.StatusBadRequest, gin.H{"msg": err.Error()})
				return
			}
		}

		// cache the results
		response := makeQueryResponse(inflatedResults, sortingScores, totalElements, cacheKey, request, totalPages)

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
