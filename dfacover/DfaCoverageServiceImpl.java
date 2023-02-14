package dfacover;

import dk.brics.automaton.Automaton;
import dk.brics.automaton.AutomatonCoverage;
import dk.brics.automaton.BasicOperations;
import dk.brics.automaton.DfaBudgetExceededException;
import dk.brics.automaton.RegExp;
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

        logger.info("Scorer");

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

        DfaCoverService.DfaCoverageResponse response = DfaCoverService.DfaCoverageResponse.newBuilder()
                .addAllCoverageInfo(coverageInfoList)
                .build();

        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }

    private double computeCoverageScore(DfaCoverService.DfaCoverageRegexEntity entity, Collection<String> positiveExamples, Collection<String> negativeExamples) {

        // Build the dfa
        Automaton entityAuto = new RegExp(entity.getPattern()).toAutomaton();
        entityAuto.determinize();

        AutomatonCoverage coverage = new AutomatonCoverage(entityAuto);

        for (String positive : positiveExamples) {
            coverage.evaluatePositive(positive);
        }

        for (String negative : negativeExamples) {
            coverage.evaluateNegative(negative);
        }

        return coverage.getCoverageScore();
    }
}
