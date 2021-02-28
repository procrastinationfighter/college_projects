package elections.voters;

import elections.Candidate;
import elections.ElectoralList;
import elections.parties.Party;

import java.util.ArrayList;
import java.util.List;

/**
 * Class representing a voter that votes basing on minimal value of one trait,
 * but votes only on candidates from one party.
 * @author Adam Boguszewski ab417730
 */
public class MinimizingVoterOfOneParty extends MinimizingVoter {

    private final Party favouriteParty;

    /**
     * Creates new voter that votes only on his favourite party,
     * but picks candidate based on minimal value of given trait.
     * @param name                  - voter's name,
     * @param surname               - voter's surname,
     * @param traitIndex            - voter's favourite trait,
     * @param favouriteParty        - voter's favourite party.
     */
    public MinimizingVoterOfOneParty(String name, String surname, int traitIndex, Party favouriteParty) {
        super(name, surname, traitIndex);
        this.favouriteParty = favouriteParty;
    }

    @Override
    protected List<Candidate> findCandidatesWithBestTraitValue(ElectoralList[] lists) {
        List<Candidate> candidates = new ArrayList<>();
        for(var list: lists) {
            if(list.getParty().equals(favouriteParty)) {
                pickCandidatesFromList(list, candidates);
            }
        }
        return candidates;
    }
}
