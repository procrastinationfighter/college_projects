package elections.voters;

import elections.Candidate;
import elections.ElectoralList;
import elections.Person;

/**
 * Class representing voter.
 * @author Adam Boguszewski ab417730
 */
public abstract class Voter extends Person {

    private String chosenCandidateInfo;

    /** Creates new voter.
     * @param name    - voter's name,
     * @param surname - voter's surname
     */
    public Voter(String name, String surname) {
        super(name, surname);
        chosenCandidateInfo = "<no one>";
    }

    /** Changes voter's preferences in traits.
     * If there is a voter type that is affected by campaign,
     * this method should be overridden in class inheriting after Voter.
     * @param campaignAction    - array of changes in every trait.
     */
    public void changeViews(int[] campaignAction) {
        // Default voter is not affected by campaign.
    }

    /** After voting, remembers candidate's name and surname.
     * @param info  - candidate's name and surname.
     */
    protected void setCandidateInfo(String info) {
        chosenCandidateInfo = info;
    }

    /** Calculates weighted sum after change of traits for this voter and given candidate.
     * If there is a voter type that decides basing on weighted sums,
     * this method should be overridden in class inheriting after Voter.
     * @param candidate     - given candidate,
     * @param change        - array of hipothetical change.
     * @return Weighted sum for pair (candidate, voter) of candidate's trait values and voter's weights.
     */
    public int calculateWeightedSumAfterChange(Candidate candidate, int[] change) {
        // Default voter doesn't care about traits.
        return 0;
    }

    /** Makes voter fulfill his civic duty through voting.
     * @param lists      - electoral lists.
     */
    public abstract void vote(ElectoralList[] lists);

    @Override
    public String toString() {
        return super.toString() +
                ", voted for " +
                chosenCandidateInfo;
    }
}
