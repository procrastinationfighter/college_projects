package elections.voters;

import elections.Candidate;
import elections.ElectoralList;
import elections.exceptions.PartyNotFound;
import elections.parties.Party;

import java.util.Random;

/**
 * Class representing a voter that votes on random candidate of his favourite party.
 * @author Adam Boguszewski ab417730
 */
public class PartyIronElectorateVoter extends IronElectorateVoter {

    private Random generator;

    /** Creates new voter, a party admirer.
     * @param name                  - voter's name,
     * @param surname               - voter's surname,
     * @param favouriteParty        - voter's favourite party.
     */
    public PartyIronElectorateVoter(String name, String surname, Party favouriteParty) {
        super(name, surname, favouriteParty);
        generator = new Random();
    }

    @Override
    public void vote(ElectoralList[] lists) {
        try {
            ElectoralList favouriteList = findFavouriteList(lists);
            int candidateIndex = generator.nextInt(favouriteList.getNumberOfCandidates());
            Candidate chosen = favouriteList.findCandidateByCurrentIndex(candidateIndex);
            chosen.addVote();
            setCandidateInfo(chosen.getNameAndSurname());
        } catch (PartyNotFound e) {
            printPartyNotFoundError();
        }
    }
}
