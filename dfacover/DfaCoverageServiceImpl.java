package dfacover;

import io.grpc.stub.StreamObserver;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import regextools.DfaCoverService;
import regextools.DfaCoverageServiceGrpc;

import java.util.List;
import java.util.stream.Collectors;

public class DfaCoverageServiceImpl extends DfaCoverageServiceGrpc.DfaCoverageServiceImplBase {

    private static final Logger logger = LoggerFactory.getLogger(DfaCoverageServiceImpl.class);

    @Override
    public void execDfaCoverage(DfaCoverService.DfaCoverageRequest request, StreamObserver<DfaCoverService.DfaCoverageResponse> responseObserver) {

        logger.info("NOP scorer");

        // basically just NOP for testing
        List<DfaCoverService.DfaCoverageInfo> coverageInfoList = request.getEntitiesList().stream()
                .map(entity -> DfaCoverService.DfaCoverageInfo.newBuilder()
                        .setId(entity.getId())
                        .setSuccess(true)
                        .setScore(1)
                        .build()
                )
                .collect(Collectors.toList());

        // TODO build this out
        DfaCoverService.DfaCoverageResponse response = DfaCoverService.DfaCoverageResponse.newBuilder()
                .addAllCoverageInfo(coverageInfoList)
                .build();

        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }
}
