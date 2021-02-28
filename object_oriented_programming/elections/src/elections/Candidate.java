package elections;

import elections.parties.Party;

import java.util.Arrays;

/**
 * Class representing person that is a candidate in the elections.
 * @author Adam Boguszewski ab417730
 */
public class Candidate extends Person {

    private int positionOnList;

    private int districtNumber;

    private final int positionOnOriginalList;

    private final int originalDistrict;

    private int[] traits;

    private Party party;

    private int votes;

    /** Creates new Candidate.
     * @param name                      - candidate's name,
     * @param surname                   - candidate's surname,
     * @param positionOnOriginalList    - candidate's position on electoral list in original district,
     * @param originalDistrict          - candidate's original district (before unification),
     * @param traits                    - candidate's traits,
     * @param party                     - name of candidate's political party.
     */
    public Candidate(String name, String surname, int positionOnOriginalList,
                     int originalDistrict, int[] traits, Party party) {
        super(name, surname);
        this.positionOnList = positionOnOriginalList;
        this.positionOnOriginalList = positionOnOriginalList;
        this.districtNumber = originalDistrict;
        this.originalDistrict = originalDistrict;
        this.traits = Arrays.copyOf(traits, traits.length);
        this.party = party;
    }

    /** Gives value of candidate's trait with number traitIndex.
     * @param traitIndex       - number of trait (not negative).
     * @return  Value of trait with number traitIndex.
     */
    public int getTraitValue(int traitIndex) {
        if(traitIndex > traits.length || traitIndex < 0) {
            throw new IndexOutOfBoundsException();
        }
        else {
            return traits[traitIndex];
        }
    }

    /** Gives candidate's position on electoral list before unification of districts.
     * @return  Candidate's original position.
     */
    public int getPositionOnOriginalList() {
        return positionOnOriginalList;
    }

    /** Gives number of candidate's original district.
     * @return  Candidate's original district's number.
     */
    public int getOriginalDistrict() { return originalDistrict; }

    /** Gives candidate's party.
     * @return Candidate's political party.
     */
    public Party getParty() {
        return party;
    }

    /** Gives number of votes that candidate earned.
     * @return Number of votes earned by this candidate.
     */
    public int getVotes() {
        return votes;
    }

    /** Changes candidate's district and position.
     * @param newDistrictNumber - number of new district,
     * @param newPosition       - new position on list.
     */
    public void changeDistrict(int newDistrictNumber, int newPosition) {
        this.districtNumber = newDistrictNumber;
        this.positionOnList = newPosition;
    }

    /** Gives current positiond on the electoral list.
     */
    public int getPosition() {
        return positionOnList;
    }

    /** Adds one vote earned by candidate.
     */
    public void addVote() {
        party.addVote(districtNumber);
        votes++;
    }

    /** Gives candidate's name and surname.
     * @return String with candidate's name and surname.
     */
    public String getNameAndSurname() {
        return super.toString();
    }

    @Override
    public String toString() {
        return super.toString() +
                ", party: " +
                party.getName() +
                ", position: " +
                positionOnList +
                ", votes earned: " +
                votes;
    }
}
