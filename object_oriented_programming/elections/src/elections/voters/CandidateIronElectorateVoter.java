package elections.voters;

import elections.Candidate;
import elections.ElectoralList;
import elections.exceptions.CandidateNotFound;
import elections.exceptions.PartyNotFound;
import elections.parties.Party;

/**
 * Class representing a voter that votes only on his favourite candidate.
 * @author Adam Boguszewski ab417730
 */
public class CandidateIronElectorateVoter extends IronElectorateVoter {

    private final int candidatePosition;

    private final int districtNumber;

    /** Creates new voter that votes only on his favourite party.
     * @param name                  - voter's name,
     * @param surname               - voter's surname,
     * @param favouriteParty        - voter's favourite party,
     * @param candidatePosition     - voter's favourite candidate position
     *                                on electoral list (before district unification)
     * @param districtNumber        - voter's favourite candidate district
     *                                (before district unification)
     */
    public CandidateIronElectorateVoter(String name, String surname, Party favouriteParty,
                                        int candidatePosition, int districtNumber) {
        super(name, surname, favouriteParty);
        this.candidatePosition = candidatePosition;
        this.districtNumber = districtNumber;
    }

    @Override
    public void vote(ElectoralList[] lists) {
        try {
            ElectoralList favouriteList = findFavouriteList(lists);
            Candidate chosen = favouriteList.findCandidateByOriginalPosition(districtNumber, candidatePosition);
            chosen.addVote();
            setCandidateInfo(chosen.getNameAndSurname());
        } catch (PartyNotFound e) {
            printPartyNotFoundError();
        } catch (CandidateNotFound e) {
            System.err.println("Candidate from district " + districtNumber +
                    ", position " + candidatePosition + " not found on the list.");
        }
    }
}
