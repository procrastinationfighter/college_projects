package cp1.solution;

import cp1.base.*;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.Semaphore;

public class TransactionManagerImpl implements TransactionManager {

    private final LocalTimeProvider timeProvider;

    private final List<Transaction> activeTransactions = new ArrayList<>();

    private final ConcurrentMap<ResourceId, ResourceState> resources = new ConcurrentHashMap<>();

    public TransactionManagerImpl(Collection<Resource> resources,
                                  LocalTimeProvider timeProvider) {
        // TransactionManager owns resources given to him, no need to copy them.
        this.timeProvider = timeProvider;
        Semaphore dfsProtection = new Semaphore(1, true);
        for (var resource : resources) {
            this.resources.put(resource.getId(), new ResourceState(resource, dfsProtection));
        }
    }

    @Override
    public synchronized void startTransaction()
            throws
            AnotherTransactionActiveException {
        try {
            getTransaction();
            // No exception caught, there must be an active, existing transaction for this thread.
            throw new AnotherTransactionActiveException();
        } catch (NoActiveTransactionException e) {
            activeTransactions.add(new Transaction(Thread.currentThread(), timeProvider.getTime()));
        }
    }

    @Override
    public void operateOnResourceInCurrentTransaction(ResourceId rid,
                                                      ResourceOperation operation)
            throws
            NoActiveTransactionException,
            UnknownResourceIdException,
            ActiveTransactionAborted,
            ResourceOperationException,
            InterruptedException {
        Transaction transaction = getTransaction();
        ResourceState resource = getResource(rid);
        //todo change isInterrupted to Interrupted
        if (transaction.isAborted()) {
            throw new ActiveTransactionAborted();
        } else if (Thread.interrupted()) {
            throw new InterruptedException();
        }

        resource.acquireResource(transaction);
        if (Thread.interrupted()) {
            throw new InterruptedException();
        }

        resource.performOperation(operation);
        if (Thread.interrupted()) {
            resource.undoLastOperation();
            throw new InterruptedException();
        }
    }

    @Override
    public synchronized void commitCurrentTransaction()
            throws
            NoActiveTransactionException,
            ActiveTransactionAborted {
        Transaction transaction = getTransaction();

        if (transaction.isAborted()) {
            throw new ActiveTransactionAborted();
        }
        transaction.freeAllResources();
        removeTransaction(transaction);
    }

    @Override
    public void rollbackCurrentTransaction() {
        try {
            Transaction transaction = getTransaction();
            transaction.undoAllOperations();
            transaction.freeAllResources();
            removeTransaction(transaction);
        } catch (NoActiveTransactionException ignored) {
            // Rollback is not possible, but it's not an error.
        }
    }

    @Override
    public boolean isTransactionActive() {
        try {
            getTransaction();
            return true;
        } catch (NoActiveTransactionException e) {
            return false;
        }
    }

    @Override
    public boolean isTransactionAborted() {
        try {
            return getTransaction().isAborted();
        } catch (NoActiveTransactionException e) {
            return false;
        }
    }

    private synchronized Transaction getTransaction()
            throws
            NoActiveTransactionException {
        Thread currThread = Thread.currentThread();
        for (Transaction t : activeTransactions) {
            if (currThread.equals(t.getExecutingThread())) {
                return t;
            }
        }
        throw new NoActiveTransactionException();
    }

    private synchronized void removeTransaction(Transaction transaction) {
        activeTransactions.remove(transaction);
    }

    private ResourceState getResource(ResourceId rid)
            throws
            UnknownResourceIdException {
        ResourceState res = resources.get(rid);
        if (res == null) {
            throw new UnknownResourceIdException(rid);
        } else {
            return res;
        }
    }
}
