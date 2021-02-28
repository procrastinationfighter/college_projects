package elections.parties;

import elections.District;

import java.util.List;

/**
 * Class representing party that always takes the cheapest actions in campaign.
 * @author Adam Boguszewski ab417730
 */
public class HumbleParty extends Party{
    /** Creates a new humble party.
     * @param name   - party name,
     * @param budget - party budget.
     * @param numberOfDistricts - number of districts in this election.
     */
    public HumbleParty(String name, int budget, int numberOfDistricts) {
        super(name, budget, numberOfDistricts);
    }

    @Override
    public boolean takeCampaignAction(List<District> districts, int[][] campaignActions) {
        District currBestDistrict = districts.get(0);
        int[] currBestAction = campaignActions[0];
        int currLowestCost = getBudget() + 1;

        for(District district: districts) {
            for(int[] action: campaignActions) {
                int currCost = calculateActionCost(district, action);
                if(currCost < currLowestCost) {
                    currBestAction = action;
                    currBestDistrict = district;
                }
            }
        }

        if(currLowestCost <= getBudget()) {
            lowerBudget(currLowestCost);
            currBestDistrict.applyCampaignEffects(currBestAction);
            return true;
        }
        else {
            return false;
        }
    }
}
