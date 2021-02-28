package elections.allocatingseatsmethods;

import elections.District;
import elections.parties.Party;

import java.util.*;

/**
 * Class representing seat allocating method that is based on quotients.
 * @author Adam Boguszewski ab417730
 */
public abstract class QuotientBasedMethod extends Method {
    private Random generator;

    public QuotientBasedMethod() {
        generator = new Random();
    }

    /** Gives next divisor for calculating quotients.
     * @param divisorCount  - number of used divisor.
     * @return  Calculated divisor.
     */
    protected abstract int nextDivisor(int divisorCount);

    /** Finds and removes currently highest quotient. If some quotients are equal,
     * @param quotients     - quotients of parties.
     * @return  Index of chosen party.
     */
    private int findAndRemoveHighestQuotient(List<Deque<Double>> quotients) {
        double currHighest = 0;
        List<Integer> indices = new ArrayList<>();
        for(int i = 0; i < quotients.size(); i++) {
            var queue = quotients.get(i);
            if(currHighest < queue.getFirst()) {
                indices.clear();
                currHighest = queue.getFirst();
                indices.add(i);
            }
            else if(currHighest == queue.getFirst()) {
                indices.add(i);
            }
        }
        int chosenIndex = indices.get(generator.nextInt(indices.size()));
        quotients.get(chosenIndex).removeFirst();
        return chosenIndex;
    }

    @Override
    protected void allocateSeatsInDistrict(District district, List<Party> parties) {
        List<Deque<Double>> quotients = new ArrayList<>();
        for(var party: parties) {
            Deque<Double> currQuotients = new ArrayDeque<>();
            for(int i = 0; i < district.getNumberOfSeats(); i++) {
                currQuotients.add((double) (party.getVotesInDistrict(district.getDistrictNumber()) / (nextDivisor(i))));
            }
            quotients.add(currQuotients);
        }
        for(int i = 0; i < district.getNumberOfSeats(); i++) {
            int partyIndex = findAndRemoveHighestQuotient(quotients);
            parties.get(partyIndex).addEarnedSeats(district.getDistrictNumber(), 1);
        }
    }
}
