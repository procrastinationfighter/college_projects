package elections;

import elections.exceptions.PartyNotFound;
import elections.parties.Party;
import elections.voters.Voter;

import java.util.ArrayList;
import java.util.List;

/**
 * Class representing an election district.
 * @author Adam Boguszewski ab417730
 */
public class District {

    private int districtNumber;

    private List<Integer> unitedDistricts;

    private List<Voter> voters;

    private ElectoralList[] electoralLists;

    public District(List<Voter> voters, ElectoralList[] electoralLists, int districtNumber) {
        this.voters = voters;
        this.electoralLists = electoralLists;
        this.districtNumber = districtNumber;
        this.unitedDistricts = new ArrayList<>();
        unitedDistricts.add(districtNumber);
    }

    /** Gives numbers of voters in this district.
     * @return Number of people allowed to vote.
     */
    public int getNumberOfVoters() {
        return voters.size();
    }

    /** Apples campaign action effects to all voters.
     * @param action    - campaign action.
     */
    public void applyCampaignEffects(int[] action) {
        for(var voter: voters) {
            voter.changeViews(action);
        }
    }

    /** Calculates sum of weighted sums in this district after change defined by action.
     * @param action    - array defining given action,
     * @param list      - electoral list of party taking the action.
     * @return  Sum of weighted sums for all pairs (candidate, voter) in this district.
     */
    private int calculateSumAfterChange(int[] action, ElectoralList list) {
        int sum = 0;
        for(int i = 0; i < list.getNumberOfCandidates(); i++) {
            for(var voter: voters) {
                sum += voter.calculateWeightedSumAfterChange(list.findCandidateByCurrentIndex(i), action);
            }
        }
        return sum;
    }

    /** Calculates change of sum weighted sum in this districts.
     * @param action    - considered action,
     * @param party     - party taking the action.
     * @return Sum of weighted sums of action array.
     * @throws PartyNotFound if party has no list in this district.
     */
    public int calculateSumChange(int[] action, Party party) throws PartyNotFound {
        ElectoralList partyList = null;
        for(ElectoralList list: electoralLists) {
            if(list.getParty().equals(party)) {
                partyList = list;
                break;
            }
        }
        if(partyList == null) {
            throw new PartyNotFound();
        }
        int inicialSum = calculateSumAfterChange(new int[action.length], partyList);
        int sumAfterChange = calculateSumAfterChange(action, partyList);
        return sumAfterChange - inicialSum;
    }

    /** Makes all citizens of the district to vote.
     */
    public void vote() {
        for(var voter: voters) {
            voter.vote(electoralLists);
        }
    }

    /** Changes number of this district.
     * @param newNumber     - new number of this district.
     */
    public void changeNumber(int newNumber) {
        this.districtNumber = newNumber;
        for(var list: electoralLists) {
            list.changeNumber(newNumber);
        }
    }

    /** Unites two districts. This districts becomes the new, united district.
     * @param district      - district to unite with.
     */
    public void unite(District district) {
        this.unitedDistricts.add(district.districtNumber);
        this.voters.addAll(district.voters);
        for(int i = 0; i < electoralLists.length; i++) {
            // Assumes that every party has the same list number in every district.
            this.electoralLists[i].unite(district.electoralLists[i]);
        }
    }

    /** Gives number of parliament seats available in this district.
     * @return  Number of seats in parliament.
     */
    public int getNumberOfSeats() {
        return voters.size()/10;
    }

    /** Gives district's number.
     * @return  District's number.
     */
    public int getDistrictNumber() {
        return districtNumber;
    }

    /** Prints result of voting in this district.
     */
    public void printVotingResult() {
        System.out.println(toString());
        for(var voter: voters) {
            System.out.println(voter);
        }

        for(var list: electoralLists) {
            // Printing every list in separate lines causes
            // candidates from different lists to be separated on output.
            System.out.println(list);
        }
    }

    @Override
    public String toString() {
        StringBuilder str = new StringBuilder();
        str.append("Voting District ");
        str.append(districtNumber);
        if(unitedDistricts.size() > 1) {
            str.append(" (merged districts:");
            for(var i: unitedDistricts) {
                str.append(" ");
                str.append(i);
            }
            str.append(")");
        }
        return str.toString();
    }
}
