package main

import (
	"fmt"
	dfa_cover_service "github.com/regextools/protos/dfa_cover_serivce"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"os"
	"sort"
	"strconv"
)

type DfaCoverageClient struct {
	Context    context.Context
	Connection *grpc.ClientConn
	Client     dfa_cover_service.DfaCoverageServiceClient
}

func NewDfaCoverageClient(ctx context.Context, dest string) (*DfaCoverageClient, error) {
	conn, err := grpc.Dial(dest, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		return nil, err
	}
	return &DfaCoverageClient{
		Context:    ctx,
		Connection: conn,
		Client:     dfa_cover_service.NewDfaCoverageServiceClient(conn),
	}, nil
}

// SortByCoverage - given a list of entities, orders them based on DFA coverage
// entities that error out are at the bottom of the list
func (conn DfaCoverageClient) SortByCoverage(entities []RegexEntity, positive, negative []string) ([]RegexEntity, []float64, error) {

	entityMap := make(map[string]RegexEntity)
	for _, entity := range entities {
		entityMap[entity.Id] = entity
	}

	var coverageEntities []*dfa_cover_service.DfaCoverageRegexEntity
	for id, entity := range entityMap {
		covEnt := &dfa_cover_service.DfaCoverageRegexEntity{
			Pattern: entity.Pattern,
			Id:      id,
		}
		coverageEntities = append(coverageEntities, covEnt)
	}

	request := dfa_cover_service.DfaCoverageRequest{
		Entities: coverageEntities,
		Positive: positive,
		Negative: negative,
	}

	response, err := conn.Client.ExecDfaCoverage(conn.Context, &request)
	if err != nil {
		return []RegexEntity{}, []float64{}, err
	}

	// Make a quick lil map for string the scores. After we sort, we can return an array of scores and send those over
	scoreMap := make(map[string]float64)
	for _, info := range response.CoverageInfo {
		scoreMap[info.Id] = info.Score
	}

	sortedCoverageInfo := response.CoverageInfo
	sort.Slice(sortedCoverageInfo, func(leftIdx, rightIdx int) bool {
		left := sortedCoverageInfo[leftIdx]
		right := sortedCoverageInfo[rightIdx]

		// Both failed, who cares
		if !left.Success && !right.Success {
			return true
		}

		// Left passes, right fails, take left
		if left.Success && !right.Success {
			return true
		}

		if !left.Success && right.Success {
			return false
		}

		// Both succeeded. Tie-break with feature count. Take fewer features
		// TODO - use the score in the future. It's not quite ready yet as of now...
		if left.Score == right.Score {
			return entityMap[left.Id].Info.FeatureCount < entityMap[right.Id].Info.FeatureCount
		} else {
			return left.Score > right.Score
		}
	})

	var sortedEntities []RegexEntity
	for _, info := range sortedCoverageInfo {
		sortedEntities = append(sortedEntities, entityMap[info.Id])
	}

	var scores []float64
	for _, entity := range sortedEntities {
		scores = append(scores, scoreMap[entity.Id])
	}

	return sortedEntities, scores, nil
}

func (conn DfaCoverageClient) GetSingleCoverage(regex string, positive, negative []string) (*dfa_cover_service.DfaSingleCoverageResponse, error) {
	request := dfa_cover_service.DfaSingleCoverageRequest{
		Regex:    regex,
		Positive: positive,
		Negative: negative,
	}

	return conn.Client.ExecDfaSingleCoverage(conn.Context, &request)
}

func (conn DfaCoverageClient) Close() error {
	return conn.Connection.Close()
}

func GetDfaCoverageString(defaultHost string, defaultPort int) (string, error) {
	var err error
	envHost := os.Getenv("DFA_COVER_HOST")
	if len(envHost) == 0 {
		// host is not set, so use default
		envHost = defaultHost
	}

	envPortStr := os.Getenv("DFA_COVER_PORT")
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
