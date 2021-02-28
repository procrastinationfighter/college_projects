package elections.voters;

import elections.Candidate;

import java.util.Arrays;
import java.util.List;

/**
 * Class representing a voter that votes basing on values of all traits of candidates.
 * @author Adam Boguszewski ab417730
 */
public class UniversalVoter extends OptimizingVoter {

    private static final int MAX_WEIGHT = 100;

    private static final int MIN_WEIGHT = -100;

    private int[] traitsWeights;

    /**
     * Creates new voter that votes only on his favourite party,
     * but picks candidate based on values of all traits.
     * @param name              - voter's name,
     * @param surname           - voter's surname,
     * @param traitsWeights     - voter's traits weight.
     */
    public UniversalVoter(String name, String surname, int[] traitsWeights) {
        super(name, surname);
        this.traitsWeights = Arrays.copyOf(traitsWeights, traitsWeights.length);
    }

    /** Calculates weighted sum of candidate's traits.
     * @param candidate     - candidate currently being checked.
     * @return  Weighted sum of candidate's traits.
     */
    private int calculateWeightedSum(Candidate candidate) {
        int sum = 0;
        for(int i = 0; i < traitsWeights.length; i++) {
            sum += traitsWeights[i] * candidate.getTraitValue(i);
        }
        return sum;
    }

    @Override
    public void changeViews(int[] campaignAction) {
        for(int i = 0; i < campaignAction.length; i++) {
            traitsWeights[i] += campaignAction[i];
            if(traitsWeights[i] > MAX_WEIGHT) {
                traitsWeights[i] = MAX_WEIGHT;
            }
            else if(traitsWeights[i] < MIN_WEIGHT) {
                traitsWeights[i] = MIN_WEIGHT;
            }
        }
    }

    @Override
    public int calculateWeightedSumAfterChange(Candidate candidate, int[] change) {
        int sum = 0;
        for(int i = 0; i < traitsWeights.length; i++) {
            int weightAfterChange = Math.max(Math.min(traitsWeights[i] + change[i], MAX_WEIGHT), MIN_WEIGHT);
            sum += (candidate.getTraitValue(i) * weightAfterChange);
        }
        return sum;
    }

    @Override
    protected int compareCandidate(List<Candidate> candidates, Candidate newCandidate) {
        if(candidates.isEmpty()) {
            return 1;
        }
        else {
            int currSum = calculateWeightedSum(candidates.get(0));
            int newSum = calculateWeightedSum(newCandidate);
            return Integer.compare(newSum, currSum);
        }
    }
}
