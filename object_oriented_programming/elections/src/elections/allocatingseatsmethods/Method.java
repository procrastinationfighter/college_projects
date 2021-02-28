package elections.allocatingseatsmethods;

import elections.District;
import elections.parties.Party;

import java.util.List;

/**
 * Class representing a method for allocating seats.
 * @author Adam Boguszewski ab417730
 */
public abstract class Method {

    /** Allocates seats in one district.
     * @param district  - given district.
     */
    protected abstract void allocateSeatsInDistrict(District district, List<Party> parties);

    /** Allocates seats in all districts after voting.
     * @param districts - voting districts,
     * @param parties   - participating parties.
     */
    public void allocateSeats(List<District> districts, List<Party> parties) {
        for(var district: districts) {
            allocateSeatsInDistrict(district, parties);
        }
    }


}
