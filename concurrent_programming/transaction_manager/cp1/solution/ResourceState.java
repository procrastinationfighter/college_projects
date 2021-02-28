package cp1.solution;

import cp1.base.ActiveTransactionAborted;
import cp1.base.Resource;
import cp1.base.ResourceOperation;
import cp1.base.ResourceOperationException;

import java.util.*;
import java.util.concurrent.Semaphore;

public class ResourceState {

    private final Resource resource;

    private final Deque<ResourceOperation> operations = new ArrayDeque<>();

    private Transaction currentOwner = null;


    private final Semaphore resourceAccess = new Semaphore(1, true);

    private final Semaphore dfsProtection;

    public ResourceState(Resource resource, Semaphore dfsProtection) {
        this.resource = resource;
        this.dfsProtection = dfsProtection;
    }

    private boolean isCurrThreadOwner() {
        return currentOwner != null && Thread.currentThread() == currentOwner.getExecutingThread();
    }

    private void checkIfNotDeadlocked(Transaction transaction) {
        Transaction currTransaction;
        ResourceState currDesiredResource = this;
        List<Transaction> youngestTransactions = new ArrayList<>();
        youngestTransactions.add(transaction);

        while (currDesiredResource != null) {

            currTransaction = currDesiredResource.currentOwner;
            if (currTransaction == null) {
                break;
            }
            currDesiredResource = currTransaction.getCurrentlyDesiredResource();

            if (currTransaction.getStartTime() > youngestTransactions.get(0).getStartTime()) {
                youngestTransactions.clear();
                youngestTransactions.add(currTransaction);
            } else if (currTransaction.getStartTime() == youngestTransactions.get(0).getStartTime()) {
                youngestTransactions.add(currTransaction);
            }

            if (currTransaction.isAborted()) {
                break;
            } else if (currDesiredResource == this) {
                // Possible deadlock.
                Transaction highestId = youngestTransactions.get(0);

                for (var trans : youngestTransactions) {
                    if (trans.getExecutingThread().getId() > highestId.getExecutingThread().getId()) {
                        highestId = trans;
                    }
                }

                highestId.abort();
            }
        }
    }

    private void tryAcquiringResource(Transaction transaction)
            throws
            InterruptedException,
            ActiveTransactionAborted {
        try {
            dfsProtection.acquire();
            transaction.setCurrentlyDesiredResource(this);
            if (currentOwner != null)
                checkIfNotDeadlocked(transaction);
            dfsProtection.release();
            resourceAccess.acquire();
        } catch (InterruptedException e) {
            transaction.setCurrentlyDesiredResource(null);
            if (transaction.isAborted()) {
                throw new ActiveTransactionAborted();
            } else {
                throw e;
            }
        }
    }

    public void acquireResource(Transaction transaction)
            throws
            InterruptedException,
            ActiveTransactionAborted {
        if (!isCurrThreadOwner()) {
            tryAcquiringResource(transaction);

            assert currentOwner == null;

            // Only one thread is allowed to be here in one time.
            transaction.setCurrentlyDesiredResource(null);
            currentOwner = transaction;
            transaction.addAcquiredResource(this);
        }
    }

    public void undoLastOperation() {
        assert !operations.isEmpty();
        resource.unapply(operations.pop());
    }

    public void undoOperations() {
        if (isCurrThreadOwner()) {
            while (!operations.isEmpty()) {
                undoLastOperation();
            }
        } else {
            System.err.println("Thread "
                    + Thread.currentThread().getId()
                    + " tried to undo operations on foreign resource.");
        }
    }

    public void freeResource() {
        if (isCurrThreadOwner()) {
            currentOwner = null;
            operations.clear();
            resourceAccess.release();
        } else {
            System.err.println("Thread "
                    + Thread.currentThread().getId()
                    + " tried to free foreign resource.");
        }
    }

    public void performOperation(ResourceOperation operation)
            throws
            ResourceOperationException {
        if (isCurrThreadOwner()) {
            operation.execute(resource);
            operations.push(operation);
        } else {
            System.err.println("Thread "
                    + Thread.currentThread().getId()
                    + " tried to perform an operation on foreign resource.");
        }
    }
}
