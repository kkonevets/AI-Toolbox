#ifndef AI_TOOLBOX_FACTORED_BANDIT_MINING_PROBLEM_HEADER_FILE
#define AI_TOOLBOX_FACTORED_BANDIT_MINING_PROBLEM_HEADER_FILE

#include <AIToolbox/Factored/Bandit/Types.hpp>

namespace AIToolbox::Factored::Bandit {
    /**
     * @brief This class represents the mining bandit problem.
     *
     * This problem was introduced in the paper
     *
     * "Learning to Coordinate with Coordination Graphs in Repeated
     * Single-Stage Multi-Agent Decision Problems"
     *
     * by Bargiacchi et al.
     *
     * There are a set of villages and mines. Each village has a number of mine
     * workers. At each timestep, the village sends all its mine workers to a
     * single mine. Each timestep, each mine produces an amount of minerals
     * proportional to its hidden productivity and the number of workers sent
     * to it.
     *
     * For each index 'i', each village 'i' is always connected to the mines
     * with indeces from 'i' onwards. The last village is always connected to 4
     * mines.
     *
     * Thus, the action for a given village is a number from 0 to N; where 0
     * corresponds to sending all the workers to the 'i' mine . Action N is
     * instead sending all the workers to the mine number 'i' + N.
     *
     * The mineral amounts produced by each mine are computed with this formula:
     * - 0 if no workers are sent to it
     * - (productivity * 1.03^workers) otherwise.
     *
     * Since these amounts are deterministic for each joint action, discovering
     * the optimal action would be too easy. To generate a proper bandit, we
     * convert these amounts into stochastic rewards through Bernoulli
     * distributions.
     *
     * First, we normalize the outputs of each mine that the maximum joint
     * mineral amount that can be produced is 1. This means that, given a joint
     * action, each mine will be associated with a number between 0 and 1. We
     * use this number as the parameter of a Bernoulli distribution, which is
     * sampled to generate the mine's actual reward (either 0 or 1).
     *
     * Note that this means that it can happen that an action randomly produces
     * a higher reward than 1 (since multiple Bernoullis are sampled). However,
     * on average the optimal action will have an expected reward of 1.
     */
    class MiningBandit {
        public:
            /**
             * @brief Basic constructor.
             *
             * @param A The action space. There is one action per village, which represents to which mine to send the workers.
             * @param workersPerVillage How many workers there are in each village.
             * @param productivityPerMine The productivity factor for each mine.
             */
            MiningBandit(Action A, std::vector<unsigned> workersPerVillage, std::vector<double> productivityPerMine);

            /**
             * @brief This function samples the rewards for each mine from a set of Bernoulli distributions.
             *
             * @param a The joint action of all villages.
             *
             * @return The rewards generated by the mines.
             */
            const Rewards & sampleR(const Action & a) const;

            /**
             * @brief This function computes the deterministic regret of the input joint action.
             *
             * This function bypassed the Bernoulli distributions and directly
             * computes the true regret of any given joint action.
             *
             * @param a The joint action of all villages.
             *
             * @return The joint regret of the input action.
             */
            double getRegret(const Action & a) const;

            /**
             * @brief This function returns the optimal action for this bandit.
             */
            const Action & getOptimalAction() const;

            /**
             * @brief This function returns the joint action space.
             */
            const Action & getA() const;

            /**
             * @brief This function returns, for each mine, which villages are connected to it.
             *
             * This function returns, for each local reward function (a mine),
             * all groups of agents connected to it (villages).
             */
            const std::vector<PartialKeys> & getGroups() const;

            /**
             * @brief This function returns a set of QFunctionRule for the bandit, ignoring stochasticity.
             *
             * This function is provided for testing maximization algorithms,
             * to automatically generate rules for a given set of parameters.
             *
             * The rules contain the true underlying rewards of the problem,
             * ignoring the sampling stochasticity that is present in the
             * sampleR() function.
             *
             * In other words, finding the joint action that maximizes these
             * rules is equivalent to finding the optimal action of the bandit.
             */
            std::vector<QFunctionRule> getDeterministicRules() const;

        private:
            /**
             * @brief This function computes the per-mine Bernoulli reward probabilities.
             *
             * This function fills the internal vector helper_ with the
             * Bernoulli reward probabilities for each mine, for the specified
             * joint action.
             */
            void computeProbabilities(const Action & a) const;

            Action A;
            std::vector<unsigned> workersPerVillage_;
            std::vector<double> productivityPerMine_;

            Action optimal_;
            double rewardNorm_;

            std::vector<PartialKeys> villagesPerMine_;

            mutable Rewards helper_;
            mutable AIToolbox::RandomEngine rand_;
    };

    /**
     * @brief This function generates the parameters for a random MiningBandit.
     *
     * The parameters are uniformly sampled from the following ranges:
     *
     * - villages:              [5,  15]
     * - mines:               villages + 3
     * - workers per village:   [1,   5]
     * - mines per village:     [2,   4]
     * - productivity per mine: [0, 0.5]
     *
     * We return a tuple containing the parameters for a MiningBandit.
     * In particular:
     *
     * - The action space.
     * - How many workers there are in each village.
     * - The productivity factor for each mine.
     *
     * @param seed A random seed for a random generator to sample the parameters.
     *
     * @return A tuple containing parameters for a MiningBandit, in the order of its constructor.
     */
    std::tuple<Action, std::vector<unsigned>, std::vector<double>> makeMiningParameters(unsigned seed);
}

#endif
