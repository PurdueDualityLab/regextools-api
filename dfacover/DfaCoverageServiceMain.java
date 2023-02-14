package dfacover;

import java.io.IOException;

public class DfaCoverageServiceMain {
    public static void main(String[] args) throws IOException, InterruptedException {
        DfaCoverageServer server = new DfaCoverageServer(50052);
        server.start();
        server.blockUntilShutdown();
    }
}
