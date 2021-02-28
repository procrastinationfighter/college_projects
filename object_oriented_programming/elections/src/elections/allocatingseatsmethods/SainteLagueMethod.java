package elections.allocatingseatsmethods;

/**
 * Class representing Sainte-Lague method for allocating seats.
 * @author Adam Boguszewski ab417730
 */
public class SainteLagueMethod extends QuotientBasedMethod {

    /** Creates new instance of Sainte-League method.
     */
    public SainteLagueMethod() {
        super();
    }

    @Override
    protected int nextDivisor(int divisorCount) {
        return (divisorCount * 2) + 1;
    }

    @Override
    public String toString() {
        return "Sainte-Lague";
    }
}
