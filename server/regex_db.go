package main

import (
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/dynamodb"
	"github.com/aws/aws-sdk-go/service/dynamodb/dynamodbattribute"
	"log"
)

// RegexSourceLocation - models the usage of a regex somewhere in a public repository
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
}

type RegexEntityRepository struct {
	client    *dynamodb.DynamoDB
	tableName string
}

func NewRegexEntityRepository() *RegexEntityRepository {
	sess := session.Must(session.NewSessionWithOptions(session.Options{
		SharedConfigState: session.SharedConfigEnable,
	}))

	client := dynamodb.New(sess)

	return &RegexEntityRepository{
		client:    client,
		tableName: "regextools-regex-entity-db", // TODO make this configurable
	}
}

func (repo *RegexEntityRepository) GetRegexesById(ids []string) ([]RegexEntity, error) {

	log.Printf("%v\n", ids)

	// Set up the request map
	var requestIds []map[string]*dynamodb.AttributeValue
	for idx := range ids {
		newRequestId := map[string]*dynamodb.AttributeValue{
			"id": {S: &ids[idx]},
		}
		requestIds = append(requestIds, newRequestId)
	}

	log.Printf("%v\n", requestIds)

	// Build input
	input := dynamodb.BatchGetItemInput{
		RequestItems: map[string]*dynamodb.KeysAndAttributes{
			repo.tableName: {
				Keys: requestIds,
			},
		},
	}

	// Perform the request
	result, err := repo.client.BatchGetItem(&input)
	if err != nil {
		return []RegexEntity{}, err
	}

	var entities []RegexEntity
	err = dynamodbattribute.UnmarshalListOfMaps(result.Responses[repo.tableName], &entities)
	if err != nil {
		return []RegexEntity{}, nil
	}

	return entities, nil
}
