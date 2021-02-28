package cp1.solution;

import java.util.ArrayList;
import java.util.List;

public class Transaction {

    private final Thread executingThread;

    private final long startTime;

    private final List<ResourceState> acquiredResources = new ArrayList<>();

    private boolean isAborted = false;

    private ResourceState currentlyDesiredResource = null;

    public Transaction(Thread executingThread, long startTime) {
        this.executingThread = executingThread;
        this.startTime = startTime;
    }

    public void freeAllResources() {
        for (var resource : acquiredResources) {
            resource.freeResource();
        }
    }

    public void undoAllOperations() {
        for (var resource : acquiredResources) {
            resource.undoOperations();
        }
    }

    public void abort() {
        isAborted = true;
        executingThread.interrupt();
    }

    public void setCurrentlyDesiredResource(ResourceState resource) {
        this.currentlyDesiredResource = resource;
    }

    public ResourceState getCurrentlyDesiredResource() {
        return currentlyDesiredResource;
    }

    public void addAcquiredResource(ResourceState resource) {
        acquiredResources.add(resource);
    }

    public boolean isAborted() {
        return isAborted;
    }

    public Thread getExecutingThread() {
        return executingThread;
    }

    public long getStartTime() {
        return startTime;
    }

}
