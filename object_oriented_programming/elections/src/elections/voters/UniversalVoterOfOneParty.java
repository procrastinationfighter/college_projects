package elections.voters;

import elections.Candidate;
import elections.ElectoralList;
import elections.parties.Party;

import java.util.ArrayList;
import java.util.List;

/**
 * Class representing a voter that votes basing on all traits,
 * but votes only on candidates from one party.
 * @author Adam Boguszewski ab417730
 */
public class UniversalVoterOfOneParty extends UniversalVoter {

    private final Party favouriteParty;

    /**
     * Creates new voter that votes only on his favourite party,
     * but picks candidate based on maximal value of given trait.
     * @param name                  - voter's name,
     * @param surname               - voter's surname,
     * @param traitsWeights         - voter's traits weight,
     * @param favouriteParty        - voter's favourite party.
     */
    public UniversalVoterOfOneParty(String name, String surname, int[] traitsWeights, Party favouriteParty) {
        super(name, surname, traitsWeights);
        this.favouriteParty = favouriteParty;
    }

    @Override
    protected List<Candidate> findCandidatesWithBestTraitValue(ElectoralList[] lists) {
        List<Candidate> candidates = new ArrayList<>();
        for(var list: lists) {
            if(list.getParty().equals(favouriteParty)) {
                pickCandidatesFromList(list, candidates);
                break;
            }
        }
        return candidates;
    }
}
