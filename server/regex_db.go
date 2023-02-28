package main

import (
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/dynamodb"
	"github.com/aws/aws-sdk-go/service/dynamodb/dynamodbattribute"
	"sort"
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
	ForumLocations  []RegexForumLocation  `json:"forumLocations"`
	Info            RegexInfo             `json:"info"`
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

// GetRegexesById - inflates regex entities by id. Size can be whatever
func (repo RegexEntityRepository) GetRegexesById(ids []string) ([]RegexEntity, error) {
	var totalEntities []RegexEntity

	// If there are more than 100 results, we need to chunk the results
	idChunks := [][]string{ids}
	if len(ids) > 100 {
		idChunks = ChunkSlice(ids, 100)
	}

	for _, idChunk := range idChunks {
		entities, err := repo.getRegexesByIdUncheckedSize(idChunk)
		if err != nil {
			return []RegexEntity{}, err
		}

		for _, entity := range entities {
			totalEntities = append(totalEntities, entity)
		}
	}

	return totalEntities, nil
}

// getRegexesByIdUncheckedSize - actually performs the request, but doesn't do any chunking
func (repo RegexEntityRepository) getRegexesByIdUncheckedSize(ids []string) ([]RegexEntity, error) {

	// Set up the request map
	var requestIds []map[string]*dynamodb.AttributeValue
	for idx := range ids {
		newRequestId := map[string]*dynamodb.AttributeValue{
			"id": {S: &ids[idx]},
		}
		requestIds = append(requestIds, newRequestId)
	}

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

	// Sort the entities by score
	sort.Slice(entities, func(i, j int) bool {
		return entities[i].Info.Score < entities[j].Info.Score
	})

	return entities, nil
}
