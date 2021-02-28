package elections.parties;

import elections.District;

import java.util.List;

/**
 * Class representing a party that always takes action that has
 * the largest sum of absolute values of changes,
 * but has effect on as few people as possible.
 * @author Adam Boguszewski ab417730
 */
public class RadicalParty extends Party {
    /** Creates a new party.
     * @param name   - party name,
     * @param budget - party budget.
     * @param numberOfDistricts - number of districts in this election.
     */
    public RadicalParty(String name, int budget, int numberOfDistricts) {
        super(name, budget, numberOfDistricts);
    }

    /** Calculates sum of absolute values of changes of given action.
     * @param action    - considered action.
     * @return  Sum of absolute values of vector of changes.
     */
    private int calculateSumOfAbsoluteValues(int[] action) {
        int sum = 0;
        for(var i: action) {
            sum += Math.abs(i);
        }
        return sum;
    }

    @Override
    public boolean takeCampaignAction(List<District> districts, int[][] campaignActions) {
        District currBestDistrict = districts.get(0);
        int[] currBestAction = campaignActions[0];
        int currBestSum = 0;
        int currBestCost = getBudget() + 1;

        for(District district: districts) {
            for(int[] action: campaignActions) {
                int currSum = calculateSumOfAbsoluteValues(action);
                int currCost = calculateActionCost(district, action);
                if(currSum > currBestSum && currCost <= getBudget() && currCost < currBestCost) {
                    currBestAction = action;
                    currBestDistrict = district;
                    currBestSum = currSum;
                    currBestCost = currCost;
                }
            }
        }

        if(currBestCost <= getBudget()) {
            lowerBudget(currBestCost);
            currBestDistrict.applyCampaignEffects(currBestAction);
            return true;
        }
        else {
            return false;
        }
    }
}
