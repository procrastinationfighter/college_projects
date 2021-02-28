package elections.voters;

import elections.ElectoralList;
import elections.exceptions.PartyNotFound;
import elections.parties.Party;

/**
 * Class representing a voter that votes only on his favourite party.
 * @author Adam Boguszewski ab417730
 */
public abstract class IronElectorateVoter extends Voter {

    private final Party favouriteParty;

    /** Creates new voter that votes only on his favourite party.
     * @param name      - voter's name,
     * @param surname   - voter's surname,
     * @param favouriteParty - voter's favourite party.
     */
    public IronElectorateVoter(String name, String surname, Party favouriteParty) {
        super(name, surname);
        this.favouriteParty = favouriteParty;
    }

    /** Finds electoral list of voter's favourite party.
     * @param lists     - electoral lists in this district.
     * @return  List of voter's favourite party if it exists.
     * @throws PartyNotFound if there is no list of such party.
     */
    protected ElectoralList findFavouriteList(ElectoralList[] lists) throws PartyNotFound {
        for(ElectoralList list: lists) {
            if (list.getParty().equals(favouriteParty)) {
                return list;
            }
        }
        throw new PartyNotFound();
    }

    protected void printPartyNotFoundError() {
        System.err.println("Party " + favouriteParty + " can't be found in this district.");
    }
}
