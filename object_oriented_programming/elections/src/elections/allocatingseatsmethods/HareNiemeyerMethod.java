package elections.allocatingseatsmethods;

import elections.District;
import elections.parties.Party;

import java.util.ArrayList;
import java.util.List;

/**
 * Class representing Hare-Niemeyer (largest remainder) method of allocating seats.
 * @author Adam Boguszewski ab417730
 */
public class HareNiemeyerMethod extends Method {

    private List<Double> remainders;

    private List<Party> partiesSortedByRemainder;

    /** Creates new instance of Hare-Niemeyer method.
     */
    public HareNiemeyerMethod() {
        this.remainders = new ArrayList<>();
        this.partiesSortedByRemainder = new ArrayList<>();
    }

    /** Inserts remainder from division to list, ascending.
     * @param remainder - remainder,
     * @param party     - party for which the remainder was calculated.
     */
    private void insertRemainder(double remainder, Party party) {
        int i = 0;
        while(i < remainders.size() && remainder < remainders.get(i)) {
            i++;
        }
        remainders.add(i, remainder);
        partiesSortedByRemainder.add(i, party);
    }

    @Override
    protected void allocateSeatsInDistrict(District district, List<Party> parties) {
        int districtNumber = district.getDistrictNumber();
        double multiplier = (district.getNumberOfSeats() * 1.0) / district.getNumberOfVoters();
        int givenSeats = 0;
        for(var party: parties) {
            double earnedSeatsQuotient = (multiplier * party.getVotesInDistrict(districtNumber));
            int earnedSeats = (int) earnedSeatsQuotient;
            double remainder = earnedSeatsQuotient - earnedSeats;
            insertRemainder(remainder, party);
            givenSeats += earnedSeats;
            party.addEarnedSeats(districtNumber, earnedSeats);
        }

        int restOfSeats = district.getNumberOfSeats() - givenSeats;
        int j = 0;
        while(restOfSeats > 0) {
            partiesSortedByRemainder.get(j).addEarnedSeats(districtNumber, 1);
            restOfSeats--;
            j++;
            if(j == parties.size()) {
                j = 0;
            }
        }
        partiesSortedByRemainder.clear();
        remainders.clear();
    }

    @Override
    public String toString() {
        return "Hare-Niemeyer";
    }
}
