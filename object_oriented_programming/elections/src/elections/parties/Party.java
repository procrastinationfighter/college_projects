package elections.parties;

import elections.District;

import java.util.List;

/**
 * Class representing a political party.
 * @author Adam Boguszewski ab417730
 */
public abstract class Party {

    private final String name;

    private int budget;

    private int[] votesInDistricts;

    private int[] seatsInDistricts;

    private int totalSeats;

    /** Creates a new party.
     * @param name              - party name,
     * @param budget            - party budget,
     * @param numberOfDistricts - number of districts in this election.
     */
    public Party(String name, int budget, int numberOfDistricts) {
        this.name = name;
        this.budget = budget;
        this.votesInDistricts = new int[numberOfDistricts];
        this.seatsInDistricts = new int[numberOfDistricts];
    }

    /** Gives party's name.
     * @return  Party's name.
     */
    public String getName() {
        return name;
    }

    /** Calculates cost of campaign action.
     * @param district  - considered district,
     * @param action    - considered action.
     * @return Cost of using given action in given district.
     */
    protected int calculateActionCost(District district, int[] action) {
        int changeVector = 0;
        for(int x: action) {
            changeVector += Math.abs(x);
        }
        return district.getNumberOfVoters() * changeVector;
    }

    /** Lowers party's budget by certain amount.
     * @param amount   - amount of money spent.
     */
    protected void lowerBudget(int amount) {
        budget -= amount;
    }

    /** Gives party's budget.
     * @return Party's budget.
     */
    protected int getBudget() {
        return budget;
    }

    /** Takes an action in election campaign.
     * @param districts         - all districts in the country,
     * @param campaignActions   - campaign actions allowed to be taken.
     * @return True if budget allowed for any action to be taken and false otherwise.
     */
    public abstract boolean takeCampaignAction(List<District> districts, int[][] campaignActions);

    /** Adds one vote earned by candidate of this party.
     * @param districtNumber    - district number of candidate.
     */
    public void addVote(int districtNumber) {
        votesInDistricts[districtNumber - 1]++;
    }

    /** Gives number of votes earned by this party in given district.
     * @param districtNumber    - number of district.
     * @return Number of votes earned by this party in district with number districtNumber.
     */
    public int getVotesInDistrict(int districtNumber) {
        return votesInDistricts[districtNumber - 1];
    }

    /** Adds seats earned by the party in given district.
     * @param districtNumber    - number of district in which seats were earned,
     * @param numberOfSeats     - number of earned seats.
     */
    public void addEarnedSeats(int districtNumber, int numberOfSeats) {
        totalSeats += numberOfSeats;
        seatsInDistricts[districtNumber - 1] += numberOfSeats;
    }

    /** Sets all earned seats to 0.
     */
    public void resetEarnedSeats() {
        totalSeats = 0;
        seatsInDistricts = new int[seatsInDistricts.length];
    }

    /** Gives string with party's name and score in given district.
     * @param numberOfDistrict  - number of district.
     * @return String with party's name and score.
     */
    public String getResultInDistrict(int numberOfDistrict) {
        return name +
                ", votes earned: " +
                seatsInDistricts[numberOfDistrict - 1];
    }

    @Override
    public String toString() {
        return name +
                ", total seats earned: " +
                totalSeats;
    }
}
