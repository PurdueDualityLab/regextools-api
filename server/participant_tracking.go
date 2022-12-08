package main

import (
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/dynamodb"
	"github.com/aws/aws-sdk-go/service/dynamodb/dynamodbattribute"
	"github.com/google/uuid"
	"log"
	"time"
)

type ParticipantQuery struct {
	Id            string   `json:"id"`
	ParticipantId string   `json:"participantId"`
	TaskId        uint32   `json:"taskId"`
	Positive      []string `json:"positive"`
	Negative      []string `json:"negative"`
	Time          string   `json:"time"`
}

type ParticipantTracker struct {
	client    *dynamodb.DynamoDB
	tableName string
}

func NewParticipantTracker() *ParticipantTracker {
	sess := session.Must(session.NewSessionWithOptions(session.Options{
		SharedConfigState: session.SharedConfigEnable,
	}))

	client := dynamodb.New(sess)

	return &ParticipantTracker{
		client:    client,
		tableName: "regextools-participant-tracking", // TODO make this configurable
	}
}

func (tracker *ParticipantTracker) StoreQuery(participantId string, taskId uint32, request QueryRequest) error {
	// Make the actual item
	item := ParticipantQuery{
		Id:            uuid.New().String(), // TODO create a unique ID
		ParticipantId: participantId,
		TaskId:        taskId,
		Positive:      request.Positive,
		Negative:      request.Negative,
		Time:          time.Now().Format(time.RFC3339), // ISO timestamp
	}

	// Marshall to AV map
	av, err := dynamodbattribute.MarshalMap(&item)
	if err != nil {
		return err
	}

	// Make a put request
	input := dynamodb.PutItemInput{
		Item:      av,
		TableName: &tracker.tableName,
	}
	_, err = tracker.client.PutItem(&input)

	log.Printf("Tracked query for participantId=%s,taskId=%d", participantId, taskId)

	return err
}
