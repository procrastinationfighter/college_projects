package elections;

import elections.exceptions.CandidateNotFound;
import elections.parties.Party;

import java.util.ArrayList;
import java.util.List;

/**
 * Class representing an electoral list of one party in one district.
 * @author Adam Boguszewski ab417730
 */
public class ElectoralList {
    private final int districtNumber;

    private List<Candidate> candidates;

    /** Creates a new electoral list.
     * @param districtNumber    - number of district.
     */
    public ElectoralList(int districtNumber) {
        candidates = new ArrayList<>();
        this.districtNumber = districtNumber;
    }

    /** Adds a new candidate to the list.
     * @param candidate     - new candidate.
     */
    public void addCandidate(Candidate candidate) {
        candidates.add(candidate);
    }

    /** Gives name of the party that list belongs to.
     * @return  Name of the party.
     */
    public Party getParty() {
        try{
            return candidates.get(0).getParty();
        } catch (NullPointerException e) {
            System.err.println("Electoral list empty, party can't be checked.");
            return null;
        }
    }

    /** Gives number of candidates on this list.
     * @return Size of the electoral list.
     */
    public int getNumberOfCandidates() {
        return candidates.size();
    }

    /** Unites this list with electoral list from different district.
     * Adds all candidates from the other list to this list.
     * @param list  - list from different district.
     */
    public void unite(ElectoralList list) {
        int position = candidates.size();
        for(Candidate candidate: list.candidates) {
            position++;
            candidate.changeDistrict(districtNumber, position);
            this.addCandidate(candidate);
        }
    }

    /** Gives candidate based on his current index on the list.
     * @param index  - candidate's index.
     * @return The candidate, if found.
     */
    public Candidate findCandidateByCurrentIndex(int index) {
        return candidates.get(index);
    }

    /** Gives candidate based on his origin district and position.
     * Returns candidate based on which district was he originally in
     * and what was his position on that list.
     * @param district  - candidate's original district,
     * @param position  - candidate's original position
     * @return The candidate, if found.
     * @throws CandidateNotFound - if candidate is not on the list.
     */
    public Candidate findCandidateByOriginalPosition(int district, int position) throws CandidateNotFound {
        for(Candidate candidate: candidates) {
            if(candidate.getPositionOnOriginalList() == position &&
                    candidate.getOriginalDistrict() == district) {
                return candidate;
            }
        }
        throw new CandidateNotFound();
    }

    /** Changes number of district the list is in.
     * @param districtNumber    - new number of district.
     */
    public void changeNumber(int districtNumber) {
        for(var candidate: candidates) {
            candidate.changeDistrict(districtNumber, candidate.getPosition());
        }
    }

    @Override
    public String toString() {
        StringBuilder str = new StringBuilder();
        for(var candidate: candidates) {
            str.append(candidate);
            str.append(System.lineSeparator());
        }
        return str.toString();
    }
}
