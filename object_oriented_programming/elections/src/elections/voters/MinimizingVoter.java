package elections.voters;

import elections.Candidate;

import java.util.List;

/**
 * Class representing a voter that votes basing on minimal value of one trait.
 * @author Adam Boguszewski ab417730
 */
public class MinimizingVoter extends OptimizingVoter {

    private final int traitIndex;

    /**
     * Creates new voter that votes only on his favourite party,
     * but picks candidate based on minimal value of given trait.
     * @param name         - voter's name,
     * @param surname      - voter's surname,
     * @param traitNumber   - voter's favourite trait (positive).
     */
    public MinimizingVoter(String name, String surname, int traitNumber) {
        super(name, surname);
        this.traitIndex = traitNumber - 1;
    }

    @Override
    protected int compareCandidate(List<Candidate> candidates, Candidate newCandidate) {
        if(candidates.isEmpty()) {
            return 1;
        }
        else {
            int currBest = candidates.get(0).getTraitValue(traitIndex);
            int candBest = newCandidate.getTraitValue(traitIndex);
            return Integer.compare(currBest, candBest);
        }
    }
}
