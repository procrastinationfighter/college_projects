package cover.strategies;

import cover.sets.SimpleSet;

import java.util.ArrayList;
import java.util.Collections;

/**
 * Class representing accurate strategy for solving set cover problem.
 * Calculates every combination of used sets and chooses the shortest one.
 * @author Adam Boguszewski 417730
 */
public class AccurateStrategy extends Strategy {

    /**
     * Creates an object of type AccurateStrategy.
     * @param upperBound upper bound of set that is to be covered
     * @param coveringSets sets that are used to solve set cover problem
     */
    public AccurateStrategy(int upperBound, ArrayList<SimpleSet> coveringSets) {
        super(upperBound, coveringSets);
    }

    /**
     * Checks which elements of set identified by setIndex belong to set that is to be covered.
     * @param setIndex index of set that is being checked
     * @return list of these elements.
     */
    private ArrayList<Integer> findCoveredNumbers(int setIndex) {
        ArrayList<Integer> toBeDeleted = new ArrayList<>();
        SimpleSet currentSet = coveringSets.get(setIndex);
        for(Integer setElement: toBeCovered) {
            if(currentSet.contains(setElement)) {
                toBeDeleted.add(setElement);
            }
        }
        return toBeDeleted;
    }

    /**
     * Simulates choosing set identified by setIndex.
     * Removes elements that belong both to this set and set that is to be covered
     * and calls function chooseSetCover on next element on the sets list.
     * Afterwards, adds removed elements back to the set.
     * @param setIndex index of currently considered set
     * @return  list of sets used to cover if succeeded and empty list otherwise.
     * If no elements were removed during current function call, returns empty list.
     */
    private ArrayList<Integer> useCurrentSet(int setIndex) {
        assert setIndex < coveringSets.size();
        ArrayList<Integer> deletedNumbers = findCoveredNumbers(setIndex);
        if(deletedNumbers.size() == 0) {
            //Current set wasn't actually used, so there is no need to go further.
            return new ArrayList<>();
        }
        else {
            removeCoveredNumbers(deletedNumbers);
            ArrayList<Integer> usedSets;
            if(toBeCovered.isEmpty()) {
                //Main set that is to be covered is now covered.
                usedSets = new ArrayList<>();
                usedSets.add(setIndex + 1);
            }
            else {
                usedSets = chooseSetCover(setIndex + 1);
                if(usedSets.size() > 0) {
                    usedSets.add(setIndex + 1);
                }
            }

            toBeCovered.addAll(deletedNumbers);
            return usedSets;
        }
    }

    /**
     * Compares lists of sets used to cover.
     * @param first first list of used sets
     * @param second second list of used sets
     * @return if sets have different lenghts, shorter, not empty list.
     * If both are the same length, returns the one that is first in lexicographical order.
     */
    private ArrayList<Integer> compareUsedSets(ArrayList<Integer> first, ArrayList<Integer> second) {
        if(first.size() == 0 || second.size() == 0) {
            return (first.size() == 0 ? second : first);
        }
        else {
            if(first.size() == second.size()) {
                Collections.sort(first);
                Collections.sort(second);
                int i = 0;
                int difference = 0;
                while(i < first.size() && difference == 0) {
                    difference = first.get(i) - second.get(i);
                    i++;
                }
                return (difference < 0 ? first : second);
            }
            else {
                return (first.size() > second.size() ? second : first);
            }
        }
    }

    /**
     * Checks two possible set covers,
     * with using or not set identified by setIndex.
     * @param setIndex index of currently considered set
     * @return "better" list of sets used to cover set.
     */
    private ArrayList<Integer> chooseSetCover(int setIndex) {
        if(setIndex >= coveringSets.size()) {
            return new ArrayList<>();
        }
        else{
            ArrayList<Integer> ifCurrentSetUsed = useCurrentSet(setIndex);
            ArrayList<Integer> ifCurrentSetNotUsed = chooseSetCover(setIndex + 1);
            return compareUsedSets(ifCurrentSetUsed, ifCurrentSetNotUsed);
        }
    }

    @Override
    public void coverSet() {
        ArrayList<Integer> usedSets = new ArrayList<>();
        fillSetToBeCovered();
        if(!toBeCovered.isEmpty()) {
            usedSets = chooseSetCover(0);
        }
        Collections.sort(usedSets);
        printCoveringResult(usedSets);
        toBeCovered.clear();
    }
}
