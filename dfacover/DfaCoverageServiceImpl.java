package dfacover;

import dk.brics.automaton.BasicOperations;
import dk.brics.automaton.DfaBudgetExceededException;
import io.grpc.stub.StreamObserver;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import regextools.DfaCoverService;
import regextools.DfaCoverageServiceGrpc;

import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

public class DfaCoverageServiceImpl extends DfaCoverageServiceGrpc.DfaCoverageServiceImplBase {

    private static final Logger logger = LoggerFactory.getLogger(DfaCoverageServiceImpl.class);

    public DfaCoverageServiceImpl() {
        super();
        // TODO this is going to have to get worked around
        BasicOperations.setDefaultDfaBudget(2_000);
    }

    @Override
    public void execDfaCoverage(DfaCoverService.DfaCoverageRequest request, StreamObserver<DfaCoverService.DfaCoverageResponse> responseObserver) {

        logger.info("NOP scorer");

        // basically just NOP for testing
        List<DfaCoverService.DfaCoverageInfo> coverageInfoList = request.getEntitiesList().stream()
                .map(entity -> {
                    double score = 0;
                    boolean success = false;
                    try {
                        score = computeCoverageScore(entity, request.getPositiveList(), request.getNegativeList());
                        success = true;
                    } catch (DfaBudgetExceededException ignored) {
                    }

                    return DfaCoverService.DfaCoverageInfo.newBuilder()
                            .setId(entity.getId())
                            .setSuccess(success)
                            .setScore(score)
                            .build();
                })
                .collect(Collectors.toList());

        // TODO build this out
        DfaCoverService.DfaCoverageResponse response = DfaCoverService.DfaCoverageResponse.newBuilder()
                .addAllCoverageInfo(coverageInfoList)
                .build();

        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }

    private double computeCoverageScore(DfaCoverService.DfaCoverageRegexEntity entity, Collection<String> positiveExamples, Collection<String> negativeExamples) {
        logger.info("Got {} positive examples and {} negative examples", positiveExamples.size(), negativeExamples.size());
        return 1;
    }
}
