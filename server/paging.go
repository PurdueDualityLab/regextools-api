package main

import (
	"errors"
	"github.com/patrickmn/go-cache"
	"hash/fnv"
	"log"
	"sort"
	"strconv"
	"time"
)

func HashQuery(request QueryRequest) string {
	hasher := fnv.New32a()

	// Sort both sets alphabetically
	positiveSorted := make([]string, len(request.Positive))
	negativeSorted := make([]string, len(request.Negative))

	copy(positiveSorted, request.Positive)
	copy(negativeSorted, request.Negative)

	sort.Strings(positiveSorted)
	sort.Strings(negativeSorted)

	for _, positive := range positiveSorted {
		_, err := hasher.Write([]byte(positive))
		if err != nil {
			return ""
		}
	}

	for _, negative := range negativeSorted {
		_, err := hasher.Write([]byte(negative))
		if err != nil {
			return ""
		}
	}

	keyVal := hasher.Sum32()
	return strconv.Itoa(int(keyVal))
}

type PageRequest struct {
	// What cache entry to look in
	CacheKey string `json:"cacheKey,omitempty"`
	// Which index of page you want
	PageNum uint64 `json:"pageNum"`
	// How many items should be on the page
	PageSize uint64 `json:"pageSize"`
}

type ResultTableEntry struct {
	Entities      []RegexEntity
	Scores        []float64
	TotalEntities uint
}

type ResultTable struct {
	cache *cache.Cache // The actual cache of items
}

func NewResultTable() *ResultTable {
	return &ResultTable{
		cache: cache.New(5*time.Minute, 10*time.Minute),
	}
}

func (tab *ResultTable) CacheResults(request QueryRequest, results []RegexEntity, scores []float64) string {
	// Create a new cache key
	cacheKey := HashQuery(request)
	if len(cacheKey) == 0 {
		// TODO do something. Panic?
		log.Fatalf("Failed while creating the cache key")
	}

	// Cache the result
	entry := ResultTableEntry{
		Entities:      results,
		Scores:        scores,
		TotalEntities: uint(len(results)),
	}
	tab.cache.Set(cacheKey, entry, cache.DefaultExpiration)

	return cacheKey
}

func (tab *ResultTable) FetchResults(key string) ([]RegexEntity, []float64, error) {
	if x, found := tab.cache.Get(key); found {
		result := x.(ResultTableEntry)
		return result.Entities, result.Scores, nil
	} else {
		return nil, nil, errors.New("no results with that key exist")
	}
}

func (tab *ResultTable) FetchResultsWithQuery(request QueryRequest) ([]RegexEntity, []float64, error) {
	key := HashQuery(request)
	if len(key) == 0 {
		return nil, nil, errors.New("could not create hash key")
	}

	return tab.FetchResults(key)
}

func (tab *ResultTable) FetchResultsWithPage(request PageRequest) ([]RegexEntity, []float64, uint64, uint64, error) {
	// Actually get the results
	results, scores, err := tab.FetchResults(request.CacheKey)
	if err != nil {
		return nil, nil, 0, 0, err
	}

	totalResults := uint64(len(results))

	// Divvy up the page
	var totalPageCount uint64
	if request.PageSize > 0 {
		totalPageCount = (totalResults / request.PageSize) + 1 // round up
	} else {
		// If the page size is 0, then just say there's one page?? that seems to make sense and
		// will eliminate divide by zero issues
		totalPageCount = 1
	}
	log.Printf("got %d pages", totalPageCount)
	if request.PageNum >= totalPageCount {
		return nil, nil, 0, 0, errors.New("invalid page request: bad page index")
	}

	// Get the bounds
	startIdx := request.PageNum * request.PageSize
	endIdx := startIdx + request.PageSize
	if endIdx >= totalResults {
		endIdx = totalResults - 1
	}

	// Page results
	pageResults := make([]RegexEntity, endIdx-startIdx)
	copy(pageResults, results[startIdx:endIdx])
	pageScores := make([]float64, endIdx-startIdx)
	copy(pageScores, scores[startIdx:endIdx])

	return pageResults, pageScores, totalPageCount, totalResults, nil
}

func (tab *ResultTable) InvalidateResults(key string) {
	tab.cache.Delete(key)
}
