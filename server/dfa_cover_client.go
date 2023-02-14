package main

import (
	dfa_cover_service "github.com/regextools/protos/dfa_cover_serivce"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"sort"
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
func (conn DfaCoverageClient) SortByCoverage(entities []RegexEntity) ([]RegexEntity, error) {

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

	request := dfa_cover_service.DfaCoverageRequest{Entities: coverageEntities}

	response, err := conn.Client.ExecDfaCoverage(conn.Context, &request)
	if err != nil {
		return []RegexEntity{}, err
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

		// Both succeeded, so compare the scores
		return left.Score > right.Score
	})

	var sortedEntities []RegexEntity
	for _, info := range sortedCoverageInfo {
		sortedEntities = append(sortedEntities, entityMap[info.Id])
	}

	return sortedEntities, nil
}

func (conn DfaCoverageClient) Close() error {
	return conn.Connection.Close()
}
