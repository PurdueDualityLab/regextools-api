package dfacover;

import io.grpc.Grpc;
import io.grpc.InsecureServerCredentials;
import io.grpc.Server;
import io.grpc.ServerBuilder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.concurrent.TimeUnit;

public class DfaCoverageServer {

    private static final Logger logger = LoggerFactory.getLogger(DfaCoverageServer.class);

    private final int port;
    private final Server server;

    public DfaCoverageServer(int port) {
        this(Grpc.newServerBuilderForPort(port, InsecureServerCredentials.create()), port);
    }

    public DfaCoverageServer(ServerBuilder<?> serverBuilder, int port) {
        this.port = port;
        server = serverBuilder
                .addService(new DfaCoverageServiceImpl())
                .build();
    }

    /** Start serving requests. */
    public void start() throws IOException {
        logger.info("Starting up server on port {}...", this.port);
        server.start();
        // logger.info("Server started, listening on " + port);
        Runtime.getRuntime().addShutdownHook(new Thread() {
            @Override
            public void run() {
                // Use stderr here since the logger may have been reset by its JVM shutdown hook.
                System.err.println("*** shutting down gRPC server since JVM is shutting down");
                try {
                    DfaCoverageServer.this.stop();
                } catch (InterruptedException e) {
                    e.printStackTrace(System.err);
                }
                System.err.println("*** server shut down");
            }
        });
        logger.info("Server is running");
    }

    /** Stop serving requests and shutdown resources. */
    public void stop() throws InterruptedException {
        logger.info("Starting to shutdown server...");
        if (server != null) {
            server.shutdown().awaitTermination(30, TimeUnit.SECONDS);
        }
    }

    /**
     * Await termination on the main thread since the grpc library uses daemon threads.
     */
    public void blockUntilShutdown() throws InterruptedException {
        if (server != null) {
            server.awaitTermination();
        }
    }
}
