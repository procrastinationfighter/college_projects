package elections.allocatingseatsmethods;

/**
 * Class representing D'Hondt method for allocating seats.
 * @author Adam Boguszewski ab417730
 */
public class DHondtMethod extends QuotientBasedMethod {

    /** Creates new instance of D'Hondt's method.
     */
    public DHondtMethod() {
        super();
    }

    @Override
    protected int nextDivisor(int divisorCount) {
        return divisorCount + 1;
    }

    @Override
    public String toString() {
        return "D'Hondt";
    }
}
