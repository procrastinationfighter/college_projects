package cover.strategies;

import cover.sets.SimpleSet;

import java.util.ArrayList;

/**
 * Class representing naive strategy for solving set cover problem.
 * With every step, picks a set if it contains a number that wasn't yet covered.
 * @author Adam Boguszewski 417730
 */
public class NaiveStrategy extends Strategy {

    /**
     * Creates an object of type NaiveStrategy.
     * @param upperBound upper bound of set that is to be covered
     * @param coveringSets sets that are used to solve set cover problem
     */
    public NaiveStrategy(int upperBound, ArrayList<SimpleSet> coveringSets) {
        super(upperBound, coveringSets);
    }

    /**
     * Finds numbers that weren't yet covered and belong to main set and currentSet.
     * @param toBeDeleted numbers that exist both in currentSet and toBeCovered.
     * @param currentSet set to be checked.
     * @return true if there is at least one such number and false otherwise.
     */
    private boolean chooseNumbersToRemove(ArrayList<Integer> toBeDeleted,
                                          SimpleSet currentSet) {
        boolean wasSetUsed = false;
        for(Integer setElement: toBeCovered) {
            if(currentSet.contains(setElement)) {
                wasSetUsed = true;
                toBeDeleted.add(setElement);
            }
        }
        return wasSetUsed;
    }

    /**
     * Removes numbers that are covered by currentSet.
     * @param currentSet set that may contains such numbers.
     * @return true if there was at least one such number and false otherwise.
     */
    private boolean removeCoveredNumbers(SimpleSet currentSet) {
        ArrayList<Integer> toBeDeleted = new ArrayList<>();
        boolean wasSetUsed = chooseNumbersToRemove(toBeDeleted, currentSet);

        for(Integer del: toBeDeleted) {
            toBeCovered.remove(del);
        }

        return wasSetUsed;
    }

    @Override
    public void coverSet() {
        ArrayList<Integer> setsUsed = new ArrayList<>();
        fillSetToBeCovered();

        int i = 1;
        for(SimpleSet currentSet: coveringSets) {
            boolean wasSetUsed = removeCoveredNumbers(currentSet);

            if(wasSetUsed) {
                setsUsed.add(i);
            }
            i++;
        }

        if(toBeCovered.isEmpty()) {
            printCoveringResult(setsUsed);
        }
        else {
            System.out.println("0");
        }

        toBeCovered.clear();
    }
}
