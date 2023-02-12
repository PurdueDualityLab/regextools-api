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

type ResultTable struct {
	cache *cache.Cache // The actual cache of items
}

func NewResultTable() *ResultTable {
	return &ResultTable{
		cache: cache.New(5*time.Minute, 10*time.Minute),
	}
}

func (tab *ResultTable) CacheResults(request QueryRequest, results []RegexEntity) string {
	// Create a new cache key
	cacheKey := HashQuery(request)
	if len(cacheKey) == 0 {
		// TODO do something. Panic?
		log.Fatalf("Failed while creating the cache key")
	}

	// Cache the result
	tab.cache.Set(cacheKey, results, cache.DefaultExpiration)

	return cacheKey
}

func (tab *ResultTable) FetchResults(key string) ([]RegexEntity, error) {
	if x, found := tab.cache.Get(key); found {
		result := x.([]RegexEntity)
		return result, nil
	} else {
		return nil, errors.New("no results with that key exist")
	}
}

func (tab *ResultTable) FetchResultsWithQuery(request QueryRequest) ([]RegexEntity, error) {
	key := HashQuery(request)
	if len(key) == 0 {
		return nil, errors.New("could not create hash key")
	}

	return tab.FetchResults(key)
}

func (tab *ResultTable) FetchResultsWithPage(request PageRequest) ([]RegexEntity, uint64, error) {
	// Actually get the results
	results, err := tab.FetchResults(request.CacheKey)
	if err != nil {
		return nil, 0, err
	}

	totalResults := uint64(len(results))

	// Divvy up the page
	totalPageCount := (totalResults / request.PageSize) + 1 // round up
	log.Printf("got %d pages", totalPageCount)
	if request.PageNum >= totalPageCount {
		return nil, 0, errors.New("invalid page request: bad page index")
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

	return pageResults, totalPageCount, nil
}

func (tab *ResultTable) InvalidateResults(key string) {
	tab.cache.Delete(key)
}
