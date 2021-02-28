package cover.strategies;

import cover.sets.SimpleSet;

import java.util.ArrayList;
import java.util.Collections;

/**
 * Class representing greedy strategy for set cover problem.
 * With every step, chooses set that contains the largest amount of not yet covered elements.
 * @author Adam Boguszewski 417730
 */
public class GreedyStrategy extends Strategy {

    /**
     * Creates an object of type GreedyStrategy.
     * @param upperBound upper bound of set that is to be covered
     * @param coveringSets sets that are used to solve set cover problem
     */
    public GreedyStrategy(int upperBound, ArrayList<SimpleSet> coveringSets) {
        super(upperBound, coveringSets);
    }

    /**
     * Finds numbers that weren't yet covered and are in main set and currentSet.
     * @param currentSet set that is currently being checked
     * @return list of numbers that currently belong both to currentSet and toBeCovered
     */
    private ArrayList<Integer> findCoveredNumbers(SimpleSet currentSet) {
        ArrayList<Integer> toBeDeleted = new ArrayList<>();
        for(Integer setElement: toBeCovered) {
            if(currentSet.contains(setElement)) {
                toBeDeleted.add(setElement);
            }
        }
        return toBeDeleted;
    }

    /**
     * Finds set that currently contains the largest amount of numbers
     * that weren't yet covered.
     * @return index of this set
     */
    private int findCoveringSet() {
        int i = 1;
        int mostCoveredSetIndex = 0;
        ArrayList<Integer> currentMostCovered = new ArrayList<>();

        for(SimpleSet currentSet: coveringSets) {
            ArrayList<Integer> currentSetCovered = findCoveredNumbers(currentSet);
            if(currentSetCovered.size() > currentMostCovered.size()) {
                currentMostCovered = currentSetCovered;
                mostCoveredSetIndex = i;
            }
            i++;
        }

        removeCoveredNumbers(currentMostCovered);
        return mostCoveredSetIndex;
    }

    @Override
    public void coverSet() {
        ArrayList<Integer> setsUsed = new ArrayList<>();
        fillSetToBeCovered();
        int currSetNumber;
        do {
            currSetNumber = findCoveringSet();
            if(currSetNumber != 0) {
                setsUsed.add(currSetNumber);
            }
        } while(currSetNumber != 0);

        if(toBeCovered.isEmpty()) {
            Collections.sort(setsUsed);
            printCoveringResult(setsUsed);
        }
        else {
            System.out.println("0");
        }

        toBeCovered.clear();
    }
}
