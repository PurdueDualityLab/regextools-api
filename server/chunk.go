package main

// ChunkSlice - break a slice into chunks of chunkSize
func ChunkSlice[ElementType any](slice []ElementType, chunkSize int) [][]ElementType {
	var chunks [][]ElementType
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
