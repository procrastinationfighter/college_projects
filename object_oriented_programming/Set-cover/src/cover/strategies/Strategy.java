package cover.strategies;

import cover.sets.SimpleSet;

import java.util.ArrayList;
import java.util.HashSet;

/**
 * Abstract class of Strategy objects.
 * Every objects of this class is able to solve set cover problem.
 * @author Adam Boguszewski 417730
 */
public abstract class Strategy {

    /**
     * List of sets used to cover.
     */
    protected final ArrayList<SimpleSet> coveringSets;

    /**
     * Set that is to be covered.
     */
    protected HashSet<Integer> toBeCovered;

    /**
     * Upper bound for the set that is to be covered.
     */
    private final int upperBound;

    /**
     * Initializes member variables of abstract class Strategy.
     * @param upperBound upper bound of set that is to be covered
     * @param coveringSets sets used to solve set cover problem
     */
    public Strategy(int upperBound, ArrayList<SimpleSet> coveringSets) {
        this.coveringSets = coveringSets;
        this.upperBound = upperBound;
        this.toBeCovered = new HashSet<>();
    }

    /**
     * Fills toBeCovered with numbers from 1 to upperBound.
     */
    protected void fillSetToBeCovered() {
        for(int i = 1; i <= upperBound; i++) {
            toBeCovered.add(i);
        }
    }

    /**
     * Removes elements from set that is to be covered.
     * @param toBeDeleted numbers that were just covered and should be removed from toBeCovered
     */
    protected void removeCoveredNumbers(ArrayList<Integer> toBeDeleted) {
        for(Integer del: toBeDeleted) {
            toBeCovered.remove(del);
        }
    }

    /**
     * Prints result of set covering.
     * Prints numbers sets used if succeeded and 0 otherwise.
     * @param setsUsed numbers of sets that were chosen as the best solution
     */
    protected void printCoveringResult(ArrayList<Integer> setsUsed) {
        if(setsUsed.size() == 0) {
            System.out.println("0");
        }
        else {
            System.out.println(setsUsed.toString()
                    .replace("[", "")
                    .replace("]", "")
                    .replace(",", ""));
        }
    }

    /**
     * Solves set cover problem.
     */
    public abstract void coverSet();
}
