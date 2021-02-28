package elections.voters;

import elections.Candidate;

import java.util.List;

/**
 * Class representing a voter that votes basing on maximal value of one trait.
 * @author Adam Boguszewski ab417730
 */
public class MaximizingVoter extends OptimizingVoter {

    private final int traitIndex;

    /**
     * Creates new voter that votes only on his favourite party,
     * but picks candidate based on maximal value of given trait.
     * @param name         - voter's name,
     * @param surname      - voter's surname,
     * @param traitIndex   - voter's favourite trait (positive).
     */
    public MaximizingVoter(String name, String surname, int traitIndex) {
        super(name, surname);
        this.traitIndex = traitIndex - 1;
    }

    @Override
    protected int compareCandidate(List<Candidate> candidates, Candidate newCandidate) {
        if(candidates.isEmpty()) {
            return 1;
        }
        else {
            int currBest = candidates.get(0).getTraitValue(traitIndex);
            int candBest = newCandidate.getTraitValue(traitIndex);
            return Integer.compare(candBest, currBest);
        }
    }
}
