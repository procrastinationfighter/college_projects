package elections.voters;

import elections.Candidate;
import elections.ElectoralList;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Class representing a voter that votes basing on value of one trait.
 * @author Adam Boguszewski ab417730
 */
public abstract class OptimizingVoter extends Voter {

    private Random generator;

    /**
     * Creates new voter that votes only on his favourite party,
     * but picks candidate based on value of given trait.
     * @param name               - voter's name,
     * @param surname            - voter's surname,
     */
    public OptimizingVoter(String name, String surname) {
        super(name, surname);
        generator = new Random();
    }

    /** Check if new candidate is considered better by the voter.
     * @param candidates     - list of currently best candidates,
     * @param newCandidate   - compared candidate.
     * @return Positive value if given candidate is considered better,
     * 0 if candidate is considered equal,
     * negative value if candidate is considered worse.
     */
    protected abstract int compareCandidate(List<Candidate> candidates, Candidate newCandidate);

    /** From a given list, pick candidates with the best trait values.
     * @param list          - electoral list of one party,
     * @param candidates    - list of candidates with currently the best values.
     */
    protected void pickCandidatesFromList(ElectoralList list, List<Candidate> candidates) {
        for(int i = 0; i < list.getNumberOfCandidates(); i++) {
            Candidate temp = list.findCandidateByCurrentIndex(i);
            int candidateComparation = compareCandidate(candidates, temp);
            if(candidateComparation > 0) {
                candidates.clear();
                candidates.add(temp);
            }
            else if(candidateComparation == 0) {
                candidates.add(temp);
            }
        }
    }

    /** Finds all candidates with best value of voter's favourite trait.
     * @param lists     - electoral lists.
     * @return  List of candidates with the same trait value,
     * that is most important for the voter.
     */
    protected List<Candidate> findCandidatesWithBestTraitValue(ElectoralList[] lists) {
        List<Candidate> candidates = new ArrayList<>();
        for(var list: lists) {
            pickCandidatesFromList(list, candidates);
        }
        return candidates;
    }

    @Override
    public void vote(ElectoralList[] lists) {
        try {
            List<Candidate> goodCandidates = findCandidatesWithBestTraitValue(lists);
            Candidate chosenCandidate = goodCandidates.get(generator.nextInt(goodCandidates.size()));
            chosenCandidate.addVote();
            setCandidateInfo(chosenCandidate.getNameAndSurname());
        } catch (NullPointerException e) {
            System.err.println("Candidate with optimal traits couldn't be found.");
        }
    }
}
