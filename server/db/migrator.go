package main

import (
	"encoding/json"
	"fmt"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/dynamodb"
	"github.com/aws/aws-sdk-go/service/dynamodb/dynamodbattribute"
	"io"
	"log"
	"os"
	"strconv"
)

type RegexForumLocation struct {
	Type string `json:"type"`
	URI  string `json:"uri"`
}

type RegexInfo struct {
	FeatureVector map[string]int `json:"featureVector"`
	Length        int            `json:"length"`
	FeatureCount  int            `json:"featureCount"`
	Score         float32        `json:"score"`
}

// OldRegexSourceLocation - models the usage of a regex somewhere in a public repository
type OldRegexSourceLocation struct {
	Commit       string `json:"commit"`
	LineNumber   string `json:"lineNumber"`
	RepoLocation string `json:"repoLocation"`
	SourceFile   string `json:"sourceFile"`
	License      string `json:"license"`
}

// OldRegexEntity - represents a single regex
type OldRegexEntity struct {
	Id              string                   `json:"id"`
	Pattern         string                   `json:"pattern"`
	SourceLocations []OldRegexSourceLocation `json:"sourceLocations"`
	ForumLocations  []RegexForumLocation     `json:"forumLocations"`
	Info            RegexInfo                `json:"info"`
}

type RegexSourceLocation struct {
	Commit       string `json:"commit"`
	LineNumber   uint32 `json:"lineNumber"`
	RepoLocation string `json:"repoLocation"`
	SourceFile   string `json:"sourceFile"`
	License      string `json:"license"`
}

// RegexEntity - represents a single regex
type RegexEntity struct {
	Id              string                `json:"id"`
	Pattern         string                `json:"pattern"`
	SourceLocations []RegexSourceLocation `json:"sourceLocations"`
	ForumLocations  []RegexForumLocation  `json:"forumLocations"`
	Info            RegexInfo             `json:"info"`
}

func chunkSlice(slice []*dynamodb.WriteRequest, chunkSize int) [][]*dynamodb.WriteRequest {
	var chunks [][]*dynamodb.WriteRequest
	for i := 0; i < len(slice); i += chunkSize {
		end := i + chunkSize

		// necessary check to avoid slicing beyond
		// slice capacity
		if end > len(slice) {
			end = len(slice)
		}

		chunks = append(chunks, slice[i:end])
	}

	return chunks
}

func truncateList(items []RegexForumLocation, maxSize int) []RegexForumLocation {

	// Take the min between maxSize and items length
	if maxSize > len(items) {
		maxSize = len(items)
	}

	return items[0:maxSize]
}

func main() {
	if len(os.Args) < 2 {
		log.Fatal("usage: tool <input file>")
	}

	inputFilePath := os.Args[1]
	readFile, err := os.Open(inputFilePath)
	if err != nil {
		log.Fatalf("Error while opening input file: %s\n", err.Error())
	}
	fileContent, err := io.ReadAll(readFile)
	var entities []OldRegexEntity
	err = json.Unmarshal(fileContent, &entities)
	if err != nil {
		log.Fatalf("Failed to parse input: %s\n", err.Error())
	}

	log.Printf("Got %d entities\n", len(entities))

	// fix up the source line information
	var fixedEntities []RegexEntity
	for _, entity := range entities {
		var fixedLocations []RegexSourceLocation
		for _, loc := range entity.SourceLocations {
			fixed, _ := strconv.Atoi(loc.LineNumber)
			fixedLocations = append(fixedLocations, RegexSourceLocation{
				Commit:       loc.Commit,
				LineNumber:   uint32(fixed),
				RepoLocation: loc.RepoLocation,
				SourceFile:   loc.SourceFile,
				License:      loc.License,
			})
		}

		fixedEntities = append(fixedEntities, RegexEntity{
			Id:              entity.Id,
			Pattern:         entity.Pattern,
			SourceLocations: fixedLocations,
			// TODO this is arbitrary but necessary for now
			ForumLocations: truncateList(entity.ForumLocations, 15),
			Info:           entity.Info,
		})
	}

	fmt.Printf("%v\n", fixedEntities[0])

	var createEntityRequests []*dynamodb.WriteRequest
	for _, entity := range fixedEntities {
		av, err := dynamodbattribute.MarshalMap(&entity)
		if err != nil {
			log.Fatalf("Error while marshalling entities: %s\n", err.Error())
		}

		req := dynamodb.WriteRequest{
			PutRequest: &dynamodb.PutRequest{
				Item: av,
			},
		}
		createEntityRequests = append(createEntityRequests, &req)
	}

	sess := session.Must(session.NewSessionWithOptions(session.Options{
		SharedConfigState: session.SharedConfigEnable,
	}))

	client := dynamodb.New(sess)

	for _, req := range chunkSlice(createEntityRequests, 24) {
		input := dynamodb.BatchWriteItemInput{
			RequestItems: map[string][]*dynamodb.WriteRequest{
				"regextools-regex-entity-db": req,
			},
		}

		_, err = client.BatchWriteItem(&input)
		if err != nil {
			log.Printf("Failed entity: %v\n", req)
			log.Fatalf("Error while performing write: %s\n", err.Error())
		}

		log.Println("Successful")
	}
}
