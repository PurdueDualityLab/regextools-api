package main

import (
	"github.com/regextools/protos/query_service"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type RegexDBClient struct {
	Context    context.Context
	Connection *grpc.ClientConn
	Client     query_service.QueryServiceClient
}

func NewRegexDBClient(ctx context.Context, dest string) (*RegexDBClient, error) {
	conn, err := grpc.Dial(dest, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		return nil, err
	}
	return &RegexDBClient{
		Context:    ctx,
		Connection: conn,
		Client:     query_service.NewQueryServiceClient(conn),
	}, nil
}

func (conn RegexDBClient) Query(positives, negatives []string) (*query_service.QueryResults, error) {
	query := query_service.Query{
		Positive: positives,
		Negative: negatives,
	}

	return conn.Client.ExecQuery(conn.Context, &query)
}

func (conn RegexDBClient) Close() error {
	err := conn.Connection.Close()
	if err != nil {
		return err
	} else {
		return nil
	}
}
